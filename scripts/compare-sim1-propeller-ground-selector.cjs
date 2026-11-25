const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {openImage} = require('./lib/pe32-dll.cjs');
const {parseNativeCoff} = require('./lib/coff.cjs');
const {originalListing, nativeCoffBody, compareNativeRanges} = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 Turbine run directory');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const original = originalListing(program, '20c1cbf0', image);
const pitch = originalListing(program, '20c47720', image);
const call = image.codeAt(0x20c4797b, 5);
assert.equal(call[0], 0xe8);
assert.equal(0x20c47980 + call.readInt32LE(1), original.va);

const vtable = 0x20bbd938;
const modeTarget = 0x20bceed0;
assert.equal(image.codeAt(vtable + 0xc, 4, false).readUInt32LE(), modeTarget);
assert.deepEqual(image.highLowFixups(vtable + 0xc, 4), [0]);
assert.equal(image.codeAt(modeTarget, 4).toString('hex'), '8b411cc3');
assert.equal(image.codeAt(0x20c1cc0e, 10).toString('hex'), '8b49148b018b500cffd2');

const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const selectorSymbol = 'FlightDynamics::UsesGroundContactLimits';
const rebuilt = nativeCoffBody(file, coff, dump, '?UsesGroundContactLimits@FlightDynamics@sim1@@');
assert.equal(rebuilt.section.relocations.length, 0);
assert.deepEqual(image.highLowFixups(original.va, original.bytes.length), []);
const selector = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, [], [], [
    [0x20c1cbf0, 0x20c1cbfd, 'Load flight state and test the qualified ground-contact count'],
    [0x20c1cbff, 0x20c1cc18, 'Test the flap factor and dispatch surface mode through slot +0x0c'],
    [0x20c1cc18, 0x20c1cc1b, 'Compare the returned mode with two'],
    [0x20c1cc23, 0x20c1cc26, 'Return zero when ground limits are ineligible']
]);

const pitchSymbol = 'FlightDynamics::UpdatePropellerBladePitch';
const rebuiltPitch = nativeCoffBody(file, coff, dump, '?UpdatePropellerBladePitch@FlightDynamics@sim1@@');
const stagePatterns = [
    /^\?UpdateAdvanceRatio@PropellerStepInputs@sim1@@/,
    /^\?UpdateGovernorFeatherState@PropellerForceRecord@sim1@@/,
    /^\?ApplyGovernorPitch@PropellerForceRecord@sim1@@/,
    /^\?UpdateFixedPitch@PropellerForceRecord@sim1@@/
];
const targets = rebuiltPitch.section.relocations;
assert.equal(targets.length, stagePatterns.length);
const rebuiltStages = targets.map(({offset, symbol, type}, index) => {
    assert.equal(type, 20);
    assert.match(symbol.name, stagePatterns[index]);
    assert(symbol.section > 0 && coff.sections[symbol.section - 1].name === '.text');
    assert.equal(rebuiltPitch.bytes[offset - 1], 0xe8);
    assert.equal(rebuiltPitch.bytes.readUInt32LE(offset), 0);
    return {offset, symbol: symbol.name, identity: symbol.name};
});
const dispatch = compareNativeRanges(pitch, rebuiltPitch, pitch.va, pitch.va + pitch.bytes.length, [], rebuiltStages, [
    [0x20c47720, 0x20c47725, 'Hotpatch entry and frame setup'],
    [0x20c47a18, 0x20c47a21, 'Restore preserved registers and remove the original argument bytes']
]);

const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    dispatchSlot: {table: vtable.toString(16), offset: '0x0c', target: modeTarget.toString(16), targetBytes: '8b411cc3'},
    stages: rebuiltStages, functions: [{symbol: selectorSymbol, ...selector}, {symbol: pitchSymbol, ...dispatch}]};
fs.writeFileSync(path.join(run, 'propeller-ground-selector-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const result of report.functions) {
    console.log(`${result.entry} ${result.status}: ${result.originalBytes} original / ${result.rebuiltBytes} native COFF bytes`);
    for (const scope of result.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
