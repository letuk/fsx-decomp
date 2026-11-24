const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const { importObjects } = require('./objects.cjs');

process.chdir(path.resolve(__dirname, '../..'));
const read = file => fs.existsSync(file) ? fs.readFileSync(file, 'utf8').replace(/^\uFEFF/, '') : '';
const json = file => JSON.parse(read(file) || 'null');
const list = dir => fs.existsSync(dir) ? fs.readdirSync(dir, { withFileTypes: true }) : [];
const hash = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const address = value => String(value).toLowerCase().replace(/^0x/, '').padStart(8, '0');
const modules = new Map();

function moduleFor(name) {
  if (!modules.has(name)) modules.set(name, new Map());
  return modules.get(name);
}

function row(module, entry) {
  const rows = moduleFor(module);
  const key = address(entry);
  if (!/^[0-9a-f]{8}$/.test(key)) throw Error('Invalid entry: ' + entry);
  if (!rows.has(key)) rows.set(key, { entry: key, kind: 'Function', notes: [], asm: [] });
  return rows.get(key);
}

function csv(file) {
  const lines = read(file).trim().split(/\r?\n/).filter(Boolean);
  function values(line) {
    return [...line.matchAll(/(?:^|,)(?:"((?:""|[^"])*)"|([^,]*))/g)]
      .map(match => match[1]?.replaceAll('""', '"') ?? match[2]);
  }
  const columns = values(lines.shift() || '');
  return lines.map(line => Object.fromEntries(values(line).map((value, i) => [columns[i], value])));
}

function dumpLine(line) {
  const match = line.match(/^\s*([\da-f]{8}): ((?:[\da-f]{2} )*[\da-f]{2})(?:\s{2,}(.*))?$/i);
  return match && [match[1].toLowerCase(), match[2].replaceAll(' ', '').toLowerCase(), match[3] || ''];
}

function objectListing(file) {
  const symbols = new Map();
  let current;
  let previousInstruction;
  for (const line of read(file).split(/\r?\n/)) {
    const header = line.match(/^(\S+)(?: \(.*\))?:$/);
    if (header) {
      current = [];
      symbols.set(header[1], current);
    }
    const instruction = dumpLine(line);
    if (instruction && current) current.push(instruction);
    // VC8 dumpbin wraps bytes beyond the first six onto unaddressed lines.
    const continuation = line.match(/^ {12}([\da-f]{2}(?: [\da-f]{2})*)\s*$/i);
    if (continuation && previousInstruction) {
      previousInstruction[1] += continuation[1].replaceAll(' ', '').toLowerCase();
    } else {
      previousInstruction = current ? instruction : null;
    }
  }
  return symbols;
}

function rawBytes(rows, length, base) {
  let next = base;
  let bytes = '';
  for (const [entry, hex] of rows) {
    if (parseInt(entry, 16) !== next) return null;
    bytes += hex;
    next += hex.length / 2;
  }
  return bytes.length === length * 2 ? bytes : null;
}

function peBytes(file, expectedHash) {
  if (!expectedHash || !fs.existsSync(file)) return () => null;
  const bytes = fs.readFileSync(file);
  if (hash(bytes) !== expectedHash.toLowerCase()) throw Error('Binary identity changed: ' + file);
  const pe = bytes.readUInt32LE(60);
  const optional = pe + 24;
  if (bytes.readUInt16LE(optional) !== 0x10b) throw Error('Expected PE32: ' + file);
  const base = bytes.readUInt32LE(optional + 28);
  const table = optional + bytes.readUInt16LE(pe + 20);
  const sections = Array.from({ length: bytes.readUInt16LE(pe + 6) }, (_, index) => {
    const offset = table + 40 * index;
    return {
      va: base + bytes.readUInt32LE(offset + 12),
      size: bytes.readUInt32LE(offset + 16),
      raw: bytes.readUInt32LE(offset + 20),
    };
  });
  return (va, length) => {
    const section = sections.find(item => va >= item.va && va + length <= item.va + item.size);
    if (!section || !Number.isInteger(length) || length <= 0) return null;
    const raw = section.raw + va - section.va;
    if (raw + length > bytes.length) throw Error('Invalid section extent: ' + file);
    return bytes.toString('hex', raw, raw + length);
  };
}

