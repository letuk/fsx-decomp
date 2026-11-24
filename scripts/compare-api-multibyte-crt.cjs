const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const {createHash} = require('node:crypto');
const {readPE32Imports} = require('./lib/pe-imports.cjs');

const run = process.argv[2];
const output = process.argv[3];
assert(run && output, 'Pass the retained CRT probe run and an evidence JSON path');
const sha = data => createHash('sha256').update(data).digest('hex');
function readImage(file) {
    const data = fs.readFileSync(file);
    const pe = data.readUInt32LE(60);
    assert.equal(data.readUInt32LE(pe), 0x4550);
    assert.equal(data.readUInt16LE(pe + 4), 0x14c);
    assert(data.readUInt16LE(pe + 22) & 0x2000);
    const table = pe + 24 + data.readUInt16LE(pe + 20);
    function read(va, size) {
        const rva = va - data.readUInt32LE(pe + 52);
        for (let i = 0; i < data.readUInt16LE(pe + 6); i++) {
            const section = table + 40 * i;
            const delta = rva - data.readUInt32LE(section + 12);
            if (delta >= 0 && delta + size <= data.readUInt32LE(section + 16)) {
                const at = data.readUInt32LE(section + 20) + delta;
                assert(at + size <= data.length);
                return data.subarray(at, at + size);
            }
        }
        throw Error('Unmapped address ' + va.toString(16));
    }
    return {read, data, entryRva: data.readUInt32LE(pe + 40)};
}

const original = readImage(path.join(__dirname, '../original/api.dll'));
const linked = readImage(path.join(run, 'api-pilot.dll'));
assert.equal(sha(original.data), '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
assert.equal(linked.entryRva, 0);
const map = fs.readFileSync(path.join(run, 'worker-output/fsx.map'), 'utf8');
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
const originalAddress = 0x20137e17;
const originalBody = original.read(originalAddress, 169);
const imports = readPE32Imports(original.data);
const copyImports = [12, 31, 66].map(offset => {
    assert.equal(originalBody.readUInt16LE(offset), 0x15ff);
    const address = originalBody.readUInt32LE(offset + 2);
    return {address: address.toString(16), ...imports.get(address)};
});
assert.deepEqual(copyImports.map(item => item.name), ['_getmbcp', 'lstrcpynA', '_ismbblead']);
assert.equal(originalBody.subarray(-3).toString('hex'), 'c20c00');

function libraryBody(symbol, member, wrapper = false) {
    const mapped = map.split(/\r?\n/).find(line => line.trim().split(/\s+/)[1] === symbol);
    assert(mapped && mapped.trim().endsWith('libcmt:' + member));
    const address = parseInt(mapped.trim().split(/\s+/)[2], 16);
    const lines = listing.split(/\r?\n/);
    const start = lines.indexOf(symbol + ':');
    assert(start >= 0);
    let size = 0;
    const instructions = [];
    for (let i = start + 1; i < lines.length && !/^\S.*:$/.test(lines[i]); i++) {
        const line = lines[i];
        const match = line.match(/^\s*([0-9A-F]{8}):\s+((?:[0-9A-F]{2} )*[0-9A-F]{2})(?:\s{2,}(.*))?$/);
        assert(match || line.trim() === '', 'Unexpected listing row: ' + line);
        if (!match) continue;
        const [va, hex, instruction] = match.slice(1);
        const continuation = lines[i + 1]?.match(/^\s+((?:[0-9A-F]{2} )*[0-9A-F]{2})\s*$/);
        const raw = Buffer.from((hex + (continuation ? continuation[1] : '')).replaceAll(' ', ''), 'hex');
        if (continuation) i++;
        assert.equal(parseInt(va, 16), address + size);
        assert.deepEqual(linked.read(address + size, raw.length), raw);
        size += raw.length;
        instructions.push([va.toLowerCase(), raw.toString('hex'), instruction?.trim() || '']);
        if (wrapper && raw.equals(Buffer.from([0xc3]))) break;
    }
    const bytes = linked.read(address, size);
    return {symbol, member, address: address.toString(16), bytes: size,
        rawEqualsOriginal: bytes.equals(originalBody), instructions};
}
const candidates = ['__mbsnbcpy', '__mbsncpy'].map(symbol => {
    const member = symbol.slice(2) + '.obj';
    const entry = libraryBody(symbol, member, true);
    const worker = libraryBody(symbol + '_l', member);
    const text = worker.instructions.map(row => row[2]);
    assert(entry.instructions.some(row => row[2].endsWith(' ' + worker.symbol)));
    const workerCalls = text.filter(instruction => /^call\s+/.test(instruction));
    return {entry, worker, workerCalls};
});
const evidence = {module: 'api.dll', originalSHA256: sha(original.data), linkedProbeSHA256: sha(linked.data),
    run: path.basename(run), original: {address: originalAddress.toString(16), bytes: originalBody.length,
        hex: originalBody.toString('hex'), imports: copyImports}, candidates,
    scope: 'Linked VC8 libcmt entry points and locale workers compared with the original helper. Raw equality and call lists are reported separately.'};
fs.writeFileSync(output, JSON.stringify(evidence, null, 2) + '\n');
console.log(candidates.map(({entry, worker}) => entry.symbol + ': ' + entry.bytes + ' / ' + worker.bytes + ' bytes').join('\n'));
