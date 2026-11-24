const assert = require('node:assert/strict');
const { createHash } = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { openImage } = require('./lib/pe32-dll.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');

const root = path.resolve(__dirname, '..');
const [run, profile = 'updates'] = process.argv.slice(2);
assert(run, 'Pass a retained API factory run');
assert(['updates', 'release', 'heap', 'adapters', 'mapping', 'leases', 'buffer', 'cleanup'].includes(profile));
const isRelease = profile === 'release';
const isHeap = profile === 'heap';
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const label = 'pointer-owner-' + profile;
const evidence = `artifacts/api.dll/${label}-20261124/ghidra-${label}.json`;
const live = JSON.parse(fs.readFileSync(path.join(root, evidence), 'utf8'));
assert.equal(live.program, 'api.dll');
const pe = fs.readFileSync(path.join(root, 'original/api.dll'));
const originalSHA256 = sha(pe);
assert.equal(originalSHA256, '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const image = openImage(pe);
const imports = readPE32Imports(pe);
const object = fs.readFileSync(path.join(run, 'api-factory.obj'));
const coff = parseNativeCoff(object);
const observed = new Map();
for (const fn of [...live.functions, ...(live.dependencies || [])]) {
    const start = parseInt(fn.entry_point, 16);
    let next = start;
    for (const [address, hex] of fn.asm) {
        assert.equal(parseInt(address, 16), next);
        next += Buffer.from(hex, 'hex').length;
    }
    assert.equal(next, start + fn.size);
    assert.equal(parseInt(fn.body_end, 16), next - 1);
    const peBody = image.codeAt(start, fn.size);
    assert.equal(peBody.toString('hex'), fn.asm.map(([, hex]) => hex).join(''));
    assert(!observed.has(start));
    observed.set(start, peBody);
}

const updates = [
    [0x20161815, '?TakeFrom@InterfacePointerOwnerPilot@@QAEPAV1@PAPAUIUnknown@@@Z',
        20, 0x201611be, '?Attach@InterfacePointerOwnerPilot@@AAEXPAUIUnknown@@@Z'],
    [0x20161834, '?ReplaceAndReturn@PrivateHeapPointerOwnerPilot@@QAEPAV1@PAX@Z',
        12, 0x201611d5, '?Replace@PrivateHeapPointerOwnerPilot@@AAEXPAX@Z'],
    [0x2016184b, '?Replace@PrefixedBufferOwnerPilot@@QAEXPAX@Z',
        9, 0x2015115f, '?Release@PrefixedBufferOwnerPilot@@AAEXXZ'],
    [0x20161862, '?ReplaceAndReturn@PrefixedBufferOwnerPilot@@QAEPAV1@PAX@Z',
        12, 0x2016184b, '?Replace@PrefixedBufferOwnerPilot@@QAEXPAX@Z']
];
const releases = [
    [0x201611be, '?Attach@InterfacePointerOwnerPilot@@AAEXPAUIUnknown@@@Z',
        9, 0x20161203, '?Release@InterfacePointerOwnerPilot@@AAEXXZ'],
    [0x201611d5, '?Replace@PrivateHeapPointerOwnerPilot@@AAEXPAX@Z',
        9, 0x20161030, '?Release@PrivateHeapPointerOwnerPilot@@AAEXXZ'],
    [0x201611ec, '?Release@MappedViewOwnerPilot@@QAEXXZ',
        14, 0x200e1834, '__imp__UnmapViewOfFile@4', 'DIR32'],
    [0x20161203, '?Release@InterfacePointerOwnerPilot@@AAEXXZ'],
    [0x20161030, '?Release@PrivateHeapPointerOwnerPilot@@AAEXXZ',
        13, 0x200f40a9, '_FreeOwnedPrivateHeapPointerPilot'],
    [0x2015115f, '?Release@PrefixedBufferOwnerPilot@@AAEXXZ',
        13, 0x20150c15, '_FreePrefixedBufferStoragePilot@4']
];
const heap = [
    [0x200f40a9, '_FreeOwnedPrivateHeapPointerPilot', [
        [14, 0x2020e050, '?g_privateHeapPilot@@3PAXA', 'DIR32'],
        [23, 0x200e19a0, '__imp__HeapFree@12', 'IAT', 'HeapFree']
    ]],
    [0x20150c15, '_FreePrefixedBufferStoragePilot@4', [
        [17, 0x2014e75f, '_FreeProcessHeapStoragePilot@4', 'REL32']
    ]],
    [0x2014e744, '_AllocateProcessHeapStoragePilot@4', [
        [12, 0x200e1994, '__imp__GetProcessHeap@0', 'IAT', 'GetProcessHeap'],
        [19, 0x200e19a8, '__imp__HeapAlloc@12', 'IAT', 'HeapAlloc']
    ]],
    [0x2014e75f, '_FreeProcessHeapStoragePilot@4', [
        [12, 0x200e1994, '__imp__GetProcessHeap@0', 'IAT', 'GetProcessHeap'],
        [19, 0x200e19a0, '__imp__HeapFree@12', 'IAT', 'HeapFree']
    ]]
];
const adapters = [
    [0x20161219, '?Replace@MappedViewOwnerPilot@@QAEXPBX@Z', [
        [9, 0x201611ec, '?Release@MappedViewOwnerPilot@@QAEXXZ', 'REL32']
    ]],
    [0x20161230, '?TakeFrom@MappedViewOwnerPilot@@QAEPAV1@PAPBX@Z', [
        [20, 0x20161219, '?Replace@MappedViewOwnerPilot@@QAEXPBX@Z', 'REL32']
    ]],
    [0x2016124f, '_ShareReferenceCounterPilot@8', [
        [16, 0x200e19c4, '__imp__InterlockedIncrement@4', 'IAT', 'InterlockedIncrement']
    ]],
    [0x20161047, '?Release@ProcessHeapPointerOwnerPilot@@AAEXXZ', [
        [13, 0x2014e75f, '_FreeProcessHeapStoragePilot@4', 'REL32']
    ]],
    [0x2016105d, '?Release@CriticalSectionLeasePilot@@QAEXXZ', [
        [17, 0x200e1758, '__imp__LeaveCriticalSection@4', 'IAT', 'LeaveCriticalSection']
    ]],
    [0x20161937, '_TransferInterfacePointerPilot@8', [
        [12, 0x20161815, '?TakeFrom@InterfacePointerOwnerPilot@@QAEPAV1@PAPAUIUnknown@@@Z', 'REL32']
    ]],
    [0x2016194d, '?Replace@ProcessHeapPointerOwnerPilot@@QAEXPAX@Z', [
        [9, 0x20161047, '?Release@ProcessHeapPointerOwnerPilot@@AAEXXZ', 'REL32']
    ]]
];
const mappingOwners = [
    [0x20160f45, '?Replace@FileMappingHandleOwnerPilot@@AAEXPAX@Z', [
        [9, 0x20160f6d, '?Close@FileMappingHandleOwnerPilot@@AAEXXZ', 'REL32'],
        [17, 0x20173c46, '_HandleValueRejectedPilot@4', 'REL32']
    ]],
    [0x20160f6d, '?Close@FileMappingHandleOwnerPilot@@AAEXXZ', [
        [10, 0x20173c46, '_HandleValueRejectedPilot@4', 'REL32'],
        [21, 0x200e176c, '__imp__CloseHandle@4', 'IAT', 'CloseHandle']
    ]],
    [0x20161077, '?ReplaceAndReturn@FileMappingHandleOwnerPilot@@QAEPAV1@PAX@Z', [
        [12, 0x20160f45, '?Replace@FileMappingHandleOwnerPilot@@AAEXPAX@Z', 'REL32']
    ]],
    [0x20161669, '?ReplaceThroughOwner@FileMappingHandleOwnerPilot@@QAEPAV1@PAX@Z', [
        [12, 0x20161077, '?ReplaceAndReturn@FileMappingHandleOwnerPilot@@QAEPAV1@PAX@Z', 'REL32']
    ]],
    [0x20161680, '?TakeFrom@FileMappingHandleOwnerPilot@@QAEPAV1@PAV1@@Z', [
        [20, 0x20160f45, '?Replace@FileMappingHandleOwnerPilot@@AAEXPAX@Z', 'REL32']
    ]],
    [0x20161964, '?TakeFrom@MappedFileOwnerPilot@@QAEPAV1@PAV1@@Z', [
        [14, 0x20161230, '?TakeFrom@MappedViewOwnerPilot@@QAEPAV1@PAPBX@Z', 'REL32'],
        [26, 0x20161680, '?TakeFrom@FileMappingHandleOwnerPilot@@QAEPAV1@PAV1@@Z', 'REL32']
    ]],
    [0x20161e65, '_TransferMappedFilePilot@8', [
        [12, 0x20161964, '?TakeFrom@MappedFileOwnerPilot@@QAEPAV1@PAV1@@Z', 'REL32']
    ]]
];
const leases = [
    [0x20161785, '?AddActivity@ActivityCounterPilot@@QAEXXZ', [
        [13, 0x200e175c, '__imp__EnterCriticalSection@4', 'IAT', 'EnterCriticalSection', 0x3d8b],
        [35, 0x200e19c4, '__imp__InterlockedIncrement@4', 'IAT', 'InterlockedIncrement'],
        [49, 0x200e1718, '__imp__ResetEvent@4', 'IAT', 'ResetEvent'],
        [58, 0x200e1758, '__imp__LeaveCriticalSection@4', 'IAT', 'LeaveCriticalSection', 0x358b]
    ]],
    [0x201617cd, '?RemoveActivity@ActivityCounterPilot@@QAEXXZ', [
        [12, 0x200e175c, '__imp__EnterCriticalSection@4', 'IAT', 'EnterCriticalSection'],
        [19, 0x200e18a4, '__imp__InterlockedDecrement@4', 'IAT', 'InterlockedDecrement'],
        [32, 0x200e1764, '__imp__SetEvent@4', 'IAT', 'SetEvent'],
        [39, 0x200e1758, '__imp__LeaveCriticalSection@4', 'IAT', 'LeaveCriticalSection']
    ]],
    [0x20161c75, '?Replace@CriticalSectionLeasePilot@@QAEXPAUCriticalSectionStoragePilot@@@Z', [
        [9, 0x2016105d, '?Release@CriticalSectionLeasePilot@@QAEXXZ', 'REL32'],
        [24, 0x2014eb01, '?EnterWhenIdle@CriticalSectionStoragePilot@@QAEXXZ', 'REL32']
    ]],
    [0x20161c95, '?Release@ActivityLeasePilot@@AAEXXZ', [
        [12, 0x201617cd, '?RemoveActivity@ActivityCounterPilot@@QAEXXZ', 'REL32']
    ]],
    [0x20161caa, '?Replace@ActivityLeasePilot@@QAEXPAVActivityCounterPilot@@@Z', [
        [9, 0x20161c95, '?Release@ActivityLeasePilot@@AAEXXZ', 'REL32'],
        [24, 0x20161785, '?AddActivity@ActivityCounterPilot@@QAEXXZ', 'REL32']
    ]],
    [0x2014eb01, '?EnterWhenIdle@CriticalSectionStoragePilot@@QAEXXZ', [
        [11, 0x200e175c, '__imp__EnterCriticalSection@4', 'IAT', 'EnterCriticalSection'],
        [29, 0x200e1874, '__imp__WaitForSingleObject@8', 'IAT', 'WaitForSingleObject']
    ]]
];
const buffer = [
    [0x20161bdd, '?ReplaceThroughOwner@PrivateHeapPointerOwnerPilot@@QAEPAV1@PAX@Z', [
        [12, 0x20161834, '?ReplaceAndReturn@PrivateHeapPointerOwnerPilot@@QAEPAV1@PAX@Z', 'REL32']
    ]],
    [0x20161bf4, '?Reset@ProcessHeapBufferOwnerPilot@@QAEXXZ', [
        [6, 0x20161047, '?Release@ProcessHeapPointerOwnerPilot@@AAEXXZ', 'REL32']
    ]],
    [0x20161c5e, '?ReplaceThroughOwner@PrefixedBufferOwnerPilot@@QAEPAV1@PAX@Z', [
        [12, 0x20161862, '?ReplaceAndReturn@PrefixedBufferOwnerPilot@@QAEPAV1@PAX@Z', 'REL32']
    ]],
    [0x20161d26, '?ReplaceAndReturn@ProcessHeapPointerOwnerPilot@@QAEPAV1@PAX@Z', [
        [12, 0x2016194d, '?Replace@ProcessHeapPointerOwnerPilot@@QAEXPAX@Z', 'REL32']
    ]]
];
const cleanup = [
    [0x20162571, '?Close@MappedFileOwnerPilot@@QAEXXZ', [
        [9, 0x20160f6d, '?Close@FileMappingHandleOwnerPilot@@AAEXXZ', 'REL32'],
        [17, 0x201611ec, '?Release@MappedViewOwnerPilot@@QAEXXZ', 'REL32', null, 0xe9]
    ]],
    [0x20162fd5, '?ReleaseIfPresent@InterfacePointerOwnerPilot@@SGXPAV1@@Z', [
        [13, 0x20161203, '?Release@InterfacePointerOwnerPilot@@AAEXXZ', 'REL32']
    ]],
    [0x20163585, '?CloseIfPresent@MappedFileOwnerPilot@@SGXPAV1@@Z', [
        [13, 0x20162571, '?Close@MappedFileOwnerPilot@@QAEXXZ', 'REL32']
    ]]
];
const customDefinitions = {heap, adapters, mapping: mappingOwners, leases, buffer, cleanup};
const definitions = customDefinitions[profile]
    ?? (isRelease ? releases : updates).map(([entry, symbol, offset, target, dependency, kind = 'REL32']) =>
        [entry, symbol, offset === undefined ? [] : [
            kind === 'DIR32' ? [offset, target, dependency, 'IAT', 'UnmapViewOfFile'] : [offset, target, dependency, kind]
        ]]);
const scoped = new Map(definitions.map(([entry, symbol]) => [entry, symbol]));
const possibleTargets = new Map([...releases, ...heap, ...updates, ...adapters, ...mappingOwners].map(([entry, symbol]) => [entry, symbol]));
const rows = definitions.map(([entry, symbol, fields]) => {
    const original = observed.get(entry);
    assert(original);
    assert.deepEqual(image.highLowFixups(entry, original.length), fields.filter(f => f[3] !== 'REL32').map(f => f[0]));
    const def = coff.unique(symbol);
    assert(def.section > 0);
    assert.equal(def.value, 0);
    const section = coff.sections[def.section - 1];
    assert(section.flags & 0x20);
    assert.equal(section.size, original.length);
    const rebuilt = object.subarray(section.raw, section.raw + section.size);
    const normalized = Buffer.from(rebuilt);
    assert.equal(section.relocations.length, fields.length);
    const mappings = fields.map(([offset, target, targetSymbol, kind, importName, opcode]) => {
        const relocation = section.relocations.find(r => r.offset === offset);
        assert(relocation);
        assert.equal(relocation.offset, offset);
        assert.equal(relocation.symbol.name, targetSymbol);
        const addend = rebuilt.readInt32LE(offset);
        assert.equal(addend, 0);
        const targetDefined = relocation.symbol.section > 0;
        const mapping = {
            offset, kind: kind === 'IAT' ? 'DIR32' : kind,
            symbol: targetSymbol, target: target.toString(16), addend, targetDefined
        };
        if (kind === 'IAT') {
            const iatOpcode = opcode ?? 0x15ff;
            assert.equal(relocation.type, 0x06);
            assert.equal(original.readUInt16LE(offset - 2), iatOpcode);
            assert.equal(rebuilt.readUInt16LE(offset - 2), iatOpcode);
            assert.equal(original.readUInt32LE(offset), target);
            assert.equal(relocation.symbol.section, 0);
            const identity = imports.get(target);
            assert.equal(identity.dll, 'kernel32.dll');
            assert.equal(identity.name, importName);
            mapping.import = identity.dll + '!' + identity.name;
            normalized.writeUInt32LE(target, offset);
        } else if (kind === 'DIR32') {
            assert.equal(relocation.type, 0x06);
            assert.equal(target, 0x2020e050);
            assert.equal(original[offset - 1], 0xa1);
            assert.equal(rebuilt[offset - 1], 0xa1);
            assert.equal(original.readUInt32LE(offset), target);
            assert.equal(live.privateHeap.address, target.toString(16));
            assert.equal(image.codeAt(target, 4, false).toString('hex'), live.privateHeap.hex);
            assert.equal(relocation.symbol.section, 0, 'Expected the existing private-heap global boundary');
            normalized.writeUInt32LE(target, offset);
        } else {
            assert(observed.has(target));
            assert.equal(relocation.type, 0x14);
            if (scoped.has(target)) {
                assert.equal(scoped.get(target), targetSymbol);
                assert(targetDefined);
            } else if (targetDefined) {
                assert.equal(possibleTargets.get(target), targetSymbol);
            } else {
                assert.equal(relocation.symbol.section, 0);
            }
            const branchOpcode = opcode ?? 0xe8;
            assert.equal(rebuilt[offset - 1], branchOpcode);
            assert.equal(original[offset - 1], branchOpcode);
            if (branchOpcode === 0xe9) mapping.branch = 'JMP';
            const next = entry + offset + 4;
            assert.equal(next + original.readInt32LE(offset), target);
            normalized.writeInt32LE(target - next, offset);
        }
        return mapping;
    });
    const different = !normalized.equals(original);
    if (different) {
        assert(profile === 'leases' && entry === 0x2014eb01, 'Unexpected difference at ' + entry.toString(16));
    }
    return {
        entry: entry.toString(16), status: different ? 'Different' : (mappings.length ? 'Normalized' : 'Exact'), symbol, bytes: original.length,
        originalHex: original.toString('hex'), rebuiltHex: rebuilt.toString('hex'),
        normalizedHex: normalized.toString('hex'),
        ...(mappings.length === 1 ? {relocation: mappings[0]} : {}),
        ...(different ? { differingOffsets: [...original.keys()].filter(offset => normalized[offset] !== original[offset]) } : {}),
        relocations: mappings
    };
});
if (profile === 'updates') {
    assert.equal(live.range.address, rows[0].entry);
    assert.equal(rows.map(row => row.originalHex).join(''), live.range.hex);
}
const result = {
    run, evidence, originalSHA256, objectSHA256: sha(object),
    buildCommandSHA256: sha(fs.readFileSync(path.join(run, 'build.cmd'))),
    scope: profile === 'leases'
        ? 'Native x86 COFF activity and lock lease comparison. Five direct calls and ten named import fields verified. Per-body status records remaining differences. Linked DLL and runtime remain open.'
        : profile === 'buffer'
        ? 'Complete native x86 COFF buffer-owner forwarding and reset bodies. Four direct calls resolve to semantically matched owner routines checked by the existing profiles. Linked DLL and runtime remain open.'
        : profile === 'cleanup'
        ? 'Complete native x86 COFF cleanup bodies with three verified direct calls and one verified tail jump. Linked DLL and runtime remain open.'
        : profile === 'mapping'
        ? 'Complete native x86 COFF mapped file and handle ownership bodies. Nine direct call targets and the CloseHandle import verified. The handle predicate has an unresolved implementation. Linked DLL and runtime remain open.'
        : profile === 'adapters'
        ? 'Complete native x86 COFF pointer-owner adapters. Five direct call targets and two named IAT entries verified. Linked DLL and runtime remain open.'
        : isHeap
        ? 'Complete native x86 COFF heap bridge bodies. Original private heap cell, imported functions and direct call target verified. Linked DLL and runtime remain open.'
        : isRelease
        ? 'Complete native x86 COFF release and replacement bodies. Four direct call targets and the UnmapViewOfFile import verified. Heap endpoints remain external. Linked DLL and runtime remain open.'
        : 'Complete native x86 COFF bodies at 0x20161815–0x20161878 with four verified REL32 fields. Linked DLL and runtime remain open.',
    totalBytes: rows.reduce((sum, row) => sum + row.bytes, 0), rows
};
fs.writeFileSync(path.join(run, label + '-comparison.json'), JSON.stringify(result, null, 2) + '\n');
console.log(`${rows.filter(row => row.status !== 'Different').length}/${rows.length} matched pointer-owner bodies, ${result.totalBytes} bytes compared and ${rows.reduce((sum, row) => sum + row.relocations.length, 0)} verified relocation fields`);
