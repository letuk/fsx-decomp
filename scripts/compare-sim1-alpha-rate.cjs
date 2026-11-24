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
const slot = 0x20bc2680;
const coefficientVA = 0x20c50020;
assert.deepEqual(image.highLowFixups(slot, 4), [0]);
assert.equal(image.initialDword(slot), coefficientVA);

const coefficient = originalListing(program, '20c50020', image);
const producer = originalListing(program, '20bfb840', image);
const parent = originalListing(program, '20c013a0', image);
const producerEnd = 0x20bfb8dc;
assert.equal(image.codeAt(0x20c0146a, 7).toString('hex'), '8b168b42688bce');
assert.equal(image.codeAt(0x20c0147b, 2).toString('hex'), 'ffd0');
assert.equal(image.codeAt(producer.va, 6).toString('hex'), '8b8a70020000');
const callProducer = image.codeAt(0x20c2e1e2, 7);
assert.equal(callProducer.subarray(0, 3).toString('hex'), '8bd6e8');
assert.equal(0x20c2e1e9 + callProducer.readInt32LE(3), producer.va);

function originalLiterals(body, end, literals) {
    const refs = image.highLowFixups(body.va, end - body.va).map(offset => {
        const address = body.bytes.readUInt32LE(offset);
        const identity = image.codeAt(address, 8, false).toString('hex');
        assert.equal(identity, literals.get(address));
        return { offset, address: address.toString(16), identity };
    });
    assert.deepEqual(refs.map(ref => ref.identity).sort(), [...literals.values()].sort());
    return refs;
}

const coefficientLiterals = new Map([[0x20bb18e8, 'ccdb0e400002403f']]);
const producerLiterals = new Map([[0x20bc3fb0, 'a915a15ddeecdbbf'], [0x20bbf2b0, 'a915a15ddeecdb3f']]);
const coefficientRefs = originalLiterals(coefficient, 0x20c500b0, coefficientLiterals);
const producerRefs = originalLiterals(producer, producerEnd, producerLiterals);
const callVA = 0x20c500a2;
const curveVA = 0x20bcbf60;
const callOffset = callVA - coefficient.va;
assert.equal(coefficient.bytes[callOffset], 0xe8);
assert.equal(callVA + 5 + coefficient.bytes.readInt32LE(callOffset + 1), curveVA);
const curveIdentity = 'sim1.dll 0x20bcbf60';
coefficientRefs.push({ offset: callOffset + 1, address: '20bcbf60', identity: curveIdentity });

const curve = nativeCoffBody(file, coff, listing, '?SampleAerodynamicCurve@sim1@@');
const newCoefficient = nativeCoffBody(file, coff, listing, '?CalculateAngleOfAttackRateCoefficient@AirplaneCoefficientModel@sim1@@');
const newProducer = nativeCoffBody(file, coff, listing, '?UpdateAngleOfAttackRate@IntegrationHistory@sim1@@');
const newMoment = nativeCoffBody(file, coff, listing, '?CalculateAngleOfAttackRateMoment@AirplaneMomentModel@sim1@@');
const rebuiltCoefficientRefs = nativeCoffReferences(file, coff, newCoefficient, curve, curveIdentity, [...coefficientLiterals.values()]);
const rebuiltProducerRefs = nativeCoffReferences(file, coff, newProducer, null, null, [...producerLiterals.values()]);
const rebuiltMomentRefs = nativeCoffReferences(file, coff, newMoment, newCoefficient, 'sim1.dll 0x20c50020', []);
assert.deepEqual(rebuiltCoefficientRefs.map(ref => ref.identity).sort(), coefficientRefs.map(ref => ref.identity).sort());
assert.deepEqual(rebuiltProducerRefs.map(ref => ref.identity).sort(), producerRefs.map(ref => ref.identity).sort());
assert.deepEqual(rebuiltMomentRefs.map(ref => ref.identity), ['sim1.dll 0x20c50020']);

const report = {
    program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    coefficientSlot: slot.toString(16),
    coefficient: {
        ...compareNativeRanges(coefficient, newCoefficient, coefficientVA, 0x20c500b0,
            coefficientRefs, rebuiltCoefficientRefs, [
                [0x20c5003c, 0x20c50044, 'Load the base coefficient and airplane dispatch'],
                [0x20c50048, 0x20c5004f, 'Query the airplane mode and test the result'],
                [0x20c50064, 0x20c50069, 'Read the lower Mach sample and form the difference'],
                [0x20c50071, 0x20c5008a, 'Interpolate, scale and store the Mach correction'],
                [0x20c5007c, 0x20c50082, 'Scale the interpolated Mach correction'],
                [0x20c5008d, 0x20c50093, 'Load the unfiltered angle of attack']
            ]),
        symbol: 'AirplaneCoefficientModel::CalculateAngleOfAttackRateCoefficient',
        originalRefs: coefficientRefs, rebuiltRefs: rebuiltCoefficientRefs
    },
    producer: {
        ...compareNativeRanges(producer, newProducer, producer.va, producerEnd,
            producerRefs, rebuiltProducerRefs, [
                [0x20bfb846, 0x20bfb860, 'Calculate planar speed squared and compare it with zero'],
                [0x20bfb860, 0x20bfb866, 'Load the negative angle-rate limit'],
                [0x20bfb869, 0x20bfb86f, 'Load the positive angle-rate limit'],
                [0x20bfb871, 0x20bfb88b, 'Calculate the angle-of-attack rate numerator'],
                [0x20bfb88f, 0x20bfb899, 'Store the rate and take its absolute value']
            ]),
        symbol: 'IntegrationHistory::UpdateAngleOfAttackRate', originalExtent: producer.bytes.length,
        originalRefs: producerRefs, rebuiltRefs: rebuiltProducerRefs
    },
    moment: {
        ...compareNativeRanges(parent, newMoment, 0x20c0147b, 0x20c01494, [], rebuiltMomentRefs, [
            [0x20c0147d, 0x20c01483, 'Multiply by the angle-of-attack rate'],
            [0x20c0148a, 0x20c01490, 'Multiply by the aerodynamic rate scale']
        ]),
        symbol: 'AirplaneMomentModel::CalculateAngleOfAttackRateMoment',
        parentEntry: '20c013a0', rebuiltRefs: rebuiltMomentRefs
    }
};
fs.writeFileSync(path.join(run, 'alpha-rate-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of [report.coefficient, report.producer, report.moment]) {
    console.log(`${item.entry} ${item.status}: ${item.originalBytes} original / ${item.rebuiltBytes} native COFF bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
