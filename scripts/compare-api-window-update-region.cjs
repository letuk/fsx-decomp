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

const entry = 0x2011d315;
const size = 106;
const exportDirectory = originalBytes(base + pe.readUInt32LE(header + 120), 40);
const ordinalBase = exportDirectory.readUInt32LE(16);
const addresses = base + exportDirectory.readUInt32LE(28);
assert.equal(base + originalBytes(addresses + 4 * (28 - ordinalBase), 4).readUInt32LE(), entry);
const imports = readPE32Imports(pe);
for (const [address, name] of [[0x200e1c1c, 'CreateRectRgn'], [0x200e1bb4, 'SetRectRgn'], [0x200e1bb8, 'CombineRgn']]) {
    assert.deepEqual(imports.get(address), {dll: 'gdi32.dll', name});
}

const object = fs.readFileSync(run + '/api-factory.obj');
const coff = parseNativeCoff(object);
const definition = coff.unique('_AddClipWindowUpdateRectanglePilot@8');
const section = coff.sections[definition.section - 1];
assert.equal(definition.value, 0);
assert.equal(section.size, size);
assert(section.flags & 0x20);
const global = '?g_sharedClipRegionFirstPilot@@3PAUHRGN__@@A';
const storage = coff.unique(global);
const storageSection = coff.sections[storage.section - 1];
assert(storageSection.flags & 0x80);
assert(storage.value + 4 <= storageSection.size);
const fields = [
    {offset: 33, type: 6, name: global, target: 0x2020de40},
    {offset: 47, type: 6, name: '__imp__CreateRectRgn@16', target: 0x200e1c1c},
    {offset: 52, type: 6, name: global, target: 0x2020de40},
    {offset: 77, type: 6, name: '__imp__SetRectRgn@20', target: 0x200e1bb4},
    {offset: 83, type: 6, name: global, target: 0x2020de40},
    {offset: 97, type: 6, name: '__imp__CombineRgn@16', target: 0x200e1bb8}
];
const expected = originalBytes(entry, size);
const candidate = object.subarray(section.raw, section.raw + size);
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
changed[16] ^= 1;
assert.throws(() => verify(changed, relocations));
assert.throws(() => verify(candidate, [...relocations.slice(0, -1), {...relocations[5], name: global}]));
const generated = JSON.parse(fs.readFileSync('scripts/matching/api-source-pass-accepted.json'));
for (const row of generated.functions) {
    const start = parseInt(row.entry, 16);
    assert(start + row.bytes <= entry || start >= entry + size, 'Generated body overlaps');
}

const result = {
    objectSHA256: sha(object), entry: entry.toString(16), ordinal: 28,
    completeCodeBytes: size, metadataBytes: 0, fields,
    negativeControls: ['changed window flag mask', 'incorrect region import'],
    scope: 'Complete update-region body with verified symbolic targets. Linked placement is separate.'
};
fs.writeFileSync(run + '/window-update-region-comparison.json', JSON.stringify(result, null, 2) + '\n');
console.log(JSON.stringify(result));