const exeLedger = 'artifacts/fsx.exe/coverage-baseline41-20261103.json';
const baseline = json(exeLedger);
const exeRead = peBytes('original/fsx.exe', baseline?.originalSHA256);
const rebuiltExeRead = peBytes('vm-share/modules/fsx/archive/41-rtm-mfc-support/fsx.exe', baseline?.rebuiltSHA256);

for (const item of csv('artifacts/fsx.exe/exe-inventory/inventory.csv')) {
  if (!/^[\da-f]{8}$/i.test(item.Address)) continue;
  Object.assign(row('fsx.exe', item.Address), {
    name: item.ModelName || item.ImportIdentity || item.Role,
    ghidraName: item.ModelName || undefined,
    kind: item.Origin,
    group: item.Category,
    description: [item.Role, item.Evidence].filter(Boolean).join(' · '),
    bytes: Number(item.Bytes),
    asm: (item.Instructions || '').split('; ').filter(Boolean).map(text => ['', '', text]),
  });
}

const exeExport = json('artifacts/fsx.exe/exe-inventory/ghidra-export.json');
for (const item of exeExport?.functions || []) {
  const entry = row('fsx.exe', item.entry);
  if (item.instructions?.length) {
    entry.asm = item.instructions.map(i => [
      i.address, exeRead(parseInt(i.address, 16), i.length) || '', i.text,
    ]);
  }
}

const exeDump = 'vm-share/modules/fsx/archive/41-rtm-mfc-support/disassembly.txt';
const exeListing = read(exeDump).split(/\r?\n/).map(dumpLine).filter(Boolean);
for (const claim of baseline?.claims || []) {
  if (claim.category !== 'executable code' || !claim.original || !claim.rebuilt) continue;
  const entry = moduleFor('fsx.exe').get(address(claim.original.va));
  if (!entry) continue;
  const full = claim.original.bytes === entry.bytes;
  if (entry.comparison?.full && !full) continue;
  const start = parseInt(claim.rebuilt.va, 16);
  const rebuilt = exeListing.filter(line => {
    const va = parseInt(line[0], 16);
    return va >= start && va < start + claim.rebuilt.bytes;
  });
  entry.comparison = {
    status: claim.status === 'exact raw' ? 'Exact' : claim.status === 'target-validated normalized'
      ? 'Normalized' : claim.status === 'known difference' ? 'Different' : 'Diagnostic',
    full, scope: claim.label + ' · ' + claim.original.bytes + ' original bytes · baseline 41',
    evidence: exeLedger, rebuilt, listing: exeDump,
    rightHex: rebuiltExeRead(start, claim.rebuilt.bytes),
    leftHex: exeRead(parseInt(entry.entry, 16), claim.original.bytes),
  };
}

function importInventory(dir) {
  const manifest = json(dir + '/manifest.json');
  if (!manifest?.program || !Array.isArray(manifest.functions)) return;
  for (const item of manifest.functions) {
    const entry = row(manifest.program.toLowerCase(), item.entry);
    const dump = dir + '/functions/' + item.entry + '.asm';
    const asm = read(dump).split(/\r?\n/).map(line =>
      line.match(/^([\da-f]+)[ \t]{2,}((?:[\da-f]{2} )*[\da-f]{2})[ \t]{2,}(.*)$/i) ||
      line.match(/^([\da-f]+)\s+([\da-f]+)\s+(.*)$/i))
      .filter(Boolean).map(match => match.slice(1));
    Object.assign(entry, { name: item.name, ghidraName: item.name, bytes: item.bodyBytes, signature: item.signature, kind: 'Stored Ghidra function' });
    for (const key of ['group', 'description', 'export', 'tags']) {
      if (item[key] !== undefined) entry[key] = item[key];
    }
    if (asm.length) Object.assign(entry, { asm, listing: dump });
  }
}

