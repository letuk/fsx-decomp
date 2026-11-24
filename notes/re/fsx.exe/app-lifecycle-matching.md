# Application lifecycle matching milestone

2026-11-02. Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.
Status: live-read original assembly plus locally verified rebuilt PE comparisons.

| Original function body | Rebuilt final003 | Bytes | Validated reference fields |
|---|---|---:|---:|
|0x01001660 event constructor/global-specialized initializer|0x00401290|95|13|
|0x010016d0 scalar deleting destructor|0x004012f0|70|4|
|0x01001720 ExitInstance override|0x00401340|58|5|

All223 bytes match after normalizing only the22 independently validated address
fields. Not literal raw equality, whole-file equality, or proof of original
source/class names. Body boundaries include RET or the final tail JMP. Import
DLL+ordinal/name identities, FS98MAIN string, application global, event+0xa4 and
window pointer+0x20 mappings are checked before normalization. Derived-vtable
associations for destructor, InitInstance and ExitInstance are checked separately;
full application vtable/RTTI/EH/CRT registration and external bodies are excluded.

Constructor/destructor were already present in canonical source and match the
existing run016; this work establishes their comparisons rather than claiming
newly generated implementations. ExitInstance is new canonical C++ source.
No manual assembly or binary patches used in production sources or builds.

## ExitInstance evidence

Original0x01001720..0x01001759 already contains instructions but is not defined as
a function in the current Ghidra model. A read-only listing walk found:

- Save incomingECX inESI; load g_fsxApp.m_pMainWnd at0x01004400.
- If nonnull, call thunk0x01002044 -> MFC80 ordinal1979, CWnd::Detach as
  confirmed against the real SDK-linked rebuilt import; clear the window pointer.
- If pfnGetApplication0x01004384 is nonnull, call it, then invoke returned
  object's vtable slot+4 withECX=this and no explicit stack arguments.
- RestoreECX fromESI and tail-jump0x01002050 -> MFC80 ordinal2248,
  CWinApp::ExitInstance, again independently matched by actual linked imports.
- Original application vtable0x01001398+0x70 points to0x01001720. Rebuilt
  CWinApp-derived virtual ExitInstance occupies the same slot and reproduces the
  entire58-byte body after five validated address references.

api.dll current vtable slot+4 points to0x2012052a. It has existing instructions
but no stored function: PUSH0x20120ed5; CALL0x2013e3c1; RET. The wrapper target
and larger shutdown implementation were not newly analyzed here. Therefore
UnknownSlot04 remains unchanged in the reconstructed interface.

Suggestions only, not applied in Ghidra:
- Create an int __thiscall FSXApp::ExitInstance function at0x01001720; the name
  is a supported reconstructed MFC role, not a recovered original identifier.
- UnknownSlot04 -> Shutdown (provisional), based on the exit-path call and
  wrapper. Inspect the full target before applying a semantic API method name.

## Reproduction and regressions

scripts/compare-app-lifecycle.ps1 independently reads both PE files and import
tables, resolves rebuilt symbols through MAP, verifies target fields, and compares
all remaining bytes. Shared PE reader in scripts/lib/pe-image.ps1 was extracted
from the established OLE comparator without changing that working comparator.

Real worker runs lifecycle-20261102-001/002/003 completed with pinned SP1.
Run001 established the new ExitInstance body;002 passed all body comparisons
but the new AppLifecycle status handling incorrectly reused ShowErrorMessage's
two-byte count, so its status was exhausted. Corrected that reporting branch;
fresh003 ends matched with AppLifecycle differences0, independently preserving
ShowErrorRegressionDifferences2. No InitInstance matching claim is inferred.

Final baseline: vm-share/modules/fsx/archive/24-application-lifecycle-matches.
EXE SHA256:88A1121284B9953F5DC24E27C2D17934F018485C7A41BE2BF98B2317BE8B7586.
It contains actual compiled source snapshots, pinned build command, logs,
EXE/PDB/MAP, receipt and comparisons. The controller checks all three new bodies
on future builds; AppLifecycle supports one reviewed source snapshot per run.
Replay fixture now references this preserved baseline, not the older23 baseline
that lacks ExitInstance source. Replay is not evidence of a new compile.

A controlled negative test changed the constructor's PUSH0 operand in a disposable
PE copy. The comparator rejected non-reference offset1; result retained in
artifacts/fsx.exe/lifecycle-negative-result.txt. The mutated EXE and temporary copies were
deleted afterwards. They were never executed or used as reference binaries.

All previous seven reported function bodies retain their matches, bringing the
explicitly counted total to10 bodies /955 bytes. Additional tiny inherited leaves
checked as vtable dependencies are not counted again. ShowErrorMessage remains
437 bytes with two differences; InitInstance remains777 vs769,246 instructions
and frame0x554, with new operand/block diagnostics regenerated on final003.

Ghidra: api.dll remained current with fsx.exe also open. Original instructions
were read explicitly by program; no navigation, creation, rename, type changes,
save or backup performed. Current save flags were not freshly inspected.

## Next targets

Continue with the application vtable and actual API slot+4 shutdown target rather
than additional blind InitInstance expressions. Separate inherited MFC imports
from FSX overrides and tiny compiler-generated stubs; do not count imported DLL
implementations as reconstructed source. The new lifecycle matches strengthen
the current CWinApp-derived layout and toolchain evidence, without uniquely
identifying the original compiler revision or original translation-unit layout.
