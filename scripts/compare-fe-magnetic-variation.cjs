const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { compareBodies, dumpbinRows, originalListing, verifyRows } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass an FE magnetic variation run directory');
const sha = data => createHash('sha256').update(data).digest('hex');
const program = 'fe.dll';
const originalBytes = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(originalBytes);
assert.equal(originalSHA256, 'ba04a13f5f6c377768b732e364a8ac4c3b61afa54dae824df9614fa91532682c');
const original = openImage(originalBytes);
const originalOptionalHeader = originalBytes.readUInt32LE(0x3c) + 24;
const exportDirectory = original.codeAt(original.base + originalBytes.readUInt32LE(originalOptionalHeader + 96), 40, false);
const exportBase = exportDirectory.readUInt32LE(16);
const addressTable = original.base + exportDirectory.readUInt32LE(28);
const exportOrdinals = [[3, 0x203efc30], [4, 0x203efc50]];
for (const [ordinal, va] of exportOrdinals) {
    const index = ordinal - exportBase;
    assert(index >= 0 && index < exportDirectory.readUInt32LE(20));
    assert.equal(original.base + original.codeAt(addressTable + 4 * index, 4, false).readUInt32LE(), va);
}
const rebuiltBytes = fs.readFileSync(path.join(run, 'fe-magnetic-variation.dll'));
const rebuilt = openImage(rebuiltBytes);
assert.equal(rebuilt.entryRva, 0, 'Expected the scoped no-entry DLL');
const listing = dumpbinRows(fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1'));
const map = fs.readFileSync(path.join(run, 'fe-magnetic-variation.map'), 'utf8');
assert.equal(Number.parseInt(map.match(/Preferred load address is ([0-9a-f]+)/i)[1], 16), rebuilt.base);
const symbols = Array.from(map.matchAll(/^\s+[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8}) +(.*)$/gim),
    ([, name, address, attributes]) => ({ name, va: Number.parseInt(address, 16), isFunction: /^f\s/.test(attributes) }));

function findSymbol(pattern) {
    const matches = symbols.filter(symbol => pattern.test(symbol.name));
    assert.equal(matches.length, 1, String(pattern));
    return matches[0];
}

const functions = [
    ['203efc30', 'GetFixedMagneticVariation', /^_GetFixedMagneticVariation@4$/],
    ['203efc50', 'GetMagneticVariation', /^_GetMagneticVariation@4$/],
    ['203ec5d0', 'SampleFixedMagneticVariation', /^\?SampleFixedMagneticVariation@@/],
    ['203ec9a0', 'SampleMagneticVariation', /^\?SampleMagneticVariation@@/],
    ['203ecb90', 'InterpolateMagneticAngle', /^\?InterpolateMagneticAngle@@/],
].map(([entry, name, pattern]) => {
    const symbol = findSymbol(pattern);
    return { entry, name, mapSymbol: symbol.name, va: symbol.va };
});
const globalTargets = [
    [0x20449a1c, 'magneticLatitudeCount'],
    [0x20449a20, 'magneticLongitudeCount'],
    [0x20449a28, 'magneticVariationGrid'],
].map(([oldVA, name]) => ({ oldVA, name, newVA: findSymbol(new RegExp(`^\\?${name}@@`)).va }));
const originalLiterals = new Set([0x20341740, 0x20341748, 0x20341750, 0x203ddcf0,
    0x203ddd00, 0x203ddd08, 0x203ddd20, 0x203ddd30]);

function absoluteReferences(image, va, bytes, originalSide) {
    return image.highLowFixups(va, bytes.length).map(offset => {
        const target = bytes.readUInt32LE(offset);
        const global = globalTargets.find(item => target === (originalSide ? item.oldVA : item.newVA));
        if (global) return { offset, target, identity: global.name, kind: 'HIGHLOW data' };
        const literal = originalSide ? originalLiterals.has(target)
            : symbols.some(symbol => symbol.va === target && /^__real@[0-9a-f]{16}$/.test(symbol.name));
        assert(literal, `Unexpected data target ${target.toString(16)}`);
        return { offset, target, identity: image.codeAt(target, 8, false).toString('hex'), kind: 'HIGHLOW literal' };
    });
}

