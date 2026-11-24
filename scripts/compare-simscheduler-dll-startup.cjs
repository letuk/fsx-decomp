const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');
const evidence = require('../artifacts/simscheduler.dll/dll-startup-ghidra.json');
const onexit = require('../artifacts/simscheduler.dll/crt-onexit-ghidra.json');
const rtc = require('../artifacts/simscheduler.dll/crt-rtc-ghidra.json');
const peSections = require('../artifacts/simscheduler.dll/crt-pe-sections-ghidra.json');
const thunks = require('../artifacts/simscheduler.dll/crt-import-thunks-ghidra.json');

const run = process.argv[2];
assert(run, 'Pass the linked DLL startup run directory');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const originalBytes = fs.readFileSync(path.join(__dirname, '../original/simscheduler.dll'));
const rebuiltBytes = fs.readFileSync(path.join(run, 'simscheduler-startup.dll'));
assert.equal(sha(originalBytes), '16059b50e722bfa10357582def8d422a8222f620189e40c8878928a7ddaa9dd4');
const original = openImage(originalBytes);
const rebuilt = openImage(rebuiltBytes);
const originalImports = readPE32Imports(originalBytes);
const rebuiltImports = readPE32Imports(rebuiltBytes);
const map = fs.readFileSync(path.join(run, 'worker-output/fsx.map'), 'utf8');
assert.equal(parseInt(map.match(/Preferred load address is ([0-9a-f]+)/i)[1], 16), rebuilt.base);
const symbols = new Map();
for (const row of map.matchAll(/^\s*[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})\s/mgi)) {
    symbols.set(row[1], parseInt(row[2], 16));
}
function symbol(name) {
    const address = symbols.get(name);
    assert(address, name);
    return address;
}

const rebuiltEntry = symbol('__DllMainCRTStartup@12');
const rebuiltDispatch = symbol('___DllMainCRTStartup');
const resetReason = rebuiltEntry - 11;
assert.equal(original.base + original.entryRva, 0x20cc6988);
assert.equal(rebuilt.base + rebuilt.entryRva, rebuiltEntry);
assert.equal(rebuilt.codeAt(resetReason, 11).toString('hex').slice(12), 'ffffffffc3');
assert.equal(rebuilt.codeAt(resetReason + 2, 4).readUInt32LE(), symbol('___native_dllmain_reason'));
assert.equal(rebuilt.codeAt(resetReason, 2).toString('hex'), 'c705');

