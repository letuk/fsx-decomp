const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { readPE32Imports } = require('./lib/pe-imports.cjs');

const run = process.argv[2];
assert(run, 'Pass the linked dispatcher run directory');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');

const { openImage } = require('./lib/pe32-dll.cjs');

const originalBytes = fs.readFileSync(path.join(__dirname, '../original/simscheduler.dll'));
const rebuiltBytes = fs.readFileSync(path.join(run, 'simscheduler-dispatch.dll'));
assert.equal(sha(originalBytes), '16059b50e722bfa10357582def8d422a8222f620189e40c8878928a7ddaa9dd4');
const original = openImage(originalBytes);
const rebuilt = openImage(rebuiltBytes);
assert.equal(rebuilt.entryRva, 0);
const map = fs.readFileSync(path.join(run, 'worker-output/fsx.map'), 'utf8');
assert.equal(Number.parseInt(map.match(/Preferred load address is ([0-9a-f]+)/i)[1], 16), rebuilt.base);

function mapAddress(symbol) {
    const rows = map.split(/\r?\n/).map(line => line.trim().split(/\s+/))
        .filter(fields => fields[1] === symbol);
    assert.equal(rows.length, 1, symbol);
    return Number.parseInt(rows[0][2], 16);
}

function probeAddress(name, legacySymbol) {
    const prefix = '?' + name + '@@';
    const symbols = map.split(/\r?\n/).map(line => line.trim().split(/\s+/)[1])
        .filter(symbol => symbol && (symbol === legacySymbol || symbol.startsWith(prefix)));
    assert.equal(symbols.length, 1, name);
    return mapAddress(symbols[0]);
}

const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'utf8')
    .replace(/^( +[0-9a-f]{8}: (?:[0-9a-f]{2} )*[0-9a-f]{2}) +(.*)\r?\n[ \t]+([0-9a-f]{2}(?: [0-9a-f]{2})*)[ \t]*$/gim,
        '$1 $3  $2');
