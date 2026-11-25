const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {openImage} = require('./lib/pe32-dll.cjs');
const {parseNativeCoff} = require('./lib/coff.cjs');
const {originalListing, nativeCoffBody, compareNativeRanges} = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 Electrical run directory or a historical Turbine run');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const standalone = path.join(run, 'sim1-electrical-circuit.obj');
const file = fs.readFileSync(fs.existsSync(standalone) ? standalone : path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const lookup = originalListing(program, '20bfef40', image);
const circuit = originalListing(program, '20bfee30', image);
const rebuiltLookup = nativeCoffBody(file, coff, dump, '?FindBusVoltage@ElectricalRuntime@sim1@@');
const rebuiltCircuit = nativeCoffBody(file, coff, dump, '?CircuitAvailability@ElectricalRuntime@sim1@@');

function verifyCall(va, target) {
    const call = image.codeAt(va, 5);
    assert.equal(call[0], 0xe8);
    assert.equal(va + 5 + call.readInt32LE(1), target);
}

verifyCall(0x20c4829e, circuit.va);
verifyCall(0x20c487ab, circuit.va);
verifyCall(0x20bfee65, lookup.va);
assert.equal(image.codeAt(0x20c48299, 5).toString('hex'), 'bf09000000');
assert.equal(image.codeAt(0x20c487a6, 5).toString('hex'), 'bf11000000');

const jumpTable = 0x20bfef70;
const option = 0x20c7c008;
assert.deepEqual(image.highLowFixups(lookup.va, lookup.bytes.length), [8]);
assert.equal(lookup.bytes.readUInt32LE(8), jumpTable);
assert.deepEqual(image.highLowFixups(jumpTable, 32), [0, 4, 8, 12, 16, 20, 24, 28]);
const table = image.codeAt(jumpTable, 32);
for (let i = 0; i < 8; i++) {
    const target = table.readUInt32LE(i * 4);
    assert.equal(target, 0x20bfef4c + i * 4);
    assert.equal(image.codeAt(target, 4).toString('hex'),
        Buffer.from([0x83, 0xc0, 4 + i * 16, 0xc3]).toString('hex'));
}
assert.deepEqual(image.highLowFixups(circuit.va, circuit.bytes.length), [0x70]);
assert.equal(circuit.bytes.readUInt32LE(0x70), option);
assert.equal(image.initialDword(option), 0);
assert.equal(rebuiltLookup.section.relocations.length, 0);
const rebuiltCircuitRefs = rebuiltCircuit.section.relocations.map(({offset, symbol, type}) => {
    assert.equal(rebuiltCircuit.bytes.readUInt32LE(offset), 0);
    if (type === 20) {
        assert.equal(rebuiltCircuit.bytes[offset - 1], 0xe8);
        assert.equal(symbol.index, rebuiltLookup.symbol.index);
        return {offset, identity: 'bus lookup', symbol: symbol.name};
    }
    assert.equal(type, 6);
    assert.equal(symbol.name, '?electricAlwaysAvailable@sim1@@3HA');
    return {offset, identity: 'electric_always_available', symbol: symbol.name};
});
assert.deepEqual(rebuiltCircuitRefs.map(ref => ref.identity).sort(), ['bus lookup', 'electric_always_available']);

const comparisons = [
    {...compareNativeRanges(lookup, rebuiltLookup, lookup.va, lookup.va + lookup.bytes.length,
        [{offset: 8, address: '20bfef70', identity: 'original jump table'}], [], []),
        symbol: 'ElectricalRuntime::FindBusVoltage'},
    {...compareNativeRanges(circuit, rebuiltCircuit, circuit.va, circuit.va + circuit.bytes.length,
        [{offset: 0x36, address: '20bfef40', identity: 'bus lookup'},
         {offset: 0x70, address: '20c7c008', identity: 'electric_always_available'}], rebuiltCircuitRefs, [
            [0x20bfee3c, 0x20bfee46, 'Load and call owner dispatch slot 0x114'],
            [0x20bfee46, 0x20bfee49, 'Compare the owner mode with one'],
            [0x20bfee4b, 0x20bfee51, 'Read the raw fallback availability'],
            [0x20bfee60, 0x20bfee63, 'Read the descriptor bus selector'],
            [0x20bfee65, 0x20bfee6a, 'Call the bus voltage lookup'],
            [0x20bfee6e, 0x20bfee78, 'Compare minimum voltage to the bus voltage'],
            [0x20bfee9e, 0x20bfeea5, 'Read electric_always_available']
        ]), symbol: 'ElectricalRuntime::CircuitAvailability'}
];
const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    circuitCalls: {automaticFeather: '20c4829e', propellerDeice: '20c487ab'},
    jumpTable: {entry: '20bfef70', bytes: 32, targets: Array.from({length: 8}, (_, i) => table.readUInt32LE(i * 4).toString(16))},
    rebuiltCircuitRefs, comparisons};
fs.writeFileSync(path.join(run, 'electrical-circuit-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const match of comparisons) {
    console.log(`${match.entry} ${match.status}: ${match.originalBytes} original / ${match.rebuiltBytes} native COFF bytes`);
    for (const scope of match.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
