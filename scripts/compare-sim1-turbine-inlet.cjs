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
const original = originalListing(program, '20c2b9d0', image);
const file = fs.readFileSync(path.join(run, 'sim1-turbine-shaft-torque.obj'));
const coff = parseNativeCoff(file);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'utf8');
const rebuilt = nativeCoffBody(file, coff, listing, '?UpdateInletConditions@TurbineEngineState@sim1@@');

const imports = readPE32Imports(binary);
const mathCalls = [
    [0x20c2ba31, 0x20c66fda, '_CIpow'],
    [0x20c2ba48, 0x20c66fc2, '_CIsqrt'],
];
const originalRefs = mathCalls.map(([va, address, importName]) => {
    const instruction = image.codeAt(va, 5);
    assert.equal(instruction[0], 0xe8);
    assert.equal(va + instruction.length + instruction.readInt32LE(1), address);
    const thunk = image.codeAt(address, 6);
    assert.equal(thunk.readUInt16LE(), 0x25ff);
    const imported = imports.get(thunk.readUInt32LE(2));
    assert.equal(imported.dll.toLowerCase(), 'msvcr80.dll');
    assert.equal(imported.name, importName);
    return { offset: va + 1 - original.va, identity: '_' + importName, address };
});
const literals = [
    [0x20c2b9e0, 0x20bb4650, 0.5],
    [0x20c2ba12, 0x20bbf7d8, 0.2],
    [0x20c2ba2b, 0x20bb17c8, 3.5],
];
const outputs = [
    [0x20c7ba70, 'turbineInletTemperatureRoot', [0x20c2ba4d]],
    [0x20c7ba78, 'turbineInletPressureRatio', [0x20c2ba3c]],
    [0x20c7ba80, 'turbineInletTemperatureRatio', [0x20c2ba25, 0x20c2ba42]],
];
for (const [va, address, value] of literals) {
    assert.equal(image.codeAt(va + 2, 4).readUInt32LE(), address);
    const data = image.codeAt(address, 8);
    assert.equal(data.readDoubleLE(), value);
    originalRefs.push({ offset: va + 2 - original.va, identity: data.toString('hex'), address });
}
for (const [address, name, uses] of outputs) {
    assert.equal(image.initialDword(address), 0);
    assert.equal(image.initialDword(address + 4), 0);
    for (const va of uses) {
        assert.equal(image.codeAt(va + 2, 4).readUInt32LE(), address);
        originalRefs.push({ offset: va + 2 - original.va, identity: name, address });
    }
}
const directCalls = new Set(mathCalls.map(([, , name]) => '_' + name));
const globalNames = new Map(outputs.map(([, name]) => [`?${name}@sim1@@3NA`, name]));
const rebuiltRefs = rebuilt.section.relocations.map(({ offset, symbol, type }) => {
    assert.equal(rebuilt.bytes.readUInt32LE(offset), 0);
    if (type === 20) {
        assert.equal(rebuilt.bytes[offset - 1], 0xe8);
        assert(directCalls.has(symbol.name), symbol.name);
        return { offset, identity: symbol.name, symbol: symbol.name };
    }
    assert.equal(type, 6);
    const data = coff.sections[symbol.section - 1];
    if (globalNames.has(symbol.name)) {
        assert(data?.name === '.bss' && symbol.value + 8 <= data.size);
        return { offset, identity: globalNames.get(symbol.name), symbol: symbol.name };
    }
    const encoded = symbol.name.match(/^__real@([0-9a-f]{8}|[0-9a-f]{16})$/i);
    assert(encoded, symbol.name);
    const size = encoded[1].length / 2;
    assert(data && symbol.value + size <= data.size);
    const bytes = file.subarray(data.raw + symbol.value, data.raw + symbol.value + size);
    assert(bytes.equals(Buffer.from(encoded[1], 'hex').reverse()));
    const value = size === 8 ? bytes.readDoubleLE() : bytes.readFloatLE();
    assert(literals.some(([, , expected]) => expected === value), symbol.name);
    return { offset, identity: bytes.toString('hex'), symbol: symbol.name };
});
for (const [, name, uses] of outputs) {
    assert.equal(rebuiltRefs.filter(ref => ref.identity === name).length, uses.length, name);
}
assert.equal(rebuiltRefs.filter(ref => ref.identity === '__CIpow').length, 1);
const squareRootCall = rebuiltRefs.some(ref => ref.identity === '__CIsqrt');
if (!squareRootCall) {
    const request = JSON.parse(fs.readFileSync(path.join(run, 'request.json'), 'utf8').replace(/^\uFEFF/, ''));
    assert.equal(request.Profile, 'Fast');
    assert.equal(rebuilt.rows.filter(([, hex, instruction]) => hex === 'd9 fa' && instruction === 'fsqrt').length, 1);
}

const report = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length,
    originalRefs, rebuiltRefs, [
        [0x20c2b9d4, 0x20c2b9de, 'Load turbine performance data and test the Mach-cap selector'],
        [0x20c2b9e0, 0x20c2b9e6, 'Load the alternative half-Mach cap'],
        [0x20c2b9e6, 0x20c2b9f1, 'Compare Mach with the selected cap'],
        [0x20c2b9f3, 0x20c2b9fb, 'Select the uncapped Mach input'],
        squareRootCall
            ? [0x20c2ba0c, 0x20c2ba53, 'Publish inlet Mach, temperature, pressure and temperature root']
            : [0x20c2ba0c, 0x20c2ba42, 'Publish inlet Mach, temperature and pressure'],
    ]);
Object.assign(report, { program, originalSHA256, objectSHA256: sha(file), originalRefs, rebuiltRefs,
    squareRoot: squareRootCall ? 'MSVCR80.dll!_CIsqrt' : 'x87 FSQRT' });
fs.writeFileSync(path.join(run, 'turbine-inlet-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`sim1.dll 0x${report.entry}: ${report.status}, ${report.originalBytes}/${report.rebuiltBytes} bytes`);
for (const scope of report.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