importInventory('artifacts/api.dll/api-source-pass-20261107');
for (const module of list('artifacts')) {
  if (!module.isDirectory() || !/\.(dll|exe)$/i.test(module.name)) continue;
  const folder = 'artifacts/' + module.name;
  for (const dir of list(folder)) {
    if (dir.isDirectory() && dir.name.endsWith('-inventory')) importInventory(folder + '/' + dir.name);
  }
}

for (const exp of csv('artifacts/api.dll/api-inventory/exports.csv')) {
  if (!exp.VA || exp.Forwarder) continue;
  const entry = row('api.dll', exp.VA);
  entry.name ||= exp.Name || 'Ordinal ' + exp.Ordinal;
  entry.export = exp.Name || 'Ordinal ' + exp.Ordinal;
  if (entry.kind === 'Function') entry.kind = 'Export address';
}
for (const exp of json('artifacts/g2d.dll/g2d-selected-exports-20261103.json')?.exports || []) {
  if (exp.forwarder) continue;
  Object.assign(row('g2d.dll', exp.va), { name: exp.name || 'Ordinal ' + exp.ordinal, kind: 'Export address', export: String(exp.ordinal) });
}

const apiRoot = 'vm-share/modules/api/runs';
const apiHash = json('artifacts/api.dll/api-inventory/original.json')?.SHA256?.toLowerCase();
const apiRuns = list(apiRoot).filter(dir => dir.isDirectory()).map(dir => dir.name)
  .sort((a, b) => (a.match(/\d{8}-\d+$/)?.[0] || '').localeCompare(b.match(/\d{8}-\d+$/)?.[0] || ''));
for (const name of apiRuns) {
  const dir = apiRoot + '/' + name;
  const evidence = dir + '/source-pass-comparison.json';
  const report = json(evidence);
  if (!Array.isArray(report?.rows) || report.originalSHA256?.toLowerCase() !== apiHash) continue;
  const dump = dir + '/worker-output/disassembly.txt';
  const symbols = objectListing(dump);
  for (const item of report.rows) {
    if (!item.entry || !item.symbol || typeof item.rawEqual !== 'boolean') continue;
    const entry = row('api.dll', item.entry);
    const compiled = symbols.get(item.symbol) || [];
    const left = rawBytes(entry.asm, item.originalBytes, parseInt(entry.entry, 16));
    const right = rawBytes(compiled, item.nativeBytes, 0);
    entry.name ||= item.symbol;
    entry.comparison = {
      status: item.rawEqual ? 'Exact' : item.normalizedMatch ? 'Normalized' : 'Different',
      symbol: item.symbol,
      full: true, scope: 'COFF model body · ' + item.originalBytes + ' original / ' + item.nativeBytes + ' rebuilt bytes',
      evidence, listing: dump, rebuilt: compiled,
      leftHex: left && hash(Buffer.from(left, 'hex')) === item.originalSHA256?.toLowerCase() ? left : null,
      rightHex: right && hash(Buffer.from(right, 'hex')) === item.nativeSHA256?.toLowerCase() ? right : null,
      mappedOffsets: (item.mappings || []).map(mapping => mapping.offset),
    };
  }
}

