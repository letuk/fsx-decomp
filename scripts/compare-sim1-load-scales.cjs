const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { originalListing, nativeCoffBody, nativeCoffReferences, compareNativeRanges } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 aerodynamic moment run directory');
const program = 'sim1.dll';
const sha = data => createHash('sha256').update(data).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const original = originalListing(program, '20c00530', image);
const file = fs.readFileSync(path.join(run, 'sim1-aerodynamic-moment.obj'));
const coff = parseNativeCoff(file);
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const rebuilt = nativeCoffBody(file, coff, dump, '?UpdateAerodynamicLoadScales@IntegrationHistory@sim1@@');
const begin = 0x20c0054f;
const end = 0x20c005f3;
const literal = '000000000000d03f';
assert.equal(image.codeAt(0x20bc3388, 8, false).toString('hex'), literal);
const originalRefs = image.highLowFixups(begin, end - begin).map(relative => {
    const offset = begin - original.va + relative;
    assert.equal(original.bytes.readUInt32LE(offset), 0x20bc3388);
    return {offset, address: '20bc3388', identity: literal};
});
assert.equal(originalRefs.length, 1);
const rebuiltRefs = nativeCoffReferences(file, coff, rebuilt, null, '', [literal]);
const range = compareNativeRanges(original, rebuilt, begin, end, originalRefs, rebuiltRefs, [
    [0x20c0054f, 0x20c00555, 'Load the stored dynamic pressure'],
    [0x20c00555, 0x20c00557, 'Multiply by wing area'],
    [0x20c0055d, 0x20c00560, 'Multiply the pressure force by wing span'],
    [0x20c00560, 0x20c00566, 'Store the span-scaled moment factor'],
    [0x20c00566, 0x20c0056c, 'Reload the rounded pressure force'],
    [0x20c0056c, 0x20c0056f, 'Multiply the rounded pressure force by reference chord'],
    [0x20c0056f, 0x20c00575, 'Store the chord-scaled moment factor'],
    [0x20c0057b, 0x20c00581, 'Load the quarter-density constant'],
    [0x20c00592, 0x20c00598, 'Store the lateral force rate factor'],
    [0x20c005a0, 0x20c005a6, 'Store the roll and yaw damping factor'],
    [0x20c005ab, 0x20c005b1, 'Store the lift rate factor'],
    [0x20c005b1, 0x20c005b7, 'Multiply by the chord twice for pitch damping'],
    [0x20c005b7, 0x20c005bd, 'Store the pitch damping factor'],
    [0x20c005bd, 0x20c005c9, 'Sum the freestream and slipstream speeds'],
    [0x20c005d5, 0x20c005da, 'Multiply the slipstream base by wing area and chord'],
    [0x20c005df, 0x20c005e5, 'Store the slipstream pitch factor'],
    [0x20c005ed, 0x20c005f3, 'Store the slipstream yaw factor']
]);
const stores = [0x1e4, 0x1ec, 0x1f4, 0x748, 0x750, 0x758, 0x760, 0x768, 0x770];
for (const offset of stores) {
    const originalStore = new RegExp('^FSTP? double ptr \\[ESI \\+ 0x' + offset.toString(16) + '\\]$');
    const rebuiltStore = new RegExp('^fstp? qword ptr \\[\\w+\\+' + offset.toString(16) + 'h\\]$', 'i');
    assert.equal(range.asm.filter(row => originalStore.test(row[2])).length, 1, offset.toString(16));
    assert.equal(range.rebuilt.filter(row => rebuiltStore.test(row[2])).length, 1, offset.toString(16));
}

const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    parentEntry: '20c00530', symbol: 'IntegrationHistory::UpdateAerodynamicLoadScales', stores, ...range};
fs.writeFileSync(path.join(run, 'load-scales-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${range.entry} ${range.status}: ${range.originalBytes} original / ${range.rebuiltBytes} native COFF bytes`);
for (const scope of range.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
