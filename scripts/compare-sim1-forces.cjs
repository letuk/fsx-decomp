const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');
const { compareBodies, dumpbinRows, originalListing, verifyRows } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 linked force run directory');
const program = 'sim1.dll';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const hex = value => value.toString(16).padStart(8, '0');
const source = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(source);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const original = openImage(source);
const originalImports = readPE32Imports(source);
const rebuilt = fs.readFileSync(path.join(run, 'sim1-forces.dll'));
const image = openImage(rebuilt);
const imports = readPE32Imports(rebuilt);
assert.equal(image.entryRva, 0, 'Expected a no-entry linked comparison DLL');

const map = fs.readFileSync(path.join(run, 'sim1-forces.map'), 'utf8');
assert.equal(Number.parseInt(map.match(/Preferred load address is ([0-9a-f]+)/i)[1], 16), image.base);
const symbols = Array.from(map.matchAll(/^\s+[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8}) +(.*)$/gim),
    ([, name, va, attributes]) => ({ name, va: Number.parseInt(va, 16), isFunction: /^f\s/.test(attributes) }));
const listing = dumpbinRows(fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1'));

const gravity = 0x20c7c180;
const fillBytes = 0x20bb11f0;
assert.deepEqual(originalImports.get(fillBytes), { dll: 'api.dll', name: '#109' });
const gravitySymbols = symbols.filter(item => item.name === '?gravityAcceleration@sim1@@3NA');
assert.equal(gravitySymbols.length, 1, 'Missing unique gravity backing-storage symbol');
const gravityRebuilt = gravitySymbols[0].va;
const importedFill = [...imports].filter(([, item]) => item.dll === 'api.dll' && item.name === '#109');
assert.equal(importedFill.length, 1, 'Expected the original API ordinal in the rebuilt IAT');
const fillRebuilt = importedFill[0][0];
const originalLiterals = new Set([0x20bc4d48, 0x20bc4d50, 0x20bc4d58, 0x20bc4d60]);

const tables = [
    { slotVA: 0x20bbefd4, target: 0x20bf6a20 },
    { slotVA: 0x20bbec6c, target: 0x20c669a6 },
    { slotVA: 0x20bbedb4, target: 0x20c2edc0 },
    { slotVA: 0x20bbef1c, target: 0x20c2ee60 },
];
for (const { slotVA, target } of tables) {
    assert.deepEqual(original.highLowFixups(slotVA, 4), [0]);
    assert.equal(original.initialDword(slotVA), target);
    assert.equal(original.codeAt(target, 1).length, 1);
}
const pureCall = original.codeAt(0x20c669a6, 6);
assert.equal(pureCall.readUInt16LE(), 0x25ff);
assert.deepEqual(originalImports.get(pureCall.readUInt32LE(2)), { dll: 'msvcr80.dll', name: '_purecall' });

function linkedBody(prefix) {
    const candidates = symbols.filter(item => item.isFunction && item.name.startsWith(prefix));
    assert.equal(candidates.length, 1, prefix);
    const { va, name } = candidates[0];
    const end = Math.min(...symbols.filter(item => item.isFunction && item.va > va).map(item => item.va));
    const first = listing.findIndex(row => Number.parseInt(row[0], 16) === va);
    assert(first >= 0, name);
    const rows = [];
    for (const row of listing.slice(first)) {
        if (Number.parseInt(row[0], 16) >= end || /^cc( cc)*$/.test(row[1]) ||
            (/^00( 00)*$/.test(row[1]) && /^\.+$/.test(row[2]))) break;
        rows.push(row);
    }
    const size = rows.reduce((total, row) => total + row[1].split(' ').length, 0);
    const bytes = image.codeAt(va, size);
    verifyRows(rows, va, bytes);
    return { va, name, rows, bytes };
}

function references(body, originalSide) {
    const pe = originalSide ? original : image;
    const { va, bytes, rows } = body;
    const items = pe.highLowFixups(va, bytes.length).map(offset => {
        const target = bytes.readUInt32LE(offset);
        let identity = null;
        if (target === (originalSide ? fillBytes : fillRebuilt)) {
            identity = 'api.dll#109';
        } else if (target === (originalSide ? gravity : gravityRebuilt)) {
            identity = 'sim1.dll gravityAcceleration';
        } else if ((originalSide && originalLiterals.has(target)) ||
            (!originalSide && symbols.some(item => item.va === target && /^__real@[0-9a-f]{16}$/.test(item.name)))) {
            identity = pe.codeAt(target, 8, false).toString('hex');
        }
        if (originalSide) assert(identity, `Unrecognized original address ${hex(target)}`);
        return { offset, kind: 'DIR32', target: hex(target), identity };
    });
    for (const [address, encoding] of rows.filter(row => /^e8 /i.test(row[1]))) {
        const offset = Number.parseInt(address, 16) - va + 1;
        const target = va + offset + 4 + bytes.readInt32LE(offset);
        items.push({ offset, kind: 'REL32', target: hex(target), identity: null });
    }
    return items;
}

function pair(left, right) {
    const unmatched = [...right];
    const pairs = [];
    for (const field of left) {
        if (!field.identity) continue;
        const same = item => item.identity === field.identity && item.kind === field.kind;
        let index = unmatched.findIndex(item => same(item) && item.offset === field.offset);
        if (index < 0) index = unmatched.findIndex(same);
        if (index < 0) continue;
        const [other] = unmatched.splice(index, 1);
        pairs.push({ originalOffset: field.offset, rebuiltOffset: other.offset, kind: field.kind,
            identity: field.identity, originalTarget: field.target, rebuiltTarget: other.target });
    }
    const complete = pairs.length === left.length && unmatched.length === 0 &&
        pairs.every(item => item.originalOffset === item.rebuiltOffset);
    return { pairs, complete };
}

const scopes = [];
const rangeByEntry = new Map([
    ['20bccae0', [0x20bccb4c, 0x20bccb7e, 'Accumulate the six force and moment components']],
    ['20bccef0', [0x20bcd05a, 0x20bcd095, 'Publish center of gravity and mass']],
    ['20c42d90', [0x20c42da5, 0x20c42de7, 'Copy the previous linear acceleration']],
]);
const targets = [
    ['20bf6a20', 'BoundForceContributor', 'Evaluate'],
    ['20bccae0', 'ForceContributors', 'Accumulate'],
    ['20bccef0', 'MassProperties', 'Recalculate'],
    ['20c42d90', 'FlightDynamics', 'PrepareAccelerations'],
];
const functions = targets.map(([entry, owner, method]) => {
    const left = originalListing(program, entry, original);
    const right = linkedBody(`?${method}@${owner}@sim1@@`);
    const leftRefs = references(left, true);
    const rightRefs = references(right, false);
    const { pairs, complete } = pair(leftRefs, rightRefs);
    const range = rangeByEntry.get(entry);
    if (range) {
        const [start, end, name] = range;
        const size = end - start;
        const leftOffset = start - left.va;
        const originalBytes = left.bytes.subarray(leftOffset, leftOffset + size);
        const selectRefs = (refs, offset) => refs.filter(ref => ref.offset >= offset && ref.offset < offset + size)
            .map(ref => ({ ...ref, offset: ref.offset - offset }));
        const selectRows = (rows, address) => rows.filter(row => {
            const va = Number.parseInt(row[0], 16);
            return va >= address && va < address + size;
        });
        const asm = selectRows(left.rows, start);
        verifyRows(asm, start, originalBytes);
        for (const row of right.rows) {
            const va = Number.parseInt(row[0], 16);
            const offset = va - right.va;
            if (offset + size > right.bytes.length) break;
            const rebuiltBytes = right.bytes.subarray(offset, offset + size);
            const scoped = pair(selectRefs(leftRefs, leftOffset), selectRefs(rightRefs, offset));
            const result = compareBodies(originalBytes, rebuiltBytes, scoped.pairs, scoped.complete);
            if (result.status === 'Different') continue;
            const rows = selectRows(right.rows, va);
            verifyRows(rows, va, rebuiltBytes);
            assert(!scopes.some(item => item.entry === hex(start)), 'Ambiguous scope ' + name);
            scopes.push({ entry: hex(start), parentEntry: entry, name, full: false, ...result, pairs: scoped.pairs,
                originalBytes: size, rebuiltBytes: size, asm, rebuilt: rows,
                leftHex: originalBytes.toString('hex'), rightHex: rebuiltBytes.toString('hex') });
        }
    }
    return { entry, symbol: owner + '::' + method, buildSymbol: right.name, rebuiltEntry: hex(right.va),
        ...compareBodies(left.bytes, right.bytes, pairs, complete), originalBytes: left.bytes.length,
        rebuiltBytes: right.bytes.length, leftHex: left.bytes.toString('hex'), rightHex: right.bytes.toString('hex'),
        rebuilt: right.rows, originalReferences: leftRefs, rebuiltReferences: rightRefs, pairs };
});

const report = { program, artifactKind: 'linked PE32 DLL', originalSHA256, rebuiltSHA256: sha(rebuilt),
    originalVtableSlots: tables, functions, scopes };
fs.writeFileSync(path.join(run, 'forces-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of functions) console.log(`${item.entry} ${item.symbol} ${item.status} ${item.originalBytes}/${item.rebuiltBytes}`);
for (const item of scopes) console.log(`${item.entry} ${item.status} ${item.originalBytes} ${item.name}`);
