const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const {createHash} = require('node:crypto');
const {readPE32Imports} = require('./lib/pe-imports.cjs');

const directory = process.argv[2];
const output = process.argv[3];
assert(directory && output, 'Pass the binary directory and evidence path');
const names = ['_getmbcp', 'lstrcpynA', '_ismbblead'];
const copyVA = 0x20137e17;
const copySize = 169;
const importOperands = [14, 33, 68];
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const vaHex = address => address.toString(16).padStart(8, '0');

function parse(file) {
    const bytes = fs.readFileSync(file);
    assert.equal(bytes.readUInt16LE(), 0x5a4d);
    const header = bytes.readUInt32LE(60);
    assert.equal(bytes.readUInt32LE(header), 0x4550);
    assert.equal(bytes.readUInt16LE(header + 4), 0x14c, 'Expected x86 PE');
    const base = bytes.readUInt32LE(header + 52);
    const table = header + 24 + bytes.readUInt16LE(header + 20);
    const sections = Array.from({length: bytes.readUInt16LE(header + 6)}, (_, i) => {
        const h = table + i * 40;
        const raw = bytes.readUInt32LE(h + 20);
        const size = bytes.readUInt32LE(h + 16);
        assert(raw + size <= bytes.length);
        return {start: base + bytes.readUInt32LE(h + 12), code: !!(bytes.readUInt32LE(h + 36) & 0x20000000),
            bytes: bytes.subarray(raw, raw + size)};
    });
    function read(address, size) {
        const s = sections.find(s => address >= s.start && address + size <= s.start + s.bytes.length);
        return s?.bytes.subarray(address - s.start, address - s.start + size);
    }
    return {hash: sha(bytes), sections, read, imports: readPE32Imports(bytes)};
}

const api = parse(path.join(directory, 'api.dll'));
assert.equal(api.hash, '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const body = api.read(copyVA, copySize);
assert(body);
assert.deepEqual(importOperands.map(offset => api.imports.get(body.readUInt32LE(offset)).name), names);
function boundariesMatch(bytes) {
    let start = 0;
    for (const end of [...importOperands, copySize]) {
        if (!bytes.subarray(start, end).equals(body.subarray(start, end))) return false;
        start = end + 4;
    }
    return true;
}

const matches = [];
const fingerprints = [];
const importsOf157 = [];
const images = [];
const skipped = [];
for (const name of fs.readdirSync(directory).filter(name => /\.(dll|exe)$/i.test(name)).sort()) {
    let pe;
    try {
        pe = parse(path.join(directory, name));
    } catch (error) {
        skipped.push({name, reason: error.message});
        continue;
    }
    const entries = [...pe.imports];
    const hasTrio = names.every(target => entries.some(([, entry]) => entry.name === target));
    images.push({name, sha256: pe.hash, hasTrio});
    const ordinalSlots = entries.filter(([, entry]) => entry.dll === 'api.dll' && entry.name === '#157');
    if (ordinalSlots.length) importsOf157.push({module: name, slots: ordinalSlots.map(([va]) => vaHex(va))});
    if (!hasTrio) continue;
    const refs = [];
    const prefix = body.subarray(0, importOperands[0]);
    for (const section of pe.sections.filter(s => s.code)) {
        const bytes = section.bytes;
        for (let at = bytes.indexOf(prefix); at >= 0; at = bytes.indexOf(prefix, at + 1)) {
            if (at + copySize > bytes.length) break;
            const candidate = bytes.subarray(at, at + copySize);
            if (boundariesMatch(candidate) && importOperands.every((offset, i) =>
                pe.imports.get(candidate.readUInt32LE(offset))?.name === names[i])) {
                matches.push({module: name, entry: vaHex(section.start + at)});
            }
        }
        for (let i = 0; i + 6 <= bytes.length; i++) {
            const address = section.start + i;
            let iat;
            if (bytes[i] === 0xff && (bytes[i + 1] === 0x15 || bytes[i + 1] === 0x25)) {
                iat = bytes.readUInt32LE(i + 2);
            } else if (bytes[i] === 0xe8) {
                const thunk = pe.read(address + 5 + bytes.readInt32LE(i + 1), 6);
                if (thunk?.readUInt16LE() === 0x25ff) iat = thunk.readUInt32LE(2);
            }
            const target = pe.imports.get(iat)?.name;
            if (names.includes(target)) refs.push({address, target});
        }
    }
    refs.sort((a, b) => a.address - b.address);
    for (let i = 0; i < refs.length; i++) {
        const first = refs[i];
        if (first.target !== names[0]) continue;
        const window = [];
        for (let j = i + 1; j < refs.length && refs[j].address - first.address < copySize; j++) {
            window.push(refs[j]);
        }
        const second = window.find(ref => ref.target === names[1]);
        const third = second && window.find(ref => ref.address > second.address && ref.target === names[2]);
        if (third) fingerprints.push({module: name, addresses: [first, second, third].map(ref => vaHex(ref.address))});
    }
}
assert(matches.some(hit => hit.module === 'api.dll' && hit.entry === vaHex(copyVA)));
fs.writeFileSync(output, JSON.stringify({
    scope: 'Executable sections of supplied x86 PE files. Exact body equality permits only three PE import operands to move. An ordered 169-byte reference window is a discovery signal, not a function match. Other instruction shapes and dynamic imports are outside the search.',
    reference: {module: 'api.dll', address: vaHex(copyVA), size: copySize, imports: names},
    images, skipped, matches, fingerprints, importsOf157
}, null, 2) + '\n');
console.log(JSON.stringify({images: images.length, skipped, matches, fingerprints, importsOf157}));
