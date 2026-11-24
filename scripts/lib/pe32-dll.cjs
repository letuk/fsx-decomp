const assert = require('node:assert/strict');

function openImage(bytes) {
    const pe = bytes.readUInt32LE(0x3c);
    const optional = pe + 24;
    assert.equal(bytes.readUInt32LE(pe), 0x4550);
    assert.equal(bytes.readUInt16LE(pe + 4), 0x14c);
    assert(bytes.readUInt16LE(pe + 22) & 0x2000, 'Expected a DLL');
    assert.equal(bytes.readUInt16LE(optional), 0x10b);
    const base = bytes.readUInt32LE(optional + 28);
    const table = optional + bytes.readUInt16LE(pe + 20);
    function codeAt(va, size, executable = true) {
        for (let i = 0; i < bytes.readUInt16LE(pe + 6); i++) {
            const section = table + i * 40;
            const offset = va - base - bytes.readUInt32LE(section + 12);
            if (offset < 0 || offset + size > bytes.readUInt32LE(section + 16)) continue;
            if (executable) assert(bytes.readUInt32LE(section + 36) & 0x20000000, 'Expected executable code');
            const start = bytes.readUInt32LE(section + 20) + offset;
            return bytes.subarray(start, start + size);
        }
        throw Error('Code address unmapped: ' + va.toString(16));
    }
    function highLowFixups(start, length) {
        const directory = optional + 96 + 5 * 8;
        const data = codeAt(base + bytes.readUInt32LE(directory), bytes.readUInt32LE(directory + 4), false);
        const offsets = [];
        for (let block = 0; block < data.length;) {
            const page = data.readUInt32LE(block);
            const size = data.readUInt32LE(block + 4);
            assert(size >= 8 && size % 2 === 0 && block + size <= data.length);
            for (let i = block + 8; i < block + size; i += 2) {
                const item = data.readUInt16LE(i);
                const va = base + page + (item & 0xfff);
                if (item >>> 12 && va >= start && va < start + length) {
                    assert.equal(item >>> 12, 3);
                    offsets.push(va - start);
                }
            }
            block += size;
        }
        return offsets;
    }
    function initialDword(va) {
        for (let i = 0; i < bytes.readUInt16LE(pe + 6); ++i) {
            const section = table + i * 40;
            const offset = va - base - bytes.readUInt32LE(section + 12);
            const raw = bytes.readUInt32LE(section + 16);
            if (offset < 0 || offset + 4 > Math.max(bytes.readUInt32LE(section + 8), raw)) continue;
            if (offset >= raw) return 0;
            assert(offset + 4 <= raw);
            return codeAt(va, 4, false).readUInt32LE();
        }
        throw Error('Data address unmapped: ' + va.toString(16));
    }
    return { initialDword, base, entryRva: bytes.readUInt32LE(optional + 16), codeAt, highLowFixups };
}

module.exports = { openImage };
