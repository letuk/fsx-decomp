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
const coefficientVA = 0x20c4fe00;
const slot = 0x20bc2678;
assert.deepEqual(image.highLowFixups(slot, 4), [0]);
assert.equal(image.initialDword(slot), coefficientVA);

const coefficient = originalListing(program, '20c4fe00', image);
const parent = originalListing(program, '20c013a0', image);
const callStart = 0x20c01451 - parent.va;
assert.equal(parent.bytes.subarray(callStart, callStart + 7).toString('hex'), '8b168b4260dc8f');
assert.equal(image.codeAt(0x20c01462, 2).toString('hex'), 'ffd0');
assert.equal(image.codeAt(0x20c4febc, 6).toString('hex'), '8b511850ffd2');
assert.equal(image.codeAt(0x20c4ff0c, 6).toString('hex'), '8b511850ffd2');

const literals = new Map([[0x20bb18e8, 'ccdb0e400002403f'], [0x20bc3cb0, '333333333333c33f']]);
const originals = image.highLowFixups(coefficient.va, coefficient.bytes.length).map(offset => {
    const address = coefficient.bytes.readUInt32LE(offset);
    const identity = image.codeAt(address, 8, false).toString('hex');
    assert.equal(identity, literals.get(address));
    return { offset, address: address.toString(16), identity };
});
assert.deepEqual(originals.map(item => item.address), ['20bb18e8', '20bc3cb0', '20bc3cb0']);
const callVA = 0x20c4fe51;
const curveVA = 0x20bcbf60;
const callOffset = callVA - coefficient.va;
assert.equal(coefficient.bytes[callOffset], 0xe8);
assert.equal(callVA + 5 + coefficient.bytes.readInt32LE(callOffset + 1), curveVA);
const curveIdentity = 'sim1.dll 0x20bcbf60';
originals.push({ offset: callOffset + 1, address: '20bcbf60', identity: curveIdentity });

const curve = nativeCoffBody(file, coff, listing, '?SampleAerodynamicCurve@sim1@@');
const newCoefficient = nativeCoffBody(file, coff, listing, '?CalculatePitchRateCoefficient@AirplaneCoefficientModel@sim1@@');
const newMoment = nativeCoffBody(file, coff, listing, '?CalculatePitchRateMoment@AirplaneMomentModel@sim1@@');
const rebuiltRefs = nativeCoffReferences(file, coff, newCoefficient, curve, curveIdentity, [...literals.values()]);
assert.deepEqual(rebuiltRefs.map(ref => ref.identity).sort(), originals.map(ref => ref.identity).sort());
const momentRefs = nativeCoffReferences(file, coff, newMoment, newCoefficient, 'sim1.dll 0x20c4fe00', []);
assert.deepEqual(momentRefs.map(ref => ref.identity), ['sim1.dll 0x20c4fe00']);

const report = {
    program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    coefficientSlot: slot.toString(16),
    coefficient: {
        ...compareNativeRanges(coefficient, newCoefficient, coefficientVA, coefficientVA + coefficient.bytes.length,
            originals, rebuiltRefs, [
                [0x20c4fe7f, 0x20c4fe8a, 'Scale, add and apply the Mach correction'],
                [0x20c4fe7f, 0x20c4fe85, 'Scale the interpolated Mach correction'],
                [0x20c4fe8f, 0x20c4fea1, 'Calculate and apply the realism weighting'],
                [0x20c4febc, 0x20c4feca, 'Call the control modifier and form its complement'],
                [0x20c4fedf, 0x20c4fee5, 'Scale the control modifier loss'],
                [0x20c4feed, 0x20c4fef3, 'Apply the weighted control modifier']
            ]),
        symbol: 'AirplaneCoefficientModel::CalculatePitchRateCoefficient', originalRefs: originals, rebuiltRefs
    },
    moment: {
        ...compareNativeRanges(parent, newMoment, 0x20c01462, 0x20c0147b, [], momentRefs, [
            [0x20c01464, 0x20c0146a, 'Multiply by the integrated angular rate'],
            [0x20c01471, 0x20c01477, 'Multiply by the aerodynamic rate scale']
        ]),
        symbol: 'AirplaneMomentModel::CalculatePitchRateMoment', parentEntry: '20c013a0', rebuiltRefs: momentRefs
    }
};
fs.writeFileSync(path.join(run, 'pitch-rate-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [report.coefficient, report.moment]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} native COFF bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