const originalRoles = new Map();
const rebuiltRoles = new Map();
function role(oldAddress, newAddress, name) {
    originalRoles.set(oldAddress, name);
    rebuiltRoles.set(newAddress, name);
}
const bodies = [
    [0x20cc4980, 48, '_SchedulerRawDllMain@12', symbol('_DllMain@12')],
    [0x20cc49c0, 60, '_DllMain@12', symbol('_pre_c_init')],
    [0x20cc6677, 77, '_pre_c_init', symbol('__CRT_INIT@12')],
    [0x20cc66c9, 415, '__CRT_INIT@12', rebuiltDispatch],
    [0x20cc686d, 267, '___DllMainCRTStartup', resetReason],
    [0x20cc6988, 33, '__DllMainCRTStartup@12', symbol('___clean_type_info_names')],
    [0x20cc6c08, 69, '__SEH_prolog4', symbol('__SEH_epilog4')],
    [0x20cc6c4d, 20, '__SEH_epilog4', symbol('__except_handler4')],
    [0x20cc6c8e, 148, '___security_init_cookie', symbol('___clean_type_info_names_internal')],
];
for (const [address, , name] of bodies) role(address, symbol(name), name);
role(0x20cc6978, resetReason, 'reset DLL notification reason');
for (const [address, name] of [
    [0x20cc6b67, '__IsNonwritableInCurrentImage'],
    [0x20cc6bd8, '__initterm'],
    [0x20cc6be4, '__initterm_e'],
    [0x20cc6bf0, '__amsg_exit'],
    [0x20cc6bfc, '___CppXcptFilter'],
    [0x20cc6c66, '__except_handler4'],
    [0x20cc6e60, '__except_handler4_common'],
    [0x20cc6e6c, '__crt_debugger_hook'],
    [0x20cc6e30, '__unlock'],
    [0x20cc6e3c, '___dllonexit'],
    [0x20cc6e48, '__lock'],
    [0x20cc6a66, '_atexit'],
    [0x20cc6a89, '___clean_type_info_names'],
    [0x20cc6e54, '___clean_type_info_names_internal'],
    [0x20cc6a9a, '__RTC_Initialize'],
    [0x20cc6ac3, '__RTC_Terminate'],
]) {
    const target = symbol(name);
    if (original.codeAt(address, 2).readUInt16LE() === 0x25ff) {
        assert.equal(rebuilt.codeAt(target, 2).readUInt16LE(), 0x25ff);
        assert.deepEqual(originalImports.get(original.codeAt(address + 2, 4).readUInt32LE()),
            rebuiltImports.get(rebuilt.codeAt(target + 2, 4).readUInt32LE()));
    }
    role(address, target, name);
}
for (const [address, name] of [
    [0x20cc8000, '___native_dllmain_reason'],
    [0x20cc8008, '___security_cookie'],
    [0x20cc800c, '___security_cookie_complement'],
    [0x20cc8024, '?__type_info_root_node@@3U__type_info_node@@A'],
    [0x20cc835c, '_schedulerModule'],
    [0x20cc83f8, '__adjust_fdiv'],
    [0x20cc83fc, '___native_startup_state'],
    [0x20cc8400, '___native_startup_lock'],
    [0x20cc8404, '___onexitend'],
    [0x20cc8408, '___onexitbegin'],
    [0x20cc840c, '___dyn_tls_init_callback'],
    [0x20cc1118, '___xc_a'],
    [0x20cc1128, '___xc_z'],
    [0x20cc112c, '___xi_a'],
    [0x20cc1138, '___xi_z'],
    [0x20cc139c, '___rtc_iaa'],
    [0x20cc13a0, '___rtc_izz'],
    [0x20cc13a4, '___rtc_taa'],
    [0x20cc13a8, '___rtc_tzz'],
]) {
    const target = symbol(name);
    assert.equal(original.initialDword(address), rebuilt.initialDword(target), name);
    role(address, target, name);
}
const init = symbol('__CRT_INIT@12');
assert.equal(original.codeAt(0x20cc66d4, 2).toString('hex'), '392d');
assert.equal(rebuilt.codeAt(init + 11, 2).toString('hex'), '392d');
assert.equal(original.codeAt(0x20cc66d6, 4).readUInt32LE(), 0x20cc8020);
const attached = rebuilt.codeAt(init + 13, 4).readUInt32LE();
assert.equal(original.initialDword(0x20cc8020), rebuilt.initialDword(attached));
role(0x20cc8020, attached, 'process attach count');

const rebuiltRawSlot = symbol('__pRawDllMain');
assert.equal(evidence.rawDllMainSlot.hex, '8049cc20');
assert.equal(original.initialDword(0x20cc1178), 0x20cc4980);
assert.equal(rebuilt.initialDword(rebuiltRawSlot), symbol('_SchedulerRawDllMain@12'));
assert.deepEqual(original.highLowFixups(0x20cc1178, 4), [0]);
assert.deepEqual(rebuilt.highLowFixups(rebuiltRawSlot, 4), [0]);
role(0x20cc1178, rebuiltRawSlot, '_pRawDllMain callback slot');

assert.equal(original.codeAt(0x20cc686f, 1)[0], 0x68);
assert.equal(rebuilt.codeAt(rebuiltDispatch + 2, 1)[0], 0x68);
const rebuiltScopeAddress = rebuilt.codeAt(rebuiltDispatch + 3, 4).readUInt32LE();
const originalScope = original.codeAt(0x20cc7340, 40, false);
const rebuiltScope = rebuilt.codeAt(rebuiltScopeAddress, 40, false);
assert.equal(evidence.scope.hex, originalScope.toString('hex'));
assert.deepEqual(original.highLowFixups(0x20cc7340, 40), [24, 32, 36]);
assert.deepEqual(rebuilt.highLowFixups(rebuiltScopeAddress, 40), [24, 32, 36]);
const resolvedScope = Buffer.from(rebuiltScope);
assert.equal(rebuiltScope.readUInt32LE(24), resetReason);
assert.equal(originalScope.readUInt32LE(24), 0x20cc6978);
for (const [offset, oldTarget, length] of [[32, 0x20cc6944, 10], [36, 0x20cc6958, 7]]) {
    assert.equal(originalScope.readUInt32LE(offset), oldTarget);
    const target = rebuiltScope.readUInt32LE(offset);
    assert(target >= rebuiltDispatch && target + length <= resetReason);
    assert.deepEqual(original.codeAt(oldTarget, length), rebuilt.codeAt(target, length));
}
for (const offset of [24, 32, 36]) originalScope.copy(resolvedScope, offset, offset, offset + 4);
assert.deepEqual(resolvedScope, originalScope);
role(0x20cc7340, rebuiltScopeAddress, 'validated EH4 scope table');

