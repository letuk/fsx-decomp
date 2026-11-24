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

const object = fs.readFileSync(run + '/api-factory.obj');
const coff = parseNativeCoff(object);
const exportDirectory = originalBytes(base + pe.readUInt32LE(header + 120), 40);
const ordinalBase = exportDirectory.readUInt32LE(16);
const addresses = base + exportDirectory.readUInt32LE(28);
const imports = readPE32Imports(pe);
const global = '?g_panelMemoryDcPilot@@3PAUHDC__@@A';
const dc = offset => ({offset, type: 6, name: global, target: 0x2020d134});
const gdi = (offset, target, api, stack) => {
    assert.deepEqual(imports.get(target), {dll: 'gdi32.dll', name: api});
    return {offset, type: 6, name: '__imp__' + api + '@' + stack, target};
};
const bodies = [
    {entry: 0x2011d05c, ordinal: 182, size: 88, symbol: '_AcquirePanelMemoryDcPilot@8',
        fields: [dc(36), gdi(50, 0x200e1be0, 'SaveDC', 4), dc(58),
            gdi(66, 0x200e1bac, 'SelectObject', 8), dc(75)]},
    {entry: 0x2011d0b4, ordinal: 183, size: 56, alreadyAccepted: true, symbol: '_ReleasePanelMemoryDcPilot@8',
        fields: [dc(26), gdi(39, 0x200e1bb0, 'RestoreDC', 8)]}
];
const generated = JSON.parse(fs.readFileSync('scripts/matching/api-source-pass-accepted.json'));
for (const {entry, ordinal, size, alreadyAccepted, symbol, fields} of bodies) {
    assert.equal(base + originalBytes(addresses + 4 * (ordinal - ordinalBase), 4).readUInt32LE(), entry);
    const definition = coff.unique(symbol);
    const section = coff.sections[definition.section - 1];
    assert.equal(definition.value, 0);
    assert.equal(section.size, size);
    assert(section.flags & 0x20);
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
    changed[0] ^= 1;
    assert.throws(() => verify(changed, relocations));
    const wrongImport = relocations.map((relocation, index) => index === 1 ?
        {...relocation, name: global} : relocation);
    assert.throws(() => verify(candidate, wrongImport));
    const overlaps = generated.functions.filter(row => {
        const start = parseInt(row.entry, 16);
        return start < entry + size && start + row.bytes > entry;
    });
    if (alreadyAccepted) {
        assert.equal(overlaps.length, 1);
        assert.equal(parseInt(overlaps[0].entry, 16), entry);
        assert.equal(overlaps[0].bytes, size);
    } else {
        assert.equal(overlaps.length, 0, 'Generated body overlaps');
    }
}

const result = {
    objectSHA256: sha(object),
    completeCodeBytes: bodies.reduce((sum, body) => sum + (body.alreadyAccepted ? 0 : body.size), 0),
    metadataBytes: 0, bodies, negativeControls: ['changed entry opcode', 'incorrect GDI import'],
    scope: 'Whole DC wrappers with verified symbolic targets. DC creation and linked placement are separate.'
};
fs.writeFileSync(run + '/panel-memory-dc-comparison.json', JSON.stringify(result, null, 2) + '\n');
console.log(JSON.stringify(result));
