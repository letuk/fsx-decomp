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
const original = originalListing(program, '20c47720', image);
const caller = originalListing(program, '20c47420', image);
const call = image.codeAt(0x20c47528, 5);
assert.equal(call[0], 0xe8);
assert.equal(0x20c4752d + call.readInt32LE(1), original.va);

const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);

const groundSelectorName = '?UsesGroundContactLimits@FlightDynamics@sim1@@QBEHXZ';
const groundSelector = coff.byName.has(groundSelectorName) ? {symbol: coff.unique(groundSelectorName)} : null;

function compareScope(begin, end, name, prefix, expected, ranges, callee, callVA) {
    const refs = image.highLowFixups(begin, end - begin).map(relativeOffset => {
        const offset = begin - original.va + relativeOffset;
        const va = original.bytes.readUInt32LE(offset);
        const value = image.codeAt(va, 8, false);
        assert.equal(value.readDoubleLE(0), expected.get(va));
        return {offset, address: va.toString(16), identity: value.toString('hex')};
    });
    assert.equal(refs.length, expected.size);
    const rebuilt = nativeCoffBody(file, coff, dump, prefix);
    const literalIdentities = refs.map(ref => ref.identity);
    const identity = 'sim1.dll 0x20c1cbf0';
    if (callee) {
        const call = image.codeAt(callVA, 5);
        assert.equal(call[0], 0xe8);
        assert.equal(callVA + 5 + call.readInt32LE(1), 0x20c1cbf0);
        refs.push({offset: callVA + 1 - original.va, address: '20c1cbf0', identity});
    }
    const rebuiltRefs = nativeCoffReferences(file, coff, rebuilt, callee, identity, literalIdentities);
    if (callee) assert(rebuiltRefs.some(ref => ref.symbol === callee.symbol.name));
    return {symbol: name, ...compareNativeRanges(original, rebuilt, begin, end, refs, rebuiltRefs, ranges)};
}

const advance = compareScope(0x20c4772e, 0x20c477e3, 'PropellerStepInputs::UpdateAdvanceRatio',
    '?UpdateAdvanceRatio@PropellerStepInputs@sim1@@',
    new Map([[0x20bc3690, 6.283185307179586], [0x20bc3590, 0.0001], [0x20bc3388, 0.25],
        [0x20bb3cf8, 0.125], [0x20bc3a38, 8.0], [0x20bc4288, 999.0]]), [
        [0x20c47730, 0x20c47736, 'Convert angular velocity to revolutions per second'],
        [0x20c47769, 0x20c47776, 'Compare absolute rotational speed with the near-stop threshold'],
        [0x20c477a1, 0x20c477b0, 'Compare absolute normalized speed with the attenuation limit'],
        [0x20c477b2, 0x20c477c5, 'Build and test the low-speed attenuation ramp'],
        [0x20c477cb, 0x20c477cd, 'Multiply the advance ratio by the selected gain'],
        [0x20c477d4, 0x20c477da, 'Load the near-stop advance-ratio sentinel']
    ]);
const fixed = compareScope(0x20c47a2f, 0x20c47ad1, 'PropellerForceRecord::UpdateFixedPitch',
    '?UpdateFixedPitch@PropellerForceRecord@sim1@@', new Map([[0x20bbfc48, 5.0]]), [
        [0x20c47a4b, 0x20c47a50, 'Read the comparison result for the configured fixed pitch'],
        [0x20c47a5d, 0x20c47a62, 'Check for ordered axial speed above the cruise threshold'],
        [0x20c47a77, 0x20c47a7c, 'Read the comparison result for cruise pitch'],
        [0x20c47a81, 0x20c47a87, 'Load the low-speed reset threshold'],
        [0x20c47a8d, 0x20c47a92, 'Check whether the airplane speed input is below five'],
        [0x20c47a97, 0x20c47a9c, 'Check whether the normalized propeller speed is zero']
    ]);
const governor = compareScope(0x20c47879, 0x20c47a2f, 'PropellerForceRecord::ApplyGovernorPitch',
    '?ApplyGovernorPitch@PropellerForceRecord@sim1@@',
    new Map([[0x20bc35a8, 0.1047197551], [0x20bc3590, 0.0001], [0x20bbf718, -0.2],
        [0x20bc4280, 0.0010000000474974513], [0x20bc4278, 0.007]]), [
        [0x20c478bf, 0x20c478c1, 'Interpolate between minimum and reference governed speed'],
        [0x20c478c7, 0x20c478cd, 'Convert the governor RPM target to angular speed'],
        [0x20c478df, 0x20c478ee, 'Compare the pitch difference with the feather tolerance'],
        [0x20c4790a, 0x20c47910, 'Load the rate for commanded reverse pitch'],
        [0x20c47941, 0x20c47947, 'Apply the defeathering accumulator increment'],
        [0x20c47962, 0x20c47968, 'Load the ground-limit control threshold'],
        [0x20c4797b, 0x20c47980, 'Call the ground contact selector'],
        [0x20c479c8, 0x20c479cc, 'Divide the pitch span by the reversed control scale'],
        [0x20c479e9, 0x20c479ee, 'Evaluate whether integrated pitch is above the upper limit'],
        [0x20c47a0c, 0x20c47a11, 'Evaluate whether the descending pitch has reached its lower limit']
    ], groundSelector, 0x20c4797b);

const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    originalEntry: original.va.toString(16), caller: {entry: caller.va.toString(16), call: '20c47528'}, scopes: [advance, governor, fixed]};
fs.writeFileSync(path.join(run, 'propeller-pitch-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const scope of report.scopes) {
    console.log(`${scope.entry} ${scope.status}: ${scope.originalBytes} original / ${scope.rebuiltBytes} native COFF bytes`);
    for (const range of scope.scopes) console.log(`${range.entry} ${range.status}: ${range.bytes} bytes, ${range.name}`);
}
