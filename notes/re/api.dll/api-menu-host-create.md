# API menu host creation

Original `api.dll`. Five listings totaling 676 bytes were checked
against the PE. The two factory callbacks were also checked in live
Ghidra. Names and layouts are reconstructed.

`2013a534` and `2013a5be` use the HWND in EDI and pass an observed
0x20-byte creation span to SimXUIWindow_Create. The span has value
0x3b at +0, callback `2013af2f` at +4, the identifier at `200f3604`
at +8, and the HWND at +0x1c. They attach the host and query the menu
interface. Query failure calls slots +0x58 and +0x28. The auxiliary
route additionally calls the graphics controller through slot +0x38.

`2013a20b` creates the default host lazily, using callback `2013aeab`
and a null creation HWND. It publishes the host at `2020e090`, reads
the host HWND through slot +0x44 and attaches its root property.
It queries the identifier at `200f3604`.

The callbacks initialize the observed 0x14-byte allocation and pass
its +4 field as output to SimXUIWindow_CreateBaseInstance. They use
distinct table addresses `200f3370` and `200f3410`. Base creation
failure calls slot +0x98 with argument 1. The source preserves the
original output assignment before this cleanup and the unchecked
outer creation result.

The API import slots `200e1460` and `200e1464` identify
`ui.dll` creation and base-instance creation. The source reaches
that interface and leaves the two API table implementations and
default host shutdown as the next integration boundary.
The two window host routes are connected to the context-click path.
The default host entry is available for its startup caller.

Suggested Ghidra names, unapplied. In both callbacks, `local_8`
→ `hostStorage` from the heap allocation output and table write.

`api-menu-host-create-20261110-002` passed all 20 existing
comparators. COFF SHA256 is
`82FF679A64FE43ADD0D283AB566F2736992C7144F03112DC0E4DB674A88B6742`.
No new byte matches were accepted. Linking and runtime verification
remain open. Ghidra was read-only.
