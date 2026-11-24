const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');

const run = process.argv[2];
assert(run, 'Pass a NativeContext run directory');
const read = name => fs.readFileSync(path.join(run, name));
const request = JSON.parse(read('request.json'));
const receipt = JSON.parse(read('receipt.json'));
const sha = bytes => createHash('sha256').update(bytes).digest('hex').toUpperCase();
assert.equal(request.DispatcherProfile, 'NativeContext');
assert.equal(receipt.Job, request.Job);
assert.equal(receipt.ExitCode, 0);
assert.equal(sha(read('build.cmd')), request.BuildCommandSHA256);
assert.equal(sha(read('worker-output/compiled-source/scheduler_object.cpp')), request.SourceSHA256);

const root = path.resolve(__dirname, '..');
const pe = fs.readFileSync(path.join(root, 'original/simscheduler.dll'));
assert.equal(sha(pe), request.OriginalSHA256);
assert.equal(sha(pe), '16059B50E722BFA10357582DEF8D422A8222F620189E40C8878928A7DDAA9DD4');
const original = require('../artifacts/simscheduler.dll/context-native-class-ghidra.json');
assert.equal(original.program, 'simscheduler.dll');
const peHeader = pe.readUInt32LE(0x3c);
const optional = peHeader + 24;
const base = pe.readUInt32LE(optional + 28);
const sections = optional + pe.readUInt16LE(peHeader + 20);
function readVA(va, size) {
    for (let i = 0; i < pe.readUInt16LE(peHeader + 6); i++) {
        const section = sections + 40 * i;
        const delta = va - base - pe.readUInt32LE(section + 12);
        if (delta >= 0 && delta + size <= pe.readUInt32LE(section + 16)) {
            const raw = pe.readUInt32LE(section + 20) + delta;
            return pe.subarray(raw, raw + size);
        }
    }
    throw Error('Unmapped original VA ' + va.toString(16));
}
const reloc = readVA(base + pe.readUInt32LE(optional + 136), pe.readUInt32LE(optional + 140));
const fixups = new Set();
for (let block = 0; block < reloc.length;) {
    const page = reloc.readUInt32LE(block);
    const size = reloc.readUInt32LE(block + 4);
    assert(size >= 8 && size % 2 === 0 && block + size <= reloc.length);
    for (let i = block + 8; i < block + size; i += 2) {
        const code = reloc.readUInt16LE(i);
        if (code >>> 12 === 3) fixups.add(base + page + (code & 0xfff));
    }
    block += size;
}

const object = read('simscheduler-object.obj');
assert(object.equals(read('worker-output/fsx.exe')));
const coff = parseNativeCoff(object);
const coffSection = s => {
    const section = coff.sections[s.section - 1];
    assert(section && s.value === 0);
    return section;
};
const vtable = coff.unique('??_7SchedulerContext@@6B@');
const table = coffSection(vtable);
const methods = ["AddReference","ReleaseReference","GetName","SetStepBounds","SetMinimumStep","SetMaximumStep","GetMinimumStep","GetMaximumStep","GetPrimaryMode","SetPrimaryMode","GetUpdateEnabled","SetUpdateEnabled","SetSecondaryMode","SetTertiaryMode","RegisterEntry","UnregisterEntry","Reset","ReadTime","ReadSubsteps","GetContext"];
const tableVA = 0x20cc1200;
assert.equal(original.table, tableVA.toString(16));
assert.equal(table.size, 84);
assert.equal(table.relocations.length, 21);
assert.equal(readVA(tableVA - 4, 4).readUInt32LE(), 0);
assert(!fixups.has(tableVA - 4));
const imports = readPE32Imports(pe);

