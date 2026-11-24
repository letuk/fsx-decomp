const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { openImage } = require('./lib/pe32-dll.cjs');
const { compareBodies, dumpbinRows, originalListing, verifyRows } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 state integration run directory');
const sha = data => createHash('sha256').update(data).digest('hex');
const program = 'sim1.dll';
const originalBytes = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(originalBytes);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(originalBytes);
const headingVtable = 0x20bbdb1c;
const headingTargets = [0x20bdec10, 0x20bdeec0, 0x20bdef30, 0x20bdefa0];
assert.deepEqual(image.highLowFixups(headingVtable, 16), [0, 4, 8, 12]);
for (const [slot, target] of headingTargets.entries()) {
    assert.equal(image.initialDword(headingVtable + slot * 4), target);
}
const objectBytes = fs.readFileSync(path.join(run, 'sim1-state-integration.obj'));
const coff = parseNativeCoff(objectBytes);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');

const targets = [
    ['20c42d90', 'PrepareAccelerations', 'FlightDynamics'],
    ['20c43110', 'IntegrateVelocities', 'FlightDynamics'],
    ['20bdeec0', 'RotateForward', 'HeadingTransform'],
    ['20bdef30', 'RotateInverse', 'HeadingTransform'],
    ['20bdefa0', 'Refresh', 'HeadingTransform'],
];

function readOriginalReference(bytes, offset) {
    const va = bytes.readUInt32LE(offset);
    if (va === 0x20c7c180) return { offset, va, identity: 'gravityAcceleration' };
    assert(va === 0x20bb4650 || (va >= 0x20bc3598 && va <= 0x20bc4d60),
        `Unexpected original reference ${va.toString(16)}`);
    return { offset, va, identity: image.codeAt(va, 8, false).toString('hex') };
}

function readCoffReference(relocation, bytes) {
    const { offset, symbol, type } = relocation;
    assert.equal(type, 6, `Expected a DIR32 relocation at ${offset}`);
    assert.equal(bytes.readUInt32LE(offset), 0, 'Expected a zero COFF relocation addend');
    if (symbol.name === '?gravityAcceleration@sim1@@3NA') {
        assert.equal(symbol.section, 0, 'Expected the external loader-owned gravity value');
        return { offset, symbol: symbol.name, identity: 'gravityAcceleration' };
    }
    assert.match(symbol.name, /^__real@[0-9a-f]{16}$/);
    const section = coff.sections[symbol.section - 1];
    assert(section && symbol.value + 8 <= section.size);
    const literal = Buffer.from(symbol.name.slice(7), 'hex').reverse();
    assert(coff.buffer.subarray(section.raw + symbol.value, section.raw + symbol.value + 8).equals(literal));
    return { offset, symbol: symbol.name, identity: literal.toString('hex') };
}

const functions = targets.map(([entry, name, owner]) => {
    const old = originalListing(program, entry, image);
    const symbols = [...coff.byName.keys()].filter(symbol => symbol.startsWith(`?${name}@${owner}@sim1@@`));
    assert.equal(symbols.length, 1, name);
    const symbol = coff.unique(symbols[0]);
    const section = coff.sections[symbol.section - 1];
    assert.equal(section.name, '.text');
    assert.equal(symbol.value, 0);
    const rebuilt = coff.buffer.subarray(section.raw, section.raw + section.size);
    const start = listing.indexOf(`\n${symbol.name} (`);
    assert(start >= 0, `Missing dumpbin symbol ${symbol.name}`);
    const rebuiltRows = dumpbinRows(listing.slice(start).split(/\r?\n\s*\r?\n/, 1)[0]);
    verifyRows(rebuiltRows, 0, rebuilt);

    const originalReferences = image.highLowFixups(old.va, old.bytes.length).map(offset => readOriginalReference(old.bytes, offset));
    const rebuiltReferences = section.relocations.map(relocation => readCoffReference(relocation, rebuilt));
    const unmatched = [...rebuiltReferences];
    const pairs = [];
    for (const reference of originalReferences) {
        let index = unmatched.findIndex(item => item.identity === reference.identity && item.offset === reference.offset);
        if (index < 0) index = unmatched.findIndex(item => item.identity === reference.identity);
        if (index < 0) continue;
        const [match] = unmatched.splice(index, 1);
        pairs.push({ originalOffset: reference.offset, rebuiltOffset: match.offset,
            originalVA: reference.va.toString(16), symbol: match.symbol, identity: match.identity });
    }
    const sameTargetFields = pairs.length === originalReferences.length && unmatched.length === 0 &&
        pairs.every(pair => pair.originalOffset === pair.rebuiltOffset);
    const result = compareBodies(old.bytes, rebuilt, pairs, sameTargetFields);
    return { entry, symbol: `${owner}::${name}`, coffSymbol: symbol.name, ...result,
        originalBytes: old.bytes.length, rebuiltBytes: rebuilt.length, originalReferences, rebuiltReferences, pairs,
        leftHex: old.bytes.toString('hex'), rightHex: rebuilt.toString('hex'), rebuilt: rebuiltRows };
});

