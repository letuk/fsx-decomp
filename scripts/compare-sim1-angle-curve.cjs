const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { compareBodies, dumpbinRows, originalListing, verifyRows } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 aerodynamic moment run directory');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const file = fs.readFileSync(path.join(run, 'sim1-aerodynamic-moment.obj'));
const coff = parseNativeCoff(file);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const hex = address => address.toString(16);
const scaleVA = 0x20bbf7f0;
const scale = image.codeAt(scaleVA, 8, false);
assert.equal(scale.readDoubleLE(), 0.017453292519943295);
const scaleIdentity = scale.toString('hex');
const slotTargets = [[0x20bc2180, 0x20c14e50], [0x20bc266c, 0x20c4fcb0], [0x20bc2670, 0x20c4ff30]];
for (const [slot, target] of slotTargets) {
    assert.deepEqual(image.highLowFixups(slot, 4), [0]);
    assert.equal(image.initialDword(slot), target);
}
const originalLookup = originalListing(program, '20c4fcb0', image);
const originalCurve = originalListing(program, '20bcbf60', image);
const callSite = 0x20c4fcce - originalLookup.va;
assert.equal(originalLookup.bytes[callSite], 0xe8);
assert.equal(originalLookup.va + callSite + 5 + originalLookup.bytes.readInt32LE(callSite + 1), originalCurve.va);

function readBody(prefix) {
    const names = [...coff.byName.keys()].filter(name => name.startsWith(prefix));
    assert.equal(names.length, 1, prefix);
    const symbol = coff.unique(names[0]);
    const section = coff.sections[symbol.section - 1];
    assert.equal(section.name, '.text');
    assert.equal(symbol.value, 0);
    const bytes = file.subarray(section.raw, section.raw + section.size);
    const start = listing.indexOf(`\n${symbol.name} (`);
    assert(start >= 0, symbol.name);
    const rows = dumpbinRows(listing.slice(start).split(/\r?\n\s*\r?\n/, 1)[0]);
    verifyRows(rows, 0, bytes);
    return { symbol, section, bytes, rows };
}
const curve = readBody('?SampleAerodynamicCurve@sim1@@');
const moment = readBody('?CalculateAngleCurveMoment@AirplaneMomentModel@sim1@@');
assert.equal(curve.section.relocations.length, 0);
const references = moment.section.relocations.map(({ offset, symbol, type }) => {
    assert.equal(moment.bytes.readUInt32LE(offset), 0);
    if (type === 20) {
        assert.equal(moment.bytes[offset - 1], 0xe8);
        assert.equal(symbol.index, curve.symbol.index);
        return { offset, identity: 'sim1.dll 0x20bcbf60' };
    }
    assert.equal(type, 6);
    assert.equal(symbol.name, '__real@3f91df46a2529d39');
    const section = coff.sections[symbol.section - 1];
    assert(section && symbol.value + 8 <= section.size);
    assert(file.subarray(section.raw + symbol.value, section.raw + symbol.value + 8).equals(scale));
    return { offset, identity: scaleIdentity };
});
assert.deepEqual(references.map(item => item.identity).sort(), ['sim1.dll 0x20bcbf60', scaleIdentity].sort());

const originalMoment = originalListing(program, '20c013a0', image);
const momentStart = 0x20c0140b;
const momentEnd = 0x20c01438;
const momentRefs = image.highLowFixups(momentStart, momentEnd - momentStart).map(offset => {
    const va = momentStart + offset;
    assert.equal(image.codeAt(va, 4).readUInt32LE(), scaleVA);
    return { offset: va - originalMoment.va, identity: scaleIdentity, target: hex(scaleVA) };
});
assert.equal(momentRefs.length, 1);
function select(rows, start, end) {
    return rows.filter(([address]) => {
        const va = Number.parseInt(address, 16);
        return va >= start && va < end;
    });
}

