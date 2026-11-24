const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { compareBodies, dumpbinRows, originalListing, verifyRows } = require('./lib/reconstruction-listings.cjs');

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
const correctionSymbols = new Map([
    [0x20c7b92c, '?highAlphaReferenceXShift@sim1@@3NA'],
    [0x20c7b93c, '?highAlphaReferenceZShift@sim1@@3NA'],
]);
const hex = value => value.toString(16);
const referenceSlots = [
    { slot: 0x20bbedec, adapter: 0x20be8420, childSlot: 0x20bc2158, getter: 0x20c454f0, field: 0x1404 },
    { slot: 0x20bbedf0, adapter: 0x20be8440, childSlot: 0x20bc215c, getter: 0x20c45500, field: 0x1408 },
];
for (const { slot, adapter, childSlot, getter, field } of referenceSlots) {
    for (const [address, target] of [[slot, adapter], [childSlot, getter]]) {
        assert.deepEqual(image.highLowFixups(address, 4), [0]);
        assert.equal(image.initialDword(address), target);
    }
    const adapterBytes = image.codeAt(adapter, 19);
    assert.equal(adapterBytes.readUInt32LE(2), 0x330);
    assert.equal(adapterBytes.readUInt32LE(8), 0x19fc);
    assert.equal(adapterBytes[16], childSlot - 0x20bc213c);
    assert.equal(adapterBytes.readUInt16LE(17), 0xe0ff);
    const getterBytes = image.codeAt(getter, 7);
    assert.equal(getterBytes.readUInt16LE(), 0x81d9);
    assert.equal(getterBytes.readUInt32LE(2), field);
    assert.equal(getterBytes[6], 0xc3);
}

function select(rows, start, end) {
    return rows.filter(([address]) => {
        const va = Number.parseInt(address, 16);
        return va >= start && va < end;
    });
}

const specs = [
    { component: 'X', parent: '20c013a0', begin: 0x20c01733, end: 0x20c01775, scopes: [
        [0x20c01753, 0x20c01769, 'Apply both force lever arms'],
        [0x20c0175a, 0x20c01763, 'Apply the vertical correction to the second force component'],
        [0x20c01760, 0x20c01769, 'Multiply both force lever arms'],
    ] },
    { component: 'Y', parent: '20c020a0', begin: 0x20c02645, end: 0x20c02680, scopes: [
        [0x20c02645, 0x20c02654, 'Calculate and store the first lever-arm product'],
        [0x20c02647, 0x20c02650, 'Apply the longitudinal correction to the third force component'],
        [0x20c02676, 0x20c02680, 'Subtract the second product and store the moment'],
    ] },
    { component: 'Z', parent: '20c01890', begin: 0x20c02025, end: 0x20c0203d, scopes: [
        [0x20c02025, 0x20c0203d, 'Calculate the third force-transfer moment'],
        [0x20c02025, 0x20c0202d, 'Read the force vector and calculate the first lever-arm product'],
        [0x20c02028, 0x20c0202d, 'Multiply the lateral arm by the first force component'],
        [0x20c02031, 0x20c02037, 'Apply the high-angle longitudinal correction'],
        [0x20c02038, 0x20c0203d, 'Multiply and subtract the corrected longitudinal arm term'],
    ] },
];

const components = specs.map(({ component, parent, begin, end, scopes }) => {
    const original = originalListing(program, parent, image);
    const old = original.bytes.subarray(begin - original.va, end - original.va);
    const asm = select(original.rows, begin, end);
    verifyRows(asm, begin, old);
    const method = `CalculateForceMoment${component}`;
    const names = [...coff.byName.keys()].filter(name => name.startsWith(`?${method}@AirplaneMomentModel@sim1@@`));
    assert.equal(names.length, 1, method);
    const symbol = coff.unique(names[0]);
    const section = coff.sections[symbol.section - 1];
    assert.equal(section.name, '.text');
    assert.equal(symbol.value, 0);
    const rebuilt = file.subarray(section.raw, section.raw + section.size);
    const start = listing.indexOf(`\n${symbol.name} (`);
    assert(start >= 0, symbol.name);
    const rebuiltRows = dumpbinRows(listing.slice(start).split(/\r?\n\s*\r?\n/, 1)[0]);
    verifyRows(rebuiltRows, 0, rebuilt);
    const originalReferences = image.highLowFixups(begin, old.length).map(offset => {
        const address = old.readUInt32LE(offset);
        const identity = correctionSymbols.get(address);
        assert(identity, `Unknown original operand ${hex(address)}`);
        return { offset, address: hex(address), identity };
    });
    const rebuiltReferences = section.relocations.map(({ offset, type, symbol }) => {
        assert.equal(type, 6);
        assert.equal(symbol.section, 0);
        assert.equal(rebuilt.readUInt32LE(offset), 0);
        assert([...correctionSymbols.values()].includes(symbol.name));
        return { offset, identity: symbol.name };
    });
    assert.deepEqual(originalReferences.map(item => item.identity).sort(),
        rebuiltReferences.map(item => item.identity).sort());
    const matches = [];
    for (const [first, last, name] of scopes) {
        const size = last - first;
        const offset = first - begin;
        const originalScope = old.subarray(offset, offset + size);
        const asmScope = select(asm, first, last);
        verifyRows(asmScope, first, originalScope);
        const leftRefs = originalReferences.filter(item => item.offset >= offset && item.offset < offset + size);
        for (const [address] of rebuiltRows) {
            const rebuiltStart = Number.parseInt(address, 16);
            const bytes = rebuilt.subarray(rebuiltStart, rebuiltStart + size);
            if (bytes.length !== size) break;
            const rightRefs = rebuiltReferences.filter(item => item.offset >= rebuiltStart && item.offset < rebuiltStart + size);
            const pairs = leftRefs.flatMap(item => {
                const relative = item.offset - offset;
                return rightRefs.some(ref => ref.identity === item.identity && ref.offset - rebuiltStart === relative)
                    ? [{ originalOffset: relative, rebuiltOffset: relative, identity: item.identity, address: item.address }] : [];
            });
            const result = compareBodies(originalScope, bytes, pairs, pairs.length === leftRefs.length && pairs.length === rightRefs.length);
            if (result.status === 'Different') continue;
            const rebuiltScope = select(rebuiltRows, rebuiltStart, rebuiltStart + size);
            if (rebuiltScope.reduce((size, row) => size + row[1].split(' ').length, 0) !== size) continue;
            verifyRows(rebuiltScope, rebuiltStart, bytes);
            matches.push({ entry: hex(first), name, bytes: size, rebuiltEntry: address, ...result, pairs,
                asm: asmScope, rebuilt: rebuiltScope, leftHex: originalScope.toString('hex'), rightHex: bytes.toString('hex') });
        }
    }
    return { component, entry: hex(begin), endExclusive: hex(end), parentEntry: parent,
        symbol: `AirplaneMomentModel::${method}`, buildSymbol: symbol.name, status: 'Different',
        originalBytes: old.length, rebuiltBytes: rebuilt.length, asm, rebuilt: rebuiltRows,
        leftHex: old.toString('hex'), rightHex: rebuilt.toString('hex'),
        originalReferences, rebuiltReferences, scopes: matches };
});
const report = { program, artifactKind: 'native x86 COFF', originalSHA256, rebuiltSHA256: sha(file), referenceSlots, components };
fs.writeFileSync(path.join(run, 'moment-arm-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of components) {
    console.log(`${item.parentEntry} ${item.component}: ${item.originalBytes} original / ${item.rebuiltBytes} extracted bytes`);
    for (const scope of item.scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
}
