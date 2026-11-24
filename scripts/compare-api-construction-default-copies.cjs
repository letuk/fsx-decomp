const assert = require('node:assert/strict');
const { createHash } = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { openImage } = require('./lib/pe32-dll.cjs');

const root = path.resolve(__dirname, '..');
const run = process.argv[2];
assert(run, 'Pass the retained API construction-default object run');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const evidence = 'artifacts/api.dll/construction-default-copies-20261124/ghidra-construction-default-copies.json';
const live = JSON.parse(fs.readFileSync(path.join(root, evidence), 'utf8'));
assert.equal(live.program, 'api.dll');
const pe = fs.readFileSync(path.join(root, 'original/api.dll'));
const originalSHA256 = sha(pe);
assert.equal(originalSHA256, '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const image = openImage(pe);
const object = fs.readFileSync(path.join(run, 'api-construction-default-copies.obj'));
const coff = parseNativeCoff(object);
const field = 21;
const codeStart = 0x20161267;
const dataStart = 0x201fb138;
const wordCounts = [7, 5, 5, 6, 8, 5, 8, 5];
const recordRanges = [];
let dataAddress = dataStart;
assert.equal(live.functions.length, wordCounts.length);

const rows = wordCounts.map((words, index) => {
    const address = codeStart + 30 * index;
    const entry = address.toString(16);
    const fn = live.functions[index];
    assert.equal(fn.entry_point, entry);
    assert.equal(fn.size, 30);
    assert.equal(parseInt(fn.body_end, 16), address + 29);
    const original = image.codeAt(address, 30);
    let next = address;
    for (const [va, hex] of fn.asm) {
        assert.equal(parseInt(va, 16), next);
        next += Buffer.from(hex, 'hex').length;
    }
    assert.equal(next, address + original.length);
    assert.equal(fn.asm.map(([, hex]) => hex).join(''), original.toString('hex'));
    assert.deepEqual(image.highLowFixups(address, original.length), [field]);
    assert.equal(original[field - 1], 0xbe);
    assert.equal(original.readUInt32LE(field), dataAddress);
    assert.equal(original[15], 0x6a);
    assert.equal(original[16], words);
    assert.equal(original.readUInt16LE(25), 0xa5f3);

    const dataEntry = dataAddress.toString(16);
    const dataSymbol = '_g_constructionDefaults_' + dataEntry;
    const bytes = words * 4;
    const data = coff.unique(dataSymbol);
    assert(data.section > 0);
    const dataSection = coff.sections[data.section - 1];
    assert.equal(dataSection.name, '.bss');
    assert.equal(dataSection.raw, 0);
    assert(dataSection.flags & 0x80, 'Expected uninitialized data');
    assert.equal(data.value % 4, 0);
    assert(data.value + bytes <= dataSection.size);
    assert(!recordRanges.some(r => r.section === data.section &&
        data.value < r.end && r.start < data.value + bytes), 'Default records must be distinct');
    recordRanges.push({ section: data.section, start: data.value, end: data.value + bytes });
    const originalData = image.codeAt(dataAddress, bytes, false);
    assert.deepEqual(originalData, Buffer.alloc(bytes));

    const symbol = '@CopyConstructionDefaults_' + entry + '@4';
    const def = coff.unique(symbol);
    assert(def.section > 0);
    assert.equal(def.value, 0);
    const section = coff.sections[def.section - 1];
    assert(section.flags & 0x20);
    assert.equal(section.size, original.length);
    assert.equal(section.relocations.length, 1);
    const [reloc] = section.relocations;
    assert.equal(reloc.offset, field);
    assert.equal(reloc.type, 0x06, 'Expected x86 DIR32');
    assert.equal(reloc.symbol.index, data.index);
    const rebuilt = object.subarray(section.raw, section.raw + section.size);
    assert.equal(rebuilt[field - 1], 0xbe);
    const addend = rebuilt.readUInt32LE(field);
    assert.equal(addend, 0);
    const normalized = Buffer.from(rebuilt);
    normalized.writeUInt32LE(dataAddress + addend, field);
    assert.deepEqual(normalized, original);
    dataAddress += bytes;
    return {
        entry, symbol, status: 'Normalized', bytes: original.length,
        originalHex: original.toString('hex'), rebuiltHex: rebuilt.toString('hex'),
        normalizedHex: normalized.toString('hex'),
        relocation: { offset: field, kind: 'DIR32', symbol: dataSymbol, target: dataEntry, addend },
        data: { entry: dataEntry, words, bytes, originalHex: originalData.toString('hex'), sectionOffset: data.value }
    };
});

assert.equal(live.range.address, codeStart.toString(16));
assert.equal(rows.map(row => row.originalHex).join(''), live.range.hex);
assert.equal(live.data.address, dataStart.toString(16));
assert.equal(rows.map(row => row.data.originalHex).join(''), live.data.hex);
const result = {
    run, evidence, originalSHA256, objectSHA256: sha(object),
    buildCommandSHA256: sha(fs.readFileSync(path.join(run, 'build.cmd'))),
    scope: 'Complete native x86 COFF copy routine bodies with individually validated DIR32 references. The cited data spans are initial zero-filled records in both the original PE and the native object model. Linked placement and runtime remain open.',
    totalBytes: rows.reduce((total, row) => total + row.bytes, 0),
    defaultDataBytes: dataAddress - dataStart,
    rows
};
fs.writeFileSync(path.join(run, 'construction-default-copies-comparison.json'), JSON.stringify(result, null, 2) + '\n');
console.log(`${rows.length} normalized copy bodies, ${result.totalBytes} code bytes and ${result.defaultDataBytes} initial data bytes`);
