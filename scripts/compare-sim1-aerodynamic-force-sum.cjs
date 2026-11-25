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
const sha = data => createHash('sha256').update(data).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const file = fs.readFileSync(path.join(run, 'sim1-aerodynamic-moment.obj'));
const coff = parseNativeCoff(file);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const read = name => nativeCoffBody(file, coff, listing, '?' + name + '@');
const hex = value => value.toString(16);
const original = entry => originalListing(program, entry, image);
const lift = original('20c00a00');
const drag = original('20c00d80');
const side = original('20c00f40');
const producer = original('20c45510');
const liftBuild = read('CalculateLift');
const dragBuild = read('CalculateDrag');
const sideBuild = read('CalculateSideForce');
const vectorBuild = read('CalculateAerodynamicForce');
const curve = read('SampleAerodynamicCurve');

function verifySlot(table, slot, target) {
    assert.deepEqual(image.highLowFixups(table + slot, 4), [0]);
    assert.equal(image.initialDword(table + slot), target);
}

assert.equal(image.codeAt(0x20c4f356, 6).toString('hex'), 'c7061826bc20');
verifySlot(0x20bc2160, 0x20, 0x20c14e50);
assert.equal(image.codeAt(0x20c14e50, 4).toString('hex'), '8b410cc3');
const forceSlots = [[8, lift.va, 0x20c45657], [12, drag.va, 0x20c45669], [16, side.va, 0x20c4567b]];
for (const [slot, target, load] of forceSlots) {
    verifySlot(0x20bc2160, slot, target);
    assert.equal(image.codeAt(load + 2, 1)[0], slot);
}
const slots = [
    [0x04, 0x20c4f3b0, 0x20c00a83, 'Raw-alpha lift'],
    [0x08, 0x20c4f3e0, 0x20c00c79, 'Pitch-rate lift'],
    [0x0c, 0x20c4f460, 0x20c00ca9, 'Elevator lift'],
    [0x10, 0x20c4f4f0, 0x20c00cf6, 'Alpha-rate lift'],
    [0x14, 0x20c4f570, 0x20c00d0f, 'Tail incidence lift'],
    [0x1c, 0x20c4f640, 0x20c00cdf, 'Spoiler lift'],
    [0x20, 0x20c4f660, 0x20c00a63, 'Span-normalized lift scale'],
    [0x24, 0x20c4f6c0, 0x20c00a70, 'Lift curve scale'],
    [0x28, 0x20c4f700, 0x20c00dc5, 'Base drag'],
    [0x2c, 0x20c4f7e0, 0x20c00dfc, 'Induced drag'],
    [0x30, 0x20c4fa50, 0x20c00e30, 'Flap drag'],
    [0x38, 0x20c4fa90, 0x20c00e6a, 'Spoiler drag'],
    [0x3c, 0x20c4f760, 0x20c00e19, 'Supplementary drag'],
    [0x40, 0x20c4f8e0, 0x20c00ddf, 'Drag scale'],
    [0x44, 0x20c4fbb0, 0x20c00f82, 'Roll-rate side force'],
    [0x48, 0x20c4fab0, 0x20c00f73, 'Yaw-rate side force'],
    [0x4c, 0x20c4fb30, 0x20c00fb8, 'Rudder side force'],
    [0x50, 0x20c4fc30, 0x20c00fc9, 'Sideslip side force']
].map(([slot, target, load, term]) => {
    verifySlot(0x20bc2618, slot, target);
    assert.equal(image.codeAt(load + 2, 1)[0], slot);
    return { slot: hex(slot), entry: hex(0x20bc2618 + slot), target: hex(target), load: hex(load), term };
});
const getters = [
    ['20c4f640', 'GetSpoilerLiftCoefficient'],
    ['20c4fa50', 'GetFlapDragCoefficient'],
    ['20c4fa90', 'GetSpoilerDragCoefficient']
].map(([entry, symbol]) => {
    const body = original(entry);
    const rebuilt = read(symbol);
    assert.equal(rebuilt.section.relocations.length, 0);
    assert.equal(image.highLowFixups(body.va, body.bytes.length).length, 0);
    return { ...compareNativeRanges(body, rebuilt, body.va, body.va + body.bytes.length, [], [], []),
        symbol: 'AirplaneCoefficientModel::' + symbol };
});

