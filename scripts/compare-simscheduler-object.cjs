const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');

const run = process.argv[2];
assert(run, 'Pass a SimScheduler pilot run directory');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const pe = fs.readFileSync(path.join(__dirname, '../original/simscheduler.dll'));
const originalSHA256 = sha(pe);
assert.equal(originalSHA256, '16059b50e722bfa10357582def8d422a8222f620189e40c8878928a7ddaa9dd4');
const header = pe.readUInt32LE(0x3c);
const optional = header + 24;
const base = pe.readUInt32LE(optional + 28);
const table = optional + pe.readUInt16LE(header + 20);

function sectionFor(va, size) {
    for (let i = 0; i < pe.readUInt16LE(header + 6); i++) {
        const section = table + i * 40;
        const delta = va - base - pe.readUInt32LE(section + 12);
        const extent = Math.max(pe.readUInt32LE(section + 8), pe.readUInt32LE(section + 16));
        if (delta >= 0 && delta + size <= extent) return { section, delta };
    }
    throw Error('Unmapped VA ' + va.toString(16));
}

function readVA(va, size) {
    const { section, delta } = sectionFor(va, size);
    assert(delta + size <= pe.readUInt32LE(section + 16));
    const raw = pe.readUInt32LE(section + 20) + delta;
    return pe.subarray(raw, raw + size);
}

const directory = optional + 96 + 5 * 8;
const relocations = readVA(base + pe.readUInt32LE(directory), pe.readUInt32LE(directory + 4));
const fixups = [];
for (let block = 0; block < relocations.length;) {
    const page = relocations.readUInt32LE(block);
    const length = relocations.readUInt32LE(block + 4);
    assert(length >= 8 && block + length <= relocations.length && length % 2 === 0);
    for (let offset = block + 8; offset < block + length; offset += 2) {
        const item = relocations.readUInt16LE(offset);
        const va = base + page + (item & 0xfff);
        if (item >>> 12) fixups.push({ va, type: item >>> 12 });
    }
    block += length;
}
const object = fs.readFileSync(path.join(run, 'simscheduler-object.obj'));
const coff = parseNativeCoff(object);

function verify({ entry, target, offsets, hex, functionSymbol, dataSymbol, file, dispatch,
    references = offsets.map(offset => ({ offset, target, symbol: dataSymbol, type: 6 })) }) {
    const original = readVA(entry, hex.length / 2);
    assert.equal(original.toString('hex'), hex);
    const bodyFixups = fixups.filter(({ va }) => va >= entry && va < entry + original.length);
    assert.deepEqual(bodyFixups, references.filter(ref => ref.type === 6)
        .map(({ offset }) => ({ va: entry + offset, type: 3 })));

    const fn = coff.unique(functionSymbol);
    const code = coff.sections[fn.section - 1];
    assert.equal(fn.value, 0);
    assert.equal(code.name, '.text');
    assert(code.flags & 0x1000, 'Expected a function COMDAT');
    assert.equal(code.size, original.length);
    const rebuilt = object.subarray(code.raw, code.raw + code.size);
    assert.deepEqual(code.relocations, references.map(({ offset, type, symbol }) =>
        ({ offset, type, symbol: coff.unique(symbol) })));
    const resolved = Buffer.from(rebuilt);
    const mappings = references.map(({ offset, target, symbol, type, imported, callbackHex, localFunction }) => {
        const external = coff.unique(symbol);
        assert.equal(external.storage, 2);
        if ((callbackHex || localFunction) && external.section > 0) {
            assert.equal(coff.sections[external.section - 1].name, '.text');
        } else {
            assert.equal(external.section, 0);
        }
        assert.equal(external.value, 0);
        assert.equal(rebuilt.readUInt32LE(offset), 0);
        const flags = pe.readUInt32LE(sectionFor(target, 4).section + 36);
        if (type === 6) {
            if (imported) {
                assert.deepEqual(readPE32Imports(pe).get(target), imported);
            } else if (callbackHex) {
                assert(flags & 0x20000000, 'Expected executable callback');
                assert.equal(readVA(target, callbackHex.length / 2).toString('hex'), callbackHex);
            } else {
                assert(flags & 0x80000000, 'Expected writable storage');
            }
            assert.equal(original.readUInt32LE(offset), target);
            resolved.writeUInt32LE(target, offset);
            return { offset, kind: 'DIR32', symbol, addend: 0, targetVA: target.toString(16), peFixup: 'HIGHLOW',
                ...(imported ? { imported } : {}), ...(callbackHex ? { callbackHex } : {}) };
        }
        assert.equal(type, 0x14);
        assert(flags & 0x20000000, 'Expected executable target');
        assert.equal(original[offset - 1], 0xe8);
        const displacement = target - (entry + offset + 4);
        assert.equal(original.readInt32LE(offset), displacement);
        resolved.writeInt32LE(displacement, offset);
        return { offset, kind: 'REL32', symbol, addend: 0, targetVA: target.toString(16) };
    });
    assert.deepEqual(resolved, original);

    const report = {
        program: 'simscheduler.dll',
        entry: entry.toString(16),
        status: 'Normalized',
        scope: 'Complete native x86 COFF function with target-validated relocations',
        originalSHA256,
        objectSHA256: sha(object),
        originalHex: original.toString('hex'),
        rebuiltHex: rebuilt.toString('hex'),
        ...(mappings.length === 1 ? { relocation: mappings[0] } : { relocations: mappings }),
        ...(dispatch ? { dispatch } : {}),
    };
    fs.writeFileSync(path.join(run, file), JSON.stringify(report, null, 2) + '\n');
    console.log(report.status + ' ' + report.entry + ' (' + original.length + ' bytes)');
}

const request = JSON.parse(fs.readFileSync(path.join(run, 'request.json'), 'utf8').replace(/^\uFEFF/, ''));
if (request.Entries.includes('20cc14d0')) {
    verify({
        entry: 0x20cc14d0, target: 0x20cc8370, offsets: [1], hex: 'b87083cc20c3',
        functionSymbol: '_GetSchedulerObject', dataSymbol: '_schedulerObject', file: 'comparison.json',
    });
}
if (request.Entries.includes('20cc15f0')) {
    const listings = require('../artifacts/simscheduler.dll/remaining-functions-ghidra.json').functions;
    const disassembly = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'utf8');
    for (const [entry, functionSymbol] of [
        ['20cc15f0', '_ReturnUnconstructedContextStorage'],
        ['20cc1ef0', '@ReleaseContextReferenceSlot@4'],
    ]) {
        const address = parseInt(entry, 16);
        const originalHex = listings[entry].instructions.map(i => i.bytes).join('');
        const original = readVA(address, originalHex.length / 2);
        assert.equal(original.toString('hex'), originalHex);
        const fn = coff.unique(functionSymbol);
        const section = coff.sections[fn.section - 1];
        assert.equal(section.name, '.text');
        assert.equal(fn.value, 0);
        const right = object.subarray(section.raw, section.raw + section.size);
        const block = disassembly.split(functionSymbol + ':')[1].split(/\r?\n\s*\r?\n/)[0];
        const rebuilt = [...block.matchAll(/^ +([0-9a-f]{8}): ((?:[0-9a-f]{2} )*[0-9a-f]{2}) +(\S[^\r\n]*)/gim)]
            .map(row => [row[1], row[2], row[3].trim().replace(/\s+/g, ' ')]);
        assert.equal(rebuilt.map(row => row[1].replaceAll(' ', '').toLowerCase()).join(''), right.toString('hex'));
        let notes;
        if (entry === '20cc15f0') {
            assert.equal(original[3], 0xe8);
            assert.equal(address + 8 + original.readInt32LE(4), 0x20cc20b0);
            const [relocation] = section.relocations;
            assert.equal(section.relocations.length, 1);
            assert.equal(relocation.type, 0x14);
            assert.equal(relocation.symbol.name, '_ReleaseSchedulerContextStorage');
            assert.equal(right[relocation.offset - 1], 0xe9);
            notes = 'Original EBX input and ECX-preserving call versus stack input and a tail jump.';
        } else {
            assert.equal(section.relocations.length, 0);
            assert.deepEqual(original.subarray(2), right.subarray(2));
            notes = 'Original pointer slot in EAX versus fastcall pointer slot in ECX. Remaining 13 bytes match.';
        }
        const report = { program: 'simscheduler.dll', entry, functionSymbol, status: 'Diagnostic',
            originalSHA256, objectSHA256: sha(object), originalHex, rebuiltHex: right.toString('hex'), rebuilt, notes };
        fs.writeFileSync(path.join(run, 'comparison-' + entry + '.json'), JSON.stringify(report, null, 2) + '\n');
        console.log('Diagnostic ' + entry + ' (' + original.length + ' original bytes, ' + right.length + ' rebuilt bytes)');
    }
}
const nameRegistry = [
    [0x20cc17d0, 220, '_FindNamedSchedulerContext@12'],
    [0x20cc18c0, 205, '_RemoveNamedSchedulerContext@8'],
    [0x20cc2150, 86, '_LookupSchedulerName@40'],
    [0x20cc22e0, 170, '_InsertNamedSchedulerContext@12'],
    [0x20cc2390, 165, '_FindSchedulerName@12'],
    [0x20cc2440, 307, '_ResizeSchedulerNameStorage'],
    [0x20cc2580, 110, '_InsertSchedulerNameSlot'],
    [0x20cc2600, 66, '_RemoveSchedulerNameSlot'],
    [0x20cc2650, 191, '_SetSchedulerNameCount'],
];
const compareRegistry = request.Job?.startsWith('simscheduler-name-registry-') ||
    request.Entries.includes('20cc17d0');
