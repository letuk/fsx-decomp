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
const read = name => nativeCoffBody(file, coff, listing, '?' + name + '@');
const hex = number => number.toString(16);
const parent = originalListing(program, '20c020a0', image);
const yaw = read('CalculateYawMoment');
const curve = read('SampleAerodynamicCurve');
const gyro = read('CalculatePropellerYawGyro');
const pFactor = read('CalculatePropellerPFactorYaw');
const tank = read('CalculateExternalTankYaw');
const simple = read('CalculateSimpleYawMoment');
const forceTransfer = read('CalculateForceMomentY');

function verifySlot(table, offset, target) {
    assert.deepEqual(image.highLowFixups(table + offset, 4), [0]);
    assert.equal(image.initialDword(table + offset), target);
}

assert.equal(image.codeAt(0x20c4f356, 6).toString('hex'), 'c7061826bc20');
verifySlot(0x20bc2160, 0x1c, 0x20c020a0);
verifySlot(0x20bc2160, 0x20, 0x20c14e50);
verifySlot(0x20bc213c, 0x10, 0x20c454c0);
verifySlot(0x20bc213c, 0x20, 0x20c45500);
assert.equal(image.codeAt(0x20c14e50, 4).toString('hex'), '8b410cc3');
assert.equal(image.codeAt(0x20c454c0, 7).toString('hex'), 'd981f8130000c3');
const slots = [
    [0xa4, 0x20c50a40, 0x20c021d2, 'Roll-rate cross coupling'],
    [0xa8, 0x20c50c90, 0x20c0210b, 'Yaw rate'],
    [0xac, 0x20c50910, 0x20c0218b, 'Rudder'],
    [0xb0, 0x20c50b80, 0x20c021f2, 'Sideslip'],
    [0xb4, 0x20c50d10, 0x20c02235, 'Ailerons'],
    [0xb8, 0x20c50dc0, 0x20c025df, 'Slipstream rudder'],
    [0xbc, 0x20c50de0, 0x20c02600, 'Slipstream rate cross coupling'],
    [0xc0, 0x20c50e00, 0x20c021bc, 'Rudder trim'],
    [0xc4, 0x20c50e20, 0x20c020f5, 'Additional yaw']
].map(([slot, target, load, term]) => {
    verifySlot(0x20bc2618, slot, target);
    assert.equal(image.codeAt(load + 2, 4).readUInt32LE(0), slot);
    return { slot: hex(slot), entry: hex(0x20bc2618 + slot), target: hex(target), load: hex(load), term };
});
const getters = [
    ['20c50dc0', 'GetSlipstreamRudderYawCoefficient'],
    ['20c50de0', 'GetSlipstreamRollRateYawCoefficient'],
    ['20c50e00', 'GetRudderTrimYawCoefficient']
].map(([entry, symbol]) => {
    const original = originalListing(program, entry, image);
    const rebuilt = read(symbol);
    assert.equal(rebuilt.section.relocations.length, 0);
    assert.equal(image.highLowFixups(original.va, original.bytes.length).length, 0);
    return { ...compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, [], [], []),
        symbol: 'AirplaneCoefficientModel::' + symbol };
});

const globals = new Map([
    [0x20c7b948, coff.byName.has('?flapGroupYawCoefficient@sim1@@3NA') ? '?flapGroupYawCoefficient@sim1@@3NA' : '?slipstreamYawCoefficient@sim1@@3NA'],
    [0x20c7b92c, '?highAlphaReferenceXShift@sim1@@3NA'],
    [0x20c7b93c, '?highAlphaReferenceZShift@sim1@@3NA']
]);
const literals = new Map([
    [0x20bb4650, 0.5], [0x20bc35a8, 0.1047197551], [0x20bc36d8, 0.05235987755982989],
    [0x20bc4888, 0.1875], [0x20bb4050, 20], [0x20bbeae0, 60], [0x20bc39c8, 40],
    [0x20bc3ca8, 12], [0x20bb46b0, 0.047], [0x20bbf6d0, 0], [0x20bc00e0, 0.75]
]);
function doubleHex(value) {
    const bytes = Buffer.alloc(8);
    bytes.writeDoubleLE(value);
    return bytes.toString('hex');
}

