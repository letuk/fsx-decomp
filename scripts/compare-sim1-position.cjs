const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { openImage } = require('./lib/pe32-dll.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');
const { compareBodies, dumpbinRows, originalListing, verifyRows } = require('./lib/reconstruction-listings.cjs');

const run = process.argv[2];
assert(run, 'Pass a Sim1 position run directory');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const hex = address => address.toString(16);
const program = 'sim1.dll';
const originalBytes = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(originalBytes);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const original = openImage(originalBytes);
const originalImports = readPE32Imports(originalBytes);
const request = JSON.parse(fs.readFileSync(path.join(run, 'request.json'), 'utf8').replace(/^\uFEFF/, ''));
const linked = request.ArtifactKind === 'PE32 DLL';
const rebuiltFilename = linked ? 'sim1-position.dll' : 'sim1-position.obj';
const rebuiltBytes = fs.readFileSync(path.join(run, rebuiltFilename));
const coff = linked ? null : parseNativeCoff(rebuiltBytes);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const rebuiltImage = linked ? openImage(rebuiltBytes) : null;
const rebuiltImports = linked ? readPE32Imports(rebuiltBytes) : null;
const symbols = linked ? readMap() : [];

const mathThunks = new Map([[0x20c66fc2, '_CIsqrt'], [0x20c670a0, '_CIcos'], [0x20c670ac, '_CIsin']]);
const literalVAs = new Set([0x20bc4cc8, 0x20bc38b0, 0x20bc3a80, 0x20bc35c8, 0x20bc3690, 0x20bc3688, 0x20bc4948]);
const publishedStateVA = 0x20bb11fc;
const publishedState = 'api.dll#304';

function thunkIdentity(image, imports, target) {
    const stub = image.codeAt(target, 6);
    assert.equal(stub.readUInt16LE(), 0x25ff, 'Expected an x86 import jump');
    const imported = imports.get(stub.readUInt32LE(2));
    assert(imported, 'Missing import identity');
    return imported.dll + '!' + imported.name;
}

function originalReferences(scope) {
    const { va, bytes, rows } = scope;
    const references = original.highLowFixups(va, bytes.length).map(offset => {
        const target = bytes.readUInt32LE(offset);
        if (target === publishedStateVA) {
            assert.deepEqual(originalImports.get(target), { dll: 'api.dll', name: '#304' });
            return { offset, target: hex(target), kind: 'DIR32', identity: publishedState };
        }
        assert(literalVAs.has(target), `Unknown original literal ${hex(target)}`);
        return { offset, target: hex(target), kind: 'DIR32', identity: original.codeAt(target, 8, false).toString('hex') };
    });
    for (const row of rows.filter(item => /^e8 /i.test(item[1]))) {
        const offset = Number.parseInt(row[0], 16) - va + 1;
        const target = va + offset + 4 + bytes.readInt32LE(offset);
        const name = mathThunks.get(target);
        assert(name, `Unknown original call ${hex(target)}`);
        const identity = thunkIdentity(original, originalImports, target);
        assert.equal(identity, 'msvcr80.dll!' + name);
        references.push({ offset, target: hex(target), kind: 'REL32', identity });
    }
    return references;
}

function coffReference(relocation, bytes) {
    const { offset, symbol, type } = relocation;
    assert.equal(bytes.readUInt32LE(offset), 0, 'Expected a zero COFF addend');
    const kind = type === 6 ? 'DIR32' : type === 20 ? 'REL32' : null;
    assert(kind, `Unexpected COFF relocation ${type}`);
    const name = symbol.name;
    let identity = null;
    if (name === '__imp__SimPublishedPosition') {
        assert.equal(symbol.section, 0);
        identity = publishedState;
    } else if (/^__CI(?:sqrt|cos|sin)$/.test(name)) {
        assert.equal(symbol.section, 0);
        identity = 'msvcr80.dll!' + name.slice(1);
    } else if (/^__real@[0-9a-f]{16}$/.test(name)) {
        const section = coff.sections[symbol.section - 1];
        assert(section && symbol.value + 8 <= section.size);
        const literal = Buffer.from(name.slice(7), 'hex').reverse();
        assert(coff.buffer.subarray(section.raw + symbol.value, section.raw + symbol.value + 8).equals(literal));
        identity = literal.toString('hex');
    }
    return { offset, kind, symbol: name, identity };
}

