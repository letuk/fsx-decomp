# Application message map and Run matching

2026-11-02; status: live API ABI/listing verification and actual XP/SP1 rebuilt
PE comparisons. Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

| Body | Original | Final rebuilt | Bytes | Validated address fields |
|---|---|---|---:|---:|
|GetMessageMap|0x01001650|0x00401340|6|1|
|Current-message callback|0x01001cf0|0x00401350|5|1|
|Run override|0x01001d00|0x00401360|35|2|

All46 body bytes match after four target-validated reference normalizations.
Message-map data is checked additionally: eight-byte AFX_MSGMAP record with
base-map import and entry pointer, and24-byte all-zero empty-entry terminator.
Names and original macro/file spelling remain reconstructed, not recovered.

## Message-map source and validation

Canonical FSXAppPilot now uses DECLARE_MESSAGE_MAP and an empty
BEGIN_MESSAGE_MAP/END_MESSAGE_MAP, as provided by installed VS2005 MFC headers.
The generated six-byte MOV EAX,<map>;RET body matches the original. Original
record0x010012f0 points to thunk0x01002020 and terminator0x010012d8. Both old
and rebuilt base-map thunks resolve to MFC80 ordinal3683; actual SDK linkage
supports CWinApp::GetThisMessageMap. Both terminators are exactly24 zero bytes.
No guessed message entries or manual MFC structure replacements introduced.

## Independently verified API slot+8 contract

api.dll SHA2569CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003,
function0x20122817: MOV argument[EBP+8] to global0x2020de7c; call wrapper
0x2013e3c1 with0x20122ae1; XOR EAX,EAX; RET4. In0x20122ae1 the global is
called with no explicit arguments, returned pointer is used as an MSG buffer
for PeekMessage/TranslateMessage/DispatchMessage; null falls back to a localMSG.
Thus earlier no-argument UnknownSlot08 source declaration was incomplete.

Canonical interface now has reconstructed int RunMessageLoop(CurrentMessageCallbackPilot)
at that slot, where the callback returns MSG* and uses WINAPI. Current caller
sets ECX=this; this particular API body does not read incomingECX. Return word
is observed zero. Callback's zero-argument calling convention is modeled from
actual SDK AfxGetCurrentMessage, not uniquely proved by a zero-argument call.
RunMessageLoop is a behavior-based reconstructed name, not an original name.
Source interface updated; Ghidra types/names have NOT been changed.

Run invokes the Application getter unconditionally, calls slot+8 with the
wrapper callback, then tail-dispatches its own virtual ExitInstance at+0x70.
The entire35-byte body preserves indirect calls, ECX handling and tail JMP EAX;
only the getter-global and callback addresses are normalized. Callback's sole
JMP targets a thunk to MFC80 ordinal1074, AfxGetCurrentMessage, verified on both
sides. No API implementation code is added to the EXE harness.

## Builds, acceptance and saved baseline

Real worker builds: message-map-20261102-001, run-loop-20261102-001, and final
app-dispatch-20261102-001. All previous10 body matches retained. Final build
passed the new mandatory compare-app-dispatch.ps1 alongside existing lifecycle,
helper and OLE regressions, source hashes, receipt and SP1 compiler checks.
Total explicitly counted matched bodies:13 /1001 bytes. Static map data and
inherited SDK leaves are not counted again as additional source bodies.

Final baseline: vm-share/modules/fsx/archive/25-application-dispatch-matches.
Final EXE SHA256:
B66678A07A9C034483CB95035E94BAA73A68CEBB7AE4FCE05853BB2A37395791.
Source snapshots, build command/log, EXE/PDB/MAP, receipt and reports preserved.
Replay fixture now points to baseline25, which includes these new mandatory gates.
A negative PE-copy test changed Run's callback address: the comparator rejected
it before normalization (artifacts/fsx.exe/dispatch-negative-result.txt). Mutated files
were deleted and never executed. Original and canonical binaries unmodified.

Updated application-vtable inventory rechecks both dispatch and lifecycle bodies:
40 import identities agree,3 SDK leaves equal raw bytes,4 of5 application entries
are now normalized-exact. InitInstance remains777 vs769 bytes,246 instructions,
frame0x554; ShowErrorMessage remains two differences. All previous OLE/vtable/EH
checks pass. Matching here does not cover original RTTI, EH/CRT registration,
external DLL bodies, whole-file equality, or executable FSX replacement behavior.

## Ghidra state and next work

User's api.dll tab remained current, fsx.exe also open. Read-only API inspection
returned saved revision1793405534817:bb3b8861:3. No agent navigation, function
creation, rename/type edit, save or backup. Original EXE getters/Run/wrapper
still lack stored Ghidra functions; creations/role names remain suggestions.
Suggested future typing: API FUN_20122817 and ApplicationVTable slot+8 as a
message-loop method taking a MSG* callback, preserving explicit this handling.

The missing EXE message-map/Run overrides are now resolved, superseding the
missing-status inventory made against baseline24. Remaining folding observation
is unchanged: original Serialize/Dump share a RET4 body, /OPT:NOICF rebuild keeps
separate bodies. Next isolated source-pinned experiment: /OPT:ICF, preserving all
13 body gates and comparing InitInstance/ShowErrorMessage diagnostics. Do not
assume original flag identity or silently change the canonical linker command.
