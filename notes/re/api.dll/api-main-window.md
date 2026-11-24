# API main-window initialization and keyboard hook

2026-11-03. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003,
x86 PE32 base200e0000. Original VAs below. Status: live/file verified where
assembly/bytes cited; working names/phase interpretations are reconstructed.
No runtime test or full linked DLL assertion. User authorizes routine continuation
without confirmation and prefers structured source without goto.

## Main flow inventory:2012205d

Stored Ghidra body76 bytes stopped at RET4 after SetCurrentDirectoryA; its old
void decompilation is incomplete and is superseded as a description of the full
routine. Read-only bounded native walk now inventories398 instructions/1346 bytes,
seven16-byte integrity descriptors and seven frame-restoring actual RET4 exits.
Together these cover2012205d..2012260e (1458 bytes) with no interior gaps or
overlaps. Bound201226b0 was provisional; reachable end is2012260f exclusive.
Only normal/success continuations are included; calls opaque, integrity failure
and detached exception paths excluded. This is analysis coverage, not rebuilt
code matching or a proof that every possible runtime path is enumerated.

Gateway201eccdf saves flags/GPRs/last-error, calls201e44b4 with saved-context
pointer, restores incoming EAX as well as other saved registers, and RETs. Helper
reads four descriptor words, processes referenced range through a table-selected
helper, handles relocation difference, and compares two computed words. Assembly
201e472f advances saved return pointer by10h. Failure may rewrite saved context
and redirect return to201ecc0d. These observations support an integrity-check
role; no protection-product attribution or bypass is implemented.

| Transfer RET4 |16-byte descriptor | Success continuation |
|---|---|---|
|201220a6|201220a9|201220b9|
|201220e9|201220ec|201220fc|
|201221a4|201221a7|201221b7|
|201222f9|201222fc|2012230c|
|201223b3|201223b6|201223c6|
|201224c7|201224ca|201224da|
|2012254f|20122552|20122562|

First descriptor07000000040b0400fc0a0400fd010000 is data, not executable CLD/OR
instructions. Never assume the prior region gateway's8-byte skip for this type.
Actual exits201221bf/201221f3/2012229e/20122434/201224e2/20122584/2012260c
restore ESP from EBP and POP EBP before RET4. They return0 on failure or1 on
final success. Original stack argument is the monitor handle supplied by startup.

Reader scripts/ghidra/InventoryApiInitializationFlow.java now accepts optional
hex entry/exclusive bound, recognizes verified16-byte gateway and explicit frame
epilogues. Unrecognized RET4 remains an explicit stop. Prior20120869 inventory
rechecked unchanged:465 instructions/1596 bytes/three8-byte gateways/no stops;
its pinned-original verifier still passes. No Ghidra disassembly/flow overrides
or giant function expansion was performed merely to obtain decompiled C.

## Phases and next dependencies

| Native range | Observed work |
|---|---|
|2012205d..201220fc|Clear locals, SetCurrentDirectoryA2020d360; integrity checks;20121083(1) font-resource registration path|
|201220fc..20122332|Register FS98MAIN (proc2012e413, style3), call FS98CHILD helper20121360, register FS98FLOAT (2013e9a8), FS98SHADOW (2013d683), FS98DLG3D (2013f2e5), auxiliary styles4003h. Failures reportf002/f005 and return0|
|20122332..20122412|Read MAIN/ReportErrors, HideMenuNormal, HideMenuFullscreen, AlwaysHideMenuFullscreen, CursorAutoHide; IsDebuggerPresent default handling; call2013e64f and require nonzero result (stored void decompilation remains suspect)|
|20122412..20122437|Geometry20121796(monitor,&RECT); nonzero status invokes201213c4, zero response retries, other response aborts|
|20122437..201224e5|simprop.dll module_init_SIMPROP; CreateWindowExA classFS98MAIN, empty title, exstyle40000h, stylecf0000h, x/y20000, size from rectangle, parent/menuNULL. Store HWND2020e010, failure errorf001 and return0|
|201224e5..20122587|Interface2020de98 vtable+3ch receives RECT by value; publish2020de70; g2d.dll ordinal1, cancellation2020e044, ordinal4/interface slot10h, result-1 calls ordinal65 then fails|
|20122587..2012260f|Load WTSAPI32.DLL, resolve WTSRegisterSessionNotification/WTSUnRegisterSessionNotification, register main HWND with0 when both present; comctl32 ordinal17; install WH_KEYBOARD2 hook20122834 for current thread, store2020d018, return1|

Hook installation return is stored but does not gate final success. Exact semantics
of g2d ordinals/comctl32 ordinal17 and mode-interface slots remain open; importing
a name from a different DLL version is not proof. WTS resolution/call failure is
not promoted into an invented initialization error. Monitor geometry helper
20121796 has observed cbSize48h (72 bytes); existing MONITORINFO decompiler type
may be too small and needs a separate layout/type audit. Do not infer original
class sizes or correct all signatures from this one phase map.

## FS98MAIN creation and graphics startup

