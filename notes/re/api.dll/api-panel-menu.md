# API panel menu construction and state

Original `api.dll`. Live decompilation and cross-references were
checked against five retained PE listings totaling 6888 bytes.
Addresses and flag values below are observed. Source names are reconstructed.

- `2012b637` releases the old context +0x44 handle through menu
  slot +0x70, clears the field and asks slot +0x6c for a replacement.
  A failed HRESULT or zero handle ends the path. The ordinary
  panel route adds commands 0x201, 0x202 and 0x300 subject to the
  controller and panel flags, then applies the common state routine.
  Its existing caller is `2012d08c` at `2012d278`. The caller body
  is part of the pointer-to-menu routing boundary.
- `20141195` is reached from construction and from FS98CHILD
  WM_INITMENU at `20142fd9`. The latter passes WPARAM as the menu
  handle. Slots +0x78 and +0x7c receive the check and availability
  state. Panel kind 3 invokes the object at window +0xd0 through
  slot +0x84, then adds the standard entries and two submenus.
  Submenu selection uses mode values 0 through 3 and 1, 0, 2
  respectively in displayed order.
- `2013ad62` checks the handle and resolves the menu interface
  before passing flags, item and text to slot +0x88. Source reuses
  this operation across the two paths. It preserves the shared
  260-byte text buffer in construction and the 0x100-character
  resource limit in the kind-3 path.

The initial DWORD at `201f70b4` is 1. The read at `201417e0`
tests it for the 0x300 check state. Writes include `20141853`,
the next menu command package. The source exposes the word for
those callers. The window IAT slots `200e1530`, `200e152c` and
`200e1528` identify `window.dll` ordinals 34, 35 and 36.
`200e1224` identifies the reused `controls.dll` ordinal 45.
These modules are represented at their import interfaces.
The receiver targets for the observed virtual calls remain open.

Suggested Ghidra names, unapplied. At `20141195`, `unaff_ESI`
→ `panelContext` from its window field reads, `param_1` →
`menuHandle` from both callers, and `local_114` →
`submenuHandle` from the +0x6c output and +0x88 calls.

`api-panel-menu-20261110-001` passed all 20 existing comparators.
COFF SHA256 is
`E5DD99E6E2F4D1D2752B1D956794887F50F9EFF03672C40A3598D45CC9589A43`.
Accepted coverage remains 5263 code and 288 metadata bytes.
Linking and runtime verification remain open. Ghidra was read-only.
