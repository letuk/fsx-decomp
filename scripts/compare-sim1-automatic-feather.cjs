const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {openImage} = require('./lib/pe32-dll.cjs');
const {parseNativeCoff} = require('./lib/coff.cjs');
const {originalListing, nativeCoffBody, compareNativeRanges} = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 Turbine run directory');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const file = fs.readFileSync(path.join(run, 'sim1-turbine.obj'));
const dump = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const coff = parseNativeCoff(file);

const torque = originalListing(program, '20c48250', image);
const arm = originalListing(program, '20c48270', image);
const step = originalListing(program, '20c47420', image);
const body = name => nativeCoffBody(file, coff, dump, name);
const rebuiltTorque = body('?HasAutomaticFeatherTorque@TurbineEngineState@sim1@@');
const rebuiltArmSection = body('?UpdateAutomaticFeatherArming@FlightDynamics@sim1@@');
const rebuiltStep = body('?UpdateMappedPropellers@FlightDynamics@sim1@@');
const electricalSymbols = [...coff.byName.keys()].filter(name =>
    name.startsWith('?CircuitAvailability@ElectricalRuntime@sim1@@'));
assert.equal(electricalSymbols.length, 1);
const electricalSymbol = coff.unique(electricalSymbols[0]);
const rebuiltIce = body('?UpdatePropellerLoadFactor@FlightDynamics@sim1@@');
const rebuiltPasses = body('?UpdateMappedPropellersAfterPreparation@FlightDynamics@sim1@@');

function callReference(original, va, target, identity) {
    const call = image.codeAt(va, 5);
    assert.equal(call[0], 0xe8);
    assert.equal(va + 5 + call.readInt32LE(1), target);
    return {offset: va + 1 - original.va, address: target.toString(16), identity};
}

const originalTable = 0x20c485a0;
const originalCases = [0x20c482cb, 0x20c48310, 0x20c48376, 0x20c4845e];
assert.deepEqual(image.highLowFixups(originalTable, 16), [0, 4, 8, 12]);
const originalTableBytes = image.codeAt(originalTable, 16);
for (const [i, address] of originalCases.entries()) {
    assert.equal(originalTableBytes.readUInt32LE(i * 4), address);
    assert(arm.rows.some(row => Number.parseInt(row[0], 16) === address));
}
const alignment = image.codeAt(0x20c4857d, 3);
assert.equal(image.codeAt(0x20c4857b, 2).toString('hex'), 'eb03');
assert.equal(arm.bytes.length - alignment.length, 811);

const literals = new Map([[0x20bc35a0, 0.17], [0x20bc4270, 88]]);
const originalLiterals = original => image.highLowFixups(original.va, original.bytes.length).map(offset => {
    const va = original.bytes.readUInt32LE(offset);
    if (va === originalTable && original === arm) {
        assert.equal(offset, 0x20c482c7 - arm.va);
        return {offset, address: va.toString(16), identity: 'arm case table'};
    }
    const bytes = image.codeAt(va, 8, false);
    assert.equal(bytes.readDoubleLE(), literals.get(va));
    return {offset, address: va.toString(16), identity: bytes.toString('hex')};
});

const tableReference = rebuiltArmSection.section.relocations.find(rel => rel.symbol.name.startsWith('$LN') &&
    rebuiltArmSection.bytes.subarray(rel.offset - 3, rel.offset).equals(Buffer.from([0xff, 0x24, 0x85])));
assert(tableReference, 'Expected switch table operand in the arming body');
assert.equal(tableReference.type, 6);
assert.equal(tableReference.symbol.section, rebuiltArmSection.section.index);
const rebuiltTable = tableReference.symbol.value;
assert.equal(rebuiltTable + 16, rebuiltArmSection.bytes.length);
const rebuiltCases = rebuiltArmSection.section.relocations.filter(rel => rel.offset >= rebuiltTable);
assert.equal(rebuiltCases.length, 4);
for (const [i, rel] of rebuiltCases.entries()) {
    assert.equal(rel.offset, rebuiltTable + i * 4);
    assert.equal(rel.type, 6);
    assert.equal(rel.symbol.section, rebuiltArmSection.section.index);
    assert.equal(rebuiltArmSection.bytes.readUInt32LE(rel.offset), 0);
    assert(rebuiltArmSection.rows.some(row => Number.parseInt(row[0], 16) === rel.symbol.value));
    if (i > 0) assert(rel.symbol.value > rebuiltCases[i - 1].symbol.value);
}
const rebuiltArm = {...rebuiltArmSection, bytes: rebuiltArmSection.bytes.subarray(0, rebuiltTable),
    rows: rebuiltArmSection.rows.filter(row => Number.parseInt(row[0], 16) < rebuiltTable)};

