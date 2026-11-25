const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { openImage } = require('./lib/pe32-dll.cjs');
const { parseNativeCoff } = require('./lib/coff.cjs');
const { readPE32Imports } = require('./lib/pe-imports.cjs');
const { compareBodies, nativeCoffBody, originalListing } = require('./lib/reconstruction-listings.cjs');

const [run, component] = process.argv.slice(2);
const names = {
    'turbine-controls': {
        '20c205d0': 'InitiateTurbineCombustion', '20c2bc90': 'CalculateThrottleInput',
        '20c2bdd0': 'UpdateGovernorTarget', '20c2beb0': 'UpdateFuelSchedule',
        '20c2bff0': 'CanSustainCombustion', '20c2c040': 'UpdateCombustion',
        '20c2c120': 'UpdateJetHighSpool', '20c2c230': 'UpdateJetLowSpool',
        '20c2c350': 'UpdateBleedFactor', '20c2c410': 'UpdateReverserPosition',
    },
    'turbine-indications': {
        '20c2c980': 'UpdatePressureRatio', '20c2caa0': 'UpdateTurbineTemperature',
        '20c2ce50': 'UpdateTurbineExhaustTemperature', '20c2cbe0': 'UpdateTurbineDriveSignal',
        '20c2cd80': 'UpdateTurbineOilTemperature', '20c2cf80': 'UpdateTurbineOilPressure',
    },
    'turbine-start': {
        '20c211c0': 'ToggleAutomaticEngineStart', '20c214b0': 'UpdateEngineStarterPower',
        '20c2a130': 'AdvanceEngineControl', '20c2d410': 'UpdateTurbineAutoStart',
    },
    'turbine-fuel': {
        '20bf77c0': 'UpdateTurbineFuelDelivery', '20bf79e0': 'SelectEngineFuelTanks',
        '20bf8170': 'CalculateTurbineFuelFlow',
    },
    'turbine-loop': { '20c2b6d0': 'UpdateTurbineEngines' },
};
assert(run && names[component], 'Pass the native VC8 turbine state run directory and its component name');
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const program = 'sim1.dll';
const binary = fs.readFileSync(path.join(__dirname, '../original', program));
const originalSHA256 = sha(binary);
assert.equal(originalSHA256, '18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8');
const image = openImage(binary);
const file = fs.readFileSync(path.join(run, `sim1-${component}.obj`));
const coff = parseNativeCoff(file);
const listing = fs.readFileSync(path.join(run, 'worker-output/disassembly.txt'), 'utf8');
const direct = Object.assign({}, ...Object.values(names), {
    '20bcc030': 'Evaluate', '20bcbf60': 'SampleAerodynamicCurve', '20c2bb20': 'AfterburnerFraction',
    '20bfee30': 'CircuitAvailability', '20c66fc2': '__CIsqrt', '20c2b9d0': 'UpdateInletConditions',
    '20c2c6d0': 'UpdateSpoolSpeed', '20c2c850': 'UpdateTurbineShaftTorque', '20c2c2e0': 'UpdateRamDrag',
    '20c2c620': 'PublishNetThrust', '20c2bc20': 'UpdateTurbineSpeedOutput',
});
const knownCalls = new Set(Object.values(direct));
const thunk = image.codeAt(0x20c66fc2, 6);
assert.equal(thunk.readUInt16LE(), 0x25ff);
const squareRootImport = readPE32Imports(binary).get(thunk.readUInt32LE(2));
assert.equal(squareRootImport.dll.toLowerCase(), 'msvcr80.dll');
assert.equal(squareRootImport.name, '_CIsqrt');
const globals = new Map([
    [0x20c7ba58, 'turbineRamDrag'], [0x20c7ba60, 'turbineGrossThrust'], [0x20c7ba68, 'turbineGovernorTarget'],
    [0x20c7ba70, 'turbineInletTemperatureRoot'], [0x20c7ba78, 'turbineInletPressureRatio'],
    [0x20c7ba80, 'turbineInletTemperatureRatio'],
]);

function readOriginalReferences(original) {
    return original.rows.flatMap(([address, hex, instruction]) => {
        const va = Number.parseInt(address, 16);
        const call = instruction.match(/^CALL 0x([0-9a-f]+)$/i);
        if (call) {
            const code = image.codeAt(va, 5);
            const target = va + 5 + code.readInt32LE(1);
            assert.equal(code[0], 0xe8);
            assert.equal(target, Number.parseInt(call[1], 16));
            return [{ offset: va + 1 - original.va, target, identity: direct[call[1]] }];
        }
        const addressOperand = instruction.match(/\[(0x[0-9a-f]+)\]/i);
        if (!addressOperand) return [];
        const target = Number(addressOperand[1]);
        const operand = va + hex.split(' ').length - 4;
        assert.equal(image.codeAt(operand, 4).readUInt32LE(), target);
        const width = /\bdouble ptr/i.test(instruction) ? 8 : /\bfloat ptr/i.test(instruction) ? 4 : 0;
        const identity = globals.get(target) || (width && target < 0x20c6e000 && /^F/.test(instruction)
            ? image.codeAt(target, width, false).toString('hex') : undefined);
        return [{ offset: operand - original.va, target, identity }];
    });
}

