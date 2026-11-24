function importObjects(json, module, folder) {
  const globals = new Map();
  const types = new Map();
  const functionData = new Map();
  const normalizeAddress = value => String(value).replace(/^0x/i, '').toLowerCase();

  function addGlobal(item) {
    const entry = normalizeAddress(item.entry);
    if (!/^[\da-f]{8}$/.test(entry)) throw Error('Invalid global address in ' + module + ': ' + item.entry);
    const previous = globals.get(entry);
    globals.set(entry, {
      group: 'Global data', kind: 'Global data', status: 'Inventoried',
      name: 'Address ' + entry, ...previous, ...item, entry,
      notes: item.notes || previous?.notes || [], asm: [],
    });
  }

  function addType(item) {
    if (typeof item.path !== 'string' || !item.path.startsWith('/')) {
      throw Error('Expected full type path in ' + module + ': ' + item.path);
    }
    types.set(item.path, {
      name: item.path.split('/').pop(), kind: 'Type', status: 'Inventoried',
      ...types.get(item.path), ...item, entry: item.path, group: item.kind || 'Type',
      notes: item.notes || types.get(item.path)?.notes || [], asm: [],
    });
  }

  if (module === 'api.dll') {
    const evidence = 'artifacts/api.dll/api-source-pass-20261107/global-types.json';
    const observed = new Map();
    for (const [fn, symbols] of Object.entries(json(evidence) || {})) {
      for (const symbol of symbols) {
        const entry = normalizeAddress(symbol.address);
        if (!/^[\da-f]{8}$/.test(entry)) continue;
        if (!observed.has(entry)) observed.set(entry, { names: new Set(), types: new Map(), functions: new Set() });
        const variants = observed.get(entry);
        variants.names.add(symbol.name);
        variants.types.set(symbol.type + ' · ' + symbol.size + ' bytes', symbol.size);
        variants.functions.add(fn);
        if (!functionData.has(fn)) functionData.set(fn, new Set());
        functionData.get(fn).add(entry);
      }
    }
    for (const [entry, variants] of observed) {
      const sizes = new Set(variants.types.values());
      addGlobal({
        entry, name: [...variants.names].sort((a, b) => Number(a.startsWith('DAT_')) - Number(b.startsWith('DAT_')))[0],
        typeViews: [...variants.types.keys()].sort(),
        bytes: sizes.size === 1 ? [...sizes][0] : undefined,
        functions: [...variants.functions].sort(), evidence, status: 'Observed references',
        description: 'Decompiler observations per referring function. Access direction and object extent require separate evidence.',
      });
    }
    const tableFile = 'artifacts/api.dll/api-boundary-types-20261103.json';
    const table = json(tableFile);
    if (table?.program === module) {
      addType({
        path: '/ApplicationVTable', kind: 'Struct', bytes: table.size,
        fields: table.fields, evidence: tableFile, status: 'Partial layout',
        description: 'Historical Ghidra type snapshot. Open slots are absent from the stored export.',
        notes: [{ description: 'Stored table layout and separately checked target signatures.', file: 'notes/re/api.dll/api-application-boundary.md' }],
      });
    }
  }

  if (module === 'fsx.exe') {
    const evidence = 'artifacts/fsx.exe/data-constants-comparison.json';
    const report = json(evidence);
    for (const item of report?.Strings || []) {
      addGlobal({
        entry: item.OriginalVA, name: item.Text, bytes: item.Bytes,
        group: 'String literals', status: item.RawExactMatch ? 'Exact initial bytes' : 'Inventoried',
        initialValue: JSON.stringify(item.Text), evidence, description: report.Scope,
      });
    }
    for (const item of report?.ZeroInitializedSpans || []) {
      addGlobal({
        entry: item.OriginalVA, name: item.Role, bytes: item.Bytes,
        group: 'Initial state', status: item.InitialZeroMatch ? 'Verified initial zeros' : 'Inventoried',
        initialValue: 'Zero-initialized span', evidence, description: 'Minimum observed span. ' + report.Scope,
      });
    }
    for (const item of report?.CrtInitialStates || []) {
      addGlobal({
        entry: item.OriginalVA, name: item.Role, bytes: 4,
        group: 'Initial state', initialValue: '0x' + item.InitialDword,
        status: 'Documented initial value', evidence,
      });
    }
    if (report?.TokenPointerGroup) {
      const item = report.TokenPointerGroup;
      addGlobal({
        entry: item.OriginalVA, name: 'MFC token pointer group', bytes: item.Bytes, group: 'Pointer tables',
        status: item.TargetsVerified ? 'Targets verified' : 'Inventoried',
        description: item.Entries + ' observed entries. ' + report.Scope, evidence,
      });
    }
    if (report?.LoadConfiguration) {
      const item = report.LoadConfiguration;
      addGlobal({
        entry: item.OriginalVA, name: 'PE load configuration', bytes: item.Bytes, group: 'PE metadata',
        status: item.TargetNormalizedExactMatch ? 'Normalized' : 'Inventoried',
        description: report.Scope, evidence,
      });
    }
    const vtableFile = 'artifacts/fsx.exe/vtables/application-vtable.json';
    const table = json(vtableFile);
    if (table?.Slots?.length) {
      addGlobal({
        entry: table.OriginalTable, name: 'Application virtual table', kind: 'VTable', group: 'VTables',
        status: 'Observed slots compared', bytes: table.Slots.length * 4,
        evidence: vtableFile, description: table.Scope,
        slots: table.Slots.map(slot => ({
          offset: parseInt(slot.Offset, 16), target: normalizeAddress(slot.OriginalTarget),
          name: slot.Role, status: slot.Status,
        })),
      });
    }
  }

  const stem = module.replace(/\.[^.]+$/, '');
  const inventory = 'artifacts/' + module + '/' + stem + '-inventory/';
  for (const item of json(inventory + 'data.json') || []) addGlobal({ evidence: inventory + 'data.json', ...item });
  for (const item of json(inventory + 'types.json') || []) addType({ evidence: inventory + 'types.json', ...item });
  for (const item of json(folder + '/' + stem + '-dashboard-data.json') || []) addGlobal(item);
  for (const item of json(folder + '/' + stem + '-dashboard-types.json') || []) addType(item);
  if (module === 'fsx.exe') {
    const readOnlyLayout = json('artifacts/fsx.exe/rdata-order-20261118/rdata-layout.json');
    for (const item of readOnlyLayout?.Objects || []) addGlobal(item);
  }

  return {
    globals: [...globals.values()].sort((a, b) => a.entry.localeCompare(b.entry)),
    types: [...types.values()].sort((a, b) => a.entry.localeCompare(b.entry)),
    functionData,
  };
}

module.exports = { importObjects };