for (const dir of list('notes/re')) {
  const module = dir.name.toLowerCase();
  if (!dir.isDirectory() || !/\.(dll|exe)$/.test(module)) continue;
  moduleFor(module);
  const folder = 'notes/re/' + dir.name;
  for (const doc of list(folder).filter(file => file.name.endsWith('.md'))) {
    const file = folder + '/' + doc.name;
    const note = read(file);
    const title = note.match(/^# (.+)/m)?.[1] || doc.name;
    for (const line of note.split(/\r?\n/)) {
      const match = line.match(/^\|([^|]+)\|(.+)\|$/);
      if (!match) continue;
      const cells = match[1].replaceAll('`', '').trim();
      if (module === 'fsx.exe' && /^rebuilt\s/i.test(cells)) continue;
      const addresses = [...cells.matchAll(/(?:0x)?\b[\da-f]{8}\b/gi)]
        .filter(match => !(/^20\d{6}$/.test(match[0]) && cells[match.index - 1] === '-' &&
          cells[match.index + match[0].length] === '-'))
        .map(match => match[0]);
      if (!addresses.length) continue;
      for (const va of addresses) {
        const known = moduleFor(module).get(address(va));
        const entry = known || row(module, va);
        if (!known) entry.kind = 'Documented address';
        entry.notes.push({ title, file, description: match[2].split('|').map(cell => cell.trim()).join(' · ') });
      }
    }
  }
  for (const item of json(folder + '/' + module.replace(/\.[^.]+$/, '') + '-dashboard-functions.json') || []) {
    const entry = row(module, item.entry);
    for (const key of ['name', 'ghidraName', 'group', 'description', 'kind', 'bytes', 'asm', 'listing', 'comparison', 'source', 'parentEntry', 'tags']) {
      if (item[key] !== undefined) entry[key] = item[key];
    }
    entry.notes.push(...(item.notes || []));
    const comparison = item.comparison;
    if (comparison?.symbol && comparison.listing && !comparison.rebuilt?.length) {
      const symbols = objectListing(comparison.listing);
      const names = [...symbols.keys()].filter(name =>
        name === comparison.symbol || name.startsWith('?' + comparison.symbol + '@'));
      if (names.length !== 1) throw Error('Missing or ambiguous symbol: ' + comparison.symbol);
      entry.comparison = { ...comparison, rebuilt: symbols.get(names[0]) };
    }
  }
  const sourceLinks = json('artifacts/' + module + '/dashboard-documented-source.json') || [];
  for (const item of sourceLinks) {
    const entry = moduleFor(module).get(address(item.entry));
    if (!entry || entry.source) continue;
    if (item.source) {
      entry.source = item.source;
      entry.parentEntry ??= item.parentEntry;
    } else {
      entry.sourceAudit = item.result;
    }
  }
  const bodyEvidence = 'artifacts/' + module + '/documented-body-comparisons.json';
  const bodies = module === 'fsx.exe' ? json(bodyEvidence) : null;
  for (const item of bodies?.comparisons || []) {
    const entry = moduleFor(module).get(address(item.entry));
    if (!entry || entry.comparison) continue;
    entry.name ||= item.name;
    entry.ghidraName ||= item.ghidraName;
    entry.parentEntry ??= item.parentEntry;
    entry.bytes = item.bytes;
    entry.asm = item.asm;
    entry.comparison = {
      status: item.status, full: item.full !== false, scope: item.scope,
      evidence: item.evidence || bodyEvidence, parentEvidence: item.parentEvidence, listing: item.listing,
      symbol: item.symbol, rebuilt: item.rebuilt, leftHex: item.originalHex,
      rightHex: item.rebuiltHex, mappedOffsets: item.mappedOffsets,
    };
  }
}

const fsxLinkedComparisons = 'artifacts/fsx.exe/iat-complete-20261119/dashboard-comparisons.json';
for (const item of json(fsxLinkedComparisons) || []) {
  const entry = moduleFor('fsx.exe').get(address(item.entry));
  if (!entry?.comparison || entry.comparison.leftHex !== item.comparison.leftHex) {
    throw Error('FSX linked comparison coverage changed at ' + item.entry);
  }
  entry.asm = item.asm;
  entry.listing = item.listing;
  entry.comparison = item.comparison;
}

function sourceDefinitions(module) {
  const stem = module.replace(/\.[^.]+$/, '');
  const directories = module === 'fsx.exe' ? ['src/fsx'] : ['src/' + stem + '-pilot'];
  const definitions = new Map();
  for (const directory of directories) {
    for (const file of list(directory)) {
      if (!file.isFile() || !/\.(?:c|cpp|h|hpp)$/i.test(file.name)) continue;
      const source = directory + '/' + file.name;
      const body = read(source);
      const signatures = /^[ \t]*(?:extern[ \t]+"C"[ \t]+)?((?:(?:__declspec\([^()\r\n]*\)|[\w:*&<>]+)[ \t]+)*)([a-z_]\w*(?:::[a-z_]\w*)*)[ \t]*\([^;{}]*?\)\s*(?:const\s*)?\{/gim;
      for (const match of body.matchAll(signatures)) {
        const [, prefix, symbol] = match;
        if (!prefix.trim() && !symbol.includes('::')) continue;
        if (!definitions.has(symbol)) definitions.set(symbol, []);
        definitions.get(symbol).push({ file: source, line: body.slice(0, match.index).split('\n').length });
      }
      const constructors = /^[ \t]*([a-z_]\w*)::\1[ \t]*\([^;{}]*?\)\s*(?=[:{]|#if\b)/gim;
      for (const match of body.matchAll(constructors)) {
        const symbol = match[1] + '::' + match[1];
        const line = body.slice(0, match.index).split('\n').length;
        const matches = definitions.get(symbol) || [];
        if (!matches.some(item => item.file === source && item.line === line)) {
          definitions.set(symbol, [...matches, { file: source, line }]);
        }
      }
    }
  }
  return definitions;
}

const exeSourceSymbols = new Map([
  ['010019e0', 'FSXAppPilot::InitInstance'],
  ['01001d30', 'ShowErrorMessage'],
]);

function sourceReference(entry, module, definitions) {
  if (entry.source) {
    if (entry.source.symbol && !entry.source.file) {
      const resolve = symbol => {
        const matches = definitions.get(symbol);
        if (matches?.length !== 1) throw Error('Missing or ambiguous source definition: ' + symbol);
        return { ...matches[0], symbol };
      };
      return {
        ...resolve(entry.source.symbol),
        related: (entry.source.relatedSymbols || []).map(resolve),
      };
    }
    const file = entry.source.file;
    if (typeof file !== 'string' || !file.startsWith('src/') || path.posix.normalize(file) !== file ||
        !fs.statSync(file, { throwIfNoEntry: false })?.isFile()) throw Error('Invalid source reference: ' + file);
    return entry.source;
  }
  const raw = entry.comparison?.symbol || (module === 'fsx.exe' ? exeSourceSymbols.get(entry.entry) : undefined);
  if (!raw) return undefined;
  const cpp = raw.match(/^\?(\w+)@(\w+)@@/);
  const symbol = cpp ? cpp[2] + '::' + cpp[1] : raw.replace(/^[_@]/, '').replace(/@\d+$/, '');
  const candidates = definitions.get(symbol);
  return candidates?.length === 1 ? { ...candidates[0], symbol } : undefined;
}

const data = [...modules].sort(([a], [b]) => (a === 'fsx.exe' ? -1 : b === 'fsx.exe' ? 1 : a.localeCompare(b)))
  .map(([name, rows]) => {
    const { globals, types, functionData } = importObjects(json, name, 'notes/re/' + name);
    const definitions = sourceDefinitions(name);
    const functions = [...rows.values()].sort((a, b) => a.entry.localeCompare(b.entry)).map(entry => ({
      ...entry,
      source: sourceReference(entry, name, definitions),
      dataReferences: [...(functionData.get(entry.entry) || [])],
      name: entry.name || 'Address ' + entry.entry,
      group: entry.group || entry.notes[0]?.title || (entry.export ? 'Exports' : 'Ungrouped'),
    }));
    return { name, rows: functions, globals, types };
  });
const template = read(path.join(__dirname, 'index.html'));
const html = template.replace('<!-- DATA -->', () => JSON.stringify(data).replaceAll('<', '\\u003c'));
const output = path.join(__dirname, 'dist');
fs.mkdirSync(output, { recursive: true });
fs.writeFileSync(path.join(output, 'index.html'), html);
for (const module of data) {
  console.log(module.name + ': ' + module.rows.length + ' entries, ' +
    module.globals.length + ' data entries, ' + module.types.length + ' types, ' +
    module.rows.filter(entry => entry.comparison).length + ' comparison records, ' +
    module.rows.filter(entry => entry.comparison?.leftHex && entry.comparison?.rightHex).length + ' byte pairs');
  const missing = module.rows.filter(entry => entry.comparison?.rebuilt?.length && !entry.asm.length);
  if (missing.length) {
    console.warn(module.name + ': ' + missing.length + ' rebuilt listings lack an original listing (' +
      missing.slice(0, 8).map(entry => '0x' + entry.entry).join(', ') +
      (missing.length > 8 ? ', …' : '') + ').');
  }
}
console.log(path.join(output, 'index.html'));
