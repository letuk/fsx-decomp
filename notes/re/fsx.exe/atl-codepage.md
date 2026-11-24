# ATL thread-codepage helpers and cached getter

2026-11-02. Status: live verified fsx.exe listing/model, PE/MAP/import/state
comparisons, actual XP worker build, bounded Ghidra edits and saved readback.
Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Complete comparisons and corrected identification

| Original | Rebuilt baseline30 | Bytes | Comparison |
|---|---|---:|---|
|ATL fallback getter0x01001f1f|0x0040241a|101|normalized-exact,5 checked fields|
|ATL real getter0x01001f89|0x0040247f|4|raw-exact,returns3|
|ATL selecting thunk0x01001f92|0x00402483|100|normalized-exact,8 checked fields|

All three rebuilt symbols come from atls:atltrace.obj. The four-byte real getter
is PUSH3; POP EAX; RET (CP_THREAD_ACP). Original selecting code explicitly takes
its address, and the rebuilt symbol independently identifies the corresponding
entry. This supersedes the earlier provisional interpretation of0x01001f89 as
a fragment of its neighbor. A separate bounded function is now justified.

Fallback behavior: GetThreadLocale, GetLocaleInfoA type0x1004 into a buffer with
capacity7, decimal parsing, then GetACP if the query fails or parsed value is0.
Capacity7 does not establish the exact original declared buffer size.

The selecting thunk passes OSVERSIONINFOA size0x94 to GetVersionExA. Platform2
and major version>=5 select the real getter; otherwise it selects the fallback.
The API return value is not explicitly tested by this code. InterlockedExchange
updates the cached getter and the same slot is then invoked. Its initial value
at original0x01004000 points to the selecting thunk, as does the rebuilt cache
at0x00405040. Import DLL/name identities and cookie/checker references verified.

scripts/compare-atl-codepage.ps1 validates all bodies, the initial cached pointer
and its repeated uses. No original source-name/compiler-version, whole-EXE/layout
or runtime behavior claim. These library identifications remain separate from
the18/1114 application/helper/wrapper body count. No C++/flags/library changes.

## Concrete layout discrepancy

Original gaps0x01001f84..0x01001f88 and0x01001f8d..0x01001f91 are each five CC
bytes, between the verified101/4/100-byte bodies. Both corresponding rebuilt
gaps are0. This is an explicit remaining non-resource layout difference.
The installed VC8 linker's preserved help lists /FUNCTIONPADMIN[:size]
(vm-share/setup/toolchain-check.txt line50). /FUNCTIONPADMIN:5 is a candidate
controlled experiment, not a proven original command/setting. No such option
was enabled or tested in this step. Do not silently fold padding into functions.

## Integration and tests

Added mandatory MfcWindows AtlCodepage gate. Real XP atl-codepage-20261102-001
passes all prior18/CRT gates. Application nonmatches unchanged; baseline30/source
unchanged. Added same-binary report to comparison.json and
atl-codepage-comparison.json. Negative controls changed return3 to4 and shifted
the initial cached pointer by one byte; both rejected. Results:
artifacts/fsx.exe/atl-codepage-negative-results.json. Altered copies deleted, never run.
PowerShell scripts parsed successfully. Worker ready/no active job.

## Applied Ghidra changes

api.dll remained user-selected; explicit fsx.exe targeting, no navigation/API edits.
Backup: artifacts/fsx.exe/ghidra-backups/fsx-before-atl-codepage-20261102.gzf,2225449 bytes.
scripts/ghidra/CompleteAtlCodepage.java defaults to read-only hash/model preflight.

- Created ATL::_AtlGetThreadACPReal at0x01001f89 with exactly4 bytes and modeled
  uint __stdcall(void). Calling convention follows matched ATL symbols/type
  metadata; zero-argument machine code alone does not uniquely distinguish it.
- Refined the generic cached pointer at0x01004000 to a4-byte pointer to
  /FSX/ATL/ThreadCodePageGetter, uint __stdcall(void).
- Replaced automatic PTR__AtlGetThreadACPThunk_01004000 with
  ATL::g_pfnGetThreadACP, using the verified library counterpart name/role.
  This is analysis identification, not proof of the original source identifier.
- Added body/cache evidence comments. Preserved meaningful fake/thunk names,
  signatures, cookie injection, existing _OSVERSIONINFOA local and all code bytes.

Readback shows a direct typed (*g_pfnGetThreadACP)() and the named real getter,
instead of a generic code-pointer cast and LAB target. Saved revision
1793649165631:5e7b737f:275 reports unsaved_changes=false.

Inventory209=74 internal+135 external declarations,128 unassigned instructions
in62 ranges,137 mixed rows,8 nonzero undefined ranges. Decoded instruction-byte
coverage unchanged: the four-byte leaf was already listed but not a function.
Counts are not original function counts or match percentages.

Suggestions only, not applied: Value -> selectedGetter and local_9c -> versionInfo
in the selecting thunk, based on the actual selected function pointer and the
existing148-byte _OSVERSIONINFOA local. Descriptive names, not originals.

Next: classify remaining undefined runtime/data ranges and close any bounded
helper/thunk definitions; then test the five-byte padding hypothesis in isolation,
followed by remaining RTTI/data/section/linker layout work.
