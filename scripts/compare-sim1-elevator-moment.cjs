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
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const file = fs.readFileSync(path.join(run, 'sim1-aerodynamic-moment.obj'));
const coff = parseNativeCoff(file);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const hex = address => address.toString(16);
const coefficientVA = 0x20c4fce0;
const curveVA = 0x20bcbf60;
const slots = [[0x20bc2674, coefficientVA], [0x20bbede8, 0x20be83e0]];
for (const [address, target] of slots) {
    assert.deepEqual(image.highLowFixups(address, 4), [0]);
    assert.equal(image.initialDword(address), target);
}
assert.equal(image.codeAt(0x20be83e0, 7).toString('hex'), '8b8134030000c3');

const coefficient = originalListing(program, hex(coefficientVA), image);
const parent = originalListing(program, '20c013a0', image);
const literalByAddress = new Map([
    [0x20bb18e8, 'ccdb0e400002403f'],
    [0x20bc4198, '989999999999b93f'],
    [0x20bb47b8, 'cdccccccccccec3f']
]);
const originals = image.highLowFixups(coefficient.va, coefficient.bytes.length).map(offset => {
    const address = coefficient.bytes.readUInt32LE(offset);
    const identity = image.codeAt(address, 8, false).toString('hex');
    assert.equal(identity, literalByAddress.get(address));
    return { offset, address: hex(address), identity };
});
assert.equal(originals.length, literalByAddress.size);
const curveIdentity = 'sim1.dll 0x20bcbf60';
for (const address of [0x20c4fd96, 0x20c4fdb8]) {
    const offset = address - coefficient.va;
    assert.equal(coefficient.bytes[offset], 0xe8);
    assert.equal(address + 5 + coefficient.bytes.readInt32LE(offset + 1), curveVA);
    originals.push({ offset: offset + 1, address: hex(curveVA), identity: curveIdentity });
}
const ownerCall = parent.bytes.subarray(0x20c01429 - parent.va, 0x20c01434 - parent.va);
assert.equal(ownerCall.toString('hex'), '8b49048b118b8218010000');
const coefficientCall = parent.bytes.subarray(0x20c01446 - parent.va, 0x20c0144d - parent.va);
assert.equal(coefficientCall.toString('hex'), '8b425c8bceffd0');

const curve = nativeCoffBody(file, coff, listing, '?SampleAerodynamicCurve@sim1@@');
const newCoefficient = nativeCoffBody(file, coff, listing, '?CalculateElevatorCoefficient@AirplaneCoefficientModel@sim1@@');
const newMoment = nativeCoffBody(file, coff, listing, '?CalculateElevatorMoment@AirplaneMomentModel@sim1@@');

const rebuiltRefs = nativeCoffReferences(file, coff, newCoefficient, curve, curveIdentity, [...literalByAddress.values()]);
assert.deepEqual(rebuiltRefs.map(ref => ref.identity).sort(), originals.map(ref => ref.identity).sort());
const momentRefs = nativeCoffReferences(file, coff, newMoment, newCoefficient, 'sim1.dll 0x20c4fce0', []);
assert.deepEqual(momentRefs.map(ref => ref.identity), ['sim1.dll 0x20c4fce0']);

const report = {
    program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    slots: slots.map(([address, target]) => ({ address: hex(address), target: hex(target) })),
    coefficient: {
        ...compareNativeRanges(coefficient, newCoefficient, coefficient.va, coefficient.va + coefficient.bytes.length,
            originals, rebuiltRefs, [
                [0x20c4fd35, 0x20c4fd4e, 'Interpolate, scale and store the Mach correction'],
                [0x20c4fd40, 0x20c4fd46, 'Scale the interpolated Mach correction'],
                [0x20c4fd5b, 0x20c4fd6c, 'Multiply the absolute trim by its coupling coefficient'],
                [0x20c4fd84, 0x20c4fd8a, 'Read the elevator deflection'],
                [0x20c4fdd2, 0x20c4fdec, 'Apply realism weighting and configured elevator effectiveness'],
                [0x20c4fdd2, 0x20c4fdd8, 'Multiply by the slope of the realism factor'],
                [0x20c4fddb, 0x20c4fded, 'Apply the realism factor and configured effectiveness and return']
            ]),
        symbol: 'AirplaneCoefficientModel::CalculateElevatorCoefficient', originalRefs: originals, rebuiltRefs
    },
    moment: {
        ...compareNativeRanges(parent, newMoment, 0x20c01438, 0x20c01462, [], momentRefs, [
            [0x20c01438, 0x20c01440, 'Query the control surfaces and load the elevator deflection'],
            [0x20c01456, 0x20c0145c, 'Apply pressure and geometry scaling']
        ]),
        symbol: 'AirplaneMomentModel::CalculateElevatorMoment', parentEntry: hex(parent.va), rebuiltRefs: momentRefs
    }
};
fs.writeFileSync(path.join(run, 'elevator-moment-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [report.coefficient, report.moment]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} native COFF bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
