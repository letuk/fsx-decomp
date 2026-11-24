# API main-window modal controller

2026-11-12. Live verified against api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Read-only Ghidra pseudo-disassembly recovered the undefined modal
targets and surface helper. Their instructions and the retained
visitor, close and graphics-message listings match 2,357 original
PE bytes. The PE table, IAT identities and target calling sequences
were checked separately.

| API address | Verified behavior |
|---|---|
| 200f2d2c, 20125979–201259d7 | Slot +0x4c uses ECX receiver and no stack arguments. Increments signed controller DWORD +0x4c. The transition to one saves GetFocus at +0x54, calls window.dll ordinal 5 with 1, hides FS98FLOAT windows on the current thread via 201250ee, selects dialog mode (2, true), emits event 0x20004 and tail-calls 20125883. Other depths return immediately. |
| 20125883–20125978 | Controller slot +0x14 selects the clear path. With a false result it fills the main-window client rectangle with BLACK_BRUSH. Otherwise g2d.dll ordinal 4 supplies the indexed list, the count at +0x0c, the records at +0 and HWNDs at +4. Only records with a true slot +0x40 predicate are cleared. The DC uses flags 0x12, then ReleaseDC follows the fill. G2D implementations remain external. |
| 200f2d30, 201259d8–20125a5c | Slot +0x50 uses ECX receiver and no stack arguments. Decrements the same depth and performs cleanup only at zero. If main.dll ordinal 20 returns nonzero, calls window.dll ordinal 5 with 0. Emits event 0x20005, selects dialog mode (2, false) and enumerates the existing 20125085 visitor, which sends 0x467 to FS98FLOAT windows. Restores the saved focus only if its HWND is valid and visible, then clears +0x54. The decrement has no lower-bound check. |
| 200f2d34, 200f2d38, 20125a5d, 20125a61 | Slot +0x54 reads byte +0x3e in AL. Slot +0x58 tests the low byte of one four-byte argument, calls the entry or exit slot, stores the input byte at +0x3e and uses RET 4. The flag records the last requested state. The separate depth controls nesting. |
| 200f2d28, 20125a8f, 2012edbb | Slot +0x48 stores its DWORD argument at +0x50 and uses RET 4. Nonzero enables the HWND and clears WS_MAXIMIZEBOX. Zero sets WS_MAXIMIZEBOX. WM_CLOSE still takes the termination path for positive depth, or for nonzero 2020e040 with +0x50 equal to zero. |
| 20123637, 2012365d | The WinTrust interactive branch calls the modal pair around WinVerifyTrustEx. The actual slot targets match the table above. |
| 2012fce4, 2012f578, 2012f637 | The FS98MAIN dispatcher routes 0x46f to 2012f4f7. Its graphics transition temporarily exits and re-enters modal mode when the controller mode is 2. This larger graphics transition is the next package. |

Cross-module reads were bounded to the imports. window.dll ordinal 5
at 210237c0 (PE SHA256
57D83DE698215ACDDEF78CE486831E5BBD19EA2E6F6DD8C297DE304FB3E946CD)
takes one stack DWORD and changes bit 0x200 in window records on a
state change. main.dll ordinal 20 at 20955a28 (PE SHA256
44400C43A5D07C390A7028654862754730050EE9F7822B37AA207FCE463F8344)
takes no stack arguments and returns 1 when 20a08dbc equals 11 and
signed 20a08db8 is at least 11. The MAIN state names are open.

## Source and verification

src/api-pilot/main_window_modal.cpp adds both bodies, the focus and
graphics paths, the byte flag getter/setter and the close override
setter. Interactive verification calls the reconstructed bodies
directly. The existing dialog-mode setter and refresh visitor are
reused. The source initializes the temporary window-class and rectangle
buffers. Imported MAIN, WINDOW and G2D operations remain boundaries.
The new controller methods are source targets. Original table placement
and other callers, including 0x46f, remain open for source integration.

VC8 x86 run api-main-window-modal-20261112-001 passed all 20 existing
comparators. The COFF relocations connect interactive verification
and the byte flag setter to both __thiscall bodies. COFF SHA256 is
06EFD03D05DDEFF592A45CA73680991C437336959B60C7995536C9C37D715B79.
The ledger remains 5,263 code bytes and 288 metadata bytes. New body
matches, complete DLL linking and runtime verification remain open.
Ghidra stayed read-only and reported changed=false for api.dll.

The pilot renamed closeField4c to modalDepth, closeField50 to
closeOverride and split the former unknown54 into modalSavedFocus
and an opaque +0x58 DWORD. These are reconstructed field names.
Suggested Ghidra names, unapplied. At 20125a61, param_1 →
modalRequested based on the branch and byte store. At 20125a8f,
param_1 → closeOverride based on the +0x50 write and WM_CLOSE
read. The original entry and exit targets have no stored Ghidra
function at the inspected addresses.

Next package is the FS98MAIN 0x46f graphics transition at 2012f4f7,
including the temporary modal suspension, clip rebuild and related
0x470 entry. The protected key and source shutdown-prefix integration
remain separate boundaries.

The FS98MAIN graphics transition package now connects the modal pair
to 0x46f and handles 0x470. See api-main-window-graphics-transition.md.
