# API menu host initialization

Original `api.dll`. Four retained instruction listings totaling
901 bytes were verified against the PE. Names are reconstructed.

Shared table slot +8 at `201391d1` forwards to base slot +8,
publishes the returned window record when an output is supplied,
sets record +8 to 0x830e4000 and +0x70 to 0x6000, and preserves
the base call's EAX result. Shared slot +4 at `20138fee` stops
on a nonzero result, otherwise selects a parent and calls the
window creation helper `2011cc66`. An error there releases the
record via `window.dll` ordinal 14 at IAT `200e1524`.

On success, the API reads the parent client dimensions and
scales a width/height pair through `2011d4cd`. It stores the
scaled pair at +0x28 and initializes both client rectangles at
+0xac and +0xbc. It attaches the host HWND, finalizes the record,
registers the message chain and focus callback, and creates the
canvas. Canvas success leads to HWND assignment and size setup.
The focus callback at `201392f6` uses the first and third of its
four stack arguments. It synchronizes Win32 focus with entry
into or departure from the default host's XUI subtree.

The API-side IAT identifies the message-chain callback in
`ui.dll` at `200e145c` and focus/canvas operations in
`xuipc.dll` at `200e15f4`, `200e15f8`, `200e15fc` and
`200e15c0`. These are separate module boundaries. The source
provides the API targets for slots +4 and +8 and the focus
callback. Table placement, callback-list registration and the
larger window record helpers remain integration work.
The next related API package is window record creation and
finalization at `2011cc66` and `2011d262`.

Suggested Ghidra names, unapplied. In `20138fee`, `piStack_4c`
→ `windowRecord` from the +8 output and later record writes.
In `201392f6`, `param_3` → `nextElement` is provisional,
based on the subtree check and subsequent SetFocus call.

`api-menu-host-init-20261111-002` passed all 20 existing
comparators. COFF SHA256 is
`9F14E4CE2B4134CE3BD3314D08ADF81BC815E128DF3FEF4C4743D5540DC38015`.
No new byte matches were accepted. Linking and runtime
verification remain open. Ghidra was unchanged.