function describe(original, rebuilt, begin, end, sourceSymbol, originalRefs, rebuiltRefs, ranges) {
    const offset = begin - original.va;
    const left = original.bytes.subarray(offset, offset + end - begin);
    const asm = select(original.rows, begin, end);
    verifyRows(asm, begin, left);
    const scopes = [];
    for (const [start, stop, name] of ranges) {
        const size = stop - start;
        const originalOffset = start - original.va;
        const originalSlice = original.bytes.subarray(originalOffset, originalOffset + size);
        const originalRows = select(asm, start, stop);
        verifyRows(originalRows, start, originalSlice);
        const parts = (refs, offset) => refs.filter(item => item.offset >= offset && item.offset < offset + size);
        const leftRefs = parts(originalRefs, originalOffset);
        for (const [address] of rebuilt.rows) {
            const index = Number.parseInt(address, 16);
            const bytes = rebuilt.bytes.subarray(index, index + size);
            if (bytes.length !== size) break;
            const rightRefs = parts(rebuiltRefs, index);
            const pairs = leftRefs.flatMap(item => {
                const fieldOffset = item.offset - originalOffset;
                return rightRefs.some(r => r.offset - index === fieldOffset && r.identity === item.identity)
                    ? [{ originalOffset: fieldOffset, rebuiltOffset: fieldOffset, identity: item.identity }] : [];
            });
            const compared = compareBodies(originalSlice, bytes, pairs, pairs.length === leftRefs.length && pairs.length === rightRefs.length);
            if (compared.status === 'Different') continue;
            const rebuiltRows = select(rebuilt.rows, index, index + size);
            if (rebuiltRows.reduce((n, row) => n + row[1].split(' ').length, 0) !== size) continue;
            const tail = originalRows.at(-1)[2];
            if (/\b(?:J[A-Z]+|CALL|RET)\b/i.test(tail)) continue;
            verifyRows(rebuiltRows, index, bytes);
            scopes.push({ entry: hex(start), name, bytes: size, rebuiltEntry: address, ...compared, pairs,
                asm: originalRows, rebuilt: rebuiltRows, leftHex: originalSlice.toString('hex'), rightHex: bytes.toString('hex') });
        }
    }
    return { entry: hex(begin), endExclusive: hex(end), parentEntry: hex(original.va), symbol: sourceSymbol,
        buildSymbol: rebuilt.symbol.name, status: left.equals(rebuilt.bytes) ? 'Exact' : 'Different',
        originalBytes: left.length, rebuiltBytes: rebuilt.bytes.length, asm, rebuilt: rebuilt.rows,
        leftHex: left.toString('hex'), rightHex: rebuilt.bytes.toString('hex'), originalRefs, rebuiltRefs, scopes };
}

const interpolation = describe(originalCurve, curve, originalCurve.va, originalCurve.va + originalCurve.bytes.length,
    'sim1::SampleAerodynamicCurve', [], [], [
        [0x20bcbfff, 0x20bcc023, 'Evaluate the interpolation between adjacent samples'],
        [0x20bcc011, 0x20bcc023, 'Calculate interval fraction and interpolate'],
        [0x20bcc013, 0x20bcc023, 'Divide by the sample interval and interpolate the coefficient'],
        [0x20bcc01d, 0x20bcc023, 'Multiply the coefficient difference by the fraction and add its base'],
    ]);
const contribution = describe(originalMoment, moment, momentStart, momentEnd,
    'AirplaneMomentModel::CalculateAngleCurveMoment', momentRefs, references, [
        [0x20c01414, 0x20c0141d, 'Convert the angle to radians and store the lookup argument'],
        [0x20c01414, 0x20c0141a, 'Multiply by the degree-to-radian conversion factor'],
        [0x20c01423, 0x20c01429, 'Multiply the moment coefficient by the pressure and geometry factor'],
    ]);
const report = { program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    coefficientLookup: { entry: '20c4fcb0', dataOffset: '0x1104', slotTargets }, interpolation, contribution };
fs.writeFileSync(path.join(run, 'angle-curve-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [interpolation, contribution]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} extracted bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