const globals = new Map([
    [0x20c7b960, 'firstWingSlipstreamDrag'], [0x20c7b968, 'secondWingSlipstreamDrag'],
    [0x20c7b970, 'firstInnerWingSlipstreamLift'], [0x20c7b978, 'secondInnerWingSlipstreamLift'],
    [0x20c7b988, 'secondOuterWingLiftMultiplier'], [0x20c7b990, 'secondInnerWingLiftMultiplier'],
    [0x20c7b998, 'firstInnerWingLiftMultiplier'], [0x20c7b9a0, 'firstOuterWingLiftMultiplier'],
    [0x20c7b9a8, 'secondOuterWingLift'], [0x20c7b9b0, 'secondInnerWingLift'],
    [0x20c7b9b8, 'firstInnerWingLift'], [0x20c7b9c0, 'firstOuterWingLift']
].map(([address, name]) => [address, '?' + name + '@sim1@@3NA']));
const literals = new Map([
    [0x20bc3388, 0.25], [0x20bc18e8, 0.21], [0x20bc3cb0, 0.15], [0x20bc3628, 0.4],
    [0x20bc4308, 0.08], [0x20bb47a8, 0.6], [0x20bbf710, 0.05], [0x20bc4358, 0.03],
    [0x20bb4650, 0.5], [0x20bc3d30, -0.6], [0x20bb46b0, 0.047], [0x20bbf6d0, 0]
]);
function doubleHex(value) {
    const bytes = Buffer.alloc(8);
    bytes.writeDoubleLE(value);
    return bytes.toString('hex');
}

function originalRefs(body) {
    return image.highLowFixups(body.va, body.bytes.length).map(offset => {
        const address = body.bytes.readUInt32LE(offset);
        if (literals.has(address)) {
            assert.equal(image.codeAt(address, 8, false).toString('hex'), doubleHex(literals.get(address)));
        }
        const identity = globals.get(address) || (literals.has(address) && doubleHex(literals.get(address)));
        assert(identity, hex(address));
        return { offset, address: hex(address), identity };
    });
}

