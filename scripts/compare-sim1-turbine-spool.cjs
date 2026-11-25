const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');
const { compareNativeRanges, nativeCoffBody, originalListing } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass the native VC8 turbine run directory');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const original = originalListing(program, '20c2c6d0', image);
const file = fs.readFileSync(path.join(run, 'sim1-turbine-shaft-torque.obj'));
const request = JSON.parse(fs.readFileSync(path.join(run, 'request.json'), 'utf8').replace(/^\uFEFF/, ''));
const coff = parseNativeCoff(file);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'utf8');
const rebuilt = nativeCoffBody(file, coff, listing, '?UpdateSpoolSpeed@TurbineEngineState@sim1@@');

const curve = '?SampleAerodynamicCurve@sim1@@YINPBUAerodynamicCurve@1@N@Z';
const squareRoot = '__CIsqrt';
const originalRefs = [[0x20c2c755, 0x20bcbf60, curve], [0x20c2c82e, 0x20c66fc2, squareRoot]]
    .map(([va, address, identity]) => {
        const instruction = image.codeAt(va, 5);
        assert.equal(instruction[0], 0xe8);
        assert.equal(va + instruction.length + instruction.readInt32LE(1), address);
        return { offset: va + 1 - original.va, identity, address };
    });
const thunk = image.codeAt(0x20c66fc2, 6);
assert.equal(thunk.readUInt16LE(), 0x25ff);
const sqrtImport = readPE32Imports(binary).get(thunk.readUInt32LE(2));
assert.equal(sqrtImport.dll.toLowerCase(), 'msvcr80.dll');
assert.equal(sqrtImport.name, '_CIsqrt');

const scheduleValues = [
    [0x20c2c6ec, 0x20bc3620, 100],
    [0x20c2c6f5, 0x20bc3cd8, 62],
    [0x20c2c6fe, 0x20bc39b0, 150],
    [0x20c2c707, 0x20bc1220, 50],
    [0x20c2c710, 0x20bc3cd0, 610],
    [0x20c2c719, 0x20bc3cc8, 101.5],
];
const literals = [
    [0x20c2c72e, 0x20bc3590, 0.0001],
    [0x20c2c73d, 0x20bb4650, 0.5],
    [0x20c2c761, 0x20bb4650, 0.5],
    [0x20c2c76b, 0x20bc3cc0, 130],
    [0x20c2c780, 0x20bc3cb8, 0.13999999999999999],
    [0x20c2c788, 0x20bc3cb0, 0.15],
    [0x20c2c796, 0x20bc3ca8, 12],
    [0x20c2c79c, 0x20bc3658, 0.3],
    [0x20c2c7eb, 0x20bb4050, 20],
    [0x20c2c7fa, 0x20bc38d0, -20],
];
for (const [va, address, value] of [...scheduleValues, ...literals]) {
    assert.equal(image.codeAt(va + 2, 4).readUInt32LE(), address);
    const data = image.codeAt(address, 8);
    assert.equal(data.readDoubleLE(), value);
    originalRefs.push({ offset: va + 2 - original.va, identity: data.toString('hex'), address });
}
assert.equal(image.codeAt(0x20c2c82a, 4).readUInt32LE(), 0x20c7ba80);
originalRefs.push({ offset: 0x20c2c82a - original.va, identity: 'Inlet temperature ratio storage', address: 0x20c7ba80 });

// The original constructs the four samples on the stack from FLDZ and six literals.
assert.equal(image.codeAt(0x20c2c6dc, 2).readUInt16LE(), 0xeed9);
assert.equal(image.codeAt(0x20c2c6de, 3).toString('hex'), 'dd55bc');
assert.equal(image.codeAt(0x20c2c6e2, 3).toString('hex'), 'dd55c4');
assert.equal(image.codeAt(0x20c2c6e8, 4).readUInt32LE(), 4);
assert.equal(image.codeAt(0x20c2c74c, 3).toString('hex'), '8d75b8');
const originalSchedule = Buffer.alloc(4 + 4 * 16);
originalSchedule.writeUInt32LE(4);
for (const [index, [va, address]] of scheduleValues.entries()) {
    const store = image.codeAt(va + 6, 3);
    assert.equal(store[0], 0xdd);
    assert.equal(store[1], 0x5d);
    assert.equal(store.readInt8(2), -0x34 + 8 * index);
    image.codeAt(address, 8).copy(originalSchedule, 20 + 8 * index);
}