function readMap() {
    assert.equal(rebuiltImage.entryRva, 0, 'Expected a no-entry comparison DLL');
    const map = fs.readFileSync(path.join(run, 'sim1-position.map'), 'utf8');
    assert.equal(Number.parseInt(map.match(/Preferred load address is ([0-9a-f]+)/i)[1], 16), rebuiltImage.base);
    return Array.from(map.matchAll(/^\s+[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8}) +(.*)$/gim),
        ([, name, address, attributes]) => ({ name, va: Number.parseInt(address, 16), isFunction: /^f\s/.test(attributes) }));
}

function readObjectBody(prefix) {
    const names = [...coff.byName.keys()].filter(name => name.startsWith(prefix));
    assert.equal(names.length, 1, prefix);
    const symbol = coff.unique(names[0]);
    const section = coff.sections[symbol.section - 1];
    assert.equal(section.name, '.text');
    assert.equal(symbol.value, 0);
    const bytes = coff.buffer.subarray(section.raw, section.raw + section.size);
    const listingStart = listing.indexOf(`\n${symbol.name} (`);
    assert(listingStart >= 0, `Missing worker listing for ${symbol.name}`);
    const rows = dumpbinRows(listing.slice(listingStart).split(/\r?\n\s*\r?\n/, 1)[0]);
    verifyRows(rows, 0, bytes);
    return { va: 0, name: symbol.name, rows, bytes, references: section.relocations.map(relocation => coffReference(relocation, bytes)) };
}

function readLinkedBody(prefix) {
    const matches = symbols.filter(symbol => symbol.isFunction && symbol.name.startsWith(prefix));
    assert.equal(matches.length, 1, prefix);
    const { va, name } = matches[0];
    const end = Math.min(...symbols.filter(symbol => symbol.isFunction && symbol.va > va).map(symbol => symbol.va));
    const decoded = dumpbinRows(listing);
    const start = decoded.findIndex(row => Number.parseInt(row[0], 16) === va);
    assert(start >= 0, name);
    const rows = [];
    for (const row of decoded.slice(start)) {
        if (Number.parseInt(row[0], 16) >= end || /^(cc)( cc)*$/.test(row[1]) ||
            (/^00( 00)*$/.test(row[1]) && /^\.+$/.test(row[2]))) break;
        rows.push(row);
    }
    assert.match(rows.at(-1)[2], /^ret(?:\s|$)/i);
    const length = rows.reduce((total, row) => total + row[1].split(' ').length, 0);
    const bytes = rebuiltImage.codeAt(va, length);
    verifyRows(rows, va, bytes);
    const references = rebuiltImage.highLowFixups(va, length).map(offset => {
        const target = bytes.readUInt32LE(offset);
        const imported = rebuiltImports.get(target);
        const symbol = symbols.find(item => item.va === target && /^__real@[0-9a-f]{16}$/.test(item.name));
        let identity = null;
        if (imported) {
            identity = imported.dll + (imported.name.startsWith('#') ? '' : '!') + imported.name;
        } else if (symbol) {
            identity = rebuiltImage.codeAt(target, 8, false).toString('hex');
        }
        return { offset, target: hex(target), kind: 'DIR32', symbol: symbol?.name ?? identity, identity };
    });
    for (const row of rows.filter(item => /^e8 /i.test(item[1]))) {
        const offset = Number.parseInt(row[0], 16) - va + 1;
        const target = va + offset + 4 + bytes.readInt32LE(offset);
        const symbol = symbols.find(item => item.isFunction && item.va === target);
        const stub = rebuiltImage.codeAt(target, 6);
        const identity = stub.readUInt16LE() === 0x25ff
            ? thunkIdentity(rebuiltImage, rebuiltImports, target)
            : null;
        references.push({ offset, target: hex(target), kind: 'REL32', symbol: symbol?.name ?? identity, identity });
    }
    return { va, name, rows, bytes, references };
}

function pairReferences(left, right) {
    const unmatched = [...right];
    const pairs = [];
    for (const field of left) {
        const sameTarget = ref => ref.identity === field.identity && ref.kind === field.kind;
        let index = unmatched.findIndex(ref => sameTarget(ref) && ref.offset === field.offset);
        if (index < 0) index = unmatched.findIndex(sameTarget);
        if (index < 0) continue;
        const [rebuilt] = unmatched.splice(index, 1);
        pairs.push({ originalOffset: field.offset, rebuiltOffset: rebuilt.offset,
            originalTarget: field.target, rebuiltTarget: rebuilt.target, rebuiltSymbol: rebuilt.symbol,
            identity: field.identity, kind: field.kind });
    }
    return { pairs, complete: pairs.length === left.length && unmatched.length === 0 &&
        pairs.every(pair => pair.originalOffset === pair.rebuiltOffset) };
}

