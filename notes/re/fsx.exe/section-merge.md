# Read-only data merged into the executable section

2026-11-02. Status: actual XP compiler/linker builds, identical source hashes,
single-option command control, PE/MAP/data/import/body comparisons and actual
wrong-layout controls. Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Measured result

/MERGE:.rdata=.text produces the observed original non-resource section names,
order, RVAs, raw sizes, raw file offsets and access/type flags:

| Section/field | Original | Merged build |
|---|---|---|
|.text RVA|0x1000|0x1000|
|.text raw file offset|0x400|0x400|
|.text raw size|9728|9728|
|.text virtual size|9654|9686 (+32)|
|.text flags|0x60000020|0x60000020|
|.data RVA|0x4000|0x4000|
|.data raw file offset|0x2a00|0x2a00|
|.data raw size|512|512|
|.data virtual size|1184|1204 (+20)|
|.data flags|0xc0000040|0xc0000040|

The original .rsrc section is not reconstructed, as previously scoped. No
resources were silently merged into executable data. The former read-only
section is incorporated by the linker into executable/readable .text, which
retains the observed original flags. This experimentally supports this merge
option for reconstruction; it does not recover the unique original command.

Important distinction: +32/+20 are virtual-size deltas, not counts of different
bytes or a whole-section content score. Code/data ordering and address fields
still differ. All existing function/data/import-content gates retain their
stated results, not a new whole-image match.

Several data-directory addresses now agree exactly with original:
IAT RVA0x1000, debug directory0x1260 and load-config0x14d0. The eight observed
mfcs80 token literals also have exact original VAs. Their already-verified
contents are not newly counted matching function bodies. The entry point is
0x0100263b versus original0x0100262e (+13); this local distance does not describe
other function locations. The source-dependent function/vtable order and128
reordered import entries remain open.

## Controlled actual builds

Actual XP jobs sections-mergedx-20261102-001 and sections-control-20261102-001
both pass all prior regressions and complete InitInstance diagnostics. The
source files have identical hashes. Generated build.cmd differs solely by
/MERGE:.rdata=.text. Both use original image base, padding5, disabled C++ RTTI,
SP1 compiler/linker, pinned RTM msvcrt.lib, MFC precedence and prior flags.

Matched-length job names produce89-byte CodeView records in both runs. Their
actual path/GUID/timestamps still differ. Original CodeView record is32 bytes;
this metadata/layout difference is not removed by merging. Do not attribute all
remaining virtual-size differences to the code, nor infer exact-match bytes from
section sizes. Report: artifacts/fsx.exe/section-merge-experiment.json.

Merged EXE SHA256:
5EED511BE9650E052B579B195DC594662696A7CE0A1AF048F84549C966E98670.
Both original and merged files still lack base relocation directories. No new
runtime execution or relocatability claim follows from section merging.

## Gate and preserved baseline

New verify-section-layout.ps1 enforces requested section names/order and
established access/type flags, while separately reporting differences against
the original non-resource sections. comparison.json contains SectionLayout.
The real separate baseline33 is rejected when merged layout is requested; the
real merged build is rejected when separate layout is requested. Results in
artifacts/fsx.exe/section-layout-negative-controls.json. No artificial binary edits.

Default controller parameter is now -SectionLayout RdataIntoText. Explicit
-SectionLayout Separate retains the previous omitted-option control. The flag
is inserted only into staged linker commands; the guest worker needs no change.

Golden/replay/inventory baseline34-merged-readonly preserves the actual merged
output, source/request/receipt, build.cmd/log, EXE/PDB/MAP, header/disassembly,
all prior comparisons and InitInstance diagnostics. Baseline33 and both actual
control/merged jobs remain retained. Standalone defaults in the four legacy body
comparators also select34. Current inventory/vtable checks pass against34.
No source, library selection or other compiler/linker setting changed.

InitInstance remains769 original versus777 rebuilt bytes,246 instructions each
and43 differing nonbranch pairs; ShowErrorMessage retains two differing bytes.
Application/helper/wrapper count remains18/1114 with previously stated scopes.

No Ghidra database edits, save or navigation were needed: original section
facts are unchanged and the last saved original model remains intact. No api.dll
changes, candidate execution or new variable-renaming suggestions.

Next: stabilize/reconstruct debug-record path/size and address the independently
observed PE header flag/version/stack differences. Code/import/data ordering,
exact virtual extents and debug/certificate/resource contents remain distinct
work toward the eventual matching target.
