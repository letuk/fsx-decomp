# Complete ordinary imports and PE-directory inventory

## 2026-11-18 packaged link and dashboard audit

Live verified for `fsx.exe`, using the packaged ATL-first RTM control
and the saved Ghidra listing. Both image headers have the COFF
`RELOCS_STRIPPED` bit set and a zero base relocation directory. The
remaining reference differences are operands resolved by the linker.
All 137 ordinary imports agree by identity and hint. Of these, 90 use
different IAT slots, comprising 66 MFC, 19 KERNEL32 and five CRT
imports. All USER32 and OLE32 slots match. All 80 jump thunks share
their original addresses. Thirteen match their full six bytes. The
other 67 have verified identity-equivalent IAT operands.

Across the 146 dashboard code entries and ranges, 55 are `Exact`, 90
are `Normalized`, and ShowErrorMessage remains `Different` with one
non-address byte at `0x01001e0e` after target validation. Forty ranges
previously marked `Normalized` now compare raw `Exact`. The 769-byte
InitInstance body at `0x010019e0` still differs in 31 bytes across 26
fields, including 10 IAT operands, 12 string references and four
writable-object references. Other normalized ranges include changed
vtable and message-map references. The initial `.data` bytes and
zero-fill agree while thirteen identified writable object addresses
differ. The audit covers the dashboard scopes, including embedded
ranges that overlap parent bodies. It gives no whole-`.text` count.
Read-only Ghidra reported saved revision `1795013130661:2514814d:58`.

[Operand and slot ledger](../../../artifacts/fsx.exe/linked-content-audit-20261118/audit.json),
[fresh Ghidra listing export](../../../artifacts/fsx.exe/linked-content-audit-20261118/original-listing.tsv).

## Historical import inventory

2026-11-02. Status: original/rebuilt PE reads, native explicit fsx.exe memory/
reference checks, actual XP worker build, negative controls and saved comments.
Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Complete import contents, distinct from order

Original and baseline32 have exactly137 unique ordinary import identities from
five DLLs. No missing/extra imports. DLL spelling and descriptor order agree:

| Descriptor index | DLL | Entries | IAT block offset | Entry order matches? |
|---:|---|---:|---:|---|
|0|MFC80.DLL|67|0x88|No|
|1|MSVCR80.dll|32|0x198|No|
|2|KERNEL32.dll|33|0|No|
|3|USER32.dll|3|0x21c|No|
|4|ole32.dll|2|0x22c|Yes|

Offsets are relative to each IAT directory start: original RVA0x1000 versus
rebuilt RVA0x3000. Module block order within the IAT is therefore kernel32,
MFC80, MSVCR80, user32, ole32, distinct from descriptor order. All block offsets
and counts agree. Each name-import hint matches by identity; ordinal imports
have the same ordinal values/reserved-bit form. Descriptor timestamp and
forwarder fields are0 in both. Both images have separate ILT/IAT tables with
equal initial unbound contents. Null descriptors, each ILT/IAT terminator and the
complete contiguous IAT directory coverage are verified:120 descriptor bytes
(five plus null),568 IAT bytes (137 entries plus five null module terminators).

Entry order is explicitly NOT matched.128 identities occupy different ordinal
positions within their DLL blocks; only9 retain positions. The exact bijection
is artifacts/fsx.exe/import-slot-comparison.csv. This is a concrete layout/order task,
not missing imported functionality or a newly solved function body.

Ghidra's135 external function declarations do not conflict with137 imports:
live references at IAT0x010011cc and0x010011e0 target external nonfunctions
_adjust_fdiv and _acmdln. They are imported data in the current model. The other
135 references target function declarations. No signatures/names were changed
to make the counts agree.

## All sixteen directory entries

| Directory | Original address/size | Baseline32 address/size | Status |
|---|---|---|---|
|Import|RVA0x2e10 /120|RVA0x3618 /120|Contents verified; entry ordering and placement differ|
|Resource|RVA0x5000 /2723616|Absent|Previously excluded resource reconstruction, still a real PE difference|
|Security|File offset0x29bc00 /9280|Absent|Certificate file range, not an RVA; not reproduced|
|Debug|RVA0x1260 /28|RVA0x3260 /28|One CodeView record each; metadata differs|
|Load config|RVA0x14d0 /64|RVA0x34d0 /64|Actual structure72 and SafeSEH contents checked separately|
|IAT|RVA0x1000 /568|RVA0x3000 /568|Complete imports checked as above; order/placement differ|

