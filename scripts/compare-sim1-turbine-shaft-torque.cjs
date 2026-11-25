const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { compareNativeRanges, nativeCoffBody, originalListing } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass the native VC8 turbine shaft torque run directory');
const program = 'sim1.dll';
const sha = data => createHash('sha256').update(data).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const original = originalListing(program, '20c2c850', image);
const file = fs.readFileSync(path.join(run, 'sim1-turbine-shaft-torque.obj'));
const coff = parseNativeCoff(file);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'utf8');
const rebuilt = nativeCoffBody(file, coff, listing, '?UpdateTurbineShaftTorque@EngineForceRecord@sim1@@');

const curve = '?SampleAerodynamicCurve@sim1@@YINPBUAerodynamicCurve@1@N@Z';
const originalRefs = [0x20c2c883, 0x20c2c912, 0x20c2c92f].map(va => {
    const call = image.codeAt(va, 5);
    assert.equal(call[0], 0xe8);
    const address = va + 5 + call.readInt32LE(1);
    assert.equal(address, 0x20bcbf60);
    return { offset: va + 1 - original.va, identity: curve, address };
});
const literals = [
    [0x20c2c894, 0x20bc4308, 0.08],
    [0x20c2c8a3, 0x20bc3ca8, 12],
    [0x20c2c8b6, 0x20bc3790, 4],
    [0x20c2c8bc, 0x20bb1700, 0.1],
    [0x20c2c93d, 0x20bbfc48, 5],
];
for (const [va, address, value] of literals) {
    assert.equal(image.codeAt(va + 2, 4).readUInt32LE(), address);
    const data = image.codeAt(address, 8);
    assert.equal(data.readDoubleLE(), value);
    originalRefs.push({ offset: va + 2 - original.va, identity: data.toString('hex'), address });
}

const rebuiltRefs = rebuilt.section.relocations.map(({ offset, symbol, type }) => {
    assert.equal(rebuilt.bytes.readUInt32LE(offset), 0);
    if (type === 20) {
        assert.equal(rebuilt.bytes[offset - 1], 0xe8);
        assert.equal(symbol.name, curve);
        return { offset, identity: curve, symbol: symbol.name };
    }
    assert.equal(type, 6);
    const encoded = symbol.name.match(/^__real@([0-9a-f]{8}|[0-9a-f]{16})$/i);
    assert(encoded, symbol.name);
    const section = coff.sections[symbol.section - 1];
    const size = encoded[1].length / 2;
    assert(section && symbol.value + size <= section.size);
    const data = file.subarray(section.raw + symbol.value, section.raw + symbol.value + size);
    assert(data.equals(Buffer.from(encoded[1], 'hex').reverse()));
    const value = size === 8 ? data.readDoubleLE() : data.readFloatLE();
    assert(literals.some(([, , expected]) => expected === value), symbol.name);
    return { offset, identity: data.toString('hex'), symbol: symbol.name };
});
assert.equal(rebuiltRefs.filter(ref => ref.identity === curve).length, 3);

const report = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length,
    originalRefs, rebuiltRefs, [
        [0x20c2c894, 0x20c2c8a9, 'Running torque reduction, binary64 store and response rate'],
        [0x20c2c8b6, 0x20c2c8c2, 'Stopped-engine response rate'],
        [0x20c2c949, 0x20c2c950, 'Ordered speed comparison and smoothing branch'],
        [0x20c2c953, 0x20c2c959, 'Torque response rate and step multiplications'],
    ]);
Object.assign(report, { program, originalSHA256, objectSHA256: sha(file), originalRefs, rebuiltRefs });
fs.writeFileSync(path.join(run, 'turbine-shaft-torque-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`sim1.dll 0x${report.entry}: ${report.status}, ${report.originalBytes}/${report.rebuiltBytes} bytes`);