function directCalls(va, bytes, rows, originalSide) {
    return rows.filter(row => /^e8 /i.test(row[1])).map(row => {
        const offset = Number.parseInt(row[0], 16) - va + 1;
        const target = va + offset + 4 + bytes.readInt32LE(offset);
        const callee = functions.find(item => target === (originalSide ? Number.parseInt(item.entry, 16) : item.va));
        assert(callee, `Unexpected call target ${target.toString(16)}`);
        return { offset, target, identity: callee.name, kind: 'REL32 call' };
    });
}

const results = functions.map(item => {
    const old = originalListing(program, item.entry, original);
    const end = Math.min(...symbols.filter(symbol => symbol.isFunction && symbol.va > item.va).map(symbol => symbol.va));
    const start = listing.findIndex(row => Number.parseInt(row[0], 16) === item.va);
    assert(start >= 0);
    const rows = [];
    for (const row of listing.slice(start)) {
        if (Number.parseInt(row[0], 16) >= end || /^(?:cc)( cc)*$/.test(row[1]) ||
            (/^00( 00)*$/.test(row[1]) && /^\.+$/.test(row[2]))) break;
        rows.push(row);
    }
    assert.match(rows.at(-1)[2], /^ret(?:\s|$)/i);
    const newLength = rows.reduce((length, row) => length + row[1].split(' ').length, 0);
    const newBytes = rebuilt.codeAt(item.va, newLength);
    verifyRows(rows, item.va, newBytes);

    const originalReferences = [...absoluteReferences(original, old.va, old.bytes, true), ...directCalls(old.va, old.bytes, old.rows, true)];
    const rebuiltReferences = [...absoluteReferences(rebuilt, item.va, newBytes, false), ...directCalls(item.va, newBytes, rows, false)];
    const unmatched = [...rebuiltReferences];
    const pairs = [];
    for (const reference of originalReferences) {
        const index = unmatched.findIndex(candidate => candidate.identity === reference.identity && candidate.kind === reference.kind);
        if (index < 0) continue;
        const [match] = unmatched.splice(index, 1);
        pairs.push({ originalOffset: reference.offset, rebuiltOffset: match.offset, identity: match.identity,
            kind: match.kind, originalVA: reference.target.toString(16), rebuiltVA: match.target.toString(16) });
    }
    const sameTargetFields = pairs.length === originalReferences.length && unmatched.length === 0 &&
        pairs.every(pair => pair.originalOffset === pair.rebuiltOffset);
    return { entry: item.entry, symbol: item.name, mapSymbol: item.mapSymbol, rebuiltEntry: item.va.toString(16),
        ...compareBodies(old.bytes, newBytes, pairs, sameTargetFields),
        originalBytes: old.bytes.length, rebuiltBytes: newLength, originalReferences, rebuiltReferences, pairs,
        leftHex: old.bytes.toString('hex'), rightHex: newBytes.toString('hex'), rebuilt: rows };
});

const report = { program, artifactKind: 'linked PE32 DLL', originalSHA256, rebuiltSHA256: sha(rebuiltBytes),
    exportOrdinals: Object.fromEntries(exportOrdinals.map(([ordinal, va]) => [ordinal, va.toString(16)])), functions: results };
fs.writeFileSync(path.join(run, 'magnetic-variation-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of results) console.log(`${item.entry} ${item.symbol} ${item.status} ${item.originalBytes}/${item.rebuiltBytes}`);
assert(['Exact', 'Normalized'].includes(results[0].status),
    'The 17-byte fixed-coordinate adapter must match after validating its call');
