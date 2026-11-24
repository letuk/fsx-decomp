# API main-window close and quit flag

Live verified in `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Eight retained listings totaling 2065 bytes agree with the original PE.
Live xrefs identify four direct writers of `2020e044`. Names below are
reconstructed.

## Close paths

The FS98MAIN dispatcher calls `2012edbb` at `2012fa30` for WM_CLOSE,
with controller in ESI. A positive signed +0x4c, or nonzero
`2020e040` with zero +0x50, selects the termination path. It calls
`demo.dll` ordinal 7 through API IAT `200e123c`, then returns the
result of TerminateProcess on the current process with exit code zero
if control returns. The demo implementation remains external.

The other path optionally unregisters the notification at `2020e114`
through `2020e110`. The resolver `201434eb` requests
UnregisterDeviceNotification from USER32. It then calls the optional
HWND hook at `2020d014`, kills timer `2020e018` and ends period
`2020e014` through WINMM IAT entries `200e1c5c` and `200e1c58`.
It clears the event and idle callbacks at `2020e030` and `2020e02c`.
If the quit flag was clear, it writes one and calls controller slot
+0x24. Table field `200f2d04` points to the previously traced hide
routine `20125ee5`. It calls the application callback at `2020e038`
when present and `2020e048` is nonzero, then returns DefWindowProcA.

## Shared quit state

| Writer | Observed action |
|---|---|
| `2012df99` in export 4 `2012df8e` | Sets the flag, hides via +0x24, sends synchronous WM_CLOSE to `2020e010`, returns one. |
| `2012dfc9` in export 5 `2012dfbd` | Sets the flag only for nonzero input, always posts WM_COMMAND 0x110a with the original LPARAM, returns one. |
| `2012ee3f` in WM_CLOSE | Sets the flag on the normal cleanup branch if clear. |
| `20122b2d` in `20122ae1` | Sets the flag when startup mode is zero and posts 0x110a with LPARAM one. The loop later calls PostQuitMessage when the flag is set. |

The window procedure now shares the existing source global for the
quit flag with the message loop and clip-region code. Constructor
field +0x5c is explicit and untouched, correcting the previous
zero-fill that extended one DWORD beyond the original +0x58 store.
The event callback's result is modeled as an int because `2012e96f`
tests EAX to decide whether WM_COMMAND was consumed.

Suggested Ghidra names, unapplied. In `2012edbb`, `unaff_ESI` →
`mainWindowController` from the dispatcher and HWND/slot reads.
At `2020e044`, `DAT_2020e044` → `quitRequested` from the four
writes and PostQuitMessage reader. At `2012dfbd`, `param_1` →
`commandData` from the PostMessageA argument. The purpose of the
controller fields +0x4c and +0x50 remains open.

## Native result

The first build caught an offset error in the newly split controller
view. The corrected `api-main-window-close-20261109-002` passed all
20 existing comparators. Its object SHA256 is
`A675701BA0452D3167E711B60FA6982D89C91F1A95E88774AB39FB5F3BF5E129`.
The 54-byte export 5 body is a match candidate with three DIR32
operands. Export 4 has different instruction ordering. No new range
was added to the byte ledger. No linked replacement was executed.
Ghidra was used read-only.

The follow-up package connects WM_COMMAND at `2012e96f` and traces
the 0x110a callback translation. See api-main-window-command.md.

The modal package resolves the +0x4c counter as modal nesting depth
and +0x50 as the close override. See api-main-window-modal.md for
the original writers, focus lifetime and the integrated source.
