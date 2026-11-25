const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {openImage} = require('./lib/pe32-dll.cjs');
const {readPE32Imports} = require('./lib/pe-imports.cjs');
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
const original = originalListing(program, '20c47ba0', image);
const caller = originalListing(program, '20c47420', image);
const call = image.codeAt(0x20c475bd, 5);
assert.equal(call[0], 0xe8);
assert.equal(0x20c475c2 + call.readInt32LE(1), original.va);

const expected = new Map([[0x20bc3658, 0.3], [0x20bc2a90, 1.0], [0x20bc0678, 2.0],
    [0x20bc3a38, 8.0], [0x20bc3a78, 9.549296586], [0x20bc35a8, 0.1047197551]]);
const importOffset = 0x20c47c78 - original.va;
const importIdentity = 'util.dll!#199';
const refs = image.highLowFixups(original.va, original.bytes.length).map(offset => {
    const va = original.bytes.readUInt32LE(offset);
    if (offset === importOffset) {
        assert.equal(original.bytes.readUInt16LE(offset - 2), 0x15ff);
        assert.equal(va, 0x20bb13e8);
        assert.deepEqual(readPE32Imports(binary).get(va), {dll: 'util.dll', name: '#199'});
        return {offset, address: va.toString(16), identity: importIdentity};
    }
    const bytes = image.codeAt(va, 8, false);
    assert.equal(bytes.readDoubleLE(0), expected.get(va));
    return {offset, address: va.toString(16), identity: bytes.toString('hex')};
});
assert.equal(refs.length, expected.size + 1);
assert(refs.some(ref => ref.offset === importOffset));

const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const symbol = 'PropellerForceRecord::IntegrateSpeed';
const rebuilt = nativeCoffBody(file, coff, dump, '?IntegrateSpeed@PropellerForceRecord@sim1@@');
const importName = '__imp__AdvanceWrappedAngle@16';
const importRefs = rebuilt.section.relocations.filter(relocation => relocation.symbol.name === importName);
assert.equal(importRefs.length, 1);
const binding = importRefs[0];
assert.equal(binding.symbol.section, 0);
assert.equal(binding.type, 6);
assert.equal(rebuilt.bytes.readUInt32LE(binding.offset), 0);
assert.equal(rebuilt.bytes.readUInt16LE(binding.offset - 2), 0x15ff);
const literals = {...rebuilt, section: {...rebuilt.section,
    relocations: rebuilt.section.relocations.filter(relocation => relocation !== binding)}};
const rebuiltRefs = nativeCoffReferences(file, coff, literals, null, '', refs.map(ref => ref.identity));
rebuiltRefs.push({offset: binding.offset, symbol: importName, identity: importIdentity});

const comparison = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, refs, rebuiltRefs, [
    [0x20c47ba0, 0x20c47ba5, 'Hotpatch entry and frame setup'],
    [0x20c47ba6, 0x20c47bb1, 'Store aerodynamic torque times initial angular speed'],
    [0x20c47bbc, 0x20c47bc2, 'Load the gear reduction ratio for a piston engine'],
    [0x20c47bca, 0x20c47beb, 'Apply the propeller load factor and calculate angular acceleration'],
    [0x20c47bee, 0x20c47c11, 'Apply the stop override, integrate angular speed and compare with two radians per second'],
    [0x20c47c13, 0x20c47c40, 'Test the acceleration deadband, calculate RPM and test rotation sign'],
    [0x20c47c40, 0x20c47c4c, 'Reverse and store RPM for the opposite rotation direction'],
    [0x20c47c4c, 0x20c47c7c, 'Normalize RPM and pass the phase and angular increment to util ordinal 199'],
    [0x20c47c7c, 0x20c47c7f, 'Store the wrapped phase as a float']
]);
const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), symbol,
    caller: {entry: caller.va.toString(16), call: '20c475bd'},
    importBinding: {symbol: importName, target: importIdentity, importLibraryRecipe: 'scripts/matching/guest-build-sim1-thrust-linked.cmd'},
    ...comparison};
fs.writeFileSync(path.join(run, 'propeller-speed-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${comparison.entry} ${comparison.status}: ${comparison.originalBytes} original / ${comparison.rebuiltBytes} native COFF bytes`);
for (const scope of comparison.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
