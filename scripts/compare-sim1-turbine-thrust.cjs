const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { compareNativeRanges, nativeCoffBody, originalListing } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass the native VC8 turbine run directory');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const file = fs.readFileSync(path.join(run, 'sim1-turbine-shaft-torque.obj'));
const coff = parseNativeCoff(file);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'utf8');

const globals = new Map([
    [0x20c7ba58, 'turbineRamDrag'],
    [0x20c7ba60, 'turbineGrossThrust'],
    [0x20c7ba70, 'turbineInletTemperatureRoot'],
    [0x20c7ba78, 'turbineInletPressureRatio'],
    [0x20c7c180, 'gravityAcceleration'],
]);
const calls = new Map([
    ['20bcc030', 'Evaluate'],
    ['20bcbf60', 'SampleAerodynamicCurve'],
    ['20c2bb20', 'AfterburnerFraction'],
    ['20c2c4e0', 'ReverserThrustFactor'],
]);
const sizes = new Map([[0x20bc3648, 4]]);
const specs = [
    ['20c2b879', 'UpdateGrossThrust', [
        [0x20c2b894, 0x20c2b8a2, 'Pass raw Mach and call the gross thrust table evaluator'],
        [0x20c2b8b1, 0x20c2b8b7, 'Apply inlet pressure ratio to gross thrust'],
        [0x20c2b8b9, 0x20c2b8bf, 'Publish gross thrust'],
    ]],
    ['20c2c2e0', 'UpdateRamDrag', [
        [0x20c2c2e0, 0x20c2c2e5, 'Hotpatch entry and frame setup'],
        [0x20c2c306, 0x20c2c30e, 'Pass inlet Mach and call the inlet flow table evaluator'],
        [0x20c2c31c, 0x20c2c328, 'Apply inlet pressure and temperature corrections'],
        [0x20c2c331, 0x20c2c33f, 'Apply inlet velocity and gravity scale and publish ram drag'],
    ]],
    ['20c2bb20', 'AfterburnerFraction', [
        [0x20c2bb20, 0x20c2bb25, 'Hotpatch entry and frame setup'],
        [0x20c2bb59, 0x20c2bb5b, 'Divide current afterburner stage by configured stages'],
    ]],
    ['20c2c4e0', 'ReverserThrustFactor', [
        [0x20c2c4e0, 0x20c2c4e5, 'Hotpatch entry and frame setup'],
        [0x20c2c5db, 0x20c2c5e9, 'Pass raw Mach and call the tabulated reverser evaluator'],
        [0x20c2c5f3, 0x20c2c5f8, 'Read the ordered reverser-effectiveness comparison'],
        [0x20c2c5fd, 0x20c2c609, 'Calculate the linear reverser slope'],
    ]],
    ['20c2c620', 'PublishNetThrust', [
        [0x20c2c620, 0x20c2c625, 'Hotpatch entry and frame setup'],
        [0x20c2c667, 0x20c2c670, 'Load raw Mach and reserve the curve argument'],
        [0x20c2c676, 0x20c2c67e, 'Store Mach and call the afterburner curve'],
        [0x20c2c67e, 0x20c2c682, 'Subtract one from the afterburner multiplier'],
        [0x20c2c6af, 0x20c2c6b5, 'Subtract the stored ram drag from adjusted thrust'],
    ]],
    ['20c2b951', 'UpdateSpeedBlend', [
        [0x20c2b959, 0x20c2b95f, 'Scale the shaft and turbine speed difference'],
        [0x20c2b962, 0x20c2b96a, 'Scale shaft speed and add both target terms'],
        [0x20c2b96d, 0x20c2b96f, 'Apply the blend response gain of two'],
    ]],
];