const liftRefs = originalRefs(lift);
const curveInputs = [[0x20c00bbe, '62c'], [0x20c00bd3, '634']];
const rebuiltCurveCalls = liftBuild.section.relocations.filter(ref => ref.type === 20 && ref.symbol.index === curve.symbol.index);
assert.equal(rebuiltCurveCalls.length, curveInputs.length);
for (const [index, [address, input]] of curveInputs.entries()) {
    const offset = address - lift.va;
    assert.equal(lift.bytes[offset], 0xe8);
    assert.equal(address + 5 + lift.bytes.readInt32LE(offset + 1), 0x20bcbf60);
    const argument = liftBuild.rows.find(([va]) => parseInt(va, 16) === rebuiltCurveCalls[index].offset - 10);
    assert(argument && argument[2].toLowerCase().includes('+' + input + 'h]'), input);
    liftRefs.push({ offset: offset + 1, address: '20bcbf60', identity: curve.symbol.name + '/' + input });
}
const constants = new Set([...literals.values()].map(doubleHex));
const forceCalls = [liftBuild, dragBuild, sideBuild];
const rebuiltForceCalls = vectorBuild.section.relocations.filter(ref => ref.type === 20);
assert.deepEqual(rebuiltForceCalls.map(ref => ref.symbol.index), forceCalls.map(target => target.symbol.index));
function rebuiltRefs(body) {
    return body.section.relocations.map(({ offset, symbol, type }) => {
        assert.equal(body.bytes.readUInt32LE(offset), 0);
        if (type === 20) {
            assert.equal(body.bytes[offset - 1], 0xe8);
            if (body === liftBuild && symbol.index === curve.symbol.index) {
                const index = rebuiltCurveCalls.findIndex(item => item.offset === offset);
                assert(index >= 0);
                return { offset, identity: symbol.name + '/' + curveInputs[index][1] };
            }
            assert(body === vectorBuild && forceCalls.some(target => target.symbol.index === symbol.index), symbol.name);
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
        assert(constants.has(literal), symbol.name);
        return { offset, identity: literal };
    });
}

function verifySum(body, terms) {
    const instructions = new Map(body.rows.map(([entry, , instruction]) => [entry, instruction]));
    return terms.map(([name, store, load, offset, popped], index) => {
        assert.equal(instructions.get(hex(store)), 'FSTP double ptr [ESP + 0x' + hex(offset) + ']');
        assert.equal(instructions.get(hex(load)), (index === 0 ? 'FLD' : 'FADD') +
            ' double ptr [ESP + 0x' + hex(offset - popped) + ']');
        return { name, store: hex(store), add: hex(load), poppedBytes: popped };
    });
}

const liftOrder = verifySum(lift, [
    ['pitch rate', 0x20c00c9b, 0x20c00d32, 0x50, 8], ['wing loads', 0x20c00c7e, 0x20c00d36, 0x58, 8],
    ['elevator', 0x20c00ccb, 0x20c00d3a, 0x60, 8], ['spoilers', 0x20c00d01, 0x20c00d3e, 0x30, 8],
    ['alpha rate', 0x20c00d1a, 0x20c00d42, 0x40, 8]
]);
const dragOrder = verifySum(drag, [
    ['induced drag', 0x20c00e0d, 0x20c00f06, 0x58, 0], ['base drag', 0x20c00dea, 0x20c00f0b, 0x48, 4],
    ['supplementary drag', 0x20c00e26, 0x20c00f10, 0x50, 8], ['slipstream drag', 0x20c00e3b, 0x20c00f14, 0x60, 8],
    ['flaps', 0x20c00e56, 0x20c00f18, 0x68, 8], ['spoilers', 0x20c00e86, 0x20c00f1c, 0x70, 8]
]);
const sideOrder = verifySum(side, [
    ['roll rate', 0x20c00faa, 0x20c00fe7, 0x18, 4], ['yaw rate', 0x20c00f8d, 0x20c00fec, 0x20, 8],
    ['rudder', 0x20c00fd4, 0x20c00ff1, 0x10, 12]
]);
for (const address of [0x20c00d46, 0x20c00d5a, 0x20c00f20, 0x20c00ff5]) {
    assert.equal(image.codeAt(address, 2).toString('hex'), 'dec1');
}
assert.equal(image.codeAt(0x20c456be, 4).toString('hex'), 'decadee1');
liftOrder.push({ name: 'tail incidence', add: '20c00d46', source: 'ST1' }, { name: 'lift adjustment', add: '20c00d5a' });
dragOrder.push({ name: 'external tank drag', add: '20c00f20', source: 'ST1' }, { name: 'lift adjustment input at zero gain', add: '20c00f22' });
sideOrder.push({ name: 'sideslip', add: '20c00ff5', source: 'ST1' });

const sections = [
    ['lift', lift, liftBuild, lift.va, lift.va + lift.bytes.length, liftRefs, liftOrder, [
        [0x20c00a90, 0x20c00a96, 'Divide the wing lift coefficient among four wing sections'],
        [0x20c00ab4, 0x20c00aba, 'Publish first outer wing lift'],
        [0x20c00ad2, 0x20c00ad8, 'Publish first inner wing lift'],
        [0x20c00af4, 0x20c00afa, 'Publish second inner wing lift'],
        [0x20c00b02, 0x20c00b08, 'Publish second outer wing lift'],
        [0x20c00bbb, 0x20c00bc3, 'Pass first wing input and call its attenuation curve'],
        [0x20c00bd0, 0x20c00bd8, 'Pass second wing input and call its attenuation curve'],
        [0x20c00ca1, 0x20c00ca7, 'Read elevator deflection'],
        [0x20c00cd1, 0x20c00cd7, 'Read the second spoiler angle'],
        [0x20c00cd9, 0x20c00cdf, 'Add the first spoiler angle'],
        [0x20c00ce4, 0x20c00cea, 'Apply the half-weight for mean spoiler angle'],
        [0x20c00d4e, 0x20c00d54, 'Apply the negative lift adjustment gain']
    ]],
    ['drag', drag, dragBuild, drag.va, drag.va + drag.bytes.length, originalRefs(drag), dragOrder, [
        [0x20c00e11, 0x20c00e15, 'Check the geometry field gating the supplementary term'],
        [0x20c00e5c, 0x20c00e68, 'Add the two spoiler deflections'],
        [0x20c00e6f, 0x20c00e75, 'Apply the half-weight for mean spoiler angle'],
        [0x20c00eb3, 0x20c00eb9, 'Load the external tank drag constant']
    ]],
    ['side', side, sideBuild, side.va, side.va + side.bytes.length, [], sideOrder, [
        [0x20c00fb0, 0x20c00fb6, 'Read the rudder deflection'],
        [0x20c00fda, 0x20c00fe6, 'Scale the sideslip coefficient by sideslip and wing pressure force']
    ]],
    ['vector', producer, vectorBuild, 0x20c4564f, 0x20c456c5, [], [], [
        [0x20c456a5, 0x20c456ab, 'Multiply by the sine of raw alpha'],
        [0x20c456c0, 0x20c456c5, 'Subtract rotated drag and publish body-axis Z']
    ]]
];

const report = { program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), slots, getters };
for (const [name, body, rebuilt, start, end, refs, originalSumOrder, ranges] of sections) {
    const result = compareNativeRanges(body, rebuilt, start, end, refs, rebuiltRefs(rebuilt), ranges);
    const symbol = rebuilt.symbol.name.slice(1).split('@', 1)[0];
    report[name] = { ...result, symbol: 'AirplaneMomentModel::' + symbol, originalSumOrder };
    if (name === 'vector') report[name].parentEntry = '20c45510';
}
fs.writeFileSync(path.join(run, 'aerodynamic-force-sum-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [...getters, ...sections.map(([name]) => report[name])]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} native COFF bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
