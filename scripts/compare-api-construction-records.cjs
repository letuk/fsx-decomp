const assert = require('node:assert/strict');
const { createHash } = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { openImage } = require('./lib/pe32-dll.cjs');

const root = path.resolve(__dirname, '..');
const run = process.argv[2];
assert(run, 'Pass the retained API factory run');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const evidence = 'artifacts/api.dll/construction-records-20261124/ghidra-construction-records.json';
const live = JSON.parse(fs.readFileSync(path.join(root, evidence), 'utf8'));
assert.equal(live.program, 'api.dll');
const pe = fs.readFileSync(path.join(root, 'original/api.dll'));
const originalSHA256 = sha(pe);
assert.equal(originalSHA256, '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const image = openImage(pe);
const object = fs.readFileSync(path.join(run, 'api-factory.obj'));
const coff = parseNativeCoff(object);
const callOffset = 6;

function verifiedBody(fn, entry, size) {
    assert.equal(fn.entry_point, entry.toString(16));
    assert.equal(fn.size, size);
    assert.equal(parseInt(fn.body_end, 16), entry + size - 1);
    let next = entry;
    for (const [address, hex] of fn.asm) {
        assert.equal(parseInt(address, 16), next);
        next += Buffer.from(hex, 'hex').length;
    }
    assert.equal(next, entry + size);
    const body = image.codeAt(entry, size);
    assert.equal(fn.asm.map(([, hex]) => hex).join(''), body.toString('hex'));
    return body;
}

assert.equal(live.constructors.length, 8);
assert.equal(live.defaultCopyRoutines.length, 8);
const dataEntry = 0x201fb138;
const defaultWordCounts = [7, 5, 5, 6, 8, 5, 8, 5];
let nextData = dataEntry;
const rows = defaultWordCounts.map((words, index) => {
    const entry = 0x2016198a + 20 * index;
    const target = 0x20161267 + 30 * index;
    const targetAddress = target.toString(16);
    const original = verifiedBody(live.constructors[index], entry, 20);
    assert.deepEqual(image.highLowFixups(entry, original.length), []);
    const callee = verifiedBody(live.defaultCopyRoutines[index], target, 30);
    assert.deepEqual(image.highLowFixups(target, callee.length), [21]);
    assert.equal(callee[20], 0xbe, 'Expected the copy source in the ESI immediate');
    assert.equal(callee.readUInt32LE(21), nextData);
    assert.equal(callee[15], 0x6a);
    assert.equal(callee[16], words);
    assert.equal(callee.readUInt16LE(25), 0xa5f3, 'Expected REP MOVSD');
    const defaults = { entry: nextData.toString(16), words };
    nextData += words * 4;

    const dependency = 'CopyConstructionDefaults_' + targetAddress;
    const symbols = ['YIXPAX', 'YIPAXPAX'].map(signature =>
        '??0?$ConstructionRecordPilot@$1?' + dependency + '@@' + signature + '@Z@@QAE@XZ'
    ).filter(candidate => coff.byName.has(candidate));
    assert.equal(symbols.length, 1, 'Expected one constructor with a known copy signature');
    const [symbol] = symbols;
    const definition = coff.unique(symbol);
    assert(definition.section > 0);
    assert.equal(definition.value, 0);
    const section = coff.sections[definition.section - 1];
    assert(section.flags & 0x20);
    assert.equal(section.size, original.length);
    assert.equal(section.relocations.length, 1);
    const relocation = section.relocations[0];
    assert.equal(relocation.type, 0x14, 'Expected x86 REL32');
    assert.equal(relocation.offset, callOffset);
    assert.equal(relocation.symbol.name, '@' + dependency + '@4');
    assert.equal(relocation.symbol.section, 0, 'Expected the external initialization boundary');
    const rebuilt = object.subarray(section.raw, section.raw + section.size);
    assert.equal(rebuilt[callOffset - 1], 0xe8);
    assert.equal(original[callOffset - 1], 0xe8);
    const addend = rebuilt.readInt32LE(callOffset);
    assert.equal(addend, 0);
    const nextInstruction = entry + callOffset + 4;
    assert.equal(nextInstruction + original.readInt32LE(callOffset), target);
    const normalized = Buffer.from(rebuilt);
    normalized.writeInt32LE(target + addend - nextInstruction, callOffset);
    assert.deepEqual(normalized, original);
    return {
        entry: entry.toString(16), symbol, status: 'Normalized', bytes: original.length,
        originalHex: original.toString('hex'), rebuiltHex: rebuilt.toString('hex'),
        normalizedHex: normalized.toString('hex'),
        relocation: { offset: callOffset, kind: 'REL32', symbol: relocation.symbol.name, target: targetAddress, addend },
        defaults
    };
});

const data = image.codeAt(dataEntry, nextData - dataEntry, false);
assert.equal(live.defaultData.program, 'api.dll');
assert.equal(live.defaultData.address, dataEntry.toString(16));
assert.deepEqual(data, Buffer.from(live.defaultData.hex, 'hex'));
assert.deepEqual(data, Buffer.alloc(data.length), 'Expected zero-initialized records');
const result = {
    run, evidence, originalSHA256, objectSHA256: sha(object),
    buildCommandSHA256: sha(fs.readFileSync(path.join(run, 'build.cmd'))),
    scope: 'Complete native x86 COFF constructor bodies at 0x2016198a–0x20161a29. Each REL32 is verified against its original callee and external symbol. Callee implementations, linked DLL and runtime remain open.',
    totalBytes: rows.reduce((bytes, row) => bytes + row.bytes, 0),
    defaultDataBytes: data.length, rows
};
fs.writeFileSync(path.join(run, 'construction-records-comparison.json'), JSON.stringify(result, null, 2) + '\n');
console.log(`${rows.length} normalized constructor bodies, ${result.totalBytes} bytes and ${rows.length} individually validated REL32 references`);
