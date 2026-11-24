const assert = require('node:assert/strict');
const { createHash } = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { openImage } = require('./lib/pe32-dll.cjs');

const root = path.resolve(__dirname, '..');
const run = process.argv[2];
assert(run, 'Pass a retained API factory run directory');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const entry = 0x2013bdcc;
const length = 34;
const ordinal = 172;
const symbol = '_GetCallbackArrayRecordPilot@8';
const evidence = 'artifacts/api.dll/callback-array-record-20261119/ghidra-2013bdcc.json';
const live = JSON.parse(fs.readFileSync(path.join(root, evidence), 'utf8'));
assert.equal(live.program, 'api.dll');
assert.equal(live.entry_point, entry.toString(16));
assert.equal(parseInt(live.body_end, 16), entry + length - 1);

const original = fs.readFileSync(path.join(root, 'original/api.dll'));
const originalSHA256 = sha(original);
assert.equal(originalSHA256, '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const image = openImage(original);
const pe = original.readUInt32LE(0x3c);
const exportDirectory = image.codeAt(image.base + original.readUInt32LE(pe + 120), 40, false);
const index = ordinal - exportDirectory.readUInt32LE(16);
assert(index >= 0 && index < exportDirectory.readUInt32LE(20));
const exportSlot = image.base + exportDirectory.readUInt32LE(28) + index * 4;
assert.equal(image.base + image.codeAt(exportSlot, 4, false).readUInt32LE(), entry);

const originalBody = image.codeAt(entry, length);
assert.deepEqual(image.highLowFixups(entry, length), []);
let next = entry;
for (const [address, hex] of live.asm) {
    assert.equal(parseInt(address, 16), next);
    next += Buffer.from(hex, 'hex').length;
}
assert.equal(next, entry + length);
assert.equal(live.asm.map(([, hex]) => hex).join(''), originalBody.toString('hex'));
assert.equal(originalBody.toString('hex'), live.assembledBytes);

const object = fs.readFileSync(path.join(run, 'api-factory.obj'));
const coff = parseNativeCoff(object);
function body(name) {
    const definition = coff.unique(name);
    assert(definition.section > 0);
    assert.equal(definition.value, 0);
    const section = coff.sections[definition.section - 1];
    assert(section.flags & 0x20, 'Expected a code section');
    assert.deepEqual(section.relocations, []);
    return object.subarray(section.raw, section.raw + section.size);
}
const rebuilt = body(symbol);
const generatedSymbol = '_Api_2013bdcc@8';
const generated = body(generatedSymbol);
const generatedDifferentOffsets = Array.from({length: Math.max(length, generated.length)}, (_, offset) => offset)
    .filter(offset => originalBody[offset] !== generated[offset]);
const result = {
    status: rebuilt.equals(originalBody) ? 'Exact' : 'Different',
    scope: 'Complete x86 COFF function body compared with the original PE export. Linked DLL and runtime unverified.',
    entry: live.entry_point, bodyEnd: live.body_end, ordinal, symbol, run,
    originalSHA256, objectSHA256: sha(object), buildCommandSHA256: sha(fs.readFileSync(path.join(run, 'build.cmd'))),
    originalBytes: originalBody.length, rebuiltBytes: rebuilt.length, relocations: 0,
    originalHex: originalBody.toString('hex'), rebuiltHex: rebuilt.toString('hex'), evidence,
    generatedControl: {symbol: generatedSymbol, hex: generated.toString('hex'), differingOffsets: generatedDifferentOffsets}
};
fs.writeFileSync(path.join(run, 'callback-array-record-comparison.json'), JSON.stringify(result, null, 2) + '\n');
assert.deepEqual(rebuilt, originalBody);
console.log(`${symbol}: ${length} exact bytes, zero relocations, original PE ordinal ${ordinal}`);
