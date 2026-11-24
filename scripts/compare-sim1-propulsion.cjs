const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');
const { compareBodies, originalListing, readLinkedBodies, verifyRows } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 linked propulsion run directory');
const program = 'sim1.dll';
const sha256 = bytes => createHash('sha256').update(bytes).digest('hex');
const hex = address => address.toString(16);
const originalBinary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha256(originalBinary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const rebuiltBinary = fs.readFileSync(path.join(run, 'sim1-propulsion.dll'));
const originalImage = openImage(originalBinary);
const rebuiltImage = openImage(rebuiltBinary);
assert.equal(rebuiltImage.entryRva, 0);
const originalImports = readPE32Imports(originalBinary);
const rebuiltImports = readPE32Imports(rebuiltBinary);
const map = fs.readFileSync(path.join(run, 'sim1-propulsion.map'), 'utf8');
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const body = readLinkedBodies(rebuiltImage, map, listing);
const helper = body('?AccumulateEngineThrust@sim1@@');
const collector = body('?CollectPropulsionForces@FlightDynamics@sim1@@');
const gravity = [...map.matchAll(/^\s+[0-9a-f]{4}:[0-9a-f]{8}\s+\?gravityAcceleration@sim1@@3NA\s+([0-9a-f]{8})\s/gim)];
assert.equal(gravity.length, 1, 'Missing unique gravity backing storage');
const rebuiltGravity = Number.parseInt(gravity[0][1], 16);
const originalGravity = 0x20c7c180;
const originalDivisor = 0x20bb1760;
const originalProjection = 0x20c01010;
const divisor = originalImage.codeAt(originalDivisor, 8, false);
assert.equal(divisor.readDoubleLE(), 3.0);

function references(fn, image, imports, projectedTarget, gravityTarget) {
    const { va, bytes, rows } = fn;
    const fields = image.highLowFixups(va, bytes.length).map(offset => {
        const target = bytes.readUInt32LE(offset);
        const row = rows.find(([address, encoding]) => {
            const start = Number.parseInt(address, 16) - va;
            return start <= offset && start + encoding.split(' ').length === offset + 4;
        });
        let identity = null;
        if (target === gravityTarget) identity = 'sim1.dll gravityAcceleration';
        else if (row && /^fdiv\b/i.test(row[2]) && image.codeAt(target, 8, false).equals(divisor)) identity = 'double 3.0';
        return { offset, target: hex(target), kind: 'DIR32', identity };
    });
    for (const [site, encoding] of rows.filter(row => /^e8 /.test(row[1]))) {
        const offset = Number.parseInt(site, 16) - va + 1;
        const target = va + offset + 4 + bytes.readInt32LE(offset);
        let identity = null;
        if (target === projectedTarget) {
            identity = 'sim1.dll 0x20c01010';
        } else {
            const stub = image.codeAt(target, 6);
            const imported = stub.readUInt16LE() === 0x25ff ? imports.get(stub.readUInt32LE(2)) : undefined;
            if (imported) identity = imported.dll + '!' + imported.name;
        }
        fields.push({ offset, target: hex(target), kind: 'REL32', identity });
    }
    return fields.sort((a, b) => a.offset - b.offset);
}

function compare(original, rebuilt, leftRefs, rightRefs) {
    const available = [...rightRefs];
    const pairs = [];
    for (const left of leftRefs) {
        assert(left.identity, `Unknown original reference to ${left.target}`);
        const same = right => right.identity === left.identity && right.kind === left.kind;
        let index = available.findIndex(right => same(right) && right.offset === left.offset);
        if (index === -1) index = available.findIndex(same);
        if (index === -1) continue;
        const [right] = available.splice(index, 1);
        pairs.push({ originalOffset: left.offset, rebuiltOffset: right.offset, kind: left.kind,
            identity: left.identity, originalTarget: left.target, rebuiltTarget: right.target });
    }
    const complete = pairs.length === leftRefs.length && available.length === 0 &&
        pairs.every(pair => pair.originalOffset === pair.rebuiltOffset);
    return { ...compareBodies(original, rebuilt, pairs, complete), pairs };
}

function describe(left, right, symbol, projectedTarget) {
    const originalReferences = references(left, originalImage, originalImports, originalProjection, originalGravity);
    const rebuiltReferences = references(right, rebuiltImage, rebuiltImports, projectedTarget, rebuiltGravity);
    const result = compare(left.bytes, right.bytes, originalReferences, rebuiltReferences);
    return {
        ...result, entry: hex(left.va), rebuiltEntry: hex(right.va), symbol, buildSymbol: right.name,
        originalBytes: left.bytes.length, rebuiltBytes: right.bytes.length, asm: left.rows, rebuilt: right.rows,
        leftHex: left.bytes.toString('hex'), rightHex: right.bytes.toString('hex'),
        originalReferences, rebuiltReferences,
    };
}

const originalHelper = originalListing(program, hex(originalProjection), originalImage);
const originalCollector = originalListing(program, '20c010f0', originalImage);
const projection = describe(originalHelper, helper, 'sim1::AccumulateEngineThrust');
const full = describe(originalCollector, collector, 'FlightDynamics::CollectPropulsionForces',
    projection.status === 'Different' ? undefined : helper.va);

const originalRanges = [
    [0x20c010f0, 0x20c01135, 'Enter the aligned frame and clear the force and moment sums'],
    [0x20c01175, 0x20c011ab, 'Project one engine thrust with a center-of-gravity-relative arm'],
    [0x20c01229, 0x20c0126c, 'Apply the optional propeller arm offset'],
    [0x20c012b5, 0x20c012ea, 'Apply the moment gate and publish thrust-to-weight'],
    [0x20c012ce, 0x20c012ea, 'Publish thrust-to-weight before the blend branch'],
    [0x20c0133f, 0x20c01392, 'Publish both propulsion vectors and return from the callback'],
];
const scopes = [];
for (const [start, end, name] of originalRanges) {
    const size = end - start;
    const originalOffset = start - originalCollector.va;
    const leftBytes = originalCollector.bytes.subarray(originalOffset, originalOffset + size);
    const rowsIn = (rows, va) => rows.filter(([address]) => {
        const value = Number.parseInt(address, 16);
        return value >= va && value < va + size;
    });
    const refsIn = (refs, offset) => refs.filter(field => field.offset >= offset && field.offset < offset + size)
        .map(field => ({ ...field, offset: field.offset - offset }));
    const asm = rowsIn(originalCollector.rows, start);
    verifyRows(asm, start, leftBytes);
    for (const [address] of collector.rows) {
        const va = Number.parseInt(address, 16);
        const offset = va - collector.va;
        const rightBytes = collector.bytes.subarray(offset, offset + size);
        if (rightBytes.length !== size) break;
        const match = compare(leftBytes, rightBytes, refsIn(full.originalReferences, originalOffset),
            refsIn(full.rebuiltReferences, offset));
        if (match.status === 'Different') continue;
        const rebuilt = rowsIn(collector.rows, va);
        verifyRows(rebuilt, va, rightBytes);
        assert(!scopes.some(item => item.entry === hex(start)), 'Ambiguous exact scope ' + name);
        scopes.push({ ...match, entry: hex(start), rebuiltEntry: address, bytes: size, name, asm, rebuilt,
            leftHex: leftBytes.toString('hex'), rightHex: rightBytes.toString('hex') });
    }
}

const report = { program, artifactKind: 'linked PE32 DLL', originalSHA256, rebuiltSHA256: sha256(rebuiltBinary),
    callback: full, projection, scopes };
fs.writeFileSync(path.join(run, 'propulsion-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const result of [full, projection]) {
    console.log(`${result.entry} ${result.status}: ${result.originalBytes} original / ${result.rebuiltBytes} linked bytes`);
}
for (const scope of scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
