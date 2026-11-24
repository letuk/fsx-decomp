# simscheduler.dll CRT conversion and adjacent import thunks

Live verified in simscheduler.dll from `0x20cc6e86` through the
padding before the previously matched SSE2 probe at `0x20cc6f67`.
Ghidra reports no unsaved changes after read-only inspection.

The conversion region spans `0x20cc6ea0–0x20cc6f4a` (171 bytes).
The RTM CRT map identifies entries at `0x20cc6ea0` (`__ftol2_sse`),
`0x20cc6ea9` (`__ftol2_pentium4`), `0x20cc6ebc`
(`__ftol2_sse_excpt`), and `0x20cc6ed6` (`__ftol2`) by their
matching offsets within `ftol2.obj`. These are linked symbol
identifications. Ghidra retains `FUN_20cc6ea0` with a 145-byte body
and no function at `0x20cc6ebc`. The 26-byte secondary entry tests
SSE2 availability and the x87 exception masks, then branches to
the shared SSE2 or x87 conversion path. The converter consumes
the x87 top-of-stack value. The SSE2 path returns a truncated
integer in EAX and the x87 path returns the wide result in EDX:EAX.

The complete region matches after validating both references to the
SSE2 availability flag at `0x20cc83f4`. The six-byte thunks at
`0x20cc6e86` (`memcpy`), `0x20cc6f50` (`__CxxFrameHandler3`),
and `0x20cc6f5c` (`memset`) each match after IAT normalization.
The comparator checks the PE HIGHLOW entry and exact MSVCR80 import
identity for each thunk. Ghidra has no function object at
`0x20cc6f50`. Runtime execution and imported implementations remain
unverified.

The XP worker returned exit code zero for
`simscheduler-crt-conversion-20261116-001`. The build uses the VC8 SP1
compiler, pinned RTM CRT, and linker includes for the conversion object
and three thunks. The retained PE32 DLL SHA-256 is
`c278af4b1b8542e73c1c73bdff5c3d27a017fc7f1e7d1dd5b4fd550e219dd000`.
Startup and SSE2 comparisons were replayed in the same build.
The pre-existing DllMain diagnostic remains. The retained Flight save
comparison also passed.

- [Original Ghidra listings](../../../artifacts/simscheduler.dll/crt-conversion-ghidra.json)
- [Linked conversion comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-crt-conversion-20261116-001/comparison-20cc6ea0.json)
- [Build command](../../../vm-share/modules/simscheduler/runs/simscheduler-crt-conversion-20261116-001/build.cmd)
- [Worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-crt-conversion-20261116-001/receipt.json)
