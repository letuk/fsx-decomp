# Getter compiler-boundary test and complete EXE inventory

2026-11-02. Work stopped at the user's usage-limit request after both requested
steps completed. No further build/search queued. XP worker remains ready.

## Getter boundary

Original/control: baseline27-mfc-windows-startup, deliberate non-GL getter TU.
Real XP/SP1 tests:
- getter-gl-20261102-001: same source hashes as27, getter TU compiled /GL.
- getter-sametu-20261102-001: definition included in application TU, separate
  getter object omitted. Source inclusion is experimental, not original packaging.
- getter-default-gl-20261102-001: selected clean default, separate ordinary /GL
  TU; obsolete unused RetainErrorCallbackPilot/retainedErrorCallback removed.

All13 body gates, startup and alias/OLE/EH checks pass. ShowErrorMessage remains
2 differences; InitInstance777 vs769 bytes,246 instructions/frame0x554 and43
non-branch positional differences. Thus the artificial boundary does not explain
the open encodings in these tested contexts. No original compiler/TU identity
is inferred. GetterCompilation GL is now default; NonGL and SameTU controls
remain explicit options. Canonical C++ getter file now contains only its dynamic
getter-pointer definition. Neither API DLL implementation nor stubs were run.

Golden baseline28-gl-getter-context, SHA256:
B255FB5ECFBCC848A1D3760EF64589B221949997C51E0689A956B2F0C3DEA6F4.
Sources/build command/log/EXE/PDB/MAP/receipt/reports retained. Default inventory
and replay fixture reference28. No candidate EXE executed.

## EXE inventory

Read-only scripts/ghidra/ExportExeInventory.java exported the actual current
fsx.exe model; scripts/inventory-exe.ps1 combines it with raw PE/import checks.
Artifacts: artifacts/fsx.exe/exe-inventory/inventory.html, inventory.csv, summary.json,
ghidra-export.json, import-thunks.csv, undefined-ranges.csv and pe-sections.csv.

Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.
Ghidra's188 function count is56 internal stored functions plus132 external
function declarations, NOT188 original local functions. Also exported174 existing
unassigned instructions in70 contiguous listing ranges. Such ranges are not
certified function boundaries and can be fragments of known routines.

Inventory has130 distinct-address rows mixing these origins:
-13 reconstructed/matched application or SDK-helper bodies;
-2 application near-matches;
-80 raw import thunks validated against actual IAT identities;
-23 runtime/framework regions (some provisional, clearly labeled);
-8 compiler/EH/static-object glue regions;
-2 SDK inline stub bodies;
-1 MFC startup wrapper;
-1 open listing fragment near ATL at01001f89.

These are NOT130 recovered functions. Three raw thunks were not even disassembled
in the model:0100282c (_XcptFilter),01002be0 (_invoke_watson),01002bec (_controlfp_s).
A52-byte CRT __allmul body at01002c50 is raw-exact against linked msvcrt:llmul.obj,
but lies in undefined listing bytes. AfxInitialize01002c0d is independently
identified by38 normalized bytes plus module-state/setmbcp import identities.
These library identifications are not added to the13 project/helper body count.
Other existing runtime names and inferred roles remain evidence-labeled, not
claimed original names or wholesale library-body matches.

Executable .text coverage:9728 bytes =4628 instruction bytes +3150 defined data
bytes +1950 undefined bytes. Fifteen undefined ranges contain bytes other than
00/CC; they include runtime code fragments, raw thunks, static records and EH data.
They remain separately listed; this inventory does not certify all code decoded.
Checks passed: coverage sum,130 unique-address rows,80 raw import thunks with3
not disassembled. No database edits/function creation/disassembly performed.

The original .rsrc has2723840 raw bytes,99.25% of the2744384-byte file. Original
sections are .text/.data/.rsrc; .text includes IAT/read-only data as well as code.
The current rebuild instead has separate .text/.rdata/.data. This is consistent
with original section merging/organization differences, not proof of a precise
/MERGE command. Resource size is file-layout evidence, not analysis completeness.

## Concrete next work after the cut

1. Review static-object glue outside vtables:01002ce0/01002d00 construction and
   atexit registration,01002d40/01002d50 destruction. Verify actual CRT initializer
   table order before changing declaration order; do not infer it from addresses.
2. Review executable undefined runtime ranges without indiscriminate autoanalysis.
3. Controlled section-layout/merging experiment and resource inventory before
   claiming whole-file matching. Then move to API/main DLL behavior as appropriate.

Do not spend more variants on the two near-matches without a concrete finding.
Ghidra fsx.exe was current (user-selected), api.dll also open. No agent navigation,
renames/types/functions, save or backup. Usage-limit request ends work here.
