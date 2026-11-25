const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { originalListing, nativeCoffBody, compareNativeRanges } = require('./lib/reconstruction-listings.cjs');

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
const read = prefix => nativeCoffBody(file, coff, listing, '?' + prefix + '@');
const hex = value => value.toString(16);
const parent = originalListing(program, '20c01890', image);
const roll = read('CalculateRollMoment');
const torque = read('CalculatePropellerRollTorque');
const simple = read('CalculateSimpleRollMoment');

function verifySlot(table, offset, target) {
    assert.deepEqual(image.highLowFixups(table + offset, 4), [0]);
    assert.equal(image.initialDword(table + offset), target);
}

assert.equal(image.codeAt(0x20c4f356, 6).toString('hex'), 'c7061826bc20');
verifySlot(0x20bc2160, 0x18, 0x20c01890);
verifySlot(0x20bc2160, 0x20, 0x20c14e50);
assert.equal(image.codeAt(0x20c14e50, 4).toString('hex'), '8b410cc3');
verifySlot(0x20bc213c, 0x10, 0x20c454c0);
assert.equal(image.codeAt(0x20c454c0, 7).toString('hex'), 'd981f8130000c3');
verifySlot(0x20bbefd8, 4, 0x20c0d7d0);
verifySlot(0x20bbefe8, 4, 0x20c2db40);
const slots = [
    [0x84, 0x20c50790, 0x20c018ef, 'Sideslip'],
    [0x88, 0x20c506e0, 0x20c01928, 'Rudder'],
    [0x8c, 0x20c50310, 0x20c01a03, 'Ailerons'],
    [0x90, 0x20c50630, 0x20c01a97, 'Roll rate'],
    [0x94, 0x20c504b0, 0x20c01b06, 'Yaw rate'],
    [0x98, 0x20c50870, 0x20c0197a, 'Spoilers'],
    [0x9c, 0x20c50890, 0x20c019b3, 'Aileron trim'],
    [0xa0, 0x20c508b0, 0x20c01b18, 'Additional roll']
].map(([slot, target, load, term]) => {
    verifySlot(0x20bc2618, slot, target);
    assert.equal(image.codeAt(load + 2, 4).readUInt32LE(0), slot);
    return { slot: hex(slot), entry: hex(0x20bc2618 + slot), target: hex(target), load: hex(load), term };
});
const getters = [
    ['20c50870', 'GetSpoilerRollCoefficient'],
    ['20c50890', 'GetAileronTrimRollCoefficient']
].map(([entry, symbol]) => {
    const original = originalListing(program, entry, image);
    const rebuilt = read(symbol);
    assert.equal(rebuilt.section.relocations.length, 0);
    assert.equal(image.highLowFixups(original.va, original.bytes.length).length, 0);
    return { ...compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, [], [], []),
        symbol: 'AirplaneCoefficientModel::' + symbol };
});

const globals = new Map([
    [0x20c7b950, 'slipstreamRollCoefficient'], [0x20c7b980, 'wingLiftMultiplierSum'],
    [0x20c7b9a8, 'secondOuterWingLift'], [0x20c7b9c0, 'firstOuterWingLift'],
    [0x20c7b9b0, 'secondInnerWingLift'], [0x20c7b9b8, 'firstInnerWingLift'],
    [0x20c7b92c, 'highAlphaReferenceXShift']
].map(([address, name]) => [address, '?' + name + '@sim1@@3NA']));
const literals = new Map([
    [0x20bb4650, 0.5], [0x20bc18e8, 0.21], [0x20bc3cb0, 0.15], [0x20bc3628, 0.4],
    [0x20bc4308, 0.08], [0x20bb47a8, 0.6], [0x20bbf710, 0.05], [0x20bc4358, 0.03],
    [0x20bc3608, 1.5], [0x20bc3558, 4294967296.0], [0x20bc3790, 4.0], [0x20bc3388, 0.25],
    [0x20bc00e0, 0.75], [0x20bc3b10, 1.3]
]);
function doubleHex(value) {
    const bytes = Buffer.alloc(8);
    bytes.writeDoubleLE(value);
    return bytes.toString('hex');
}

