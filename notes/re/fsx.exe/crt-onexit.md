# CRT exit registration, locking and finally cleanup

2026-11-02. Status: live verified fsx.exe code/types, PE/MAP/import/state/EH
checks, actual XP build, Ghidra edits and saved readback. Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Complete comparisons

| Original | Rebuilt baseline30 | Bytes | Validated fields |
|---|---|---:|---:|
|0x01002651 __onexit|0x00401b1a|150|14|
|0x010026e7 lock cleanup|0x00401bb0|9|1|
|0x010026f5 atexit adapter|0x00401bb9|18|1|

All three bodies normalized-exact. The28-byte EH4 table at0x01002da8 matches
after validating its one cleanup pointer. Filter field0 identifies finally
handling, with handler0x010026e7. This is the same function called directly on
the normal registration path. Cleanup calls import-identified _unlock(8).
No exception-catching callback/prototype is inferred from the null filter field.

__onexit decodes the encoded begin pointer at0x01004498. Sentinel-1 selects the
imported MSVCR80 _onexit route. Otherwise it acquires CRT lock8, decodes begin/end
(end0x01004494), calls imported __dllonexit with callback and local table pointers,
re-encodes the updated global pointers and releases the lock. The finally table
provides the exception-unwind cleanup as well. This is static code evidence, not
a trace proving which branch actually executes. Imported registration/encoding/
locking implementations are outside this EXE comparison.

atexit maps a nonnull __onexit registration result to0 and null to-1. The
initialization/sentinel mapping already checked in pre_c_init remains consistent.
scripts/compare-crt-onexit.ps1 gates all bytes, selected targets/imports, encoded
table globals/initial state, SEH helpers, adapter and cleanup/scope relationships.

## Build and controls

Added mandatory MfcWindows CrtOnexit gate. Actual XP worker
crt-onexit-20261102-001 passes all earlier18 body/CRT gates. No C++ source,
compiler/flags/library or selected baseline30 change. Application nonmatches
unchanged. Same-binary comparison.json extended and crt-onexit-comparison.json
preserved beside baseline30. These library checks do not inflate18/1114.
No original/candidate EXE executed.

Negative controls changed cleanup lock8 to9 and shifted the finally target by
one byte; both rejected. Results: artifacts/fsx.exe/crt-onexit-negative-results.json.
Temporary altered copies deleted. PowerShell parsing checks passed.

## Applied Ghidra changes and callback models

api.dll remained user-selected; explicit fsx.exe calls, no navigation/API edits.
Pre-edit backup: artifacts/fsx.exe/ghidra-backups/fsx-before-crt-onexit-20261102.gzf,
2226165 bytes. scripts/ghidra/DocumentCrtOnexit.java defaults to read-only preflight
and checks live body/table hashes and existing model before applying.

- Preserved __onexit name; typed it as _onexit_t __cdecl __onexit(_onexit_t callback).
- FUN_010026e7 -> void __cdecl ReleaseCrtExitLock(void), a reconstructed descriptive
  cleanup name, not an original recovered identifier.
- FUN_010026f5 -> int __cdecl atexit(CrtAtexitCallback *callback), identifying its
  verified standard CRT counterpart. Added evidence/callback-role comments.
- Added finally-table comment and DATA reference from0x01002dc0 to0x010026e7.
- Preserved encoded global storage/types rather than claiming they contain plain
  decoded pointers. No instruction bytes or function boundaries changed.

Existing /stdlib.h/_onexit_t was inspected: a four-byte pointer to an int(void)
callback. It is reused unchanged. atexit's void(void) callback is modeled separately
as /FSX/CRT/CrtAtexitCallback (function definition plus pointer). That descriptive
type name is new analysis metadata, not an original source identifier. The wrapper
casts between compatible machine pointer representations; callback return-type
models are deliberately not conflated.

The bridge's text prototype parser rejected the existing callback-return typedef
and nested callback expression. scripts/ghidra/TypeCrtExitCallbacks.java applies
the actual Ghidra DataType objects transactionally instead. Reproduction order:
DocumentCrtOnexit apply, then TypeCrtExitCallbacks apply. Readback confirms both
callback signatures and the void cleanup. Final saved revision
1793648448204:5e7b737f:247 reports unsaved_changes=false.

Inventory counts unchanged:208 total=73 internal+135 external declarations,
137 mixed-origin rows,8 nonzero undefined ranges. Worker ready/no active job.
These are model counts, not matching percentages or full EXE/layout equality.

Suggestions only, not applied: p_Var1 -> registrationResult in the two registration
functions (it carries the returned callback pointer); DAT_01004498 ->
g_crtEncodedOnExitBegin and DAT_01004494 -> g_crtEncodedOnExitEnd. Evidence: verified
encode/decode operations and table argument roles. Keep encoded storage distinct
from decoded callback-array pointers; names are reconstructed descriptions.

Next bounded block: ATL thread-codepage helpers and their associated state/imports,
then remaining runtime fragments and full data/RTTI/linker-layout work.
