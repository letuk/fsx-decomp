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
const original = originalListing(program, '20c47420', image);
const begin = 0x20c47456;
const end = original.va + original.bytes.length;
const hex = value => value.toString(16);

function callTarget(va) {
    const call = image.codeAt(va, 5);
    assert.equal(call[0], 0xe8);
    return va + 5 + call.readInt32LE(1);
}

assert.equal(callTarget(0x20c204ac), original.va);
const preparations = [[0x20c47441, 0x20c48270], [0x20c47451, 0x20c48710]];
assert(preparations.every(([call, target]) => call < begin && callTarget(call) === target));

const expected = new Map([[0x20bc2a90, 1.0], [0x20bc4290, 0.7853981633974483], [0x20bc35a8, 0.1047197551]]);
const refs = image.highLowFixups(begin, end - begin).map(relativeOffset => {
    const offset = begin - original.va + relativeOffset;
    const va = original.bytes.readUInt32LE(offset);
    const bytes = image.codeAt(va, 8, false);
    assert.equal(bytes.readDoubleLE(0), expected.get(va));
    return {offset, address: hex(va), identity: bytes.toString('hex')};
});
assert.equal(refs.length, expected.size);
const literalIdentities = refs.map(ref => ref.identity);

const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const symbol = 'FlightDynamics::UpdateMappedPropellersAfterPreparation';
const rebuilt = nativeCoffBody(file, coff, dump, '?UpdateMappedPropellersAfterPreparation@FlightDynamics@sim1@@');
const targets = [
    [0x20c47528, 0x20c47720, '?UpdatePropellerBladePitch@FlightDynamics@sim1@@', 'Update blade pitch and advance ratio'],
    [0x20c47536, 0x20c47ae0, '?CalculateAerodynamicTorque@PropellerForceRecord@sim1@@', 'Calculate aerodynamic torque'],
    [0x20c475bd, 0x20c47ba0, '?IntegrateSpeed@PropellerForceRecord@sim1@@', 'Integrate propeller speed'],
    [0x20c47698, 0x20c47c90, '?UpdateThrust@PropellerForceRecord@sim1@@', 'Publish propeller thrust'],
    [0x20c476c3, 0x20c48150, '?UpdateSynchronization@PropellerForceRecord@sim1@@', 'Update synchronization correction'],
    [0x20c476e1, 0x20c480b0, '?UpdateInducedAirSpeed@EngineForceRecord@sim1@@', 'Publish induced airspeed to the mapped engine']
].map(([call, va, prefix, name]) => {
    assert.equal(callTarget(call), va);
    const callee = nativeCoffBody(file, coff, dump, prefix);
    const identity = program + ' 0x' + hex(va);
    refs.push({offset: call + 1 - original.va, address: hex(va), identity});
    return {call, va, callee, name, identity};
});

const calls = rebuilt.section.relocations.filter(relocation => relocation.type === 20);
assert.deepEqual(calls.map(relocation => relocation.symbol.name), targets.map(target => target.callee.symbol.name));
const rebuiltRefs = rebuilt.section.relocations.flatMap(relocation => {
    const target = targets.find(candidate => candidate.callee.symbol.index === relocation.symbol.index);
    return nativeCoffReferences(file, coff, {...rebuilt, section: {...rebuilt.section, relocations: [relocation]}},
        target?.callee ?? null, target?.identity ?? '', literalIdentities);
});

const namedRanges = [
    [0x20c474d0, 0x20c474d6, 'Add one to the mapped propeller count'],
    [0x20c474ed, 0x20c474f3, 'Apply the disk-area factor'],
    [0x20c47508, 0x20c4750e, 'Convert the RPM reference to angular speed'],
    [0x20c475b6, 0x20c475bd, 'Store the step and divided torque as stack arguments'],
    [0x20c4760b, 0x20c4760e, 'Read the last mapped propeller speed fraction'],
    [0x20c47613, 0x20c4761b, 'Check the rotation factor before publishing engine speed'],
    [0x20c4761d, 0x20c47621, 'Negate the fraction for negative propeller rotation'],
    [0x20c47628, 0x20c4762e, 'Load the runtime RPM reference for engine speed feedback']
];
const ranges = [...namedRanges, ...targets.map(({call, name}) => [call, call + 5, name])].sort((a, b) => a[0] - b[0]);
const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), symbol,
    originalEntry: hex(original.va), caller: {entry: '20c20430', call: '20c204ac'},
    preparationCalls: preparations.map(([call, target]) => ({call: hex(call), target: hex(target)})),
    mappedCalls: targets.map(({call, va, name}, i) => ({originalCall: hex(call), target: hex(va), name,
        rebuiltCall: hex(calls[i].offset - 1), symbol: calls[i].symbol.name})),
    ...compareNativeRanges(original, rebuilt, begin, end, refs, rebuiltRefs, ranges)};
fs.writeFileSync(path.join(run, 'propeller-mapped-pass-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${report.entry} ${report.status}: ${report.originalBytes} original / ${report.rebuiltBytes} native COFF bytes`);
for (const scope of report.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
