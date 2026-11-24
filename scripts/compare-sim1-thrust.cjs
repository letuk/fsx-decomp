const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');
const { compareBodies, originalListing, readLinkedBodies, verifyRows } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 linked thrust run directory');
const program = 'sim1.dll';
const entry = '20c01010';
const sha256 = bytes => createHash('sha256').update(bytes).digest('hex');
const originalBytes = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha256(originalBytes);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const originalImage = openImage(originalBytes);
const rebuiltBytes = fs.readFileSync(path.join(run, 'sim1-thrust.dll'));
const rebuiltImage = openImage(rebuiltBytes);
assert.equal(rebuiltImage.entryRva, 0, 'Expected a no-entry linked comparison DLL');

const original = originalListing(program, entry, originalImage);
const map = fs.readFileSync(path.join(run, 'sim1-thrust.map'), 'utf8');
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const { va: rebuiltVA, name: buildSymbol, rows, bytes: linkedBody } =
    readLinkedBodies(rebuiltImage, map, listing)('?AccumulateEngineThrust@sim1@@');
assert(rows.some(row => /^ret\b/i.test(row[2])), 'Missing linked thrust return');

function references(image, imports, body, address, instructions) {
    const fields = image.highLowFixups(address, body.length).map(offset => {
        const target = body.readUInt32LE(offset);
        const imported = imports.get(target);
        return { offset, target, kind: 'DIR32', identity: imported ? imported.dll + '!' + imported.name : null };
    });
    for (const [site, encoding] of instructions.filter(row => /^e8 /.test(row[1]))) {
        const offset = Number.parseInt(site, 16) - address + 1;
        const target = address + offset + 4 + body.readInt32LE(offset);
        const stub = image.codeAt(target, 6);
        const imported = stub.readUInt16LE() === 0x25ff ? imports.get(stub.readUInt32LE(2)) : undefined;
        fields.push({ offset, target, kind: 'REL32', identity: imported ? imported.dll + '!' + imported.name : null });
    }
    return fields;
}

const originalReferences = references(originalImage, readPE32Imports(originalBytes), original.bytes, original.va, original.rows);
assert.deepEqual(originalReferences.map(field => field.identity),
    ['msvcr80.dll!_CIcos', 'msvcr80.dll!_CIsin', 'msvcr80.dll!_CIsin', 'msvcr80.dll!_CIcos']);
const rebuiltReferences = references(rebuiltImage, readPE32Imports(rebuiltBytes), linkedBody, rebuiltVA, rows);
const pairs = originalReferences.flatMap(field => {
    const rebuilt = rebuiltReferences.find(item => item.offset === field.offset &&
        item.identity === field.identity && item.kind === field.kind);
    return rebuilt ? [{
        originalOffset: field.offset, rebuiltOffset: rebuilt.offset, kind: field.kind,
        identity: field.identity, originalTarget: field.target, rebuiltTarget: rebuilt.target,
    }] : [];
});
const completeTargets = pairs.length === originalReferences.length && pairs.length === rebuiltReferences.length;
const full = compareBodies(original.bytes, linkedBody, pairs, completeTargets);

const scopes = [];
for (const [start, end, name] of [
    [0x20c0109b, 0x20c010e1, 'Accumulate force and moment'],
    [0x20c0109b, 0x20c010b1, 'Accumulate the three force components'],
    [0x20c010b1, 0x20c010e1, 'Accumulate the three moment components'],
]) {
    const size = end - start;
    const originalScope = original.bytes.subarray(start - original.va, end - original.va);
    const inRange = (listing, va) => listing.filter(row => {
        const address = Number.parseInt(row[0], 16);
        return address >= va && address < va + size;
    });
    const asm = inRange(original.rows, start);
    verifyRows(asm, start, originalScope);
    const matches = rows.filter(([address]) => {
        const offset = Number.parseInt(address, 16) - rebuiltVA;
        return linkedBody.subarray(offset, offset + size).equals(originalScope);
    });
    assert(matches.length <= 1, 'Ambiguous exact scoped match');
    for (const [address] of matches) {
        const va = Number.parseInt(address, 16);
        const rebuilt = inRange(rows, va);
        verifyRows(rebuilt, va, originalScope);
        scopes.push({ entry: start.toString(16), name, bytes: size, rebuiltEntry: address, status: 'Exact',
            asm, rebuilt, leftHex: originalScope.toString('hex'), rightHex: originalScope.toString('hex') });
    }
}

const report = {
    program, artifactKind: 'linked PE32 DLL', originalSHA256, rebuiltSHA256: sha256(rebuiltBytes),
    entry, symbol: 'sim1::AccumulateEngineThrust', buildSymbol, rebuiltEntry: rebuiltVA.toString(16),
    ...full, originalBytes: original.bytes.length, rebuiltBytes: linkedBody.length,
    leftHex: original.bytes.toString('hex'), rightHex: linkedBody.toString('hex'),
    originalReferences, rebuiltReferences, pairs, asm: original.rows, rebuilt: rows, scopes,
};
fs.writeFileSync(path.join(run, 'thrust-comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log(`${entry} ${full.status}: ${original.bytes.length} original / ${linkedBody.length} linked bytes`);
for (const scope of scopes) console.log(`${scope.entry} ${scope.status}: ${scope.bytes} bytes, ${scope.name}`);