const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'utf8')
    .replace(/^( +[0-9a-f]{8}: (?:[0-9a-f]{2} )*[0-9a-f]{2}) +(.*)\r?\n[ \t]+([0-9a-f]{2}(?: [0-9a-f]{2})*)[ \t]*$/gim,
        '$1 $3  $2');
const rebuiltRows = Array.from(listing.matchAll(/^ +([0-9a-f]{8}): ((?:[0-9a-f]{2} )*[0-9a-f]{2}) +(\S[^\r\n]*)/gim),
    row => [row[1].toLowerCase(), row[2], row[3].trim().replace(/\s+/g, ' ')]);

function references(image, imports, roles, start, body, instructionAddresses) {
    const result = [];
    function add(offset, kind, target) {
        const imported = imports.get(target);
        const identity = imported ? imported.dll.toLowerCase() + '!' + imported.name : roles.get(target);
        result.push({ offset, kind, target, identity });
    }
    for (const offset of image.highLowFixups(start, body.length)) {
        add(offset, 'DIR32', body.readUInt32LE(offset));
    }
    for (const address of instructionAddresses) {
        const offset = parseInt(address, 16) - start;
        if (body[offset] === 0xe8 || (body[offset] === 0xe9 && offset === 10
            && roles.get(start) === '@__security_check_cookie@4')) {
            add(offset + 1, 'REL32', start + offset + 5 + body.readInt32LE(offset + 1));
        }
    }
    return result.sort((a, b) => a.offset - b.offset);
}
const ranges = bodies.map(([entry, length, name, end]) => [entry, length, symbol(name), end]);
ranges.push([0x20cc6978, 11, resetReason, rebuiltEntry]);
const listings = { ...evidence.functions, ...onexit.functions, ...rtc.functions, ...peSections.functions,
    ...thunks.functions };
listings['20cc6c66'] = require('../artifacts/simscheduler.dll/crt-eh4-handler-ghidra.json');
const remaining = require('../artifacts/simscheduler.dll/remaining-functions-ghidra.json');
for (const [entry, name] of [[0x20cc6e30, '__unlock'], [0x20cc6e3c, '___dllonexit'],
    [0x20cc6e48, '__lock'], [0x20cc6e54, '___clean_type_info_names_internal'],
    [0x20cc6e60, '__except_handler4_common']]) {
    const start = symbol(name);
    listings[entry.toString(16)] = remaining.functions[entry.toString(16)];
    assert.equal(original.codeAt(entry, 2).readUInt16LE(), 0x25ff);
    assert.equal(rebuilt.codeAt(start, 2).readUInt16LE(), 0x25ff);
    assert.deepEqual(original.highLowFixups(entry, 6), [2]);
    assert.deepEqual(rebuilt.highLowFixups(start, 6), [2]);
    ranges.push([entry, 6, start, start + 6]);
}
const cookieCheck = symbol('@__security_check_cookie@4');
for (const [image, start, cookie] of [[original, 0x20cc69ae, 0x20cc8008],
    [rebuilt, cookieCheck, symbol('___security_cookie')]]) {
    const prefix = image.codeAt(start, 6);
    assert.equal(prefix.readUInt16LE(), 0x0d3b);
    assert.equal(prefix.readUInt32LE(2), cookie);
}
role(0x20cc69ae, cookieCheck, '@__security_check_cookie@4');
const gsFailure = symbol('___report_gsfailure');
role(0x20cc6d27, gsFailure, '___report_gsfailure');
listings['20cc69ae'] = require('../artifacts/simscheduler.dll/crt-cookie-check-ghidra.json');
listings['20cc6d27'] = require('../artifacts/simscheduler.dll/crt-gs-failure-ghidra.json');
listings['20cc6e6c'] = require('../artifacts/simscheduler.dll/crt-debugger-hook-ghidra.json');
ranges.push([0x20cc69ae, 15, cookieCheck, cookieCheck + 15]);
const debuggerHook = symbol('__crt_debugger_hook');
ranges.push([0x20cc6d27, 260, gsFailure, debuggerHook], [0x20cc6e6c, 6, debuggerHook, debuggerHook + 6]);

