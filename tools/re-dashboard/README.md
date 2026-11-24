# FSX decompilation dashboard

From the project root, regenerate the local dashboard with Node.js:

```powershell
node tools/re-dashboard/generate.cjs
Invoke-Item tools/re-dashboard/dist/index.html
```

The generated page works offline and needs no server or package installation.
Regenerate it after new analysis or comparison runs. It reads the retained
evidence and writes only the dashboard HTML.

The URL fragment retains the active module, view, search, filters, page,
selected entry and listing mode. Reload or reopen that URL to restore the view.

The **Reconstructed source** section links to the current workspace source.
**Function identity** lists the original module and address alongside the
available Ghidra name, reconstructed symbol and distinct build symbol.
Search accepts each of those names. For a manually mapped function, set
`ghidraName` from a verified export or listing if it is available, and include
`symbol` in the `source` record.
The generator resolves compiler symbols and API source-pass candidates to
unique definitions in the module's source directories. For ambiguous or
renamed symbols, set `source: { "file": "src/path.cpp", "line": 42 }` on the
function entry in the module dashboard JSON. The line is optional. Retained
comparison evidence continues to identify the historical build.
For unique definitions, use `source: { "symbol": "FunctionName" }` to
resolve the current file and line automatically. Optional `relatedSymbols`
lists additional definitions, such as an inlined constructor.

The module status chart counts all entries in the module's Functions view,
including documented addresses and exports. Click a legend entry to filter
by that status.

The Status filter shows the comparison result when one is available.
`Documented` identifies entries with a description or topic notes and no
comparison result. Search includes topic note titles.

The documented-source review in
`artifacts/<binary-name>/dashboard-documented-source.json` supplies
source locations and records unresolved lookups. An instruction inside a
larger body can link to its enclosing source function. These links preserve
the existing comparison status.

For `fsx.exe`, verified instruction ranges within a linked function
can supply scoped comparison records through
`artifacts/fsx.exe/documented-body-comparisons.json`.

When both instruction listings are present, the Disassembly section opens
with a side-by-side alignment. Equal instruction bytes, differences,
one-sided instructions and validated relocation fields have separate labels.
If a function has comparisons for individual address ranges, the
**Listing scope** selector opens them directly on the function page.
The first scoped rebuild is selected when the function has only an
original listing of its full body. The selector also offers the full
original listing. **Build disassembly file** opens the retained dumpbin
output when the comparison provides its path.
Listings with unavailable bytes can show text similarity. The alignment is
heuristic, and the comparison report remains the source of the match status.
Choose **Raw listings** to see each listing in its original order.

## Global data and types

Under **View**, choose **Global data** or **Types** in the selected module.
The generator imports API decompiler global observations and references,
the saved API ApplicationVTable layout, and EXE initial-state, literal,
PE data and virtual-table evidence. Conflicting decompiler observations
are shown together. References from the API snapshot link both ways
between the data entry and the referring functions. EXE VTable slots link
to inventoried functions when available. Type snapshots preserve the
recorded extent and evidence status.

For new exports, provide JSON arrays in `artifacts/<binary-name>/<module>-inventory/data.json`
and `types.json`. The module stem is the binary name without its extension.
Optional module notes under `notes/re/<binary-name>/` can supplement those
records with `<module>-dashboard-data.json` and
`<module>-dashboard-types.json`. Records merge by original VA or full type path.

- Data records use `entry` (eight hexadecimal digits), `name`, `kind`,
  `status`, `bytes`, `typePath`, `initialValue`, or `originalHex` as applicable.
  A VTable can include `slots` with `offset`, `name`, `signature`, `target`
  (original function VA) and `status`.
- Type records use `path` (full Ghidra path), `kind` (for example Struct, Class
  or Enum), `status` and optional `bytes`. Layouts use `fields` with `offset`,
  `name`, `type`, optional `typePath`, and `size`. Enums use `values` with
  `name` and `value`. `functions` and `globals` can link to related addresses.
