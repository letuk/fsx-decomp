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
const original = originalListing(program, '20c47c90', image);
const caller = originalListing(program, '20c47420', image);

function directCallTarget(va) {
    const call = image.codeAt(va, 5);
    assert.equal(call[0], 0xe8);
    return va + 5 + call.readInt32LE(1);
}

assert.equal(directCallTarget(0x20c47698), original.va);
const airTableIdentity = 'sim1.dll 0x20bcc030';
const curves = [
    {name: 'staticThrustByPowerDensity', displacement: -0x190, call: 0x20c47f77},
    {name: 'staticThrustByBladePitch', displacement: -0x60, call: 0x20c47fb6},
    {name: 'staticThrustByAxialSpeed', displacement: -0xb8, call: 0x20c4803f}
];

// Replay the original literal loads and x87 stores that build the three local curves.
function readOriginalCurves() {
    const stack = [];
    const slots = new Map();
    for (const [address, , operation] of original.rows) {
        const va = Number.parseInt(address, 16);
        if (va === 0x20c47f1b) assert.deepEqual(stack, [1, 0, 10]);
        if (va >= 0x20c47e3f && va < 0x20c47f1b || va >= 0x20c47f68) continue;
        let match;
        if ((match = operation.match(/^FLD double ptr \[(0x[0-9a-f]+)\]$/))) {
            stack.unshift(image.codeAt(Number(match[1]), 8, false).readDoubleLE());
        } else if (operation === 'FLD1' || operation === 'FLDZ') {
            stack.unshift(operation === 'FLD1' ? 1 : 0);
        } else if ((match = operation.match(/^FST(P?) double ptr \[EBP \+ (-?0x[0-9a-f]+)\]$/))) {
            assert(stack.length);
            const offset = Number.parseInt(match[2], 16) | 0;
            slots.set(offset, match[1] ? stack.shift() : stack[0]);
        } else if ((match = operation.match(/^FXCH(?: ST(\d))?$/))) {
            const index = Number(match[1] || 1);
            assert(stack.length > index);
            [stack[0], stack[index]] = [stack[index], stack[0]];
        } else if ((match = operation.match(/^MOV dword ptr \[EBP \+ (-?0x[0-9a-f]+)\],(0x[0-9a-f]+)$/))) {
            slots.set(Number.parseInt(match[1], 16) | 0, Number(match[2]));
        } else {
            assert(!/^F/.test(operation), operation);
        }
    }
    assert.equal(stack.length, 0);
    return curves.map(curve => {
        const size = slots.get(curve.displacement);
        assert(size === 5 || size === 13);
        const bytes = Buffer.alloc(4 + size * 16);
        bytes.writeUInt32LE(size);
        const points = Array.from({length: size}, (_, i) => {
            const index = 4 + 16 * i;
            const coordinates = [slots.get(curve.displacement + index), slots.get(curve.displacement + index + 8)];
            assert(coordinates.every(Number.isFinite));
            bytes.writeDoubleLE(coordinates[0], index);
            bytes.writeDoubleLE(coordinates[1], index + 8);
            return coordinates;
        });
        return {...curve, points, bytes};
    });
}

const originalCurves = readOriginalCurves();
const refs = image.highLowFixups(original.va, original.bytes.length).map(offset => {
    const va = original.bytes.readUInt32LE(offset);
    return {offset, address: va.toString(16), identity: image.codeAt(va, 8, false).toString('hex')};
});
const literalIdentities = refs.map(ref => ref.identity);
assert.equal(directCallTarget(0x20c47ecc), 0x20bcc030);
refs.push({offset: 0x20c47ecd - original.va, address: '20bcc030', identity: airTableIdentity});
for (const {name, call} of originalCurves) {
    assert.equal(directCallTarget(call), 0x20bcbf60);
    refs.push({offset: call + 1 - original.va, address: '20bcbf60', identity: name});
}

