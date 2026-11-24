# Complete CRT startup region and embedded exception paths

## 2026-11-18 dashboard verification

Live verified in `fsx.exe` and rechecked against a fresh XP VC8 RTM build.
The 47-byte range at `0x010024dd` maps to `0x010024bd` and matches
after validating four operands through the enclosing CRT gate. The
seven-byte range at `0x01002529` maps to `0x01002509` and matches raw.
The original and rebuilt EH4 tables point to the corresponding filters
and handlers. The dashboard compares each range separately and retains
the enclosing 479-byte comparison.
[Scoped comparisons and listings](../../../artifacts/fsx.exe/crt-documented-remainder-20261118/code-matches.json).

2026-11-02. Status: live verified fsx.exe memory/pseudo decoding, actual XP build,
PE/MAP/import/state comparisons, bounded Ghidra changes and saved readback.
Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Complete region versus stored function

Original0x01002363..0x01002541 is a contiguous479-byte CRT startup region,
including embedded exception filters/handlers. The stored ___tmainCRTStartup
normal-flow function covers425 noncontiguous bytes within that span. Earlier
undefined gaps0x010024dd..0x0100250b (47 bytes) and0x01002529..0x0100252f
(7 bytes) contain the54 remaining exception-path bytes, not padding or evidence
of additional FSX application logic. Whole-region pseudo decoding gives157
instructions. Do not call the479-byte region one ordinary function body or
silently change its stored425-byte normal-flow ownership.

Baseline30 ___tmainCRTStartup at0x00401c16 has the same complete479 bytes after
39 individually validated address fields. All other bytes, including branches,
error constants and embedded filter/handler instructions, match exactly.
No application C++/compiler/linker/library change was necessary.

Normal startup covers startup-lock acquisition/reentrancy, native initialization
state, C/C++ initializer table calls, optional dynamic-TLS callback validation,
quoted/multibyte program-name skipping, STARTUPINFO show-state selection, MFC
WinMain invocation, and managed/unmanaged exit handling. The embedded code
handles the corresponding structured-exception filter/return paths.

## Checked dependencies and metadata

| Region/record | Original | Verified scope |
|---|---|---|
|CRT startup region|0x01002363|479 bytes,39 validated fields|
|EH4 scope table|0x01002d80|40 bytes; four local filter/handler pointers validated, remaining bytes exact|
|PE entry wrapper|0x0100262e|10 bytes; checked actual PE entry, cookie-init CALL and CRT tail JMP|
|Security cookie initialization|0x010029d4|148 bytes; cookie/complement initial values and four references, five entropy-import identities checked|
|SEH prologue|0x01002940|69 bytes; handler and cookie references mapped, all other bytes exact|
|SEH epilogue|0x01002985|20 bytes raw-exact|
|MFC WinMain wrapper|0x01002c03|E9 target resolves to MFC80 ordinal1207 in original and rebuilt imports|

Scope table header DWORDs are FFFFFFFE,0,FFFFFF88,0. Both records have enclosing
levelFFFFFFFE. Record0 maps filter0x01002529/handler0x0100252d; record1 maps
filter0x010024dd/handler0x010024f1. The always-one filter returns1; its handler
restores saved ESP and joins the return255 path. The main filter reads exception
pointers through the shared EBP frame, saves the exception code and invokes
import-identified _XcptFilter. Its handler restores ESP and enters the checked
managed/unmanaged exception exit path. These embedded entries require shared
frame context; standalone standard C prototypes were not guessed.

scripts/compare-crt-startup.ps1 composes the existing initializer gates and
validates the tables, selected imports, public-symbol globals, private state
relationships/initial values, managed flag established by pre_c_init, scope
record targets, MFC entry and stated dependencies. Private state role mappings
are not original source names or full data-layout equality. The complete bodies
of __IsNonwritableInCurrentImage0x010028b7 and __except_handler4 counterpart
0x01002768 remain separate, uncertified dependencies of this report. External
DLL implementations, all remaining CRT code, complete EXE/layout, original
compiler identity and runtime behavior are not certified.

