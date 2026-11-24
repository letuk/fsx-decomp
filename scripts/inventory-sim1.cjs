const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { readPE32Imports } = require('./lib/pe-imports.cjs');
const { openImage } = require('./lib/pe32-dll.cjs');

const program = 'sim1.dll';
const repo = path.resolve(__dirname, '..');
const folder = path.join(repo, 'artifacts', program, 'sim1-complete-inventory');
const original = fs.readFileSync(path.join(repo, 'original', program));
const sha256 = createHash('sha256').update(original).digest('hex');
assert.equal(sha256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(original);
const hex = value => value.toString(16).padStart(8, '0');
const read = name => JSON.parse(fs.readFileSync(path.join(folder, name), 'utf8'));
const write = (name, value) =>
    fs.writeFileSync(path.join(folder, name), JSON.stringify(value, null, 2) + '\n');

const ghidra = read('ghidra-functions.json');
const listing = read('ghidra-boundary.json');
assert.equal(ghidra.program, program);
assert.equal(listing.program, program);
assert.equal(ghidra.imageBase, hex(image.base));
assert.equal(ghidra.modelFunctionCount, ghidra.functions.length + ghidra.externalFunctions.length);

const functions = new Map();
for (const item of ghidra.functions) {
    assert.match(item.entry, /^[0-9a-f]{8}$/);
    assert(!functions.has(item.entry), 'Duplicate Ghidra function ' + item.entry);
    const address = Number.parseInt(item.entry, 16);
    assert(item.bodyRanges.some(([start, end]) => item.entry >= start && item.entry <= end));
    let total = 0;
    let previousEnd = 0;
    for (const [start, end] of item.bodyRanges) {
        const from = Number.parseInt(start, 16);
        const to = Number.parseInt(end, 16);
        assert(from > previousEnd && to >= from);
        assert.equal(image.codeAt(from, to - from + 1).length, to - from + 1);
        total += to - from + 1;
        previousEnd = to;
    }
    assert.equal(item.bodyBytes, total, item.entry);
    assert.equal(item.bodyEnd, hex(previousEnd), item.entry);
    assert.equal(image.codeAt(address, 1).length, 1);
    functions.set(item.entry, item);
}

const externalAddresses = new Set(ghidra.externalFunctions.map(item => item.entry));
assert.equal(externalAddresses.size, ghidra.externalFunctions.length);
const imports = [...readPE32Imports(original)].map(([address, identity]) => ({ iatVA: hex(address), ...identity }));
const ghidraImports = listing.imports.items;
assert.equal(listing.imports.program, program);
assert.equal(imports.length, ghidraImports.length);
assert.equal(new Set(ghidraImports.map(item => item.address)).size, ghidraImports.length);
const importedAddresses = new Set(ghidraImports.map(item => item.address));
assert([...externalAddresses].every(address => importedAddresses.has(address)));
const importsWithoutFunction = ghidraImports.filter(item => !externalAddresses.has(item.address)).map(item => {
    const importName = (item.original_imported_name || item.name).replace(/^Ordinal_/, '#');
    const matches = imports.filter(record => record.name === importName);
    assert.equal(matches.length, 1, importName);
    return { externalAddress: item.address, ...matches[0] };
});
assert.equal(importsWithoutFunction.length, imports.length - externalAddresses.size);

const optionalHeader = original.readUInt32LE(0x3c) + 24;
const exportRva = original.readUInt32LE(optionalHeader + 96);
const exportSize = original.readUInt32LE(optionalHeader + 100);
const exportDirectory = image.codeAt(image.base + exportRva, 40, false);
const ordinalBase = exportDirectory.readUInt32LE(16);
const addressCount = exportDirectory.readUInt32LE(20);
const nameCount = exportDirectory.readUInt32LE(24);
const addressTable = image.codeAt(image.base + exportDirectory.readUInt32LE(28), 4 * addressCount, false);
const nameTable = image.codeAt(image.base + exportDirectory.readUInt32LE(32), 4 * nameCount, false);
const ordinalTable = image.codeAt(image.base + exportDirectory.readUInt32LE(36), 2 * nameCount, false);
const namesByIndex = new Map();
assert.equal(listing.exports.program, program);
const modelExportNames = new Map(listing.exports.items.map(item => [item.name, item.address]));
for (let index = 0; index < nameCount; index++) {
    const ordinalIndex = ordinalTable.readUInt16LE(2 * index);
    const nameVA = image.base + nameTable.readUInt32LE(index * 4);
    let name = '';
    for (let position = 0; position < 512; position++) {
        const byte = image.codeAt(nameVA + position, 1, false)[0];
        if (byte === 0) break;
        name += String.fromCharCode(byte);
    }
    assert(name && name.length < 512);
    assert(ordinalIndex < addressCount);
    assert.equal(modelExportNames.get(name), hex(image.base + addressTable.readUInt32LE(ordinalIndex * 4)));
    if (!namesByIndex.has(ordinalIndex)) namesByIndex.set(ordinalIndex, []);
    namesByIndex.get(ordinalIndex).push(name);
}

const exportsByAddress = new Map();
const peExports = [];
for (let index = 0; index < addressCount; index++) {
    const rva = addressTable.readUInt32LE(index * 4);
    if (!rva) continue;
    assert(rva < exportRva || rva >= exportRva + exportSize, 'Unexpected forwarded export');
    const entry = hex(image.base + rva);
    assert(functions.get(entry)?.externalEntryPoint, `Missing Ghidra export function ${entry}`);
    const exported = { ordinal: ordinalBase + index, entry, names: namesByIndex.get(index) || [] };
    peExports.push(exported);
    if (!exportsByAddress.has(entry)) exportsByAddress.set(entry, []);
    exportsByAddress.get(entry).push(exported);
}
const peEntry = hex(image.base + image.entryRva);
assert(functions.get(peEntry)?.externalEntryPoint);
assert(ghidra.functions.filter(item => item.externalEntryPoint)
    .every(item => item.entry === peEntry || exportsByAddress.has(item.entry)));

const manifest = {
    program,
    source: 'Read-only Ghidra MCP function metadata and byte-verified original PE entry points',
    originalSHA256: sha256,
    functions: ghidra.functions.map(item => {
        const summary = { entry: item.entry, name: item.name, bodyBytes: item.bodyBytes, signature: item.signature };
        if (item.tags.length) summary.tags = item.tags;
        if (item.isThunk) {
            summary.group = item.thunkTarget?.startsWith('EXTERNAL:') ? 'Import thunks' : 'Internal thunks';
            if (item.thunkTargetName) summary.description = 'Ghidra thunk target ' + item.thunkTargetName;
        }
        if (item.name.startsWith('Unwind@')) summary.group = 'Exception unwind';
        if (item.entry === peEntry) summary.group = 'DLL startup';
        if (exportsByAddress.has(item.entry)) {
            summary.group = 'PE exports';
            summary.export = exportsByAddress.get(item.entry)
                .map(exported => [`Ordinal ${exported.ordinal}`, ...exported.names].join(' · ')).join(' · ');
        }
        return summary;
    }),
};

write('manifest.json', manifest);
write('pe-boundary.json', { program, originalSHA256: sha256, imageBase: hex(image.base), entryPoint: peEntry,
    exportAddressTableEntries: addressCount, exports: peExports, imports, importsWithoutFunction });
console.log(`${functions.size} internal entries, ${externalAddresses.size} external functions, ${imports.length} IAT imports, ${peExports.length} PE exports`);
console.log(`${importsWithoutFunction.length} import entries are absent from the Ghidra external function table`);