// EXCEPTION_POINTERS independently anchors the exception record and context.
const pointers = 0x20cc1160;
const rebuiltPointers = rebuilt.codeAt(gsFailure + (0x20cc6dfc - 0x20cc6d27), 4).readUInt32LE();
assert.deepEqual(original.highLowFixups(pointers, 8), [0, 4]);
assert.deepEqual(rebuilt.highLowFixups(rebuiltPointers, 8), [0, 4]);
assert.equal(original.initialDword(pointers), 0x20cc8030);
assert.equal(original.initialDword(pointers + 4), 0x20cc8088);
role(pointers, rebuiltPointers, 'GS EXCEPTION_POINTERS');
for (const [base, linkedBase, name, offsets] of [
    [0x20cc8030, rebuilt.initialDword(rebuiltPointers), 'GS EXCEPTION_RECORD', [0, 4, 12]],
    [0x20cc8088, rebuilt.initialDword(rebuiltPointers + 4), 'GS x86 CONTEXT',
        [0, 0x8c, 0x90, 0x94, 0x98, 0x9c, 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4,
            0xb8, 0xbc, 0xc0, 0xc4, 0xc8]],
]) {
    for (const offset of offsets) {
        assert.equal(original.initialDword(base + offset), rebuilt.initialDword(linkedBase + offset));
        role(base + offset, linkedBase + offset, name + '+' + offset.toString(16));
    }
}
// The debugger flag is the destination of EAX immediately after IsDebuggerPresent.
const rebuiltDebuggerFlag = rebuilt.codeAt(gsFailure + (0x20cc6de7 - 0x20cc6d27), 4).readUInt32LE();
assert.equal(original.initialDword(0x20cc8080), rebuilt.initialDword(rebuiltDebuggerFlag));
role(0x20cc8080, rebuiltDebuggerFlag, 'GS debugger-present flag');
const eh4Handler = symbol('__except_handler4');
ranges.push([0x20cc6c66, 35, eh4Handler, eh4Handler + 35]);
assert.equal(original.codeAt(0x20cc6bd8, 48).toString('hex'), thunks.raw.hex);
for (const [entry, name] of [[0x20cc6bd8, '__initterm'], [0x20cc6be4, '__initterm_e'],
    [0x20cc6bf0, '__amsg_exit'], [0x20cc6bfc, '___CppXcptFilter']]) {
    const start = symbol(name);
    assert.equal(original.codeAt(entry, 2).readUInt16LE(), 0x25ff);
    assert.equal(rebuilt.codeAt(start, 2).readUInt16LE(), 0x25ff);
    assert.deepEqual(original.highLowFixups(entry, 6), [2]);
    assert.deepEqual(rebuilt.highLowFixups(start, 6), [2]);
    const imported = originalImports.get(original.codeAt(entry + 2, 4).readUInt32LE());
    assert.equal(imported?.dll.toLowerCase(), 'msvcr80.dll');
    ranges.push([entry, 6, start, start + 6]);
}
const validateImage = symbol('__ValidateImageBase');
const findSection = symbol('__FindPESection');
const checkSection = symbol('__IsNonwritableInCurrentImage');
assert.equal(symbol('___ImageBase'), rebuilt.base);
role(0x20cc0000, rebuilt.base, 'current PE image base');
role(0x20cc6af0, validateImage, '__ValidateImageBase');
role(0x20cc6b20, findSection, '__FindPESection');
assert.equal(original.codeAt(0x20cc6b69, 1)[0], 0x68);
const sectionPrologue = rebuilt.codeAt(checkSection, 5);
const inlineSectionSeh = sectionPrologue.toString('hex') === '558bec6afe';
const scopeOperand = inlineSectionSeh ? 6 : 3;
assert.equal(rebuilt.codeAt(checkSection + scopeOperand - 1, 1)[0], 0x68);
assert.equal(original.codeAt(0x20cc6b6a, 4).readUInt32LE(), 0x20cc7388);
const sectionScopeAddress = rebuilt.codeAt(checkSection + scopeOperand, 4).readUInt32LE();
const oldSectionScope = original.codeAt(0x20cc7388, 28, false);
const newSectionScope = rebuilt.codeAt(sectionScopeAddress, 28, false);
assert.equal(oldSectionScope.toString('hex'), peSections.scope.hex);
assert.deepEqual(oldSectionScope.subarray(0, 20), newSectionScope.subarray(0, 20));
assert.deepEqual(original.highLowFixups(0x20cc7388, 28), [20, 24]);
assert.deepEqual(rebuilt.highLowFixups(sectionScopeAddress, 28), [20, 24]);
for (const [offset, target] of [[20, 0x20cc6bad], [24, 0x20cc6bc1]]) {
    assert.equal(oldSectionScope.readUInt32LE(offset), target);
    const linkedTarget = newSectionScope.readUInt32LE(offset);
    if (inlineSectionSeh) {
        assert(linkedTarget > checkSection && linkedTarget + 3 <= symbol('__initterm'));
        assert.deepEqual(rebuilt.codeAt(linkedTarget, 3), original.codeAt(target, 3));
    } else {
        assert.equal(linkedTarget, checkSection + target - 0x20cc6b67);
    }
}
role(0x20cc7388, sectionScopeAddress, 'validated PE section EH4 scope table');
ranges.push([0x20cc6af0, 41, validateImage, findSection], [0x20cc6b20, 66, findSection, checkSection],
    [0x20cc6b67, 108, checkSection, symbol('__initterm')]);