function readListing(start, end, stopAtReturn = false) {
    const rows = [];
    let next = start;
    for (const match of listing.matchAll(/^ +([0-9a-f]{8}): ((?:[0-9a-f]{2} )*[0-9a-f]{2}) +(\S[^\r\n]*)/gim)) {
        const address = Number.parseInt(match[1], 16);
        if (address < start) continue;
        if (address >= end || /^CC( CC)*$/i.test(match[2]) ||
            (/^00( 00)*$/.test(match[2]) && /^\.+\s*$/.test(match[3]))) break;
        assert.equal(address, next);
        const hex = match[2].replaceAll(' ', '');
        const instruction = match[3].trim().replace(/\s+/g, ' ');
        assert.equal(rebuilt.codeAt(address, hex.length / 2).toString('hex'), hex.toLowerCase());
        rows.push([match[1], match[2], instruction]);
        next += hex.length / 2;
        if (stopAtReturn && instruction.startsWith('ret')) break;
    }
    return rows;
}
function decodeCall(image, address) {
    const call = image.codeAt(address, 5);
    assert.equal(call[0], 0xe8);
    return address + 5 + call.readInt32LE(1);
}
const request = JSON.parse(fs.readFileSync(path.join(run, 'request.json'), 'utf8').replace(/^\uFEFF/, ''));
if (request.DispatcherProfile === 'ContextStorageCleanupLtcg') {
    const entry = 0x20cc15f0;
    const action = 0x20cc71e9;
    const release = 0x20cc20b0;
    const originalBody = original.codeAt(entry, 10);
    const originalAction = original.codeAt(action, 8);
    assert.equal(decodeCall(original, entry + 3), release);
    assert.equal(decodeCall(original, 0x20cc28d3), release);
    assert.equal(originalAction[3], 0xe9);
    assert.equal(action + 8 + originalAction.readInt32LE(4), entry);

    const wrapperSymbols = {
        New: '??3SchedulerContext@@SAXPAX@Z',
        Placement: '??3@YAXPAXUContextPoolAllocation@@@Z',
        Guard: '?ReturnUnconstructedContextStorage@@YAXPAUSchedulerContext@@@Z',
    };
    const wrapperSymbol = wrapperSymbols[request.ContextStorageCleanup ?? 'New'];
    const actionSymbol = '__unwindfunclet$?ProbeConstructPooledContext@@YGPAUSchedulerContext@@PBD@Z$0';
    const rebuiltEntry = mapAddress(wrapperSymbol);
    const rebuiltRelease = probeAddress('ReleaseContextPoolStorage');
    const rebuiltAction = mapAddress(actionSymbol);
    const nextFunction = address => Math.min(...[...map.matchAll(/^\s+0001:[0-9a-f]+\s+\S+\s+([0-9a-f]+)\s+f\s/gim)]
        .map(match => Number.parseInt(match[1], 16)).filter(candidate => candidate > address));
    const rows = readListing(rebuiltEntry, nextFunction(rebuiltEntry), true);
    const releaseRows = readListing(rebuiltRelease, nextFunction(rebuiltRelease));
    const actionRows = readListing(rebuiltAction, nextFunction(rebuiltAction), true);
    const rebuiltBody = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
    const directTransfers = rows => rows.filter(row => /^(?:E8|E9) /i.test(row[1]))
        .map(row => {
            const site = Number.parseInt(row[0], 16);
            const target = site + 5 + rebuilt.codeAt(site, 5).readInt32LE(1);
            return { site, target, op: row[1].slice(0, 2).toUpperCase() };
        });
    const transfers = directTransfers(rows);
    assert.equal(transfers.length, 1);
    const [transfer] = transfers;
    assert.equal(transfer.target, rebuiltRelease);
    const actionTarget = request.ContextStorageCleanup === 'Guard'
        ? mapAddress('??1ContextStorageOwner@@QAE@XZ') : rebuiltEntry;
    assert(directTransfers(actionRows).some(({ target }) => target === actionTarget));
    const deletingSymbol = '?destroy@SchedulerContext@@QAEPAU1@I@Z';
    const deletingEntry = map.includes(deletingSymbol)
        ? mapAddress(deletingSymbol) : probeAddress('ProbeDeletePooledContext');
    const deletingRows = readListing(deletingEntry, nextFunction(deletingEntry), true);
    assert(directTransfers(deletingRows).some(({ target }) => target === rebuiltRelease));
    const normalized = Buffer.from(rebuiltBody);
    originalBody.copy(normalized, transfer.site - rebuiltEntry + 1, 4, 8);
    const remainingByteDifferences = normalized.length === originalBody.length
        ? Array.from(originalBody.entries()).filter(([offset, value]) => normalized[offset] !== value)
            .map(([offset, value]) => ({ offset, original: value, rebuilt: normalized[offset] }))
        : null;
    const status = rebuiltBody.equals(originalBody) ? 'Exact'
        : normalized.equals(originalBody) ? 'Normalized' : 'Diagnostic';
    const report = {
        program: 'simscheduler.dll', entry: entry.toString(16), rebuiltEntry: rebuiltEntry.toString(16),
        wrapperSymbol, status, scope: 'Complete linked wrapper and verified release target in the factory construction projection',
        originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
        originalHex: originalBody.toString('hex'), rebuiltHex: rebuiltBody.toString('hex'), rebuiltListing: rows,
        remainingByteDifferences,
        reference: { originalSite: '20cc15f3', originalTarget: release.toString(16),
            rebuiltSite: transfer.site.toString(16), rebuiltTarget: rebuiltRelease.toString(16), opcode: transfer.op },
        action: { originalEntry: action.toString(16), originalHex: originalAction.toString('hex'),
            rebuiltEntry: rebuiltAction.toString(16), rebuiltTarget: actionTarget.toString(16), rebuiltListing: actionRows },
        release: { rebuiltEntry: rebuiltRelease.toString(16), rebuiltListing: releaseRows },
        deletingWrapper: { originalEntry: '20cc28c0', rebuiltEntry: deletingEntry.toString(16), rebuiltListing: deletingRows },
    };
    fs.writeFileSync(path.join(run, 'comparison-20cc15f0.json'), JSON.stringify(report, null, 2) + '\n');
    console.log(status + ' 20cc15f0: ' + originalBody.length + ' original bytes, ' + rebuiltBody.length + ' linked bytes');
    process.exit(0);
}
if (request.DispatcherProfile === 'ContextConstructorLtcg') {
    const evidence = require('../artifacts/simscheduler.dll/context-constructor-ltcg-ghidra.json');
    assert.equal(evidence.program, request.Module);
    assert.equal(sha(originalBytes), request.OriginalSHA256.toLowerCase());
    const buildCommand = fs.readFileSync(path.join(run, 'build.cmd'));
    assert.equal(sha(buildCommand), request.BuildCommandSHA256.toLowerCase());
    const framePointerOption = buildCommand.toString('ascii').match(/\/Oy-?(?=\s)/)[0];
    const fastFp = buildCommand.includes('/fp:fast');
    const compiledSource = fs.readFileSync(path.join(run, 'worker-output/compiled-source/scheduler_object.cpp'));
    assert.equal(sha(compiledSource), request.SourceSHA256.toLowerCase());
    const directConstructor = buildCommand.includes('/DMATCH_DIRECT_CONTEXT_CONSTRUCTOR');
    const internalConstructor = buildCommand.includes('/DMATCH_INTERNAL_CONTEXT_CONSTRUCTOR');
    const compareMember = process.argv[3] === '--member-body';
    assert(Number(directConstructor) + Number(internalConstructor) + Number(compareMember) < 2);
    const owner = directConstructor || internalConstructor || compareMember
        ? '??0SchedulerContext@@QAE@PBD@Z' : '_ConstructSchedulerContext@8';
    const constructor = mapAddress(owner);
    let wrapper;
    if (compareMember) {
        wrapper = mapAddress('_ConstructSchedulerContext@8');
        const thunk = rebuilt.codeAt(wrapper, 11);
        assert.equal(thunk.subarray(0, 7).toString('hex'), '8bff558bec5de9');
        assert.equal(wrapper + thunk.length + thunk.readInt32LE(7), constructor);
    }
    const verifyExport = (symbol, entry) => {
        const optionalHeader = rebuiltBytes.readUInt32LE(0x3c) + 24;
        const directoryRva = rebuiltBytes.readUInt32LE(optionalHeader + 96);
        const directorySize = rebuiltBytes.readUInt32LE(optionalHeader + 100);
        assert(directoryRva && directorySize >= 40);
        const directory = rebuilt.codeAt(rebuilt.base + directoryRva, 40, false);
        const exportTable = rebuilt.base + directory.readUInt32LE(28);
        const nameTable = rebuilt.base + directory.readUInt32LE(32);
        const ordinals = rebuilt.base + directory.readUInt32LE(36);
        const count = directory.readUInt32LE(24);
        const expectedName = Buffer.from(symbol + '\0');
        let matches = 0;
        for (let index = 0; index < count; ++index) {
            const nameRva = rebuilt.codeAt(nameTable + index * 4, 4, false).readUInt32LE();
            if (!rebuilt.codeAt(rebuilt.base + nameRva, expectedName.length, false).equals(expectedName)) continue;
            const ordinal = rebuilt.codeAt(ordinals + index * 2, 2, false).readUInt16LE();
            assert(ordinal < directory.readUInt32LE(20));
            const entryRva = rebuilt.codeAt(exportTable + ordinal * 4, 4, false).readUInt32LE();
            assert(entryRva < directoryRva || entryRva >= directoryRva + directorySize);
            assert.equal(rebuilt.base + entryRva, entry);
            ++matches;
        }
        assert.equal(matches, 1);
    };
    if (directConstructor) verifyExport(owner, constructor);
    let callerCheck;
    if (internalConstructor) {
        const caller = mapAddress('_InitializeBorrowedContext@12');
        verifyExport('_InitializeBorrowedContext@12', caller);
        assert.equal(decodeCall(original, 0x20cc1745), 0x20cc27a0);
        assert.equal(original.codeAt(0x20cc1743, 2).toString('hex'), '5150');
        const rows = readListing(caller, mapAddress('@__security_check_cookie@4'), true);
        const directCalls = rows.filter(row => row[1].startsWith('E8 '));
        assert.equal(directCalls.length, 1);
        const call = parseInt(directCalls[0][0], 16);
        assert.equal(decodeCall(rebuilt, call), constructor);
        const callIndex = rows.indexOf(directCalls[0]);
        assert.equal(rows[callIndex - 6][2], 'mov eax,dword ptr [ebp+0Ch]');
        assert.deepEqual(rows.slice(callIndex - 3, callIndex).map(row => row[2]),
            ['mov ecx,dword ptr [ebp+10h]', 'push ecx', 'push eax']);
        callerCheck = { originalSite: '20cc1745', rebuiltSite: call.toString(16),
            caller: caller.toString(16), stackArguments: ['storage', 'name'] };
    }
    const helper = mapAddress('_InitializeContextTimeValue@12');
    const nameIat = mapAddress('__imp__CopySchedulerContextName@12');
    const originalImports = readPE32Imports(originalBytes);
    const rebuiltImports = readPE32Imports(rebuiltBytes);
    assert.deepEqual(rebuiltImports.get(nameIat), originalImports.get(0x20cc10f4));
    assert.deepEqual(rebuiltImports.get(mapAddress('__imp__ReleaseSchedulerContainerStorage@4')),
        originalImports.get(0x20cc10d8));
    const handler = mapAddress('__ehhandler$' + owner);
    const borrowedStorage = buildCommand.includes('/DMATCH_BORROWED_CONTEXT_STORAGE');
    const earlyName = buildCommand.includes('/DMATCH_EARLY_CONTEXT_NAME');
    const readUnwindMap = (image, address) => {
        const handlerBytes = image.codeAt(address, 27);
        assert.equal(handlerBytes[17], 0xb8);
        assert.deepEqual(image.highLowFixups(address, 27), [18]);
        const info = image.codeAt(handlerBytes.readUInt32LE(18), 36, false);
        assert.equal(info.readUInt32LE(), 0x19930522);
        const count = info.readUInt32LE(4);
        assert(count >= 3 && count <= 4);
        const mapAddress = info.readUInt32LE(8);
        const table = image.codeAt(mapAddress, count * 8, false);
        assert.deepEqual(image.highLowFixups(mapAddress, count * 8),
            Array.from({length: count}, (_, i) => i * 8 + 4));
        return Array.from({length: count}, (_, i) => {
            const next = table.readInt32LE(i * 8);
            assert.equal(next, i - 1);
            return {next, action: table.readUInt32LE(i * 8 + 4).toString(16)};
        });
    };
    const originalUnwind = readUnwindMap(original, 0x20cc70b4);
    const rebuiltUnwind = readUnwindMap(rebuilt, handler);
    assert.deepEqual(originalUnwind.map(state => state.action), ['20cc7090', '20cc709b', '20cc70a6']);
    assert.equal(rebuiltUnwind.length, borrowedStorage ? 3 : 4);
    for (let i = 0; i < rebuiltUnwind.length; ++i) {
        assert.equal(parseInt(rebuiltUnwind[i].action, 16),
            mapAddress('__unwindfunclet$' + owner + '$' + i));
    }
    const cleanupTargets = [
        ['8b750883c660', 'SchedulerLiveEntryList'],
        ['8b750883c670', 'SchedulerEntryIndex'],
        ['8b750881c680000000', 'SchedulerPendingQueue'],
    ];
    const cleanupActions = cleanupTargets.map(([prefix, member], i) => {
        const length = prefix.length / 2;
        const originalAction = parseInt(originalUnwind[i].action, 16);
        const rebuiltAction = parseInt(rebuiltUnwind[i + (borrowedStorage ? 0 : 1)].action, 16);
        assert.equal(original.codeAt(originalAction, length).toString('hex'), prefix);
        const contextOffset = directConstructor ? 'f0' : framePointerOption === '/Oy-' ? '08' : '04';
        assert.equal(rebuilt.codeAt(rebuiltAction, length).toString('hex'),
            '8b75' + contextOffset + prefix.slice(6));
        const jumpTarget = (image, address) => {
            const jump = image.codeAt(address + length, 5);
            assert.equal(jump[0], 0xe9);
            return address + length + 5 + jump.readInt32LE(1);
        };
        assert.equal(jumpTarget(original, originalAction), 0x20cc28f0);
        const target = '??1' + member + '@@QAE@XZ';
        assert.equal(jumpTarget(rebuilt, rebuiltAction), mapAddress(target));
        if (internalConstructor && !fastFp) {
            const evidence = require('../artifacts/simscheduler.dll/context-constructor-linked-funclets-ghidra.json');
            const entry = originalAction.toString(16);
            const left = original.codeAt(originalAction, length + 5);
            const right = rebuilt.codeAt(rebuiltAction, length + 5);
            assert.equal(evidence[entry].program, request.Module);
            assert.equal(evidence[entry].instructions.map(row => row.bytes).join(''), left.toString('hex'));
            assert.deepEqual(original.highLowFixups(originalAction, left.length), []);
            assert.deepEqual(rebuilt.highLowFixups(rebuiltAction, right.length), []);
            // Verify both callees use ESI, check the same ownership fields,
            // clear a positive count, then release and reset the buffer.
            const cleanup = '66 83 4e 0a 01 c7 46 0c 00 00 00 00 c7 46 04 00 00 00 00 8b e5 5d c3'.replaceAll(' ', '');
            const checks = [
                [original, 0x20cc28f0, 71, originalImports, '8bff558bec518b4e0c85c97436f6460a017430837e0400742a8b0685c07e0233c089068d45fc50894dfcff15'],
                [rebuilt, mapAddress(target), 72, rebuiltImports, '8bff558bec518b460c85c07437f6460a017431837e0400742b833e008945fc7e06c706000000008d45fc50ff15'],
            ];
            const destinations = checks.map(([image, address, size, imports, prefix]) => {
                const body = image.codeAt(address, size);
                const operand = prefix.length / 2;
                assert.deepEqual(image.highLowFixups(address, size), [operand]);
                assert.equal(body.subarray(0, operand).toString('hex'), prefix);
                assert.equal(body.subarray(operand + 4).toString('hex'), cleanup);
                return imports.get(body.readUInt32LE(operand));
            });
            assert.deepEqual(destinations, [{dll: 'api.dll', name: '#108'}, {dll: 'api.dll', name: '#108'}]);
            const rows = readListing(rebuiltAction, rebuiltAction + right.length);
            assert.equal(rows.map(row => row[1].replaceAll(' ', '').toLowerCase()).join(''), right.toString('hex'));
            const report = {program: request.Module, entry, status: 'Normalized',
                scope: 'Complete linked VC8 constructor unwind action. The sole REL32 destination has the same ESI input and container cleanup behavior.',
                originalHex: left.toString('hex'), rebuiltHex: right.toString('hex'),
                rebuilt: rows, mappedOffsets: [length + 1],
                originalTarget: '20cc28f0', rebuiltTarget: target};
            fs.writeFileSync(path.join(run, 'comparison-' + entry + '.json'), JSON.stringify(report, null, 2) + '\n');
            console.log('Normalized ' + entry + ' (' + left.length + ' linked bytes)');
        }
        return {member, originalAction: originalAction.toString(16),
            rebuiltAction: rebuiltAction.toString(16), rebuiltTarget: target};
    });
    let placementDelete;
    if (!borrowedStorage) {
        placementDelete = mapAddress('??3@YAXPAX0@Z');
        assert.equal(rebuilt.codeAt(placementDelete, 1)[0], 0xc3);
        const placementAction = mapAddress('__unwindfunclet$_ConstructSchedulerContext@8$0');
        const placementListing = readListing(placementAction, mapAddress('__unwindfunclet$_ConstructSchedulerContext@8$1'));
        const last = placementListing.at(-1);
        const targetInstruction = Number.parseInt(last[0], 16);
        if (last[2].startsWith('jmp ')) {
            assert.equal(rebuilt.codeAt(targetInstruction, 1)[0], 0xe9);
            assert.equal(targetInstruction + 5 + rebuilt.codeAt(targetInstruction + 1, 4).readInt32LE(), placementDelete);
        } else {
            const call = placementListing.find(row => row[1].startsWith('E8 '));
            assert.equal(decodeCall(rebuilt, Number.parseInt(call[0], 16)), placementDelete);
            assert.equal(last[2], 'ret');
        }
    }
    const fields = [
        [8, 'DIR32', 0x20cc70b4, handler, 'constructor EH handler'],
        [25, 'DIR32', 0x20cc8008, mapAddress('___security_cookie'), 'security cookie'],
        [48, 'DIR32', 0x20cc1200, mapAddress('_schedulerContextTable'), 'context table address'],
        [161, 'DIR32', 0x20cc10f4, nameIat, 'API name copy IAT'],
        [200, 'DIR32', 0x20cc8370, mapAddress('_schedulerObject'), 'scheduler object'],
        [209, 'DIR32', 0x20cc8370, mapAddress('_schedulerObject'), 'scheduler object'],
        [238, 'REL32', 0x20cc15d0, helper, 'time value helper'],
    ];
    const linkedScope = 'Linked VC8 LTCG probe with individual target verification';
    let constructorScope = linkedScope;
    if (internalConstructor) {
        constructorScope = 'Internal VC8 LTCG constructor with a verified direct call from the projected caller, stack arguments, ret 8 and individually verified address operands.';
    }
    if (directConstructor) {
        constructorScope = 'Directly exported VC8 LTCG member. VC8 emits ECX this and ret 4, which differ from the original stack this and ret 8.';
    }
    if (compareMember) {
        constructorScope = 'Separate VC8 LTCG member body reached by a verified tail jump from the exported wrapper. The wrapper and member together differ from the original entry.';
    }
    if (fastFp) constructorScope += ' The time helper specializes the zero value and takes no stack argument.';
    for (const [entry, rebuiltEntry, end] of [
        [0x20cc27a0, constructor, compareMember ? wrapper : mapAddress('@__security_check_cookie@4')],
        [0x20cc15d0, helper, placementDelete ?? constructor],
    ]) {
        const address = entry.toString(16);
        const originalHex = evidence.functions[address].asm.map(row => row[1]).join('');
        const left = original.codeAt(entry, originalHex.length / 2);
        assert.equal(left.toString('hex'), originalHex);
        const rows = readListing(rebuiltEntry, end, true);
        assert(rows.length > 0);
        const right = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
        const exportedThiscall = directConstructor && entry === 0x20cc27a0;
        const returnInstruction = rows.at(-1)[2];
        if (fastFp && entry === 0x20cc15d0) {
            assert.equal(right.toString('hex'), 'd9eedd18c3');
        } else {
            assert.match(returnInstruction, exportedThiscall ? /^ret 4h?$/i : /^ret 8h?$/i);
        }
        if (exportedThiscall) assert(rows.some(row => row[2] === 'mov esi,ecx'));
        if (internalConstructor && entry === 0x20cc27a0) {
            assert(rows.some(row => row[2] === 'mov esi,dword ptr [ebp+8]'));
            assert(rows.some(row => row[2] === 'mov ecx,dword ptr [ebp+0Ch]'));
            assert(!rows.some(row => row[2].startsWith('mov dword ptr [ebp+8],')));
        }
        const leftFixups = original.highLowFixups(entry, left.length);
        const rightFixups = rebuilt.highLowFixups(rebuiltEntry, right.length);
        const references = [];
        if (entry === 0x20cc27a0) {
            assert.deepEqual(leftFixups, fields.filter(field => field[1] === 'DIR32').map(field => field[0]));
            const candidates = rightFixups.slice();
            const helperCalls = rows.filter(row => row[1].startsWith('E8 '));
            assert.equal(helperCalls.length, 1);
            const callIndex = rows.indexOf(helperCalls[0]);
            if (fastFp) {
                assert.match(rows[callIndex - 1][2], /^lea eax,/);
            } else {
                assert.match(rows[callIndex - 2][2], /^lea eax,/);
                assert.equal(rows[callIndex - 1][2], 'fstp qword ptr [esp]');
            }
            for (const [offset, kind, oldTarget, newTarget, identity] of fields) {
                let rebuiltOffset;
                if (kind === 'DIR32') {
                    assert.equal(left.readUInt32LE(offset), oldTarget);
                    const index = candidates.findIndex(pos => right.readUInt32LE(pos) === newTarget);
                    assert(index >= 0, identity);
                    [rebuiltOffset] = candidates.splice(index, 1);
                } else {
                    const site = Number.parseInt(helperCalls[0][0], 16);
                    assert.equal(decodeCall(original, entry + offset - 1), oldTarget);
                    assert.equal(decodeCall(rebuilt, site), newTarget);
                    rebuiltOffset = site + 1 - rebuiltEntry;
                }
                references.push({offset, rebuiltOffset, kind, identity,
                    originalTarget: oldTarget.toString(16), rebuiltTarget: newTarget.toString(16)});
            }
            assert.equal(candidates.length, 0);
        } else {
            assert.deepEqual(leftFixups, []);
            assert.deepEqual(rightFixups, []);
            assert(rows.some(row => row[1] === 'DD 18' && row[2] === 'fstp qword ptr [eax]'));
        }
        const comparable = Buffer.from(right);
        const mappedOffsets = [];
        for (const reference of references) {
            if (reference.offset === reference.rebuiltOffset) {
                left.copy(comparable, reference.offset, reference.offset, reference.offset + 4);
                mappedOffsets.push(reference.offset);
            }
        }
        const status = !compareMember && !exportedThiscall && references.length === mappedOffsets.length && comparable.equals(left)
            ? (right.equals(left) ? 'Exact' : 'Normalized') : 'Diagnostic';
        const prefixLeft = Buffer.from(left);
        const prefixRight = Buffer.from(right);
        for (const reference of references) {
            prefixLeft.fill(0, reference.offset, reference.offset + 4);
            prefixRight.fill(0, reference.rebuiltOffset, reference.rebuiltOffset + 4);
        }
        const limit = Math.min(left.length, right.length);
        let commonPrefixBytes = 0;
        let commonSuffixBytes = 0;
        while (commonPrefixBytes < limit && prefixLeft[commonPrefixBytes] === prefixRight[commonPrefixBytes]) {
            commonPrefixBytes++;
        }
        while (commonSuffixBytes < limit - commonPrefixBytes &&
            prefixLeft[left.length - 1 - commonSuffixBytes] === prefixRight[right.length - 1 - commonSuffixBytes]) {
            commonSuffixBytes++;
        }
        const report = {
            program: request.Module, entry: address, rebuiltEntry: rebuiltEntry.toString(16), status,
            scope: entry === 0x20cc27a0 ? constructorScope : linkedScope,
            originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
            originalHex, rebuiltHex: right.toString('hex'), rebuilt: rows, references, mappedOffsets,
            framePointerOption, commonPrefixBytes, commonSuffixBytes,
            placementDeleteHex: placementDelete ? 'c3' : null,
            ...(borrowedStorage || earlyName ? {
                originalUnwind, rebuiltUnwind, cleanupActions, borrowedStorage, earlyName,
            } : {}),
            ...(compareMember ? { wrapper: wrapper.toString(16), owner } : {}),
            ...(directConstructor ? { directConstructor, owner } : {}),
            ...(internalConstructor ? { internalConstructor, owner, callerCheck } : {}),
            ...(fastFp ? { fastFp } : {}),
        };
        fs.writeFileSync(path.join(run, 'comparison-' + address + '.json'), JSON.stringify(report, null, 2) + '\n');
        console.log(status + ' ' + address + ': ' + left.length + ' original bytes, ' + right.length + ' linked bytes');
    }
    if (buildCommand.includes('/EXPORT:_SetSchedulerUpdateEnabled@8')) {
        const entry = 0x20cc2c40;
        const start = mapAddress('_SetSchedulerUpdateEnabled@8');
        const helperEntry = mapAddress(map.includes('@PropagateSchedulerTime@16')
            ? '@PropagateSchedulerTime@16' : '_PropagateSchedulerTime@12');
        const left = original.codeAt(entry, 50);
        const rows = readListing(start, mapAddress('@__security_check_cookie@4'), true);
        const right = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
        const live = require('../artifacts/simscheduler.dll/context-update-setter-ghidra.json');
        assert.equal(live.program, request.Module);
        assert.equal(live.start_address, entry.toString(16));
        assert.equal(live.instructions.map(row => row.bytes).join(''), left.toString('hex'));
        assert.equal(original.codeAt(0x20cc122c, 4, false).readUInt32LE(), entry);
        assert.deepEqual(original.highLowFixups(0x20cc122c, 4), [0]);
        assert.deepEqual(original.highLowFixups(entry, left.length), []);
        assert.deepEqual(rebuilt.highLowFixups(start, right.length), []);
        const calls = rows.filter(row => row[1].startsWith('E8 '));
        assert.equal(calls.length, 1);
        const site = parseInt(calls[0][0], 16);
        assert.equal(decodeCall(original, 0x20cc2c67), 0x20cc2fa0);
        assert.equal(decodeCall(rebuilt, site), helperEntry);
        assert.equal(right.subarray(-3).toString('hex'), 'c20800');
        const mapped = Buffer.from(right);
        const offset = site - start + 1;
        if (offset === 40) left.copy(mapped, offset, offset, offset + 4);
        const status = mapped.equals(left) ? 'Normalized' : 'Diagnostic';
        fs.writeFileSync(path.join(run, 'comparison-20cc2c40.json'), JSON.stringify({
            program: request.Module, entry: entry.toString(16), rebuiltEntry: start.toString(16),
            status, scope: 'Complete linked update setter with the original VTable slot and both propagation call targets verified.',
            originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
            originalHex: left.toString('hex'), rebuiltHex: right.toString('hex'), rebuilt: rows,
            references: [{offset: 40, rebuiltOffset: offset, kind: 'REL32', originalTarget: '20cc2fa0',
                rebuiltTarget: helperEntry.toString(16)}],
        }, null, 2) + '\n');
        console.log(status + ' 20cc2c40: ' + left.length + ' original bytes, ' + right.length + ' linked bytes');

        const helperVA = 0x20cc2fa0;
        const helperEvidence = require('../artifacts/simscheduler.dll/context-update-register-abi-ghidra.json');
        const originalHelper = original.codeAt(helperVA, 72);
        assert.equal(helperEvidence.program, request.Module);
        assert.equal(helperEvidence.helperByteListing.start_address, helperVA.toString(16));
        assert.equal(helperEvidence.helperByteListing.instructions.map(row => row.bytes).join(''), originalHelper.toString('hex'));
        const helperRows = readListing(helperEntry, helperEntry + 128, true);
        const rebuiltHelper = Buffer.from(helperRows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
        assert.deepEqual(original.highLowFixups(helperVA, originalHelper.length), []);
        assert.deepEqual(rebuilt.highLowFixups(helperEntry, rebuiltHelper.length), []);
        assert.equal(rebuiltHelper.subarray(-3).toString('hex'), 'c20800');
        const input = helperRows.find(row => /\[(ecx|edx)\+60h\]/.test(row[2]));
        assert(input, 'Missing live-entry count access');
        const contextRegister = input[2].match(/\[(ecx|edx)\+60h\]/)[1].toUpperCase();
        const helperStatus = rebuiltHelper.equals(originalHelper) ? 'Exact' : 'Diagnostic';
        const report = {
            program: request.Module, entry: helperVA.toString(16), rebuiltEntry: helperEntry.toString(16),
            status: helperStatus, scope: 'Complete linked propagation helper. No address operands in either body.',
            originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
            originalHex: originalHelper.toString('hex'), rebuiltHex: rebuiltHelper.toString('hex'),
            rebuilt: helperRows, contextRegister,
        };
        fs.writeFileSync(path.join(run, 'comparison-20cc2fa0.json'), JSON.stringify(report, null, 2) + '\n');
        console.log(helperStatus + ' 20cc2fa0: ' + originalHelper.length + ' original bytes, ' + rebuiltHelper.length + ' linked bytes (' + contextRegister + ')');
    }
    if (internalConstructor && !fastFp) {
        const handlerEvidence = require('../artifacts/simscheduler.dll/context-constructor-handler-ghidra.json');
        const entry = 0x20cc70b4;
        const left = original.codeAt(entry, 27);
        const right = rebuilt.codeAt(handler, 27);
        assert.equal(handlerEvidence.program, request.Module);
        assert.equal(handlerEvidence.listing.start_address, entry.toString(16));
        assert.equal(handlerEvidence.memory.hex, left.toString('hex'));
        assert.equal(handlerEvidence.listing.instructions.map(row => row.bytes).join(''), left.toString('hex'));
        const rows = readListing(handler, handler + right.length);
        assert.equal(rows.map(row => row[1].replaceAll(' ', '').toLowerCase()).join(''), right.toString('hex'));
        const cookieCheck = mapAddress('@__security_check_cookie@4');
        const crtThunk = mapAddress('___CxxFrameHandler3');
        const references = [
            {offset: 13, kind: 'REL32', originalTarget: 0x20cc69ae, rebuiltTarget: cookieCheck, identity: 'stack cookie check'},
            {offset: 18, kind: 'DIR32', originalTarget: 0x20cc73f0, rebuiltTarget: right.readUInt32LE(18), identity: 'constructor FuncInfo'},
            {offset: 23, kind: 'REL32', originalTarget: 0x20cc6f50, rebuiltTarget: crtThunk, identity: '__CxxFrameHandler3 import thunk'},
        ];
        const decodeJump = (image, address) => {
            const code = image.codeAt(address, 5);
            assert.equal(code[0], 0xe9);
            return address + 5 + code.readInt32LE(1);
        };
        assert.equal(decodeCall(original, entry + 12), references[0].originalTarget);
        assert.equal(decodeCall(rebuilt, handler + 12), cookieCheck);
        assert.equal(left[17], 0xb8);
        assert.equal(right[17], 0xb8);
        assert.equal(left.readUInt32LE(18), references[1].originalTarget);
        assert.deepEqual(original.highLowFixups(entry, 27), [18]);
        assert.deepEqual(rebuilt.highLowFixups(handler, 27), [18]);
        // The FuncInfo owner and its three unwind actions were checked above.
        assert.equal(decodeJump(original, entry + 22), references[2].originalTarget);
        assert.equal(decodeJump(rebuilt, handler + 22), crtThunk);
        const verifyCookie = (image, start, cookie, failure) => {
            const code = image.codeAt(start, 15);
            assert.equal(code.readUInt16LE(0), 0x0d3b);
            assert.equal(code.readUInt32LE(2), cookie);
            assert.equal(code.subarray(6, 10).toString('hex'), '7502f3c3');
            assert.equal(decodeJump(image, start + 10), failure);
            assert.deepEqual(image.highLowFixups(start, 15), [2]);
        };
        verifyCookie(original, references[0].originalTarget, 0x20cc8008, 0x20cc6d27);
        verifyCookie(rebuilt, cookieCheck, mapAddress('___security_cookie'), mapAddress('___report_gsfailure'));
        const verifyCrtThunk = (image, imports, address) => {
            const bytes = image.codeAt(address, 6);
            assert.equal(bytes.readUInt16LE(0), 0x25ff);
            assert.deepEqual(image.highLowFixups(address, 6), [2]);
            assert.deepEqual(imports.get(bytes.readUInt32LE(2)), {dll: 'msvcr80.dll', name: '__CxxFrameHandler3'});
        };
        verifyCrtThunk(original, originalImports, references[2].originalTarget);
        verifyCrtThunk(rebuilt, rebuiltImports, crtThunk);
        const mapped = Buffer.from(right);
        for (const {offset} of references) left.copy(mapped, offset, offset, offset + 4);
        assert(mapped.equals(left));
        const report = {
            program: request.Module, entry: entry.toString(16), rebuiltEntry: handler.toString(16), status: 'Normalized',
            scope: 'Complete linked VC8 x86 exception handler. Three individually checked address operands. FuncInfo and cleanup bodies are outside the byte match.',
            originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
            originalHex: left.toString('hex'), rebuiltHex: right.toString('hex'), rebuilt: rows,
            references, mappedOffsets: references.map(ref => ref.offset),
        };
        fs.writeFileSync(path.join(run, 'comparison-' + report.entry + '.json'), JSON.stringify(report, null, 2) + '\n');
        console.log('Normalized ' + report.entry + ' (' + left.length + ' linked bytes)');
    }
    process.exit(0);
}
if (request.DispatcherProfile === 'EntryInsertionLtcg') {
    const rebuiltNames = [
        [0x20cc3f50, 239, '?ResizeLiveSchedulerEntries@@YAHPAUSchedulerLiveEntryList@@H@Z'],
        [0x20cc40d0, 239, '?ResizeSchedulerEntryIndex@@YAHPAUSchedulerEntryIndex@@H@Z'],
        [0x20cc3bd0, 72, '?AppendSchedulerEntry@@YGXPAUSchedulerLiveEntryList@@PBQAUSchedulerTimedEntry@@@Z'],
        [0x20cc3b00, 106, '?InsertSchedulerEntry@@YGXPAUSchedulerLiveEntryList@@HPBQAUSchedulerTimedEntry@@@Z'],
        [0x20cc42c0, 86, '?AppendSchedulerIndexSlot@@YAPAUSchedulerEntryIndexRecord@@PAUSchedulerEntryIndex@@@Z'],
        [0x20cc4320, 88, '?InsertSchedulerIndexSlot@@YAPAUSchedulerEntryIndexRecord@@PAUSchedulerEntryIndex@@I@Z'],
        [0x20cc3c20, 94, '?InsertSchedulerEntryIndex@@YGHPAUSchedulerEntryIndex@@PBQAXPBQAUSchedulerTimedEntry@@@Z'],
    ];
    const starts = rebuiltNames.map(([, , symbol]) => mapAddress(symbol));
    const rebuiltTargets = new Map(rebuiltNames.map(([entry], i) => [entry, starts[i]]));
    rebuiltTargets.set(0x20cc4050, mapAddress('?FindSchedulerEntry@@YA_NPBUSchedulerEntryIndex@@PBQAXPAPAUSchedulerEntryIndexRecord@@@Z'));
    rebuiltTargets.set(0x20cc6e86, mapAddress('_memcpy'));
    const originalImports = readPE32Imports(originalBytes);
    const rebuiltImports = readPE32Imports(rebuiltBytes);
    const imports = new Map([
        [0x20cc10dc, '__imp__AllocateSchedulerContainerStorage@16'],
        [0x20cc10d8, '__imp__ReleaseSchedulerContainerStorage@4'],
        [0x20cc10e8, '__imp__MovePendingStorage@12'],
    ].map(([va, symbol]) => {
        const iat = mapAddress(symbol);
        assert.deepEqual(rebuiltImports.get(iat), originalImports.get(va));
        return [va, iat];
    }));
    for (const [image, thunk, table] of [
        [original, 0x20cc6e86, originalImports], [rebuilt, rebuiltTargets.get(0x20cc6e86), rebuiltImports],
    ]) {
        const jump = image.codeAt(thunk, 6);
        assert.equal(jump.readUInt16LE(), 0x25ff);
        assert.deepEqual(table.get(jump.readUInt32LE(2)), { dll: 'msvcr80.dll', name: 'memcpy' });
    }
    const callSites = [
        [[0x20cc400d, 0x20cc6e86]], [[0x20cc418d, 0x20cc6e86]],
        [[0x20cc3bf8, 0x20cc3f50]], [[0x20cc3b29, 0x20cc3f50]],
        [[0x20cc4302, 0x20cc40d0]], [[0x20cc4346, 0x20cc40d0]],
        [[0x20cc3c34, 0x20cc42c0], [0x20cc3c42, 0x20cc4050], [0x20cc3c61, 0x20cc4320]],
    ];
    const transfers = [
        [0x20cc10dc, 0x20cc10d8], [0x20cc10dc, 0x20cc10d8],
        [], [0x20cc10e8], [], [0x20cc10e8], [],
    ];
    for (const [i, [entry, size]] of rebuiltNames.entries()) {
        const start = starts[i];
        const rows = readListing(start, starts[i + 1] || mapAddress('_ProbeAppendSchedulerEntry@8'));
        const originalBody = original.codeAt(entry, size);
        const rebuiltBody = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
        assert.match(rows[rows.length - 1][2], /^ret(?: [0-9a-f]+h?)?$/i);
        const calls = rows.filter(row => row[1].startsWith('E8 '));
        assert.equal(calls.length, callSites[i].length);
        const references = callSites[i].map(([site, target], j) => {
            const rebuiltSite = Number.parseInt(calls[j][0], 16);
            assert.equal(decodeCall(original, site), target);
            assert.equal(decodeCall(rebuilt, rebuiltSite), rebuiltTargets.get(target));
            return { originalSite: site.toString(16), originalTarget: target.toString(16),
                rebuiltSite: rebuiltSite.toString(16), rebuiltTarget: rebuiltTargets.get(target).toString(16) };
        });
        const iatCalls = rows.filter(row => row[1].startsWith('FF 15 '));
        assert.equal(iatCalls.length, transfers[i].length);
        transfers[i].forEach((va, j) => {
            const address = Number.parseInt(iatCalls[j][0], 16);
            assert.equal(rebuilt.codeAt(address, 6).readUInt32LE(2), imports.get(va));
        });
        const report = {
            program: 'simscheduler.dll', entry: entry.toString(16), rebuiltEntry: start.toString(16),
            status: 'Diagnostic', scope: 'Complete linked C++ insertion helper with checked direct calls and import identities',
            originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
            originalHex: originalBody.toString('hex'), rebuiltHex: rebuiltBody.toString('hex'),
            rebuiltListing: rows, references, transfers: transfers[i].map(va => originalImports.get(va)),
        };
        fs.writeFileSync(path.join(run, 'comparison-' + report.entry + '.json'), JSON.stringify(report, null, 2) + '\n');
        console.log('Diagnostic ' + report.entry + ': ' + size + ' original bytes, ' + rebuiltBody.length + ' linked bytes');
    }
    process.exit(0);
}
if (request.DispatcherProfile === 'EntryHelpersLtcg') {
    const lookup = mapAddress('?FindSchedulerEntry@@YA_NPBUSchedulerEntryIndex@@PBQAXPAPAUSchedulerEntryIndexRecord@@@Z');
    const removalSymbol = '?RemoveSchedulerEntryReferences@@YAHPAUSchedulerLiveEntryList@@PBQAUSchedulerTimedEntry@@@Z';
    const removeEntries = mapAddress(map.includes(removalSymbol) ? removalSymbol
        : '?RemoveSchedulerEntryReferences@@YAHPAUSchedulerContext@@PBQAUSchedulerTimedEntry@@@Z');
    const removeIndex = mapAddress('?RemoveSchedulerIndexRecord@@YAHPAUSchedulerEntryIndex@@PBQAX@Z');
    const probes = ['_ProbeSchedulerEntryLookup@12', '_ProbeSchedulerEntryRemoval@8', '_ProbeSchedulerIndexRemoval@8']
        .map(mapAddress);
    const originalImport = readPE32Imports(originalBytes).get(0x20cc10e8);
    const rebuiltIat = mapAddress('__imp__MovePendingStorage@12');
    assert.deepEqual(originalImport, { dll: 'api.dll', name: '#110' });
    assert.deepEqual(readPE32Imports(rebuiltBytes).get(rebuiltIat), originalImport);
    assert.equal(decodeCall(original, 0x20cc3ca4), 0x20cc4050);
    for (const [entry, hex, start, end, expectedDirect, expectedImport] of [
        [0x20cc4050, '8bff558bec5156578b780c8b0033c985c08945fc8bf076268b4508538b188bff8bc62bc1d1e803c18b14c73bda76058d4801eb028bf03bce72e68b45fc5b8b550c85d28d3ccf7402893a3bc873178b4d088b018b0f3bc1720c770a5fb0015e8be55dc208005f32c05e8be55dc20800', lookup, removeEntries, false, false],
        [0x20cc3b70, '8bff558bec5153578b3e33db83ef0178358b08894dfc8b560c8b4dfc390cba8d04ba751d8b162bd78d0c95fcffffff518d50045250ff15e810cc208306ff83c30183ef0179d05f8bc35b8be55dc3', removeEntries, removeIndex, false, true],
        [0x20cc3c90, '8bff558bec51568b3785f674408d45fc50518bc7e8a703000084c074308b4f0c8b45fc2bc1c1f8032bf08d0cc18d14f5f8ffffff528d41085051ff15e810cc208307ffb8010000005e8be55dc333c05e8be55dc3', removeIndex, Math.min(...probes), true, true],
    ]) {
        const originalBody = original.codeAt(entry, hex.length / 2);
        assert.equal(originalBody.toString('hex'), hex);
        const rows = readListing(start, end);
        const rebuiltBody = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
        assert.match(rows[rows.length - 1][2], /^ret(?: [0-9a-f]+h)?$/i);
        const calls = rows.filter(row => row[2].startsWith('call '));
        const direct = calls.filter(row => row[1].startsWith('E8 '));
        assert.equal(direct.length, Number(expectedDirect));
        if (expectedDirect) {
            assert.equal(decodeCall(rebuilt, Number.parseInt(direct[0][0], 16)), lookup);
        }
        const indirect = calls.filter(row => row[1].startsWith('FF 15 '));
        assert.equal(indirect.length, Number(expectedImport));
        if (expectedImport) {
            const originalSite = entry === 0x20cc3b70 ? 0x20cc3ba5 : 0x20cc3cca;
            assert.equal(original.codeAt(originalSite, 6).readUInt32LE(2), 0x20cc10e8);
            assert.equal(rebuilt.codeAt(Number.parseInt(indirect[0][0], 16), 6).readUInt32LE(2), rebuiltIat);
        }
        let status = 'Diagnostic';
        const mappedOffsets = [];
        if (entry === 0x20cc3b70) {
            const operand = 0x20cc3ba5 + 2 - entry;
            const rebuiltOperand = parseInt(indirect[0][0], 16) + 2 - start;
            assert.deepEqual(original.highLowFixups(entry, originalBody.length), [operand]);
            assert.deepEqual(rebuilt.highLowFixups(start, rebuiltBody.length), [rebuiltOperand]);
            if (operand === rebuiltOperand) {
                const normalized = Buffer.from(rebuiltBody);
                originalBody.copy(normalized, operand, operand, operand + 4);
                if (normalized.equals(originalBody)) {
                    status = 'Normalized';
                    mappedOffsets.push(operand);
                }
            }
        }
        const report = {
            program: 'simscheduler.dll', entry: entry.toString(16), rebuiltEntry: start.toString(16),
            status, scope: 'Complete linked C++ entry helper with checked code and call identities',
            originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
            originalHex: hex, rebuiltHex: rebuiltBody.toString('hex'), rebuiltListing: rows,
            transferImport: expectedImport ? originalImport : null,
            ...(mappedOffsets.length ? { mappedOffsets } : {}),
            directTarget: expectedDirect ? { original: '20cc4050', rebuilt: lookup.toString(16) } : null,
        };
        fs.writeFileSync(path.join(run, 'comparison-' + report.entry + '.json'), JSON.stringify(report, null, 2) + '\n');
        console.log(report.status + ' ' + report.entry + ': ' + originalBody.length + ' original bytes, ' + rebuiltBody.length + ' linked bytes');
    }
    process.exit(0);
}
if (request.DispatcherProfile === 'PendingRegistration') {
    const resize = mapAddress('?ResizePendingSchedulerEntries@@YAHPAUSchedulerPendingQueue@@H@Z');
    const queue = mapAddress('?QueueSchedulerRegistration@@YGHPAUSchedulerContext@@PAXHHH@Z');
    const probe = mapAddress('_ProbeSchedulerRegistration@20');
    const memcpy = mapAddress('_memcpy');
    const oldImports = readPE32Imports(originalBytes);
    const newImports = readPE32Imports(rebuiltBytes);
    const transferImports = [
        [0x20cc4253, '__imp__AllocatePendingStorage@16', '#106'],
        [0x20cc429d, '__imp__ReleasePendingStorage@4', '#108'],
        [0x20cc34f8, '__imp__MovePendingStorage@12', '#110'],
    ];
    const verifiedImports = transferImports.map(([site, symbol, name]) => {
        const originalCall = original.codeAt(site, 6);
        assert.equal(originalCall.readUInt16LE(), 0x15ff);
        const originalIat = originalCall.readUInt32LE(2);
        const rebuiltIat = mapAddress(symbol);
        const imported = { dll: 'api.dll', name };
        assert.deepEqual(oldImports.get(originalIat), imported);
        assert.deepEqual(newImports.get(rebuiltIat), imported);
        return { site, rebuiltIat, imported };
    });
    for (const [image, thunk, imports] of [[original, 0x20cc6e86, oldImports], [rebuilt, memcpy, newImports]]) {
        const jump = image.codeAt(thunk, 6);
        assert.equal(jump.readUInt16LE(), 0x25ff);
        assert.deepEqual(imports.get(jump.readUInt32LE(2)), { dll: 'msvcr80.dll', name: 'memcpy' });
    }
    assert.equal(decodeCall(rebuilt, Number.parseInt(readListing(probe, memcpy)
        .find(row => row[1].startsWith('E8 '))[0], 16)), queue);
    for (const [entry, hex, start, end, site, oldTarget, newTarget] of [
        [0x20cc3490, '8bff558bec518b8a8000000056578dba8000000033c085c97e178bb28c0000008d5604391a741583c00183c2103bc17cf28bf183c601794633f6eb4f8b55108bc8c1e1043b1431750a5f33c05e8be55dc20c008b570c03ca8b172bd083ea01c1e204528d41105051ff15e810cc208307ff5fb8010000005e8be55dc20c0081feffffff077e05beffffff0785f68bc67d0433c0eb0d81feffffff077e05b8ffffff073b47047e0985c07c05e8900c00008b570c8b4d108937c1e6048d4416f08b550c89088b4d085f89580489500889480cb8010000005e8be55dc20c00', queue, probe, 0x20cc353b, 0x20cc41d0, resize],
        [0x20cc41d0, '8bff558bec83ec08538bd881fbffffff077e0733c05b8be55dc35633f63bde8975fc7e36663977087f0666c747080500f6470a0474120fbf77083bde7e1f8bff03f63bde7ffaeb150fbf4f088d4419ff99f7f98bf00faff1eb038975fc3b7704750b5eb8010000005b8be55dc385f67e22a16c83cc20508d4df8518bd6c1e204526a01ff15dc10cc208b45f885c08945fc74138b073bd87c028bd885db7e228b45fc85c075085e33c05b8be55dc38b570c8bcbc1e104515250e8f82b000083c40c8b470c8d4df851891f8945f8ff15d810cc208b55fc897704b8010000006609470a5e89570c5b8be55dc3', resize, queue, 0x20cc4289, 0x20cc6e86, memcpy],
    ]) {
        const originalBody = original.codeAt(entry, hex.length / 2);
        assert.equal(originalBody.toString('hex'), hex);
        const rows = readListing(start, end);
        const rebuiltBody = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
        assert.match(rows[rows.length - 1][2], /^ret(?: [0-9a-f]+h)?$/i);
        const calls = rows.filter(row => row[1].startsWith('E8 '));
        assert.equal(calls.length, 1);
        const linkedSite = Number.parseInt(calls[0][0], 16);
        assert.equal(decodeCall(original, site), oldTarget);
        assert.equal(decodeCall(rebuilt, linkedSite), newTarget);
        const imports = verifiedImports.filter(item => item.site >= entry && item.site < entry + originalBody.length);
        for (const item of imports) {
            assert(rows.some(row => {
                const bytes = Buffer.from(row[1].replaceAll(' ', ''), 'hex');
                return bytes.length === 6 && bytes.readUInt16LE() === 0x15ff && bytes.readUInt32LE(2) === item.rebuiltIat;
            }));
        }
        if (entry === 0x20cc41d0) {
            assert.equal(original.codeAt(0x20cc4241, 5).readUInt32LE(1), 0x20cc836c);
            const allocator = mapAddress('_schedulerPendingAllocator');
            assert(rows.some(row => {
                const bytes = Buffer.from(row[1].replaceAll(' ', ''), 'hex');
                return bytes.length >= 5 && bytes.readUInt32LE(bytes.length - 4) === allocator;
            }));
        }
        const report = {
            program: 'simscheduler.dll', entry: entry.toString(16), rebuiltEntry: start.toString(16),
            status: 'Diagnostic', scope: 'Complete linked C++ pending registration body, with API imports and direct call checked',
            originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
            originalHex: originalBody.toString('hex'), rebuiltHex: rebuiltBody.toString('hex'), imports,
            directCall: { originalSite: site.toString(16), originalTarget: oldTarget.toString(16),
                rebuiltSite: linkedSite.toString(16), rebuiltTarget: newTarget.toString(16) },
            rebuiltListing: rows,
        };
        fs.writeFileSync(path.join(run, 'comparison-' + report.entry + '.json'), JSON.stringify(report, null, 2) + '\n');
        console.log('Diagnostic ' + report.entry + ': ' + originalBody.length + ' original bytes, ' + rebuiltBody.length + ' linked bytes');
    }
    process.exit(0);
}
if (request.DispatcherProfile === 'ContextLtcg') {
    const split = mapAddress('?SplitSchedulerStep@@YGXPAUSchedulerContext@@N@Z');
    const context = mapAddress('?ProcessSchedulerContext@@YGHPAUSchedulerContext@@HHN@Z');
    const advance = mapAddress('_AdvanceSchedulerTime@12');
    const conversion = mapAddress('__ftol2_sse');
    const readTime = mapAddress('?ReadSchedulerTime@@YANXZ');
    const originalImports = readPE32Imports(originalBytes);
    const rebuiltImports = readPE32Imports(rebuiltBytes);
    const importChecks = [
        [0x20cc10ec, '__imp__schedulerApiState', { dll: 'api.dll', name: '#304' }],
        [0x20cc10f8, '__imp__DispatchSchedulerApiEvent@12', { dll: 'api.dll', name: '#233' }],
    ];
    for (const [originalIat, symbol, identity] of importChecks) {
        assert.deepEqual(originalImports.get(originalIat), identity);
        assert.deepEqual(rebuiltImports.get(mapAddress(symbol)), identity);
    }
    const functions = [
        [0x20cc3050, '8bff558becdd4508dd17dc5630dfe0f6c4417536dc7630e8343e000083c00189450cdb450c8903dc3fdc5628dfe0f6c4057a29ddd8dd07dc7628e8113e00008903dd4628dd1f5dc20800dc5e28dfe0f6c4057a0ed9eec70300000000dd1f5dc20800c703010000005dc20800', split, Math.min(readTime, context),
            [[0x20cc3067, 0x20cc6ea0, conversion], [0x20cc308a, 0x20cc6ea0, conversion]], 'ret 8'],
        [0x20cc30d0, '8bff558bec83e4c083ec7453568bf08b463ca801570f84860300008b566085d2895424480f84770300008b7d0885ff74188b4e38f6c102750b33c05f5e5b8be55dc21000f6c10174f0837d0c00740d8b4e38f6c10474e2f6c10174dd83c802f646380189463c0f841b01000085ff742b8b0d7083cc208b51208d44247050687083cc20ffd2dd45108b08894e448b5004895648dc6644dc464ceb06dd4510dc664483ec08dd5424688bc48d7e54dd188d5e5ce8c9feffffdb465c837d0800dc4e54751cdc4644dd5e44d9eedd5c24608b4424608b4c246489464c894e50eb16dc6c2460dd5c24608b5424608b44246489564c894650b8010000003b465c894424440f8f340200007d10db442444dc4e54dc4644dd5c2458eb0e8b4e448b5648894c24588954245c33ff397e607e488bff8b466c8b1cb8f60302752f8b430cdd46548b088b1183ec08dd1c2450ffd2837d08007516dd442458dd5c24608b4424608b4c2464894304894b0883c7013b7e607cbe8b44244483c0013b465c894424447e85e9b4010000dd4510dc6644d9eed8d1dfe0f6c4057b06ddd9d9c0d9c9dd4628d8dadfe0f6c4417512db442448decad9c9dc7628e8163c0000eb04ddd98bc285c00f8e71010000dd54245089442448ddd885ffdd45108b56408b466c8b04908944244c74308b157083cc20ddd88b42208d4c247851687083cc20ffd0dd464c8b08dc45108b5004894c246089542464dd442460eb07dd4004dd542460d8d1dfe0f6c4050f8abc000000dee983ec088bc48d7e54dd5424788d5e5cdd18e83efdffff837d08007504d9eeeb09db465cdc0fdc6c2470b801000000dd5c245839465c894424447c5e8b5c244cf6030275458b430cdd078b088b1183ec08dd1c2450ffd2837d080074258b0d7083cc208b51208d44246850687083cc20ffd28b4424688b4c246c894304894b08eb08dd4304dc07dd5b048b44244483c0013b465c894424447ea2dd442450dc5c2458dfe0f6c4057a168b5424588b44245c8954245089442454eb04ddd9ddd88b464083c00199f77e60836c2448018b7d088956400f85cdfeffff85ffd9ee75208b4d10dd5c24688b55148b442468894e448b4c246c89564889464c894e50eb108b5424508b44245489564c894650ddd883663cfd33db399e800000007e4033ffeb038d49008b868c00000003c783380074128b50088b40048b0e8b4938525056ffd1eb0c8b40048b168b4a3c5056ffd183c30183c7103b9e800000007cc7c7868000000000000000b8010000005f5e5b8be55dc210008b45148b55105f8946488956445eb8010000005b8be55dc21000', context, advance,
            [[0x20cc3182, 0x20cc3050, split], [0x20cc3285, 0x20cc6ea0, conversion], [0x20cc330d, 0x20cc3050, split]], /^ret (?:0Ch|10h)$/],
        [0x20cc1b90, '8bff558bec83e4c083ec3453568b75088b068b481c5756ffd18944242cdb44242ca1ec10cc2033d233dbdc4d0c399810010000895c24280f95c2395e30dc0dd812cc20dd542438dc46048954242cdd5c24307e748b4e3c8b440b208b10508b424cffd0dd4424308b54242c83ec088bf88bc4dd188b4e2451528bc7e8c014000085c07431393d6083cc207529833d6483cc200074208b078b482857ffd185c07414a16483cc20d9ee8b1083ec08dd1c24508b02ffd08b44242883c00183c3243b4630894424287c8c837c242c00751e8b4c24308b5424348b442438894e048b4c243c89560889460c894e10eb14d9eedd5c24388b5424388b44243c89560c894610dd450c6a006a00dd5e146a22ff15f810cc205f5e5b8be55dc20c00', advance, conversion,
            [[0x20cc1c0b, 0x20cc30d0, context]], 'ret 0Ch'],
    ];
    for (const [entry, hex, rebuiltEntry, end, targets, ret] of functions) {
        const originalBody = original.codeAt(entry, hex.length / 2);
        assert.equal(originalBody.toString('hex'), hex);
        const rows = readListing(rebuiltEntry, end, entry === 0x20cc1b90);
        const rebuiltBody = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
        if (ret instanceof RegExp) assert.match(rows[rows.length - 1][2], ret);
        else assert.equal(rows[rows.length - 1][2], ret);
        const calls = rows.filter(row => row[1].startsWith('E8 ')).map(row => Number.parseInt(row[0], 16))
            .map(site => ({ site, target: decodeCall(rebuilt, site) }));
        const references = targets.map(([site, originalTarget, rebuiltTarget]) => {
            assert.equal(decodeCall(original, site), originalTarget);
            const index = calls.findIndex(call => call.target === rebuiltTarget);
            assert(index >= 0, 'Missing rebuilt call to ' + rebuiltTarget.toString(16));
            const [call] = calls.splice(index, 1);
            return { originalSite: site.toString(16), originalTarget: originalTarget.toString(16),
                rebuiltSite: call.site.toString(16), rebuiltTarget: rebuiltTarget.toString(16) };
        });
        assert(calls.every(call => call.target === readTime), 'Unexpected residual direct call');
        const report = {
            program: 'simscheduler.dll', entry: entry.toString(16), rebuiltEntry: rebuiltEntry.toString(16),
            status: 'Diagnostic', scope: 'Complete linked C++ scheduler time-update body with checked direct call targets',
            originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
            originalHex: originalBody.toString('hex'), rebuiltHex: rebuiltBody.toString('hex'),
            references, rebuiltListing: rows,
        };
        fs.writeFileSync(path.join(run, 'comparison-' + report.entry + '.json'), JSON.stringify(report, null, 2) + '\n');
        console.log('Diagnostic ' + report.entry + ': ' + originalBody.length + ' original bytes, ' + rebuiltBody.length + ' linked bytes');
    }
    process.exit(0);
}
if (request.DispatcherProfile === 'FlightLoadLtcg' || process.argv[3] === '--flight-load') {
    const entry = 0x20cc4790;
    const rebuiltEntry = mapAddress('?LoadSchedulerFlightState@@YAXPAX@Z');
    const rows = readListing(rebuiltEntry, mapAddress('?SaveSchedulerFlightState@@YGXPAX@Z'));
    const originalBody = original.codeAt(entry, 308);
    assert.equal(originalBody.toString('hex'), '8bff558bec83e4f883ec70d9ee5357dd5c24108b3d0811cc208d442410568944242033c0508d4c242051687c11cc20c7442428b011cc20c744243001000000c744243413000000894424388944243c8944244089442444894424488944244c8944245089442454ffd785c07417566a008d54242052688c11cc20ffd785c07404d9eeeb04dd44241083ec088bc4bb7083cc20dd18e817d2ffff5633db538d4424148d4c24508944245433c051687c11cc20c744241c00010000c74424589c11cc20c744246001000000c744246416000000c74424685b000000895c246c895c247089442474894424788944247c8984248000000089842484000000ffd785c0741956538d5424505268a811cc20ffd785c074076800010000eb058b54240c52a17083cc208b4814687083cc20ffd15f5b8be55dc3');
    const rebuiltBody = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
    assert.equal(rows[rows.length - 1][2], 'ret');
    const calls = rows.filter(row => row[2].startsWith('call '));
    assert.equal(calls.length, 6);
    const [applyCall] = calls.filter(row => row[1].startsWith('E8 '));
    const applyAddress = Number.parseInt(applyCall[0], 16);
    const target = decodeCall(rebuilt, applyAddress);
    assert.equal(decodeCall(original, 0x20cc4824), 0x20cc1a40);
    assert.equal(target, mapAddress(map.includes('?ApplySchedulerTime@@YGXPAUSchedulerState@@N@Z')
        ? '?ApplySchedulerTime@@YGXPAUSchedulerState@@N@Z' : '?ApplySchedulerTime@@YAXPAUSchedulerState@@N@Z'));
    const slot = 0x14;
    assert.equal(original.codeAt(0x20cc11cc + slot, 4, false).readUInt32LE(), 0x20cc19e0);
    assert(rows.some(row => row[2].includes('+14h]')));
    const originalIat = original.codeAt(0x20cc47a3, 6).readUInt32LE(2);
    const imported = readPE32Imports(originalBytes).get(originalIat);
    assert.deepEqual(imported, { dll: 'flight.dll', name: '#13' });
    const linkedImport = request.DispatcherProfile === 'FlightLoadLtcg';
    const rebuiltPointer = linkedImport ? mapAddress('__imp__readFlightState@16')
        : probeAddress('readFlightState', '_readFlightState');
    if (linkedImport) assert.deepEqual(readPE32Imports(rebuiltBytes).get(rebuiltPointer), imported);
    const referenceRows = rows.filter(row => {
        const bytes = Buffer.from(row[1].replaceAll(' ', ''), 'hex');
        return bytes.length >= 5 && bytes.readUInt32LE(bytes.length - 4) === rebuiltPointer;
    });
    assert(referenceRows.length > 0, 'Expected body references to the Flight load pointer');
    const transferCalls = calls.filter((_, index) => [0, 1, 3, 4].includes(index));
    if (linkedImport) {
        const registerLoad = referenceRows.find(row => row[2].startsWith('mov '));
        assert(registerLoad, 'Expected register load of the Flight import');
        const register = registerLoad[2].match(/^mov ([a-z]+),/)[1];
        assert(transferCalls.every(row => row[2] === 'call ' + register));
        for (const va of [0x20cc47f7, 0x20cc480a, 0x20cc488b, 0x20cc489d]) {
            assert.equal(original.codeAt(va, 2).toString('hex'), 'ffd7');
        }
    } else {
        assert(transferCalls.every(row => referenceRows.includes(row)));
    }
    const strings = [
        ['SimScheduler', 0x20cc117c], ['DateTimeSeason', 0x20cc118c], ['Options', 0x20cc11a8],
        ['SimTime', 0x20cc11b0], ['SimSpeed', 0x20cc119c],
    ];
    for (const [value, va] of strings) {
        const literal = Buffer.from(value + '\0');
        assert.deepEqual(original.codeAt(va, literal.length, false), literal);
        const symbol = map.match(new RegExp('\\S+@' + value + '\\?\\$AA@'))[0];
        const address = mapAddress(symbol);
        assert.deepEqual(rebuilt.codeAt(address, literal.length, false), literal);
        assert(rows.some(row => row[2].includes(symbol)));
    }
    const report = {
        program: 'simscheduler.dll', entry: entry.toString(16), rebuiltEntry: rebuiltEntry.toString(16),
        status: 'Diagnostic', scope: 'Complete linked Flight load handler with checked transfer, SimSpeed slot and propagation call',
        originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
        originalHex: originalBody.toString('hex'), rebuiltHex: rebuiltBody.toString('hex'),
        transfer: { originalIat: originalIat.toString(16), rebuiltPointer: rebuiltPointer.toString(16),
            imported, linkedImport, referenceSites: referenceRows.map(row => row[0]),
            callSites: transferCalls.map(row => row[0]) },
        propagation: { originalSite: '20cc4824', originalTarget: '20cc1a40',
            rebuiltSite: applyAddress.toString(16), rebuiltTarget: target.toString(16) },
        rebuiltListing: rows,
    };
    fs.writeFileSync(path.join(run, 'comparison-20cc4790.json'), JSON.stringify(report, null, 2) + '\n');
    console.log('Diagnostic 20cc4790: ' + originalBody.length + ' original bytes, ' + rebuiltBody.length + ' linked bytes');
    process.exit(0);
}
if (request.DispatcherProfile === 'FlightSaveLtcg' || process.argv[3] === '--flight-save') {
    const entry = 0x20cc48d0;
    const rebuiltEntry = mapAddress('?SaveSchedulerFlightState@@YGXPAX@Z');
    const rows = readListing(rebuiltEntry, probeAddress('SchedulerGroup10Callback', '_SchedulerGroup10Callback@8'));
    const originalBody = original.codeAt(entry, 164);
    assert.equal(originalBody.toString('hex'), '8bff558bec83ec648b0d7083cc208b51208d45e450687083cc20ffd2dd00a17083cc20dd5dec8b4818687083cc20ffd1b9010000008d55f88955a0894da4894dbc8b4d08518945f833c08d55ec8955b8508d559c52687c11cc20c7459c9c11cc20c745a816000000c745ac5b0000008945b0c745b4b011cc20c745c0130000008945c48945c88945cc8945d08945d48945d88945dc8945e0ff150411cc208be55dc20400');
    const rebuiltBody = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
    assert.equal(rows[rows.length - 1][2], 'ret 4');
    for (const [slot, target] of [[0x18, 0x20cc1a00], [0x20, 0x20cc1ad0]]) {
        assert.equal(original.codeAt(0x20cc11cc + slot, 4, false).readUInt32LE(), target);
        assert(rows.some(row => row[2].includes('+' + slot.toString(16) + 'h]')));
    }
    const calls = rows.filter(row => row[2].startsWith('call '));
    assert.equal(calls.length, 3);
    const originalIat = original.codeAt(0x20cc4968, 6).readUInt32LE(2);
    const imported = readPE32Imports(originalBytes).get(originalIat);
    assert.deepEqual(imported, { dll: 'flight.dll', name: '#14' });
    const lastCall = rebuilt.codeAt(Number.parseInt(calls[2][0], 16), 6);
    assert.equal(lastCall.readUInt16LE(), 0x15ff);
    const rebuiltPointer = lastCall.readUInt32LE(2);
    const rebuiltImport = readPE32Imports(rebuiltBytes).get(rebuiltPointer);
    if (rebuiltImport) assert.deepEqual(rebuiltImport, imported);
    else assert.equal(rebuiltPointer, probeAddress('writeFlightState', '_writeFlightState'));
    const literals = [
        [86, '??_C@_0N@HIPEPPF@SimScheduler?$AA@', 0x20cc117c, 'SimScheduler'],
        [93, '??_C@_08EHFBDKJI@SimSpeed?$AA@', 0x20cc119c, 'SimSpeed'],
        [117, '??_C@_07DIAINHBC@SimTime?$AA@', 0x20cc11b0, 'SimTime'],
    ];
    for (const [, symbol, va, value] of literals) {
        const literal = Buffer.from(value + '\0');
        assert.deepEqual(original.codeAt(va, literal.length, false), literal);
        assert.deepEqual(rebuilt.codeAt(mapAddress(symbol), literal.length, false), literal);
    }
    let status = 'Diagnostic';
    const mappings = [];
    if (rebuiltImport && rebuiltBody.length === originalBody.length) {
        const objectVA = probeAddress('schedulerObject', '_schedulerObject');
        const fields = [
            ...[10, 22, 31, 42].map(offset => [offset, 0x20cc8370, objectVA, 'scheduler object']),
            ...literals.map(([offset, symbol, va, value]) => [offset, va, mapAddress(symbol), value]),
            [154, originalIat, rebuiltPointer, 'Flight save import'],
        ];
        const offsets = fields.map(field => field[0]);
        assert.deepEqual(original.highLowFixups(entry, originalBody.length), offsets);
        assert.deepEqual(rebuilt.highLowFixups(rebuiltEntry, rebuiltBody.length), offsets);
        const resolved = Buffer.from(rebuiltBody);
        for (const [offset, originalTarget, rebuiltTarget, label] of fields) {
            assert.equal(originalBody.readUInt32LE(offset), originalTarget);
            assert.equal(rebuiltBody.readUInt32LE(offset), rebuiltTarget);
            resolved.writeUInt32LE(originalTarget, offset);
            mappings.push({ offset, label, originalTarget: originalTarget.toString(16), rebuiltTarget: rebuiltTarget.toString(16) });
        }
        if (resolved.equals(originalBody)) status = 'Normalized';
    }
    const report = {
        program: 'simscheduler.dll', entry: entry.toString(16), rebuiltEntry: rebuiltEntry.toString(16),
        status, scope: 'Complete linked C++ Flight save handler with checked VTable slots and transfer reference',
        originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
        originalHex: originalBody.toString('hex'), rebuiltHex: rebuiltBody.toString('hex'),
        transfer: { originalIat: originalIat.toString(16), rebuiltPointer: rebuiltPointer.toString(16),
            imported, rebuiltImport: rebuiltImport || null },
        rebuiltListing: rows, mappings,
    };
    fs.writeFileSync(path.join(run, 'comparison-20cc48d0.json'), JSON.stringify(report, null, 2) + '\n');
    console.log(status + ' 20cc48d0: ' + originalBody.length + ' original bytes, ' + rebuiltBody.length + ' linked bytes');
    process.exit(0);
}
if (request.DispatcherProfile === 'TimePropagationLtcg' || process.argv[3] === '--time-propagation') {
    const entry = 0x20cc1a40;
    const symbol = request.DispatcherProfile === 'TimePropagationLtcg'
        ? '?ApplySchedulerTime@@YGXPAUSchedulerState@@N@Z'
        : '?ApplySchedulerTime@@YAXPAUSchedulerState@@N@Z';
    const rebuiltEntry = mapAddress(symbol);
    const rows = readListing(rebuiltEntry, mapAddress('?UpdateSchedulerTimeState@@YAXPAUSchedulerState@@H@Z'));
    const originalBody = original.codeAt(entry, 138);
    assert.equal(originalBody.toString('hex'), '8bff558bec83ec188b45088b4d0c89430433c03943305657894b088945f87e62dd45088945fcdd5de88b533c8b45fc8b4402208b088b514c50ffd233c93948607e22dd4508dd5df08b75f48b7df08bff8b506c8b148a83c101897a048972083b48607cec8b4de88b55ec8345fc248948448950488b45f883c0013b43308945f87ca75f5e8be55dc20800');
    const rebuiltBody = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
    assert.match(rows[rows.length - 1][2], /^ret(?: 8)?$/);
    assert.equal(rows.filter(row => row[2].startsWith('call ')).length, 1);
    assert(rows.some(row => /mov .*\+4Ch\]/i.test(row[2])), 'Expected VTable slot +0x4c');
    const status = rebuiltBody.equals(originalBody) ? 'Exact' : 'Diagnostic';
    const report = {
        program: 'simscheduler.dll', entry: entry.toString(16), rebuiltEntry: rebuiltEntry.toString(16),
        status, scope: 'Complete linked LTCG propagation function, with indirect VTable slot checked',
        originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
        originalHex: originalBody.toString('hex'), rebuiltHex: rebuiltBody.toString('hex'),
        rebuiltListing: rows,
    };
    fs.writeFileSync(path.join(run, 'comparison-20cc1a40.json'), JSON.stringify(report, null, 2) + '\n');
    console.log(status + ' 20cc1a40: ' + originalBody.length + ' original bytes, ' + rebuiltBody.length + ' linked bytes');
    process.exit(0);
}
if (request.DispatcherProfile === 'TimeStateLtcg') {
    const entry = 0x20cc1b30;
    const rebuiltEntry = mapAddress('?UpdateSchedulerTimeState@@YAXPAUSchedulerState@@H@Z');
    const rows = readListing(rebuiltEntry, probeAddress('ProbeSchedulerTimeState', '_ProbeSchedulerTimeState@8'));
    const originalBody = original.codeAt(entry, 81);
    assert.equal(originalBody.toString('hex'), '8bff558bec83ec0c85f6538bd87425837b240075348b038b50208d4df85153ffd28b08894b288b500489532c8973245b8be55dc3837b2400740fdd432883ec088bc4dd18e8c7feffff8973245b8be55dc3');
    const rebuiltBody = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
    assert.match(rows[rows.length - 1][2], /^ret$/);
    const calls = rows.filter(row => row[2].startsWith('call ') && row[1].startsWith('E8 '));
    assert.equal(calls.length, 1);
    const rebuiltSite = Number.parseInt(calls[0][0], 16);
    const target = mapAddress('?ApplySchedulerTime@@YAXPAUSchedulerState@@N@Z');
    assert.equal(decodeCall(original, 0x20cc1b74), 0x20cc1a40);
    assert.equal(decodeCall(rebuilt, rebuiltSite), target);
    const resolved = Buffer.from(rebuiltBody);
    resolved.writeInt32LE(originalBody.readInt32LE(0x45), rebuiltSite - rebuiltEntry + 1);
    const status = resolved.equals(originalBody) ? 'Normalized' : 'Diagnostic';
    const report = {
        program: 'simscheduler.dll', entry: entry.toString(16), rebuiltEntry: rebuiltEntry.toString(16),
        status, scope: 'Complete linked LTCG time-state handler with verified direct call target',
        originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
        originalHex: originalBody.toString('hex'), rebuiltHex: rebuiltBody.toString('hex'),
        reference: { originalSite: '20cc1b74', originalTarget: '20cc1a40',
            rebuiltSite: rebuiltSite.toString(16), rebuiltTarget: target.toString(16) },
        rebuiltListing: rows,
    };
    fs.writeFileSync(path.join(run, 'comparison-20cc1b30.json'), JSON.stringify(report, null, 2) + '\n');
    console.log(status + ' 20cc1b30: ' + originalBody.length + ' original bytes, ' + rebuiltBody.length + ' linked bytes');
    process.exit(0);
}
const entry = 0x20cc1400;
const rebuiltEntry = probeAddress('SchedulerGroup10Callback', '_SchedulerGroup10Callback@8');
const nextFunction = Math.min(...[...map.matchAll(/^\s+0001:[0-9a-f]+\s+\S+\s+([0-9a-f]+)\s+f\s/gim)]
    .map(match => Number.parseInt(match[1], 16)).filter(address => address > rebuiltEntry));