function readRebuiltReferences(body) {
    return body.section.relocations.map(({ offset, symbol, type }) => {
        if (body.bytes.readUInt32LE(offset) !== 0) return { offset, symbol: symbol.name };
        if (type === 20) {
            assert.equal(body.bytes[offset - 1], 0xe8);
            const name = symbol.name.match(/^\?(\w+)@/)?.[1] || symbol.name;
            if (name === 'Evaluate') {
                assert(symbol.name.startsWith('?Evaluate@?$TurbineCoefficientTable@'));
            }
            return { offset, symbol: symbol.name, identity: knownCalls.has(name) ? name : undefined };
        }
        if (type !== 6) return { offset, symbol: symbol.name };
        const name = symbol.name.match(/^\?(\w+)@sim1@@3NA$/)?.[1];
        if ([...globals.values()].includes(name)) return { offset, symbol: symbol.name, identity: name };
        const encoded = symbol.name.match(/^__real@([0-9a-f]{8}|[0-9a-f]{16})$/i);
        if (!encoded) return { offset, symbol: symbol.name };
        const size = encoded[1].length / 2;
        const section = coff.sections[symbol.section - 1];
        assert(section && symbol.value + size <= section.size);
        const bytes = file.subarray(section.raw + symbol.value, section.raw + symbol.value + size);
        assert(bytes.equals(Buffer.from(encoded[1], 'hex').reverse()));
        return { offset, symbol: symbol.name, identity: bytes.toString('hex') };
    });
}

function instructions(body, references, va) {
    return body.rows.map(row => {
        const [address, hex, instruction] = row;
        const begin = Number.parseInt(address, 16) - va;
        const bytes = Buffer.from(hex.replaceAll(' ', ''), 'hex');
        const refs = references.filter(ref => ref.offset >= begin && ref.offset < begin + bytes.length);
        if (refs.some(ref => !ref.identity) || /^(?:j\w+|loop\w*)\b/i.test(instruction) ||
            (/^call\b/i.test(instruction) && !refs.length)) {
            return { begin, bytes, refs, row };
        }
        const normalized = Buffer.from(bytes);
        for (const ref of refs) normalized.fill(0, ref.offset - begin, ref.offset - begin + 4);
        const key = [normalized.toString('hex'), ...refs.map(ref => [ref.offset - begin, ref.identity])].flat().join(':');
        return { begin, bytes, refs, row, key };
    });
}

// Find contiguous instruction ranges in order, excluding relative branches and unvalidated operands.
function matchingRanges(original, rebuilt, originalRefs, rebuiltRefs) {
    const left = instructions(original, originalRefs, original.va);
    const right = instructions(rebuilt, rebuiltRefs, 0);
    const ranges = [];
    function find(firstLeft, endLeft, firstRight, endRight) {
        let best;
        for (let i = firstLeft; i < endLeft; i++) {
            for (let j = firstRight; j < endRight; j++) {
                if (!left[i].key || left[i].key !== right[j].key) continue;
                if (i > firstLeft && j > firstRight && left[i - 1].key &&
                    left[i - 1].key === right[j - 1].key) continue;
                let count = 0;
                let size = 0;
                while (i + count < endLeft && j + count < endRight &&
                    left[i + count].key && left[i + count].key === right[j + count].key) {
                    size += left[i + count].bytes.length;
                    count++;
                }
                if ((count >= 2 && size >= 4) || (count === 1 && size === 5 && /^CALL /.test(left[i].row[2]))) {
                    if (!best || size > best.size) best = { i, j, count, size };
                }
            }
        }
        if (!best) return;
        const { i, j, count, size } = best;
        find(firstLeft, i, firstRight, j);
        const lhs = left.slice(i, i + count);
        const rhs = right.slice(j, j + count);
        const leftBytes = Buffer.concat(lhs.map(row => row.bytes));
        const rightBytes = Buffer.concat(rhs.map(row => row.bytes));
        const pairs = lhs.flatMap(row => row.refs.map(ref => ({ originalOffset: ref.offset - lhs[0].begin,
            rebuiltOffset: ref.offset - lhs[0].begin, identity: ref.identity, target: ref.target })));
        const result = compareBodies(leftBytes, rightBytes, pairs, true);
        assert.notEqual(result.status, 'Different');
        ranges.push({ entry: lhs[0].row[0], rebuiltEntry: rhs[0].row[0], bytes: size, ...result, pairs,
            asm: lhs.map(row => row.row), rebuilt: rhs.map(row => row.row),
            leftHex: leftBytes.toString('hex'), rightHex: rightBytes.toString('hex') });
        find(i + count, endLeft, j + count, endRight);
    }
    find(0, left.length, 0, right.length);
    return ranges;
}