assert.equal(original.codeAt(0x20cc139c, 16, false).toString('hex'), rtc.sentinels.hex);
assert.equal(rtc.sentinels.hex, '00000000000000000000000000000000');
assert.equal(symbol('___rtc_izz') - symbol('___rtc_iaa'), 4);
assert.equal(symbol('___rtc_tzz') - symbol('___rtc_taa'), 4);
ranges.push([0x20cc6a9a, 36, symbol('__RTC_Initialize'), symbol('__RTC_Terminate')],
    [0x20cc6ac3, 36, symbol('__RTC_Terminate'), symbol('__ValidateImageBase')]);
listings['20cc6a89'] = require('../artifacts/simscheduler.dll/crt-type-info-cleanup-ghidra.json');
ranges.push([0x20cc6a89, 12, symbol('___clean_type_info_names'), symbol('__onexit')]);
const registerExit = symbol('__onexit');
const exitWrapper = symbol('_atexit');
assert.equal(rebuilt.codeAt(registerExit + 2, 1)[0], 0x68);
assert.equal(original.codeAt(0x20cc69c4, 1)[0], 0x68);
assert.equal(original.codeAt(0x20cc69c5, 4).readUInt32LE(), 0x20cc7368);
const exitScopeAddress = rebuilt.codeAt(registerExit + 3, 4).readUInt32LE();
const originalExitScope = original.codeAt(0x20cc7368, 28, false);
const rebuiltExitScope = rebuilt.codeAt(exitScopeAddress, 28, false);
const unlockExit = rebuiltExitScope.readUInt32LE(24);
assert(unlockExit > registerExit && unlockExit + 9 === exitWrapper);
assert.equal(symbol('__RTC_Initialize') - exitWrapper, 18);
assert.equal(originalExitScope.toString('hex'), onexit.scope.hex);
assert.deepEqual(original.highLowFixups(0x20cc7368, 28), [24]);
assert.deepEqual(rebuilt.highLowFixups(exitScopeAddress, 28), [24]);
assert.equal(originalExitScope.readUInt32LE(24), 0x20cc6a58);
assert.deepEqual(originalExitScope.subarray(0, 24), rebuiltExitScope.subarray(0, 24));
role(0x20cc7368, exitScopeAddress, 'validated on-exit EH4 scope table');
role(0x20cc69c2, registerExit, '__onexit');
role(0x20cc6a58, unlockExit, 'CRT exit registration lock cleanup');
ranges.push([0x20cc69c2, 150, registerExit, unlockExit], [0x20cc6a58, 9, unlockExit, exitWrapper],
    [0x20cc6a66, 18, exitWrapper, symbol('__RTC_Initialize')]);
