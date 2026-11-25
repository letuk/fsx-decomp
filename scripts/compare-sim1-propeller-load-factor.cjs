const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {openImage} = require('./lib/pe32-dll.cjs');
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
const original = originalListing(program, '20c48710', image);
const caller = originalListing(program, '20c47420', image);

function callTarget(va) {
    const call = image.codeAt(va, 5);
    assert.equal(call[0], 0xe8);
    return va + 5 + call.readInt32LE(1);
}

assert.equal(callTarget(0x20c47451), original.va);
assert.equal(callTarget(0x20c487ab), 0x20bfee30);
const expected = new Map([[0x20bc01c4, 0.0], [0x20bc3798, 0.0004], [0x20bc4268, 0.0008],
    [0x20bc4260, 0.0012], [0x20bc4258, 0.0016], [0x20bc4250, -0.0016], [0x20bc4248, -0.016]]);
const refs = image.highLowFixups(original.va, original.bytes.length).map(offset => {
    const va = original.bytes.readUInt32LE(offset);
    const float = va === 0x20bc01c4;
    const bytes = image.codeAt(va, float ? 4 : 8, false);
    assert.equal(float ? bytes.readFloatLE(0) : bytes.readDoubleLE(0), expected.get(va));
    return {offset, address: va.toString(16), identity: bytes.toString('hex')};
});
assert.equal(refs.length, expected.size);

const table = Buffer.alloc(40);
assert.equal(image.codeAt(0x20c48718, 2).toString('hex'), 'd9ee');
for (const [i, address] of [0x20c48723, 0x20c48733, 0x20c4873e, 0x20c48747].entries()) {
    const load = image.codeAt(address, 6);
    assert.equal(load.readUInt16LE(), 0x05dd);
    image.codeAt(load.readUInt32LE(2), 8, false).copy(table, (i + 1) * 8);
}

const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const symbol = 'FlightDynamics::UpdatePropellerLoadFactor';
const rebuilt = nativeCoffBody(file, coff, dump, '?UpdatePropellerLoadFactor@FlightDynamics@sim1@@');
const circuitSymbol = [...coff.symbols.values()].find(item =>
    item.name.startsWith('?CircuitAvailability@ElectricalRuntime@sim1@@'));
const circuitIdentity = 'ElectricalRuntime::CircuitAvailability';
if (circuitSymbol) {
    const calls = rebuilt.section.relocations.filter(relocation => relocation.type === 20);
    assert.equal(calls.length, 1);
    assert.equal(calls[0].symbol.index, circuitSymbol.index);
    refs.push({offset: 0x20c487ac - original.va, address: '20bfee30', identity: circuitIdentity});
}
const special = rebuilt.section.relocations.filter(({symbol}) =>
    symbol.name === '__real@00000000' || symbol.name.startsWith('?accumulationRates@'));
assert.equal(special.length, 2);
const rebuiltRefs = special.map(({offset, symbol, type}) => {
    assert.equal(type, 6);
    assert.equal(rebuilt.bytes.readUInt32LE(offset), 0);
    const data = coff.sections[symbol.section - 1];
    const start = data.raw + symbol.value;
    if (symbol.name === '__real@00000000') {
        assert.equal(file.readUInt32LE(start), 0);
        return {offset, symbol: symbol.name, identity: '00000000'};
    }
    assert(symbol.value + table.length <= data.size);
    assert(file.subarray(start, start + table.length).equals(table));
    return {offset, symbol: symbol.name, identity: 'Propeller ice accumulation rates'};
});
rebuiltRefs.push(...nativeCoffReferences(file, coff, {...rebuilt, section: {...rebuilt.section,
    relocations: rebuilt.section.relocations.filter(relocation => !special.includes(relocation))}},
    circuitSymbol ? {symbol: circuitSymbol} : null, circuitIdentity, refs.map(ref => ref.identity)));

const comparison = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, refs, rebuiltRefs, [
    [0x20c48710, 0x20c48715, 'Hotpatch entry and frame setup'],
    [0x20c4871a, 0x20c4871d, 'Read the flight environment pointer'],
    [0x20c48720, 0x20c48723, 'Read the icing severity slot'],
    [0x20c48753, 0x20c48764, 'Round temperature to float and compare with zero'],
    [0x20c48766, 0x20c48768, 'Test the severity for zero'],
    [0x20c4876a, 0x20c48771, 'Check the environment icing-condition flag'],
    [0x20c48777, 0x20c4877a, 'Apply the selected accumulation rate to the step'],
    [0x20c4877c, 0x20c48785, 'Apply the slow decay rate'],
    [0x20c48787, 0x20c48790, 'Apply the fast decay rate'],
    [0x20c48797, 0x20c4879a, 'Round the increment to binary64 before accumulating it'],
    [0x20c487ab, 0x20c487b0, 'Call the shared electrical availability query'],
    [0x20c48843, 0x20c48845, 'Store the clamped propeller load factor'],
    [0x20c4885d, 0x20c48861, 'Restore the saved register and frame']
]);

const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), symbol,
    caller: {entry: caller.va.toString(16), call: '20c47451'}, circuitQuery: {call: '20c487ab', target: '20bfee30'},
    accumulationRates: {status: 'Exact', bytes: table.length, values: Array.from({length: 5}, (_, i) => table.readDoubleLE(i * 8))},
    ...comparison};
fs.writeFileSync(path.join(run, 'propeller-load-factor-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${comparison.entry} ${comparison.status}: ${comparison.originalBytes} original / ${comparison.rebuiltBytes} native COFF bytes`);
for (const scope of comparison.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
console.log(`Exact: ${table.length} bytes, ice accumulation rate table`);