let rebuiltSchedule;
const rebuiltRefs = rebuilt.section.relocations.map(({ offset, symbol, type }) => {
    assert.equal(rebuilt.bytes.readUInt32LE(offset), 0);
    if (type === 20) {
        assert.equal(rebuilt.bytes[offset - 1], 0xe8);
        assert([curve, squareRoot].includes(symbol.name), symbol.name);
        return { offset, identity: symbol.name, symbol: symbol.name };
    }
    assert.equal(type, 6);
    const data = coff.sections[symbol.section - 1];
    if (symbol.name === '_spoolSpeedSchedule') {
        assert.equal(rebuilt.bytes[offset - 1], 0xb9);
        assert(data && symbol.value + originalSchedule.length <= data.size);
        rebuiltSchedule = file.subarray(data.raw + symbol.value, data.raw + symbol.value + originalSchedule.length);
        assert(rebuiltSchedule.equals(originalSchedule));
        return { offset, identity: 'Spool speed schedule', symbol: symbol.name };
    }
    const encoded = symbol.name.match(/^__real@([0-9a-f]{8}|[0-9a-f]{16})$/i);
    assert(encoded, symbol.name);
    const size = encoded[1].length / 2;
    assert(data && symbol.value + size <= data.size);
    const bytes = file.subarray(data.raw + symbol.value, data.raw + symbol.value + size);
    assert(bytes.equals(Buffer.from(encoded[1], 'hex').reverse()));
    const value = size === 8 ? bytes.readDoubleLE() : bytes.readFloatLE();
    const foldedGain = request.Profile === 'Fast' && value === 0.13999999999999999 / 130;
    if (foldedGain) assert.equal(rebuilt.bytes.readUInt16LE(offset - 2), 0x0ddc);
    assert(foldedGain || literals.some(([, , expected]) => expected === value), symbol.name);
    return { offset, identity: bytes.toString('hex'), symbol: symbol.name };
});
for (const identity of [curve, 'Spool speed schedule']) {
    assert.equal(rebuiltRefs.filter(ref => ref.identity === identity).length, 1, identity);
}
const squareRootCalls = rebuiltRefs.filter(ref => ref.identity === squareRoot);
let rebuiltSquareRoot = sqrtImport.dll + '!' + sqrtImport.name;
if (squareRootCalls.length === 0) {
    assert.equal(request.Profile, 'Fast');
    const instructions = rebuilt.rows.filter(([, hex, instruction]) => hex === 'd9 fa' && instruction === 'fsqrt');
    assert.equal(instructions.length, 1);
    rebuiltSquareRoot = 'x87 FSQRT';
} else {
    assert.equal(squareRootCalls.length, 1);
}

const report = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length,
    originalRefs, rebuiltRefs, [
        [0x20c2c6d0, 0x20c2c6d5, 'Hotpatch entry and frame setup'],
        [0x20c2c72c, 0x20c2c739, 'Absolute drive signal and comparison against the threshold'],
        [0x20c2c755, 0x20c2c75a, 'Call the shared one-axis spool schedule evaluator'],
        [0x20c2c780, 0x20c2c78e, 'Calculate nonrunning spool response rate'],
        [0x20c2c7aa, 0x20c2c7b1, 'Check axial airflow before applying the windmilling gain'],
        [0x20c2c7eb, 0x20c2c7f8, 'Compare the spool rate with its upper limit'],
        [0x20c2c82e, 0x20c2c833, 'Call the CRT square root for temperature correction'],
    ]);
Object.assign(report, { program, originalSHA256, objectSHA256: sha(file), originalRefs, rebuiltRefs, rebuiltSquareRoot,
    schedule: { status: 'Exact', bytes: originalSchedule.length, originalHex: originalSchedule.toString('hex'),
        rebuiltHex: rebuiltSchedule.toString('hex') } });
fs.writeFileSync(path.join(run, 'turbine-spool-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`sim1.dll 0x${report.entry}: ${report.status}, ${report.originalBytes}/${report.rebuiltBytes} bytes`);
for (const scope of report.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
console.log(`Exact: ${report.schedule.bytes} spool schedule bytes`);
