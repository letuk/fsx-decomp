const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { compareBodies, dumpbinRows, originalListing, verifyRows } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 native turbine run directory');
const program = 'sim1.dll';
const entry = '20c2bc20';
const sha = data => createHash('sha256').update(data).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const { bytes: original, va, rows: asm } = originalListing(program, entry, image);
const callSite = 0x20c2b8d5;
const call = image.codeAt(callSite, 5);
assert.equal(call[0], 0xe8);
assert.equal(callSite + 5 + call.readInt32LE(1), va);

const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const coff = parseNativeCoff(file);
const symbol = 'EngineForceRecord::UpdateTurbineSpeedOutput';
const candidates = [...coff.byName.keys()].filter(name => name.startsWith('?UpdateTurbineSpeedOutput@EngineForceRecord@sim1@@'));
assert.equal(candidates.length, 1);
const built = coff.unique(candidates[0]);
const section = coff.sections[built.section - 1];
assert.equal(section.name, '.text');
assert.equal(built.value, 0);
const rebuilt = file.subarray(section.raw, section.raw + section.size);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const start = listing.indexOf(`\n${built.name} (`);
assert(start >= 0, `Missing dumpbin symbol ${built.name}`);
const rebuiltRows = dumpbinRows(listing.slice(start).split(/\r?\n\s*\r?\n/, 1)[0]);
verifyRows(rebuiltRows, 0, rebuilt);

const constants = new Map([[0x20bb4650, 0.5], [0x20bbeaf8, 120], [0x20bbf7d8, 0.2], [0x20bc3640, 80], [0x20bb4048, 10]]);
const originalReferences = image.highLowFixups(va, original.length).map(offset => {
    const target = original.readUInt32LE(offset);
    assert(constants.has(target));
    const literal = image.codeAt(target, 8, false);
    assert.equal(literal.readDoubleLE(), constants.get(target));
    return { offset, target: target.toString(16), value: constants.get(target), identity: literal.toString('hex') };
});
assert.equal(originalReferences.length, constants.size);
const rebuiltReferences = section.relocations.map(({ offset, type, symbol: target }) => {
    assert.equal(type, 6, 'Expected a native DIR32 operand');
    assert.equal(rebuilt.readUInt32LE(offset), 0);
    const storage = coff.sections[target.section - 1];
    const decoded = /^__real@([0-9a-f]{8}|[0-9a-f]{16})$/.exec(target.name);
    const size = decoded ? decoded[1].length / 2 : 0;
    assert(storage && size && target.value + size <= storage.size, `Unknown COFF literal ${target.name}`);
    const literal = file.subarray(storage.raw + target.value, storage.raw + target.value + size);
    assert.equal(literal.toString('hex'), Buffer.from(decoded[1], 'hex').reverse().toString('hex'));
    return { offset, target: target.name, value: size === 8 ? literal.readDoubleLE() : literal.readFloatLE(),
        identity: literal.toString('hex') };
});
const unmatched = [...rebuiltReferences];
const pairs = originalReferences.flatMap(left => {
    let index = unmatched.findIndex(right => right.identity === left.identity && right.offset === left.offset);
    if (index < 0) index = unmatched.findIndex(right => right.identity === left.identity);
    if (index < 0) return [];
    const [right] = unmatched.splice(index, 1);
    return [{ originalOffset: left.offset, rebuiltOffset: right.offset, originalTarget: left.target,
        rebuiltTarget: right.target, value: left.value, identity: left.identity }];
});
const complete = pairs.length === originalReferences.length && unmatched.length === 0 &&
    pairs.every(pair => pair.originalOffset === pair.rebuiltOffset);
const result = compareBodies(original, rebuilt, pairs, complete);
const thresholdSize = 0x15;
const thresholdPairs = pairs.filter(pair => pair.originalOffset === 7 && pair.rebuiltOffset === 7 && pair.value === 0.5);
const threshold = compareBodies(original.subarray(0, thresholdSize), rebuilt.subarray(0, thresholdSize),
    thresholdPairs, thresholdPairs.length === 1 &&
    originalReferences.filter(item => item.offset < thresholdSize).length === 1 &&
    rebuiltReferences.filter(item => item.offset < thresholdSize).length === 1);
const scopes = [];
if (threshold.status !== 'Different') {
    const originalThreshold = asm.filter(([address]) => Number.parseInt(address, 16) < va + thresholdSize);
    const rebuiltThreshold = rebuiltRows.filter(([offset]) => Number.parseInt(offset, 16) < thresholdSize);
    verifyRows(originalThreshold, va, original.subarray(0, thresholdSize));
    verifyRows(rebuiltThreshold, 0, rebuilt.subarray(0, thresholdSize));
    assert.equal(va + thresholdSize + original.readInt8(thresholdSize - 1), 0x20c2bc48);
    const lowerThreshold = thresholdSize + rebuilt.readInt8(thresholdSize - 1);
    assert(rebuiltReferences.some(item => item.offset === lowerThreshold + 2 && item.value === 0.2));
    scopes.push({ entry, bytes: thresholdSize, name: 'Entry and high-speed threshold branch', ...threshold,
        pairs: thresholdPairs, asm: originalThreshold, rebuilt: rebuiltThreshold,
        leftHex: original.subarray(0, thresholdSize).toString('hex'),
        rightHex: rebuilt.subarray(0, thresholdSize).toString('hex') });
}
const report = { program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), entry,
    symbol, buildSymbol: built.name, ...result, originalBytes: original.length, rebuiltBytes: rebuilt.length,
    originalReferences, rebuiltReferences, pairs, leftHex: original.toString('hex'), rightHex: rebuilt.toString('hex'),
    asm, rebuilt: rebuiltRows, scopes };
fs.writeFileSync(path.join(run, 'turbine-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${entry} ${result.status}: ${original.length} original / ${rebuilt.length} native COFF bytes`);
for (const scope of scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