function nativeReferences(rebuilt, callees) {
    return rebuilt.section.relocations.filter(rel => rel.offset < rebuilt.bytes.length).map(({offset, symbol, type}) => {
        assert.equal(rebuilt.bytes.readUInt32LE(offset), 0);
        if (type === 20) {
            assert.equal(rebuilt.bytes[offset - 1], 0xe8);
            assert(callees.has(symbol.index), symbol.name);
            return {offset, symbol: symbol.name, identity: callees.get(symbol.index)};
        }
        assert.equal(type, 6);
        if (rebuilt === rebuiltArm && offset === tableReference.offset) {
            return {offset, symbol: symbol.name, identity: 'arm case table'};
        }
        assert.match(symbol.name, /^__real@[0-9a-f]{16}$/);
        const data = coff.sections[symbol.section - 1];
        assert(data && symbol.value + 8 <= data.size);
        const bytes = file.subarray(data.raw + symbol.value, data.raw + symbol.value + 8);
        assert.equal(bytes.toString('hex'), Buffer.from(symbol.name.slice(7), 'hex').reverse().toString('hex'));
        assert([...literals.values()].includes(bytes.readDoubleLE()));
        return {offset, symbol: symbol.name, identity: bytes.toString('hex')};
    });
}

const torqueCalls = [0x20c482e6, 0x20c4832f, 0x20c48395, 0x20c483cf, 0x20c483f1,
    0x20c4842e, 0x20c4847d, 0x20c484bb, 0x20c484fc, 0x20c4853d];
const armRefs = originalLiterals(arm).concat(
    callReference(arm, 0x20c4829e, 0x20bfee30, 'electrical query'),
    torqueCalls.map(va => callReference(arm, va, torque.va, 'torque gate')));
const nativeArmRefs = nativeReferences(rebuiltArm, new Map([
    [electricalSymbol.index, 'electrical query'], [rebuiltTorque.symbol.index, 'torque gate']]));
assert.equal(nativeArmRefs.filter(rel => rel.identity === 'torque gate').length, torqueCalls.length);
assert.equal(nativeArmRefs.filter(rel => rel.identity === 'electrical query').length, 1);

const start = step.va;
const end = 0x20c47456;
assert.deepEqual(image.highLowFixups(start, end - start), []);
const stepRefs = [callReference(step, 0x20c47441, arm.va, 'arming stage'),
    callReference(step, 0x20c47451, 0x20c48710, 'icing stage')];
const nativeStepRefs = nativeReferences(rebuiltStep, new Map([
    [rebuiltArm.symbol.index, 'arming stage'], [rebuiltIce.symbol.index, 'icing stage'],
    [rebuiltPasses.symbol.index, 'mapped passes']]));
assert.deepEqual(nativeStepRefs.map(rel => rel.identity), ['arming stage', 'icing stage', 'mapped passes']);

const comparisons = [
    {symbol: 'TurbineEngineState::HasAutomaticFeatherTorque',
        ...compareNativeRanges(torque, rebuiltTorque, torque.va, torque.va + torque.bytes.length,
            originalLiterals(torque), nativeReferences(rebuiltTorque, new Map()), [
                [0x20c48250, 0x20c48256, 'Load the verified 0.17 torque threshold'],
                [0x20c48259, 0x20c48269, 'Return the ordered torque comparison']
            ])},
    {symbol: 'FlightDynamics::UpdateAutomaticFeatherArming', originalBodyBytes: 811,
        ...compareNativeRanges(arm, rebuiltArm, arm.va, arm.va + arm.bytes.length, armRefs, nativeArmRefs, [
            [0x20c48270, 0x20c48273, 'Hotpatch entry and save EBX'],
            [0x20c4829e, 0x20c482a5, 'Call the electrical availability query and test its result'],
            [0x20c482bb, 0x20c482be, 'Compare the dispatch index with three'],
            [0x20c482c4, 0x20c482cb, 'Dispatch through the verified four-entry case table'],
            [0x20c482d5, 0x20c482db, 'Load the verified 88.0 turbine speed threshold'],
            [0x20c482dd, 0x20c482e2, 'Read x87 comparison flags for ordered speed'],
            [0x20c482e6, 0x20c482ed, 'Call the torque gate for the single-propeller path'],
            [0x20c4858f, 0x20c48592, 'Advance one propeller record while clearing flags'],
            [0x20c4859a, 0x20c4859e, 'Restore saved registers and return']
        ])},
    {symbol: 'FlightDynamics::UpdateMappedPropellers',
        ...compareNativeRanges(step, rebuiltStep, start, end, stepRefs, nativeStepRefs, [
            [0x20c47420, 0x20c47425, 'Hotpatch entry and frame setup'],
            [0x20c47441, 0x20c47446, 'Call automatic-feather preparation'],
            [0x20c47449, 0x20c4744c, 'Reserve stack space for the icing step'],
            [0x20c4744e, 0x20c47456, 'Pass the time step and call propeller icing preparation']
        ])}
];
const report = {program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file),
    thresholds: {turbineSpeed: 88, normalizedShaftTorque: 0.17}, armPadding: alignment.toString('hex'),
    tables: {originalTable, originalCases, rebuiltTable, rebuiltCases: rebuiltCases.map(rel => rel.symbol.value)},
    references: {armRefs, nativeArmRefs, stepRefs, nativeStepRefs}, comparisons};
fs.writeFileSync(path.join(run, 'automatic-feather-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const match of comparisons) {
    console.log(`${match.entry} ${match.status}: ${match.originalBytes} original / ${match.rebuiltBytes} native COFF span bytes`);
    for (const scope of match.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
