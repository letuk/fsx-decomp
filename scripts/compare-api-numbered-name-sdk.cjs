const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const {createHash} = require('node:crypto');
const {readPE32Imports} = require('./lib/pe-imports.cjs');

const run = process.argv[2];
assert(run, 'Pass the linked SDK probe run directory');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
function image(file) {
    const data = fs.readFileSync(file);
    const pe = data.readUInt32LE(60);
    assert.equal(data.readUInt16LE(pe + 4), 0x14c);
    assert(data.readUInt16LE(pe + 22) & 0x2000, 'Expected a DLL');
    const base = data.readUInt32LE(pe + 52);
    const table = pe + 24 + data.readUInt16LE(pe + 20);
    function bytes(va, size) {
        const rva = va - base;
        for (let i = 0; i < data.readUInt16LE(pe + 6); i++) {
            const section = table + i * 40;
            const delta = rva - data.readUInt32LE(section + 12);
            if (delta < 0 || delta + size > data.readUInt32LE(section + 16)) continue;
            const start = data.readUInt32LE(section + 20) + delta;
            assert(start + size <= data.length);
            return data.subarray(start, start + size);
        }
        throw Error('Unmapped VA ' + va.toString(16));
    }
    const directory = bytes(base + data.readUInt32LE(pe + 120), 40);
    function ordinal(n) {
        const slot = n - directory.readUInt32LE(16);
        assert(slot >= 0 && slot < directory.readUInt32LE(20));
        return base + bytes(base + directory.readUInt32LE(28) + slot * 4, 4).readUInt32LE();
    }
    return {bytes, ordinal, hash: sha(data), imports: readPE32Imports(data)};
}

