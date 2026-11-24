const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');

const run = process.argv[2];
assert(run, 'Pass a GPS pilot run directory');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const pe = fs.readFileSync(path.join(__dirname, '../original/gps.dll'));
const originalSHA256 = sha(pe);
assert.equal(originalSHA256, 'e36be481bf8a6a8a0a56ea72d57961b169050f70de401766c17fb374ee74e609');
const header = pe.readUInt32LE(0x3c);
const optional = header + 24;
const base = pe.readUInt32LE(optional + 28);
const sectionTable = optional + pe.readUInt16LE(header + 20);

function readVA(va, size) {
    const rva = va - base;
    for (let i = 0; i < pe.readUInt16LE(header + 6); i++) {
        const section = sectionTable + i * 40;
        const delta = rva - pe.readUInt32LE(section + 12);
        if (delta < 0 || delta + size > pe.readUInt32LE(section + 16)) continue;
        const raw = pe.readUInt32LE(section + 20) + delta;
        assert(raw + size <= pe.length);
        return pe.subarray(raw, raw + size);
    }
    throw Error('Unmapped GPS VA ' + va.toString(16));
}

const entry = 0x20855fb0;
const exportsVA = base + pe.readUInt32LE(optional + 96);
const exportDirectory = readVA(exportsVA, 40);
assert.equal(exportDirectory.readUInt32LE(16), 1, 'Export ordinal base');
assert.equal(exportDirectory.readUInt32LE(20), 2, 'Export address count');
const eat = readVA(base + exportDirectory.readUInt32LE(28), 8);
assert.equal(base + eat.readUInt32LE(0), 0x20855f20);
assert.equal(base + eat.readUInt32LE(4), entry);

const original = readVA(entry, 42);
const normalized = Buffer.from(original);
const object = fs.readFileSync(path.join(run, 'gps-registration.obj'));
const coff = parseNativeCoff(object);
const fn = coff.unique('_UnregisterGpsProvider');
const code = coff.sections[fn.section - 1];
assert.equal(code.name, '.text');
assert.equal(fn.value, 0);
const rebuilt = object.subarray(code.raw, code.raw + code.size);
const imports = readPE32Imports(pe);
const lifecycleName = coff.byName.has('_GpsLegacyDataLifecycle@8')
    ? '_GpsLegacyDataLifecycle@8' : '_GpsLegacyDataLifecycle';
const updateName = lifecycleName.endsWith('@8') ? '_GpsLegacyDataUpdate@8' : '_GpsLegacyDataUpdate';
const targets = [
    { offset: 0x06, originalVA: 0x20851994, symbol: '??_C@_06JIIKIAMP@FS9GPS?$AA@' },
    { offset: 0x0c, originalVA: 0x2085117c, symbol: '__imp__PanelsOrdinal72@8', import: { dll: 'panels.dll', name: '#72' } },
    { offset: 0x12, originalVA: 0x20851118, symbol: '__imp__ApiOrdinal232@8', import: { dll: 'api.dll', name: '#232' } },
    { offset: 0x17, originalVA: 0x2086a520, symbol: updateName, registerOperand: 0x20855f91 },
    { offset: 0x20, originalVA: 0x2086a380, symbol: lifecycleName, registerOperand: 0x20855f88 },
];

function isLocalLifecycle(symbol) {
    if (symbol.name !== '_GpsLegacyDataLifecycle@8') return false;
    assert.equal(coff.sections[symbol.section - 1]?.name, '.text');
    assert.equal(symbol.value, 0);
    return true;
}

assert.equal(rebuilt.length, original.length, 'Complete body size');
assert.equal(code.relocations.length, targets.length, 'Complete relocation count');
for (const target of targets) {
    const { offset, originalVA, symbol, registerOperand } = target;
    const relocation = code.relocations.find(item => item.offset === offset);
    assert.equal(relocation?.type, 6, 'DIR32 relocation at ' + offset);
    assert.equal(relocation.symbol.name, symbol);
    assert.equal(original.readUInt32LE(offset), originalVA);
    assert.equal(rebuilt.readUInt32LE(offset), 0, 'Relocation addend');
    if (target.import) assert.deepEqual(imports.get(originalVA), target.import);
    if (registerOperand) assert.equal(readVA(registerOperand, 4).readUInt32LE(), originalVA);
    if (target.import || registerOperand) {
        if (!isLocalLifecycle(relocation.symbol)) assert.equal(relocation.symbol.section, 0, 'External symbol');
    } else {
        const literal = coff.sections[relocation.symbol.section - 1];
        assert.equal(literal.name, '.rdata');
        const value = Buffer.from('FS9GPS\0');
        assert.deepEqual(readVA(originalVA, value.length), value);
        const start = literal.raw + relocation.symbol.value;
        assert(relocation.symbol.value + value.length <= literal.size);
        assert.deepEqual(object.subarray(start, start + value.length), value);
    }
    normalized.writeUInt32LE(0, offset);
}
assert.deepEqual(rebuilt, normalized, 'Non-relocated body bytes');