const integrator = functions.find(item => item.entry === '20c43110');
const historyCopyStart = 0x20c4312d;
const historyCopyEnd = 0x20c431bd;
const originalCopy = image.codeAt(historyCopyStart, historyCopyEnd - historyCopyStart);
assert.deepEqual(image.highLowFixups(historyCopyStart, originalCopy.length), []);
const integrated = Buffer.from(integrator.rightHex, 'hex');
const rebuiltCopyStart = integrated.indexOf(originalCopy);
const copyLength = originalCopy.length;
const scopes = [];
if (rebuiltCopyStart >= 0) {
    const rebuiltCopyEnd = rebuiltCopyStart + copyLength;
    const bytes = integrated.subarray(rebuiltCopyStart, rebuiltCopyEnd);
    const originalRows = originalListing(program, integrator.entry, image).rows.filter(row => {
        const address = Number.parseInt(row[0], 16);
        return address >= historyCopyStart && address < historyCopyEnd;
    });
    const rebuiltRows = integrator.rebuilt.filter(row => {
        const offset = Number.parseInt(row[0], 16);
        return offset >= rebuiltCopyStart && offset < rebuiltCopyEnd;
    });
    assert.equal(integrated.indexOf(originalCopy, rebuiltCopyStart + 1), -1, 'Expected a unique rebuilt history block');
    assert(integrator.rebuiltReferences.every(ref => ref.offset < rebuiltCopyStart || ref.offset >= rebuiltCopyEnd));
    verifyRows(originalRows, historyCopyStart, originalCopy);
    verifyRows(rebuiltRows, rebuiltCopyStart, bytes);
    scopes.push({ entry: historyCopyStart.toString(16), parentEntry: integrator.entry,
        symbol: integrator.symbol, status: 'Exact', full: false, originalBytes: copyLength, rebuiltBytes: copyLength,
        rebuildSectionOffset: rebuiltCopyStart, asm: originalRows, rebuilt: rebuiltRows,
        leftHex: originalCopy.toString('hex'), rightHex: bytes.toString('hex') });
}

const report = { program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(objectBytes),
    headingVtable: { entry: headingVtable.toString(16), originalTargets: headingTargets.map(va => va.toString(16)) },
    functions, scopes };
fs.writeFileSync(path.join(run, 'state-integration-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of functions) console.log(`${item.entry} ${item.symbol} ${item.status} ${item.originalBytes}/${item.rebuiltBytes}`);
for (const item of scopes) console.log(`${item.entry} acceleration history copy ${item.status} ${item.originalBytes} bytes`);
for (const method of functions.filter(item => item.symbol.startsWith('HeadingTransform::'))) {
    assert.equal(method.status, 'Exact', `${method.symbol} COFF regression`);
}
assert.equal(scopes.length, 1, 'Expected the exact acceleration history copy range');
