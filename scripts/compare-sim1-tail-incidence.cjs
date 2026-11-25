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
const coefficientVA = 0x20c500c0;
const slot = 0x20bc267c;
assert.deepEqual(image.highLowFixups(slot, 4), [0]);
assert.equal(image.initialDword(slot), coefficientVA);

const coefficient = originalListing(program, '20c500c0', image);
const parent = originalListing(program, '20c013a0', image);
assert.equal(image.codeAt(0x20c013c2, 10).toString('hex'), '8b838019000089442448');
assert.equal(image.codeAt(0x20c01485, 3).toString('hex'), '8b4264');
assert.equal(image.codeAt(0x20c01494, 12).toString('hex'), 'ffd08b4c2448dc89cc000000');
const literals = new Map([[0x20bb18e8, 'ccdb0e400002403f'], [0x20bc3cb0, '333333333333c33f']]);
const originalRefs = image.highLowFixups(coefficient.va, coefficient.bytes.length).map(offset => {
    const address = coefficient.bytes.readUInt32LE(offset);
    const identity = image.codeAt(address, 8, false).toString('hex');
    assert.equal(identity, literals.get(address));
    return { offset, address: address.toString(16), identity };
});
assert.deepEqual(originalRefs.map(ref => ref.identity).sort(), [...literals.values()].sort());
const callVA = 0x20c50144;
const callOffset = callVA - coefficientVA;
assert.equal(coefficient.bytes[callOffset], 0xe8);
assert.equal(callVA + 5 + coefficient.bytes.readInt32LE(callOffset + 1), 0x20bcbf60);
const curveIdentity = 'sim1.dll 0x20bcbf60';
originalRefs.push({ offset: callOffset + 1, address: '20bcbf60', identity: curveIdentity });

const curve = nativeCoffBody(file, coff, listing, '?SampleAerodynamicCurve@sim1@@');
const newCoefficient = nativeCoffBody(file, coff, listing, '?CalculateHorizontalTailIncidenceCoefficient@AirplaneCoefficientModel@sim1@@');
const newMoment = nativeCoffBody(file, coff, listing, '?CalculateHorizontalTailIncidenceMoment@AirplaneMomentModel@sim1@@');
const rebuiltRefs = nativeCoffReferences(file, coff, newCoefficient, curve, curveIdentity, [...literals.values()]);
assert.deepEqual(rebuiltRefs.map(ref => ref.identity).sort(), originalRefs.map(ref => ref.identity).sort());
const momentRefs = nativeCoffReferences(file, coff, newMoment, newCoefficient, 'sim1.dll 0x20c500c0', []);
assert.deepEqual(momentRefs.map(ref => ref.identity), ['sim1.dll 0x20c500c0']);

const report = {
    program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    coefficientSlot: slot.toString(16),
    coefficient: {
        ...compareNativeRanges(coefficient, newCoefficient, coefficientVA, coefficientVA + coefficient.bytes.length,
            originalRefs, rebuiltRefs, [
                [0x20c500ea, 0x20c500f1, 'Query the airplane mode controlling Mach correction'],
                [0x20c50106, 0x20c5010b, 'Read the lower Mach sample and form the difference'],
                [0x20c5011e, 0x20c50124, 'Scale the interpolated Mach correction'],
                [0x20c5012f, 0x20c50135, 'Load the raw angle of attack'],
                [0x20c5015c, 0x20c50175, 'Select the null default or call the modifier and compute its loss'],
                [0x20c5017f, 0x20c50189, 'Calculate the weighted control modifier']
            ]),
        symbol: 'AirplaneCoefficientModel::CalculateHorizontalTailIncidenceCoefficient',
        originalRefs, rebuiltRefs
    },
    moment: {
        ...compareNativeRanges(parent, newMoment, 0x20c01494, 0x20c014bc, [], momentRefs, [
            [0x20c0149a, 0x20c014a0, 'Multiply by horizontal-tail incidence'],
            [0x20c014a9, 0x20c014af, 'Apply pressure and geometry scaling']
        ]),
        symbol: 'AirplaneMomentModel::CalculateHorizontalTailIncidenceMoment', parentEntry: '20c013a0',
        rebuiltRefs: momentRefs
    }
};
fs.writeFileSync(path.join(run, 'tail-incidence-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [report.coefficient, report.moment]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} native COFF bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
