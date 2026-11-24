# Controlled PE flags and version fields

2026-11-03. Status: original PE anchors, five actual sequential XP builds,
source/command checks, byte comparisons and wrong-mode controls. Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Single-option sequence

Each step adds only the stated option to the previous command. Source-file
hashes, compiler/linker, libraries and all prior options stay unchanged.

| Actual job prefix (all suffixed -20261103-001) | Added option | COFF flags | DLL flags | Image version | OS version |
|---|---|---|---|---|---|
|baseline36 control|none|0103|0000|0.0|4.0|
|header-step1-large|/LARGEADDRESSAWARE|0123|0000|0.0|4.0|
|header-step2-nx|/NXCOMPAT|0123|0100|0.0|4.0|
|header-step3-ts|/TSAWARE|0123|8100|0.0|4.0|
|header-step4-image|/VERSION:5.2|0123|8100|5.2|4.0|
|header-step5-os|/OSVERSION:5.2|0123|8100|5.2|5.2|

Final selected fields agree exactly with original. Subsystem version stays4.0
throughout, also as original. The installed SP1 linker actually emits the OS
version requested by /OSVERSION; no ignored-option warning is present. Support
is measured, not inferred from older saved RTM help. Header OS version is not
being interpreted as the application's runtime Windows-version check or a
minimum supported OS statement. The unique original build command is unproven.

Relevant PE file offsets relative to the PE signature: COFF characteristics+22,
DLL characteristics+94, image-version pair+68, OS-version pair+64. Each version
pair consists of major/minor16-bit fields. The separate subsystem-version pair
is intentionally preserved.

## Whole-file step controls

compare-header-steps.ps1 checks source hashes and exact one-option command deltas
for all five transitions. It compares whole EXEs, excluding only the intended
header field and the parsed COFF/debug timestamps and RSDS GUID at unchanged
semantic locations. Every transition has zero differences outside those fields.
No code/data byte changes are attributed to these options in this experiment;
no arbitrary difference-based masks or runtime equivalence claim are used.
Report: artifacts/fsx.exe/pe-header-steps-20261103.json.

All five actual jobs pass existing application/CRT/ATL/RTTI/data/import-content/
debug/section/stack/base gates and complete InitInstance diagnostics. Known
ShowErrorMessage two-byte difference and InitInstance769 original/777 rebuilt,
246 instructions each,43 differing nonbranch pairs remain. Application/helper/
wrapper count18/1114 is unchanged.

## Gate, defaults and preserved output

verify-pe-header-settings.ps1 checks all selected fields against the requested
HeaderFlagsMode, ImageVersionMode and OsVersionMode and the observed original
anchors; it rejects unintended subsystem-version changes. comparison.json
contains PeHeaderSettings. Actual36 rejected under all-Original expectation;
actual37 rejected independently under default flags, default image version and
default OS version. Results: artifacts/fsx.exe/pe-header-negative-controls-20261103.json.

Defaults are now Original for all three modes. Explicit Default restores the
respective baseline behavior. HeaderFlagsMode also retains LargeAddress and
LargeAddressNx intermediate controls. The worker needs no restart.

Golden/replay/inventory37-original-header preserves actual header-step5-os output
with source/request/receipt, build.cmd/log, EXE/PDB/MAP, diagnostics, all current
comparisons and step experiment. SHA256:
787831F3E09F7A490269686069598D7FEE8CFE2304871A762D724D73A64AAC09.
Baseline36 and all five actual stages remain retained. Inventory/vtable checks
pass37; selected-header-fields match is now reported separately in inventory.
No candidate EXE was executed, no C++ source changed, and no Ghidra database edit,
save or program navigation was needed. Last saved original model is unchanged;
no new variable-renaming suggestion follows from these link options.

## Remaining layout lead

The original and37 both have the32-byte CodeView record at VA0x01001518 ending
exclusively at0x01001538. Their next SafeSEH tables differ:

| Binary | SafeSEH VA | Bytes after CodeView before SafeSEH | Contents |
|---|---|---:|---|
|Original fsx.exe|0x01001560|40|All zero|
|Baseline37|0x01001540|8|All zero|

This is a live on-disk verified32-byte extent discrepancy in a specific range,
not32 differing code bytes. Cause remains a hypothesis/open question: allocation,
section padding or a release/debug postprocess could explain it, but none is
established. Basename path matching alone does not reproduce this gap. Next
inspect the original surrounding listing and linker ordering before a bounded
layout experiment. Current .text virtual9622 versus original9654 (-32) and
.data1204 versus1184 (+20), entry25fb versus262e (-51), code/import/data order,
debug GUID/timestamps and certificate/resource differences remain open. Selected
header-field equality is not whole PE-header or full-image equality; checksum,
timestamps, extents and entry/location fields still differ.