const report = {
    program: 'gps.dll',
    entry: entry.toString(16),
    ordinal: 2,
    status: 'target-validated normalized',
    scope: 'Complete original body against native x86 COFF with five validated DIR32 operands',
    originalSHA256,
    objectSHA256: sha(object),
    originalHex: original.toString('hex'),
    rebuiltHex: rebuilt.toString('hex'),
    mappings: targets.map(item => ({ ...item, originalVA: item.originalVA.toString(16) })),
};
fs.writeFileSync(path.join(run, 'comparison.json'), JSON.stringify(report, null, 2) + '\n');
console.log('gps.dll ordinal 2: 42-byte normalized COFF match, five validated relocations');

// Older retained objects contain only ordinal 2.
if (coff.byName.has('_RegisterGpsProvider')) {
    const providerTable = 0x208519e4;
    const releaseEntry = 0x20856050;
    assert.equal(readVA(providerTable + 4, 4).readUInt32LE(), releaseEntry);

    function compareBody(va, size, name, fields) {
        const originalBody = readVA(va, size);
        const left = Buffer.from(originalBody);
        const fn = coff.unique(name);
        const code = coff.sections[fn.section - 1];
        assert.equal(code.name, '.text');
        assert.equal(fn.value, 0);
        const rebuiltBody = object.subarray(code.raw, code.raw + code.size);
        const right = Buffer.from(rebuiltBody);
        assert.equal(code.relocations.length, fields.length);
        const mappings = fields.map(([originalOffset, rebuiltOffset, address, symbol, relative, local, addend = 0]) => {
            const relocation = code.relocations.find(item => item.offset === rebuiltOffset);
            assert.equal(relocation?.type, relative ? 0x14 : 6);
            assert.equal(relocation.symbol.name, symbol);
            assert.equal(right.readUInt32LE(rebuiltOffset), addend);
            const resolved = relative
                ? va + originalOffset + 4 + left.readInt32LE(originalOffset)
                : left.readUInt32LE(originalOffset);
            assert.equal(resolved, address + addend);
            if (symbol.startsWith('??_C@')) {
                const literal = coff.sections[relocation.symbol.section - 1];
                assert.equal(literal.name, '.rdata');
                const start = literal.raw + relocation.symbol.value;
                const length = literal.size - relocation.symbol.value;
                assert(length > 0);
                assert.deepEqual(object.subarray(start, start + length), readVA(address, length));
                assert.equal(object[start + length - 1], 0);
            } else if (local || isLocalLifecycle(relocation.symbol) || (symbol === '_DestroyGpsProvider@4' && relocation.symbol.section > 0)) {
                assert.match(coff.sections[relocation.symbol.section - 1].name, /^\.text(?:\$\w+)?$/);
            } else {
                assert.equal(relocation.symbol.section, 0);
            }
            left.writeUInt32LE(0, originalOffset);
            right.writeUInt32LE(0, rebuiltOffset);
            return { originalOffset, rebuiltOffset, target: resolved.toString(16), symbol, ...(addend ? { addend } : {}) };
        });
        return { left, right, evidence: {
            entry: va.toString(16), symbol: name, status: 'diagnostic',
            originalHex: originalBody.toString('hex'), rebuiltHex: rebuiltBody.toString('hex'), mappings,
        } };
    }

    assert.deepEqual(imports.get(0x2085110c), { dll: 'api.dll', name: '#106' });
    assert.deepEqual(imports.get(0x2085111c), { dll: 'api.dll', name: '#231' });
    assert.deepEqual(imports.get(0x20851170), { dll: 'multiplayer.dll', name: '#4' });
    assert.deepEqual(imports.get(0x20851114), { dll: 'api.dll', name: '#108' });
    const init = compareBody(0x20855f20, 137, '_RegisterGpsProvider', [
        [0x07, 0x07, 0x2086eab0, '_GpsAllocationContext'],
        [0x17, 0x17, 0x2085110c, '__imp__ApiOrdinal106@16'],
        [0x26, 0x27, 0x208519b0, '_GpsProviderBaseTable'],
        [0x2c, 0x2d, providerTable, '_GpsProviderTableInstance'],
        [0x47, 0x32, 0x20851994, targets[0].symbol],
        [0x54, 0x54, 0x2085117c, '__imp__PanelsOrdinal72@8'],
        [0x63, 0x63, 0x2085111c, '__imp__ApiOrdinal231@8'],
        [0x68, 0x68, 0x2086a380, lifecycleName],
        [0x71, 0x71, 0x2086a520, updateName],
        [0x7b, 0x7b, 0x20851170, '__imp__MultiplayerOrdinal4@0'],
        [0x80, 0x80, 0x2086eaa8, '_GpsMultiplayerValue'],
    ]);
    // Move the original PUSH ESI and PUSH imm32 to the observed rebuilt positions.
    assert.equal(init.left[0x30], 0x56);
    assert.equal(init.left[0x46], 0x68);
    assert.deepEqual(init.right, Buffer.concat([
        init.left.subarray(0, 0x24), init.left.subarray(0x30, 0x31),
        init.left.subarray(0x24, 0x30), init.left.subarray(0x46, 0x4b),
        init.left.subarray(0x31, 0x46), init.left.subarray(0x4b),
    ]));
    init.evidence.difference = 'Two argument pushes moved across field stores';

    const release = compareBody(releaseEntry, 49, '?Release@GpsProvider@@QAEIXZ', [
        [0x18, 0x15, 0x208560e0, '_DestroyGpsProvider@4', true],
        [0x25, 0x22, 0x20851114, '__imp__ApiOrdinal108@4'],
    ]);
    assert.deepEqual(release.left.subarray(0, 0x11), release.right.subarray(0, 0x11));
    assert.equal(release.left.subarray(0x11, 0x16).toString('hex'), '83ff017313');
    assert.equal(release.right.subarray(0x11, 0x13).toString('hex'), '7513');
    assert.deepEqual(release.left.subarray(0x16), release.right.subarray(0x13));
    release.evidence.difference = 'Original CMP EDI,1 and JNC versus JNE using flags from the preceding decrement';

    const followup = { program: 'gps.dll', originalSHA256, objectSHA256: sha(object),
        rows: [init.evidence, release.evidence] };
    fs.writeFileSync(path.join(run, 'provider-comparison.json'), JSON.stringify(followup, null, 2) + '\n');
    console.log('GPS registration and Release: all 13 relocations checked; code-generation differences recorded');

    const handlerName = '__ehhandler$_DestroyGpsProvider@4';
    if (coff.byName.has(handlerName)) {
        const entriesName = '?ReleaseEntries@GpsProvider@@QAEXXZ';
        const entriesEntry = 0x208561b0;
        const importFree = '__imp__ApiOrdinal108@4';
        assert.equal(readVA(providerTable + 0x20, 4).readUInt32LE(), entriesEntry);
        const entries = compareBody(entriesEntry, 78, entriesName, [
            [0x14, 0x14, 0x20851114, importFree],
        ]);
        assert.deepEqual(entries.left, entries.right);
        entries.evidence.status = 'target-validated normalized';

        const destructor = compareBody(0x208560e0, 148, '_DestroyGpsProvider@4', [
            [0x08, 0x08, 0x2086b82b, handlerName, false, true],
            [0x15, 0x16, 0x2086d008, '___security_cookie'],
            [0x2a, 0x2e, providerTable, '_GpsProviderTableInstance'],
            [0x38, 0x3d, entriesEntry, entriesName, true, true],
            [0x6b, 0x74, 0x20851114, importFree],
        ]);
        assert.equal(destructor.right.length, 157);
        destructor.evidence.difference = 'RAII guard changes frame layout and storage base; count clamp uses SETG and AND';

        // Follow the original exception handler through its one-state unwind map.
        const functionInfo = readVA(0x2086b82b + 0x12, 4).readUInt32LE();
        assert.equal(functionInfo, 0x2086be00);
        const originalInfo = readVA(functionInfo, 12);
        assert.equal(originalInfo.readUInt32LE(0), 0x19930522);
        assert.equal(originalInfo.readUInt32LE(4), 1);
        const unwindMap = readVA(originalInfo.readUInt32LE(8), 8);
        assert.equal(unwindMap.readInt32LE(0), -1);
        assert.equal(unwindMap.readUInt32LE(4), 0x2086b820);
        assert.equal(readVA(0x2086b820, 7).toString('hex'), '8b750883c608e9');
        assert.equal(0x2086b82b + readVA(0x2086b827, 4).readInt32LE(), 0x20856500);

        function checkReference(name, offset, target, type) {
            const symbol = coff.unique(name);
            const section = coff.sections[symbol.section - 1];
            const relocation = section.relocations.find(item => item.offset === symbol.value + offset);
            assert.equal(relocation?.type, type);
            assert.equal(relocation.symbol.name, target);
            const position = section.raw + symbol.value;
            assert.equal(object.readUInt32LE(position + offset), 0);
            return position;
        }
        const functionInfoName = '__ehfuncinfo$_DestroyGpsProvider@4';
        const unwindMapName = '__unwindtable$_DestroyGpsProvider@4';
        const unwindName = '__unwindfunclet$_DestroyGpsProvider@4$0';
        checkReference(handlerName, 0x0d, '@__security_check_cookie@4', 0x14);
        checkReference(handlerName, 0x12, functionInfoName, 6);
        checkReference(handlerName, 0x17, '___CxxFrameHandler3', 0x14);
        const info = checkReference(functionInfoName, 8, unwindMapName, 6);
        assert.equal(object.readUInt32LE(info), 0x19930522);
        assert.equal(object.readUInt32LE(info + 4), 1);
        const unwind = checkReference(unwindMapName, 4, unwindName, 6);
        assert.equal(object.readInt32LE(unwind), -1);
        const guardName = '??1GpsStorageCleanup@@QAE@XZ';
        checkReference(unwindName, 4, guardName, 0x14);
        checkReference(guardName, 0x35, importFree, 6);

        const legacy = compareBody(0x2086a4c0, 86, '_ReleaseGpsLegacyStorageBody', [
            [0x0e, 0x12, 0x20851114, importFree],
        ]);
        const adapterName = '_ReleaseGpsLegacyStorage';
        const adapter = checkReference(adapterName, 2, legacy.evidence.symbol, 0x14);
        assert.equal(object.subarray(adapter, adapter + 12).toString('hex'), '56e80000000083c4048bc6c3');
        assert.equal(0x2086a3c1 + readVA(0x2086a3bd, 4).readInt32LE(), 0x2086a4c0);
        legacy.evidence.difference = 'Rebuilt C++ body takes a stack argument through a separate ESI-to-stack adapter';
        legacy.evidence.adapter = { symbol: adapterName, bytes: 12, input: 'ESI', result: 'EAX' };

        const cleanup = { program: 'gps.dll', originalSHA256, objectSHA256: sha(object),
            exceptionCleanup: { originalHandler: '2086b82b', originalUnwindTarget: '20856500',
                rebuiltHandler: handlerName, rebuiltCleanup: guardName, states: 1 },
            rows: [destructor.evidence, entries.evidence, legacy.evidence] };
        fs.writeFileSync(path.join(run, 'cleanup-comparison.json'), JSON.stringify(cleanup, null, 2) + '\n');
        console.log('GPS cleanup: 78-byte normalized entry-release match; destructor unwind and legacy ESI adapter checked');
    }

    if (lifecycleName.endsWith('@8')) {
        const dataName = '??_C@_0BA@GBLEFNCN@FS2002?5GPS?5DATA?$AA@';
        const requestName = '??_C@_0BI@KNHPOHPH@FS2002?5GPS?5DATA?5REQUEST?$AA@';
        const storage = 0x2086e9f0;
        const data = 0x2086e9f8;
        const request = 0x2086e940;
        const initializer = 0x2086aa10;
        const adapterName = '_InitializeGpsLegacyData';
        const initializerName = '@InitializeGpsLegacyDataBody@12';
        assert.deepEqual(imports.get(0x20851178), { dll: 'panels.dll', name: '#34' });
        assert.deepEqual(imports.get(0x20851180), { dll: 'panels.dll', name: '#38' });
        const requestOffsets = [
            [0xa6, 0x20], [0xad, 0x28], [0xb4, 0x30], [0xba, 0],
            [0xc4, 0x38], [0xca, 4], [0xd0, 0x80], [0xd6, 0x40],
            [0xdc, 0x90], [0xe2, 0x44], [0xe8, 0x98], [0xee, 0x48],
            [0xf4, 0xa0], [0xfa, 0x4c], [0x100, 0xa8], [0x106, 0xac],
        ];
        const lifecycle = compareBody(0x2086a380, 315, lifecycleName, [
            [0x1a, 0x105, 0x20851180, '__imp__PanelsOrdinal38@4'],
            [0x1f, 0x10a, 0x20855c78, requestName],
            [0x26, 0x111, 0x20855c68, dataName],
            [0x2e, 0x118, storage, '_GpsLegacyStorage'],
            [0x3d, 0x129, 0x2086a4c0, '_ReleaseGpsLegacyStorage', true, true],
            [0x43, 0x12f, storage, '_GpsLegacyStorage'],
            [0x4e, 0x14, storage, '_GpsLegacyStorage'],
            [0x59, 0x1f, 0x2086eab0, '_GpsAllocationContext'],
            [0x68, 0x2e, 0x2085110c, '__imp__ApiOrdinal106@16'],
            [0x90, 0x4e, storage, '_GpsLegacyStorage'],
            [0x99, 0x5c, data, '_GpsLegacyData'],
            [0x9e, 0x61, initializer, adapterName, true, true],
            ...requestOffsets.map(([offset, addend]) =>
                [offset, offset - 0x3d, request, '_GpsLegacyRequest', false, false, addend]),
            [0x10a, 0xcd, 0x2086e140, '_GpsLegacyRequestEntries'],
            [0x10f, 0xd2, 0x2086a520, updateName, true],
            [0x115, 0xd8, 0x20851178, '__imp__PanelsOrdinal34@12'],
            [0x11a, 0xdd, 0x20855c68, dataName],
            [0x121, 0xe4, data, '_GpsLegacyData'],
            [0x128, 0xeb, 0x20855c78, requestName],
            [0x12f, 0xf2, request, '_GpsLegacyRequest'],
        ]);
        assert.equal(lifecycle.right.length, 313);
        assert.deepEqual(lifecycle.left.subarray(0xa2, 0x135), lifecycle.right.subarray(0x65, 0xf8));
        lifecycle.evidence.matchedBlock = { originalOffset: 0xa2, rebuiltOffset: 0x65, bytes: 0x93 };
        lifecycle.evidence.difference = 'Event dispatch, allocation initialization, and ABI call setup differ; 147-byte request reset and publish block matches after target validation';

        const init = compareBody(initializer, 85, initializerName, []);
        const renamed = Buffer.from(init.left);
        // Only the ModR/M bytes which use EAX or ECX change. x87 opcode extensions stay fixed.
        for (const offset of [10, 16, 22, 28, 34, 43, 58, 64, 70]) renamed[offset] ^= 1;
        renamed[55] ^= 8;
        for (const offset of [8, 13, 19, 25, 31, 40, 49, 76]) renamed[offset] ^= 9;
        assert.deepEqual(init.right, renamed);
        const adapter = coff.unique(adapterName);
        const section = coff.sections[adapter.section - 1];
        assert.equal(adapter.value, 0);
        assert.equal(section.relocations.length, 1);
        const [jump] = section.relocations;
        assert.equal(jump.offset, 3);
        assert.equal(jump.type, 0x14);
        assert.equal(jump.symbol.name, initializerName);
        assert.equal(object.subarray(section.raw, section.raw + section.size).toString('hex'), '8bc8e900000000');
        init.evidence.difference = '85-byte body matches after EAX/ECX reassignment; separate 7-byte adapter preserves the EAX input';
        init.evidence.adapter = { symbol: adapterName, bytes: 7, input: 'EAX', poppedStackBytes: 4 };
        const legacy = { program: 'gps.dll', originalSHA256, objectSHA256: sha(object),
            rows: [lifecycle.evidence, init.evidence] };
        fs.writeFileSync(path.join(run, 'legacy-lifecycle-comparison.json'), JSON.stringify(legacy, null, 2) + '\n');
        console.log('GPS legacy lifecycle: 35 relocations checked; 147-byte block and initializer register mapping verified');
    }
}