const originals = {};
const functions = Object.entries(names[component]).map(([entry, name]) => {
    const original = originals[entry] = originalListing(program, entry, image);
    const rebuilt = nativeCoffBody(file, coff, listing, `?${name}@`);
    const originalRefs = readOriginalReferences(original);
    const rebuiltRefs = readRebuiltReferences(rebuilt);
    const scopes = matchingRanges(original, rebuilt, originalRefs, rebuiltRefs);
    const mapped = originalRefs.flatMap(ref => rebuiltRefs.some(other =>
        other.offset === ref.offset && ref.identity && other.identity === ref.identity)
        ? [{ originalOffset: ref.offset, rebuiltOffset: ref.offset, identity: ref.identity, target: ref.target }] : []);
    const result = compareBodies(original.bytes, rebuilt.bytes, mapped,
        mapped.length === originalRefs.length && mapped.length === rebuiltRefs.length);
    return { entry, name, ...result, originalBytes: original.bytes.length, rebuiltBytes: rebuilt.bytes.length,
        buildSymbol: rebuilt.symbol.name, originalRefs, rebuiltRefs, asm: original.rows, rebuilt: rebuilt.rows,
        leftHex: original.bytes.toString('hex'), rightHex: rebuilt.bytes.toString('hex'), scopes };
});

const scheduleSpecs = {
    'turbine-controls': [
        ['20c2c410', '20c2c450', '20c2c489', 0x50, 0x44, '_reverserPositionSchedule', [0.0]],
    ],
    'turbine-indications': [
        ['20c2c980', '20c2c98b', '20c2ca37', 0xac, 0xa8, '_pressureRatioSchedule', []],
        ['20c2caa0', '20c2caa8', '20c2cb14', 0x64, 0x54, '_turbineTemperatureSchedule', []],
        ['20c2ce50', '20c2ce58', '20c2cec1', 0x5c, 0x54, '_exhaustTemperatureSchedule', []],
        ['20c2cbe0', '20c2cbe8', '20c2cc30', 0x58, 0x44, '_driveSchedule', []],
        ['20c2cd80', '20c2cd8c', '20c2cde2', 0x50, 0x44, '_oilTemperatureSchedule', []],
    ],
};
const tables = (scheduleSpecs[component] || []).map(([entry, begin, end, base, size, name, stack]) => {
    const values = Buffer.alloc(size);
    const written = new Set();
    for (const [address, , instruction] of originals[entry].rows) {
        if (address < begin || address > end) continue;
        if (instruction === 'FLDZ') stack.unshift(0);
        else if (instruction === 'FLD1') stack.unshift(1);
        else if (instruction.startsWith('FLD double ptr')) {
            const literal = Number(instruction.match(/\[(0x[0-9a-f]+)\]/i)[1]);
            stack.unshift(image.codeAt(literal, 8).readDoubleLE());
        } else {
            const store = instruction.match(/^FST(P)? double ptr \[EBP \+ (-?0x[0-9a-f]+)\]$/i);
            const count = instruction.match(/^MOV dword ptr \[EBP \+ (-?0x[0-9a-f]+)\],(0x[0-9a-f]+)$/i);
            if (store) {
                const offset = base + (Number.parseInt(store[2], 16) | 0);
                assert(stack.length);
                values.writeDoubleLE(stack[0], offset);
                for (let i = offset; i < offset + 8; i++) written.add(i);
                if (store[1]) stack.shift();
            } else if (count) {
                const offset = base + (Number.parseInt(count[1], 16) | 0);
                values.writeUInt32LE(Number(count[2]), offset);
                for (let i = offset; i < offset + 4; i++) written.add(i);
            }
        }
    }
    assert.equal(written.size, size, name);
    const symbol = coff.unique(name);
    const data = coff.sections[symbol.section - 1];
    assert(data && symbol.value + size <= data.size);
    const rebuilt = file.subarray(data.raw + symbol.value, data.raw + symbol.value + size);
    assert(values.equals(rebuilt), name);
    return { entry, name, bytes: size, status: 'Exact', originalHex: values.toString('hex'), rebuiltHex: rebuilt.toString('hex') };
});
const report = { program, component, originalSHA256, objectSHA256: sha(file), functions, tables };
fs.writeFileSync(path.join(run, `${component}-comparison.json`), JSON.stringify(report, null, 2) + '\n');
for (const item of functions) {
    const exact = item.scopes.filter(scope => scope.status === 'Exact').reduce((sum, scope) => sum + scope.bytes, 0);
    const normalized = item.scopes.filter(scope => scope.status === 'Normalized').reduce((sum, scope) => sum + scope.bytes, 0);
    console.log(`${item.entry} ${item.name}: ${item.status}, ${item.originalBytes}/${item.rebuiltBytes} bytes. ${exact} exact and ${normalized} normalized scope bytes.`);
}
for (const table of tables) console.log(`${table.name}: ${table.bytes} exact table bytes`);