const highAngleSamples = [[0, 1], [0.8, 1.06], [1, 1.07], [1.4, 1.08], [1.8, 1.02], [3.71, 0.45]];
const highAngleBytes = image.codeAt(0x20c71d10, 100, false);
assert.equal(highAngleBytes.readUInt32LE(0), highAngleSamples.length);
assert.equal(highAngleBytes.subarray(4).toString('hex'), highAngleSamples.flat().map(doubleHex).join(''));
const originalRefs = image.highLowFixups(parent.va, parent.bytes.length).map(offset => {
    const address = parent.bytes.readUInt32LE(offset);
    if (literals.has(address)) {
        assert.equal(image.codeAt(address, 8, false).toString('hex'), doubleHex(literals.get(address)));
    }
    const identity = globals.get(address) || (literals.has(address) && doubleHex(literals.get(address))) ||
        (address === 0x20c71d10 && 'original high-angle curve');
    assert(identity, hex(address));
    return { offset, address: hex(address), identity };
});
const curve = read('SampleAerodynamicCurve');
const curveCalls = [
    [0x20c01aee, 'high-angle ratio', '18c'],
    [0x20c01c09, 'first wing input', '62c'],
    [0x20c01c1e, 'second wing input', '634']
];
const rebuiltCurveCalls = roll.section.relocations.filter(ref => ref.type === 20 && ref.symbol.index === curve.symbol.index);
assert.equal(rebuiltCurveCalls.length, curveCalls.length);
for (const [index, [address, input, field]] of curveCalls.entries()) {
    const offset = address - parent.va;
    assert.equal(parent.bytes[offset], 0xe8);
    assert.equal(address + 5 + parent.bytes.readInt32LE(offset + 1), 0x20bcbf60);
    const rebuiltOffset = rebuiltCurveCalls[index].offset;
    const argumentLoad = roll.rows.find(([va]) => parseInt(va, 16) === rebuiltOffset - 10);
    assert(argumentLoad && argumentLoad[2].toLowerCase().includes('+' + field + 'h]'), input);
    originalRefs.push({ offset: offset + 1, address: '20bcbf60', identity: curve.symbol.name + ' / ' + input });
}
const expectedConstants = new Set([...literals.values(), ...highAngleSamples.flat()].map(doubleHex));
function rebuildRefs(body) {
    return body.section.relocations.map(({ offset, symbol, type }) => {
        assert.equal(body.bytes.readUInt32LE(offset), 0);
        if (type === 20) {
            assert.equal(body.bytes[offset - 1], 0xe8);
            assert([curve, torque, simple].some(target => target.symbol.index === symbol.index), symbol.name);
            const index = body === roll ? rebuiltCurveCalls.findIndex(item => item.offset === offset) : -1;
            if (index !== -1) return { offset, identity: symbol.name + ' / ' + curveCalls[index][1] };
            return { offset, identity: symbol.name };
        }
        assert.equal(type, 6);
        if ([...globals.values()].includes(symbol.name)) {
            assert.equal(symbol.section, 0);
            return { offset, identity: symbol.name };
        }
        const section = coff.sections[symbol.section - 1];
        assert(section && symbol.value + 8 <= section.size, symbol.name);
        const literal = file.subarray(section.raw + symbol.value, section.raw + symbol.value + 8).toString('hex');
        assert(expectedConstants.has(literal), symbol.name);
        return { offset, identity: literal };
    });
}

