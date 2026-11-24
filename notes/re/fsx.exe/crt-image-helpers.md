# CRT exception dispatch and PE image helpers

## 2026-11-18 — embedded filter dashboard comparison

Live verified in `fsx.exe` at `0x010028fd..0x01002913`. The 23-byte
range contains the complete filter through `0x01002910` and the initial
stack restore of the handler at `0x01002911`. The filter returns one for
exception code `0xc0000005`.

The fresh VC8 RTM XP build maps the range to `0x010028dd..0x010028f3`
in the linked CRT member `msvcrt:pesect.obj`. All 23 raw bytes are
identical. The existing CRT comparator also validates the two pointers
in the enclosing EH4 scope table and the full 108-byte linked region.
The `InitInstance` comparison and controller regression gates passed.
No local source reconstruction or runtime test was needed for this
linked-library comparison.
[Scoped proof and listings](../../../artifacts/fsx.exe/crt-image-filter-010028fd-20261118/match.json).

Ghidra listing exported with a dry-run readback. The database reported
`unsaved_changes=false` at revision `1795013130661:2514814d:54`.

2026-11-02. Status: live verified fsx.exe code/model, PE/MAP/import comparisons,
actual XP build, bounded Ghidra edits and saved readback. Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Verified code

| Original region | Rebuilt baseline30 | Bytes | Result |
|---|---|---:|---|
|0x01002840 __ValidateImageBase|0x00402220|41|raw-exact|
|0x01002870 __FindPESection|0x00402250|66|raw-exact|
|0x010028b7 nonwritable image region|0x00402292|108|normalized-exact with6 checked fields; includes23 embedded EH bytes|
|0x01002768 exception dispatch wrapper|0x00401f3b|35|normalized-exact with3 checked fields|
|0x0100263d security cookie checker|0x00401ac2|15|normalized-exact with2 checked fields|

scripts/compare-crt-image-helpers.ps1 also verifies the28-byte EH4 table at
0x01002dc8: two filter/handler pointers normalized only after target validation;
all header/enclosing-level bytes exact. Its record maps to filter0x010028fd and
handler0x01002911. Existing initializer/startup checks are composed as prerequisites.
Source, compiler, library selection and selected baseline30 remain unchanged.
These are separately reported CRT identifications, not an increase of the18
application/helper/wrapper bodies/1114-byte count.

__ValidateImageBase checks MZ, PE signature and PE32 optional-header magic0x10b.
This does not validate every possible PE bound/content. __FindPESection searches
40-byte section headers using NumberOfSections and
[VirtualAddress,VirtualAddress+VirtualSize), not raw-file size.

__IsNonwritableInCurrentImage validates its own image base, finds the target's
section and returns whether Characteristics lacks the write bit0x80000000.
Its embedded filter returns1 only for access violation0xc0000005; the handler
restores the shared frame's saved ESP and enters the FALSE-return path. The
stored normal-flow function remains85 bytes, distinct from the108-byte region.

The exception wrapper forwards four incoming stack words, prefixed by the cookie
address and cookie-check callback, to MSVCR80 _except_handler4_common. The actual
original CALL target0x01002b9a is a verified import thunk, matching the rebuilt
DLL/name identity. The imported DLL implementation is outside this EXE body check.
The cookie checker's failure jump maps to ___report_gsfailure0x01002a6d; that
reporter's complete body is still outside this report. The checker bytes include
the same prefixed return as the rebuilt counterpart; no residual mismatch here.

## Integration and controls

Mandatory MfcWindows CrtImageHelpers gate added to the host controller. Real XP
job crt-image-helpers-20261102-001 passes all prior body/CRT gates. Existing
ShowErrorMessage/InitInstance nonmatches remain open. Production EXEs not executed.
Baseline30 comparison.json extended with a report generated against that same
preserved binary, also stored as crt-image-helpers-comparison.json.

Negative controls changed the MZ comparison literal and an EH4 filter pointer
individually in disposable PE copies; both rejected. Results:
artifacts/fsx.exe/crt-image-helpers-negative-results.json. Copies deleted, never executed.
PowerShell scripts parsed successfully.

## Applied Ghidra changes

api.dll remained user-selected; every operation explicitly targeted fsx.exe.
No tab switches or API database edits. Before editing, exported
artifacts/fsx.exe/ghidra-backups/fsx-before-crt-image-helpers-20261102.gzf (2216665 bytes).
scripts/ghidra/CompleteCrtImageHelpers.java checks live hashes, instruction
boundaries, expected23-byte gap and existing definitions before bounded edits.

- Defined complete35-byte _except_handler4 at0x01002768. Native prototype call
  then applied int __cdecl _except_handler4(void *exceptionRecord,
  void *registrationFrame, void *context, void *dispatcherContext).
  This is an ABI-compatible minimum model: four stack pointer words, caller
  cleanup and integer disposition result. No complete Windows structure layouts,
  recovered original parameter names or unique original source identifier claim.
- Decoded23 previously undefined embedded-EH bytes at0x010028fd..0x01002913.
  Preserved85-byte parent ownership and its established BOOL/PBYTE signature.
- Added PE_ImageAccessViolationFilter/PE_ImageAccessViolationHandler labels,
  comments, two scope-table DATA references and a scope-table comment.
- Added comparison evidence to existing PE-helper and cookie-checker comments;
  preserved established names, signatures and the cookie-check caller injection.

Saved readback1793647354097:5e7b737f:171 reports unsaved_changes=false.

## Analysis-only register-effect correction

Before correction, the image-helper decompiler showed an apparently uninitialized
pImageBase after calling __ValidateImageBase. Actual code sets EDX to01000000
before that call, and the41-byte leaf uses EAX/ECX without changing EDX. The
generic cdecl volatile-register model loses that concrete EDX value. This is a
decompiler model artifact, not an uninitialized access in original machine code.

Enabled the Ghidra analysis-only inline flag on this one verified leaf. No bytes,
calling convention, return type or parameter types changed. The decompiler now
expands the real leaf and correctly emits __FindPESection with the concrete own
image base. Its known-header tests may fold away for this particular image.
This is not evidence that the original source used inline or that the compiler
inlined the call: the machine code still contains the CALL. The flag's purpose
and limit are documented at the leaf and the call site0x010028cd, and included
in the bounded script for reproduction. No broad convention override applied.

Inventory208 total=73 stored internal functions+135 external declarations;
131 unassigned instructions/63 ranges,137 mixed-origin rows. Executable9728 bytes:
5011 instruction bytes,3158 defined data,1559 undefined;9 nonzero undefined
ranges remain. This reflects model coverage, not a match percentage. Existing
one-byte import-thunk placeholders remain separate cleanup; raw identities are
already checked. Worker ready/no job active.

Suggestion only, not applied: current decompiler p_Var1 -> sectionHeader in
__IsNonwritableInCurrentImage. Evidence: PIMAGE_SECTION_HEADER returned by
__FindPESection, then its Characteristics field supplies the write-bit test.
Descriptive local role, not an original source name.

Next bounded block: complete report_gsfailure and stack-probe comparisons, then
exit-registration internals/ATL and the data/linker-layout parts of the EXE roadmap.
