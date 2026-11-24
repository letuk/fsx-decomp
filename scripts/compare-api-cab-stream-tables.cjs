const fs = require('fs');
const assert = require('assert');
const crypto = require('crypto');
const {parseNativeCoff} = require('./lib/coff.cjs');
const {readPE32Imports} = require('./lib/pe-imports.cjs');

const run = process.argv[2];
assert(run);
const original = fs.readFileSync('original/api.dll');
const sha = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
assert.equal(sha(original), '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const object = fs.readFileSync(run + '/api-factory.obj');
const coff = parseNativeCoff(object);
const pe = original.readUInt32LE(60), opt = pe + 24;
const base = original.readUInt32LE(opt + 28), table = opt + original.readUInt16LE(pe + 20);
function bytes(va, size) {
    for (let i = 0; i < original.readUInt16LE(pe + 6); i++) {
        const s = table + i * 40;
        const offset = va - base - original.readUInt32LE(s + 12);
        if (offset >= 0 && offset + size <= original.readUInt32LE(s + 16)) {
            const start = original.readUInt32LE(s + 20) + offset;
            return original.subarray(start, start + size);
        }
    }
    throw Error('Unmapped original span');
}
const methods = [
    [0x2012438f,57,'_WriteCabFileMethodPilot@16'],
    [0x2012707c,8,'_ResizeCabWrappedPilot@12'],
    [0x2012a682,5,'_CommitCabStreamPilot@8'],
    [0x201284f3,8,'_RevertCabStreamPilot@4'],
    [0x201245cc,8,'_RegionCabStreamPilot@24']
];
const generated = JSON.parse(fs.readFileSync('scripts/matching/api-source-pass-accepted.json'));
const functions = methods.map(([entry, size, name]) => {
    const s = coff.unique(name), sec = coff.sections[s.section - 1];
    assert.equal(s.value, 0);
    assert.equal(sec.size, size);
    const actual = Buffer.from(object.subarray(sec.raw, sec.raw + size));
    const expected = bytes(entry, size);
    assert.equal(sec.relocations.length, entry === 0x2012438f ? 1 : 0);
    if (sec.relocations.length) {
        const r = sec.relocations[0];
        assert.equal(r.offset, 35);
        assert.equal(r.type, 6);
        assert.equal(r.symbol.name, '__imp__WriteFile@20');
        assert.equal(actual.readUInt32LE(35), 0);
        assert.equal(expected.readUInt32LE(35), 0x200e190c);
        assert.deepEqual(readPE32Imports(original).get(0x200e190c), {dll:'kernel32.dll',name:'WriteFile'});
        actual.writeUInt32LE(0x200e190c, 35);
    }
    assert(actual.equals(expected), name);
    for (const offset of [0, size - 2, ...(sec.relocations.length ? [35] : [])]) {
        const bad = Buffer.from(actual);
        bad[offset] ^= 1;
        assert(!bad.equals(expected));
    }
    for (const f of generated.functions) {
        const start = parseInt(f.entry, 16);
        assert(start + f.bytes <= entry || start >= entry + size, 'Generated code overlap');
    }
    return {entry:entry.toString(16), bytes:size, symbol:name, rawEqual:sec.relocations.length === 0};
});

const shared = [[0x2012451d,'_CopyCabStreamPilot@24'],[0x2012a682,'_CommitCabStreamPilot@8'],
    [0x201284f3,'_RevertCabStreamPilot@4'],[0x201245cc,'_RegionCabStreamPilot@24'],[0x201245cc,'_RegionCabStreamPilot@24']];
const tables = [
    [0x200f2c70, '?g_cabFileStreamVTablePilot@@3UCabStreamVTableObservedPilot@@B', [
        [0x201242bc,'_QueryCabFileStreamPilot@12'],[0x200f40da,'_AddCabStreamReferencePilot@4'],
        [0x2012431e,'_ReleaseCabFileMethodPilot@4'],[0x20124356,'_ReadCabFileMethodPilot@16'],
        [0x2012438f,'_WriteCabFileMethodPilot@16'],[0x201243c8,'_SeekCabFileMethodPilot@20'],
        [0x201244d2,'_ResizeCabFilePilot@12'],...shared,
        [0x20124408,'_StatCabFileMethodPilot@12'],[0x20124549,'_CloneCabFileStreamPilot@8']]],
    [0x200f2d50, '?g_cabWrappedStreamVTablePilot@@3UCabStreamVTableObservedPilot@@B', [
        [0x20126cc3,'_QueryCabWrappedStreamPilot@12'],[0x200f40da,'_AddCabStreamReferencePilot@4'],
        [0x20126d5b,'_ReleaseCabWrappedMethodPilot@4'],[0x20126dcf,'_ReadCabWrappedMethodPilot@16'],
        [0x20126f2d,'_WriteCabWrappedMethodPilot@16'],[0x20126fc0,'_SeekCabWrappedMethodPilot@20'],
        [0x2012707c,'_ResizeCabWrappedPilot@12'],...shared,
        [0x2012701c,'_StatCabWrappedMethodPilot@12'],[0x201271d7,'_CloneCabWrappedStreamPilot@8']]]
];
const metadata = tables.map(([entry, name, slots]) => {
    const s = coff.unique(name), sec = coff.sections[s.section - 1], size = 56;
    assert.equal(sec.name, '.rdata');
    assert.equal(slots.length * 4, size);
    const actual = Buffer.from(object.subarray(sec.raw + s.value, sec.raw + s.value + size));
    const expected = bytes(entry, size);
    const relocations = sec.relocations.filter(r => r.offset >= s.value && r.offset < s.value + size);
    assert.equal(relocations.length, slots.length);
    const fields = slots.map(([target, targetName], index) => {
        const offset = index * 4;
        const r = relocations.find(r => r.offset === s.value + offset);
        assert(r);
        assert.equal(r.type, 6);
        assert.equal(r.symbol.name, targetName);
        assert.equal(actual.readUInt32LE(offset), 0);
        assert.equal(expected.readUInt32LE(offset), target);
        actual.writeUInt32LE(target, offset);
        return {offset,target:target.toString(16),symbol:targetName};
    });
    assert(actual.equals(expected));
    actual.writeUInt32LE(slots[1][0], 0);
    assert(!actual.equals(expected));
    for (const [start, length] of [[0x200f0528,44],[0x200f2b24,16],[0x200f2cb0,44],[0x200f2d90,44],[0x201f0808,28]])
        assert(entry + size <= start || entry >= start + length, 'Metadata overlap');
    return {entry:entry.toString(16),bytes:size,fields};
});
const result = {
    objectSHA256:sha(object), functions, metadata,
    completeCodeBytes:functions.reduce((n,f) => n + f.bytes, 0),
    metadataBytes:metadata.reduce((n,f) => n + f.bytes, 0),
    scope:'Five full methods and 28 target-validated table fields. Shared AddRef was accepted earlier. Linked placement and remaining bodies are separate.'
};
fs.writeFileSync(run + '/cab-stream-tables-comparison.json', JSON.stringify(result, null, 2) + '\n');
console.log(JSON.stringify({completeCodeBytes:result.completeCodeBytes, metadataBytes:result.metadataBytes, objectSHA256:result.objectSHA256}));