function originalTable(rows) {
    const table = Buffer.alloc(208);
    const stack = [];
    for (const [address, , instruction] of rows) {
        if (address < '20c2c4fe' || address > '20c2c5d1') continue;
        if (instruction === 'FLDZ') stack.unshift(0.0);
        else if (instruction === 'FLD1') stack.unshift(1.0);
        else if (instruction === 'FXCH') [stack[0], stack[1]] = [stack[1], stack[0]];
        else if (instruction.startsWith('FLD double ptr')) {
            const literal = Number(instruction.match(/\[(0x[0-9a-f]+)\]/i)[1]);
            stack.unshift(image.codeAt(literal, 8).readDoubleLE());
        } else {
            const store = instruction.match(/^FST(P)? double ptr \[EBP \+ (0x[0-9a-f]+|-0x[0-9a-f]+)\]$/i);
            if (store) {
                const displacement = Number.parseInt(store[2], 16) | 0;
                table.writeDoubleLE(stack[0], 0xd0 + displacement);
                if (store[1]) stack.shift();
            } else if (/^MOV dword ptr \[EBP/.test(instruction)) {
                const displacement = Number(instruction.match(/(0x[0-9a-f]+)\]/i)[1]) | 0;
                table.writeUInt32LE(5, 0xd0 + displacement);
            } else {
                assert(/^(MOV ECX,0x5|SUB ESP,0x10|LEA ESI,\[EBP)/.test(instruction), instruction);
            }
        }
    }
    assert.equal(stack.length, 0);
    assert.equal(table.readUInt32LE(0), 5);
    assert.equal(table.readUInt32LE(4), 5);
    return table;
}

const originalRows = specs.map(([entry]) => originalListing(program, entry, image));
const reverserTable = originalTable(originalRows[3].rows);
const tableSymbol = coff.unique('_reverserThrustTable');
const tableSection = coff.sections[tableSymbol.section - 1];
assert(tableSection && tableSymbol.value + reverserTable.length <= tableSection.size);
const rebuiltTable = file.subarray(tableSection.raw + tableSymbol.value,
    tableSection.raw + tableSymbol.value + reverserTable.length);
assert(rebuiltTable.equals(reverserTable), 'The original stack table and the rebuilt reverser data differ');

const functions = specs.map(([entry, name, ranges], index) => {
    const original = originalRows[index];
    const rebuilt = nativeCoffBody(file, coff, listing, `?${name}@TurbineEngineState@sim1@@`);
    const literals = new Set();
    const originalRefs = original.rows.flatMap(([address, hex, instruction]) => {
        if (instruction === 'FLD1') literals.add(1.0);
        if (instruction === 'FLDZ') literals.add(0.0);
        const va = Number.parseInt(address, 16);
        const call = instruction.match(/^CALL 0x([0-9a-f]+)$/i);
        if (call) {
            const identity = calls.get(call[1]);
            assert(identity, instruction);
            const bytes = image.codeAt(va, 5);
            assert.equal(bytes[0], 0xe8);
            const target = va + 5 + bytes.readInt32LE(1);
            assert.equal(target, Number.parseInt(call[1], 16));
            return [{ offset: va + 1 - original.va, identity, address: target }];
        }
        const memory = instruction.match(/\[(0x[0-9a-f]+)\]/i);
        if (!memory) return [];
        const target = Number(memory[1]);
        const operand = va + hex.split(' ').length - 4;
        assert.equal(image.codeAt(operand, 4).readUInt32LE(), target);
        let identity = globals.get(target);
        if (!identity) {
            const size = sizes.get(target) || 8;
            const data = image.codeAt(target, size);
            literals.add(size === 4 ? data.readFloatLE() : data.readDoubleLE());
            identity = data.toString('hex');
        }
        return [{ offset: operand - original.va, identity, address: target }];
    });
    const originalCalls = new Set(originalRefs.filter(ref => calls.has(ref.address.toString(16))).map(ref => ref.identity));
    const rebuiltRefs = rebuilt.section.relocations.map(({ offset, symbol, type }) => {
        assert.equal(rebuilt.bytes.readUInt32LE(offset), 0);
        if (type === 20) {
            assert.equal(rebuilt.bytes[offset - 1], 0xe8);
            const identity = symbol.name.match(/^\?(\w+)@/)?.[1];
            assert(originalCalls.has(identity), symbol.name);
            if (identity === 'Evaluate') {
                const capacity = name === 'ReverserThrustFactor' ? 'BJ' : 'OH';
                assert(symbol.name.startsWith(`?Evaluate@?$TurbineCoefficientTable@$0${capacity}@@sim1@@`));
                assert.equal(coff.sections[symbol.section - 1]?.name, '.text');
            }
            return { offset, identity, symbol: symbol.name };
        }
        assert.equal(type, 6);
        const global = symbol.name.match(/^\?(\w+)@sim1@@3NA$/)?.[1];
        if (global && [...globals.values()].includes(global)) {
            assert(originalRefs.some(ref => ref.identity === global), symbol.name);
            return { offset, identity: global, symbol: symbol.name };
        }
        if (symbol.index === tableSymbol.index) {
            assert.equal(name, 'ReverserThrustFactor');
            return { offset, identity: 'Rebuilt reverser table', symbol: symbol.name };
        }
        const encoded = symbol.name.match(/^__real@([0-9a-f]{8}|[0-9a-f]{16})$/i);
        assert(encoded, symbol.name);
        const section = coff.sections[symbol.section - 1];
        const size = encoded[1].length / 2;
        assert(section && symbol.value + size <= section.size);
        const data = file.subarray(section.raw + symbol.value, section.raw + symbol.value + size);
        assert(data.equals(Buffer.from(encoded[1], 'hex').reverse()));
        assert(literals.has(size === 4 ? data.readFloatLE() : data.readDoubleLE()), symbol.name);
        return { offset, identity: data.toString('hex'), symbol: symbol.name };
    });
    for (const identity of originalCalls) {
        assert.equal(rebuiltRefs.filter(ref => ref.identity === identity).length, 1, identity);
    }
    return { name, ...compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length,
        originalRefs, rebuiltRefs, ranges), originalRefs, rebuiltRefs };
});
const report = { program, originalSHA256, objectSHA256: sha(file), functions,
    table: { status: 'Exact', bytes: reverserTable.length, originalHex: reverserTable.toString('hex'),
        rebuiltHex: rebuiltTable.toString('hex') } };
fs.writeFileSync(path.join(run, 'turbine-thrust-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of functions) {
    console.log(`${item.entry} ${item.name}: ${item.status}, ${item.originalBytes}/${item.rebuiltBytes} bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
console.log(`Exact: ${report.table.bytes} reverser table bytes`);