const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);
const symbol = 'PropellerForceRecord::UpdateThrust';
const rebuilt = nativeCoffBody(file, coff, dump, '?UpdateThrust@PropellerForceRecord@sim1@@');
const rebuiltTable = nativeCoffBody(file, coff, dump, '?Evaluate@PropellerCoefficientTable@sim1@@');
const rebuiltCurve = {symbol: coff.unique('?SampleAerodynamicCurve@sim1@@YINPBUAerodynamicCurve@1@N@Z')};
assert.equal(rebuiltCurve.symbol.section, 0);
const rebuiltRefs = [];
const curveRefs = new Map();
for (const relocation of rebuilt.section.relocations) {
    const {offset, symbol: target, type} = relocation;
    const localCurve = originalCurves.find(({name}) => target.name.startsWith('?' + name + '@'));
    if (localCurve) {
        assert.equal(type, 6);
        assert.equal(rebuilt.bytes[offset - 1], 0xb9, 'Expected the curve address in ECX');
        assert.equal(rebuilt.bytes.readUInt32LE(offset), 0);
        const data = coff.sections[target.section - 1];
        assert(data && target.value + localCurve.bytes.length <= data.size);
        const start = data.raw + target.value;
        assert(file.subarray(start, start + localCurve.bytes.length).equals(localCurve.bytes), localCurve.name);
        curveRefs.set(offset, localCurve.name);
        rebuiltRefs.push({offset, symbol: target.name, identity: localCurve.name});
    } else {
        const isCurve = target.index === rebuiltCurve.symbol.index;
        const identity = isCurve ? curveRefs.get(Math.max(...[...curveRefs.keys()].filter(key => key < offset))) : airTableIdentity;
        if (isCurve) assert(identity, 'Missing curve address before lookup');
        rebuiltRefs.push(...nativeCoffReferences(file, coff, {...rebuilt, section: {...rebuilt.section,
            relocations: [relocation]}}, isCurve ? rebuiltCurve : rebuiltTable, identity, literalIdentities));
    }
}
assert.equal(curveRefs.size, curves.length);
assert.deepEqual(rebuiltRefs.filter(ref => ref.symbol === rebuiltCurve.symbol.name).map(ref => ref.identity).sort(),
    curves.map(curve => curve.name).sort());

const comparison = compareNativeRanges(original, rebuilt, original.va, original.va + original.bytes.length, refs, rebuiltRefs, [
    [0x20c47c90, 0x20c47c95, 'Hotpatch entry and frame setup'],
    [0x20c47e45, 0x20c47e52, 'Compare absolute axial speed with the small-speed threshold'],
    [0x20c47e73, 0x20c47e79, 'Multiply the reverse factor by the negative efficiency constant'],
    [0x20c47ebc, 0x20c47ec6, 'Convert the pitch to degrees and store the table argument'],
    [0x20c47ec9, 0x20c47ed1, 'Pass advance ratio and call the thrust-efficiency table'],
    [0x20c47f6b, 0x20c47f71, 'Convert aerodynamic power to horsepower'],
    [0x20c47f74, 0x20c47f7c, 'Pass the power density to the static-thrust curve'],
    [0x20c47f87, 0x20c47f92, 'Convert shaft power to horsepower and apply the static-thrust factor'],
    [0x20c47fad, 0x20c47fbb, 'Convert pitch and call the low-speed blade-pitch curve'],
    [0x20c47fbb, 0x20c47fbe, 'Apply the low-speed pitch factor'],
    [0x20c4800e, 0x20c4801d, 'Check the pitch tolerance for the feathered low-speed path'],
    [0x20c4803c, 0x20c48047, 'Call the axial-speed curve and apply the low-speed factor'],
    [0x20c48083, 0x20c48089, 'Scale the bounded propeller load factor by 0.2'],
    [0x20c4809b, 0x20c480a1, 'Restore the frame and remove the original stack arguments']
]);

const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), symbol,
    caller: {entry: caller.va.toString(16), call: '20c47698'},
    curves: originalCurves.map(({name, points, bytes}) => ({name, points, bytes: bytes.length, status: 'Exact'})),
    ...comparison};
fs.writeFileSync(path.join(run, 'propeller-thrust-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${comparison.entry} ${comparison.status}: ${comparison.originalBytes} original / ${comparison.rebuiltBytes} native COFF bytes`);
for (const scope of comparison.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
for (const curve of report.curves) console.log(`${curve.status}: ${curve.bytes} table bytes, ${curve.name}`);
