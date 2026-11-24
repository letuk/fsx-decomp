const fs = require('fs');
const assert = require('node:assert/strict');
const crypto = require('crypto');
const {parseNativeCoff} = require('./lib/coff.cjs');
const {readPE32Imports} = require('./lib/pe-imports.cjs');

const run = process.argv[2];
assert(run, 'Pass a retained API run directory');
const sha = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const pe = fs.readFileSync('original/api.dll');
assert.equal(sha(pe), '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const header = pe.readUInt32LE(60);
const base = pe.readUInt32LE(header + 52);
const firstSection = header + 24 + pe.readUInt16LE(header + 20);
const sections = Array.from({length: pe.readUInt16LE(header + 6)}, (_, i) => firstSection + 40 * i);
function originalBytes(address, size) {
    const section = sections.find(at => {
        const start = base + pe.readUInt32LE(at + 12);
        return start <= address && address + size <= start + pe.readUInt32LE(at + 16);
    });
    assert(section !== undefined);
    const offset = pe.readUInt32LE(section + 20) + address - base - pe.readUInt32LE(section + 12);
    return pe.subarray(offset, offset + size);
}

const entry = 0x20135f51;
const size = 43;
const exportDirectory = originalBytes(base + pe.readUInt32LE(header + 120), 40);
const ordinalBase = exportDirectory.readUInt32LE(16);
const addresses = base + exportDirectory.readUInt32LE(28);
assert.equal(base + originalBytes(addresses + 4 * (108 - ordinalBase), 4).readUInt32LE(), entry);
assert.deepEqual(readPE32Imports(pe).get(0x200e19a0), {dll: 'kernel32.dll', name: 'HeapFree'});

const object = fs.readFileSync(run + '/api-factory.obj');
const coff = parseNativeCoff(object);
const definition = coff.unique('_FreePrivateHeapSlotPilot@4');
const section = coff.sections[definition.section - 1];
assert.equal(definition.value, 0);
assert.equal(section.size, size);
assert(section.flags & 0x20);
const expected = originalBytes(entry, size);
const candidate = object.subarray(section.raw, section.raw + size);
const fields = [
    {offset: 17, type: 6, name: '?g_privateHeapPilot@@3PAXA', target: 0x2020e050},
    {offset: 26, type: 6, name: '__imp__HeapFree@12', target: 0x200e19a0}
];
const relocations = section.relocations.map(r => ({offset: r.offset, type: r.type, name: r.symbol.name}));
const expectedRelocations = fields.map(({target, ...field}) => field);
function verify(body, actualRelocations) {
    assert.deepEqual(actualRelocations, expectedRelocations);
    const normalized = Buffer.from(body);
    for (const field of fields) {
        assert.equal(normalized.readUInt32LE(field.offset), 0);
        assert.equal(expected.readUInt32LE(field.offset), field.target);
        normalized.writeUInt32LE(field.target, field.offset);
    }
    assert.deepEqual(normalized, expected);
}
verify(candidate, relocations);
const changed = Buffer.from(candidate);
changed[30] ^= 1; // Null store after HeapFree.
assert.throws(() => verify(changed, relocations));
assert.throws(() => verify(candidate, [relocations[0], {...relocations[1], name: '__imp__HeapAlloc@12'}]));

const generated = JSON.parse(fs.readFileSync('scripts/matching/api-source-pass-accepted.json'));
for (const row of generated.functions) {
    const start = parseInt(row.entry, 16);
    assert(start + row.bytes <= entry || start >= entry + size, 'Generated body overlaps');
}
const result = {
    objectSHA256: sha(object), entry: entry.toString(16), ordinal: 108,
    completeCodeBytes: size, metadataBytes: 0, fields,
    negativeControls: ['changed slot clear', 'wrong HeapFree import'],
    scope: 'Complete release wrapper with verified symbolic targets. Linked placement is separate.'
};
fs.writeFileSync(run + '/private-heap-release-comparison.json', JSON.stringify(result, null, 2) + '\n');
console.log(JSON.stringify(result));
