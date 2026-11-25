const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {openImage} = require('./lib/pe32-dll.cjs');
const {parseNativeCoff} = require('./lib/coff.cjs');
const {originalListing, nativeCoffBody, nativeCoffReferences, compareNativeRanges} = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 aerodynamic moment run directory');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const original = originalListing(program, '20c45760', image);
const call = image.codeAt(0x20c45541, 5);
assert.equal(call[0], 0xe8);
assert.equal(0x20c45546 + call.readInt32LE(1), original.va);
const file = fs.readFileSync(path.join(run, 'sim1-aerodynamic-moment.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const rebuilt = nativeCoffBody(file, coff, dump, '?UpdatePropellerSlipstream@AirplaneDynamics@sim1@@');
const expected = new Map([[0x20bc3558, 4294967296.0], [0x20bb4650, 0.5]]);
const refs = image.highLowFixups(original.va, original.bytes.length).map(offset => {
    const va = original.bytes.readUInt32LE(offset);
    const value = image.codeAt(va, 8, false);
    assert.equal(value.readDoubleLE(0), expected.get(va));
    return {offset, address: va.toString(16), identity: value.toString('hex')};
});
assert.equal(refs.length, expected.size);
const originalOps = new Map(original.rows.map(([address, , operation]) => [address, operation]));
for (const [address, operation] of [
    ['20c457f6', 'FST double ptr [ESI + 0x204]'], ['20c4580c', 'FST double ptr [ESI + 0x1fc]'],
    ['20c45823', 'FSTP double ptr [ESI + 0x1fc]'], ['20c45834', 'FST double ptr [ESI + 0x204]'],
    ['20c4583a', 'FSTP double ptr [ESI + 0x1fc]']
]) assert.equal(originalOps.get(address), operation);
const rebuiltRefs = nativeCoffReferences(file, coff, rebuilt, null, '', refs.map(ref => ref.identity));
const comparison = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, refs, rebuiltRefs, [
    [0x20c4576b, 0x20c4576e, 'Read the airplane mode slot'],
    [0x20c457a5, 0x20c457ac, 'Check that the lateral mount coordinate is zero'],
    [0x20c457b0, 0x20c457b8, 'Check for positive mount Z'],
    [0x20c457cf, 0x20c457d5, 'Advance to the next 0xec-byte engine record'],
    [0x20c457eb, 0x20c457f1, 'Convert the selected-engine count as unsigned'],
    [0x20c457f6, 0x20c457fc, 'Publish the mean velocity increment'],
    [0x20c457fc, 0x20c45804, 'Load and double the air-relative speed'],
    [0x20c45806, 0x20c4580c, 'Add the quadratic velocity term'],
    [0x20c4580c, 0x20c45812, 'Publish the squared-speed increment'],
    [0x20c4581b, 0x20c45821, 'Multiply the density by one half'],
    [0x20c45823, 0x20c45829, 'Publish the final pressure increment'],
    [0x20c45834, 0x20c45840, 'Clear both outputs when no engine qualifies']
]);
const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    symbol: 'AirplaneDynamics::UpdatePropellerSlipstream', ...comparison};
fs.writeFileSync(path.join(run, 'tail-slipstream-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${comparison.entry} ${comparison.status}: ${comparison.originalBytes} original / ${comparison.rebuiltBytes} native COFF bytes`);
for (const scope of comparison.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
