# API panel menu host lookup and release

Live verified in original `api.dll`. Eight retained listings,
1126 instruction bytes, were checked against the original PE.
The API IAT confirms GetAncestor, GetPropA and GlobalAddAtomA.
Names in the new source are reconstructed.

`20138ee1` takes the HWND in EAX and an interface ID in EDI.
For a nonnull HWND it finds GA_ROOT and reads the lazily atomized
`MenuHostWindow` property. A null HWND uses `2020e090`.
It calls host slot zero with the host, interface ID and output
pointer, returning the output only for a nonnegative HRESULT.
The decompiler omits arguments at this call. The instruction
listing establishes all three arguments and the output load.

`20138f47` supplies interface ID
`4875a937-debd-43d4-bf61-37e34b5951d8` from `200f3614`.
For a panel, its link at +0x98 and linked HWND must be nonnull.
A null panel queries the default host directly.
`20139211` preserves an existing root property, installing
the supplied host only on a valid root with no property.
`2013928e` removes an existing property and returns its
previous value. The source preserves the original treatment
of SetPropA's return value.

The source connects the release sequences in `2013f296` and
`201401d3` through this lookup and interface slot +0x70. The
historical symbol ReleasePanelContextGraphicsSourcePilot is
retained for existing callers, with a correction comment.
Context +0x44 is a menu-interface handle in this path.
The earlier graphics-context interpretation is superseded.

`2012b637` confirms the paired operation at slot +0x6c,
which obtains a handle for the panel before adding menu entries.
It ends by calling the menu-state handler `20141195`.
The default host is published by `2013a20b`, which calls
SimXUIWindow_Create, and cleared during `2013a1ac`.
These bodies were inspected for provenance and delimit the
next menu-construction package. The [panel menu follow-up](api-panel-menu.md)
covers the entries and state. Host creation and the interface
implementations remain integration work.
Only API bodies were inspected and reconstructed in this package.

Suggested Ghidra names, unapplied. At `20138ee1`, `puVar1` →
`menuHost` from the root property lookup. At `20138f47`,
`in_EAX` → `panelWindow` from the link and HWND accesses.
At `2012b637`, `local_8` → `menuHandle` is a provisional
name supported by the paired +0x6c and +0x70 calls.

`api-panel-menu-host-20261110-001` passed all 20 existing
comparators. COFF SHA256 is
`4747D775A8CB99AEF9489C8BC9A9AEA021341DDDAE5A36207AE6AC5C93F67C66`.
No newly accepted matches. Coverage remains 5263 code and 288
metadata bytes. The result is a COFF compilation and regression
check, with linking and runtime verification open.
Ghidra was used read-only.
