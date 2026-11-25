const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {openImage} = require('./lib/pe32-dll.cjs');
const {readPE32Imports} = require('./lib/pe-imports.cjs');
const {parseNativeCoff} = require('./lib/coff.cjs');
const {originalListing, nativeCoffBody, nativeCoffReferences, compareNativeRanges} = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 Turbine run directory');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const original = originalListing(program, '20c47ae0', image);
const table = originalListing(program, '20bcc030', image);
const caller = originalListing(program, '20c47420', image);

function directCallTarget(va) {
    const call = image.codeAt(va, 5);
    assert.equal(call[0], 0xe8);
    return va + 5 + call.readInt32LE(1);
}

assert.equal(directCallTarget(0x20c47536), original.va);
assert.equal(directCallTarget(0x20c47b44), table.va);
const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const tableSymbol = 'PropellerCoefficientTable::Evaluate';
const symbol = 'PropellerForceRecord::CalculateAerodynamicTorque';
const rebuilt = nativeCoffBody(file, coff, dump, '?CalculateAerodynamicTorque@PropellerForceRecord@sim1@@');
const rebuiltTable = nativeCoffBody(file, coff, dump, '?Evaluate@PropellerCoefficientTable@sim1@@');

const expected = new Map([[0x20bc3590, 0.0001], [0x20bc3550, 57.29577951308232], [0x20bc3690, 6.283185307179586]]);
const refs = image.highLowFixups(original.va, original.bytes.length).map(offset => {
    const va = original.bytes.readUInt32LE(offset);
    const bytes = image.codeAt(va, 8, false);
    assert.equal(bytes.readDoubleLE(0), expected.get(va));
    return {offset, address: va.toString(16), identity: bytes.toString('hex')};
});
assert.equal(refs.length, expected.size);
const literalIdentities = refs.map(ref => ref.identity);
refs.push({offset: 0x20c47b45 - original.va, address: table.va.toString(16), identity: tableSymbol});
const rebuiltRefs = nativeCoffReferences(file, coff, rebuilt, rebuiltTable, tableSymbol, literalIdentities);
const comparison = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, refs, rebuiltRefs, [
    [0x20c47ae0, 0x20c47ae5, 'Hotpatch entry and frame setup'],
    [0x20c47afb, 0x20c47b0c, 'Compare the absolute pitch difference with the feather tolerance'],
    [0x20c47b2e, 0x20c47b34, 'Convert blade pitch from radians to degrees'],
    [0x20c47b41, 0x20c47b4b, 'Pass the advance ratio to the coefficient lookup and call it'],
    [0x20c47b4e, 0x20c47b5d, 'Reverse the power coefficient for negative angular speed'],
    [0x20c47b60, 0x20c47b66, 'Load the ambient air density'],
    [0x20c47b8f, 0x20c47b95, 'Divide by one full revolution in radians']
]);

const isnan = 'msvcr80.dll!_isnan';
const imports = readPE32Imports(binary);
const tableRefs = image.highLowFixups(table.va, table.bytes.length).map(offset => {
    const va = table.bytes.readUInt32LE(offset);
    assert.equal(table.bytes.readUInt16LE(offset - 2), 0x15ff);
    assert.deepEqual(imports.get(va), {dll: 'msvcr80.dll', name: '_isnan'});
    return {offset, address: va.toString(16), identity: isnan};
});
assert.equal(tableRefs.length, 2);
const tableImports = rebuiltTable.section.relocations.filter(relocation => relocation.symbol.name === '__imp___isnan');
assert(tableImports.length > 0);
const rebuiltTableRefs = tableImports.map(({offset, symbol: imported, type}) => {
    assert.equal(type, 6);
    assert.equal(imported.section, 0);
    assert.equal(rebuiltTable.bytes.readUInt32LE(offset), 0);
    return {offset, symbol: imported.name, identity: isnan};
});
const otherRelocations = rebuiltTable.section.relocations.filter(relocation => !tableImports.includes(relocation));
const tableHelper = otherRelocations.length ? {symbol: coff.unique(otherRelocations[0].symbol.name)} : null;
if (tableHelper) assert.match(tableHelper.symbol.name, /^\?FindCoefficientInterval@sim1@@/);
rebuiltTableRefs.push(...nativeCoffReferences(file, coff, {...rebuiltTable, section: {...rebuiltTable.section,
    relocations: otherRelocations}}, tableHelper, 'Reconstructed coefficient interval helper', []));
const tableComparison = compareNativeRanges(table, rebuiltTable, table.va, table.va + table.bytes.length,
    tableRefs, rebuiltTableRefs, [
        [0x20bcc030, 0x20bcc035, 'Hotpatch entry and frame setup'],
        [0x20bcc059, 0x20bcc05e, 'Clean up the stack and check the first NaN result'],
        [0x20bcc064, 0x20bcc06d, 'Load and pass the second coordinate to the NaN checker'],
        [0x20bcc21d, 0x20bcc223, 'Combine the two row interpolations with the advance-ratio weight'],
        [0x20bcc233, 0x20bcc237, 'Return and remove the two coordinate arguments']
    ]);

const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    caller: {entry: caller.va.toString(16), call: '20c47536'},
    functions: [{symbol, ...comparison}, {symbol: tableSymbol, ...tableComparison}]};
fs.writeFileSync(path.join(run, 'propeller-load-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const result of report.functions) {
    console.log(`${result.entry} ${result.status}: ${result.originalBytes} original / ${result.rebuiltBytes} native COFF bytes`);
    for (const scope of result.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