- Either record can have `description`, a project-relative `evidence` path and
  `notes` with `description` and `file`. Record whether a size is a minimum
  extent, and use a status that reflects the available evidence. If there is a
  verified data comparison, use `comparison` with `status`, `scope`,
  `evidence` and optional `leftHex` and `rightHex`.

## Evidence sources

- `fsx.exe`: EXE inventory, Ghidra listing export and the baseline 41 range
  ledger. Original and reconstructed bytes are read from the hash-checked PE
  files. A scoped match applies only to the stated range.
- `api.dll`: API exports, saved Ghidra model bodies and the most recent dated
  source-pass comparison record per address. The byte view uses listings that
  reproduce the body hashes in the comparison report. COFF model-body matches
  have their own scope.
- `g2d.dll`: selected exports, addresses from topic tables and explicitly
  mapped compile results. Documented addresses can also be call sites or data.
- `gps.dll`: partial Ghidra inventory of provider exports, Release and
  legacy data callbacks. Includes COFF diagnostics for registration and
  Release, and a target-validated comparison for ordinal 2.
- `sim1.dll`: all stored internal Ghidra functions, PE ordinals and named
  exports, reviewed subsystem anchors and the existing flight-state
  comparisons. External declarations and IAT entries are in the linked
  [inventory notes](../../notes/re/sim1.dll/sim1-function-inventory.md).
- Topic table descriptions appear alongside the recorded entries. Unnamed
  addresses and inferred groups remain provisional. The inventory counts
  include exports and documented addresses as well as stored functions.

The raw byte view compares offsets. The reported match status comes from the
evidence source. The viewer does not calculate instruction alignment or certify
relocation targets.

## Adding results

Place a new module's notes in `notes/re/<binary-name>/`. The dashboard discovers
module directories and reads address tables there. To import a full Ghidra
inventory, use `artifacts/<binary-name>/<module>-inventory/manifest.json` with the same
`program`, `functions`, `entry`, `name`, `bodyBytes` and `signature` fields
as the existing API source-pass manifest. Optional `functions/<entry>.asm`
files use one `address  bytes  instruction` line per instruction.
For compiled counterparts, retain the original listing in that inventory or in
the module JSON's `asm` field when the verified listing is available. The
worker's listing supplies the rebuilt side only.
Export the original listing via Ghidra MCP for the exact module and entry.
After regenerating, check both sides. The generator warns when a rebuilt
listing has no original listing. If the export is unavailable, add the reason
to the comparison scope.

For named groups, compile-only progress or comparison formats without an
adapter, add a small `<module>-dashboard-functions.json` array in the module's
notes directory. Use hexadecimal VAs as strings and link the source evidence.
Use an optional `tags` string array for verified Ghidra function tags. The
function details display the tags and the main search uses them.
For example:

```json
[
  {
    "entry": "20680590",
    "name": "Shader binding lookup",
    "group": "D3D10 shader bindings",
    "comparison": {
      "status": "Compiled",
      "scope": "COFF build passed. Byte comparison remains open.",
      "evidence": "notes/re/g2d.dll/g2d-dxgi-shader-lookup.md",
      "listing": "vm-share/modules/g2d/runs/g2d-shader-lookup-20261114-001/worker-output/disassembly.txt",
      "symbol": "FindG2dShaderBindingPilot"
    }
  }
]
```

Use `Exact`, `Normalized`, `Different` or `Diagnostic` only with a comparator
report for the stated scope. Set `full: false` for a partial range. Optional
`asm` and `comparison.rebuilt` are arrays of
`[address, hexBytes, instruction]` rows. Optional `comparison.leftHex` and
`rightHex` are the verified raw byte ranges. `mappedOffsets` lists validated
four-byte relocation fields relative to the start of the range.
For a code range inside a function, use `kind: "Code range"` and
`parentEntry` with the enclosing function's original VA. Optional
`notes` entries provide a title, description and project-relative file link.