const stackTerms = [
    ['first aileron', 0x20c01a93, 0x20c0203d, 0x70],
    ['second aileron', 0x20c01a26, 0x20c02041, 0x78],
    ['aileron trim', 0x20c019dd, 0x20c02045, 0x90],
    ['spoilers', 0x20c019a2, 0x20c0204c, 0x80],
    ['yaw rate', 0x20c01b26, 0x20c02050, 0x48],
    ['roll rate', 0x20c01ab7, 0x20c02054, 0x98],
    ['rudder', 0x20c0194b, 0x20c0205b, 0xd8],
    ['sideslip', 0x20c01917, 0x20c02062, 0xc8],
    ['slipstream', 0x20c018f7, 0x20c02069, 0xc0],
    ['additional roll', 0x20c01b3d, 0x20c02074, 0xd0],
    ['supplemental', 0x20c01b4c, 0x20c0207b, 0xe0]
];
const instructions = new Map(parent.rows.map(([address, , instruction]) => [address, instruction]));
const originalSumOrder = stackTerms.map(([name, store, load, stackOffset], index) => {
    assert.equal(instructions.get(hex(store)), 'FSTP double ptr [ESP + 0x' + hex(stackOffset) + ']');
    const mnemonic = index === 0 ? 'FLD' : 'FADD';
    assert.equal(instructions.get(hex(load)), mnemonic + ' double ptr [ESP + 0x' + hex(stackOffset - 12) + ']');
    return { name, store: hex(store), add: hex(load) };
});
assert.equal(image.codeAt(0x20c02070, 4).toString('hex'), 'dec3d9ca');
assert.equal(image.codeAt(0x20c02082, 6).toString('hex'), 'dec2dec1dec1');
originalSumOrder.splice(9, 0, { name: 'propeller reaction torque', add: '20c02070', source: 'ST3' });
originalSumOrder.push({ name: 'force reference transfer', add: '20c02082', source: 'ST2' },
    { name: 'outer wing lift difference', add: '20c02084' }, { name: 'inner wing lift difference', add: '20c02086' });

const slices = [
    [0x20c018e9, 0x20c018ef, 'Multiply by the published slipstream roll coefficient'],
    [0x20c01920, 0x20c01926, 'Read the rudder deflection'],
    [0x20c01957, 0x20c0195d, 'Read the first spoiler deflection'],
    [0x20c0196b, 0x20c01971, 'Read the second spoiler deflection'],
    [0x20c019ab, 0x20c019b1, 'Read the binary32 aileron trim setting'],
    [0x20c019c5, 0x20c019cb, 'Apply aileron trim effectiveness'],
    [0x20c01a5f, 0x20c01a6b, 'Add the two aileron deflections in the combined path'],
    [0x20c01a75, 0x20c01a7b, 'Scale the combined aileron deflections by one half'],
    [0x20c01aeb, 0x20c01af3, 'Pass relative angle and call the high-angle curve lookup'],
    [0x20c01c06, 0x20c01c0e, 'Pass the first wing attenuation input to the curve lookup'],
    [0x20c01c1b, 0x20c01c23, 'Pass the second wing attenuation input to the curve lookup'],
    [0x20c01f68, 0x20c01f7a, 'Subtract and weight the outer wing lift values'],
    [0x20c01f84, 0x20c01f96, 'Subtract and weight the inner wing lift values']
];
const moment = compareNativeRanges(parent, roll, parent.va, parent.va + parent.bytes.length, originalRefs, rebuildRefs(roll), slices);
const torqueScope = compareNativeRanges(parent, torque, 0x20c01c55, 0x20c01f41, originalRefs, rebuildRefs(torque), [
    [0x20c01c86, 0x20c01c8c, 'Scale the torque reference speed by 1.5'],
    [0x20c01e0c, 0x20c01e12, 'Index the 0xec-byte engine record']
]);
const simpleScope = compareNativeRanges(parent, simple, 0x20c01fde, 0x20c02003, originalRefs, rebuildRefs(simple), [
    [0x20c01fe2, 0x20c01fea, 'Load and apply the 0.75 aileron weight'],
    [0x20c01ffb, 0x20c02003, 'Scale the yaw rate term by 1.3 and add it']
]);
const report = {
    program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), slots, highAngleSamples, getters,
    moment: { ...moment, symbol: 'AirplaneMomentModel::CalculateRollMoment', originalSumOrder },
    torque: { ...torqueScope, symbol: 'AirplaneMomentModel::CalculatePropellerRollTorque', parentEntry: moment.entry },
    simple: { ...simpleScope, symbol: 'AirplaneMomentModel::CalculateSimpleRollMoment', parentEntry: moment.entry }
};
fs.writeFileSync(path.join(run, 'roll-sum-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [...getters, report.moment, report.torque, report.simple]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} native COFF bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
