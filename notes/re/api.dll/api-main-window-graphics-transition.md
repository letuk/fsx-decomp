# API main-window graphics transition

2026-11-12. Live verified against FSX Acceleration api.dll, SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Seven retained listings match 2,044 instruction bytes in the pinned PE.
Additional PE reads establish the controller slot +0x0c, import identities
and exporting module addresses. Cross-module listings were read without
switching tabs or editing programs.

| API address | Verified behavior |
|---|---|
| 2012fce4, 2012f4f7 | FS98MAIN dispatches 0x46f with the controller in ESI and two stack arguments. Every branch returns the result of DefWindowProcA for 0x46f. The first call is controller slot +0x0c, whose table cell 200f2cec targets 2012543c. The target calls g2d.dll ordinal 61, saves AL at controller +0x39 and returns AL. |
| 2012f50e–2012f557 | If the slot returns false, optionally moves windows when controller +0x38 is set and G2D exists, invokes G2D controller slot +0x14 when G2D exists, and stores 1 at controller +0x3c. With a true slot result but byte +0x3a clear, it only stores 1. |
| 2012f64b–2012f68f | With the slot result and byte +0x3a set, but +0x38 clear, invokes G2D +0x10, invokes ordinal 65 if its result is nonzero, then calls G2D +0x38 and broadcasts synchronous 0x46c to descendants through 201252a6. Clears controller byte +0x3c and refreshes visibility through controller +0x18. |
| 2012f563–2012f646 | With all three gates set, mode 2 at +0x44 suspends one modal level. Emits event 0x20025 with WPARAM and zero, moves the windows, brackets G2D +0x10 and the conditional ordinal 65 with controller DWORD +0x34 increment/decrement, calls G2D +0x38, and restores parents. Emits 0x20026 with WPARAM and zero, enables the cursor override, posts WM_SIZE with zero parameters to indexed G2D HWNDs, reorders the clip windows and disables the override. Restores the suspended modal level, clears +0x3c and refreshes visibility. |
| 20126346 | Enumerates window.dll ordinal 15 entries. Sets link +0x10 to 0xffffffff even for an entry excluded by flag 0x40000000. Eligible records with cached graphics data and an HWND outside the main window record the parent index from graphics slot +0x30 and reparent to the main HWND. |
| 201263e4, 201265b6 | For eligible entries with a saved index, looks up the HWND through G2D +4, reparents when the HWND exists, appends the child HWND and resets the index. Calls WINDOW ordinal 16 before iterating the populated array and sending synchronous 0x467 to each recorded HWND. The original checked-vector loop supersedes the retained C output that eliminated the append-driven iteration. |
| 2012fccc, 2012f694 | FS98MAIN dispatches 0x470 only when controller byte +0x38 is nonzero. The helper finds the main HWND in G2D via +0x1c. A hit sets cursor resource 0x7f02 and invokes the graphics window's +0x88 slot with 1. All paths return zero. |

G2D ordinals 4, 61 and 65 resolve to 20698050, 206bea10 and 20699720
in g2d.dll (SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3).
Ordinal 4 returns the pointer stored at 20709bd8. Ordinal 61 opens
the desktop named Default, calls SwitchDesktop, closes the handle
and returns the switch outcome in AL. The internal graphics
transition and failure-report targets remain external.

WINDOW ordinals 15 and 16 resolve to 21025a70 and 21041590 in
window.dll (SHA256
57D83DE698215ACDDEF78CE486831E5BBD19EA2E6F6DD8C297DE304FB3E946CD).
The first publishes borrowed array storage at 210480c0. Ordinal 16
clears EAX and returns with RET 4 in this build. The API source retains
the observed pair of calls.

## Source and verification

src/api-pilot/main_window_graphics_transition.cpp connects both messages
to the FS98MAIN procedure and reuses modal enter/leave, cursor override
and clip reordering. The source uses a standard vector for the restored
HWNDs, keeps the WINDOW snapshot boundary, and models the observed
controller +0x0c target directly. Original table placement remains open.
The source names controller +0x34 controllerOperationDepth and +0x3c
graphicsTransitionFlag. Their wider uses remain open. Ghidra received
no edits and has no new unsaved changes from this work.

VC8 x86 build api-main-window-graphics-transition-20261112-002 passed
all 20 existing comparators. The final build models the G2D ordinal 61
return as bool in AL and checks the field offsets at compile time.
COFF SHA256 is
7E07832E163AD37A302CD833DB5FD14C051605F8F1DC23635CBE2ACEDDFF27E6.
The accepted ledger remains 5,263 code bytes and 288 metadata bytes.
New body matches, DLL linking and runtime verification remain open.

Suggested Ghidra names, unapplied. At 2012f4f7, param_1 to wParam
and param_2 to lParam follow the DefWindowProcA call. At 201263e4,
local_18 to windowEntries and local_14 to windowCount follow the
WINDOW ordinal 15 outputs. The retained C puVar4 has multiple uses
and needs its live ranges separated before applying a name.

The next boundary is the G2D controller behind ordinal 4. Begin with
the targets of slots +0x10, +0x14 and +0x38 and their relationship
to ordinal 65. The 0x470 window slot +0x88 is a separate G2D receiver.

The G2D shared slot and ordinal 65 implementations now have a separate
source unit. See ../g2d.dll/g2d-controller-transition.md for the backend boundary.