## Builds and controls

Real XP worker crt-full-startup-20261102-001 passes the new mandatory MfcWindows
CrtStartup gate, all prior CRT gates and the18 existing application/helper/wrapper
body gates. Existing ShowErrorMessage/InitInstance nonmatches remain unchanged.
The startup library identification is separate from that18/1114 count.

Baseline30 selected binary remains unchanged:
D8973AC14E07DA44C01F0FA4AE207023E290361C2B2125460BB57FF902E17A7B.
Its comparison.json is extended with a report generated against that same EXE,
also preserved as crt-startup-comparison.json. Application source unchanged.
Installed-SP1 baseline29 also passes this particular region/dependency check;
it does not uniquely identify RTM versus SP1 compiler/library revision. The
separate C-table XIY difference remains the reason for the selected RTM CRT.

Two negative controls were rejected: changed failure return255 literal, and a
one-byte change to an EH4 filter pointer. Results in
artifacts/fsx.exe/crt-startup-negative-results.json. Disposable altered copies deleted;
no original/candidate EXE was executed or patched in production.

## Applied Ghidra work

After restart, native Ghidra tools became available again. fsx.exe and api.dll
were open, api.dll user-selected. All calls explicitly targeted fsx.exe; no UI
tab changes or API database edits. The previously attempted shell/socket fallback
was not used; its denied helper request was never executed. Unused fallback
helper/schema/request scratch files were removed after native access recovered.

Pre-edit backup: artifacts/fsx.exe/ghidra-backups/fsx-before-full-crt-startup-20261102.gzf,
2210817 bytes. Read-only preflight and application script:
scripts/ghidra/CompleteCrtStartupListing.java. It checks exact live479-byte and
40-byte hashes, instruction boundaries, expected missing54 bytes and existing
parent ownership before applying in a transaction.

- Filled only the verified listing gaps inside the479-byte region.
- Preserved the425-byte normal-flow parent function body and its established name;
  completed its modeled signature as int __cdecl ___tmainCRTStartup(void).
- Added descriptive labels CRT_MainExceptionFilter at0x010024dd,
  CRT_MainExceptionHandler at0x010024f1, CRT_StartupFailureFilter at0x01002529,
  CRT_StartupFailureHandler at0x0100252d, with English context/scope comments.
- Added four data references from EH4 pointer fields and a scope-table comment.
- Added main-startup/entry verification comments; entry name/prototype preserved.

No guessed standalone functions for embedded filters, and no meaningful existing
global names/types replaced. Saved/readback revision
1793646807668:5e7b737f:88 reports unsaved_changes=false. Earlier revisions use a
different session epoch because the user restarted Ghidra.

Observed follow-on model: new one-byte placeholder FUN_0100282c at the already
raw-import-verified _XcptFilter thunk; complete six-byte stored body/signature
remains a model-cleanup task. Inventory207 total=72 internal stored functions+
135 external declarations,131 unassigned instructions/63 listing ranges,136 mixed
rows. Executable9728 bytes:4988 instructions/3158 data/1582 undefined;10 nonzero
undefined ranges. More unassigned instructions reflect newly decoded embedded
EH code, not regressions or extra independent source functions. Not a match
percentage. Worker ready; no active job remains.

Suggestions only, not applied (live decompiler names): Exchange -> startupLockOwner
because it holds StackBase as the lock-owner token; pvVar2 -> previousStartupLockOwner
because it receives InterlockedCompareExchange's previous value; local_24 ->
commandLineTail because it skips program name/whitespace and becomes WinMain's
command-line argument. These are descriptive reconstructed roles, not original
identifiers. Existing approved g_crtManagedImageFlag remains unchanged.

Next bounded block: full exception-dispatch wrapper and PE image helpers. Security
check/report helpers, stack probes, exit-registration internals, ATL, data/RTTI
and exact linker layout remain on the EXE roadmap.
