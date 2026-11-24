const assert = require('node:assert/strict');
const fs = require('node:fs');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const listing = require('../artifacts/simscheduler.dll/eh-funclets-ghidra.json');

const artifact = 'artifacts/simscheduler.dll';
const runs = 'vm-share/modules/simscheduler/runs';
const hash = bytes => createHash('sha256').update(bytes).digest('hex');
const va = rva => 0x20cc0000 + rva;
const hex = address => address.toString(16);
const original = fs.readFileSync('original/simscheduler.dll');
const originalSHA256 = hash(original);
assert.equal(originalSHA256, '16059b50e722bfa10357582def8d422a8222f620189e40c8878928a7ddaa9dd4');
const image = openImage(original);
assert.equal(image.codeAt(va(0x702e), 0x282).toString('hex'), listing.asm.map(row => row[1]).join(''));
const group = {
    name: 'registry-unwind-20261116-001',
    context: 'context-lifecycle-20261116-003',
    pool: 'pool-lifecycle-20261116-003',
    poolGuard: 'pool-guard-unwind-20261116-001',
    fallback: 'fallback-unwind-20261116-001',
    factory: 'context-factory-unwind-20261116-001',
};
const report = [];
for (const [handler, info, source, owners] of [
    [0x7061, 0x73b4, group.name, [[0x17d0, 8, '_FindNamedSchedulerContext@12']]],
    [0x70b4, 0x73f0, group.context, [[0x27a0, 8, '??0SchedulerContext@@QAE@PBD@Z']]],
    [0x70e8, 0x741c, group.name, [[0x18c0, 8, '_RemoveNamedSchedulerContext@8']]],
    [0x7118, 0x7448, group.pool, [[0x3eb0, 8, '_ConstructSchedulerContextPool'], [0x46f0, 8, '_ConstructSchedulerEntryPool']]],
    [0x7148, 0x7474, group.pool, [[0x3810, 8, '_DestroySchedulerContextPool@4'], [0x4380, 8, '_DestroySchedulerEntryPool@4']]],
    [0x717e, 0x74a0, group.poolGuard, [[0x4650, 8, '_GetSchedulerEntryPool']]],
    [0x71ae, 0x74cc, group.poolGuard, [[0x3cf0, 8, '_GetSchedulerContextPool']]],
    [0x71f9, 0x7508, group.factory, [[0x1670, 8, '_CreateNamedSchedulerContext@12']]],
    [0x7238, 0x7544, group.fallback, [[0x1cc0, 11, '_UpdateSimConnectFallback@8']]],
    [0x7284, 0x7580, group.context, [[0x2940, 8, '??1SchedulerContext@@QAE@XZ']]],
]) {
    const entry = va(handler);
    const left = image.codeAt(entry, 27);
    assert.equal(left[12], 0xe8);
    assert.equal(entry + 17 + left.readInt32LE(13), va(0x69ae));
    assert.equal(left[17], 0xb8);
    assert.equal(left.readUInt32LE(18), va(info));
    assert.deepEqual(image.highLowFixups(entry, 27), [18]);
    assert.equal(left[22], 0xe9);
    assert.equal(entry + 27 + left.readInt32LE(23), va(0x6f50));
    const infoBytes = image.codeAt(va(info), 36, false);
    assert.equal(infoBytes.readUInt32LE(), 0x19930522);
    assert.deepEqual(image.highLowFixups(va(info), 36), [8]);
    const count = infoBytes.readUInt32LE(4);
    const map = infoBytes.readUInt32LE(8);
    const mapBytes = image.codeAt(map, count * 8, false);
    assert.deepEqual(image.highLowFixups(map, count * 8), Array.from({ length: count }, (_, i) => i * 8 + 4));
    const states = Array.from({ length: count }, (_, i) => ({
        next: mapBytes.readInt32LE(i * 8),
        action: hex(mapBytes.readUInt32LE(i * 8 + 4)),
    }));
    const asm = listing.asm.filter(([address]) => parseInt(address, 16) >= entry && parseInt(address, 16) < entry + 27);
    assert.equal(asm.map(row => row[1]).join(''), left.toString('hex'));
    const funclets = states.map(({ action }, index) => {
        const start = parseInt(action, 16);
        const end = index + 1 === states.length ? entry : parseInt(states[index + 1].action, 16);
        const rows = listing.asm.filter(([address]) => parseInt(address, 16) >= start && parseInt(address, 16) < end);
        assert.equal(rows[0][0], action);
        assert.match(rows.at(-1)[2], /^(RET|JMP)\b/);
        return { entry: action, asm: rows };
    });
    const run = runs + '/simscheduler-' + source;
    const object = fs.readFileSync(run + '/simscheduler-object.obj');
    const coff = parseNativeCoff(object);
    const workerListing = fs.readFileSync(run + '/worker-output/disassembly.txt', 'utf8');
    const candidates = [];
    for (const [rva, offset, owner] of owners) {
        assert.equal(image.codeAt(va(rva + offset), 4).readUInt32LE(), entry);
        assert.deepEqual(image.highLowFixups(va(rva + offset), 4), [0]);
        const symbol = '__ehhandler$' + owner;
        const fn = coff.unique(symbol);
        const section = coff.sections[fn.section - 1];
        const end = section.raw + section.size;
        const right = object.subarray(section.raw + fn.value, end);
        const relocations = section.relocations.filter(rel => rel.offset >= fn.value)
            .map(rel => ({ offset: rel.offset - fn.value, type: rel.type, target: rel.symbol.name }));
        const infoSymbol = '__ehfuncinfo$' + owner;
        const infoEntry = coff.unique(infoSymbol);
        const infoSection = coff.sections[infoEntry.section - 1];
        assert.equal(object.readUInt32LE(infoSection.raw + infoEntry.value), 0x19930522);
        const expected = [[13, 20, '@__security_check_cookie@4'], [18, 6, infoSymbol], [23, 20, '___CxxFrameHandler3']];
        const mappedOffsets = expected.filter(([offset, type, target]) =>
            relocations.some(rel => rel.offset === offset && rel.type === type && rel.target === target)
            && right.readUInt32LE(offset) === 0).map(entry => entry[0]);
        const normalized = Buffer.from(right);
        for (const offset of mappedOffsets) left.copy(normalized, offset, offset, offset + 4);
        const status = mappedOffsets.length === 3 && relocations.length === 3
            && left.equals(normalized) ? 'Normalized' : 'Diagnostic';
        const stateCount = object.readUInt32LE(infoSection.raw + infoEntry.value + 4);
        const mapRef = infoSection.relocations.find(rel => rel.offset === infoEntry.value + 8);
        const rebuiltStates = [];
        if (stateCount) {
            assert.equal(mapRef.type, 6);
            assert.equal(mapRef.symbol.name, '__unwindtable$' + owner);
            const table = mapRef.symbol;
            const tableSection = coff.sections[table.section - 1];
            for (let index = 0; index < stateCount; index++) {
                const offset = table.value + index * 8;
                const next = object.readInt32LE(tableSection.raw + offset);
                const action = tableSection.relocations.find(rel => rel.offset === offset + 4)?.symbol;
                rebuiltStates.push({ next, action: action?.name ?? null });
            }
        }
        const block = workerListing.split(symbol + ':')[1].split(/\r?\n\s*\r?\n/)[0];
        const rebuilt = [...block.matchAll(/^ +([0-9a-f]{8}): ((?:[0-9a-f]{2} )*[0-9a-f]{2}) +(\S[^\r\n]*)/gim)]
            .map(row => [row[1], row[2], row[3].trim().replace(/\s+/g, ' ')]);
        assert.equal(rebuilt.map(row => row[1].replaceAll(' ', '').toLowerCase()).join(''), right.toString('hex'));
        for (const state of rebuiltStates.filter(state => state.action)) {
            const action = coff.unique(state.action);
            const actionSection = coff.sections[action.section - 1];
            const actionBlock = workerListing.split(state.action + ':')[1].split(/\r?\n(?=\S)/)[0];
            state.asm = [...actionBlock.matchAll(/^ +([0-9a-f]{8}): ((?:[0-9a-f]{2} )*[0-9a-f]{2}) +(\S[^\r\n]*)/gim)]
                .map(row => [row[1], row[2], row[3].trim().replace(/\s+/g, ' ')]);
            const bytes = state.asm.map(row => row[1].replaceAll(' ', '').toLowerCase()).join('');
            const start = actionSection.raw + action.value;
            assert.equal(object.subarray(start, start + bytes.length / 2).toString('hex'), bytes);
            state.bytes = bytes;
            state.relocations = actionSection.relocations
                .filter(rel => rel.offset >= action.value && rel.offset < action.value + bytes.length / 2)
                .map(rel => ({ offset: rel.offset - action.value, type: rel.type, target: rel.symbol.name }));
        }
        const candidate = { owner: hex(va(rva)), symbol, run, objectSHA256: hash(object),
            status, bytes: right.toString('hex'), relocations, mappedOffsets, rebuilt, rebuiltStates };
        if (handler === 0x7118) {
            const action = funclets[0];
            const start = parseInt(action.entry, 16);
            const code = image.codeAt(start, 8);
            assert.equal(code.subarray(0, 4).toString('hex'), '8b4df0e9');
            assert.equal(start + 8 + code.readInt32LE(4), va(0x3580));
            assert.deepEqual(image.highLowFixups(start, code.length), []);
            assert.deepEqual(states, [{ next: -1, action: '20cc7110' }]);
            // The original enters state 0 after publishing the list node.
            assert.equal(image.codeAt(va(rva + 0x43), 3).toString('hex'), '8975fc');
            assert.equal(image.codeAt(va(rva + 0x5c), 1)[0], 0xe8);
            const parent = coff.unique(owner);
            const parentSection = coff.sections[parent.section - 1];
            const parentBytes = object.subarray(parentSection.raw, parentSection.raw + parentSection.size);
            assert.equal(parentBytes.subarray(0x34, 0x37).toString('hex'), '8975fc');
            assert.equal(parentBytes.subarray(0x46, 0x4a).toString('hex'), 'c645fc01');
            assert(parentSection.relocations.some(rel => rel.offset === 0x61
                && rel.symbol.name === '_CreateSchedulerPoolLock'));
            const [placement, registration] = rebuiltStates;
            assert.deepEqual(rebuiltStates.map(state => state.next), [-1, 0]);
            assert.deepEqual(placement.relocations.map(rel => rel.target), [
                rva === 0x3eb0 ? '_schedulerContextPoolStorage' : '_schedulerEntryPoolStorage',
                '??3@YAXPAX0@Z',
            ]);
            const placementDelete = coff.unique('??3@YAXPAX0@Z');
            const deleteSection = coff.sections[placementDelete.section - 1];
            assert.equal(placementDelete.value, 0);
            assert.equal(deleteSection.size, 1);
            assert.equal(object[deleteSection.raw], 0xc3);
            assert.deepEqual(deleteSection.relocations, []);
            assert.deepEqual(registration.relocations,
                [{ offset: 4, type: 20, target: '??1SchedulerPoolRegistration@@QAE@XZ' }]);
            const unlink = JSON.parse(fs.readFileSync(run + '/comparison-20cc3580.json'));
            assert.equal(unlink.originalSHA256, originalSHA256);
            assert.equal(unlink.objectSHA256, candidate.objectSHA256);
            assert.equal(unlink.symbol, '@UnlinkSchedulerPool@4');
            const destructor = coff.unique(registration.relocations[0].target);
            const destructorSection = coff.sections[destructor.section - 1];
            const unlinkSymbol = coff.unique(unlink.symbol);
            const unlinkSection = coff.sections[unlinkSymbol.section - 1];
            assert.equal(destructor.value, 0);
            assert.equal(unlinkSymbol.value, 0);
            assert.deepEqual(object.subarray(destructorSection.raw, destructorSection.raw + destructorSection.size),
                object.subarray(unlinkSection.raw, unlinkSection.raw + unlinkSection.size));
            assert.deepEqual(destructorSection.relocations, unlinkSection.relocations);
            const normalizedAction = Buffer.from(registration.bytes, 'hex');
            assert.equal(normalizedAction.length, code.length);
            assert.equal(normalizedAction.readInt32LE(4), 0);
            code.copy(normalizedAction, 4, 4);
            assert.deepEqual(normalizedAction, code);
            candidate.cleanup = {
                entry: action.entry, status: 'Normalized', mappedOffsets: [4],
                originalHex: code.toString('hex'), rebuiltHex: registration.bytes,
                rebuilt: registration.asm, originalTarget: '20cc3580',
                rebuiltTarget: registration.relocations[0].target,
                placementDeleteHex: 'c3',
            };
        }
        if (handler === 0x7148) {
            const action = funclets[0];
            const start = parseInt(action.entry, 16);
            const code = image.codeAt(start, 8);
            assert.equal(code.subarray(0, 4).toString('hex'), '8b4d08e9');
            assert.equal(start + 8 + code.readInt32LE(4), va(0x3580));
            assert.deepEqual(image.highLowFixups(start, code.length), []);
            assert.deepEqual(states, [{ next: -1, action: '20cc7140' }]);
            const stateWrite = 'c745fc00000000';
            assert.equal(image.codeAt(va(rva + 0x30), 7).toString('hex'), stateWrite);
            for (const [offset, target] of [[0x37, rva + 0xb0], [0x43, 0x63e0]]) {
                const call = image.codeAt(va(rva + offset), 5);
                assert.equal(call[0], 0xe8);
                assert.equal(va(rva + offset + 5) + call.readInt32LE(1), va(target));
            }
            const parent = coff.unique(owner);
            const parentSection = coff.sections[parent.section - 1];
            const parentBytes = object.subarray(parentSection.raw, parentSection.raw + parentSection.size);
            assert.equal(parentBytes.subarray(0x2e, 0x31).toString('hex'), '897508');
            assert.equal(parentBytes.subarray(0x33, 0x3a).toString('hex'), stateWrite);
            assert(parentSection.relocations.some(rel => rel.offset === 0x3b
                && rel.symbol.name === (rva === 0x3810 ? '@DrainSchedulerContextPool@4' : '@DrainSchedulerEntryPool@4')));
            assert(parentSection.relocations.some(rel => rel.offset === 0x48
                && rel.symbol.name === '_DestroySchedulerPoolLock'));
            const [registration] = rebuiltStates;
            assert.equal(rebuiltStates.length, 1);
            assert.equal(registration.next, -1);
            assert.equal(registration.bytes, '8d4d08e900000000');
            assert.deepEqual(registration.relocations,
                [{ offset: 4, type: 20, target: '??1SchedulerPoolUnlink@@QAE@XZ' }]);
            const guard = coff.unique(registration.relocations[0].target);
            const guardSection = coff.sections[guard.section - 1];
            const guardCode = object.subarray(guardSection.raw, guardSection.raw + guardSection.size);
            assert.equal(guardCode.subarray(0, 2).toString('hex'), '8b09');
            assert.deepEqual([...new Set(guardSection.relocations.map(rel => rel.symbol.name))].sort(),
                ['_schedulerListHead', '_schedulerPoolBaseTable']);
            candidate.cleanup = {
                entry: action.entry, status: 'Diagnostic', mappedOffsets: [4],
                originalHex: code.toString('hex'), rebuiltHex: registration.bytes,
                rebuilt: registration.asm, originalTarget: '20cc3580',
                rebuiltTarget: registration.relocations[0].target, guardPrefix: '8b09',
            };
        }
        if (source === group.poolGuard) {
            const action = funclets[0];
            const start = parseInt(action.entry, 16);
            const code = image.codeAt(start, 14);
            const entryPool = rva === 0x4650;
            const guardAddress = va(entryPool ? 0x83ec : 0x83f0);
            const poolName = entryPool ? 'Entry' : 'Context';
            const guardSymbol = '_scheduler' + poolName + 'PoolGuard';
            assert.deepEqual(image.highLowFixups(start, 14), [1, 9]);
            assert.equal(code.readUInt32LE(1), guardAddress);
            assert.equal(code.readUInt32LE(9), guardAddress);
            assert.equal(code.toString('hex'), 'a1' + code.subarray(1, 5).toString('hex')
                + '83e0fea3' + code.subarray(1, 5).toString('hex') + 'c3');
            assert.deepEqual(states, [{ next: -1, action: action.entry }]);
            assert.equal(image.codeAt(va(rva + 0x37), 7).toString('hex'), 'c745fc00000000');
            for (const [offset, target] of [[0x3e, entryPool ? 0x46f0 : 0x3eb0], [0x48, 0x6a66]]) {
                const call = image.codeAt(va(rva + offset), 5);
                assert.equal(call[0], 0xe8);
                assert.equal(va(rva + offset + 5) + call.readInt32LE(1), va(target));
            }
            assert.equal(object.readUInt32LE(infoSection.raw + infoEntry.value + 12), 0);
            const [registration] = rebuiltStates;
            assert.equal(rebuiltStates.length, 1);
            assert.equal(registration.next, -1);
            assert.equal(registration.bytes, '8d4df3e900000000');
            const [guardRelocation] = registration.relocations;
            assert.equal(registration.relocations.length, 1);
            assert.equal(guardRelocation.offset, 4);
            assert.equal(guardRelocation.type, 20);
            const destructor = coff.unique(guardRelocation.target);
            const destructorSection = coff.sections[destructor.section - 1];
            const destructorBytes = object.subarray(destructorSection.raw, destructorSection.raw + destructorSection.size);
            assert.equal(destructorBytes.toString('hex'), '8039007507832500000000fec3');
            assert.deepEqual(destructorSection.relocations.map(rel => [rel.offset, rel.type, rel.symbol.name]),
                [[7, 6, guardSymbol]]);
            const parent = coff.unique(owner);
            const parentSection = coff.sections[parent.section - 1];
            const parentBytes = object.subarray(parentSection.raw, parentSection.raw + parentSection.size);
            assert.equal(parentBytes.subarray(0x38, 0x40).toString('hex'), '33c08845f38945fc');
            for (const [offset, target] of [[0x34, guardSymbol], [0x41, '_ConstructScheduler' + poolName + 'Pool'],
                [0x4b, '_atexit']]) {
                assert(parentSection.relocations.some(rel => rel.offset === offset && rel.symbol.name === target));
            }
            candidate.cleanup = {
                entry: action.entry, status: 'Diagnostic', originalHex: code.toString('hex'),
                rebuiltHex: registration.bytes, rebuilt: registration.asm,
                guardAddress: hex(guardAddress), guardSymbol, destructorHex: destructorBytes.toString('hex'),
            };
        }
        if (source === group.factory) {
            assert.deepEqual(states.map(state => state.next), [-1, 0, 0]);
            assert.deepEqual(rebuiltStates.map(state => state.next), [-1, 0, 0]);
            assert.equal(object.readUInt32LE(infoSection.raw + infoEntry.value + 12), 0);
            const parent = coff.unique(owner);
            const parentSection = coff.sections[parent.section - 1];
            const parentBytes = object.subarray(parentSection.raw, parentSection.raw + parentSection.size);
            for (const [offset, bytes] of [
                [0x9c, '895dbcc645c0008975fc'], // Result slot stored, publication disabled.
                [0xb7, 'c645fc01e8'], // Raw storage owned during construction.
                [0xc4, 'c645fc00'], // No raw-storage cleanup during addReference.
                [0xe0, 'c645fc02e8'], // Constructed reference owned during insertion.
                [0x10a, '8933c645c001c645fc00'], // Publish and enter state 0 before local release.
                [0x116, '8b0e8b510456ffd2'],
            ]) {
                assert.equal(parentBytes.subarray(offset, offset + bytes.length / 2).toString('hex'), bytes);
            }
            for (const [offset, bytes] of [[0xcb, '895dfc'], [0xe2, 'c645fc00'],
                [0xfc, 'c745fc02000000'], [0x12c, '8937895decc645fc00']]) {
                assert.equal(image.codeAt(va(rva + offset), bytes.length / 2).toString('hex'), bytes);
            }
            candidate.cleanupActions = rebuiltStates.map((state, index) => {
                const action = funclets[index];
                const originalHex = action.asm.map(row => row[1]).join('');
                const originalAction = Buffer.from(originalHex, 'hex');
                const jumpOffset = index === 0 ? 19 : 3;
                assert.equal(originalAction[jumpOffset], 0xe9);
                const target = parseInt(action.entry, 16) + jumpOffset + 5 + originalAction.readInt32LE(jumpOffset + 1);
                assert.equal(target, va(index === 1 ? 0x15f0 : 0x1ef0));
                const destructor = ['??1PublishedSchedulerContextReference@@QAE@XZ',
                    '??1UnconstructedSchedulerContextStorage@@QAE@XZ', '??1SchedulerContextReference@@QAE@XZ'][index];
                assert.deepEqual(state.relocations, [{ offset: 4, type: 20, target: destructor }]);
                const targetSymbol = coff.unique(destructor);
                const targetSection = coff.sections[targetSymbol.section - 1];
                const targetBytes = object.subarray(targetSection.raw, targetSection.raw + targetSection.size);
                assert.equal(targetSymbol.value, 0);
                assert.equal(targetBytes.toString('hex'), [
                    '8079040074148b01c64104008b0085c074088b088b510450ffd2c3',
                    '8b0185c0740750e80000000059c3',
                    '8b0185c074088b088b510450ffd2c3',
                ][index]);
                assert.deepEqual(targetSection.relocations.map(rel => [rel.offset, rel.type, rel.symbol.name]),
                    index === 1 ? [[8, 20, '_ReleaseSchedulerContextStorage']] : []);
                return { entry: action.entry, status: 'Diagnostic', originalHex, rebuiltHex: state.bytes,
                    rebuilt: state.asm, originalTarget: hex(target), rebuiltTarget: destructor,
                    rebuiltTargetHex: targetBytes.toString('hex') };
            });
        }
        if (source === group.fallback) {
            assert.deepEqual(states.map(state => state.next), [-1, 0, 0]);
            assert.deepEqual(rebuiltStates.map(state => state.next), [-1, 0, 0]);
            assert.equal(object.readUInt32LE(infoSection.raw + infoEntry.value + 12), 0);
            const parent = coff.unique(owner);
            const parentSection = coff.sections[parent.section - 1];
            const parentBytes = object.subarray(parentSection.raw, parentSection.raw + parentSection.size);
            for (const [offset, target, bytes] of [
                [0x67, '_FindNamedSchedulerContext@12', '8b3089b53cffffff85f6c645fc01'],
                [0xce, '_CreateNamedSchedulerContext@12', '8b3889bd3cffffff85ffc645fc02'],
            ]) {
                assert.equal(parentBytes[offset], 0xe8);
                assert(parentSection.relocations.some(rel => rel.offset === offset + 1 && rel.symbol.name === target));
                assert.equal(parentBytes.subarray(offset + 5, offset + 5 + bytes.length / 2).toString('hex'), bytes);
            }
            for (const [address, state, target] of [[0x1d13, 1, 0x17d0], [0x1da4, 2, 0x1670]]) {
                const call = image.codeAt(va(address), 13);
                assert.equal(call[0], 0xe8);
                assert.equal(va(address + 5) + call.readInt32LE(1), va(target));
                assert.equal(call.subarray(5).toString('hex'), 'c68424800000000' + state);
            }
            for (const offset of [0x88, 0xfd]) {
                assert.equal(parentBytes.subarray(offset, offset + 4).toString('hex'), 'c645fc00');
            }
            const destructor = coff.unique('??1SchedulerContextReference@@QAE@XZ');
            const destructorSection = coff.sections[destructor.section - 1];
            assert.equal(object.subarray(destructorSection.raw, destructorSection.raw + destructorSection.size)
                .toString('hex'), '8b0185c074088b088b510450ffd2c3');
            assert.deepEqual(destructorSection.relocations, []);
            candidate.cleanupActions = rebuiltStates.map((state, index) => {
                const action = funclets[index];
                const originalHex = action.asm.map(row => row[1]).join('');
                const originalAction = Buffer.from(originalHex, 'hex');
                assert.equal(originalAction[3], 0xe9);
                const target = parseInt(action.entry, 16) + 8 + originalAction.readInt32LE(4);
                assert.equal(target, va(0x1ef0));
                assert.equal(state.bytes, index === 0 ? '8d4dbce900000000' : '8d8d3cffffffe900000000');
                assert.deepEqual(state.relocations,
                    [{ offset: index === 0 ? 4 : 7, type: 20, target: destructor.name }]);
                return { entry: action.entry, status: 'Diagnostic', originalHex, rebuiltHex: state.bytes,
                    rebuilt: state.asm, originalTarget: hex(target), rebuiltTarget: destructor.name };
            });
        }
        if (handler === 0x7284 || handler === 0x70b4) {
            const isConstructor = handler === 0x70b4;
            assert.deepEqual(states.map(state => state.next), [-1, 0, 1]);
            assert.deepEqual(rebuiltStates.map(state => state.next), [-1, 0, 1]);
            const parent = coff.unique(owner);
            const parentSection = coff.sections[parent.section - 1];
            const parentBytes = object.subarray(parentSection.raw, parentSection.raw + parentSection.size);
            const stateWrites = isConstructor ? [
                [0x60, 0x5c, '897dfc', '897dfc'],
                [0x9b, 0x9a, 'c645fc02', 'c645fc02'],
            ] : [
                [0x35, 0x3d, 'c745fc02000000', 'c745fc02000000'],
                [0x58, 0x66, 'c645fc01', 'c645fc01'],
                [0xac, 0xad, '885dfc', '885dfc'],
                [0xe1, 0xde, 'c745fcffffffff', 'c745fcffffffff'],
            ];
            for (const [originalOffset, rebuiltOffset, originalCode, rebuiltCode] of stateWrites) {
                assert.equal(image.codeAt(va(rva + originalOffset), originalCode.length / 2).toString('hex'), originalCode);
                assert.equal(parentBytes.subarray(rebuiltOffset, rebuiltOffset + rebuiltCode.length / 2)
                    .toString('hex'), rebuiltCode);
            }
            if (isConstructor) {
                assert.equal(image.codeAt(va(0x283f), 6).toString('hex'), 'ff15f410cc20');
                assert(parentSection.relocations.some(rel => rel.offset === 0xa0
                    && rel.symbol.name === '__imp__CopySchedulerContextName@12'));
                assert.equal(image.codeAt(va(0x2875), 2).toString('hex'), 'ffd1');
                assert.equal(parentBytes.subarray(0xd2, 0xd4).toString('hex'), 'ffd1');
                const factoryOwner = '_ConstructSchedulerContext@8';
                const factoryInfo = coff.unique('__ehfuncinfo$' + factoryOwner);
                const factoryInfoSection = coff.sections[factoryInfo.section - 1];
                assert.equal(object.readUInt32LE(factoryInfoSection.raw + factoryInfo.value + 4), 4);
                const factoryMap = coff.unique('__unwindtable$' + factoryOwner);
                const factoryMapSection = coff.sections[factoryMap.section - 1];
                assert.deepEqual(Array.from({ length: 4 }, (_, index) =>
                    object.readInt32LE(factoryMapSection.raw + factoryMap.value + index * 8)), [-1, 0, 1, 2]);
                for (let index = 0; index < 4; index++) {
                    assert(factoryMapSection.relocations.some(rel => rel.offset === factoryMap.value + index * 8 + 4
                        && rel.type === 6 && rel.symbol.name === '__unwindfunclet$' + factoryOwner + '$' + index));
                }
                const firstAction = coff.unique('__unwindfunclet$' + factoryOwner + '$0');
                const firstActionSection = coff.sections[firstAction.section - 1];
                assert(firstActionSection.relocations.some(rel => rel.offset === firstAction.value + 9
                    && rel.type === 20 && rel.symbol.name === '??3@YAXPAX0@Z'));
                const placementDelete = coff.unique('??3@YAXPAX0@Z');
                const deleteSection = coff.sections[placementDelete.section - 1];
                assert.equal(placementDelete.value, 0);
                assert.equal(object.subarray(deleteSection.raw, deleteSection.raw + deleteSection.size).toString('hex'), 'c3');
                assert.deepEqual(deleteSection.relocations, []);
                candidate.inlinedFactoryUnwind = { states: [-1, 0, 1, 2], placementDeleteHex: 'c3' };
            } else {
                const fallbackCall = image.codeAt(va(0x298d), 5);
                assert.equal(fallbackCall[0], 0xe8);
                assert.equal(va(0x2992) + fallbackCall.readInt32LE(1), va(0x1cc0));
                assert(parentSection.relocations.some(rel => rel.offset === 0x4e
                    && rel.symbol.name === '_UpdateSimConnectFallback@8'));
            }
            candidate.cleanupActions = rebuiltStates.map((state, index) => {
                const action = funclets[index];
                const originalHex = action.asm.map(row => row[1]).join('');
                const originalAction = Buffer.from(originalHex, 'hex');
                const jumpOffset = index === 2 ? 9 : 6;
                assert.equal(originalAction[jumpOffset], 0xe9);
                const target = parseInt(action.entry, 16) + jumpOffset + 5 + originalAction.readInt32LE(jumpOffset + 1);
                assert.equal(target, va(0x28f0));
                const member = ['SchedulerLiveEntryList', 'SchedulerEntryIndex', 'SchedulerPendingQueue'][index];
                const destructor = coff.unique('??1' + member + '@@QAE@XZ');
                const targetSection = coff.sections[destructor.section - 1];
                const targetBytes = object.subarray(targetSection.raw, targetSection.raw + targetSection.size);
                assert.equal(destructor.value, 0);
                assert.equal(targetBytes.toString('hex'),
                    '8bff558bec51568bf18b460c85c07437f6460a017431837e0400742b833e008945fc7e06c706000000008d45fc50ff150000000066834e0a01c7460c00000000c74604000000005e8be55dc3');
                assert.deepEqual(targetSection.relocations.map(rel => [rel.offset, rel.type, rel.symbol.name]),
                    [[0x30, 6, '__imp__ReleaseSchedulerContainerStorage@4']]);
                assert.equal(state.bytes, (isConstructor ? '8b4df0' : '8b4dec')
                    + ['83c160', '83c170', '81c180000000'][index] + 'e900000000');
                assert.deepEqual(state.relocations, [{ offset: jumpOffset + 1, type: 20, target: destructor.name }]);
                if (isConstructor) {
                    const inlined = coff.unique('__unwindfunclet$_ConstructSchedulerContext@8$' + (index + 1));
                    const inlinedSection = coff.sections[inlined.section - 1];
                    const offset = inlinedSection.raw + inlined.value;
                    assert.equal(object.subarray(offset, offset + state.bytes.length / 2).toString('hex'), state.bytes);
                    assert(inlinedSection.relocations.some(rel => rel.offset === inlined.value + jumpOffset + 1
                        && rel.type === 20 && rel.symbol.name === destructor.name));
                }
                return { entry: action.entry, status: 'Diagnostic', originalHex, rebuiltHex: state.bytes,
                    rebuilt: state.asm, originalTarget: hex(target), rebuiltTarget: destructor.name,
                    rebuiltTargetHex: targetBytes.toString('hex') };
            });
        }
        if (source === group.name) {
            const lookup = handler === 0x7061;
            const transitions = lookup ? [-1, 0] : [-1];
            assert.deepEqual(states.map(state => state.next), transitions);
            assert.deepEqual(rebuiltStates.map(state => state.next), transitions);
            assert.equal(object.readUInt32LE(infoSection.raw + infoEntry.value + 12), 0);
            const parent = coff.unique(owner);
            const parentSection = coff.sections[parent.section - 1];
            const parentBytes = object.subarray(parentSection.raw, parentSection.raw + parentSection.size);
            const checkpoints = lookup ? [
                [0x6e, 0x63, 'c745fc01000000', 'c645fc01'],
                [0xb0, 0xb0, '8937c745f001000000c645fc00', '8933c645c401c645fc00'],
                [0xc5, 0xc2, 'ffd2', 'ffd1'],
            ] : [
                [0x2a, 0x48, 'c745fc00000000', '895dfc'],
                [0x3e, 0x4b, 'ffd1', 'ffd1'],
                [0x92, 0x9d, 'c745fcffffffff', 'c745fcffffffff'],
                [0x9f, 0xa4, 'ffd0', 'ffd2'],
            ];
            for (const [originalOffset, rebuiltOffset, originalCode, rebuiltCode] of checkpoints) {
                assert.equal(image.codeAt(va(rva + originalOffset), originalCode.length / 2).toString('hex'), originalCode);
                assert.equal(parentBytes.subarray(rebuiltOffset, rebuiltOffset + rebuiltCode.length / 2)
                    .toString('hex'), rebuiltCode);
            }
            candidate.cleanupActions = rebuiltStates.map((state, index) => {
                const action = funclets[index];
                const originalHex = action.asm.map(row => row[1]).join('');
                const originalAction = Buffer.from(originalHex, 'hex');
                const published = lookup && index === 0;
                const jumpOffset = published ? 19 : 3;
                assert.equal(originalAction[jumpOffset], 0xe9);
                const target = parseInt(action.entry, 16) + jumpOffset + 5 + originalAction.readInt32LE(jumpOffset + 1);
                assert.equal(target, va(0x1ef0));
                const destructor = coff.unique(published
                    ? '??1PublishedSchedulerContextReference@@QAE@XZ' : '??1SchedulerContextReference@@QAE@XZ');
                assert.equal(state.bytes, (lookup ? (published ? '8d4dc0' : '8d4dcc') : '8d4dc8') + 'e900000000');
                assert.deepEqual(state.relocations, [{ offset: 4, type: 20, target: destructor.name }]);
                const destructorSection = coff.sections[destructor.section - 1];
                assert.equal(object.subarray(destructorSection.raw, destructorSection.raw + destructorSection.size)
                    .toString('hex'), published ? '8079040074148b01c64104008b0085c074088b088b510450ffd2c3'
                        : '8b0185c074088b088b510450ffd2c3');
                assert.deepEqual(destructorSection.relocations, []);
                return { entry: action.entry, status: 'Diagnostic', originalHex, rebuiltHex: state.bytes,
                    rebuilt: state.asm, originalTarget: hex(target), rebuiltTarget: destructor.name };
            });
        }
        candidates.push(candidate);
    }
    report.push({ entry: hex(entry), originalSHA256, funcInfo: hex(va(info)), unwindMap: hex(map),
        states, funclets, asm, handlerHex: left.toString('hex'), candidates });
}
fs.writeFileSync(artifact + '/eh-funclets-comparison.json', JSON.stringify(report, null, 2) + '\n');
