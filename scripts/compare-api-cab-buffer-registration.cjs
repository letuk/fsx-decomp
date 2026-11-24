const fs = require('fs');
const assert = require('assert');
const crypto = require('crypto');
const {parseNativeCoff} = require('./lib/coff.cjs');

const run = process.argv[2];
assert(run, 'Pass a retained API run directory');
const sha = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const original = fs.readFileSync('original/api.dll');
assert.equal(sha(original), '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const entry = 0x201ee724;
const size = 17;
const pe = original.readUInt32LE(60);
const base = original.readUInt32LE(pe + 52);
const firstSection = pe + 24 + original.readUInt16LE(pe + 20);
const sectionHeaders = Array.from({length: original.readUInt16LE(pe + 6)}, (_, i) => firstSection + i * 40);
const header = sectionHeaders.find(at => {
    const va = base + original.readUInt32LE(at + 12);
    return va <= entry && entry + size <= va + original.readUInt32LE(at + 16);
});
assert(header !== undefined);
const offset = original.readUInt32LE(header + 20) + entry - base - original.readUInt32LE(header + 12);
const expected = original.subarray(offset, offset + size);

const object = fs.readFileSync(run + '/api-factory.obj');
const coff = parseNativeCoff(object);
const symbol = coff.unique('_RegisterCabBufferCachePilot');
const section = coff.sections[symbol.section - 1];
assert.equal(symbol.value, 0);
assert.equal(section.size, size);
const candidate = object.subarray(section.raw, section.raw + size);
const fields = [
    {offset: 1, type: 20, target: 0x201274a6, name: '_InitializeCabBufferCachePilot'},
    {offset: 6, type: 6, target: 0x201eeeed, name: '?CleanUpCabBufferCache@@YAXXZ'},
    {offset: 11, type: 20, target: 0x20148c1f, name: '_atexit'}
];
const expectedRelocations = fields.map(field => ({offset: field.offset, type: field.type, name: field.name}));
const relocations = section.relocations.map(r => ({offset: r.offset, type: r.type, name: r.symbol.name}));
for (const field of fields.slice(0, 2)) assert(coff.unique(field.name).section > 0);

function verify(body, actualRelocations) {
    assert.deepEqual(actualRelocations, expectedRelocations);
    const normalized = Buffer.from(body);
    for (const field of fields) {
        assert.equal(normalized.readUInt32LE(field.offset), 0);
        if (field.type === 20) {
            const nextInstruction = entry + field.offset + 4;
            assert.equal(nextInstruction + expected.readInt32LE(field.offset), field.target);
            normalized.writeInt32LE(field.target - nextInstruction, field.offset);
        } else {
            assert.equal(expected.readUInt32LE(field.offset), field.target);
            normalized.writeUInt32LE(field.target, field.offset);
        }
    }
    assert(normalized.equals(expected));
}
verify(candidate, relocations);
for (const at of [0, size - 1]) {
    const changed = Buffer.from(candidate);
    changed[at] ^= 1;
    assert.throws(() => verify(changed, relocations));
}
const wrongCallback = relocations.map(r => r.offset === 6 ? {...r, name: '_InitializeCabBufferCachePilot'} : r);
assert.throws(() => verify(candidate, wrongCallback));

const generated = JSON.parse(fs.readFileSync('scripts/matching/api-source-pass-accepted.json'));
for (const row of generated.functions) {
    const start = parseInt(row.entry, 16);
    assert(start + row.bytes <= entry || start >= entry + size, 'Generated body overlaps');
}
const result = {
    objectSHA256: sha(object), entry: entry.toString(16), completeCodeBytes: size,
    metadataBytes: 0, fields,
    negativeControls: ['first opcode', 'return opcode', 'wrong callback relocation'],
    scope: 'Registration body with symbolic targets. Target bodies and linked CRT placement are separate.'
};
fs.writeFileSync(run + '/cab-buffer-registration-comparison.json', JSON.stringify(result, null, 2) + '\n');
console.log(JSON.stringify(result));
