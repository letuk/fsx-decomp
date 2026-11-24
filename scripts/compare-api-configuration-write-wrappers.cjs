const fs = require('fs');
const assert = require('node:assert/strict');
const crypto = require('crypto');
const {parseNativeCoff} = require('./lib/coff.cjs');

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
const target = 0x20130b6b;
const targetSymbol = '_WriteConfigurationIntPilot@12';
const definitions = [
    {entry: 0x20130cdf, ordinal: 201, symbol: '_WriteConfigurationWordPilot@12'},
    {entry: 0x20130cfa, ordinal: 203, symbol: '_WriteConfigurationBytePilot@12'}
];
const exportDirectory = originalBytes(base + pe.readUInt32LE(header + 120), 40);
const ordinalBase = exportDirectory.readUInt32LE(16);
const addresses = base + exportDirectory.readUInt32LE(28);
const size = 27;
const callOperand = 19;
assert(coff.unique(targetSymbol).section > 0);
assert.equal(base + originalBytes(addresses + 4 * (40 - ordinalBase), 4).readUInt32LE(), target);

const generated = JSON.parse(fs.readFileSync('scripts/matching/api-source-pass-accepted.json'));
for (const {entry, ordinal, symbol} of definitions) {
    assert.equal(base + originalBytes(addresses + 4 * (ordinal - ordinalBase), 4).readUInt32LE(), entry);
    const expected = originalBytes(entry, size);
    const definition = coff.unique(symbol);
    const section = coff.sections[definition.section - 1];
    assert.equal(definition.value, 0);
    assert.equal(section.size, size);
    assert(section.flags & 0x20);
    const candidate = object.subarray(section.raw, section.raw + size);
    const relocations = section.relocations.map(r => ({offset: r.offset, type: r.type, name: r.symbol.name}));
    function verify(body, actualRelocations) {
        assert.deepEqual(actualRelocations, [{offset: callOperand, type: 20, name: targetSymbol}]);
        assert.equal(body.readUInt32LE(callOperand), 0);
        const next = entry + callOperand + 4;
        assert.equal(next + expected.readInt32LE(callOperand), target);
        const normalized = Buffer.from(body);
        normalized.writeInt32LE(target - next, callOperand);
        assert.deepEqual(normalized, expected);
    }
    verify(candidate, relocations);
    const changed = Buffer.from(candidate);
    changed[6] ^= 1;
    assert.throws(() => verify(changed, relocations));
    assert.throws(() => verify(candidate, [{...relocations[0], name: symbol}]));
    for (const row of generated.functions) {
        const start = parseInt(row.entry, 16);
        assert(start + row.bytes <= entry || start >= entry + size, 'Generated body overlaps');
    }
}

const result = {
    objectSHA256: sha(object), completeCodeBytes: definitions.length * size, metadataBytes: 0,
    definitions, callOperand, target, targetSymbol,
    negativeControls: ['changed zero-extension opcode', 'wrong call relocation'],
    scope: 'Complete wrapper bodies with verified symbolic targets. The callee body is separate.'
};
fs.writeFileSync(run + '/configuration-write-wrappers-comparison.json', JSON.stringify(result, null, 2) + '\n');
console.log(JSON.stringify(result));