const rows = readListing(rebuiltEntry, nextFunction);
assert.match(rows.at(-1)[2], /^ret 8$/);
const calls = rows.filter(row => row[2].startsWith('call '));
assert.equal(calls.length, 3);
const targets = [
    [0x20cc142d, 0x20cc1b30, '?UpdateSchedulerTimeState@@YAXPAUSchedulerState@@H@Z'],
    [0x20cc143d, 0x20cc48d0, '?SaveSchedulerFlightState@@YGXPAX@Z'],
    [0x20cc144c, 0x20cc4790, '?LoadSchedulerFlightState@@YAXPAX@Z'],
];
const references = targets.map(([site, target, symbol]) => {
    assert.equal(decodeCall(original, site), target);
    const rebuiltTarget = mapAddress(symbol);
    const matchingSites = calls.map(row => Number.parseInt(row[0], 16))
        .filter(address => decodeCall(rebuilt, address) === rebuiltTarget);
    assert.equal(matchingSites.length, 1, symbol);
    const [rebuiltSite] = matchingSites;
    return { originalSite: site.toString(16), originalTarget: target.toString(16),
        rebuiltSite: rebuiltSite.toString(16), rebuiltTarget: rebuiltTarget.toString(16), symbol };
});
const originalBody = original.codeAt(entry, 88);
const rebuiltBody = Buffer.from(rows.map(row => row[1].replaceAll(' ', '')).join(''), 'hex');
assert.deepEqual(original.highLowFixups(entry, originalBody.length), [0x29]);
const normalized = Buffer.from(rebuiltBody);
for (const reference of references) {
    const originalOffset = Number.parseInt(reference.originalSite, 16) - entry;
    const rebuiltOffset = Number.parseInt(reference.rebuiltSite, 16) - rebuiltEntry;
    originalBody.copy(normalized, rebuiltOffset + 1, originalOffset + 1, originalOffset + 5);
}
const objectAddress = mapAddress('?schedulerObject@@3USchedulerState@@A');
const objectLoads = rows.filter(row => /^B8 /i.test(row[1]) &&
    rebuilt.codeAt(Number.parseInt(row[0], 16), 5).readUInt32LE(1) === objectAddress);
