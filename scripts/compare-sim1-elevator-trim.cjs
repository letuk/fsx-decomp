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
const coefficientVA = 0x20c501a0;
const slot = 0x20bc2684;
const curveVA = 0x20bcbf60;
assert.deepEqual(image.highLowFixups(slot, 4), [0]);
assert.equal(image.initialDword(slot), coefficientVA);

const original = originalListing(program, '20c501a0', image);
const parent = originalListing(program, '20c013a0', image);
assert.deepEqual(image.highLowFixups(original.va, original.bytes.length), []);
assert.equal(image.codeAt(0x20c014be, 11).toString('hex'), 'dd80fc0000008b068b506c');
assert.equal(image.codeAt(0x20c014cf, 2).toString('hex'), 'ffd2');
assert.equal(image.codeAt(0x20c50228, 6).toString('hex'), '8b511850ffd2');

const curve = nativeCoffBody(file, coff, listing, '?SampleAerodynamicCurve@sim1@@');
const coefficient = nativeCoffBody(file, coff, listing, '?CalculateElevatorTrimCoefficient@AirplaneCoefficientModel@sim1@@');
const moment = nativeCoffBody(file, coff, listing, '?CalculateElevatorTrimMoment@AirplaneMomentModel@sim1@@');
const curveRefs = nativeCoffReferences(file, coff, coefficient, curve, 'sim1.dll 0x20bcbf60', [])
    .sort((left, right) => left.offset - right.offset);
const momentRefs = nativeCoffReferences(file, coff, moment,
    coefficient, 'sim1.dll 0x20c501a0', []);
assert.equal(curveRefs.length, 2);
assert.deepEqual(momentRefs.map(ref => ref.identity), ['sim1.dll 0x20c501a0']);

// Give the two calls distinct identities after checking both arguments and the shared callee.
const inputs = [
    { call: 0x20c501d7, curve: 'b10', argument: '194', identity: 'AIR 0x536 raw-alpha curve' },
    { call: 0x20c50203, curve: '57c', argument: '1dc', identity: 'configuration dynamic pressure curve' }
];
const originalRefs = inputs.map(({ call, curve, argument, identity }, index) => {
    const originalOffset = call - original.va;
    assert.equal(original.bytes[originalOffset], 0xe8);
    assert.equal(call + 5 + original.bytes.readInt32LE(originalOffset + 1), curveVA);
    const originalSetupStart = index === 0 ? original.va : inputs[index - 1].call + 5;
    const originalSetup = original.rows.filter(([address]) => {
        const va = Number.parseInt(address, 16);
        return va >= originalSetupStart && va < call;
    }).map(row => row[2].toLowerCase());
    assert(originalSetup.some(instruction => instruction.includes('+ 0x' + curve + ']')));
    assert(originalSetup.some(instruction => instruction.includes('+ 0x' + argument + ']')));

    const ref = curveRefs[index];
    const rebuiltSetupStart = index === 0 ? 0 : curveRefs[index - 1].offset + 4;
    const rebuiltSetup = coefficient.rows.filter(([address]) => {
        const offset = Number.parseInt(address, 16);
        return offset >= rebuiltSetupStart && offset < ref.offset - 1;
    }).map(row => row[2].toLowerCase());
    assert(rebuiltSetup.some(instruction => new RegExp('^lea .*\\+0?' + curve + 'h\\]$').test(instruction)));
    assert(rebuiltSetup.some(instruction => new RegExp('^fld .*\\+0?' + argument + 'h\\]$').test(instruction)));
    ref.identity = identity;
    return { offset: originalOffset + 1, address: '20bcbf60', identity };
});

const report = {
    program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    coefficientSlot: slot.toString(16),
    coefficient: {
        ...compareNativeRanges(original, coefficient, coefficientVA, coefficientVA + original.bytes.length,
            originalRefs, curveRefs, [
                [0x20c501d4, 0x20c501dc, 'Pass raw alpha and call the shared curve lookup'],
                [0x20c50200, 0x20c50208, 'Pass dynamic pressure and call the shared curve lookup'],
                [0x20c5021d, 0x20c50224, 'Query the control surfaces and test their provider'],
                [0x20c50226, 0x20c50232, 'Call the trim modifier and calculate its loss'],
                [0x20c5023a, 0x20c5023e, 'Subtract the rounded loss from one']
            ]),
        symbol: 'AirplaneCoefficientModel::CalculateElevatorTrimCoefficient',
        originalRefs, rebuiltRefs: curveRefs
    },
    moment: {
        ...compareNativeRanges(parent, moment, 0x20c014bc, 0x20c014e6, [], momentRefs, [
            [0x20c014be, 0x20c014c4, 'Load signed elevator trim from the control block'],
            [0x20c014da, 0x20c014e0, 'Apply pressure and geometry scaling']
        ]),
        symbol: 'AirplaneMomentModel::CalculateElevatorTrimMoment', parentEntry: '20c013a0', rebuiltRefs: momentRefs
    }
};
fs.writeFileSync(path.join(run, 'elevator-trim-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [report.coefficient, report.moment]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} native COFF bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
