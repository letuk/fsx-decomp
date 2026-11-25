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
const original = originalListing(program, '20c00530', image);
const file = fs.readFileSync(path.join(run, 'sim1-aerodynamic-moment.obj'));
const coff = parseNativeCoff(file);
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const rebuilt = nativeCoffBody(file, coff, dump, '?UpdateHighAlphaReferenceShifts@AirplaneDynamics@sim1@@');
const curve = nativeCoffBody(file, coff, dump, '?SampleAerodynamicCurve@sim1@@');
const begin = 0x20c005f3;
const end = 0x20c00775;
const literals = new Map([
    [0x20bbf7f0, 0.017453292519943295], [0x20bb4650, 0.5], [0x20bc3b58, 0.83],
    [0x20bc39f0, 0.73], [0x20bc3b90, 1.25], [0x20bc3880, 1.4],
    [0x20bc4898, -0.08], [0x20bc4890, -0.26], [0x20bbf6c8, -1.0]
]);
const globals = new Map([
    [0x20c7b92c, '?highAlphaReferenceXShift@sim1@@3NA'],
    [0x20c7b934, '?highAlphaReferenceMiddleShift@sim1@@3NA'],
    [0x20c7b93c, '?highAlphaReferenceZShift@sim1@@3NA']
]);
const originalRefs = image.highLowFixups(begin, end - begin).map(relative => {
    const offset = begin - original.va + relative;
    const address = original.bytes.readUInt32LE(offset);
    let identity = globals.get(address);
    if (literals.has(address)) {
        const bytes = image.codeAt(address, 8, false);
        assert.equal(bytes.readDoubleLE(0), literals.get(address));
        identity = bytes.toString('hex');
    }
    assert(identity, address.toString(16));
    return {offset, address: address.toString(16), identity};
});
const axes = ['longitudinal', 'vertical'];
const originalCalls = [0x20c00671, 0x20c006d5];
for (const [index, address] of originalCalls.entries()) {
    const bytes = image.codeAt(address, 5);
    assert.equal(bytes[0], 0xe8);
    assert.equal(address + 5 + bytes.readInt32LE(1), 0x20bcbf60);
    originalRefs.push({offset: address + 1 - original.va, address: '20bcbf60', identity: axes[index]});
}
const calls = rebuilt.section.relocations.filter(r => r.type === 20);
assert.equal(calls.length, 2);
for (const [index, target] of [0x20c7b92c, 0x20c7b93c].entries()) {
    const call = calls[index];
    const store = rebuilt.section.relocations.find(item => item.offset === call.offset + 6);
    assert.equal(store?.symbol.name, globals.get(target));
    assert.equal(rebuilt.bytes.readUInt16LE(store.offset - 2), 0x1ddd);
}
const rebuiltRefs = rebuilt.section.relocations.map(({offset, symbol, type}) => {
    assert.equal(rebuilt.bytes.readUInt32LE(offset), 0);
    if (type === 20) {
        assert.equal(rebuilt.bytes[offset - 1], 0xe8);
        assert.equal(symbol.index, curve.symbol.index);
        return {offset, identity: axes[calls.findIndex(r => r.offset === offset)]};
    }
    assert.equal(type, 6);
    if ([...globals.values()].includes(symbol.name)) {
        assert.equal(symbol.section, 0);
        return {offset, identity: symbol.name};
    }
    const section = coff.sections[symbol.section - 1];
    assert(section && symbol.value + 8 <= section.size, symbol.name);
    const bytes = file.subarray(section.raw + symbol.value, section.raw + symbol.value + 8);
    assert([...literals.values(), 0, 1].includes(bytes.readDoubleLE(0)), symbol.name);
    return {offset, identity: bytes.toString('hex')};
});
const comparison = compareNativeRanges(original, rebuilt, begin, end, originalRefs, rebuiltRefs, [
    [0x20c00609, 0x20c0060f, 'Convert filtered alpha from degrees to radians'],
    [0x20c00642, 0x20c00648, 'Load the common 0.83 breakpoint'],
    [0x20c0066e, 0x20c00676, 'Pass relative alpha to the longitudinal curve'],
    [0x20c00676, 0x20c0067c, 'Store the unscaled longitudinal reference shift'],
    [0x20c006a6, 0x20c006ac, 'Load the vertical ordinate at 0.83'],
    [0x20c006b4, 0x20c006ba, 'Load the vertical ordinate at 1.0'],
    [0x20c006d2, 0x20c006da, 'Pass relative alpha to the vertical curve'],
    [0x20c006da, 0x20c006e0, 'Store the unscaled vertical reference shift'],
    [0x20c006ea, 0x20c006f0, 'Clear the middle reference component'],
    [0x20c0070d, 0x20c00712, 'Check the x87 rudder-limit comparison status'],
    [0x20c0071c, 0x20c00722, 'Divide rudder deflection by its limit'],
    [0x20c00749, 0x20c00755, 'Scale and publish the vertical correction']
]);
const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    parentEntry: '20c00530', symbol: 'AirplaneDynamics::UpdateHighAlphaReferenceShifts', ...comparison};
fs.writeFileSync(path.join(run, 'high-alpha-reference-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${comparison.entry} ${comparison.status}: ${comparison.originalBytes} original / ${comparison.rebuiltBytes} native COFF bytes`);
for (const scope of comparison.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