2026-11-11. The original PE inventory verifies 304 instruction
bytes in the phase from `20122437` to `20122587`. The source
calls `simprop.dll!module_init_SIMPROP` through the API boundary
at `200e13e0`, then creates FS98MAIN at (20000, 20000) with the
size derived from the startup RECT. The HWND is published before
the null check. Two source globals represent the original HWND
at `2020e010` and both receive the result. Creation failure
reports 0xf001 through the shared error callback.

The controller is published at the source boundary for `2020de70`
before its +0x3c slot receives the RECT by value. The graphics
phase calls `g2d.dll` ordinal 1, checks the quit flag, and queries
slot +0x10 on the receiver from ordinal 4. A result of -1 calls
ordinal 65 and returns failure. The original IAT slots are
`200e1328`, `200e1338` and `200e1324` respectively.
The package covers the API call sites. SIMPROP and graphics
receiver bodies are separate module work.

`api-main-window-create-20261111-001` passed all 20 comparators.
COFF SHA256 is
`595847D38B5B4B32576A9F0FCBD7B1B13F6C909FFA6E392764A0E7C6FAA2F642`.
No new byte matches were accepted. Full startup integration and
runtime verification remain open. Ghidra was unchanged.
The final phase starts at `20122587` with WTS session
registration, common controls and the keyboard hook.

## Session notifications and keyboard hook installation

2026-11-11. The original PE inventory verifies the final 136
instruction bytes from `20122587` through the return at
`2012260c`. The source loads WTSAPI32.DLL and resolves the two
session notification functions. It registers the main HWND for
session scope 0 when both are present. The unregister pointer
is the existing close callback at `2020d014`, now typed with
the BOOL result. The source checks the module before resolution.
The original attempts both lookups even when the load returns null.

The final calls are `comctl32.dll` ordinal 17 at IAT `200e1c28`
and SetWindowsHookExA with WH_KEYBOARD, the existing hook procedure,
null module and the current thread ID. The hook handle is stored
at the source boundary for `2020d018`. The phase returns success
even if WTS registration or hook installation fails.

`api-main-window-complete-20261111-001` passed all 20 comparators.
COFF SHA256 is
`A74B678236B0C9FFAD4ECC8898D37ECA0FE5FCE35C1978B789BAA5103EF291FD`.
No new byte matches were accepted. Ghidra was unchanged. Full
startup integration and runtime verification remain open.
The counterpart at `2012260f` unhooks the keyboard and unloads
the session module as part of module shutdown.

## Main window module shutdown

2026-11-11. The source now implements `2012260f`, called by
`ShutdownApplicationBodyPilot` before the stream cleanup. The
original unhooks `2020d018`, then unregisters FS98MAIN,
FS98CHILD, FS98SHADOW and FS98DLG3D in that order. FS98FLOAT is
absent from this sequence. It calls API service cleanup
`200f7c66`, `g2d.dll` ordinal 2 and
`simprop.dll!module_deinit_SIMPROP`. It unloads and clears the
language and WTS module handles, sets the directory from
`2020d360` and calls font resource removal at `20121083`.
WTS session unregistration occurs in the separate close handler
at `2012edbb`. The module shutdown routine leaves the resolved
function pointers and keyboard hook variable as observed.

The PE check covered 289 instruction bytes across `2012260f`
and `200f7c66`. IAT `200e1320` and `200e13dc` identify
the graphics and SIMPROP calls. Those DLL bodies are separate
module work. The API service cleanup and its callback lists
remain the next boundary.

`api-main-window-shutdown-20261111-001` passed all 20 comparators.
COFF SHA256 is
`BEC1E4B78B151EE9624D5EB91E5DC1B274C0A3CB43CC0BF46EA81666E845BB7D`.
New byte matches and runtime verification remain open. Ghidra
was unchanged.

## FS98CHILD source:100-byte near match, not accepted

2026-11-11 profile follow-up. The source entry
`ReadMainWindowStartupProfilePilot` covers `20122332` through the
transition at `20122412`. The pinned inventory verifies 208
instruction bytes in this phase. The original IAT identifies
IsDebuggerPresent and GetPrivateProfileIntA at `200e16b0` and
`200e16b4`. The original strings and destination stores give
these keys in section MAIN.

| Key | Default | Destination |
|---|---|---|
| ReportErrors | 0 with a debugger, otherwise 1 | `2020e0c0` |
| HideMenuNormal | 0 | `2020e01c` |
| HideMenuFullscreen | 1 | `201f716c` |
| AlwaysHideMenuFullscreen | 0 | `2020e020` |
| CursorAutoHide | 1 | `201f7164` |

The entry publishes the ReportErrors default before the profile
read and calls the existing language loader afterward. Loader
failure returns false before the idle flag or the remaining
profile settings are written. On success it sets `2020e040`
and reads the remaining keys in order.

`api-main-window-profile-20261111-002` passed all 20 comparators.
COFF SHA256 is
`331CF8B006246393F1B25059015CF7F319E1E794BF5218DA5B1BE317AC458A6C`.
Startup integration, new byte matches and runtime verification
remain open. Ghidra was unchanged. The next phase at `20122412`
starts the monitor geometry and retry loop.