const curveSamples = [[0, 1], [0.8, 1.06], [1, 1.07], [1.4, 1.08], [1.8, 1.02], [3.71, 0.45]];
const curveBytes = image.codeAt(0x20c71d10, 100, false);
assert.equal(curveBytes.readUInt32LE(0), curveSamples.length);
assert.equal(curveBytes.subarray(4).toString('hex'), curveSamples.flat().map(doubleHex).join(''));
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
assert.equal(image.codeAt(0x20c0215d, 1)[0], 0xe8);
assert.equal(0x20c02162 + image.codeAt(0x20c0215e, 4).readInt32LE(0), 0x20bcbf60);
originalRefs.push({ offset: 0x20c0215e - parent.va, address: '20bcbf60', identity: curve.symbol.name });
const constants = new Set([...curveSamples.flat(), ...literals.values()].map(doubleHex));
const direct = [curve, gyro, pFactor, forceTransfer, tank, simple];
function rebuildRefs(body) {
    return body.section.relocations.map(({ offset, symbol, type }) => {
        assert.equal(body.bytes.readUInt32LE(offset), 0);
        if (type === 20) {
            assert.equal(body.bytes[offset - 1], 0xe8);
            assert(direct.some(target => target.symbol.index === symbol.index), symbol.name);
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

const instructions = new Map(parent.rows.map(([address, , instruction]) => [address, instruction]));
const terms = [
    ['rudder', 0x20c021ae, 0x20c02721, 0x48, 0],
    ['yaw rate', 0x20c0212d, 0x20c02726, 0x38, 4],
    ['rudder trim', 0x20c021e4, 0x20c0272c, 0x60, 12],
    ['roll rate cross coupling', 0x20c02200, 0x20c02730, 0x50, 12],
    ['sideslip', 0x20c02221, 0x20c02734, 0x58, 12],
    ['ailerons', 0x20c02261, 0x20c02738, 0x68, 12],
    ['additional yaw', 0x20c02113, 0x20c0273c, 0x70, 12],
    ['slipstream', 0x20c020fd, 0x20c02740, 0x78, 12],
    ['propeller gyro', 0x20c023bd, 0x20c02744, 0x20, 12],
    ['propeller P-factor', 0x20c025d3, 0x20c02748, 0x28, 12],
    ['force reference transfer', 0x20c0267c, 0x20c0274c, 0x30, 12],
    ['slipstream tail', 0x20c02641, 0x20c02752, 0x18, 12]
].map(([name, store, load, offset, poppedBytes], index) => {
    assert.equal(instructions.get(hex(store)), 'FSTP double ptr [ESP + 0x' + hex(offset) + ']');
    const operand = ' double ptr [ESP + 0x' + hex(offset - poppedBytes) + ']';
    assert.equal(instructions.get(hex(load)), (index === 0 ? 'FLD' : 'FADD') + operand);
    return { name, store: hex(store), add: hex(load), poppedBytes };
});
assert.equal(instructions.get('20c02750'), 'FADDP');
terms.splice(11, 0, { name: 'external fuel tanks', add: '20c02750', source: 'ST1' });
const sections = [
    ['moment', yaw, 0x20c020a0, 0x20c0275c, [
        [0x20c020ef, 0x20c020f5, 'Multiply by the published flap-group yaw coefficient'],
        [0x20c0215a, 0x20c02162, 'Pass relative alpha and call the high-angle damping curve'],
        [0x20c0217f, 0x20c02185, 'Read the rudder deflection'],
        [0x20c021b4, 0x20c021ba, 'Read the binary32 rudder trim setting'],
        [0x20c021ca, 0x20c021d0, 'Apply the rudder trim effectiveness factor'],
        [0x20c0223d, 0x20c02243, 'Average the two aileron angles'],
        [0x20c025f6, 0x20c025fc, 'Apply the slipstream rudder area/span factor']
    ]],
    ['gyro', gyro, 0x20c02265, 0x20c023cf, [
        [0x20c0229f, 0x20c022a5, 'Load the signed RPM conversion constant'],
        [0x20c022e6, 0x20c022ec, 'Multiply by the pitch angular velocity']
    ]],
    ['pFactor', pFactor, 0x20c023cf, 0x20c025dd, [
        [0x20c0241c, 0x20c02422, 'Subtract the three-degree alpha offset'],
        [0x20c02476, 0x20c0247c, 'Multiply the propeller diameter by 0.1875'],
        [0x20c0258f, 0x20c02595, 'Load the lower forward-speed breakpoint'],
        [0x20c025b2, 0x20c025b8, 'Load the upper forward-speed breakpoint'],
        [0x20c025cb, 0x20c025d3, 'Divide by the 40-unit interval and scale the P-factor moment']
    ]],
    ['tank', tank, 0x20c02680, 0x20c026f4, [
        [0x20c0269b, 0x20c026a4, 'Load the tank drag coefficient and test the capacity comparison']
    ]],
    ['simple', simple, 0x20c02702, 0x20c02718, [
        [0x20c02706, 0x20c0270c, 'Apply the SimpleAero rudder weight']
    ]]
];
const report = { program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), slots, getters };
for (const [name, body, start, end, ranges] of sections) {
    const result = compareNativeRanges(parent, body, start, end, originalRefs, rebuildRefs(body), ranges);
    const symbol = body.symbol.name.slice(1).split('@', 1)[0];
    report[name] = { ...result, symbol: 'AirplaneMomentModel::' + symbol };
    if (name === 'moment') report[name].originalSumOrder = terms;
    else report[name].parentEntry = '20c020a0';
}
fs.writeFileSync(path.join(run, 'yaw-sum-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [...getters, ...sections.map(([name]) => report[name])]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} native COFF bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
