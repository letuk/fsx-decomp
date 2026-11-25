const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { originalListing, nativeCoffBody, nativeCoffReferences, compareNativeRanges } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 aerodynamic moment run directory');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const file = fs.readFileSync(path.join(run, 'sim1-aerodynamic-moment.obj'));
const coff = parseNativeCoff(file);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const read = prefix => nativeCoffBody(file, coff, listing, '?' + prefix);
const hex = value => value.toString(16);
const parent = originalListing(program, '20c013a0', image);
const curve = read('SampleAerodynamicCurve@sim1@@');
const moment = read('CalculatePitchMoment@AirplaneMomentModel@sim1@@');
const simple = read('CalculateSimplePitchMoment@AirplaneMomentModel@sim1@@');
const tail = read('CalculateSlipstreamTailPitchMoment@AirplaneMomentModel@sim1@@');

function checkCall(body, relocation, target) {
    assert.equal(relocation.type, 20);
    assert.equal(body.bytes[relocation.offset - 1], 0xe8);
    assert.equal(body.bytes.readUInt32LE(relocation.offset), 0);
    assert.equal(relocation.symbol.index, target.symbol.index);
}

const getters = [
    ['20c50270', 'GetFlapPitchCoefficient'],
    ['20c50290', 'GetSpoilerPitchCoefficient'],
    ['20c502b0', 'GetSlipstreamElevatorCoefficient'],
    ['20c502d0', 'GetSlipstreamPitchRateCoefficient']
].map(([entry, name], index) => {
    const original = originalListing(program, entry, image);
    const rebuilt = read(name + '@AirplaneCoefficientModel@sim1@@');
    const slot = 0x20bc2688 + index * 4;
    assert.deepEqual(image.highLowFixups(slot, 4), [0]);
    assert.equal(image.initialDword(slot), original.va);
    assert.deepEqual(image.highLowFixups(original.va, original.bytes.length), []);
    assert.equal(rebuilt.section.relocations.length, 0);
    return {
        original, rebuilt,
        record: { ...compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, [], [], []),
            slot: hex(slot), symbol: 'AirplaneCoefficientModel::' + name }
    };
});
assert.equal(tail.section.relocations.length, 2);
for (const [index, item] of getters.slice(2).entries()) {
    checkCall(tail, tail.section.relocations[index], item.rebuilt);
}
assert.equal(image.codeAt(0x20c016ca, 7).toString('hex'), '8b507c8bceffd2');
assert.equal(image.codeAt(0x20c016ea, 6).toString('hex'), '8b9080000000');
const tailRefs = tail.section.relocations.map(({ offset, symbol }) => ({ offset, symbol: symbol.name }));

const literals = new Map([
    [0x20bbf7f0, '399d52a246df913f'],
    [0x20bb4650, '000000000000e03f'],
    [0x20bc2f50, '8744e74a1857d63f'],
    [0x20bc4290, '182d4454fb21e93f'],
    [0x20bbf7d8, '9a9999999999c93f'],
    [0x20bc3628, '9a9999999999d93f'],
    [0x20bb1760, '0000000000000840']
]);
// The pre-existing gyro comparator validates this conversion against the same pinned PE.
literals.set(0x20bc35a8, image.codeAt(0x20bc35a8, 8, false).toString('hex'));
const globals = new Map([
    [0x20c7b958, '?slipstreamPitchCoefficient@sim1@@3NA'],
    [0x20c7b93c, '?highAlphaReferenceZShift@sim1@@3NA']
]);
assert.equal(image.codeAt(0x20c009e5, 6).toString('hex'), 'dd1d58b9c720');
const originalRefs = image.highLowFixups(parent.va, parent.bytes.length).map(offset => {
    const address = parent.bytes.readUInt32LE(offset);
    const identity = globals.get(address) || literals.get(address);
    assert(identity, hex(address));
    if (literals.has(address)) assert.equal(image.codeAt(address, 8, false).toString('hex'), identity);
    return { offset, address: hex(address), identity };
});
const simpleStart = 0x20c0177b;
const simpleEnd = 0x20c01848;
const simpleRefs = originalRefs.filter(ref => ref.offset >= simpleStart - parent.va && ref.offset < simpleEnd - parent.va);
const curveCall = 0x20c01812;
const curveOffset = curveCall - parent.va;
assert.equal(parent.bytes[curveOffset], 0xe8);
assert.equal(curveCall + 5 + parent.bytes.readInt32LE(curveOffset + 1), 0x20bcbf60);
const curveIdentity = 'sim1.dll 0x20bcbf60';
const curveRef = { offset: curveOffset + 1, address: '20bcbf60', identity: curveIdentity };
simpleRefs.push(curveRef);
originalRefs.push(curveRef);
const rebuiltSimpleRefs = nativeCoffReferences(file, coff, simple, curve,
    curveIdentity, [...literals.values()]);
assert.deepEqual(rebuiltSimpleRefs.map(ref => ref.identity).sort(), simpleRefs.map(ref => ref.identity).sort());

