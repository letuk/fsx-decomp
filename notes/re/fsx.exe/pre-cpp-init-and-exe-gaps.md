# pre_cpp_init and completed CRT helper definitions

2026-11-02; status: live verified fsx.exe listing/memory, actual XP worker build,
PE/MAP/import comparisons, bounded Ghidra edits and readback.
Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Matching and role

Original pre_cpp_init0x01002313..0x0100235d (75 bytes) matches baseline30
_pre_cpp_init0x00401bcb from msvcrt:crtexew.obj after12 validated address fields.
The actual C++ initializer table slot0x0100123c points to it, before the MFC,
global-window and global-application initializers.

It registers RTC termination through atexit, copies __newmode into the startup
info passed to MSVCR80 __getmainargs, passes the wildcard setting and argc/argv/
environment output addresses, stores the result, and invokes _amsg_exit(8) on a
negative result. Its C++-initializer callback is modeled void __cdecl(void).
This is CRT startup preparation, not an FSX-authored application initializer.

New scripts/compare-pre-cpp-init.ps1 verifies every non-reference byte, the table
slot, atexit adapter target/body, RTC termination callback body/table, CRT import
identities, public __newmode/__dowildcard symbols, private argument/output slots,
their initial values, distinctness and repeated references. Private global names
are role mappings, not recovered original names or complete data-layout equality.
The script deliberately dot-sources compare-pre-c-init.ps1 to reuse its validated
context/helpers; its output is captured and only the new JSON report is emitted.

RTC termination0x01002802..0x01002825 matches36 bytes with both pointers checked.
The effective interval [0x01001574,0x01001574) is empty. As for RTC initialization,
the presence of these library walkers does not prove /RTC application compilation.

Real XP build pre-cpp-init-20261102-001 passed the new mandatory MfcWindows
PreCppInit gate plus all existing gates, including PreCInit and18 previously
counted application/helper/wrapper bodies. No canonical C++/compiler/library
choice changed; baseline30 remains selected. Reports added to that same binary's
preserved comparison.json and pre-cpp-init-comparison.json. Runtime library
identifications remain separately reported; do not inflate18/1114 with them.

Negative control changed the rebuilt _amsg_exit argument8 to9 in a disposable
copy; the comparator rejected non-reference bytes. Result:
artifacts/fsx.exe/pre-cpp-init-negative-result.txt. Mutated copy deleted, never executed.

## Ghidra edits actually applied

fsx.exe was user-selected with api.dll also open; no tab switches or API edits.
Pre-edit backup: artifacts/fsx.exe/ghidra-backups/fsx-before-pre-cpp-init-20261102.gzf,
2210569 bytes. scripts/ghidra/CompleteCrtInitializers.java verifies exact live
body hashes, existing names/body sizes, instruction boundaries and table slot;
default mode is read-only. Applied in one transaction:

| Original address | Current function | Complete body |
|---|---|---:|
|0x01002313|pre_cpp_init|75|
|0x010027d9|_RTC_Initialize|36|
|0x0100299e|_setdefaultprecision|41|
|0x010029cc|CrtReturnZeroFallback|3|
|0x01002802|_RTC_Terminate|36|

The three earlier one-byte FUN_* helper placeholders now have complete, bounded
definitions. Existing instruction bytes were preserved; disassembly was limited
to the five checked body address sets. Added evidence/scope comments and modeled
void cdecl signatures for initializer/walker/precision functions; the shared zero
stub returns int. CrtReturnZeroFallback is a reconstructed descriptive name. Its
no-argument model describes the direct call; the same leaf also serves the
matherr callback, so the unique original parameter list/name remains unresolved.
Do not mistake one folded machine-code body for one unique source-level function.

User-approved rename applied exactly: DAT_01004030 -> g_crtManagedImageFlag.
Existing four-byte undefined4 storage/type preserved, and a provenance comment
added. Decompiled pre_c_init readback shows the approved name. No Hungarian
prefix substitution was made. The pCrtModeValue local suggestion remains unapplied.

Names/bounds/signatures read back; fsx.exe saved. Final readback revision
1793626307309:8bd7bef0:445 reports unsaved_changes=false.

Observed follow-on model changes: _amsg_exit import thunk0x010027ce now has a
stored6-byte function. Import thunks0x01002be0/0x01002bec have new one-byte
placeholders; their full six-byte raw FF25/IAT identities were already inventory
verified, but complete stored bodies/signatures remain a model-cleanup task.
No claim that all Ghidra function definitions are complete follows from this work.

Inventory now206 model functions=71 internal stored functions+135 external
declarations,110 unassigned instructions in61 ranges,134 mixed-origin rows.
Executable9728 bytes:4934 decoded instruction bytes,3158 defined data,1636
undefined bytes;12 nonzero undefined ranges remain. Counts include analysis
side effects and are not original function counts or a match percentage.
No candidate EXE executed. Worker ready; no active build remains.

## Remaining EXE work

See [exe-matching-roadmap.md](exe-matching-roadmap.md) for the scope-separated
checklist. Recommended next bounded analysis: complete CRT entry/main-startup
coverage and compare its actual full body/dependencies, including currently
undecoded gaps, then security/SEH/stack helpers and ATL library bodies.
