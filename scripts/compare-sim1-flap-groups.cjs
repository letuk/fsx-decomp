const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {openImage} = require('./lib/pe32-dll.cjs');
const {parseNativeCoff} = require('./lib/coff.cjs');
const {originalListing, nativeCoffBody, compareNativeRanges} = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 aerodynamic moment run directory');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const parent = originalListing(program, '20c00840', image);
const file = fs.readFileSync(path.join(run, 'sim1-aerodynamic-moment.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const read = symbol => nativeCoffBody(file, coff, dump, '?' + symbol + '@');
const rebuilt = read('UpdateFlapGroupCoefficients');
const call = image.codeAt(0x20c45562, 5);
assert.equal(call[0], 0xe8);
assert.equal(0x20c45567 + call.readInt32LE(1), parent.va);
assert.equal(image.codeAt(0x20c4f356, 6).toString('hex'), 'c7061826bc20');
const slots = [
    [0x18, '20c4f620', 'GetFlapGroupLiftCoefficient', 0x20c00861, 0x24],
    [0x34, '20c4fa70', 'GetFlapGroupDragCoefficient', 0x20c00870, 0x3c],
    [0x70, '20c50270', 'GetPitchCoefficientSlot70', 0x20c00882, 0x84]
];
const getters = slots.map(([slot, entry, name, load, field]) => {
    const address = 0x20bc2618 + slot;
    assert.deepEqual(image.highLowFixups(address, 4), [0]);
    assert.equal(image.initialDword(address), parseInt(entry, 16));
    assert.equal(image.codeAt(load + 2, 1)[0], slot);
    const original = originalListing(program, entry, image);
    const object = read(name);
    assert.equal(object.section.relocations.length, 0);
    assert.deepEqual(image.highLowFixups(original.va, original.bytes.length), []);
    assert.equal(original.rows.at(-2)[2], 'FLD double ptr [EAX + 0x' + field.toString(16) + ']');
    return {symbol: 'AirplaneCoefficientModel::' + name, slot, field,
        ...compareNativeRanges(original, object, original.va, original.va + original.bytes.length, [], [], [])};
});
const globals = new Map([
    [0x20c7b948, 'flapGroupYawCoefficient'], [0x20c7b950, 'flapGroupRollCoefficient'],
    [0x20c7b958, 'flapGroupPitchCoefficient'], [0x20c7b960, 'firstWingFlapDrag'],
    [0x20c7b968, 'secondWingFlapDrag'], [0x20c7b970, 'firstInnerWingFlapLift'], [0x20c7b978, 'secondInnerWingFlapLift']
].map(([address, symbol]) => [address, '?' + symbol + '@sim1@@3NA']));
const half = '000000000000e03f';
assert.equal(image.codeAt(0x20bb4650, 8, false).toString('hex'), half);
const originalRefs = image.highLowFixups(parent.va, parent.bytes.length).map(offset => {
    const address = parent.bytes.readUInt32LE(offset);
    const identity = address === 0x20bb4650 ? half : globals.get(address);
    assert(identity, address.toString(16));
    return {offset, address: address.toString(16), identity};
});
const rebuiltRefs = rebuilt.section.relocations.map(({offset, symbol, type}) => {
    assert.equal(type, 6);
    assert.equal(rebuilt.bytes.readUInt32LE(offset), 0);
    if ([...globals.values()].includes(symbol.name)) {
        assert.equal(symbol.section, 0);
        return {offset, identity: symbol.name};
    }
    const section = coff.sections[symbol.section - 1];
    assert(section && symbol.value + 8 <= section.size, symbol.name);
    const literal = file.subarray(section.raw + symbol.value, section.raw + symbol.value + 8).toString('hex');
    assert.equal(literal, half);
    return {offset, identity: literal};
});
for (const symbol of globals.values()) assert(rebuiltRefs.some(ref => ref.identity === symbol), symbol);
const comparison = compareNativeRanges(parent, rebuilt, parent.va, parent.va + parent.bytes.length, originalRefs, rebuiltRefs, [
    [0x20c00868, 0x20c0086e, 'Apply a half-weight to the base coefficient'],
    [0x20c0089b, 0x20c008ad, 'Clear both lift coefficients and first-side drag'],
    [0x20c008b5, 0x20c008bb, 'Read the runtime flap-group list'],
    [0x20c00901, 0x20c0090d, 'Accumulate and publish first-side lift'],
    [0x20c00919, 0x20c00925, 'Accumulate and publish second-side lift'],
    [0x20c00941, 0x20c0094d, 'Accumulate and publish first-side drag'],
    [0x20c0096b, 0x20c00971, 'Publish second-side drag'],
    [0x20c0099b, 0x20c009a7, 'Subtract the running side lift coefficients'],
    [0x20c009d9, 0x20c009df, 'Publish flap-group yaw'],
    [0x20c009df, 0x20c009e5, 'Publish flap-group roll'],
    [0x20c009e5, 0x20c009eb, 'Publish flap-group pitch']
]);
const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), getters,
    update: {symbol: 'AirplaneMomentModel::UpdateFlapGroupCoefficients', ...comparison}};
fs.writeFileSync(path.join(run, 'flap-groups-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const record of [...getters, report.update]) {
    console.log(`${record.entry} ${record.status}: ${record.originalBytes} original / ${record.rebuiltBytes} native COFF bytes`);
    for (const scope of record.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
