const assert = require('node:assert/strict');
const { createHash } = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { openImage } = require('./lib/pe32-dll.cjs');

const run = process.argv[2];
assert(run, 'Pass a retained API factory run directory');
const root = path.resolve(__dirname, '..');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const evidence = 'artifacts/api.dll/cab-error-20261119/ghidra-2014a51b.json';
const live = JSON.parse(fs.readFileSync(path.join(root, evidence), 'utf8'));
const entry = 0x2014a51b;
const length = 30;
const symbol = '_ErfSetCodes@12';
assert.equal(live.program, 'api.dll');
assert.equal(live.entry_point, entry.toString(16));
assert.equal(parseInt(live.body_end, 16), entry + length - 1);
assert.equal(live.name, symbol);

const original = fs.readFileSync(path.join(root, 'original/api.dll'));
const originalSHA256 = sha(original);
assert.equal(originalSHA256, '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const image = openImage(original);
const originalBody = image.codeAt(entry, length);
assert.deepEqual(image.highLowFixups(entry, length), []);
let next = entry;
for (const [address, hex] of live.asm) {
    assert.equal(parseInt(address, 16), next);
    next += Buffer.from(hex, 'hex').length;
}
assert.equal(next, entry + length);
assert.equal(live.asm.map(([, hex]) => hex).join(''), live.assembledBytes);
assert.equal(originalBody.toString('hex'), live.assembledBytes);

const object = fs.readFileSync(path.join(run, 'api-factory.obj'));
const coff = parseNativeCoff(object);
const definition = coff.unique(symbol);
assert(definition.section > 0);
assert.equal(definition.value, 0);
const section = coff.sections[definition.section - 1];
assert(section.flags & 0x20, 'Expected a code section');
assert.deepEqual(section.relocations, []);
const rebuilt = object.subarray(section.raw, section.raw + section.size);

const result = {
    status: rebuilt.equals(originalBody) ? 'Exact' : 'Different',
    scope: 'Complete x86 COFF function body compared with the original PE. No linked DLL or runtime verification.',
    entry: live.entry_point, bodyEnd: live.body_end, symbol,
    originalSHA256, objectSHA256: sha(object), buildCommandSHA256: sha(fs.readFileSync(path.join(run, 'build.cmd'))),
    originalBytes: originalBody.length, rebuiltBytes: rebuilt.length, relocations: 0,
    originalHex: originalBody.toString('hex'), rebuiltHex: rebuilt.toString('hex'), evidence
};
fs.writeFileSync(path.join(run, 'cab-error-comparison.json'), JSON.stringify(result, null, 2) + '\n');
assert.deepEqual(rebuilt, originalBody);
console.log(`${symbol}: ${length} exact bytes, zero relocations`);
