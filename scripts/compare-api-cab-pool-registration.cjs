const fs = require('fs');
const assert = require('assert');
const crypto = require('crypto');
const {parseNativeCoff} = require('./lib/coff.cjs');
const {readPE32Imports} = require('./lib/pe-imports.cjs');

const run = process.argv[2];
assert(run, 'Pass a retained API run directory');
const sha = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const original = fs.readFileSync('original/api.dll');
assert.equal(sha(original), '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const object = fs.readFileSync(run + '/api-factory.obj');
const coff = parseNativeCoff(object);
const pe = original.readUInt32LE(60);
const optional = pe + 24;
const base = original.readUInt32LE(optional + 28);
const table = optional + original.readUInt16LE(pe + 20);
const sections = [];
for (let i = 0; i < original.readUInt16LE(pe + 6); i++) {
    const s = table + i * 40;
    sections.push({va:base + original.readUInt32LE(s + 12), size:original.readUInt32LE(s + 16), raw:original.readUInt32LE(s + 20)});
}
function originalBytes(va, size) {
    const section = sections.find(s => va >= s.va && va + size <= s.va + s.size);
    assert(section, 'Unmapped original span');
    const offset = section.raw + va - section.va;
    return original.subarray(offset, offset + size);
}

const bodies = [
    [0x201128d7, 31, '@CountCabPoolUsedPilot@4'],
    [0x20124bf6, 7, '@CountCabFilePoolCapacityPilot@4'],
    [0x20127cf7, 7, '@CountCabWrapperPoolCapacityPilot@4'],
    [0x20112953, 4, '@CountCabPoolBlocksPilot@4'],
    [0x20124bfd, 4, '@CountCabPoolPeakBlocksPilot@4'],
    [0x200f48b0, 6, '_CountCabFilePoolSlotsPerBlockPilot'],
    [0x20127cfe, 6, '_CountCabWrapperPoolSlotsPerBlockPilot'],
    [0x20124c01, 10, '@CountCabFilePoolBytesPilot@4'],
    [0x20127d04, 10, '@CountCabWrapperPoolBytesPilot@4'],
    [0x20124c0b, 6, '_CabFilePoolRecordBytesPilot'],
    [0x20127d0e, 6, '_CabWrapperPoolRecordBytesPilot']
];
const generated = JSON.parse(fs.readFileSync('scripts/matching/api-source-pass-accepted.json'));
const functions = bodies.map(([entry, size, name]) => {
    const symbol = coff.unique(name);
    const section = coff.sections[symbol.section - 1];
    assert.equal(symbol.value, 0);
    assert.equal(section.size, size);
    assert.equal(section.relocations.length, 0);
    const actual = object.subarray(section.raw, section.raw + size);
    const expected = originalBytes(entry, size);
    assert(actual.equals(expected), name);
    for (const offset of [0, size - 1]) {
        const bad = Buffer.from(actual);
        bad[offset] ^= 1;
        assert(!bad.equals(expected));
    }
    for (const row of generated.functions) {
        const start = parseInt(row.entry, 16);
        assert(start + row.bytes <= entry || start >= entry + size, 'Generated body overlaps');
    }
    return {entry:entry.toString(16), bytes:size, symbol:name, rawEqual:true};
});
const targets = new Map(bodies.map(([entry, , name]) => [entry, name]));
for (const [entry, name] of [
    [0x200f492a, '@DeleteCabPoolRegistryNodePilot@12'],
    [0x20148c31, '__purecall'],
    [0x20124c75, '@DeleteCabFilePoolOwnerPilot@12'],
    [0x201247c3, '@ResetCabFilePoolPilot@4'],
    [0x20124845, '@TrimCabFilePoolPilot@4'],
    [0x20124ba0, '@CountCabFilePoolAvailablePilot@4'],
    [0x20127da5, '@DeleteCabWrapperPoolOwnerPilot@12'],
    [0x201278c4, '@ResetCabWrapperPoolPilot@4'],
    [0x20127946, '@TrimCabWrapperPoolPilot@4'],
    [0x20127ca1, '@CountCabWrapperPoolAvailablePilot@4']
]) targets.set(entry, name);

const purecall = originalBytes(0x20148c31, 6);
assert.equal(purecall.readUInt16LE(0), 0x25ff);
assert.deepEqual(readPE32Imports(original).get(purecall.readUInt32LE(2)), {dll:'msvcr80.dll', name:'_purecall'});

const tables = [
    [0x200f0528, '?g_cabPoolRegistryVTablePilot@@3QBQAXB', [0x200f492a, ...Array(10).fill(0x20148c31)]],
    [0x200f2cb0, '?g_cabFilePoolOwnerVTablePilot@@3QBQAXB', [0x20124c75,0x201247c3,0x20124845,0x201128d7,0x20124ba0,0x20124bf6,0x20112953,0x20124bfd,0x200f48b0,0x20124c01,0x20124c0b]],
    [0x200f2d90, '?g_cabWrapperPoolOwnerVTablePilot@@3QBQAXB', [0x20127da5,0x201278c4,0x20127946,0x201128d7,0x20127ca1,0x20127cf7,0x20112953,0x20124bfd,0x20127cfe,0x20127d04,0x20127d0e]]
];
const metadataRanges = [...tables.map(([entry, , slots]) => [entry, slots.length * 4]),
    [0x200f2b24, 16], [0x201f0808, 28]];
for (let i = 0; i < metadataRanges.length; i++) {
    const [start, size] = metadataRanges[i];
    for (const [other, otherSize] of metadataRanges.slice(i + 1)) {
        assert(start + size <= other || other + otherSize <= start, 'Metadata coverage overlaps');
    }
}
const metadata = tables.map(([entry, name, slots]) => {
    const symbol = coff.unique(name);
    const section = coff.sections[symbol.section - 1];
    const size = slots.length * 4;
    assert.equal(section.name, '.rdata');
    assert(symbol.value + size <= section.size);
    const actual = Buffer.from(object.subarray(section.raw + symbol.value, section.raw + symbol.value + size));
    const expected = originalBytes(entry, size);
    const relocations = section.relocations.filter(r => r.offset >= symbol.value && r.offset < symbol.value + size);
    assert.equal(relocations.length, slots.length);
    const fields = slots.map((target, index) => {
        const offset = index * 4;
        const relocation = relocations.find(r => r.offset === symbol.value + offset);
        assert(relocation, 'Missing slot relocation');
        assert.equal(relocation.type, 6);
        assert.equal(relocation.symbol.name, targets.get(target));
        assert.equal(actual.readUInt32LE(offset), 0);
        assert.equal(expected.readUInt32LE(offset), target);
        actual.writeUInt32LE(target, offset);
        return {offset, target:target.toString(16), symbol:relocation.symbol.name};
    });
    assert(actual.equals(expected));
    const wrongSlot = Buffer.from(actual);
    wrongSlot.writeUInt32LE(slots[1], 0);
    assert(!wrongSlot.equals(expected));
    return {entry:entry.toString(16), bytes:size, fields};
});
const result = {
    objectSHA256:sha(object), functions, metadata,
    completeCodeBytes:functions.reduce((sum, row) => sum + row.bytes, 0),
    metadataBytes:metadata.reduce((sum, row) => sum + row.bytes, 0),
    negativeControls:['first opcode', 'final return opcode', 'wrong VTable slot'],
    scope:'Raw full statistic methods and target-validated VTable spans. Shared targets counted once. Base table is an observed prefix. Linked placement and runtime remain open.'
};
fs.writeFileSync(run + '/cab-pool-registration-comparison.json', JSON.stringify(result, null, 2) + '\n');
console.log(JSON.stringify({completeCodeBytes:result.completeCodeBytes, metadataBytes:result.metadataBytes, objectSHA256:result.objectSHA256}));