let objectReference = null;
if (objectLoads.length === 1) {
    const originalSite = 0x20cc1428;
    assert.equal(original.codeAt(originalSite, 5).toString('hex'), 'b87083cc20');
    const rebuiltSite = Number.parseInt(objectLoads[0][0], 16);
    normalized.writeUInt32LE(0x20cc8370, rebuiltSite - rebuiltEntry + 1);
    objectReference = { originalSite: '20cc1428', originalTarget: '20cc8370',
        rebuiltSite: rebuiltSite.toString(16), rebuiltTarget: objectAddress.toString(16) };
}
assert.deepEqual(rebuilt.highLowFixups(rebuiltEntry, rebuiltBody.length), objectReference
    ? [Number.parseInt(objectReference.rebuiltSite, 16) - rebuiltEntry + 1]
    : []);
const status = rebuiltBody.equals(originalBody) ? 'Exact'
    : objectReference && normalized.equals(originalBody) ? 'Normalized' : 'Diagnostic';
const report = {
    program: 'simscheduler.dll', entry: entry.toString(16), rebuiltEntry: rebuiltEntry.toString(16),
    status, scope: 'Complete linked LTCG dispatcher with verified direct call targets and any EAX scheduler-object load',
    originalSHA256: sha(originalBytes), rebuiltSHA256: sha(rebuiltBytes),
    originalHex: originalBody.toString('hex'), rebuiltHex: rebuiltBody.toString('hex'),
    references, objectReference, rebuiltListing: rows,
};
fs.writeFileSync(path.join(run, 'comparison-20cc1400.json'), JSON.stringify(report, null, 2) + '\n');
console.log(status + ' 20cc1400: ' + originalBody.length + ' original bytes, ' + rebuiltBody.length + ' linked bytes');
