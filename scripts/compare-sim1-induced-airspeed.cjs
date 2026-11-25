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
const original = originalListing(program, '20c480b0', image);

function directCallTarget(va) {
    const instruction = image.codeAt(va, 5);
    assert.equal(instruction[0], 0xe8);
    return va + 5 + instruction.readInt32LE(1);
}

assert.equal(directCallTarget(0x20c476e1), original.va);
const sqrtThunk = directCallTarget(0x20c4811d);
assert.equal(sqrtThunk, 0x20c66fc2);
const jump = image.codeAt(sqrtThunk, 6);
assert.equal(jump.readUInt16LE(), 0x25ff);
assert.deepEqual(readPE32Imports(binary).get(jump.readUInt32LE(2)), {dll: 'msvcr80.dll', name: '_CIsqrt'});
const sqrtIdentity = 'msvcr80.dll!_CIsqrt';
const expected = new Map([[0x20bc3388, 0.25], [0x20bb4650, 0.5]]);
const refs = image.highLowFixups(original.va, original.bytes.length).map(offset => {
    const va = original.bytes.readUInt32LE(offset);
    const bytes = image.codeAt(va, 8, false);
    assert.equal(bytes.readDoubleLE(0), expected.get(va));
    return {offset, address: va.toString(16), identity: bytes.toString('hex')};
});
assert.equal(refs.length, expected.size);
const literalIdentities = refs.map(ref => ref.identity);
refs.push({offset: 0x20c4811e - original.va, address: sqrtThunk.toString(16), identity: sqrtIdentity});

const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const symbol = 'EngineForceRecord::UpdateInducedAirSpeed';
const rebuilt = nativeCoffBody(file, coff, dump, '?UpdateInducedAirSpeed@EngineForceRecord@sim1@@');
const sqrt = coff.unique('__CIsqrt');
assert.equal(sqrt.section, 0);
const rebuiltRefs = nativeCoffReferences(file, coff, rebuilt, {symbol: sqrt}, sqrtIdentity, literalIdentities);
const comparison = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, refs, rebuiltRefs, [
    [0x20c480b0, 0x20c480b5, 'Hotpatch entry and frame setup'],
    [0x20c480b7, 0x20c480c3, 'Compare axial velocity with zero'],
    [0x20c480c6, 0x20c480d7, 'Clamp axial velocity to an ordered positive value or zero'],
    [0x20c480df, 0x20c480e6, 'Branch on the thrust comparison flags'],
    [0x20c480ed, 0x20c480fd, 'Read the environment and check for ordered positive density'],
    [0x20c48102, 0x20c4810a, 'Load air density onto the x87 stack'],
    [0x20c4810c, 0x20c48122, 'Apply disk area to doubled density and take the square root of the loaded velocity term'],
    [0x20c48125, 0x20c48130, 'Subtract half the forward speed and store the induced speed'],
    [0x20c48136, 0x20c4813d, 'Clear the induced speed when density is invalid']
]);
const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), symbol, ...comparison};
fs.writeFileSync(path.join(run, 'induced-airspeed-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${comparison.entry} ${comparison.status}: ${comparison.originalBytes} original / ${comparison.rebuiltBytes} native COFF bytes`);
for (const scope of comparison.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
