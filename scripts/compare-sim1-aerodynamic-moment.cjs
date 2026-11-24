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
const entry = '20c013a0';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const original = originalListing(program, entry, image);
const file = fs.readFileSync(path.join(run, 'sim1-aerodynamic-moment.obj'));
const coff = parseNativeCoff(file);
const names = [...coff.byName.keys()].filter(name => name.startsWith('?CalculatePropellerGyroscopicMoment@AirplaneMomentModel@sim1@@'));
assert.equal(names.length, 1);
const symbol = coff.unique(names[0]);
const section = coff.sections[symbol.section - 1];
assert.equal(section.name, '.text');
assert.equal(symbol.value, 0);
const rebuilt = file.subarray(section.raw, section.raw + section.size);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const start = listing.indexOf(`\n${symbol.name} (`);
assert(start >= 0, `Missing dumpbin symbol ${symbol.name}`);
const rebuiltRows = dumpbinRows(listing.slice(start).split(/\r?\n\s*\r?\n/, 1)[0]);
verifyRows(rebuiltRows, 0, rebuilt);

const startVA = 0x20c0155d;
const endVA = 0x20c016ad;
const left = original.bytes.subarray(startVA - original.va, endVA - original.va);
const select = (rows, start, end) => rows.filter(([address]) => {
    const value = Number.parseInt(address, 16);
    return value >= start && value < end;
});
const asm = select(original.rows, startVA, endVA);
verifyRows(asm, startVA, left);
const rotationScale = image.codeAt(0x20bc35a8, 8, false);
assert.equal(rotationScale.readDoubleLE(), 0.1047197551);
const originalReferences = image.highLowFixups(startVA, left.length).map(offset => {
    const target = left.readUInt32LE(offset);
    assert.equal(target, 0x20bc35a8);
    return { offset, target: target.toString(16), value: rotationScale.readDoubleLE(), identity: rotationScale.toString('hex') };
});
assert.equal(originalReferences.length, 1);
const rebuiltReferences = section.relocations.map(({ offset, symbol: target, type }) => {
    assert.equal(type, 6);
    assert.equal(rebuilt.readUInt32LE(offset), 0);
    const literal = /^__real@([0-9a-f]{16})$/.exec(target.name);
    assert(literal, `Unexpected COFF literal ${target.name}`);
    const data = coff.sections[target.section - 1];
    assert(data && target.value + 8 <= data.size);
    const contents = file.subarray(data.raw + target.value, data.raw + target.value + 8);
    assert(contents.equals(Buffer.from(literal[1], 'hex').reverse()));
    return { offset, target: target.name, value: contents.readDoubleLE(), identity: contents.toString('hex') };
});
const rotationScaleCopies = rebuiltReferences.filter(item => item.identity === rotationScale.toString('hex'));
assert(rotationScaleCopies.length >= 1, 'Missing the original RPM conversion literal');

const scopes = [];
for (const [address, end, name] of [
    [0x20c01570, 0x20c0157a, 'Test positive ordered gyro gain'],
    [0x20c015c4, 0x20c015e7, 'First propeller contribution in the four-record loop'],
    [0x20c01677, 0x20c01693, 'Multiply and sum a remainder propeller contribution'],
    [0x20c01681, 0x20c01693, 'Apply inertia, angular rate and gain in the remainder loop'],
    [0x20c01687, 0x20c0169b, 'Apply angular rate and both gyro factors, sum and close the remainder loop'],
    [0x20c0168d, 0x20c01693, 'Apply both gyro factors and add the remainder contribution'],
]) {
    const leftBytes = original.bytes.subarray(address - original.va, end - original.va);
    const size = leftBytes.length;
    const leftRows = select(original.rows, address, end);
    verifyRows(leftRows, address, leftBytes);
    for (const [addressOffset] of rebuiltRows) {
        const offset = Number.parseInt(addressOffset, 16);
        const bytes = rebuilt.subarray(offset, offset + size);
        if (!leftBytes.equals(bytes) || rebuiltReferences.some(item => item.offset >= offset && item.offset < offset + size)) continue;
        const rightRows = select(rebuiltRows, offset, offset + size);
        verifyRows(rightRows, offset, bytes);
        let loopBackEdge;
        if (address === 0x20c01687) {
            const branchOffset = 0x20c01693 - address;
            const originalLoop = address + branchOffset + 2 + leftBytes.readInt8(branchOffset + 1);
            const rebuiltLoop = offset + branchOffset + 2 + bytes.readInt8(branchOffset + 1);
            assert.equal(originalLoop, 0x20c01677);
            const loadIndex = rebuiltRows.findIndex(([address]) => Number.parseInt(address, 16) === rebuiltLoop);
            assert(loadIndex >= 0);
            const propellerLoad = /^fld qword ptr \[(e[a-z]+)\]$/i.exec(rebuiltRows[loadIndex][2]);
            assert(propellerLoad && rebuiltRows[loadIndex + 1][2].toLowerCase() === `add ${propellerLoad[1].toLowerCase()},54h`);
            loopBackEdge = { original: originalLoop.toString(16), rebuilt: rebuiltLoop.toString(16) };
        }
        scopes.push({ entry: address.toString(16), name, bytes: size, rebuiltEntry: addressOffset, status: 'Exact',
            asm: leftRows, rebuilt: rightRows, leftHex: leftBytes.toString('hex'), rightHex: bytes.toString('hex'),
            ...(loopBackEdge ? { loopBackEdge } : {}) });
    }
}

const report = { program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    parentEntry: entry, entry: startVA.toString(16), endExclusive: endVA.toString(16),
    symbol: 'AirplaneMomentModel::CalculatePropellerGyroscopicMoment', buildSymbol: symbol.name,
    status: 'Different', originalBytes: left.length, rebuiltBytes: rebuilt.length, asm, rebuilt: rebuiltRows,
    leftHex: left.toString('hex'), rightHex: rebuilt.toString('hex'), originalReferences, rebuiltReferences,
    verifiedReference: { original: originalReferences[0], rebuilt: rotationScaleCopies }, scopes };
fs.writeFileSync(path.join(run, 'aerodynamic-moment-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${report.entry} Different: ${left.length} original in-function bytes / ${rebuilt.length} native COFF bytes`);
for (const scope of scopes) console.log(`${scope.entry} Exact: ${scope.bytes} bytes, ${scope.name}`);
