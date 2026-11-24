const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');

function originalListing(program, entry, image) {
    const stem = path.parse(program).name;
    const root = path.join(__dirname, '../../artifacts', program, `${stem}-inventory`);
    const manifest = JSON.parse(fs.readFileSync(path.join(root, 'manifest.json'), 'utf8'));
    assert.equal(manifest.program, program);
    const item = manifest.functions.find(fn => fn.entry === entry);
    assert(item, `Missing Ghidra entry ${program}:${entry}`);
    const rows = fs.readFileSync(path.join(root, 'functions', `${entry}.asm`), 'utf8').trimEnd()
        .split(/\r?\n/).map(line => {
            const match = line.match(/^([0-9a-f]{8}) {2}((?:[0-9a-f]{2} )*[0-9a-f]{2}) {2}(.+)$/i);
            assert(match, line);
            return match.slice(1);
        });
    const va = Number.parseInt(entry, 16);
    const bytes = image.codeAt(va, item.bodyBytes);
    verifyRows(rows, va, bytes);
    return { rows, va, bytes };
}

function dumpbinRows(listing) {
    const joined = listing.replace(/^( +[0-9a-f]{8}: (?:[0-9a-f]{2} )*[0-9a-f]{2}) +(.*)\r?\n[ \t]+([0-9a-f]{2}(?: [0-9a-f]{2})*)[ \t]*$/gim,
        '$1 $3  $2');
    return Array.from(joined.matchAll(/^ +([0-9a-f]{8}): ((?:[0-9a-f]{2} )*[0-9a-f]{2}) +(\S[^\r\n]*)/gim),
        match => [match[1].toLowerCase(), match[2].toLowerCase(), match[3].trim().replace(/\s+/g, ' ')]);
}

function verifyRows(rows, va, bytes) {
    let cursor = va;
    for (const [address, hex] of rows) {
        assert.equal(Number.parseInt(address, 16), cursor, `Noncontiguous listing at ${address}`);
        const encoding = Buffer.from(hex.replaceAll(' ', ''), 'hex');
        assert(encoding.length > 0 && encoding.length <= 15, `Invalid x86 instruction at ${address}`);
        assert(bytes.subarray(cursor - va, cursor - va + encoding.length).equals(encoding),
            `Listing bytes differ at ${address}`);
        cursor += encoding.length;
    }
    assert.equal(cursor - va, bytes.length, 'Listing does not cover the complete body');
}

function readLinkedBodies(image, map, disassembly) {
    assert.equal(Number.parseInt(map.match(/Preferred load address is ([0-9a-f]+)/i)[1], 16), image.base);
    const symbols = Array.from(map.matchAll(/^\s+[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8}) +f\s/gim),
        ([, name, address]) => ({ name, va: Number.parseInt(address, 16) }));
    const listing = dumpbinRows(disassembly);
    return prefix => {
        const candidates = symbols.filter(symbol => symbol.name.startsWith(prefix));
        assert.equal(candidates.length, 1, `Missing unique linked symbol ${prefix}`);
        const { va, name } = candidates[0];
        const next = Math.min(...symbols.filter(symbol => symbol.va > va).map(symbol => symbol.va));
        const first = listing.findIndex(row => Number.parseInt(row[0], 16) === va);
        assert(first >= 0, `Missing linked listing for ${name}`);
        const rows = [];
        for (const row of listing.slice(first)) {
            if (Number.parseInt(row[0], 16) >= next || /^cc( cc)*$/.test(row[1]) ||
                (/^00( 00)*$/.test(row[1]) && /^\.+$/.test(row[2]))) break;
            rows.push(row);
        }
        const bytes = image.codeAt(va, rows.reduce((size, row) => size + row[1].split(' ').length, 0));
        verifyRows(rows, va, bytes);
        return { name, va, rows, bytes };
    };
}

function compareBodies(original, rebuilt, pairs, completeTargets) {
    const normalized = Buffer.from(rebuilt);
    const mappedOffsets = [];
    for (const pair of pairs) {
        if (pair.originalOffset !== pair.rebuiltOffset) continue;
        const offset = pair.originalOffset;
        original.copy(normalized, offset, offset, offset + 4);
        mappedOffsets.push(offset);
    }
    const sameSize = original.length === rebuilt.length;
    const differences = sameSize ? Array.from(original.entries())
        .filter(([offset, byte]) => normalized[offset] !== byte)
        .map(([offset, byte]) => ({ offset, original: byte, rebuilt: normalized[offset] })) : null;
    const status = completeTargets && normalized.equals(original)
        ? (rebuilt.equals(original) ? 'Exact' : 'Normalized') : 'Different';
    return { status, mappedOffsets, remainingByteDifferences: differences };
}