const original = image(path.join(__dirname, '../original/api.dll'));
const rebuilt = image(path.join(run, 'api-pilot.dll'));
assert.equal(original.hash, '9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'latin1');
function body(symbol, size, pe = rebuilt, disassembly = listing, returnCount = 2) {
    const lines = disassembly.split(/\r?\n/);
    const label = lines.indexOf(symbol + ':');
    assert(label >= 0);
    const rows = [];
    let length = 0;
    for (let i = label + 1; size === undefined || length < size; i++) {
        const match = lines[i]?.match(/^\s*([0-9A-F]{8}):\s+((?:[0-9A-F]{2} )*[0-9A-F]{2})\s{2,}(.+)$/);
        assert(match, 'Missing instruction at ' + symbol);
        const [va, hex, instruction] = match.slice(1);
        const continuation = lines[i + 1]?.match(/^\s+((?:[0-9A-F]{2} )*[0-9A-F]{2})\s*$/);
        const bytes = Buffer.from((hex + (continuation ? continuation[1] : '')).replaceAll(' ', ''), 'hex');
        if (continuation) i++;
        const address = parseInt(va, 16);
        if (rows.length) assert.equal(address, parseInt(rows[0][0], 16) + length);
        assert.deepEqual(pe.bytes(address, bytes.length), bytes);
        rows.push([va.toLowerCase(), bytes.toString('hex'), instruction.trim()]);
        length += bytes.length;
        if (size === undefined && rows.filter(row => /^ret\b/.test(row[2])).length === returnCount) break;
    }
    if (size !== undefined) assert.equal(length, size);
    assert.match(rows.at(-1)[2], /^ret\b/);
    return {symbol, rows, length, address: parseInt(rows[0][0], 16)};
}

const formatter = body('?StringCchPrintfA@@YAJPADIPBDZZ', 92);
const formatterOriginal = 0x200fb1ab;
const crt = {dll: 'msvcr80.dll', name: '_vsnprintf'};
const records = [];
function compare(entry, candidate, size, mappings, extra = {}) {
    const left = original.bytes(entry, size);
    const right = rebuilt.bytes(candidate.address, size);
    const normalized = Buffer.from(right);
    for (const offset of mappings) left.copy(normalized, offset, offset, offset + 4);
    assert.deepEqual(normalized, left, candidate.symbol);
    records.push({entry: entry.toString(16), rebuiltEntry: candidate.address.toString(16),
        symbol: candidate.symbol, size, mappedOffsets: mappings, leftHex: left.toString('hex'),
        rightHex: right.toString('hex'), rebuilt: candidate.rows, ...extra});
}
assert.deepEqual(original.imports.get(original.bytes(formatterOriginal + 52, 4).readUInt32LE()), crt);
assert.deepEqual(rebuilt.imports.get(rebuilt.bytes(formatter.address + 52, 4).readUInt32LE()), crt);
compare(formatterOriginal, formatter, 92, [52], {import: crt});

for (const spec of [
    {ordinal: 207, entry: 0x2013241b, symbol: '_FormatNumberedNamePilot@16', size: 38, literalOffset: 21, callOffset: 26, format: '%s.%d'},
    {ordinal: 208, entry: 0x20132441, symbol: '_FormatDoubleNumberedNamePilot@20', size: 42, literalOffset: 25, callOffset: 30, format: '%s.%d.%d'}
]) {
    const {ordinal, entry, symbol, size, literalOffset, callOffset, format} = spec;
    const candidate = body(symbol, size);
    assert.equal(original.ordinal(ordinal), entry);
    assert.equal(rebuilt.ordinal(ordinal), candidate.address);
    for (const [pe, va, callee] of [[original, entry, formatterOriginal], [rebuilt, candidate.address, formatter.address]]) {
        const literal = pe.bytes(va + literalOffset, 4).readUInt32LE();
        assert.equal(pe.bytes(literal, format.length + 1).toString('ascii'), format + '\0');
        assert.equal(pe.bytes(va + callOffset - 1, 1)[0], 0xe8);
        assert.equal(va + callOffset + 4 + pe.bytes(va + callOffset, 4).readInt32LE(), callee);
    }
    compare(entry, candidate, size, [literalOffset, callOffset], {ordinal, format});
}
const result = {originalSHA256: original.hash, linkedDllSHA256: rebuilt.hash, functions: records};
if (process.argv[3]) {
    const controlRun = process.argv[3];
    const control = image(path.join(controlRun, 'api-pilot.dll'));
    const controlListing = fs.readFileSync(path.join(controlRun, 'worker-output/disassembly.txt'), 'latin1');
    const integrated = /^_AllocateNumberedNamePilot@8:/m.test(listing);
    const symbol = integrated ? '_AllocateNumberedNamePilot@8' : '_AllocateNumberedNameSdkProbe@8';
    const candidate = body(symbol);
    const controlSymbol = /^_AllocateNumberedNamePilot@8:/m.test(controlListing) ?
        '_AllocateNumberedNamePilot@8' : '_AllocateNumberedNameSdkProbe@8';
    const baseline = body(controlSymbol, undefined, control, controlListing);
    const entry = 0x201322e2;
    assert.equal(original.ordinal(206), entry);
    assert.equal(rebuilt.ordinal(206), candidate.address);
    assert.equal(control.ordinal(206), baseline.address);
    const left = original.bytes(entry, 313);
    const right = rebuilt.bytes(candidate.address, candidate.length);
    const start = 0x30;
    const end = 0x6c;
    const mappedOffsets = [0x36, 0x43];
    for (const [pe, va, bytes, target] of [
        [original, entry, left, formatterOriginal],
        [rebuilt, candidate.address, right, formatter.address]
    ]) {
        assert.equal(pe.bytes(bytes.readUInt32LE(0x36), 4).toString('ascii'), '.%d\0');
        assert.equal(va + 0x47 + bytes.readInt32LE(0x43), target);
    }
    const normalized = Buffer.from(right);
    for (const offset of mappedOffsets) left.copy(normalized, offset, offset, offset + 4);
    assert.deepEqual(normalized.subarray(start, end), left.subarray(start, end));

    const calls = item => item.rows.filter(row => /^call\b/.test(row[2])).map(row => row[2]);
    const optimizedCalls = calls(candidate);
    const controlCalls = calls(baseline);
    for (const helper of ['AllocatePrivateHeapMemoryPilot', 'AppendResolvedPathPilot']) {
        assert(controlCalls.some(call => call.includes(helper)));
        assert(!optimizedCalls.some(call => call.includes(helper)));
    }
    const heapLoad = candidate.rows.find(row => /mov\s+ebx,.*__imp__HeapAlloc@12/.test(row[2]));
    assert(heapLoad);
    assert.deepEqual(rebuilt.imports.get(Buffer.from(heapLoad[1], 'hex').readUInt32LE(2)),
        {dll: 'kernel32.dll', name: 'HeapAlloc'});
    assert.equal(optimizedCalls.filter(call => /^call\s+ebx$/.test(call)).length, 2);
    assert.equal(optimizedCalls.filter(call => call.includes('_CopyResolvedPathPilot@12')).length, 2);
    const indirectImports = (rows, pe) => rows
        .filter(row => /^(?:call|jmp)\s+dword ptr \[__imp__/.test(row[2]))
        .map(row => pe.imports.get(Buffer.from(row[1], 'hex').readUInt32LE(2)));
    const heapFailureImports = [0x200e18d8, 0x200e1a04, 0x200e1b8c, 0x200e1720, 0x200e1848]
        .map(address => original.imports.get(address));
    assert.deepEqual(indirectImports(candidate.rows, rebuilt), heapFailureImports);
    result.allocatedName = {entry: entry.toString(16), ordinal: 206, symbol,
        originalBytes: left.length, rebuiltBytes: right.length, controlBytes: baseline.length,
        controlDllSHA256: control.hash, rebuiltEntry: candidate.address.toString(16),
        leftHex: left.toString('hex'), rightHex: right.toString('hex'), rebuilt: candidate.rows,
        matchedRange: {start, end, mappedOffsets}, optimizedCalls, controlCalls, heapFailureImports,
        scope: 'Linked diagnostic. The SDK formatting and length scan range matches with two target mappings. Allocation and copy call flow is verified; full body differs.'};
    if (integrated) {
        const copy = body('_CopyResolvedPathPilot@12', undefined, rebuilt, listing, 3);
        const originalCopy = 0x20137e17;
        const copyCalls = candidate.rows.filter(row => row[2].includes('call') && row[2].includes(copy.symbol));
        for (const [pe, callAddresses, target] of [
            [original, [0x201323d6, 0x20132402], originalCopy],
            [rebuilt, copyCalls.map(row => parseInt(row[0], 16)), copy.address]
        ]) {
            for (const address of callAddresses) {
                const instruction = pe.bytes(address, 5);
                assert.equal(instruction[0], 0xe8);
                assert.equal(address + 5 + instruction.readInt32LE(1), target);
            }
        }
        const imports = [0x200e1098, 0x200e1850, 0x200e1094].map(address => original.imports.get(address));
        assert.deepEqual(indirectImports(copy.rows, rebuilt), imports);
        result.copy = {entry: originalCopy.toString(16), rebuiltEntry: copy.address.toString(16),
            originalBytes: 169, rebuiltBytes: copy.length, imports,
            leftHex: original.bytes(originalCopy, 169).toString('hex'),
            rightHex: rebuilt.bytes(copy.address, copy.length).toString('hex'), rebuilt: copy.rows,
            scope: 'Linked copy dependency with verified call targets and imports. Instruction selection and body size differ.'};
    }
    if (/^_DuplicateHeapStringPilot@4:/m.test(listing)) {
        const duplicate = body('_DuplicateHeapStringPilot@4', undefined, rebuilt, listing, 2);
        const originalEntry = 0x20137efe;
        assert.equal(original.ordinal(158), originalEntry);
        assert.equal(rebuilt.ordinal(158), duplicate.address);
        const originalBody = original.bytes(originalEntry, 178);
        const rebuiltBody = rebuilt.bytes(duplicate.address, duplicate.length);
        let prefix = 0;
        while (prefix < Math.min(originalBody.length, rebuiltBody.length) &&
            originalBody[prefix] === rebuiltBody[prefix]) prefix++;
        assert(prefix >= 29);
        const map = fs.readFileSync(path.join(run, 'worker-output/fsx.map'), 'utf8');
        function mapSymbol(name) {
            const parts = map.split(/\r?\n/).map(line => line.trim().split(/\s+/))
                .filter(columns => columns[1] === name);
            assert.equal(parts.length, 1, name);
            return parseInt(parts[0][2], 16);
        }
        const originalFailure = original.bytes(0x20137f59, 67);
        assert.deepEqual(originalFailure, original.bytes(0x2013238d, 67));
        assert.deepEqual(originalFailure, original.bytes(0x20135e3b, 67));
        const allocator = body('?AllocatePrivateHeapMemoryPilot@@YGHHKPAPAXH@Z', undefined, rebuilt, listing, 2);
        const registrySymbol = '?RegisterCallbackListOwnerBoundaryPilot@@YGHPAPAUCallbackListDataBoundaryPilot@@P6AXKK@ZHKK@Z';
        const registryLabel = listing.indexOf(registrySymbol + ':');
        const registryBlock = registryLabel < 0 ? '' : listing.slice(registryLabel).split(/\r?\n(?=\S[^\r\n]*:\s*(?:\r?\n|$))/)[0];
        const registry = registryBlock ? body(registrySymbol, undefined, rebuilt, listing,
            (registryBlock.match(/\bret\s+(?:10|14)h\b/g) || []).length) : null;
        if (registry) assert.deepEqual(originalFailure, original.bytes(0x200f64ab, 67));
        const matches = [duplicate, candidate, allocator, ...(registry ? [registry] : [])].map(fn => {
            const errorReference = fn.rows.find(row => /mov\s+eax,.*g_errorResourceInstancePilot/.test(row[2]));
            assert(errorReference, 'Inlined failure path missing');
            const address = parseInt(errorReference[0], 16);
            const linked = rebuilt.bytes(address, originalFailure.length);
            const normalized = Buffer.from(linked);
            const mappings = [];
            for (const [offset, originalTarget, symbol] of [
                [1, 0x2020e028, '?g_errorResourceInstancePilot@@3PAUHINSTANCE__@@A'],
                [22, 0x2020d770, '?g_heapErrorTextPilot@@3PADA'],
                [40, 0x2020d770, '?g_heapErrorTextPilot@@3PADA']
            ]) {
                assert.equal(originalFailure.readUInt32LE(offset), originalTarget);
                assert.equal(linked.readUInt32LE(offset), mapSymbol(symbol));
                originalFailure.copy(normalized, offset, offset, offset + 4);
                mappings.push({offset, target: symbol});
            }
            for (const offset of [12, 31, 48, 56, 63]) {
                assert.equal(linked.readUInt16LE(offset - 2), 0x15ff);
                const originalImport = original.imports.get(originalFailure.readUInt32LE(offset));
                const linkedImport = rebuilt.imports.get(linked.readUInt32LE(offset));
                assert(originalImport);
                assert.deepEqual(linkedImport, originalImport);
                originalFailure.copy(normalized, offset, offset, offset + 4);
                mappings.push({offset, target: originalImport});
            }
            assert.deepEqual(normalized, originalFailure);
            return {function: fn.symbol, address: address.toString(16), bytes: linked.length, mappings};
        });
        const heapCalls = calls(duplicate).filter(call => /^call\s+ebx$/.test(call));
        assert.equal(heapCalls.length, 2);
        assert(!calls(duplicate).some(call => call.includes('AllocatePrivateHeapMemoryPilot')));
        const allocLoad = duplicate.rows.find(row => /mov\s+ebx,.*__imp__HeapAlloc@12/.test(row[2]));
        assert(allocLoad);
        assert.deepEqual(rebuilt.imports.get(Buffer.from(allocLoad[1], 'hex').readUInt32LE(2)),
            {dll: 'kernel32.dll', name: 'HeapAlloc'});
        assert.deepEqual(indirectImports(duplicate.rows, rebuilt).at(-1), {dll: 'kernel32.dll', name: 'lstrcpyA'});
        function matchComplete(originalVA, expectedSize, fn, originalFailureVA, fields, reportDifferences = false) {
            if (fn.length !== expectedSize) return null;
            const expected = original.bytes(originalVA, expectedSize);
            const linked = rebuilt.bytes(fn.address, fn.length);
            const normalized = Buffer.from(linked);
            const mappings = [];
            if (originalFailureVA !== null) {
                const fail = matches.find(row => row.function === fn.symbol);
                const failOffset = originalFailureVA - originalVA;
                if (parseInt(fail.address, 16) - fn.address !== failOffset) return null;
                mappings.push(...fail.mappings.map(field => ({...field, offset: field.offset + failOffset})));
            }
            for (const field of mappings)
                expected.copy(normalized, field.offset, field.offset, field.offset + 4);
            for (const [operandVA, kind, originalTarget, target] of fields) {
                const offset = operandVA - originalVA;
                const left = expected.readUInt32LE(offset);
                const right = linked.readUInt32LE(offset);
                if (kind === 'rel') {
                    if (linked[offset - 1] !== 0xe8 ||
                        operandVA + 4 + (left | 0) !== originalTarget ||
                        fn.address + offset + 4 + (right | 0) !== mapSymbol(target)) return null;
                } else if (kind === 'global') {
                    if (left !== originalTarget || right !== mapSymbol(target)) return null;
                } else if (kind === 'literal') {
                    const literal = Buffer.from(target + '\0', 'ascii');
                    if (left !== originalTarget ||
                        !original.bytes(left, literal.length).equals(literal) ||
                        !rebuilt.bytes(right, literal.length).equals(literal)) return null;
                } else {
                    if (left !== originalTarget || original.imports.get(left)?.name !== target ||
                        rebuilt.imports.get(right)?.name !== target ||
                        rebuilt.imports.get(right)?.dll !== original.imports.get(left)?.dll) return null;
                }
                expected.copy(normalized, offset, offset, offset + 4);
                mappings.push({offset, target});
            }
            if (!normalized.equals(expected)) {
                if (!reportDifferences) return null;
                return {bytes: expectedSize, mappings: mappings.sort((a, b) => a.offset - b.offset),
                    differences: Array.from(normalized.keys()).filter(offset => normalized[offset] !== expected[offset])};
            }
            return {bytes: expectedSize, mappings: mappings.sort((a, b) => a.offset - b.offset)};
        }
        const duplicateMatch = matchComplete(originalEntry, 178, duplicate, 0x20137f59, [
            [0x20137f23, 'global', 0x2020e054, '?g_heapInitializedPilot@@3HA'],
            [0x20137f2c, 'rel', 0x2013623e, '?InitializeHeapManagerPilot@@YAHXZ'],
            [0x20137f32, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA'],
            [0x20137f38, 'import', 0x200e19a8, 'HeapAlloc'],
            [0x20137f4a, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA'],
            [0x20137fa3, 'import', 0x200e1990, 'lstrcpyA']
        ]);
        const allocatedMatch = matchComplete(entry, 313, candidate, 0x2013238d, [
            [0x201322eb, 'global', 0x201f6284, '___security_cookie'],
            [0x20132308, 'rel', 0x20148b71, '@__security_check_cookie@4'],
            [0x20132318, 'literal', 0x200f30fc, '.%d'],
            [0x20132325, 'rel', formatterOriginal, formatter.symbol],
            [0x20132350, 'global', 0x2020e054, '?g_heapInitializedPilot@@3HA'],
            [0x2013235f, 'rel', 0x2013623e, '?InitializeHeapManagerPilot@@YAHXZ'],
            [0x20132365, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA'],
            [0x2013236c, 'import', 0x200e19a8, 'HeapAlloc'],
            [0x2013237d, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA'],
            [0x201323d7, 'rel', 0x20137e17, '_CopyResolvedPathPilot@12'],
            [0x20132403, 'rel', 0x20137e17, '_CopyResolvedPathPilot@12'],
            [0x20132411, 'rel', 0x20148b71, '@__security_check_cookie@4']
        ]);
        result.allocatedName.completeMatch = allocatedMatch;
        const allocatorMatch = matchComplete(0x20135dd9, 174, allocator, 0x20135e3b, [
            [0x20135ddf, 'global', 0x2020e054, '?g_heapInitializedPilot@@3HA'],
            [0x20135de8, 'rel', 0x2013623e, '?InitializeHeapManagerPilot@@YAHXZ'],
            [0x20135e00, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA'],
            [0x20135e06, 'import', 0x200e19a8, 'HeapAlloc'],
            [0x20135e29, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA']
        ]);
        result.allocator = {entry: '20135dd9', rebuiltEntry: allocator.address.toString(16),
            originalBytes: 174, rebuiltBytes: allocator.length, completeMatch: allocatorMatch,
            leftHex: original.bytes(0x20135dd9, 174).toString('hex'),
            rightHex: rebuilt.bytes(allocator.address, allocator.length).toString('hex'), rebuilt: allocator.rows};
        const initializer = body('?InitializeHeapManagerPilot@@YAHXZ', undefined, rebuilt, listing, 2);
        const initMatch = matchComplete(0x2013623e, 144, initializer, null, [
            [0x20136244, 'global', 0x2020e054, '?g_heapInitializedPilot@@3HA'],
            [0x20136259, 'import', 0x200e19b0, 'HeapCreate'],
            [0x2013625f, 'import', 0x200e19b4, 'HeapSetInformation'],
            [0x2013626d, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA'],
            [0x2013627c, 'import', 0x200e1994, 'GetProcessHeap'],
            [0x20136294, 'import', 0x200e109c, '_get_heap_handle'],
            [0x201362b1, 'global', 0x2020e058, '?g_heapInitCallCountPilot@@3HA'],
            [0x201362b6, 'global', 0x2020e054, '?g_heapInitializedPilot@@3HA'],
            [0x201362c3, 'global', 0x2020e058, '?g_heapInitCallCountPilot@@3HA']
        ]);
        result.initializer = {entry: '2013623e', rebuiltEntry: initializer.address.toString(16),
            originalBytes: 144, rebuiltBytes: initializer.length, completeMatch: initMatch,
            leftHex: original.bytes(0x2013623e, 144).toString('hex'),
            rightHex: rebuilt.bytes(initializer.address, initializer.length).toString('hex'), rebuilt: initializer.rows};
        const reallocSymbol = '?ReallocatePrivateHeapMemoryBoundaryPilot@@YGHHKPAPAX@Z';
        if (listing.includes(reallocSymbol + ':')) {
            const fn = body(reallocSymbol, undefined, rebuilt, listing, 3);
            const match = matchComplete(0x20135e98, 185, fn, null, [
                [0x20135e9e, 'global', 0x2020e054, '?g_heapInitializedPilot@@3HA'],
                [0x20135ea7, 'rel', 0x2013623e, initializer.symbol],
                [0x20135ec1, 'rel', 0x20135dd9, allocator.symbol],
                [0x20135ecc, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA'],
                [0x20135edb, 'import', 0x200e19a4, 'HeapReAlloc'],
                [0x20135ef9, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA'],
                [0x20135f05, 'import', 0x200e19a4, 'HeapReAlloc'],
                [0x20135f11, 'global', 0x2020e028, '?g_errorResourceInstancePilot@@3PAUHINSTANCE__@@A'],
                [0x20135f1b, 'global', 0x2020d770, '?g_heapErrorTextPilot@@3PADA'],
                [0x20135f24, 'import', 0x200e1a04, 'LoadStringA'],
                [0x20135f2c, 'global', 0x2020d770, '?g_heapErrorTextPilot@@3PADA'],
                [0x20135f33, 'import', 0x200e1b8c, 'MessageBoxA'],
                [0x20135f3b, 'import', 0x200e1720, 'GetCurrentProcess'],
                [0x20135f42, 'import', 0x200e1848, 'TerminateProcess']
            ]);
            result.reallocator = {entry: '20135e98', rebuiltEntry: fn.address.toString(16), originalBytes: 185,
                rebuiltBytes: fn.length, completeMatch: match, rebuilt: fn.rows,
                leftHex: original.bytes(0x20135e98, 185).toString('hex'),
                rightHex: rebuilt.bytes(fn.address, fn.length).toString('hex')};
            if (registry) {
                const call = registry.rows.find(row => row[2].startsWith('call') && row[2].includes(reallocSymbol));
                assert(call);
                const callAddress = parseInt(call[0], 16);
                const target = (pe, address) => {
                    assert.equal(pe.bytes(address, 1)[0], 0xe8);
                    return address + 5 + pe.bytes(address + 1, 4).readInt32LE();
                };
                assert.equal(target(original, 0x200f6526), 0x20135e98);
                assert.equal(target(rebuilt, callAddress), fn.address);
                assert.equal(calls(registry).filter(text => /^call\s+edi$/.test(text)).length, 2);
                assert(!calls(registry).some(text => text.includes(allocator.symbol)));
                result.registry = {entry: '200f6451', rebuiltEntry: registry.address.toString(16),
                    originalBytes: 318, rebuiltBytes: registry.length,
                    leftHex: original.bytes(0x200f6451, 318).toString('hex'),
                    rightHex: rebuilt.bytes(registry.address, registry.length).toString('hex'),
                    rebuilt: registry.rows, failureMatch: matches.find(row => row.function === registrySymbol),
                    reallocatorCall: {original: '200f6526', rebuilt: call[0]},
                    scope: 'Linked probe with different owner ABI. The 67-byte allocation failure block matches after address mapping.'};
                const matchedLayout = matchComplete(0x200f6451, 318, registry, 0x200f64ab, [
                    [0x200f646b, 'global', 0x2020e054, '?g_heapInitializedPilot@@3HA'],
                    [0x200f6474, 'rel', 0x2013623e, initializer.symbol],
                    [0x200f647a, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA'],
                    [0x200f6481, 'import', 0x200e19a8, 'HeapAlloc'],
                    [0x200f6497, 'global', 0x2020e050, '?g_privateHeapPilot@@3PAXA'],
                    [0x200f6527, 'rel', 0x20135e98, reallocSymbol]
                ], true);
                result.registry.completeMatch = matchedLayout?.differences ? null : matchedLayout;
                if (matchedLayout?.differences) result.registry.normalizedDiagnostic = matchedLayout;
                if (listing.includes('_RegisterSimpleCallbackProbe@8:')) {
                    assert.equal(registry.rows[3][1], '85f6');
                    assert(registry.rows.filter(row => /^ret\b/.test(row[2])).every(row => row[1] === 'c21000'));
                    result.registry.scope = 'Internal C++ probe reproduces ESI owner and RET 0x10. Full body differs. The 67-byte failure block matches.';
                    if (result.registry.completeMatch) result.registry.scope = 'Complete linked internal C++ body matches after verified address mappings.';
                    result.registry.callers = [
                        [0x200f63df, 36, '_RegisterSimpleCallbackProbe@8', 0x200f63f5, 0x200f63fa],
                        [0x200f6403, 40, '_RegisterContextCallbackProbe@16', 0x200f641d, 0x200f6422]
                    ].map(([address, size, symbol, tableField, callField]) => {
                        const caller = body(symbol, size);
                        const completeMatch = matchComplete(address, size, caller, null, [
                            [tableField, 'global', 0x2020bb30, '_g_registryProbeOwners'],
                            [callField, 'rel', 0x200f6451, registrySymbol]
                        ]);
                        assert(completeMatch, symbol + ' caller match failed');
                        return {entry: address.toString(16), rebuiltEntry: caller.address.toString(16), symbol,
                            originalBytes: size, completeMatch, rebuilt: caller.rows,
                            leftHex: original.bytes(address, size).toString('hex'),
                            rightHex: rebuilt.bytes(caller.address, size).toString('hex')};
                    });
                }
            }
        }
        result.duplicate = {entry: originalEntry.toString(16), ordinal: 158, symbol: duplicate.symbol,
            originalBytes: originalBody.length, rebuiltBytes: rebuiltBody.length,
            rebuiltEntry: duplicate.address.toString(16), rawEntryMatchBytes: prefix,
            originalFailure: {entry: '20137f59', numberedNameEntry: '2013238d', bytes: originalFailure.length},
            failureMatches: matches, leftHex: originalBody.toString('hex'), rightHex: rebuiltBody.toString('hex'),
            completeMatch: duplicateMatch, rebuilt: duplicate.rows,
            scope: duplicateMatch ? 'Complete linked body matches after verified address mappings.' :
                'Raw entry and 67-byte target-validated failure path match. Full body differs.'};
    }
}
if (process.argv.includes('--require-allocator-match')) {
    assert.equal(result.allocator?.completeMatch?.bytes, 174, 'Standalone allocator match regressed');
    console.log('Complete standalone allocator matches: 174 bytes.');
}
if (process.argv.includes('--require-initializer-match')) {
    assert.equal(result.initializer?.completeMatch?.bytes, 144, 'Heap initializer match regressed');
    console.log('Complete heap initializer matches: 144 bytes.');
}
if (process.argv.includes('--require-reallocator-match')) {
    assert.equal(result.reallocator?.completeMatch?.bytes, 185, 'Heap reallocator match regressed');
    console.log('Complete heap reallocator matches: 185 bytes.');
}
if (process.argv.includes('--require-heap-matches')) {
    assert.equal(result.duplicate?.completeMatch?.bytes, 178, 'DuplicateHeapStringPilot full match regressed');
    assert.equal(result.allocatedName?.completeMatch?.bytes, 313, 'AllocateNumberedNamePilot full match regressed');
    console.log('Two complete linked heap caller bodies match: 491 bytes.');
}
fs.writeFileSync(path.join(run, 'numbered-name-sdk-comparison.json'), JSON.stringify(result, null, 2) + '\n');
console.log('Three complete linked bodies match: 172 bytes, five validated address fields.');