const compareFallback = request.Entries.includes('20cc1cc0');
const compareContextAccessors = request.Entries.includes('20cc2a80');
const compareObjectAccessors = request.Entries.includes('20cc19a0');
const compareContextReset = request.Entries.includes('20cc2e20');
const compareObjectLifecycle = request.Entries.includes('20cc1600');
const comparePoolLifecycle = request.Entries.includes('20cc72d0');
const comparePoolMetrics = request.Entries.includes('20cc3a60');
const compareFreeList = request.Entries.includes('20cc4d90');
const comparePoolLock = request.Entries.includes('20cc54b0');
if (compareRegistry || compareFallback || compareContextAccessors || compareObjectAccessors || compareContextReset ||
    compareObjectLifecycle || comparePoolLifecycle || comparePoolMetrics || compareFreeList || comparePoolLock) {
    const imports = readPE32Imports(pe);
    const targets = new Map(nameRegistry.map(([entry, , symbol]) => [entry, symbol]));
    const addressAddends = new Map();
    const functionTargets = new Map();
    const slots = [
        [0x20cc10d4, '#159', '__imp__GetSchedulerContextNameLength@4'],
        [0x20cc10d8, '#108', '__imp__ReleaseSchedulerContainerStorage@4'],
        [0x20cc10dc, '#106', '__imp__AllocateSchedulerContainerStorage@16'],
        [0x20cc10e8, '#110', '__imp__MovePendingStorage@12'],
        [0x20cc10f0, '#162', '__imp__CompareSchedulerContextNames@8'],
        [0x20cc10f4, '#157', '__imp__CopySchedulerContextName@12'],
    ];
    for (const [va, name, symbol] of slots) {
        assert.deepEqual(imports.get(va), { dll: 'api.dll', name });
        targets.set(va, symbol);
    }
    targets.set(0x20cc6e86, '_memcpy');
    targets.set(0x20cc8008, '___security_cookie');
    targets.set(0x20cc836c, '_schedulerPendingAllocator');
    targets.set(0x20cc7061, '__ehhandler$_FindNamedSchedulerContext@12');
    targets.set(0x20cc70e8, '__ehhandler$_RemoveNamedSchedulerContext@8');

    const listings = compareRegistry ? JSON.parse(fs.readFileSync(path.join(__dirname,
        '../artifacts/simscheduler.dll/name-registry-ghidra.json'), 'utf8')) : {};
    const functions = compareRegistry ? nameRegistry.slice() : [];
    if (compareFallback) {
        const listing = require('../artifacts/simscheduler.dll/fallback-context-ghidra.json').listing;
        listings['20cc1cc0'] = listing;
        functions.push([0x20cc1cc0, 542, '_UpdateSimConnectFallback@8']);
        targets.set(0x20cc7238, '__ehhandler$_UpdateSimConnectFallback@8');
        targets.set(0x20cc1670, '_CreateNamedSchedulerContext@12');
        targets.set(0x20cc8360, '_currentSchedulerContext');
        targets.set(0x20cc8364, '_simConnectFallbackSetting');
        targets.set(0x20cc8370, '_schedulerObject');
        targets.set(0x20cc11b8, '_schedulerFallbackName');
        const literal = coff.unique('_schedulerFallbackName');
        const section = coff.sections[literal.section - 1];
        const offset = section.raw + literal.value;
        assert.equal(object.subarray(offset, offset + 19).toString('ascii'), 'SimConnectFallback\0');
        assert.equal(readVA(0x20cc11b8, 19).toString('ascii'), 'SimConnectFallback\0');
    }
    if (compareContextAccessors) {
        const evidence = require('../artifacts/simscheduler.dll/context-accessors-ghidra.json');
        for (const [address, symbol, slot] of [
            ['20cc2780', '_GetSchedulerContext@4', 0x4c],
            ['20cc2a80', '_GetSchedulerContextName@4', 0x08],
            ['20cc2aa0', '_SetSchedulerStepBounds@20', 0x0c],
            ['20cc2ae0', '_SetSchedulerMinimumStep@12', 0x10],
            ['20cc2b20', '_SetSchedulerMaximumStep@12', 0x14],
            ['20cc2b60', '_GetSchedulerMinimumStep@4', 0x18],
            ['20cc2b80', '_GetSchedulerMaximumStep@4', 0x1c],
            ['20cc2ba0', '_GetSchedulerPrimaryMode@4', 0x20],
            ['20cc2bc0', '_SetSchedulerPrimaryMode@8', 0x24],
            ['20cc2c20', '_GetSchedulerUpdateEnabled@4', 0x28],
            ['20cc2c40', '_SetSchedulerUpdateEnabled@8', 0x2c],
            ['20cc2c80', '_SetSchedulerSecondaryMode@8', 0x30],
            ['20cc2cb0', '_SetSchedulerTertiaryMode@8', 0x34],
            ['20cc2fa0', '@PropagateSchedulerTime@16', null],
            ['20cc2ff0', '_ReadSchedulerContextTime@8', 0x44],
            ['20cc3010', '_ReadSchedulerSubsteps@12', 0x48],
        ]) {
            const entry = parseInt(address, 16);
            const body = evidence.functions[address];
            if (slot !== null) {
                const slotVA = 0x20cc1200 + slot;
                assert.equal(readVA(slotVA, 4).readUInt32LE(), entry);
                assert.equal(Buffer.from(evidence.table.hex, 'hex').readUInt32LE(slot), entry);
            }
            assert.equal(body.size, parseInt(body.body_end, 16) + 1 - entry);
            functions.push([entry, body.size, symbol]);
            listings[address] = body.listing;
            targets.set(entry, symbol);
        }
    }
    if (compareObjectAccessors) {
        const evidence = require('../artifacts/simscheduler.dll/scheduler-object-accessors-ghidra.json');
        const tableBytes = Buffer.from(evidence.table.hex, 'hex');
        const memberProfile = request.DispatcherProfile === 'ObjectAccessorsCpp';
        for (const [slot, entry, length, legacySymbol, memberSymbol] of [
            [0x0c, 0x20cc19a0, 18, '_WriteSchedulerMaximumSimSpeed@8', '?SetMaximumSimSpeed@SchedulerObject@@QAGXH@Z'],
            [0x10, 0x20cc19c0, 15, '_ReadSchedulerMaximumSimSpeed@4', '?GetMaximumSimSpeed@SchedulerObject@@QAGHXZ'],
            [0x14, 0x20cc19e0, 18, '_WriteSchedulerSimSpeed@8', '?SetSimSpeed@SchedulerObject@@QAGXH@Z'],
            [0x18, 0x20cc1a00, 15, '_ReadSchedulerSimSpeed@4', '?GetSimSpeed@SchedulerObject@@QAGHXZ'],
            [0x1c, 0x20cc1a20, 24, '_ReadSchedulerEffectiveSimSpeed@4', '?GetEffectiveSimSpeed@SchedulerObject@@QAGHXZ'],
            [0x20, 0x20cc1ad0, 20, '_ReadSchedulerSimTime@8', '?ReadSimTime@SchedulerObject@@QAGPANPAN@Z'],
            [0x24, 0x20cc1af0, 20, '_ReadSchedulerLastScaledStep@8', '?ReadLastScaledStep@SchedulerObject@@QAGPANPAN@Z'],
            [0x28, 0x20cc1b10, 15, '_ReadSchedulerLastInputStep@4', '?GetLastInputStep@SchedulerObject@@QAGNXZ'],
        ]) {
            assert.equal(readVA(0x20cc11cc + slot, 4).readUInt32LE(), entry);
            assert.equal(tableBytes.readUInt32LE(slot), entry);
            assert(fixups.some(fixup => fixup.va === 0x20cc11cc + slot && fixup.type === 3));
            functions.push([entry, length, memberProfile ? memberSymbol : legacySymbol]);
            listings[entry.toString(16)] = evidence.functions[entry.toString(16)];
        }
    }
    if (compareContextReset) {
        const evidence = require('../artifacts/simscheduler.dll/context-reset-ghidra.json');
        const entry = 0x20cc2e20;
        assert.equal(readVA(0x20cc1240, 4).readUInt32LE(), entry);
        assert.equal(Buffer.from(evidence.slot.hex, 'hex').readUInt32LE(), entry);
        const length = parseInt(evidence.function.body_end, 16) + 1 - entry;
        assert.equal(length, 86);
        functions.push([entry, length, '_ResetSchedulerContext@4']);
        listings['20cc2e20'] = evidence.listing;
        targets.set(0x20cc3770, '_ReleaseSchedulerEntry');
    }
    if (compareObjectLifecycle) {
        const evidence = require('../artifacts/simscheduler.dll/scheduler-object-lifecycle-ghidra.json');
        const crtSlot = 0x20cc111c;
        assert.equal(readVA(crtSlot, 4).readUInt32LE(), 0x20cc72b0);
        assert.equal(Buffer.from(evidence.startupTable.hex, 'hex').readUInt32LE(4), 0x20cc72b0);
        assert(fixups.some(({ va, type }) => va === crtSlot && type === 3));
        for (const [entry, length, symbol] of [
            [0x20cc1600, 102, '_InitializeSchedulerObject'],
            [0x20cc21b0, 135, '_DestroySchedulerNameIndex'],
            [0x20cc72b0, 17, '_InitializeSchedulerObjectAtStartup'],
            [0x20cc7300, 15, '_DestroySchedulerObject'],
        ]) {
            const address = entry.toString(16);
            const listing = evidence.functions[address];
            if (listing.dry_run) {
                const hex = listing.instructions.map(ins => ins.bytes).join('');
                assert.equal(readVA(entry, length).toString('hex'), hex);
                listings[address] = {
                    program: listing.program,
                    instructions: listing.instructions.map(({ address, mnemonic, operands }) =>
                        ({ address, instruction: (mnemonic + ' ' + operands).trim() })),
                };
            } else {
                listings[address] = listing;
            }
            functions.push([entry, length, symbol]);
            targets.set(entry, symbol);
        }
        targets.set(0x20cc6a66, '_atexit');
        targets.set(0x20cc11cc, '_schedulerObjectTable');
        for (const offset of [0, 4, 0x0c, 0x14, 0x1c, 0x20, 0x24, 0x28, 0x30, 0x34, 0x38, 0x3a, 0x3c]) {
            const target = 0x20cc8370 + offset;
            targets.set(target, '_schedulerObject');
            addressAddends.set(target, offset);
        }
    }
    if (comparePoolLifecycle) {
        const evidence = require('../artifacts/simscheduler.dll/pool-lifecycle-ghidra.json');
        for (const [va, target] of [[0x20cc1120, 0x20cc72d0], [0x20cc1124, 0x20cc72e0],
            [0x20cc1284, 0x20cc38c0], [0x20cc12b0, 0x20cc4430],
            [0x20cc128c, 0x20cc3a30], [0x20cc12b8, 0x20cc3a30]]) {
            const snapshot = va < 0x20cc1200 ? evidence.startupSlots : evidence.table;
            assert.equal(Buffer.from(snapshot.hex, 'hex').readUInt32LE(va - parseInt(snapshot.address, 16)), target);
            assert.equal(readVA(va, 4).readUInt32LE(), target);
            assert(fixups.some(field => field.va === va && field.type === 3));
        }
        for (const [entry, length, symbol, handler] of [
            [0x20cc72d0, 11, '_InitializeSchedulerContextPoolAtStartup'],
            [0x20cc72e0, 11, '_InitializeSchedulerEntryPoolAtStartup'],
            [0x20cc3cf0, 100, '_GetSchedulerContextPool', 0x20cc71ae],
            [0x20cc4650, 100, '_GetSchedulerEntryPool', 0x20cc717e],
            [0x20cc3eb0, 145, '_ConstructSchedulerContextPool', 0x20cc7118],
            [0x20cc46f0, 145, '_ConstructSchedulerEntryPool', 0x20cc7118],
            [0x20cc7320, 11, '_DestroySchedulerContextPoolAtExit'],
            [0x20cc7330, 11, '_DestroySchedulerEntryPoolAtExit'],
            [0x20cc3810, 171, '_DestroySchedulerContextPool@4', 0x20cc7148],
            [0x20cc4380, 171, '_DestroySchedulerEntryPool@4', 0x20cc7148],
            [0x20cc38c0, 143, '@DrainSchedulerContextPool@4'],
            [0x20cc4430, 143, '@DrainSchedulerEntryPool@4'],
            [0x20cc3580, 63, '@UnlinkSchedulerPool@4'],
            [0x20cc3a30, 31, '@CountUsedSchedulerPoolSlots@4'],
        ]) {
            const address = entry.toString(16);
            const listing = evidence.functions[address];
            if (listing.instructions[0].bytes) {
                assert.equal(readVA(entry, length).toString('hex'), listing.instructions.map(ins => ins.bytes).join(''));
            }
            functions.push([entry, length, symbol]);
            listings[address] = listing;
            targets.set(entry, symbol);
            const local = new Map();
            if (handler) local.set(handler, '__ehhandler$' + symbol);
            if (entry === 0x20cc38c0 || entry === 0x20cc4430) {
                local.set(0x20cc10d8, '__imp__ReleaseSchedulerPoolStorage@4');
            }
            functionTargets.set(entry, local);
        }
        assert.deepEqual(imports.get(0x20cc1028), { dll: 'kernel32.dll', name: 'DebugBreak' });
        for (const [target, symbol] of [
            [0x20cc6a66, '_atexit'],
            [0x20cc1028, '__imp__BreakOnSchedulerPoolFailure@0'],
            [0x20cc6580, '_CreateSchedulerPoolLock'],
            [0x20cc63e0, '_DestroySchedulerPoolLock'],
            [0x20cc54b0, '_AcquireSchedulerPoolLock@28'],
            [0x20cc61c0, '_ReleaseSchedulerPoolLock@8'],
            [0x20cc4eb0, '_DestroySchedulerPoolFreeList'],
            [0x20cc1254, '_schedulerPoolBaseTable'],
            [0x20cc1280, '_schedulerContextPoolTable'],
            [0x20cc12ac, '_schedulerEntryPoolTable'],
            [0x20cc83b8, '_schedulerListHead'],
            [0x20cc83b0, '_schedulerContextPool'],
            [0x20cc83b4, '_schedulerEntryPool'],
            [0x20cc83ec, '_schedulerEntryPoolGuard'],
            [0x20cc83f0, '_schedulerContextPoolGuard'],
        ]) targets.set(target, symbol);
        for (const [start, symbol] of [[0x20cc83d4, '_schedulerContextPoolStorage'], [0x20cc83bc, '_schedulerEntryPoolStorage']]) {
            for (let offset = 0; offset < 0x18; offset += 4) {
                targets.set(start + offset, symbol);
                addressAddends.set(start + offset, offset);
            }
        }
    }
    if (comparePoolMetrics) {
        const evidence = require('../artifacts/simscheduler.dll/pool-metrics-ghidra.json');
        const table = Buffer.from(evidence.table.hex, 'hex');
        for (const [base, entries] of [
            [0x20cc1280, [0x20cc3dd0, 0x20cc38c0, 0x20cc3960, 0x20cc3a30, 0x20cc3a60,
                0x20cc3ac0, 0x20cc3ad0, 0x20cc4610, 0x20cc4640, 0x20cc3ae0, 0x20cc3af0]],
            [0x20cc12ac, [0x20cc46c0, 0x20cc4430, 0x20cc44d0, 0x20cc3a30, 0x20cc45a0,
                0x20cc4600, 0x20cc3ad0, 0x20cc4610, 0x20cc4620, 0x20cc4630, 0x20cc4640]],
        ]) {
            entries.forEach((target, index) => {
                const slot = base + 4 * index;
                assert.equal(table.readUInt32LE(slot - 0x20cc1280), target);
                assert.equal(readVA(slot, 4).readUInt32LE(), target);
                assert(fixups.some(field => field.va === slot && field.type === 3));
            });
        }
        const baseEvidence = require('../artifacts/simscheduler.dll/pool-lifecycle-ghidra.json');
        assert.equal(Buffer.from(baseEvidence.table.hex, 'hex').readUInt32LE(), 0x20cc3d60);
        assert.equal(readVA(0x20cc1254, 4).readUInt32LE(), 0x20cc3d60);
        assert(fixups.some(field => field.va === 0x20cc1254 && field.type === 3));
        for (const [entry, length, symbol] of [
            [0x20cc3960, 189, '@TrimSchedulerContextPool@4'],
            [0x20cc44d0, 192, '@TrimSchedulerEntryPool@4'],
            [0x20cc3a60, 86, '@CountAvailableSchedulerContextSlots@4'],
            [0x20cc45a0, 86, '@CountAvailableSchedulerEntrySlots@4'],
            [0x20cc3ac0, 7, '@ReadSchedulerContextPoolCapacity@4'],
            [0x20cc4600, 7, '@ReadSchedulerEntryPoolCapacity@4'],
            [0x20cc3ad0, 4, '@ReadSchedulerPoolBlockCount@4'],
            [0x20cc4610, 4, '@ReadSchedulerPoolPeakBlockCount@4'],
            [0x20cc4640, 6, '@ReadSchedulerContextsPerBlock@4'],
            [0x20cc4620, 6, '@ReadSchedulerEntriesPerBlock@4'],
            [0x20cc3ae0, 10, '@ReadSchedulerContextPoolBlockBytes@4'],
            [0x20cc4630, 10, '@ReadSchedulerEntryPoolBlockBytes@4'],
            [0x20cc3af0, 6, '@ReadSchedulerContextStorageSize@4'],
            [0x20cc3d60, 96, '?destroy@SchedulerPoolRegistration@@QAEPAU1@I@Z'],
            [0x20cc3dd0, 40, '?destroy@?$SchedulerPool@U?$SchedulerPoolBlock@TSchedulerContextStorage@@$0BA@@@@@QAEPAU1@I@Z'],
            [0x20cc46c0, 40, '?destroy@?$SchedulerPool@U?$SchedulerPoolBlock@USchedulerTimedEntry@@$0IA@@@@@QAEPAU1@I@Z'],
        ]) {
            const address = entry.toString(16);
            const listing = evidence.functions[address];
            if (listing.instructions[0].bytes) {
                assert.equal(readVA(entry, length).toString('hex'), listing.instructions.map(ins => ins.bytes).join(''));
            }
            functions.push([entry, length, symbol]);
            listings[address] = listing;
            functionTargets.set(entry, new Map([[0x20cc10d8, '__imp__ReleaseSchedulerPoolStorage@4']]));
        }
        for (const [target, symbol] of [
            [0x20cc54b0, '_AcquireSchedulerPoolLock@28'],
            [0x20cc61c0, '_ReleaseSchedulerPoolLock@8'],
            [0x20cc4eb0, '_DestroySchedulerPoolFreeList'],
            [0x20cc1254, '_schedulerPoolBaseTable'],
            [0x20cc83b8, '_schedulerListHead'],
            [0x20cc3810, '_DestroySchedulerContextPool@4'],
            [0x20cc4380, '_DestroySchedulerEntryPool@4'],
        ]) targets.set(target, symbol);
    }
    if (compareFreeList) {
        const evidence = require('../artifacts/simscheduler.dll/free-list-ghidra.json');
        for (const [entry, length, symbol] of [
            [0x20cc4a40, 88, '_InitializeSchedulerAtomicMode'],
            [0x20cc4aa0, 144, '_AllocateSchedulerFreeListStorage'],
            [0x20cc4b40, 83, '_ReleaseSchedulerFreeListStorage'],
            [0x20cc4ba0, 147, '_AllocateAlignedSchedulerFreeListStorage'],
            [0x20cc4c40, 112, '_AllocateSchedulerFreeListBatch'],
            [0x20cc4cc0, 189, '_ReleaseSchedulerFreeListBatches'],
            [0x20cc4d90, 283, '_CreateSchedulerPoolFreeList@8'],
            [0x20cc4eb0, 238, '_DestroySchedulerPoolFreeList'],
            [0x20cc4fb0, 332, '_RefillSchedulerPoolFreeList@4'],
            [0x20cc5110, 455, '_PushSchedulerPoolEntry@8'],
            [0x20cc52e0, 452, '_PopSchedulerPoolEntry'],
        ]) {
            functions.push([entry, length, symbol]);
            listings[entry.toString(16)] = evidence.functions[entry.toString(16)];
            targets.set(entry, symbol);
        }
        for (const [target, name, parameterBytes] of [
            [0x20cc1020, 'InterlockedExchange', 8],
            [0x20cc1030, 'InterlockedIncrement', 4],
            [0x20cc1044, 'HeapAlloc', 12],
            [0x20cc1048, 'HeapFree', 12],
            [0x20cc1054, 'CloseHandle', 4],
            [0x20cc1058, 'GetSystemInfo', 4],
            [0x20cc105c, 'HeapSize', 12],
            [0x20cc1060, 'ReleaseSemaphore', 12],
            [0x20cc106c, 'GetProcessHeap', 0],
        ]) {
            assert.deepEqual(imports.get(target), { dll: 'kernel32.dll', name });
            targets.set(target, '__imp__' + name + '@' + parameterBytes);
        }
        targets.set(0x20cc8010, '_schedulerAtomicMode');
        targets.set(0x20cc6f5c, '_memset');
    }
    if (comparePoolLock) {
        const evidence = require('../artifacts/simscheduler.dll/pool-lock-ghidra.json');
        for (const [entry, length, symbol] of [
            [0x20cc4a10, 41, '_PauseSchedulerPoolLock'],
            [0x20cc54b0, 3325, '_AcquireSchedulerPoolLock@28'],
            [0x20cc61c0, 538, '_ReleaseSchedulerPoolLock@8'],
            [0x20cc63e0, 396, '_DestroySchedulerPoolLock'],
            [0x20cc6580, 242, '_CreateSchedulerPoolLock'],
        ]) {
            functions.push([entry, length, symbol]);
            listings[entry.toString(16)] = evidence.functions[entry.toString(16)];
            targets.set(entry, symbol);
        }
        for (const [target, name, parameterBytes] of [
            [0x20cc1000, 'CreateSemaphoreA', 16],
            [0x20cc100c, 'GetCurrentThreadId', 0],
            [0x20cc1020, 'InterlockedExchange', 8],
            [0x20cc1030, 'InterlockedIncrement', 4],
            [0x20cc1048, 'HeapFree', 12],
            [0x20cc104c, 'SetEvent', 4],
            [0x20cc1050, 'WaitForSingleObjectEx', 12],
            [0x20cc1054, 'CloseHandle', 4],
            [0x20cc1058, 'GetSystemInfo', 4],
            [0x20cc1060, 'ReleaseSemaphore', 12],
            [0x20cc1068, 'CreateEventA', 16],
            [0x20cc106c, 'GetProcessHeap', 0],
        ]) {
            assert.deepEqual(imports.get(target), { dll: 'kernel32.dll', name });
            targets.set(target, '__imp__' + name + '@' + parameterBytes);
        }
        targets.set(0x20cc4a40, '_InitializeSchedulerAtomicMode');
        targets.set(0x20cc4ba0, '_AllocateAlignedSchedulerFreeListStorage');
        targets.set(0x20cc8010, '_schedulerAtomicMode');
    }
    for (const [entry, length, symbol] of functions) {
        const address = entry.toString(16);
        const listing = listings[address];
        assert.equal(listing.program, 'simscheduler.dll');
        assert.equal(listing.instructions[0].address, address);
        const original = readVA(entry, length);
        const fn = coff.unique(symbol);
        const code = coff.sections[fn.section - 1];
        assert.equal(fn.value, 0);
        assert.equal(code.name, '.text');
        assert(code.flags & 0x1000);
        const rebuilt = object.subarray(code.raw, code.raw + code.size);
        const fields = fixups.filter(({ va }) => va >= entry && va < entry + length)
            .map(({ va, type }) => {
                assert.equal(type, 3);
                return { offset: va - entry, type: 6, target: original.readUInt32LE(va - entry) };
            });
        for (const { address: instructionVA, instruction } of listing.instructions) {
            const call = /^CALL 0x([0-9a-f]+)$/i.exec(instruction);
            if (call) {
                const offset = parseInt(instructionVA, 16) - entry + 1;
                const target = parseInt(call[1], 16);
                assert.equal(original[offset - 1], 0xe8);
                assert.equal(entry + offset + 4 + original.readInt32LE(offset), target);
                fields.push({ offset, type: 20, target });
            }
        }
        fields.sort((a, b) => a.offset - b.offset);
        const remaining = code.relocations.slice();
        const resolved = Buffer.from(rebuilt);
        const references = [];
        const originalOnly = [];
        for (const field of fields) {
            const name = functionTargets.get(entry)?.get(field.target) || targets.get(field.target);
            assert(name, 'Unidentified original target ' + field.target.toString(16));
            const expectedAddend = field.type === 6 ? addressAddends.get(field.target) || 0 : 0;
            const matchesTarget = rel => rel.type === field.type && rel.symbol.name === name &&
                rebuilt.readUInt32LE(rel.offset) === expectedAddend;
            const sameOffset = remaining.findIndex(rel => rel.offset === field.offset && matchesTarget(rel));
            const index = sameOffset >= 0 ? sameOffset : remaining.findIndex(rel =>
                matchesTarget(rel));
            if (index < 0) {
                originalOnly.push({ ...field, symbol: name, ...(expectedAddend ? { expectedAddend } : {}) });
                continue;
            }
            const [rel] = remaining.splice(index, 1);
            const addend = rebuilt.readUInt32LE(rel.offset);
            references.push({ ...field, rebuiltOffset: rel.offset, symbol: name, addend });
            if (rel.offset === field.offset) {
                original.copy(resolved, rel.offset, field.offset, field.offset + 4);
            }
        }
        const rebuiltOnly = remaining.map(rel => ({
            offset: rel.offset, type: rel.type, symbol: rel.symbol.name,
        }));
        const sameFields = !originalOnly.length && !rebuiltOnly.length &&
            references.every(ref => ref.offset === ref.rebuiltOffset);
        const matched = sameFields && original.equals(resolved);
        const status = matched ? (original.equals(rebuilt) ? 'Exact' : 'Normalized') : 'Diagnostic';
        const report = {
            program: 'simscheduler.dll', entry: address, status, symbol,
            originalSHA256, objectSHA256: sha(object),
            originalHex: original.toString('hex'), rebuiltHex: rebuilt.toString('hex'),
            references, originalOnly, rebuiltOnly,
        };
        fs.writeFileSync(path.join(run, 'comparison-' + address + '.json'), JSON.stringify(report, null, 2) + '\n');
        console.log(status + ' ' + address + ' (' + original.length + ' original bytes, ' + rebuilt.length + ' rebuilt bytes)');
    }
}
if (request.Entries.includes('20cc14e0')) {
    verify({
        entry: 0x20cc14e0, target: 0x20cc8368, offsets: [9], hex: '8bff558bec8b4508a36883cc205dc20400',
        functionSymbol: '_SetSchedulerDispatchKey@4', dataSymbol: '_schedulerDispatchKey', file: 'comparison-20cc14e0.json',
    });
}
function verifyDispatch({ entry, slot, targetVA, targetHex, hex, functionSymbol, file,
    offsets = [1, 9], abi = 'int __stdcall(SchedulerObject*)' }) {
    const objectVA = 0x20cc8370;
    const tableVA = 0x20cc11cc;
    const initializer = readVA(0x20cc1630, 10);
    assert.equal(initializer.readUInt16LE(0), 0x05c7);
    assert.equal(initializer.readUInt32LE(2), objectVA);
    assert.equal(initializer.readUInt32LE(6), tableVA);
    assert.equal(readVA(tableVA + slot, 4).readUInt32LE(), targetVA);
    assert.equal(readVA(targetVA, targetHex.length / 2).toString('hex'), targetHex);
    for (const va of [0x20cc1632, 0x20cc1636, tableVA + slot]) {
        assert(fixups.some(fixup => fixup.va === va && fixup.type === 3));
    }
    verify({
        entry, target: objectVA, offsets, hex,
        functionSymbol, dataSymbol: '_schedulerObject', file,
        dispatch: { tableVA: tableVA.toString(16), slot, targetVA: targetVA.toString(16), abi },
    });
}
if (request.Entries.includes('20cc1560')) {
    verifyDispatch({
        entry: 0x20cc1560, slot: 0x1c, targetVA: 0x20cc1a20,
        targetHex: '8bff558bec8b4d088b411c8b49203bc17c028bc15dc20400',
        hex: 'a17083cc208b481c687083cc20ffd1c3',
        functionSymbol: '_GetSchedulerFieldMinimum', file: 'comparison-20cc1560.json',
    });
}
if (request.Entries.includes('20cc15b0')) {
    verifyDispatch({
        entry: 0x20cc15b0, slot: 0x18, targetVA: 0x20cc1a00,
        targetHex: '8bff558bec8b45088b401c5dc20400',
        hex: 'a17083cc208b4818687083cc20ffd1c3',
        functionSymbol: '_GetSchedulerSimSpeed', file: 'comparison-20cc15b0.json',
    });
}
if (request.Entries.includes('20cc1580')) {
    verifyDispatch({
        entry: 0x20cc1580, slot: 0x14, targetVA: 0x20cc19e0,
        targetHex: '8bff558bec8b45088b4d0c89481c5dc20800',
        hex: '8bff558bec8b45088b0d7083cc208b511450687083cc20ffd25dc20400',
        offsets: [10, 19], abi: 'void __stdcall(SchedulerObject*, int)',
        functionSymbol: '_SetSchedulerSimSpeed@4', file: 'comparison-20cc1580.json',
    });
}
if (request.Entries.includes('20cc1530')) {
    verifyDispatch({
        entry: 0x20cc1530, slot: 0x20, targetVA: 0x20cc1ad0,
        targetHex: '8bff558bec8b4508dd40048b450cdd185dc20800',
        hex: '8bff558bec8b0d7083cc208b512083ec088d45f850687083cc20ffd2dd008be55dc3',
        offsets: [7, 22], abi: 'double* __stdcall(SchedulerObject*, double*)',
        functionSymbol: '_GetSchedulerSimTime', file: 'comparison-20cc1530.json',
    });
}
if (request.Entries.includes('20cc1500')) {
    assert.equal(readVA(0x20cc1cfd, 5).toString('hex'), '68b811cc20');
    assert.equal(readVA(0x20cc11b8, 19).toString('ascii'), 'SimConnectFallback\0');
    for (const ret of [0x20cc1eaf, 0x20cc1edb]) {
        assert.equal(readVA(ret, 3).toString('hex'), 'c20800');
    }
    verify({
        entry: 0x20cc1500, hex: '8bff558beca16083cc20568b75085650e8ab07000089356483cc205e5dc20400',
        functionSymbol: '_SetSimConnectFallback@4', file: 'comparison-20cc1500.json',
        references: [
            { offset: 6, type: 6, target: 0x20cc8360, symbol: '_currentSchedulerContext' },
            { offset: 17, type: 0x14, target: 0x20cc1cc0, symbol: '_UpdateSimConnectFallback@8', localFunction: true },
            { offset: 23, type: 6, target: 0x20cc8364, symbol: '_simConnectFallbackSetting' },
        ],
    });
}
if (request.Entries.includes('20cc1460')) {
    verify({
        entry: 0x20cc1460, hex: '8bff568b35fc10cc2068c013cc206a20ffd6680014cc206a10ffd65ec3',
        functionSymbol: '_RegisterSchedulerCallbacks', file: 'comparison-20cc1460.json',
        references: [
            { offset: 5, type: 6, target: 0x20cc10fc, symbol: '__imp__RegisterApiCallback@8',
                imported: { dll: 'api.dll', name: '#231' } },
            { offset: 10, type: 6, target: 0x20cc13c0, symbol: '_SchedulerGroup20Callback@8',
                callbackHex: 'ff151011cc2085c07522ff15cc10cc20' },
            { offset: 19, type: 6, target: 0x20cc1400, symbol: '_SchedulerGroup10Callback@8',
                callbackHex: '8bff558bec83e4f8518b450883e82556' },
        ],
    });
}
if (request.Entries.includes('20cc1490')) {
    verify({
        entry: 0x20cc1490, hex: '8bff568b35d010cc20680014cc206a10ffd668c013cc206a20ffd68b35b883cc2085f674108b068b50048bceffd28b760485f675f05ec3',
        functionSymbol: '_UnregisterSchedulerCallbacks', file: 'comparison-20cc1490.json',
        references: [
            { offset: 5, type: 6, target: 0x20cc10d0, symbol: '__imp__UnregisterApiCallback@8',
                imported: { dll: 'api.dll', name: '#232' } },
            { offset: 10, type: 6, target: 0x20cc1400, symbol: '_SchedulerGroup10Callback@8',
                callbackHex: '8bff558bec83e4f8518b450883e82556' },
            { offset: 19, type: 6, target: 0x20cc13c0, symbol: '_SchedulerGroup20Callback@8',
                callbackHex: 'ff151011cc2085c07522ff15cc10cc20' },
            { offset: 29, type: 6, target: 0x20cc83b8, symbol: '_schedulerListHead' },
        ],
    });
}
if (request.Entries.includes('20cc2720')) {
    for (const [address, target] of [[0x20cc1200, 0x20cc2720], [0x20cc1204, 0x20cc2740], [0x20cc1250, 0x20cc28c0]]) {
        assert.equal(readVA(address, 4).readUInt32LE(), target);
        assert(fixups.some(fixup => fixup.va === address && fixup.type === 3));
    }
    verify({
        entry: 0x20cc2720, hex: '8bff558bec8b450883c00450ff153010cc205dc20400',
        functionSymbol: '_AddSchedulerContextReference@4', file: 'comparison-20cc2720.json',
        references: [{ offset: 14, type: 6, target: 0x20cc1030, symbol: '__imp__IncrementSchedulerPoolCount@4',
            imported: { dll: 'kernel32.dll', name: 'InterlockedIncrement' } }],
    });
    verify({
        entry: 0x20cc2740, hex: '8bff558bec568b7508578d460450ff152c10cc208bf885ff751185f6740d8b168b42506a018bceffd08bc75f5e5dc20400',
        functionSymbol: '_ReleaseSchedulerContextReference@4', file: 'comparison-20cc2740.json',
        references: [{ offset: 16, type: 6, target: 0x20cc102c, symbol: '__imp__DecrementSchedulerPoolCount@4',
            imported: { dll: 'kernel32.dll', name: 'InterlockedDecrement' } }],
    });
}
if (request.Entries.includes('20cc13c0')) {
    assert.equal(readVA(0x20cc11f8, 4).readUInt32LE(), 0x20cc1b90);
    assert.equal(readVA(0x20cc1bba, 3).toString('hex'), 'dc4d0c');
    assert.equal(readVA(0x20cc1ca9, 3).toString('hex'), 'c20c00');
    for (const va of [0x20cc1632, 0x20cc1636, 0x20cc11f8]) {
        assert(fixups.some(fixup => fixup.va === va && fixup.type === 3));
    }
    verify({
        entry: 0x20cc13c0, hex: 'ff151011cc2085c07522ff15cc10cc2085c07418dd4064a17083cc208b482c83ec08dd1c24687083cc20ffd1c20800',
        functionSymbol: '_SchedulerGroup20Callback@8', file: 'comparison-20cc13c0.json',
        dispatch: { tableVA: '20cc11cc', slot: 0x2c, targetVA: '20cc1b90', abi: 'void __stdcall(SchedulerObject*, double)' },
        references: [
            { offset: 2, type: 6, target: 0x20cc1110, symbol: '__imp__CanSkipSchedulerUpdate',
                imported: { dll: 'main.dll', name: '#19' } },
            { offset: 12, type: 6, target: 0x20cc10cc, symbol: '__imp__GetSchedulerTimeInput',
                imported: { dll: 'api.dll', name: '#269' } },
            { offset: 24, type: 6, target: 0x20cc8370, symbol: '_schedulerObject' },
            { offset: 38, type: 6, target: 0x20cc8370, symbol: '_schedulerObject' },
        ],
    });
}
if (request.Entries.includes('20cc3490') || process.argv[3] === '--pending-registration') {
    const resize = '?ResizePendingSchedulerEntries@@YAHPAUSchedulerPendingQueue@@H@Z';
    const targets = [
        [0x20cc3490, '_QueueSchedulerRegistration@20', '8bff558bec518b8a8000000056578dba8000000033c085c97e178bb28c0000008d5604391a741583c00183c2103bc17cf28bf183c601794633f6eb4f8b55108bc8c1e1043b1431750a5f33c05e8be55dc20c008b570c03ca8b172bd083ea01c1e204528d41105051ff15e810cc208307ff5fb8010000005e8be55dc20c0081feffffff077e05beffffff0785f68bc67d0433c0eb0d81feffffff077e05b8ffffff073b47047e0985c07c05e8900c00008b570c8b4d108937c1e6048d4416f08b550c89088b4d085f89580489500889480cb8010000005e8be55dc20c00', [
            [106, 6, '__imp__MovePendingStorage@12', 0x20cc10e8],
            [172, 0x14, resize, 0x20cc41d0],
        ]],
        [0x20cc41d0, resize, '8bff558bec83ec08538bd881fbffffff077e0733c05b8be55dc35633f63bde8975fc7e36663977087f0666c747080500f6470a0474120fbf77083bde7e1f8bff03f63bde7ffaeb150fbf4f088d4419ff99f7f98bf00faff1eb038975fc3b7704750b5eb8010000005b8be55dc385f67e22a16c83cc20508d4df8518bd6c1e204526a01ff15dc10cc208b45f885c08945fc74138b073bd87c028bd885db7e228b45fc85c075085e33c05b8be55dc38b570c8bcbc1e104515250e8f82b000083c40c8b470c8d4df851891f8945f8ff15d810cc208b55fc897704b8010000006609470a5e89570c5b8be55dc3', [
            [114, 6, '_schedulerPendingAllocator', 0x20cc836c],
            [133, 6, '__imp__AllocatePendingStorage@16', 0x20cc10dc],
            [186, 0x14, '_memcpy', 0x20cc6e86],
            [207, 6, '__imp__ReleasePendingStorage@4', 0x20cc10d8],
        ]],
    ];
    if (request.Entries.includes('20cc27a0')) {
        const imports = readPE32Imports(pe);
        assert.deepEqual(imports.get(0x20cc10f4), { dll: 'api.dll', name: '#157' });
        assert.deepEqual(imports.get(0x20cc10d8), { dll: 'api.dll', name: '#108' });
        assert.equal(readVA(0x20cc1250, 4).readUInt32LE(), 0x20cc28c0);
        assert(fixups.some(fixup => fixup.va === 0x20cc1250 && fixup.type === 3));
        const constructor = request.LifecycleFactory ? '_ConstructSchedulerContext@8' : '??0SchedulerContext@@QAE@PBD@Z';
        const lifecycle = [
            [0x20cc27a0, constructor,
                '8bff558bec6aff68b470cc2064a1000000005083ec105657a10880cc2033c5508d45f464a3000000008b7508d9eec7060012cc2033ff897e04dd5644dd5e4c897e088d4608ba05000000b904000000897e6c897e60897e646689566866894e6a897dfc897e7c897e70897e746689567866894e7a89be8c00000089be8000000089be840000006689968800000066898e8a0000008b4d0c6a205150c645fc02ff15f410cc20d9e8836638f8dd562883663cfcdd5e30d9ee8d55ecdd5e54c74640ffffffff897e5ca17083cc208b482052687083cc20ffd1d9ee8b108956448b400489464883ec088d45e4dd1c24e83eedffff8b08894e4c8b50048956508bc68b4df464890d00000000595f5e8be55dc20800', [
                    [8, 6, '__ehhandler$' + constructor, 0x20cc70b4],
                    [25, 6, '___security_cookie', 0x20cc8008],
                    [48, 6, '_schedulerContextTable', 0x20cc1200],
                    [161, 6, '__imp__CopySchedulerContextName@12', 0x20cc10f4],
                    [200, 6, '_schedulerObject', 0x20cc8370],
                    [209, 6, '_schedulerObject', 0x20cc8370],
                    [238, 0x14, '_InitializeContextTimeValue@12', 0x20cc15d0],
                ]],
            [0x20cc2940, '??1SchedulerContext@@QAE@XZ',
                '8bff558bec6aff688472cc2064a1000000005083ec08535657a10880cc2033c5508d45f464a3000000008bf18975ecc7060012cc20c745fc0200000033db39356083cc207512a16483cc205053e82ef3ffff891d6083cc20c645fc018b8e8c0000003bcb8b3dd810cc207440f6868a000000017437399e84000000742f8b86800000003bc37e0233c0894df08d4df051898680000000ffd766838e8a00000001899e8c000000899e84000000885dfc8b4e7c3bcb742bf6467a017425395e7474208b46703bc37e0233c08d55f052894670894df0ffd766834e7a01895e7c895e74c745fcffffffff8b4e6c3bcb742bf6466a017425395e6474208b46603bc37e0233c08946608d45f050894df0ffd766834e6a01895e6c895e648b4df464890d00000000595f5e5b8be55dc3', [
                    [8, 6, '__ehhandler$??1SchedulerContext@@QAE@XZ', 0x20cc7284],
                    [26, 6, '___security_cookie', 0x20cc8008],
                    [49, 6, '_schedulerContextTable', 0x20cc1200],
                    [64, 6, '_currentSchedulerContext', 0x20cc8360],
                    [71, 6, '_simConnectFallbackSetting', 0x20cc8364],
                    [78, 0x14, '_UpdateSimConnectFallback@8', 0x20cc1cc0],
                    [84, 6, '_currentSchedulerContext', 0x20cc8360],
                    [102, 6, '__imp__ReleaseSchedulerContainerStorage@4', 0x20cc10d8],
                ]],
            [0x20cc28c0, '?destroy@SchedulerContext@@QAEPAU1@I@Z',
                '8bff558bec538bd9e873000000f64508017405e8d8f7ffff8bc35b5dc20400', [
                    [9, 0x14, '??1SchedulerContext@@QAE@XZ', 0x20cc2940],
                    [20, 0x14, '_ReleaseSchedulerContextStorage', 0x20cc20b0],
                ]],
            [0x20cc15d0, '_InitializeContextTimeValue@12',
                '8bff558bec83e4f8dd4508dd188be55dc20800', [

                ]],
            [0x20cc28f0, '_ClearContextPendingStorage',
                '8bff558bec518b4e0c85c97436f6460a017430837e0400742a8b0685c07e0233c089068d45fc50894dfcff15d810cc2066834e0a01c7460c00000000c74604000000008be55dc3', [
                    [44, 6, '__imp__ReleaseSchedulerContainerStorage@4', 0x20cc10d8],
                ]],
        ];
        targets.push(...lifecycle.filter(([entry]) => request.Entries.includes(entry.toString(16))));
        for (const symbol of [constructor, '??1SchedulerContext@@QAE@XZ']) {
            const handler = coff.unique('__ehhandler$' + symbol);
            const handlerCode = coff.sections[handler.section - 1];
            assert.equal(handlerCode.name, '.text$x');
            assert(handlerCode.relocations.some(rel => rel.symbol?.name === '__ehfuncinfo$' + symbol));
            for (let i = 0; i < 3; i++) coff.unique('__unwindfunclet$' + symbol + '$' + i);
        }
    }
    for (const [entry, symbol, hex, expected] of targets) {
        const original = readVA(entry, hex.length / 2);
        assert.equal(original.toString('hex'), hex);
        const fn = coff.unique(symbol);
        const code = coff.sections[fn.section - 1];
        assert.equal(fn.value, 0);
        assert.equal(code.name, '.text');
        assert(code.flags & 0x1000, 'Expected a function COMDAT');
        const rebuilt = object.subarray(code.raw, code.raw + code.size);
        assert.equal(code.relocations.length, expected.length);
        const remaining = code.relocations.slice();
        const references = expected.map(([offset, type, name, target]) => {
            const originalTarget = type === 6 ? original.readUInt32LE(offset)
                : entry + offset + 4 + original.readInt32LE(offset);
            assert.equal(originalTarget, target);
            const index = remaining.findIndex(rel => rel.type === type && rel.symbol.name === name);
            assert(index >= 0, name);
            const [relocation] = remaining.splice(index, 1);
            assert.equal(rebuilt.readUInt32LE(relocation.offset), 0);
            return { originalOffset: offset, rebuiltOffset: relocation.offset, symbol: name, targetVA: target.toString(16) };
        });
        const report = {
            program: 'simscheduler.dll', entry: entry.toString(16), status: 'Diagnostic',
            scope: 'Complete native COFF pending registration body with checked target identities',
            originalSHA256, objectSHA256: sha(object),
            originalHex: hex, rebuiltHex: rebuilt.toString('hex'), references,
        };
        fs.writeFileSync(path.join(run, 'comparison-' + report.entry + '.json'), JSON.stringify(report, null, 2) + '\n');
        console.log('Diagnostic ' + report.entry + ' (' + original.length + ' original bytes, ' + rebuilt.length + ' rebuilt bytes)');
    }
}
if (request.Entries.includes('20cc2ce0')) {
    const targets = [
        [550256720,"?FindSchedulerEntry@@YA_NPBUSchedulerEntryIndex@@PBQAXPAPAUSchedulerEntryIndexRecord@@@Z","8bff558bec5156578b780c8b0033c985c08945fc8bf076268b4508538b188bff8bc62bc1d1e803c18b14c73bda76058d4801eb028bf03bce72e68b45fc5b8b550c85d28d3ccf7402893a3bc873178b4d088b018b0f3bc1720c770a5fb0015e8be55dc208005f32c05e8be55dc20800",[]],
        [550251744,"_RegisterSchedulerEntry@12","8bff558bec83e4f883ec14538b5d0c33c03b1d6883cc20568b75080f94c0f6463c02578bf874178b4d106a0151578bd6e87b0700005f5e5b8be55dc20c00837e70008d4670895c240c8944241c74278d542410528d4c241051e81213000084c074148b44241083c004740b33c05f5e5b8be55dc20c00e87508000085c0743cdd464483e701dd5c24108b542410d9ee8b4c2414dd580403ff8950048b551083e2010bfa8948088b0803ff03ff83e1f10bf983cf0189580c8938eb0233c08b156483cc20525689442414e812efffff837e60008d466089356083cc2075138d4c240c51e8090e0000c7464000000000eb29837d10007412f6463801740c8d54240c52e8ea0d0000eb118b5e408d4c240c51e80b0d0000834640018b74241c8d54240c528d7d0ce8160e00005f5eb8010000005b8be55dc20c00",[[19,6,"_schedulerDispatchKey",550273896],[49,20,"_QueueSchedulerRegistration@20",550253712],[90,20,"?FindSchedulerEntry@@YA_NPBUSchedulerEntryIndex@@PBQAXPAPAUSchedulerEntryIndexRecord@@@Z",550256720],[119,20,"_AcquireSchedulerEntry",550254032],[191,6,"_simConnectFallbackSetting",550273892],[202,20,"_UpdateSimConnectFallback@8",550247616],[215,6,"_currentSchedulerContext",550273888],[227,20,"_AppendSchedulerEntry@8",550255568],[258,20,"_AppendSchedulerEntry@8",550255568],[273,20,"_InsertSchedulerEntry@12",550255360],[294,20,"_InsertSchedulerEntryIndex@12",550255648]]],
        [550252160,"_UnregisterSchedulerEntry@8","8bff558bec5153568b750c578b7d08837f70008d5f708975fc74478d4508508d4dfc518bc3e8a611000084c07434f6473c028b55088b72048975080f84900000008b5d0c6a006a006a008bd7e8bf050000830e025f5eb8010000005b8be55dc2080033c03987800000007e1e8b8f8c0000003971047505833900751983c00183c1103b87800000007ce85f5e33c05b8be55dc208008b978c0000008bc8c1e10403ca8b97800000002bd083ea01c1e204528d41105051ff15e810cc20838780000000ff5f5eb8010000005b8be55dc208008d77608d4508e8140c000085c074aa8b0685c07509c74740ffffffffeb0c3947407c07c74740000000008d4d0c8bfbe80b0d00008b5d08e8e30700005f5eb8010000005b8be55dc20800",[[38,20,"?FindSchedulerEntry@@YA_NPBUSchedulerEntryIndex@@PBQAXPAPAUSchedulerEntryIndexRecord@@@Z",550256720],[77,20,"_QueueSchedulerRegistration@20",550253712],[184,6,"__imp__MovePendingStorage@12",550244584],[216,20,"?RemoveSchedulerEntryReferences@@YAHPAUSchedulerContext@@PBQAUSchedulerTimedEntry@@@Z",550255472],[257,20,"?RemoveSchedulerIndexRecord@@YAHPAUSchedulerEntryIndex@@PBQAX@Z",550255760],[265,20,"_ReleaseSchedulerEntry",550254448]]],
        [550255472,"?RemoveSchedulerEntryReferences@@YAHPAUSchedulerContext@@PBQAUSchedulerTimedEntry@@@Z","8bff558bec5153578b3e33db83ef0178358b08894dfc8b560c8b4dfc390cba8d04ba751d8b162bd78d0c95fcffffff518d50045250ff15e810cc208306ff83c30183ef0179d05f8bc35b8be55dc3",[[55,6,"__imp__MovePendingStorage@12",550244584]]],
        [550255760,"?RemoveSchedulerIndexRecord@@YAHPAUSchedulerEntryIndex@@PBQAX@Z","8bff558bec51568b3785f674408d45fc50518bc7e8a703000084c074308b4f0c8b45fc2bc1c1f8032bf08d0cc18d14f5f8ffffff528d41085051ff15e810cc208307ffb8010000005e8be55dc333c05e8be55dc3",[[21,20,"?FindSchedulerEntry@@YA_NPBUSchedulerEntryIndex@@PBQAXPAPAUSchedulerEntryIndexRecord@@@Z",550256720],[60,6,"__imp__MovePendingStorage@12",550244584]]],
    ];
    const imports = readPE32Imports(pe);
    assert.deepEqual(imports.get(0x20cc10e8), { dll: 'api.dll', name: '#110' });
    if (request.Entries.includes('20cc35d0')) {
        targets.push(
            [0x20cc35d0, '_AcquireSchedulerEntry',
                '8bff558bec83ec1453568b35b483cc20578975fcc745f800000000eb038d49008b7e0c85ff0f84f80000008b45f86a006aff6a006a006a0050e8a21e000085c00f85dd0000008b760833db85f67424908bbe0408000085ff740fe8b11c00008bd885db0f85c30000008bb60808000085f675dd8b0d6c83cc20518d55f452680c0800006a01ff15dc10cc208b75f433ff3bf70f84d600000056c786000800008000000089be0408000089be08080000e87c07000085c00f84b2000000397df88b5dfc75778b7b0c85ffc745f80100000074246a006aff6a006a006a006a03e8fd1d000085c074548b430c85c074086a0050e8fa2a00008b860408000085c0740fe8db170000c78604080000000000008d45f0508975f0ff15d810cc208bf3e9fdfeffff33c05f5e5b8be55dc381c60008000056ff152c10cc20eb338b4b08898e08080000834310018b43103b431489730876038943148bbe04080000e8af1b00008bd885db7407838600080000ff8b55fc8b420c85c0740f33c9394df80f95c15150e8692a00005f5e8bc35b8be55dc3', [
                    [12, 6, '_schedulerEntryPool', 0x20cc83b4],
                    [58, 0x14, '_AcquireSchedulerPoolLock@28', 0x20cc54b0],
                    [91, 0x14, '_PopSchedulerPoolEntry', 0x20cc52e0],
                    [117, 6, '_schedulerPendingAllocator', 0x20cc836c],
                    [135, 6, '__imp__AllocateSchedulerPoolStorage@16', 0x20cc10dc],
                    [176, 0x14, '_InitializeSchedulerPoolBlock@4', 0x20cc3e00],
                    [223, 0x14, '_AcquireSchedulerPoolLock@28', 0x20cc54b0],
                    [242, 0x14, '_ReleaseSchedulerPoolLock@8', 0x20cc61c0],
                    [257, 0x14, '_DestroySchedulerPoolFreeList', 0x20cc4eb0],
                    [280, 6, '__imp__ReleaseSchedulerPoolStorage@4', 0x20cc10d8],
                    [309, 6, '__imp__DecrementSchedulerPoolCount@4', 0x20cc102c],
                    [349, 0x14, '_PopSchedulerPoolEntry', 0x20cc52e0],
                    [387, 0x14, '_ReleaseSchedulerPoolLock@8', 0x20cc61c0],
                ]],
            [0x20cc3770, '_ReleaseSchedulerEntry',
                '8bff558bec83ec08568b35b483cc20578b7e0c85ff8975fc746d6a006aff6a006a006a006a00e8151d000085c075588b760885f6743f3bde720a8d86000800003bd8720c8bb60808000085f675e8eb2585f674218bbe0408000085ff741753e83c19000085c0740d81c60008000056ff153010cc208b4dfc8b410c85c074086a0050e8c92900005f5e8be55dc3', [
                    [11, 6, '_schedulerEntryPool', 0x20cc83b4],
                    [39, 0x14, '_AcquireSchedulerPoolLock@28', 0x20cc54b0],
                    [96, 0x14, '_PushSchedulerPoolEntry@8', 0x20cc5110],
                    [113, 6, '__imp__IncrementSchedulerPoolCount@4', 0x20cc1030],
                    [131, 0x14, '_ReleaseSchedulerPoolLock@8', 0x20cc61c0],
                ]],
            [0x20cc3e00, '_InitializeSchedulerPoolBlock@4',
                '8bff558bec578b7d0883bf0408000000752e6880000000b880000000e86f0f000085c08987040800007527ff152810cc2033c05f5dc20400eb068d9b000000008bbf04080000e89514000085c08b7d0875ee535633f68bdfeb098d9b000000008b7d088bbf0408000053e8a112000085c0742783c60183c31081fe8000000072df8b45085e5bc7800008000080000000b8010000005f5dc204005e5b33c05f5dc20400', [
                    [29, 0x14, '_CreateSchedulerPoolFreeList@8', 0x20cc4d90],
                    [45, 6, '__imp__BreakOnSchedulerPoolFailure@0', 0x20cc1028],
                    [71, 0x14, '_PopSchedulerPoolEntry', 0x20cc52e0],
                    [107, 0x14, '_PushSchedulerPoolEntry@8', 0x20cc5110],
                ]],
        );
        for (const [address, dll, name] of [
            [0x20cc10dc, 'api.dll', '#106'], [0x20cc10d8, 'api.dll', '#108'],
            [0x20cc1028, 'kernel32.dll', 'DebugBreak'],
            [0x20cc102c, 'kernel32.dll', 'InterlockedDecrement'],
            [0x20cc1030, 'kernel32.dll', 'InterlockedIncrement'],
        ]) {
            assert.deepEqual(imports.get(address), { dll, name });
        }
    }
    for (const [address, target] of [
        [0x20cc27d0, 0x20cc1200], [0x20cc2971, 0x20cc1200],
        [0x20cc1238, 0x20cc2ce0], [0x20cc123c, 0x20cc2e80],
    ]) {
        assert.equal(readVA(address, 4).readUInt32LE(), target);
        assert(fixups.some(fixup => fixup.va === address && fixup.type === 3));
    }
    if (request.Entries.includes('20cc3bd0')) {
        const memcpyThunk = readVA(0x20cc6e86, 6);
        assert.equal(memcpyThunk.readUInt16LE(), 0x25ff);
        assert.deepEqual(imports.get(memcpyThunk.readUInt32LE(2)), { dll: 'msvcr80.dll', name: 'memcpy' });
        targets.push(
            [0x20cc3bd0, '_AppendSchedulerEntry@8',
                '8bff558bec578bf88b0783c001790433c0eb0c3dffffff1f7e05b8ffffff1f3b47047e0985c07c05e8530300008b078b4f0c8d048185c074078b55088b0a89088307015f5dc20400', [
                    [41, 0x14, '_ResizeLiveSchedulerEntries', 0x20cc3f50],
                ]],
            [0x20cc3b00, '_InsertSchedulerEntry@12',
                '8bff558bec56578bf88b0783c001790433c0eb0c3dffffff1f7e05b8ffffff1f3b47047e0985c07c05e8220400008b0f8b470c2bcb03c903c98d349d0000000003c6515083c00450ff15e810cc208b470c83070103c674078b55088b0a89088b470c5f03c65e5dc20400', [
                    [42, 0x14, '_ResizeLiveSchedulerEntries', 0x20cc3f50],
                    [74, 6, '__imp__MovePendingStorage@12', 0x20cc10e8],
                ]],
            [0x20cc3c20, '_InsertSchedulerEntryIndex@12',
                '8bff558bec51833e00c745fc0000000075098bc6e887060000eb2c8d45fc50578bc6e80904000084c0740833c08be55dc204008b460c538b5dfc2bd8c1fb038bc6e8ba0600005b8b0f8b550889088b0a894804b8010000008be55dc20400', [
                    [21, 0x14, '_AppendSchedulerIndexSlot', 0x20cc42c0],
                    [35, 0x14, '?FindSchedulerEntry@@YA_NPBUSchedulerEntryIndex@@PBQAXPAPAUSchedulerEntryIndexRecord@@@Z', 0x20cc4050],
                    [66, 0x14, '_InsertSchedulerIndexSlot', 0x20cc4320],
                ]],
            [0x20cc3f50, '_ResizeLiveSchedulerEntries',
                '8bff558bec83ec08538bd881fbffffff1f7e0733c05b8be55dc35633f63bde8975fc7e36663977087f0666c747080500f6470a0474120fbf77083bde7e1f8bff03f63bde7ffaeb150fbf4f088d4419ff99f7f98bf00faff1eb038975fc3b7704750b5eb8010000005b8be55dc385f67e24a16c83cc20508d4df8518d14b500000000526a01ff15dc10cc208b45f885c08945fc74138b073bd87c028bd885db7e248b45fc85c075085e33c05b8be55dc38b570c8d0c9d00000000515250e8742e000083c40c8b470c8d4df851891f8945f8ff15d810cc208b55fc897704b8010000006609470a5e89570c5b8be55dc3', [
                    [114, 6, '_schedulerPendingAllocator', 0x20cc836c],
                    [135, 6, '__imp__AllocateSchedulerContainerStorage@16', 0x20cc10dc],
                    [190, 0x14, '_memcpy', 0x20cc6e86],
                    [211, 6, '__imp__ReleaseSchedulerContainerStorage@4', 0x20cc10d8],
                ]],
            [0x20cc40d0, '_ResizeSchedulerEntryIndex',
                '8bff558bec83ec08538bd881fbffffff0f7e0733c05b8be55dc35633f63bde8975fc7e36663977087f0666c747080500f6470a0474120fbf77083bde7e1f8bff03f63bde7ffaeb150fbf4f088d4419ff99f7f98bf00faff1eb038975fc3b7704750b5eb8010000005b8be55dc385f67e24a16c83cc20508d4df8518d14f500000000526a01ff15dc10cc208b45f885c08945fc74138b073bd87c028bd885db7e248b45fc85c075085e33c05b8be55dc38b570c8d0cdd00000000515250e8f42c000083c40c8b470c8d4df851891f8945f8ff15d810cc208b55fc897704b8010000006609470a5e89570c5b8be55dc3', [
                    [114, 6, '_schedulerPendingAllocator', 0x20cc836c],
                    [135, 6, '__imp__AllocateSchedulerContainerStorage@16', 0x20cc10dc],
                    [190, 0x14, '_memcpy', 0x20cc6e86],
                    [211, 6, '__imp__ReleaseSchedulerContainerStorage@4', 0x20cc10d8],
                ]],
            [0x20cc42c0, '_AppendSchedulerIndexSlot',
                '8bff558bec5156578bf88b3783c601790433f6eb0d81feffffff0f7e05beffffff0f85f68bc67d0433c0eb0d81feffffff0f7e05b8ffffff0f3b47047e0985c07c05e8c9fdffff8b4f0c89375f8d44f1f85e8be55dc3', [
                    [67, 0x14, '_ResizeSchedulerEntryIndex', 0x20cc40d0],
                ]],
            [0x20cc4320, '_InsertSchedulerIndexSlot',
                '8bff56578bf88b0783c001790433c0eb0c3dffffff0f7e05b8ffffff0f3b47047e0985c07c05e885fdffff8b0f8b470c2bcb03c903c903c98d34dd0000000003c6515083c00850ff15e810cc208b470c8307015f03c65ec3', [
                    [39, 0x14, '_ResizeSchedulerEntryIndex', 0x20cc40d0],
                    [73, 6, '__imp__MovePendingStorage@12', 0x20cc10e8],
                ]],
        );
    }
    if (request.Entries.includes('20cc1670')) {
        const hasFactoryGuards = coff.byName.has('??1PublishedSchedulerContextReference@@QAE@XZ');
        assert.deepEqual(imports.get(0x20cc10d4), { dll: 'api.dll', name: '#159' });
        assert.deepEqual(imports.get(0x20cc10f4), { dll: 'api.dll', name: '#157' });
        targets.push(
            [0x20cc1670, '_CreateNamedSchedulerContext@12',
                '8bff558bec6aff68f971cc2064a1000000005083ec28535657a10880cc2033c5508d45f464a300000000c745ec000000008b751056ff15d410cc2083f820721d8b450cc700000000008b4df464890d00000000595f5e5b8be55dc20c006a20568d45cc50ff15f410cc208b7d088b0f8b49048d55cc528d45105057ffd18338008b45100f95c385c074088b10508b4204ffd084db74268b750c8b0f8b41048d55cc525657ffd08bc68b4df464890d00000000595f5e5b8be55dc20c00e8df07000089451085c0bb01000000895dfc740e8d4dcc5150e8561000008bf0eb0233f685f6c645fc0074078b168b0256ffd08975108d4d10518d4f308d45ccc745fc02000000e8680b000085f68b7d0cc7070000000074158b168b0256ffd08b0785c074088b088b510450ffd285f68937895decc645fc0074088b068b480456ffd18bc78b4df464890d00000000595f5e5b8be55dc20c00', [
                    [8, 6, '__ehhandler$_CreateNamedSchedulerContext@12', 0x20cc71f9],
                    [26, 6, '___security_cookie', 0x20cc8008],
                    [55, 6, '__imp__GetSchedulerContextNameLength@4', 0x20cc10d4],
                    [102, 6, '__imp__CopySchedulerContextName@12', 0x20cc10f4],
                    [189, 0x14, '_AcquireSchedulerContextStorage', 0x20cc1f10],
                    [214, 0x14, '_ConstructSchedulerContext@8', 0x20cc27a0],
                    [260, 0x14, '_InsertNamedSchedulerContext@12', 0x20cc22e0],
                ], hasFactoryGuards ? [['@__security_check_cookie@4', 20]]
                    : [['@__security_check_cookie@4', 20], ['_ReleaseSchedulerContextStorage', 20],
                        ['__CxxThrowException@8', 20], ['__CxxThrowException@8', 20]]],
            [0x20cc1f10, '_AcquireSchedulerContextStorage',
                '8bff558bec83ec1453568b35b083cc20578975fcc745f800000000eb038d49008b7e0c85ff0f84f80000008b45f86a006aff6a006a006a0050e86235000085c00f85dd0000008b760833db85f67424908bbe0409000085ff740fe8713300008bd885db0f85c30000008bb60809000085f675dd8b0d6c83cc20518d55f452680c0900006a01ff15dc10cc208b75f433ff3bf70f84d600000056c786000900001000000089be0409000089be08090000e87c02000085c00f84b2000000397df88b5dfc75778b7b0c85ffc745f80100000074246a006aff6a006a006a006a03e8bd34000085c074548b430c85c074086a0050e8ba4100008b860409000085c0740fe89b2e0000c78604090000000000008d45f0508975f0ff15d810cc208bf3e9fdfeffff33c05f5e5b8be55dc381c60009000056ff152c10cc20eb338b4b08898e08090000834310018b43103b431489730876038943148bbe04090000e86f3200008bd885db7407838600090000ff8b55fc8b420c85c0740f33c9394df80f95c15150e8294100005f5e8bc35b8be55dc3', [
                    [12, 6, '_schedulerContextPool', 0x20cc83b0],
                    [58, 0x14, '_AcquireSchedulerPoolLock@28', 0x20cc54b0],
                    [91, 0x14, '_PopSchedulerPoolEntry', 0x20cc52e0],
                    [117, 6, '_schedulerPendingAllocator', 0x20cc836c],
                    [135, 6, '__imp__AllocateSchedulerPoolStorage@16', 0x20cc10dc],
                    [176, 0x14, '_InitializeSchedulerContextBlock@4', 0x20cc2240],
                    [223, 0x14, '_AcquireSchedulerPoolLock@28', 0x20cc54b0],
                    [242, 0x14, '_ReleaseSchedulerPoolLock@8', 0x20cc61c0],
                    [257, 0x14, '_DestroySchedulerPoolFreeList', 0x20cc4eb0],
                    [280, 6, '__imp__ReleaseSchedulerPoolStorage@4', 0x20cc10d8],
                    [309, 6, '__imp__DecrementSchedulerPoolCount@4', 0x20cc102c],
                    [349, 0x14, '_PopSchedulerPoolEntry', 0x20cc52e0],
                    [387, 0x14, '_ReleaseSchedulerPoolLock@8', 0x20cc61c0],
                ]],
            [0x20cc20b0, '_ReleaseSchedulerContextStorage',
                '8bff558bec83ec08568b35b083cc20578b7e0c85ff8975fc746d6a006aff6a006a006a006a00e8d533000085c075588b760885f6743f3bde720a8d86000900003bd8720c8bb60809000085f675e8eb2585f674218bbe0409000085ff741753e8fc2f000085c0740d81c60009000056ff153010cc208b4dfc8b410c85c074086a0050e8894000005f5e8be55dc3', [
                    [11, 6, '_schedulerContextPool', 0x20cc83b0],
                    [39, 0x14, '_AcquireSchedulerPoolLock@28', 0x20cc54b0],
                    [96, 0x14, '_PushSchedulerPoolEntry@8', 0x20cc5110],
                    [113, 6, '__imp__IncrementSchedulerPoolCount@4', 0x20cc1030],
                    [131, 0x14, '_ReleaseSchedulerPoolLock@8', 0x20cc61c0],
                ]],
            [0x20cc2240, '_InitializeSchedulerContextBlock@4',
                '8bff558bec578b7d0883bf040900000075236a10b810000000e8322b000085c0898704090000751fff152810cc2033c05f5dc204008bbf04090000e86030000085c08b7d0875ee535633f68bdfeb04908b7d088bbf0409000053e8712e000085c0742783c60181c39000000083fe1072df8b45085e5bc7800009000010000000b8010000005f5dc204005e5b33c05f5dc20400', [
                    [26, 0x14, '_CreateSchedulerPoolFreeList@8', 0x20cc4d90],
                    [42, 6, '__imp__BreakOnSchedulerPoolFailure@0', 0x20cc1028],
                    [60, 0x14, '_PopSchedulerPoolEntry', 0x20cc52e0],
                    [91, 0x14, '_PushSchedulerPoolEntry@8', 0x20cc5110],
                ]],
        );
        const handler = coff.unique('__ehhandler$_CreateNamedSchedulerContext@12');
        assert.equal(coff.sections[handler.section - 1].name, '.text$x');
        assert(coff.sections[handler.section - 1].relocations.some(
            rel => rel.symbol?.name === '__ehfuncinfo$_CreateNamedSchedulerContext@12'));
    }
    const liveRemoval = '?RemoveSchedulerEntryReferences@@YAHPAUSchedulerLiveEntryList@@PBQAUSchedulerTimedEntry@@@Z';
    const resolveSymbol = (name, address) => address === 0x20cc3b70 && coff.byName.has(liveRemoval)
        ? liveRemoval : name;
    for (const [entry, legacySymbol, hex, expected, additional = []] of targets) {
        const symbol = resolveSymbol(legacySymbol, entry);
        const original = readVA(entry, hex.length / 2);
        assert.equal(original.toString('hex'), hex);
        const fn = coff.unique(symbol);
        const code = coff.sections[fn.section - 1];
        assert.equal(fn.value, 0);
        assert.equal(code.name, '.text');
        assert(code.flags & 0x1000, 'Expected a function COMDAT');
        const rebuilt = object.subarray(code.raw, code.raw + code.size);
        assert.equal(code.relocations.length, expected.length + additional.length);
        assert.deepEqual(fixups.filter(({ va }) => va >= entry && va < entry + original.length),
            expected.filter(row => row[1] === 6).map(([offset]) => ({ va: entry + offset, type: 3 })));
        const remaining = code.relocations.slice();
        const references = expected.map(([offset, type, legacyName, target]) => {
            const name = resolveSymbol(legacyName, target);
            const originalTarget = type === 6 ? original.readUInt32LE(offset)
                : entry + offset + 4 + original.readInt32LE(offset);
            assert.equal(originalTarget, target);
            const index = remaining.findIndex(rel => rel.type === type && rel.symbol.name === name);
            assert(index >= 0, name);
            const [relocation] = remaining.splice(index, 1);
            assert.equal(rebuilt.readUInt32LE(relocation.offset), 0);
            return { originalOffset: offset, rebuiltOffset: relocation.offset, symbol: name, targetVA: target.toString(16) };
        });
        const rebuiltOnly = additional.map(([symbol, type]) => {
            const index = remaining.findIndex(rel => rel.type === type && rel.symbol.name === symbol);
            assert(index >= 0, symbol);
            const [relocation] = remaining.splice(index, 1);
            assert.equal(rebuilt.readUInt32LE(relocation.offset), 0);
            return { rebuiltOffset: relocation.offset, symbol };
        });
        const report = {
            program: 'simscheduler.dll', entry: entry.toString(16), status: 'Diagnostic',
            scope: 'Complete native C++ COFF body with checked references. Internal argument ABIs and code generation differ.',
            originalSHA256, objectSHA256: sha(object),
            originalHex: hex, rebuiltHex: rebuilt.toString('hex'), references, rebuiltOnly,
        };
        fs.writeFileSync(path.join(run, 'comparison-' + report.entry + '.json'), JSON.stringify(report, null, 2) + '\n');
        console.log('Diagnostic ' + report.entry + ' (' + original.length + ' original bytes, ' + rebuilt.length + ' rebuilt bytes)');
    }
}
if (request.Entries.includes('20cc1b90')) {
    const entry = 0x20cc1b90;
    const original = readVA(entry, 284);
    assert.equal(original.toString('hex'), '8bff558bec83e4c083ec3453568b75088b068b481c5756ffd18944242cdb44242ca1ec10cc2033d233dbdc4d0c399810010000895c24280f95c2395e30dc0dd812cc20dd542438dc46048954242cdd5c24307e748b4e3c8b440b208b10508b424cffd0dd4424308b54242c83ec088bf88bc4dd188b4e2451528bc7e8c014000085c07431393d6083cc207529833d6483cc200074208b078b482857ffd185c07414a16483cc20d9ee8b1083ec08dd1c24508b02ffd08b44242883c00183c3243b4630894424287c8c837c242c00751e8b4c24308b5424348b442438894e048b4c243c89560889460c894e10eb14d9eedd5c24388b5424388b44243c89560c894610dd450c6a006a00dd5e146a22ff15f810cc205f5e5b8be55dc20c00');
    const fn = coff.unique('_AdvanceSchedulerTime@12');
    const code = coff.sections[fn.section - 1];
    assert.equal(fn.value, 0);
    assert.equal(code.name, '.text');
    assert(code.flags & 0x1000, 'Expected function COMDAT');
    const rebuilt = object.subarray(code.raw, code.raw + code.size);
    assert.equal(rebuilt.subarray(-3).toString('hex'), 'c20c00');
    const expected = [
        [34, 6, '__imp__schedulerApiState', 0x20cc10ec],
        [63, 6, '__real@3f70000000000000', 0x20cc12d8],
        [124, 0x14, '_ProcessSchedulerContext@20', 0x20cc30d0],
        [134, 6, '_currentSchedulerContext', 0x20cc8360],
        [142, 6, '_simConnectFallbackSetting', 0x20cc8364],
        [162, 6, '_simConnectFallbackSetting', 0x20cc8364],
        [271, 6, '__imp__DispatchSchedulerApiEvent@12', 0x20cc10f8],
    ];
    assert.deepEqual(fixups.filter(({ va }) => va >= entry && va < entry + original.length),
        expected.filter(row => row[1] === 6).map(row => ({ va: entry + row[0], type: 3 })));
    assert.equal(code.relocations.length, expected.length);
    const originalImports = readPE32Imports(pe);
    assert.deepEqual(originalImports.get(0x20cc10ec), { dll: 'api.dll', name: '#304' });
    assert.deepEqual(originalImports.get(0x20cc10f8), { dll: 'api.dll', name: '#233' });
    assert.equal(readVA(0x20cc12d8, 8).readDoubleLE(), 1 / 256);
    const constant = coff.unique('__real@3f70000000000000');
    const data = coff.sections[constant.section - 1];
    assert.equal(data.name, '.rdata');
    assert(constant.value + 8 <= data.size);
    assert.equal(object.readDoubleLE(data.raw + constant.value), 1 / 256);
    const references = expected.map(([offset, type, symbol, target], index) => {
        const originalTarget = type === 6 ? original.readUInt32LE(offset)
            : entry + offset + 4 + original.readInt32LE(offset);
        assert.equal(originalTarget, target);
        const relocation = code.relocations[index];
        assert.equal(relocation.type, type);
        assert.equal(relocation.symbol.name, symbol);
        assert.equal(rebuilt.readUInt32LE(relocation.offset), 0);
        if (symbol === '_ProcessSchedulerContext@20' && relocation.symbol.section > 0) {
            assert.equal(coff.sections[relocation.symbol.section - 1].name, '.text');
            assert.equal(relocation.symbol.value, 0);
        } else if (symbol !== constant.name) {
            assert.equal(relocation.symbol.section, 0);
        }
        return { originalOffset: offset, rebuiltOffset: relocation.offset, symbol,
            targetVA: target.toString(16), originalAbiCompatible: type !== 0x14 };
    });
    const report = {
        program: 'simscheduler.dll', entry: entry.toString(16), status: 'Diagnostic',
        scope: 'Complete C++ COFF body with checked references. Original context input travels in EAX, rebuilt context input travels on the stack.',
        originalSHA256, objectSHA256: sha(object),
        originalHex: original.toString('hex'), rebuiltHex: rebuilt.toString('hex'), references,
    };
    fs.writeFileSync(path.join(run, 'comparison-20cc1b90.json'), JSON.stringify(report, null, 2) + '\n');
    console.log('Diagnostic 20cc1b90 (' + original.length + ' original bytes, ' + rebuilt.length + ' rebuilt bytes)');
}
if (request.Entries.includes('20cc1400') && request.DispatcherProfile !== 'Cpp') {
    assert.equal(readVA(0x20cc1b38, 5).toString('hex'), '85f6538bd8');
    assert.equal(readVA(0x20cc4971, 3).toString('hex'), 'c20400');
    assert.equal(readVA(0x20cc48c3, 1)[0], 0xc3);
    verify({
        entry: 0x20cc1400, hex: '8bff558bec83e4f8518b450883e82556743783e80174222d4b010000753333c039450c0f95c08bf0b87083cc20e8fe0600005e8be55dc208008b4d0c51e88e3400005e8be55dc208008b750ce83f3300005e8be55dc20800',
        functionSymbol: '_SchedulerGroup10Callback@8', file: 'comparison-20cc1400.json',
        references: [
            { offset: 41, type: 6, target: 0x20cc8370, symbol: '_schedulerObject' },
            { offset: 46, type: 0x14, target: 0x20cc1b30, symbol: '_UpdateSchedulerTimeState' },
            { offset: 62, type: 0x14, target: 0x20cc48d0, symbol: '_SaveSchedulerFlightState@4' },
            { offset: 77, type: 0x14, target: 0x20cc4790, symbol: '_LoadSchedulerFlightState' },
        ],
    });
}
if (request.Entries.includes('20cc1400') && request.DispatcherProfile === 'Cpp') {
    const entry = 0x20cc1400;
    const original = readVA(entry, 88);
    const callback = coff.unique('_SchedulerGroup10Callback@8');
    const code = coff.sections[callback.section - 1];
    assert.equal(callback.value, 0);
    assert.equal(code.name, '.text');
    assert(code.flags & 0x1000, 'Expected a function COMDAT');
    const rebuilt = object.subarray(code.raw, code.raw + code.size);
    const expected = [
        [41, 6, '_schedulerObject', 0x20cc8370, true],
        [46, 0x14, '_UpdateSchedulerTimeState', 0x20cc1b30, false],
        [62, 0x14, '_SaveSchedulerFlightState@4', 0x20cc48d0, true],
        [77, 0x14, '_LoadSchedulerFlightState', 0x20cc4790, false],
    ];
    assert.equal(code.relocations.length, expected.length);
    const references = expected.map(([offset, type, symbol, target, abiCompatible]) => {
        const originalTarget = type === 6 ? original.readUInt32LE(offset)
            : entry + offset + 4 + original.readInt32LE(offset);
        assert.equal(originalTarget, target);
        const relocation = code.relocations.find(ref => ref.symbol?.name === symbol);
        assert.equal(relocation?.type, type, symbol);
        assert.equal(relocation.symbol.section, 0);
        assert.equal(rebuilt.readUInt32LE(relocation.offset), 0);
        return { originalOffset: offset, rebuiltOffset: relocation.offset, symbol,
            targetVA: target.toString(16), abiCompatible };
    });
    const report = {
        program: 'simscheduler.dll', entry: entry.toString(16), status: 'Diagnostic',
        scope: 'Complete C++ COFF body with symbol mappings. The load and time-state handlers use different argument ABIs.',
        originalSHA256, objectSHA256: sha(object),
        originalHex: original.toString('hex'), rebuiltHex: rebuilt.toString('hex'), references,
    };
    fs.writeFileSync(path.join(run, 'comparison-20cc1400.json'), JSON.stringify(report, null, 2) + '\n');
    console.log('Diagnostic ' + report.entry + ' (' + original.length + ' original bytes, ' + rebuilt.length + ' rebuilt bytes)');
}