The other ten entries are absent in both: export, exception, base relocation,
architecture, global pointer, TLS, bound import, delay import, CLR and reserved.
No absent directory is being treated as an unimplemented source function.

Original certificate directory is a single9280-byte record, revision0x0200,
type2, ending exactly at EOF. Range/aligned-record boundaries are checked;
cryptographic validity and signer identity were NOT verified. It is not a
runtime mapped section. These bytes remain outside this gate's matching scope,
not an assumed new exclusion from the user's eventual whole-EXE target.

Both debug records have Type2, RSDS signature and Age1. Original data at RVA1518
has record Size32 and PDB path fsx.pdb; existing Ghidra DotNetPdbInfo31 ends before
the path's null terminator at0x01001537. That final byte is a terminator, not an
additional padding byte. Baseline32 data at RVA3518 has Size85 and worker-local path
C:\fsx-build\matching-jobs\rtti-default-20261102-001\fsx.pdb. GUIDs and timestamps
differ. Raw debug file-pointer bytes are checked against the mapped data range,
but no debug identity match is claimed. Each report belongs to its own EXE/hash;
fresh job names can change the newly generated PDB path/length.

## Header/section differences and next experiments

| Field | Original | Baseline32 |
|---|---|---|
|ImageBase|0x01000000|0x00400000|
|Machine / linker / subsystem|x86 /8.0 /GUI2|Same|
|COFF flags|0x0123|0x0103|
|DLL flags|0x8100|0|
|OS version|5.2|4.0|
|Image version|5.2|0.0|
|Subsystem version|4.0|4.0|
|Stack reserve/commit|0x40000 /0x2000|0x100000 /0x1000|
|Heap reserve/commit|0x100000 /0x1000|Same|
|Section/file alignment|0x1000 /0x200|Same|
|SizeOfHeaders|0x400|Same|
|Checksum|2797628|0|

Original sections: .text RVA1000 virtual9654/raw9728 flags60000020;
.data RVA4000 virtual1184/raw512 flagsC0000040;
.rsrc RVA5000 virtual2723616/raw2723840 flags40000040.
Baseline32: .text RVA1000 virtual6142/raw6144 flags60000020;
.rdata RVA3000 virtual3518/raw3584 flags40000040;
.data RVA4000 virtual1204/raw512 flagsC0000040. Both lack base relocations and
have COFF relocation-stripped flags. These are observed fields, not proof of
one unique original linker command. Sizes are not a match percentage.

Next isolate /BASE:0x01000000, then test read-only-section merging/ordering from
actual output. Audit legacy comparator assumptions about base0x00400000 before
the base experiment; rejecting valid relocated references would be a test bug.
Header flag/version/stack changes should be separate recorded experiments.
Import entry order remains a distinct unresolved task; no unsupported claim
that /ORDER alone will fix import tables. Local VC8 help confirms relevant
base/merge/section/large-address/NX/TSAWARE options exist, not their original use.

## Gate, actual build and Ghidra persistence

scripts/compare-pe-imports.ps1 -RequireImportContents is now a mandatory
MfcWindows gate, stored as PeImports in comparison.json. It enforces identical
contents/hints/descriptor/module-block shape and intact table termination;
ImportContentsAndOrderMatch remains FALSE. -RequireExactOrder separately rejects
the real current build. Header/directory/section/debug/certificate differences
are inventoried, not normalized away. Import content and import order must not
be conflated when reporting progress.

Fresh actual XP job pe-imports-20261102-001 passes this content gate and all prior
application/CRT/ATL/RTTI/data gates. Fresh EXE SHA256:
0B7BD395AED23E685BA8342AD3BF3DDE9671FF0AB6DB6098CDF1FE4A99F36BB0.
Baseline32 gains its own new comparison report; no binary/source/flag/library
replacement. Name-hint and IAT-terminator mutations rejected; actual order also
rejected by the stricter order switch. Results in
artifacts/fsx.exe/pe-imports-negative-controls.json; modified copies deleted, never run.
Full baseline report: artifacts/fsx.exe/pe-imports-comparison.json.

Pre-edit backup artifacts/fsx.exe/ghidra-backups/fsx-before-pe-import-audit-20261102.gzf
(2237723 bytes). DocumentPeImports.java verifies live hashes for IAT568,
descriptors120 and debug directory28; appends comments only at0x01001000,
0x01002e10 and0x01001260. Established names/types/prototypes/bodies unchanged.
fsx.exe explicitly saved, readback unsaved_changes=false. No tab navigation or
api.dll changes. No useful new local-variable rename follows from this audit.
Known application nonmatches and18/1114 body count remain unchanged.
