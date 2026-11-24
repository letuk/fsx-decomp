const assert = require('node:assert/strict');
const { createHash } = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { openImage } = require('./lib/pe32-dll.cjs');

const root = path.resolve(__dirname, '..');
const [target, run, ...controls] = process.argv.slice(2);
const targets = {
    'checked-add': { entry: 0x2014e674, symbol: '_AddUnsignedCheckedPilot@12' },
    'check-nonnegative': { entry: 0x201513ba, symbol: '_CheckNonnegativeLongPilot@8', evidenceDate: '20261124' },
    'checked-subtract': { entry: 0x2015347a, symbol: '_SubtractUnsignedCheckedPilot@12' },
    'checked-subtract64': { entry: 0x20171131, symbol: '_SubtractUnsigned64CheckedPilot@20' },
    'compare-unsigned32': { entry: 0x20153784, symbol: '_CompareUnsigned32Pilot@8', diagnostic: true },
    'checked-subtract64-forwarder': {
        entry: 0x201711d2, symbol: '_ForwardUnsigned64SubtractionPilot@20',
        call: { offset: 21, target: 0x20171131, symbol: '_SubtractUnsigned64CheckedPilot@20' }
    },
    'string-compare': { entry: 0x20137fd2, symbol: '_CompareStringsPilot@8', ordinal: 160, diagnostic: true }
};
assert(run && Object.hasOwn(targets, target), 'Pass a supported target and the retained factory run: ' + Object.keys(targets).join(', '));
const { entry, symbol, ordinal, diagnostic, call, evidenceDate = '20261119' } = targets[target];
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const evidence = `artifacts/api.dll/${target}-${evidenceDate}/ghidra-${entry.toString(16)}.json`;
const live = JSON.parse(fs.readFileSync(path.join(root, evidence), 'utf8'));
assert.equal(live.program, 'api.dll');
assert.equal(live.entry_point, entry.toString(16));
const length = parseInt(live.body_end, 16) - entry + 1;
assert.equal(length, live.size);

const original = fs.readFileSync(path.join(root, 'original/api.dll'));
const originalSHA256 = sha(original);
assert.equal(originalSHA256, '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const image = openImage(original);
if (ordinal !== undefined) {
    const pe = original.readUInt32LE(0x3c);
    const directory = image.codeAt(image.base + original.readUInt32LE(pe + 120), 40, false);
    const index = ordinal - directory.readUInt32LE(16);
    assert(index >= 0 && index < directory.readUInt32LE(20));
    const exportSlot = image.base + directory.readUInt32LE(28) + index * 4;
    assert.equal(image.base + image.codeAt(exportSlot, 4, false).readUInt32LE(), entry);
}

const originalBody = image.codeAt(entry, length);
assert.deepEqual(image.highLowFixups(entry, length), []);
let next = entry;
for (const [address, hex] of live.asm) {
    assert.equal(parseInt(address, 16), next);
    next += Buffer.from(hex, 'hex').length;
}
assert.equal(next, entry + length);
const originalHex = originalBody.toString('hex');
assert.equal(live.asm.map(([, hex]) => hex).join(''), originalHex);
assert.equal(live.assembledBytes, originalHex);

function compare(build) {
    const object = fs.readFileSync(path.join(build, 'api-factory.obj'));
    const coff = parseNativeCoff(object);
    const definition = coff.unique(symbol);
    assert(definition.section > 0);
    assert.equal(definition.value, 0);
    const section = coff.sections[definition.section - 1];
    assert(section.flags & 0x20, 'Expected a code section');
    const rebuilt = object.subarray(section.raw, section.raw + section.size);
    const normalized = Buffer.from(rebuilt);
    const mappings = [];
    if (call) {
        assert.equal(section.relocations.length, 1);
        const [reloc] = section.relocations;
        assert.equal(reloc.type, 0x14, 'Expected x86 REL32');
        assert.equal(reloc.offset, call.offset);
        assert.equal(reloc.symbol.name, call.symbol);
        assert(reloc.symbol.section > 0, 'Expected a defined callee');
        assert.equal(rebuilt[call.offset - 1], 0xe8);
        assert.equal(originalBody[call.offset - 1], 0xe8);
        const addend = rebuilt.readInt32LE(call.offset);
        assert.equal(addend, 0, 'Call must target the symbol entry');
        const originalDisplacement = originalBody.readInt32LE(call.offset);
        assert.equal(entry + call.offset + 4 + originalDisplacement, call.target);
        normalized.writeInt32LE(originalDisplacement, call.offset);
        mappings.push({offset: call.offset, kind: 'REL32', symbol: call.symbol, target: call.target.toString(16), addend});
    } else {
        assert.deepEqual(section.relocations, []);
    }
    const status = normalized.equals(originalBody) ? (call ? 'Normalized' : 'Exact') : 'Different';
    return {
        run: build, status, rebuiltBytes: rebuilt.length, rebuiltHex: rebuilt.toString('hex'),
        relocations: section.relocations.length, ...(call && {mappings, normalizedHex: normalized.toString('hex')}),
        objectSHA256: sha(object), buildCommandSHA256: sha(fs.readFileSync(path.join(build, 'build.cmd')))
    };
}

const result = {
    entry: live.entry_point, bodyEnd: live.body_end, ordinal, symbol,
    scope: 'Complete x86 COFF function body compared with the original PE. Linked DLL and runtime unverified.',
    originalSHA256, originalBytes: length, originalHex, evidence,
    ...compare(run), controls: controls.map(compare)
};
fs.writeFileSync(path.join(run, target + '-comparison.json'), JSON.stringify(result, null, 2) + '\n');
console.log(`${symbol}: ${result.status} (${result.rebuiltBytes}/${length} bytes, ${result.relocations} relocations)`);
if (!diagnostic) assert.equal(result.status, call ? 'Normalized' : 'Exact');