function compare(listing, symbol, cleanup, expectedImport, forwardTarget) {
    assert.equal(listing.program, 'simscheduler.dll');
    const first = listing.instructions[0], last = listing.instructions.at(-1);
    const va = parseInt(first.address, 16);
    const end = parseInt(last.address, 16);
    assert.equal(last.instruction, 'RET 0x' + cleanup.toString(16));
    const originalBytes = readVA(va, end - va + 3);
    const section = coffSection(symbol);
    const rebuilt = object.subarray(section.raw, section.raw + section.size);
    const tail = Buffer.from([0xc2, cleanup, 0]);
    assert(originalBytes.subarray(-3).equals(tail), symbol.name);
    let forwardsTo;
    if (!rebuilt.subarray(-3).equals(tail)) {
        const ref = section.relocations.find(candidate => candidate.offset === rebuilt.length - 4);
        assert.equal(rebuilt[rebuilt.length - 5], 0xe9, symbol.name);
        assert(ref && ref.type === 0x14, symbol.name);
        assert.equal(ref.symbol.name, forwardTarget);
        const target = coffSection(ref.symbol);
        assert(object.subarray(target.raw + target.size - 3, target.raw + target.size).equals(tail), ref.symbol.name);
        forwardsTo = ref.symbol.name;
    }
    const mappedOffsets = [];
    let status = rebuilt.equals(originalBytes) && section.relocations.length === 0 ? 'Exact' : 'Diagnostic';
    if (expectedImport) {
        const [offset, address, imported, target] = expectedImport;
        assert(fixups.has(va + offset));
        assert.equal(originalBytes.readUInt32LE(offset), address);
        assert.deepEqual(imports.get(address), { dll: 'kernel32.dll', name: imported });
        assert.equal(section.relocations.length, 1);
        const ref = section.relocations[0];
        assert.deepEqual([ref.offset, ref.type, ref.symbol.name, rebuilt.readUInt32LE(offset)], [offset, 6, target, 0]);
        mappedOffsets.push(offset);
        const normalized = Buffer.from(originalBytes);
        normalized.writeUInt32LE(0, offset);
        if (normalized.equals(rebuilt)) status = 'Normalized';
    }
    return { entry: first.address, symbol: symbol.name, cleanup, forwardsTo, status, mappedOffsets,
        originalBytes: originalBytes.length, rebuiltBytes: rebuilt.length,
        leftHex: originalBytes.toString('hex'), rightHex: rebuilt.toString('hex') };
}

const stackCleanup = [4, 4, 4, 20, 12, 12, 4, 4, 4, 8, 4, 8, 8, 8, 12, 8, 4, 8, 12, 4, 4];
const forwards = {
    9: '_SetSchedulerPrimaryMode@8', 11: '_SetSchedulerUpdateEnabled@8',
    14: '_RegisterSchedulerEntry@12', 15: '_UnregisterSchedulerEntry@8',
    16: '_ResetSchedulerContext@4',
};
const slots = stackCleanup.map((cleanup, i) => {
    const offset = i * 4;
    const ref = table.relocations[i];
    assert.equal(ref.offset, offset);
    assert.equal(ref.type, 6);
    assert.equal(object.readUInt32LE(table.raw + offset), 0);
    assert(fixups.has(tableVA + offset));
    const listing = original.functions[i];
    assert.equal(readVA(tableVA + offset, 4).readUInt32LE(), parseInt(listing.instructions[0].address, 16));
    let symbol = ref.symbol;
    if (i < methods.length) {
        assert(symbol.name.startsWith('?' + methods[i] + '@SchedulerContext@@UAG'), symbol.name);
    } else {
        assert.equal(symbol.name, '??_ESchedulerContext@@UAEPAXI@Z');
        assert.equal(symbol.storage, 105); // IMAGE_SYM_CLASS_WEAK_EXTERNAL
        const aux = object.readUInt32LE(8) + (symbol.index + 1) * 18;
        const fallback = coff.symbols.get(object.readUInt32LE(aux));
        assert.equal(fallback.name, '??_GSchedulerContext@@UAEPAXI@Z');
        symbol = coff.byName.get(fallback.name).find(candidate => candidate.section > 0);
        assert(symbol, 'Missing defined scalar deleting destructor');
    }
    const expectedImport = [
        [14, 0x20cc1030, 'InterlockedIncrement', '__imp__IncrementSchedulerPoolCount@4'],
        [16, 0x20cc102c, 'InterlockedDecrement', '__imp__DecrementSchedulerPoolCount@4'],
    ][i];
    return { offset, ...compare(listing, symbol, cleanup, expectedImport, forwards[i]) };
});
assert.equal(slots[1].status, 'Normalized');
const constructor = compare(original.functions[21], coff.unique('_ConstructSchedulerContext@8'), 8);
const constructorSection = coffSection(coff.unique('_ConstructSchedulerContext@8'));
assert(constructorSection.relocations.some(ref => ref.type === 6 && ref.symbol === vtable));
const report = { scope: 'VC8 SP1 x86 COFF only. All 21 VTable targets mapped by slot; no linked DLL or runtime test.',
    run: request.Job, objectSHA256: sha(object), table: '20cc1200', slots, constructor };
fs.writeFileSync(path.join(run, 'native-context-comparison.json'), JSON.stringify(report, null, 2) + '\n');
for (const fn of [...slots, constructor]) {
    console.log(fn.entry, fn.status, fn.originalBytes + '/' + fn.rebuiltBytes, fn.symbol);
}
