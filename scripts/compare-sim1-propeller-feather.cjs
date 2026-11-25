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
const original = originalListing(program, '20c485c0', image);
const caller = originalListing(program, '20c47720', image);
const call = image.codeAt(0x20c47874, 5);
assert.equal(call[0], 0xe8);
assert.equal(0x20c47879 + call.readInt32LE(1), original.va);

const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const symbol = 'PropellerForceRecord::UpdateFeatherState';
const rebuilt = nativeCoffBody(file, coff, dump, '?UpdateFeatherState@PropellerForceRecord@sim1@@');
const rebuiltLatch = nativeCoffBody(file, coff, dump, '?UpdateGovernorFeatherState@PropellerForceRecord@sim1@@');

function literalReferences(scope, begin, end, expected) {
    const refs = image.highLowFixups(begin, end - begin).map(relativeOffset => {
        const offset = begin - scope.va + relativeOffset;
        const va = scope.bytes.readUInt32LE(offset);
        const value = image.codeAt(va, 8, false);
        assert.equal(value.readDoubleLE(0), expected.get(va));
        return {offset, address: va.toString(16), identity: value.toString('hex')};
    });
    assert.deepEqual(new Set(refs.map(ref => Number.parseInt(ref.address, 16))), new Set(expected.keys()));
    return refs;
}

const refs = literalReferences(original, original.va, original.va + original.bytes.length,
    new Map([[0x20bc3530, 0.10000000149011612], [0x20bb4650, 0.5], [0x20bc3590, 0.0001], [0x20bbf6d0, 0.0]]));
const rebuiltRefs = nativeCoffReferences(file, coff, rebuilt, null, '', refs.map(ref => ref.identity));
const feather = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, refs, rebuiltRefs, [
    [0x20c485c0, 0x20c485c5, 'Hotpatch entry and frame setup'],
    [0x20c485ce, 0x20c485d3, 'Check the timer comparison flags'],
    [0x20c485d5, 0x20c485d8, 'Load the float feather request timer'],
    [0x20c485e7, 0x20c485f3, 'Store remaining request time and test it against zero'],
    [0x20c48600, 0x20c48603, 'Publish the clamped feather request timer'],
    [0x20c48607, 0x20c4860e, 'Compare the engine type with five'],
    [0x20c4860e, 0x20c48611, 'Set the automatic-feather engine-type flag'],
    [0x20c48611, 0x20c48615, 'Check whether feathering is already active'],
    [0x20c48621, 0x20c48627, 'Load the normalized torque threshold for activation'],
    [0x20c48630, 0x20c48635, 'Check whether torque is below the threshold'],
    [0x20c48637, 0x20c4863b, 'Read the automatic feather arm flag'],
    [0x20c4863d, 0x20c48644, 'Set the feather state'],
    [0x20c4864f, 0x20c48657, 'Check the remaining feather request time'],
    [0x20c48666, 0x20c48673, 'Compare absolute normalized speed with the request threshold'],
    [0x20c48697, 0x20c4869d, 'Load the normalized torque threshold for clearing'],
    [0x20c486bf, 0x20c486c7, 'Load the feather angle tolerance'],
    [0x20c486e9, 0x20c486f2, 'Check that absolute normalized speed is below the feather-stop tolerance'],
    [0x20c486f9, 0x20c48703, 'Clear and return the feather state']
]);

const start = 0x20c47818;
const end = 0x20c47879;
const latchRefs = literalReferences(caller, start, end, new Map([[0x20bc3a78, 9.549296586]]));
const literalIdentities = latchRefs.map(ref => ref.identity);
latchRefs.push({offset: 0x20c47875 - caller.va, address: original.va.toString(16), identity: symbol});
const nativeLatchRefs = nativeCoffReferences(file, coff, rebuiltLatch, rebuilt, symbol, literalIdentities);
assert(nativeLatchRefs.some(ref => ref.symbol === rebuilt.symbol.name));
const latch = compareNativeRanges(caller, rebuiltLatch, start, end, latchRefs, nativeLatchRefs, [
    [0x20c4781e, 0x20c47824, 'Convert angular speed to RPM for the latch'],
    [0x20c47836, 0x20c4783b, 'Read the flags from the positive-command check'],
    [0x20c47866, 0x20c47869, 'Load the time step passed to the feather helper'],
    [0x20c47874, 0x20c47879, 'Call the feather state routine']
]);

const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    caller: {entry: caller.va.toString(16), call: '20c47874'},
    functions: [{symbol, ...feather}, {symbol: 'PropellerForceRecord::UpdateGovernorFeatherState', ...latch}]};
fs.writeFileSync(path.join(run, 'propeller-feather-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const result of report.functions) {
    console.log(`${result.entry} ${result.status}: ${result.originalBytes} original / ${result.rebuiltBytes} native COFF bytes`);
    for (const scope of result.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
