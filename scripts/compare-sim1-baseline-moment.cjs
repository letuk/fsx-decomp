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
const literals = new Map([[0x20bb18e8, 0.0004885197851], [0x20bc3658, 0.3]]);
const slot = 0x20bc2670;
assert.deepEqual(image.highLowFixups(slot, 4), [0]);
assert.equal(image.initialDword(slot), 0x20c4ff30);

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
const original = originalListing(program, '20c4ff30', image);
const baseline = readBody('?CalculateBaselineMoment@AirplaneCoefficientModel@sim1@@');
const scaled = readBody('?CalculateScaledBaselineMoment@AirplaneMomentModel@sim1@@');
const originalRefs = image.highLowFixups(original.va, original.bytes.length).map(offset => {
    const address = original.bytes.readUInt32LE(offset);
    const bytes = image.codeAt(address, 8, false);
    assert.equal(bytes.readDoubleLE(), literals.get(address));
    return { offset, address: hex(address), value: literals.get(address), identity: bytes.toString('hex') };
});
assert.equal(originalRefs.length, 3);
const rebuiltRefs = baseline.section.relocations.map(({ offset, type, symbol }) => {
    assert.equal(type, 6);
    assert.equal(baseline.bytes.readUInt32LE(offset), 0);
    assert.match(symbol.name, /^__real@[0-9a-f]{16}$/);
    const data = coff.sections[symbol.section - 1];
    assert(data && symbol.value + 8 <= data.size);
    const bytes = file.subarray(data.raw + symbol.value, data.raw + symbol.value + 8);
    const identity = bytes.toString('hex');
    assert.equal(identity, Buffer.from(symbol.name.slice(7), 'hex').reverse().toString('hex'));
    assert(originalRefs.some(item => item.identity === identity), `Unknown literal ${symbol.name}`);
    return { offset, symbol: symbol.name, value: bytes.readDoubleLE(), identity };
});
assert.equal(scaled.section.relocations.length, 1);
const scaledCall = scaled.section.relocations[0];
assert.equal(scaledCall.type, 20);
assert.equal(scaledCall.symbol.index, baseline.symbol.index);
assert.equal(scaled.bytes[scaledCall.offset - 1], 0xe8);
assert.equal(scaled.bytes.readUInt32LE(scaledCall.offset), 0);

function select(rows, first, last) {
    return rows.filter(([address]) => {
        const va = Number.parseInt(address, 16);
        return va >= first && va < last;
    });
}
function compareScopes(left, right, ranges, leftRefs, rightRefs) {
    const matches = [];
    for (const [start, end, name] of ranges) {
        const length = end - start;
        const from = start - left.va;
        const originalBytes = left.bytes.subarray(from, from + length);
        const originalRows = select(left.rows, start, end);
        verifyRows(originalRows, start, originalBytes);
        const refs = (items, offset) => items.filter(item => item.offset >= offset && item.offset < offset + length);
        const originalFields = refs(leftRefs, from);
        for (const [address] of right.rows) {
            const offset = Number.parseInt(address, 16);
            const rebuiltBytes = right.bytes.subarray(offset, offset + length);
            if (rebuiltBytes.length !== length) break;
            const fields = refs(rightRefs, offset);
            const pairs = originalFields.flatMap(field => {
                const originalOffset = field.offset - from;
                return fields.some(item => item.offset - offset === originalOffset && item.identity === field.identity)
                    ? [{ originalOffset, rebuiltOffset: originalOffset, identity: field.identity, target: field.address }] : [];
            });
            const result = compareBodies(originalBytes, rebuiltBytes, pairs,
                pairs.length === fields.length && pairs.length === originalFields.length);
            if (result.status === 'Different') continue;
            const rebuiltRows = select(right.rows, offset, offset + length);
            if (rebuiltRows.reduce((n, row) => n + row[1].split(' ').length, 0) !== length) continue;
            verifyRows(rebuiltRows, offset, rebuiltBytes);
            matches.push({ entry: hex(start), rebuiltEntry: address, name, bytes: length, ...result, pairs,
                asm: originalRows, rebuilt: rebuiltRows, leftHex: originalBytes.toString('hex'),
                rightHex: rebuiltBytes.toString('hex') });
        }
    }
    return matches;
}

const baselineScopes = compareScopes(original, baseline, [
    [0x20c4ff8e, 0x20c4ffa3, 'Interpolate, scale and add the Mach correction'],
    [0x20c4ff99, 0x20c4ffa3, 'Scale and add the interpolated Mach correction'],
    [0x20c4ff99, 0x20c4ff9f, 'Scale the interpolated Mach correction'],
    [0x20c4ffc3, 0x20c4ffd7, 'Apply the optional modifier with 0.3 weighting'],
    [0x20c4ffc3, 0x20c4ffd3, 'Calculate the optional modifier with 0.3 weighting'],
    [0x20c4ffdd, 0x20c50008, 'Blend the Mach-adjusted coefficient with realism tuning'],
    [0x20c4fff4, 0x20c50008, 'Mix the realism scale and bias'],
    [0x20c4fff9, 0x20c50008, 'Mix the weighted coefficient and bias'],
    [0x20c4fffe, 0x20c50008, 'Apply the tuning scale and add the bias contribution'],
], originalRefs, rebuiltRefs);

const fullMoment = originalListing(program, '20c013a0', image);
const start = 0x20c013e6;
const end = 0x20c0140b;
const originalScaled = fullMoment.bytes.subarray(start - fullMoment.va, end - fullMoment.va);
const originalScaledRows = select(fullMoment.rows, start, end);
verifyRows(originalScaledRows, start, originalScaled);
const scaledScopes = compareScopes(fullMoment, scaled, [
    [0x20c013f9, 0x20c013ff, 'Scale the baseline by pressure and geometry'],
], [], [{ offset: scaledCall.offset, identity: 'sim1.dll 0x20c4ff30' }]);

const describe = (entry, left, asm, right, scopes, symbol) => ({
    entry: hex(entry), symbol, buildSymbol: right.symbol.name,
    status: left.equals(right.bytes) && right.section.relocations.length === 0 ? 'Exact' : 'Different',
    originalBytes: left.length, rebuiltBytes: right.bytes.length,
    asm, rebuilt: right.rows, leftHex: left.toString('hex'), rightHex: right.bytes.toString('hex'), scopes,
});
const report = { program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    baselineSlot: hex(slot),
    baseline: { ...describe(original.va, original.bytes, original.rows, baseline, baselineScopes,
        'AirplaneCoefficientModel::CalculateBaselineMoment'), originalRefs, rebuiltRefs },
    scaled: { ...describe(start, originalScaled, originalScaledRows, scaled, scaledScopes,
        'AirplaneMomentModel::CalculateScaledBaselineMoment'), parentEntry: '20c013a0', verifiedCallOffset: scaledCall.offset },
};
fs.writeFileSync(path.join(run, 'baseline-moment-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [report.baseline, report.scaled]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} native COFF bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
