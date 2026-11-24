const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const {createHash} = require('node:crypto');
const {parseNativeCoff} = require('./lib/coff.cjs');

const run = process.argv[2];
assert(run, 'Pass the retained API ABI probe run directory');
const sha256 = bytes => createHash('sha256').update(bytes).digest('hex');
const pe = fs.readFileSync(path.join(__dirname, '../original/api.dll'));
assert.equal(sha256(pe), '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
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

const ordinal = 207;
const entry = 0x2013241b;
const size = 38;
const formatAddress = 0x200f3100;
const formatterAddress = 0x200fb1ab;
const exportDirectory = originalBytes(base + pe.readUInt32LE(header + 120), 40);
const slot = ordinal - exportDirectory.readUInt32LE(16);
assert(slot >= 0 && slot < exportDirectory.readUInt32LE(20));
assert.equal(base + originalBytes(base + exportDirectory.readUInt32LE(28) + slot * 4, 4).readUInt32LE(), entry);

const object = fs.readFileSync(path.join(run, 'api-factory.obj'));
const coff = parseNativeCoff(object);
const symbol = '_FormatNumberedNameAbiProbe@16';
const body = coff.unique(symbol);
const code = coff.sections[body.section - 1];
assert.equal(body.value, 0);
assert.equal(code.size, size);
assert(code.flags & 0x20);
const format = coff.unique('_g_numberedNameProbeFormat');
const formatSection = coff.sections[format.section - 1];
const literal = Buffer.from('%s.%d\0', 'ascii');
assert.deepEqual(originalBytes(formatAddress, literal.length), literal);
assert(format.value + literal.length <= formatSection.size);
assert.deepEqual(object.subarray(formatSection.raw + format.value,
    formatSection.raw + format.value + literal.length), literal);
const formatter = coff.unique('_FormatBoundedTextRegisterBoundary');
assert.equal(formatter.section, 0);
assert.equal(formatter.value, 0);

const fields = [
    {offset: 21, type: 6, symbol: format.name, originalTarget: formatAddress},
    {offset: 26, type: 20, symbol: formatter.name, originalTarget: formatterAddress}
];
const original = originalBytes(entry, size);
const rebuilt = object.subarray(code.raw, code.raw + size);
const relocations = code.relocations.map(r => ({offset: r.offset, type: r.type, symbol: r.symbol.name}));
function verify(bytes, actualRelocations) {
    assert.deepEqual(actualRelocations, fields.map(({originalTarget, ...field}) => field));
    const normalized = Buffer.from(bytes);
    for (const field of fields) {
        const value = field.type === 6 ? field.originalTarget : field.originalTarget - (entry + field.offset + 4);
        assert.equal(original.readUInt32LE(field.offset), value >>> 0);
        assert.equal(normalized.readUInt32LE(field.offset), 0);
        normalized.writeUInt32LE(value >>> 0, field.offset);
    }
    assert.deepEqual(normalized, original);
}
verify(rebuilt, relocations);
const mutated = Buffer.from(rebuilt);
mutated[32] = 0x14;
assert.throws(() => verify(mutated, relocations));
assert.throws(() => verify(rebuilt, [relocations[0], {...relocations[1], symbol: '_FormatBoundedTextPilot'}]));

const report = {
    module: 'api.dll', entry: entry.toString(16), ordinal, symbol, bodyBytes: size,
    originalSHA256: sha256(pe), objectSHA256: sha256(object), fields,
    leftHex: original.toString('hex'), rightHex: rebuilt.toString('hex'),
    negativeControls: ['changed stack cleanup', 'ordinary C formatter relocation'],
    scope: 'Complete COFF probe body matches after two relocation mappings. The formatter symbol is unresolved and represents the original register ABI target.'
};
fs.writeFileSync(path.join(run, 'numbered-name-abi-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(JSON.stringify(report));
