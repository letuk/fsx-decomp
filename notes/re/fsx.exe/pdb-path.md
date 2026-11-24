# Basename-only CodeView PDB path

2026-11-03. Status: actual XP builds, source/command comparison, parsed PE debug
records, cross-directory repeat check and wrong-mode controls. Original fsx.exe
SHA256:61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Measured linker behavior

The installed SP1 linker accepts /PDBALTPATH:fsx.pdb and emits exactly the short
path fsx.pdb in the PE CodeView record. The existing /PDB:fsx.pdb still produces
the actual PDB artifact. No EXE/PDB postprocessing or binary patch was used.
The older saved RTM help did not list PDBALTPATH; installed-linker support is
established by actual output, not inferred from that help. Logs have no ignored-
option LNK4044 warning, and the new gate requires the actual intended result.

Original and new record agree in Type2, RSDS signature, Age1,32-byte terminated
extent and basename fsx.pdb. Data RVA1518/file offset0x918 also agree in this
merged layout. PDB GUID and timestamps remain different; no original PDB/debug
identity match or unique original-command recovery is claimed.

## Controls and repeat stability

Actual jobs pdb-basename-20261103-001 and
pdb-basename-repeat-with-longer-id-20261103-001 both pass all prior regressions
and InitInstance diagnostics. The latter runs with the new default and new debug
gate. Source hashes match baseline35 and each other. The sole command delta
against35 is /PDBALTPATH:fsx.pdb; both basename build commands are identical.
Original worker directories differ in name and length, but the embedded path
and its32-byte extent no longer depend on those directories.

Across the two basename EXEs only18 raw bytes differ. All occur in three parsed
metadata fields: four-byte COFF timestamp, four-byte IMAGE_DEBUG_DIRECTORY
timestamp and16-byte RSDS GUID. Not every byte in those24 field bytes differs.
Outside those fields the entire EXE files are byte-equal. Field locations are
derived from PE/debug structures and required to agree before the comparison;
no arbitrary difference-based masking is used. This is a bounded repeat result,
not proof of deterministic whole builds or equality to the original EXE.

Rebuilt .text virtual size is now9622 versus original9654 (-32); .data remains
1204 versus1184 (+20). Raw sizes/file offsets still match the non-resource
original. Entry is now0x010025fb versus original0x0100262e (-51). These are
layout/extent observations, not counts of mismatching bytes or a uniform offset
for all functions. Earlier positive virtual-size deltas and entry distances
were specific to the older embedded worker paths and are superseded for36.

Import content/hints pass;128 import entry positions remain reordered. Known
ShowErrorMessage two-byte difference and InitInstance769/777,246 instructions,
43 nonbranch differences remain. Existing18/1114 body count is unchanged.

## Gate, baseline and persistence

verify-debug-record.ps1 enforces requested Worker or Basename mode, one Type2
RSDS/Age1 record, exact terminated path extent and actual PDB artifact presence.
It separately reports original GUID/timestamp and data/file-location equality.
comparison.json now contains PdbDebug. Actual35 is rejected under Basename;
actual36 is rejected under Worker. Reports:
- artifacts/fsx.exe/pdb-path-experiment-20261103.json
- artifacts/fsx.exe/pdb-path-negative-controls-20261103.json

Default -PdbPathMode Basename; explicit Worker restores the omitted-option
control. Compiler/linker, source, libraries and all other settings are unchanged;
worker needs no restart and candidate EXEs were not executed.

Golden/replay/inventory36-basename-pdb preserves the actual longer-ID repeat
output with all source, commands, request/receipt, log, EXE/PDB/MAP, diagnostics
and comparisons. SHA256:
1A3FB2A1E4E491778A77E6FC163567F8AD98028096C583C53FB3E7FE76254BB7.
First basename EXE SHA256:
D1E330D39461385592E298207C287FD6DAD3FFDEA47AF4E1A6D02D9A7294FF36.
Baseline35 and both actual runs remain retained. Inventory/vtable checks pass36.

No Ghidra edits/save/program navigation or api.dll changes were needed; original
debug bytes are unchanged. Earlier Ghidra comments about a longer rebuilt path
describe older baseline32-era evidence; this topic supersedes that issue for the
default build. No new variable-name suggestion follows from the linker option.

Next: independently observed PE COFF/DLL flags and OS/image-version differences.
Keep unresolved code/import/data order, virtual extents, debug GUID/timestamps
and certificate/resource differences explicit during further experiments.
