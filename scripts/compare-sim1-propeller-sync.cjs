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
const original = originalListing(program, '20c48150', image);
const caller = originalListing(program, '20c47420', image);
const call = image.codeAt(0x20c476c3, 5);
assert.equal(call[0], 0xe8);
assert.equal(0x20c476c8 + call.readInt32LE(1), original.va);

const expected = new Map([[0x20bc3740, 0.04], [0x20bc39e8, 0.088]]);
const refs = image.highLowFixups(original.va, original.bytes.length).map(offset => {
    const va = original.bytes.readUInt32LE(offset);
    const bytes = image.codeAt(va, 8, false);
    assert.equal(bytes.readDoubleLE(0), expected.get(va));
    return {offset, address: va.toString(16), identity: bytes.toString('hex')};
});
assert.equal(refs.length, expected.size);

const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const symbol = 'PropellerForceRecord::UpdateSynchronization';
const rebuilt = nativeCoffBody(file, coff, dump, '?UpdateSynchronization@PropellerForceRecord@sim1@@');
const rebuiltRefs = nativeCoffReferences(file, coff, rebuilt, null, '', refs.map(ref => ref.identity));
const comparison = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, refs, rebuiltRefs, [
    [0x20c48150, 0x20c48155, 'Hotpatch entry and frame setup'],
    [0x20c48155, 0x20c48157, 'Load the propulsion configuration'],
    [0x20c4815f, 0x20c48165, 'Multiply the inertia by 0.04'],
    [0x20c48165, 0x20c48168, 'Multiply the inertia gain by the time step'],
    [0x20c48181, 0x20c48188, 'Check configuration support for propeller synchronization'],
    [0x20c48193, 0x20c4819a, 'Check runtime enable for propeller synchronization'],
    [0x20c481a4, 0x20c481b3, 'Compare the absolute speed difference with 0.088'],
    [0x20c481b5, 0x20c481b7, 'Multiply the speed difference by the correction scale'],
    [0x20c481bd, 0x20c481c1, 'Return and remove the engine pointer and time step'],
    [0x20c481c1, 0x20c481c7, 'Discard both inputs and load zero for the disabled path']
]);
const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), symbol,
    caller: {entry: caller.va.toString(16), call: '20c476c3'}, ...comparison};
fs.writeFileSync(path.join(run, 'propeller-sync-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${comparison.entry} ${comparison.status}: ${comparison.originalBytes} original / ${comparison.rebuiltBytes} native COFF bytes`);
for (const scope of comparison.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
