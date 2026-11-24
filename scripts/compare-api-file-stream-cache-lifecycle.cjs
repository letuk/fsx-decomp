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

const imports = readPE32Imports(pe);
assert.deepEqual(imports.get(0x200e1760), {dll: 'kernel32.dll', name: 'InitializeCriticalSectionAndSpinCount'});
assert.deepEqual(imports.get(0x200e1898), {dll: 'kernel32.dll', name: 'DeleteCriticalSection'});
assert.equal(originalBytes(0x200e1ec4, 4).readUInt32LE(), 0x201ee73a);

const object = fs.readFileSync(run + '/api-factory.obj');
const coff = parseNativeCoff(object);
const lock = '_g_fileStreamCacheLockPilot';
const cleanup = '_DeleteFileStreamCacheLockPilot';
const lockStorage = coff.unique(lock);
assert(lockStorage.section > 0);
const storageSection = coff.sections[lockStorage.section - 1];
assert(storageSection.flags & 0x80);
assert(lockStorage.value + 24 <= storageSection.size);
const bodies = [
    {entry: 0x201ee73a, size: 25, symbol: '_RegisterFileStreamCachePilot', fields: [
        {offset: 3, type: 6, name: lock, target: 0x2020e284},
        {offset: 9, type: 6, name: '__imp__InitializeCriticalSectionAndSpinCount@8', target: 0x200e1760},
        {offset: 14, type: 6, name: cleanup, target: 0x201eef1c},
        {offset: 19, type: 20, name: '_atexit', target: 0x20148c1f}
    ]},
    {entry: 0x201eef1c, size: 12, symbol: cleanup, fields: [
        {offset: 1, type: 6, name: lock, target: 0x2020e284},
        {offset: 7, type: 6, name: '__imp__DeleteCriticalSection@4', target: 0x200e1898}
    ]}
];
const generated = JSON.parse(fs.readFileSync('scripts/matching/api-source-pass-accepted.json'));
for (const {entry, size, symbol, fields} of bodies) {
    const expected = originalBytes(entry, size);
    const definition = coff.unique(symbol);
    const section = coff.sections[definition.section - 1];
    assert.equal(definition.value, 0);
    assert.equal(section.size, size);
    assert(section.flags & 0x20);
    const candidate = object.subarray(section.raw, section.raw + size);
    const relocations = section.relocations.map(r => ({offset: r.offset, type: r.type, name: r.symbol.name}));
    const expectedRelocations = fields.map(({target, ...field}) => field);
    function verify(body, actualRelocations) {
        assert.deepEqual(actualRelocations, expectedRelocations);
        const normalized = Buffer.from(body);
        for (const field of fields) {
            assert.equal(normalized.readUInt32LE(field.offset), 0);
            if (field.type === 20) {
                const next = entry + field.offset + 4;
                assert.equal(next + expected.readInt32LE(field.offset), field.target);
                normalized.writeInt32LE(field.target - next, field.offset);
            } else {
                assert.equal(expected.readUInt32LE(field.offset), field.target);
                normalized.writeUInt32LE(field.target, field.offset);
            }
        }
        assert.deepEqual(normalized, expected);
    }
    verify(candidate, relocations);
    const changed = Buffer.from(candidate);
    changed[size - 1] ^= 1;
    assert.throws(() => verify(changed, relocations));
    const wrongTarget = relocations.map((r, i) => i === 1 ? {...r, name: lock} : r);
    assert.throws(() => verify(candidate, wrongTarget));
    for (const row of generated.functions) {
        const start = parseInt(row.entry, 16);
        assert(start + row.bytes <= entry || start >= entry + size, 'Generated body overlaps');
    }
}

const result = {
    objectSHA256: sha(object), completeCodeBytes: bodies.reduce((sum, body) => sum + body.size, 0),
    metadataBytes: 0, bodies,
    negativeControls: ['return opcode', 'wrong import relocation'],
    scope: 'Whole object bodies with verified symbolic targets. Linked CRT placement remains separate.'
};
fs.writeFileSync(run + '/file-stream-cache-lifecycle-comparison.json', JSON.stringify(result, null, 2) + '\n');
console.log(JSON.stringify(result));