const positionScopes = new Map([
    ['20bd1d90', [0x20bd1dc5, 0x20bd1de2, 'Velocity publication and horizontal speed']],
    ['20bd1bd0', [0x20bd1c12, 0x20bd1c31, 'Longitude radians and trigonometric cache']],
    ['20bd1c90', [0x20bd1d0c, 0x20bd1d22, 'Query height and subtract it from altitude']],
]);

function sliceReferences(references, start, length) {
    return references.filter(item => item.offset >= start && item.offset < start + length)
        .map(item => ({ ...item, offset: item.offset - start }));
}

function compareScope(entry, old, rebuilt, left, right) {
    const [start, end, name] = positionScopes.get(entry);
    const offset = start - old.va;
    const length = end - start;
    const originalBody = old.bytes.subarray(offset, offset + length);
    const originalRows = old.rows.filter(row => {
        const va = Number.parseInt(row[0], 16);
        return va >= start && va < end;
    });
    verifyRows(originalRows, start, originalBody);
    const scopedLeft = sliceReferences(left, offset, length);
    const candidates = [];
    for (const [i, row] of rebuilt.rows.entries()) {
        const va = Number.parseInt(row[0], 16);
        const offset = va - rebuilt.va;
        if (offset + length > rebuilt.bytes.length) break;
        const rebuiltBody = rebuilt.bytes.subarray(offset, offset + length);
        const scopedRight = sliceReferences(right, offset, length);
        const { pairs, complete } = pairReferences(scopedLeft, scopedRight);
        const result = compareBodies(originalBody, rebuiltBody, pairs, complete);
        if (result.status === 'Different') continue;
        const rows = rebuilt.rows.slice(i).filter(item => Number.parseInt(item[0], 16) < va + length);
        verifyRows(rows, va, rebuiltBody);
        candidates.push({ entry: hex(start), parentEntry: entry, name, full: false, ...result, pairs,
            originalBytes: length, rebuiltBytes: length, asm: originalRows, rebuilt: rows,
            leftHex: originalBody.toString('hex'), rightHex: rebuiltBody.toString('hex') });
    }
    assert(candidates.length <= 1, `Ambiguous rebuilt range for ${name}`);
    return candidates[0];
}

const scopes = [];
const functions = [
    ['20bd1d90', 'StoreWorldVelocity'],
    ['20bd1bd0', 'SetLongitude'],
    ['20bd1c90', 'SetAltitude'],
].map(([entry, method]) => {
    const old = originalListing(program, entry, original);
    const prefix = `?${method}@FlightDynamics@sim1@@`;
    const rebuilt = linked ? readLinkedBody(prefix) : readObjectBody(prefix);
    const left = originalReferences(old);
    const right = rebuilt.references;
    const { pairs, complete } = pairReferences(left, right);
    const scope = compareScope(entry, old, rebuilt, left, right);
    if (scope) scopes.push(scope);
    return { entry, symbol: 'FlightDynamics::' + method, buildSymbol: rebuilt.name, rebuiltEntry: hex(rebuilt.va),
        ...compareBodies(old.bytes, rebuilt.bytes, pairs, complete),
        originalBytes: old.bytes.length, rebuiltBytes: rebuilt.bytes.length, leftHex: old.bytes.toString('hex'),
        rightHex: rebuilt.bytes.toString('hex'), rebuilt: rebuilt.rows, originalReferences: left, rebuiltReferences: right, pairs };
});

const report = { program, artifactKind: linked ? 'linked PE32 DLL' : 'native x86 COFF', originalSHA256,
    rebuiltSHA256: sha(rebuiltBytes), functions, scopes };
fs.writeFileSync(path.join(run, 'position-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const item of functions) console.log(`${item.entry} ${item.symbol} ${item.status} ${item.originalBytes}/${item.rebuiltBytes}`);
for (const item of scopes) console.log(`${item.entry} ${item.name} ${item.status} ${item.originalBytes} bytes`);
