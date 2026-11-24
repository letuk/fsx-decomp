# G2D backend controller initialization

2026-11-12. Live verified in g2d.dll, SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.
Read-only Ghidra pseudo-disassembly of the four targets and three
shared helpers matches 1,518 pinned PE instruction bytes. The original
table cells, dynamic library strings and API import were verified
separately. Backend names describe the observed module dependencies.

| G2D address | Verified behavior |
|---|---|
| 20668d10 +0x44, 20696b90 | DXGI table points to a three-byte leaf that clears AL. |
| 20668a00 +0x44, 20683270 | D3D9 table points to a three-byte leaf that sets AL to one. The shared transition uses this predicate when controller byte +0x1d is already set. |
| 206935c0, 20689aa0 | The DXGI-side startup target loads dxgi.dll and d3d10.dll. The other target loads D3D9.DLL and resolves Direct3DCreate9. The shared caller publishes different primary tables for these paths. Dynamic object setup remains a separate source boundary. |
| 206988d0 | Returns blocked if +0x1a is set, API reports quit, the window array is nonempty, or the API controller is absent. Otherwise sets +0x1a and initializes +0x18 from API controller slot +4. Both +0x4c bodies translate a blocked start to 0x40002. |
| 20689b80–20689d2c | D3D9 uses the interface at controller +0x24. Around the desktop and legacy preparation calls, sets API ordinal 216 to 1 and restores the returned prior value. Reads the adapter count through interface +0x10, capped at nine. Each created window is appended before its +0xbc call. Allocation failure sets -1. A nonzero +0xbc sets the partial flag and stops enumeration without directly replacing the status. Legacy preparation cleanup runs before shared finalization. |
| 20693730–206939f0 | DXGI enumerates adapters through factory +0x2c's slot +0x1c, then outputs through each adapter's +0x1c, capped at nine successfully appended windows. Queries adapter and output descriptors. The first output of each adapter uses 2068f710. Later outputs use 2068f8f0 with that first window. Appends only after window +0xbc succeeds. A failed +0xbc is retained as status and invokes that window's +0x9c with 1. Releases each enumerated adapter after its output loop. A zero count from controller +0x0c returns 0x20001 directly and bypasses shared finalization. |
| 20698950 | If the first window reports true from +0x48, suspends the range and sets status -1. In the partial case, visits secondary windows through +0x8c with 1. With zero status and no deferred suspend, services deferred refresh or calls 20698a70 directly. |
| 20698a20, 20689ce1–20689d18 | On nonzero status and a clear API quit flag, invokes ordinal 65. Then clears +0x1a. If +0x19 is set, applies suspend and converts an otherwise successful result to 0x40002. Clears deferred suspend and refresh bytes +0x19 and +0x1b. |

The API import at g2d.dll:206613ec is api.dll ordinal 216. The
API export table resolves it to 2011f16e, where the input is stored
at 2020de48 and the prior value is loaded for return. Deeper API
preparation effects are outside this G2D package.

## Source and verification

src/g2d-pilot/controller_transition.cpp contains the four source
targets and shared initialization, completion and finalization
logic. It uses compile-time checks for the backend field offsets,
keeps the G2D and API sources separate, and retains the original
dispatch boundary. Constructors, private-heap allocation and window
array insertion are represented as source interfaces. The DXGI
construction interface covers the distinct first and later output
helpers. The backend descriptors are opaque buffers sized to cover the
observed x86 stack destinations. No original full class layout or
table-placement change is claimed.

VC8 x86 run g2d-controller-backends-20261112-001 compiled the unit
with /EHsc and no compiler warnings. The worker source hash agrees
with the workspace. All four target symbols emit ECX fastcall
entries. COFF SHA256 is
436FFC191761823E94A67F0B5CC1C6BFD8E0669C6A5858F2C8EBF00E2AD60702.
Byte matching, full DLL linking and runtime verification remain open.
Ghidra stayed read-only and reported changed=false for g2d.dll.

Suggested Ghidra names, unapplied. In 20693730 the loop slots
[ESP+0x20] and [ESP+0x24] are adapterIndex and outputIndex based on
the two enumeration calls. In 20689b80, [ESP+0x1c] is adapterIndex
based on its increment and comparison against the capped adapter
count. Stack offsets refer to the fixed frame between calls.

The next package is backend window construction and publication at
206857d0, 2068f710, 2068f8f0 and the array append paths. Dynamic
backend setup at the two +0x40 targets and later window replacement
at 20698a70 remain separate boundaries.

The constructors and controller-list append are now connected.
See g2d-window-construction.md for the per-window initialization boundary.