const mainCallSymbols = [
    'CalculateBaselineMoment@AirplaneCoefficientModel@sim1@@',
    'SampleAerodynamicCurve@sim1@@',
    'CalculateElevatorMoment@AirplaneMomentModel@sim1@@',
    'CalculatePitchRateCoefficient@AirplaneCoefficientModel@sim1@@',
    'CalculateAngleOfAttackRateCoefficient@AirplaneCoefficientModel@sim1@@',
    'CalculateHorizontalTailIncidenceMoment@AirplaneMomentModel@sim1@@',
    'CalculateElevatorTrimMoment@AirplaneMomentModel@sim1@@',
    'GetFlapPitchCoefficient@AirplaneCoefficientModel@sim1@@',
    'GetSpoilerPitchCoefficient@AirplaneCoefficientModel@sim1@@',
    'CalculatePropellerGyroscopicMoment@AirplaneMomentModel@sim1@@',
    'CalculateSlipstreamTailPitchMoment@AirplaneMomentModel@sim1@@',
    'CalculateSimplePitchMoment@AirplaneMomentModel@sim1@@'
];
const calls = moment.section.relocations.filter(item => item.type === 20);
assert.equal(calls.length, mainCallSymbols.length);
mainCallSymbols.forEach((name, index) => checkCall(moment, calls[index], read(name)));
const mainDataRefs = moment.section.relocations.filter(item => item.type !== 20).map(({ offset, symbol, type }) => {
    assert.equal(type, 6);
    assert.equal(moment.bytes.readUInt32LE(offset), 0);
    if ([...globals.values()].includes(symbol.name)) {
        assert.equal(symbol.section, 0);
        return { offset, identity: symbol.name };
    }
    const data = coff.sections[symbol.section - 1];
    assert(data && symbol.value + 8 <= data.size);
    const identity = file.subarray(data.raw + symbol.value, data.raw + symbol.value + 8).toString('hex');
    assert([...literals.values()].includes(identity));
    return { offset, identity };
});

const sumTerms = [
    ['angle', 0x20c01434, 0x20c01848, 0x58, 0],
    ['baseline', 0x20c01404, 0x20c0184d, 0x80, 4],
    ['elevator', 0x20c0145e, 0x20c01853, 0x50, 12],
    ['pitch rate', 0x20c01477, 0x20c01857, 0x38, 12],
    ['alpha rate', 0x20c01490, 0x20c0185b, 0x70, 12],
    ['tail incidence', 0x20c014b5, 0x20c0185f, 0x90, 12],
    ['elevator trim', 0x20c014e2, 0x20c01866, 0x60, 12],
    ['slipstream', 0x20c014f2, 0x20c0186a, 0x68, 12],
    ['flaps', 0x20c0150d, 0x20c0186e, 0x78, 12],
    ['spoilers', 0x20c01550, 0x20c01872, 0x88, 12],
    ['gyro', 0x20c0169b, 0x20c01876, 0x30, 12],
    ['supplemental', 0x20c01741, 0x20c0187c, 0x48, 12],
    ['slipstream tail', 0x20c01727, 0x20c01880, 0x40, 12]
];
const originalInstruction = new Map(parent.rows.map(([address, , instruction]) => [address, instruction]));
const sum = sumTerms.map(([name, store, load, offset, freed]) => {
    assert.equal(originalInstruction.get(hex(store)), 'FSTP double ptr [ESP + 0x' + hex(offset) + ']');
    const operation = load === 0x20c01848 ? 'FLD' : 'FADD';
    assert.equal(originalInstruction.get(hex(load)), operation + ' double ptr [ESP + 0x' + hex(offset - freed) + ']');
    return { name, store: hex(store), add: hex(load), stackOffset: hex(offset), poppedBytes: freed };
});
assert.equal(originalInstruction.get('20c0187a'), 'FADDP');
sum.splice(11, 0, { name: 'force reference transfer', add: '20c0187a', source: 'ST1' });

const report = {
    program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    getters: getters.map(item => item.record),
    moment: { ...compareNativeRanges(parent, moment, parent.va, parent.va + parent.bytes.length,
        originalRefs, mainDataRefs, [
            [0x20c014ec, 0x20c014f2, 'Multiply by the published slipstream pitch coefficient'],
            [0x20c01519, 0x20c0151f, 'Read the second spoiler angle'],
            [0x20c0151f, 0x20c01525, 'Add the first spoiler angle'],
            [0x20c0152c, 0x20c01532, 'Scale the combined spoiler angle by one half']
        ]),
        symbol: 'AirplaneMomentModel::CalculatePitchMoment', originalSumOrder: sum, directCalls: calls.map(item => item.symbol.name)
    },
    simple: { ...compareNativeRanges(parent, simple, simpleStart, simpleEnd, simpleRefs, rebuiltSimpleRefs, [
        [0x20c01785, 0x20c01790, 'Compare absolute pitch to the lower breakpoint'],
        [0x20c017b7, 0x20c017c2, 'Test elevator deflection times pitch against one'],
        [0x20c0180f, 0x20c01817, 'Pass absolute pitch and evaluate its scale curve'],
        [0x20c0182d, 0x20c0183f, 'Weight elevator and pitch-rate moments and add them']
    ]), symbol: 'AirplaneMomentModel::CalculateSimplePitchMoment', parentEntry: '20c013a0' },
    tail: { ...compareNativeRanges(parent, tail, 0x20c016ad, 0x20c0172b, [], tailRefs, [
        [0x20c016b4, 0x20c016c4, 'Call the control-surface getter and read elevator deflection'],
        [0x20c016d9, 0x20c016df, 'Subtract horizontal-tail incidence'],
        [0x20c016e1, 0x20c016e8, 'Apply the coefficient and calculate the wing area and chord product']
    ]), symbol: 'AirplaneMomentModel::CalculateSlipstreamTailPitchMoment', parentEntry: '20c013a0' }
};
fs.writeFileSync(path.join(run, 'pitch-sum-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [...report.getters, report.moment, report.simple, report.tail]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} native COFF bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