2026-11-11 class registration follow-up. The source entry
`RegisterStartupWindowClassesPilot` models the class phase at
`201220fc` through the transition at `20122332`. It calls
FS98MAIN, FS98CHILD, FS98FLOAT, FS98SHADOW and FS98DLG3D in order.
The first failure stops registration. FS98MAIN reports 0xf002
and all four other failures report 0xf005 through the shared
error callback. The dialog registration stores the returned
atom before the status check, including zero on failure.

The pinned PE verifier passed for the whole bounded inventory,
including 534 instruction bytes in this phase. The source entry
covers the class phase and its error reports. The enclosing
startup routine and its integrity transfers remain integration
boundaries. The next phase at `20122332` reads MAIN profile
settings before the monitor geometry and main HWND setup.
Ghidra was unchanged.

`api-startup-classes-20261111-001` passed all 20 comparators.
COFF SHA256 is
`5985F3B7E701F069B31A62948BF2D53FA8BF2372C942CE387ADAB1BD0D97DDF7`.
New byte matches and runtime verification remain open.

Source main_window_classes.cpp implements observed fields with explicit ordinary
C++ assignments and bool RegisterClassA result. Run api-main-child-class-20261103-001
emits100 bytes. Six fields/string identities validate; full-body comparator
compare-api-main-child-class.cjs rejects exactly offset7: native stack reserve28h,
original2ch. Other99 byte positions agree after these field mappings. This is
diagnostic near-match evidence only, no accepted partial/complete coverage.
Do not insert synthetic padding/unused locals to make a frame match. Stop this
source-shape question at1 without a flag matrix. Source/hash/log/compiled inputs
and main-child-class-mismatch.json retained. Object SHA256
571136890B17B93933F674172A953ECF251C46D9DB0A5DB5593510C4674E261A.

## Keyboard hook20122834: complete188-byte source match

Original three stack words/RET12; EAX returns CallNextHookEx result, contradicting
earlier void decompilation. Source main_keyboard_hook.cpp uses structured branches.
Only code0 performs processing. It reads release bit31 and Alt-context bit29;
queries interface2020de70 slot+4 as bool and requires focus differs from main HWND.
Then forwards Alt+Tab release, Alt+Escape release or Ctrl+Escape release to main
window with PostMessageA(WM_SYSKEYUP). Always chains via hook2020d018. Mode slot
meaning is provisional; source defines only two observed vtable slots with
UnknownSlot00 and QueryMode, not an original class name/size.

Three bounded trials, same fixed factory COFF profile:
1. api-main-keyboard-hook-20261103-001:188 bytes, reversed mask-register allocation.
2.002: exchanged independent mask-computation declarations; all but byte138 match,
   native JGE versus original JNS after TEST AX,AX from signed GetKeyState<0.
3.003: natural explicit GetKeyState&8000h condition emits original JNS; all188 bytes
   match after8 individually checked DIR32 fields. No asm, padding or goto.

Comparator scripts/compare-api-main-keyboard-hook.cjs validates full body and
rejects opcode, mode-pointer and WM_SYSKEYUP mutations. Final object SHA256
5589D1F544267CA8A00C12F53481D633E2794E7C732D15C3A14042C4151260FE.
All prior six gates pass on trial003; new disjoint complete total1028 code bytes,
44 metadata; Initialize partial123 separate. Stop at3 success, no further variants.

## Evidence and actual Ghidra edits

Artifacts api-main-window-{flow,verification,analysis,memory,typing}-20261103.json.
Verifier scripts/verify-api-main-window-flow.ps1 checks every raw instruction,
non-overlap, descriptor separation and interval coverage against pinned original.
Five additional live spans (main1458, gateway84, helper822, hook188, class100)
equal original bytes, including after type edits.

Pre-edit API current as user selected; fsx.exe also open. Backup
artifacts/api.dll/ghidra-backups/api-before-main-window-evidence-20261103.gzf,8541915 bytes.
TypeApiMainWindowHelpers.java actually applied hook stdcall3/LRESULT32 with named
arguments and class cdecl/bool, preserving existing FUN names. Main entry receives
warning/evidence plate comment only; its old76-byte stored body/signature remain
explicitly incomplete, no disassembly/pcode/flow/data/table patch. Target arguments
and readback verified; no VTable type changes. Saved revision
1793739333501:636d9e2e:603, unsaved_changes=false. Maintained src search has no goto;
Ghidra decompiler may still display its own goto statements.

Unapplied naming suggestions: FUN_20122834 -> MainKeyboardHook,
FUN_20121360 -> RegisterMainChildClass, FUN_2012205d -> InitializeMainWindow
(last provisional for whole routine until all callee contracts are verified).
Next geometry20121796/201215fd monitor-info layout and error-response201213c4,
then font20121083 and2013e64f true-return inventory. Do not reopen ended native
questions without new evidence. No additional installation files required.