function nativeCoffBody(file, coff, listing, prefix) {
    const names = [...coff.byName.keys()].filter(name => name.startsWith(prefix));
    assert.equal(names.length, 1, prefix);
    const symbol = coff.unique(names[0]);
    const section = coff.sections[symbol.section - 1];
    assert.equal(section.name, '.text');
    assert.equal(symbol.value, 0);
    const bytes = file.subarray(section.raw, section.raw + section.size);
    const start = listing.indexOf('\n' + symbol.name + ' (');
    assert(start >= 0, symbol.name);
    const rows = dumpbinRows(listing.slice(start).split(/\r?\n\s*\r?\n/, 1)[0]);
    verifyRows(rows, 0, bytes);
    return { symbol, section, bytes, rows };
}

function nativeCoffReferences(file, coff, body, callee, identity, knownLiterals) {
    return body.section.relocations.map(({ offset, symbol, type }) => {
        assert.equal(body.bytes.readUInt32LE(offset), 0);
        if (type === 20) {
            assert.equal(body.bytes[offset - 1], 0xe8);
            assert.equal(symbol.index, callee.symbol.index);
            return { offset, identity, symbol: symbol.name };
        }
        assert.equal(type, 6);
        assert.match(symbol.name, /^__real@[0-9a-f]{16}$/);
        const data = coff.sections[symbol.section - 1];
        assert(data && symbol.value + 8 <= data.size);
        const literal = file.subarray(data.raw + symbol.value, data.raw + symbol.value + 8).toString('hex');
        assert.equal(literal, Buffer.from(symbol.name.slice(7), 'hex').reverse().toString('hex'));
        assert(knownLiterals.includes(literal), symbol.name);
        return { offset, identity: literal, symbol: symbol.name };
    });
}

function select(rows, begin, end) {
    return rows.filter(([address]) => {
        const va = Number.parseInt(address, 16);
        return va >= begin && va < end;
    });
}

function compareRange(original, rebuilt, begin, end, originalRefs, rebuiltRefs, offset) {
    const size = end - begin;
    const originalOffset = begin - original.va;
    const left = original.bytes.subarray(originalOffset, originalOffset + size);
    const right = rebuilt.bytes.subarray(offset, offset + size);
    const refs = (items, start) => items.filter(item => item.offset >= start && item.offset < start + size);
    const leftRefs = refs(originalRefs, originalOffset);
    const rightRefs = refs(rebuiltRefs, offset);
    const pairs = leftRefs.flatMap(item => {
        const originalOffsetInRange = item.offset - originalOffset;
        return rightRefs.some(ref => ref.offset - offset === originalOffsetInRange && ref.identity === item.identity)
            ? [{ originalOffset: originalOffsetInRange, rebuiltOffset: originalOffsetInRange,
                identity: item.identity, target: item.address }] : [];
    });
    const result = compareBodies(left, right, pairs, pairs.length === leftRefs.length && pairs.length === rightRefs.length);
    return { left, right, pairs, result };
}

function compareNativeRanges(original, rebuilt, begin, end, originalRefs, rebuiltRefs, ranges) {
    const originalSize = end - begin;
    const left = original.bytes.subarray(begin - original.va, end - original.va);
    const asm = select(original.rows, begin, end);
    verifyRows(asm, begin, left);
    const scopes = [];
    for (const [start, stop, name] of ranges) {
        const originalRows = select(asm, start, stop);
        for (const [address] of rebuilt.rows) {
            const offset = Number.parseInt(address, 16);
            const candidate = compareRange(original, rebuilt, start, stop, originalRefs, rebuiltRefs, offset);
            if (candidate.result.status === 'Different') continue;
            const rebuiltRows = select(rebuilt.rows, offset, offset + stop - start);
            if (rebuiltRows.reduce((size, row) => size + row[1].split(' ').length, 0) !== stop - start) continue;
            verifyRows(originalRows, start, candidate.left);
            verifyRows(rebuiltRows, offset, candidate.right);
            scopes.push({ entry: start.toString(16), rebuiltEntry: address, name, bytes: stop - start, ...candidate.result,
                pairs: candidate.pairs, asm: originalRows, rebuilt: rebuiltRows,
                leftHex: candidate.left.toString('hex'), rightHex: candidate.right.toString('hex') });
            break;
        }
    }
    const full = originalSize === rebuilt.bytes.length
        ? compareRange(original, rebuilt, begin, end, originalRefs, rebuiltRefs, 0).result.status : 'Different';
    return { entry: begin.toString(16), status: full, originalBytes: originalSize, rebuiltBytes: rebuilt.bytes.length,
        buildSymbol: rebuilt.symbol.name, asm, rebuilt: rebuilt.rows,
        leftHex: left.toString('hex'), rightHex: rebuilt.bytes.toString('hex'), scopes };
}

module.exports = {
    compareBodies, dumpbinRows, originalListing, readLinkedBodies, verifyRows,
    nativeCoffBody, nativeCoffReferences, compareNativeRanges,
};