let sse2Scope;
assert.equal(original.codeAt(0x20cc1118, 36, false).toString('hex'), evidence.initializerTables.hex);
assert.equal(original.initialDword(0x20cc1130), 0x20cc6677);
assert.equal(original.initialDword(0x20cc1134), 0x20cc7021);
assert.equal(rebuilt.initialDword(symbol('___xi_a') + 4), symbol('_pre_c_init'));
if (symbols.has('___sse2_available')) {
    assert.equal(symbol('___xi_z') - symbol('___xi_a'), 12);
    const start = rebuilt.initialDword(symbol('___xi_a') + 8);
    assert.deepEqual(rebuilt.highLowFixups(symbol('___xi_a') + 8, 4), [0]);
    assert.equal(original.initialDword(0x20cc83f4), rebuilt.initialDword(symbol('___sse2_available')));
    role(0x20cc83f4, symbol('___sse2_available'), 'CRT SSE2 availability flag');
    const sse2 = require('../artifacts/simscheduler.dll/sse2-detection-ghidra.json');
    Object.assign(listings, sse2.functions);
    const probe = symbol('_has_osfxsr_set');
    const detect = symbol('__get_sse2_info');
    assert.equal(start, symbol('___sse2_available_init'));
    assert.equal(detect - probe, 80);
    assert.equal(start - detect, 96);
    assert.equal(original.codeAt(0x20cc6f69, 1)[0], 0x68);
    assert.equal(rebuilt.codeAt(probe + 2, 1)[0], 0x68);
    assert.equal(original.codeAt(0x20cc6f6a, 4).readUInt32LE(), 0x20cc75a8);
    const scopeAddress = rebuilt.codeAt(probe + 3, 4).readUInt32LE();
    const oldScope = original.codeAt(0x20cc75a8, 28, false);
    const newScope = rebuilt.codeAt(scopeAddress, 28, false);
    assert.equal(oldScope.toString('hex'), sse2.scope.hex);
    assert.deepEqual(original.highLowFixups(0x20cc75a8, 28), [20, 24]);
    assert.deepEqual(rebuilt.highLowFixups(scopeAddress, 28), [20, 24]);
    const resolved = Buffer.from(newScope);
    for (const [offset, relative] of [[20, 0x1d], [24, 0x39]]) {
        assert.equal(oldScope.readUInt32LE(offset), 0x20cc6f67 + relative);
        assert.equal(newScope.readUInt32LE(offset), probe + relative);
        oldScope.copy(resolved, offset, offset, offset + 4);
    }
    assert.deepEqual(resolved, oldScope);
    sse2Scope = { original: '20cc75a8', rebuilt: scopeAddress.toString(16), mappedOffsets: [20, 24] };
    role(0x20cc75a8, scopeAddress, 'validated SSE2 EH4 scope table');
    role(0x20cc6f67, probe, '_has_osfxsr_set');
    role(0x20cc6fbc, detect, '__get_sse2_info');
    ranges.push([0x20cc6f67, 80, probe, detect], [0x20cc6fbc, 96, detect, start]);
    const ret = rebuiltRows.find(row => parseInt(row[0], 16) >= start && /^ret\b/i.test(row[2]));
    assert(ret);
    ranges.push([0x20cc7021, 13, start, parseInt(ret[0], 16) + ret[1].replaceAll(' ', '').length / 2]);
}
if (symbols.has('__ftol2_sse')) {
    const conversion = require('../artifacts/simscheduler.dll/crt-conversion-ghidra.json');
    Object.assign(listings, conversion.functions);
    const start = symbol('__ftol2_sse');
    assert.equal(symbol('__ftol2_pentium4') - start, 9);
    assert.equal(symbol('__ftol2_sse_excpt') - start, 28);
    assert.equal(symbol('__ftol2') - start, 54);
    ranges.push([0x20cc6ea0, 171, start, start + 171]);
    for (const [entry, name, importedName] of [
        [0x20cc6e86, '_memcpy', 'memcpy'],
        [0x20cc6f50, '___CxxFrameHandler3', '__CxxFrameHandler3'],
        [0x20cc6f5c, '_memset', 'memset'],
    ]) {
        const thunk = symbol(name);
        for (const [image, imports, address] of [[original, originalImports, entry], [rebuilt, rebuiltImports, thunk]]) {
            const code = image.codeAt(address, 6);
            assert.equal(code.readUInt16LE(), 0x25ff);
            assert.deepEqual(image.highLowFixups(address, 6), [2]);
            const imported = imports.get(code.readUInt32LE(2));
            assert.equal(imported?.dll.toLowerCase(), 'msvcr80.dll');
            assert.equal(imported?.name, importedName);
        }
        ranges.push([entry, 6, thunk, thunk + 6]);
    }
}
for (const [entry, length, start, end] of ranges) {
    const address = entry.toString(16);
    const left = original.codeAt(entry, length);
    const storedInstructions = listings[address].instructions;
    if (storedInstructions[0].bytes) {
        assert.equal(left.toString('hex'), storedInstructions.map(ins => ins.bytes).join(''));
    }
    const rows = rebuiltRows.filter(([address]) => parseInt(address, 16) >= start && parseInt(address, 16) < end);
    if (entry === 0x20cc49c0 || entry === 0x20cc6988 || entry === 0x20cc6c8e) {
        // These probe bodies have a single return. The CRT can place other
        // library members between the body and the next anchor in the map.
        const ret = rows.findIndex(row => /^ret\b/i.test(row[2]));
        assert(ret >= 0);
        rows.length = ret + 1;
    }
    while (rows.length && (/^int 3$/i.test(rows.at(-1)[2]) || /^cc( cc)*$/i.test(rows.at(-1)[1]))) rows.pop();
    assert.equal(parseInt(rows[0][0], 16), start, address);
    let offset = 0;
    for (const [address, bytes] of rows) {
        const raw = Buffer.from(bytes.replaceAll(' ', ''), 'hex');
        assert.equal(parseInt(address, 16), start + offset);
        assert.deepEqual(rebuilt.codeAt(start + offset, raw.length), raw);
        offset += raw.length;
    }
    const right = rebuilt.codeAt(start, offset);
    const leftRefs = references(original, originalImports, originalRoles, entry, left,
        storedInstructions.map(ins => ins.address));
    const rightRefs = references(rebuilt, rebuiltImports, rebuiltRoles, start, right, rows.map(row => row[0]));
    assert(leftRefs.every(ref => ref.identity), 'Unverified original references at ' + address);
    const mappedOffsets = [];
    const normalized = Buffer.from(right);
    for (const ref of leftRefs) {
        const match = rightRefs.find(candidate => candidate.offset === ref.offset
            && candidate.kind === ref.kind && candidate.identity === ref.identity);
        if (match) {
            left.copy(normalized, ref.offset, ref.offset, ref.offset + 4);
            mappedOffsets.push(ref.offset);
        }
    }
    const matches = leftRefs.length === rightRefs.length && mappedOffsets.length === leftRefs.length
        && left.equals(normalized);
    const status = !matches ? 'Diagnostic' : left.equals(right) ? 'Exact' : 'Normalized';
    const report = {
        program: 'simscheduler.dll', entry: address, rebuiltEntry: start.toString(16), status,
        originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
        originalHex: left.toString('hex'), rebuiltHex: right.toString('hex'), mappedOffsets,
        originalReferences: leftRefs, rebuiltReferences: rightRefs, rebuilt: rows,
        ...(entry === 0x20cc686d ? { verifiedScopeFields: [24, 32, 36] } : {}),
        ...(entry === 0x20cc69c2 ? { verifiedScope: {
            original: '20cc7368', rebuilt: exitScopeAddress.toString(16), mappedOffsets: [24],
        } } : {}),
        ...(entry === 0x20cc6b67 ? { verifiedScope: {
            original: '20cc7388', rebuilt: sectionScopeAddress.toString(16), mappedOffsets: [20, 24],
        } } : {}),
        ...(entry === 0x20cc6f67 ? { verifiedScope: sse2Scope } : {}),
    };
    fs.writeFileSync(path.join(run, 'comparison-' + address + '.json'), JSON.stringify(report, null, 2) + '\n');
    console.log(status + ' ' + address + ' (' + left.length + ' original bytes, ' + right.length + ' rebuilt bytes)');
}
