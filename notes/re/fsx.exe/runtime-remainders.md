# Remaining runtime listing and bounded metadata

2026-11-18 dashboard follow-up. Live verified `fsx.exe 0x010029c2`
through `0x010029c4`. The unreachable stack cleanup matches raw at
rebuilt `0x010029a2` in a fresh VC8 RTM XP link. The enclosing 41-byte
precision region passes the existing pre-C initialization comparator.
[Scoped listing and comparison](../../../artifacts/fsx.exe/crt-documented-remainder-20261118/code-matches.json).

2026-11-02. Status: live verified in fsx.exe, original PE bytes/imports,
baseline30 MAP/body comparison, fresh XP build and saved Ghidra readback.
Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Verified code

| Original VA | Rebuilt baseline30 VA | Bytes | Evidence |
|---|---|---:|---|
|0x0100270c|0x00401ee4|72|GS SEH prologue, normalized-exact, two checked targets|
|0x01002754|0x00401f2c|15|GS SEH epilogue, normalized-exact, two checked targets|
|0x01002c50|0x00402520|52|64-bit multiplication helper, raw-exact|
|0x0100282c|0x00402214|6|FF25 thunk to MSVCR80.DLL!_XcptFilter|
|0x01002be0|0x0040240e|6|FF25 thunk to MSVCR80.DLL!_invoke_watson|
|0x01002bec|0x00402414|6|FF25 thunk to MSVCR80.DLL!_controlfp_s|

MAP provenance: msvcrt:sehprolg4gs.obj for both GS helpers and msvcrt:llmul.obj
for multiplication. Prologue references the verified EH4 forwarding helper and
security cookie; epilogue calls the verified cookie checker and jumps to the
common SEH epilogue. The comparator validates targets before normalizing four
address bytes. Its reused `call` field kind computes rel32 targets for CALL/JMP.

The multiplication helper consumes two 64-bit stack operands, returns EDX:EAX,
and uses RET16. GS helpers use specialized stack/register conventions. No
ordinary C parameter/return prototype was guessed for these three new functions.
Identified library names are evidence-backed analysis names, not proof of the
original source spelling. The18/1114 application/helper/wrapper count is unchanged.

Correction: the first read-only inspection queried IAT0x01001214 (memset) when
trying to inspect the last thunk. Actual bytes FF25 9C110001 at0x01002bec point
to IAT0x0100119c, which is _controlfp_s. The prior thunk identification was
correct; the inspection address was wrong and is now fixed. Both raw PE import
identity and the existing Ghidra external signature were checked independently.

## Classification of the former eight nonzero undefined ranges

| Former range | Classification and applied model |
|---|---|
|0x0100124c,20 bytes|C++ end sentinel124c, C begin1250, pre_c_init pointer1254, C end1258 typed; zero125c remains unresolved|
|0x0100129c,20|Val/B/D/M/S strings typed at129c/12a0/12a4/12a8/12ac with actual terminators; alignment zeros remain undefined|
|0x010012f4,4|Second pointer of the known empty MFC message map12f0, points to terminator12d8; first pointer preserved|
|0x01002707,97|Five CC bytes, GS prologue72, GS epilogue15, five CC bytes|
|0x01002826,26|Six CC bytes, XcptFilter thunk6, fourteen CC bytes|
|0x01002bdb,29|Five CC bytes, Watson thunk6, six CC bytes, controlfp thunk6, six CC bytes|
|0x01002c4a,86|Six CC bytes, multiply52, twenty-eight CC bytes|
|0x01002d7e,102|Two CC bytes, main EH4 table40, onexit table28, four zero bytes, image-helper table28|

Existing pointer slots0x010012b0..0x010012cc point, in order, to
S/M/D/B/Val/ForceRemove/NoRemove/Delete. This is an observed eight-pointer group;
registry-related interpretation is provisional. Original name, full table extent
and exact rebuilt counterpart remain unproven. No guessed table symbol applied.

The three EH4 tables were already checked by the startup/onexit/image-helper
comparators. Applied reconstructed types under /FSX/CRT:
EH4ScopeRecordModel (12 bytes), EH4ScopeTable1Model (28) and
EH4ScopeTable2Model (40). Header consists of four cookie offset/XOR-offset ints;
record consists of enclosing level and two generic pointers. No callback ABI
was inferred from filter/finally fields. These are minimum observed metadata
models, not recovered original typedef names. Existing labels/comments retained.

## Ghidra changes, listing caveat and persistence

Pre-edit packed database:
artifacts/fsx.exe/ghidra-backups/fsx-before-runtime-remainders-20261102.gzf (2226314 bytes).
CompleteRuntimeRemainders.java checks exact live hashes, program/base, existing
one-byte placeholders, ownership, user symbols and external targets before edits.
Default is read-only; apply creates three bounded functions, completes three
thunks and links each to its existing typed external function, then types the
stated data. No instruction bytes, established user names or API types changed.

Initial bounded disassembly decoded only the first multiplication path.
VerifyRuntimeRemainders.java subsequently decoded each instruction boundary,
including27 missing bytes in its other branch. External Watson is already
noreturn; linking the thunk allowed analysis to store only38 reachable bytes
for _setdefaultprecision at0x0100299e. Its complete41-byte compared region still
includes unreachable ADD ESP,0x14 at0x010029c2..0x010029c4. Those three bytes are
now explicitly listed, without expanding the normal-flow body to hide that
distinction. Earlier reports of a41-byte stored body are superseded here;
41-byte comparison evidence remains valid. Verification readback shows no
listing gaps in all seven checked regions. fsx.exe explicitly saved successfully.
No program switching, api.dll edits or candidate runtime execution occurred.

## Build, regression checks and limits

New mandatory MfcWindows RuntimeRemainders gate composes the earlier ATL/CRT
gates. Fresh worker job runtime-remainders-20261102-001 completed and all existing
application gates passed; ShowErrorMessage still has two differing bytes.
Fresh EXE SHA256:
885CA6130A6CDCDFCDCACBF19D7B5A10CB97264FDB7E204B18AD91A2CB43FFC8.
Baseline30 remains selected and gains the new comparison report against its own
EXE. No C++ source, compiler flags or library selection changed. Opcode and
cookie-target mutations were rejected; results in
artifacts/fsx.exe/runtime-remainder-negative-controls.json, mutated binaries deleted.

Refreshed inventory:212 model functions =77 internal +135 external declarations;
140 mixed-origin rows,129 unassigned instructions in63 ranges. Executable section:
5168 instruction bytes,3290 defined-data bytes,1270 undefined bytes. No remaining
undefined range contains bytes other than00/CC. This is listing coverage, NOT
proof of complete recovered functions, matched code, or padding-only contents.
Zero data, sentinels and metadata still require a semantic/layout audit.

All80 raw import thunks are listed;29 have complete stored six-byte bodies and
51 are unassigned instructions without stored functions. Inventory now reports
those separately; a stored one-byte placeholder alone is no longer sufficient
to claim a complete thunk body. No indiscriminate conversion of the51 is needed.

Next: one controlled /FUNCTIONPADMIN:5 experiment, comparing gap content/spacing
and all existing gates. It remains an untested option hypothesis, not an original
flag identification. Then audit RTTI/data, image base, sections/import placement
and linker layout. InitInstance and ShowErrorMessage remain open as documented.
