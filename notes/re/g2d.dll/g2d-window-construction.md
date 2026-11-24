# G2D window construction and publication

2026-11-12. Live verified against the pinned g2d.dll PE with SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.
Read-only pseudo-disassembly matched 2,349 listed bytes in the PE
across the constructors, sentinel allocator and pointer-list routines.
The table cells and import identities were checked separately.

| g2d.dll address | Verified behavior |
|---|---|
| 20696ba0 | Shared constructor takes four stack arguments with RET 0x10. Installs base table 20669928, constructs two VC8 strings at +0x54 and +0x70, and stores the three caller indices at +0x8c, +0x90 and +0x94. Defaults include byte +0xcb set to one and DWORD +0xe8 set to four. |
| 20689c47, 206857d0 | D3D9 allocates 0x810 bytes via API ordinal 106. The constructor receives the adapter index in EAX and the storage on the stack. The base receives the adapter index for its first two indices and zero for the third. Installs primary table 20668890 and secondary table 20668968 at +0xfc. Initializes the defaults block at +0x34 from 20701c90. |
| 20693836, 20693884, 2068f710, 2068f8f0 | Both DXGI paths allocate 0x1b0 bytes via API ordinal 106 and install table 20668c20. The base receives the global window index, adapter index and output index in that order. The constructor stores the adapter at +0xfc and calls its COM AddRef. It stores the enumerated output at +0x100 without a corresponding AddRef. Later outputs store the first window of their adapter at +0x108. The first-output path initializes DWORDs +0x1a8 and +0x1ac to one and zero. Both paths point +0x110 at the new window and initialize the defaults block from 20701c94. |
| 206a8540, 2068f78b, 2068f96e | DXGI allocates a 0x18-byte sentinel via API ordinal 106. The constructor stores it at +0x154 and makes the first three pointers self-referential. The allocator sets byte +0x14 to one, and the constructor sets byte +0x15 to one. |
| 20689d80, 20689df0, 20689eb0, 2068a1c0 | End insertion writes directly when capacity is available. Growth uses the larger of size plus one and capacity plus half capacity when representable. It copies the old range, inserts the pointer, frees the old block via API ordinal 108 and publishes begin, end and capacity-end at controller +8, +0xc and +0x10. The source specializes the generic insertion path for the observed end insertion. |
| 20689c8c, 20689c9b, 206938de, 2069391e | D3D9 publishes before slot +0xbc. DXGI publishes only after +0xbc succeeds. On a failed DXGI +0xbc, the existing controller path calls the window's +0x9c target and leaves it out of the list. |

The implementation is in src/g2d-pilot/window_construction.h and is
compiled by controller_transition.cpp. Both backend loops call the
new constructors and list append. The source uses the existing API
private-heap interface and keeps all table symbols explicit. To handle
recoverable allocation failure it allocates the DXGI sentinel before
base construction and uses C++ cleanup for partially constructed
strings. Object sizes above are original allocation observations.
Final table placement, import binding and runtime behavior remain open.

VC8 x86 run g2d-window-construction-20261112-002 compiled both inputs
with /EHsc and without compiler warnings. The first build warned
about internal functions declared with C linkage that can throw.
The retained build uses C++ linkage and emits call relocations from
the backend loops. Both compiled-source hashes match the workspace.
COFF SHA256 is
C40D2517FC76D752CDB9A64891FFEBAFA8FFE1C7F9225821C5769C9A053B0F27.
No new byte-match comparator or full DLL link. Ghidra received no
database edits. The previously observed g2d.dll state was changed=false.

Suggested Ghidra names, unapplied. At 20696ba0 the stack slots
EBP+0xc, EBP+0x10 and EBP+0x14 can be named windowIndex,
adapterIndex and outputIndex based on the callers and the stores
to +0x8c, +0x90 and +0x94. In the two DXGI constructors, the stack
arguments at EBP+0x10 and EBP+0x14 are adapter and output based on
the enumeration and COM ownership flows.

Next package is per-window graphics initialization at D3D9 20685a60
and DXGI 2068fc80, the targets of +0xbc. Their failure cleanup targets
at +0x9c are 20685920 and 2068f8c0, respectively.

See g2d-window-initialization.md for the connected +0xbc paths.
