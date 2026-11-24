# simscheduler.dll CRT SSE2 detection

Live verified in simscheduler.dll. `__get_sse2_info` at `0x20cc6fbc`
tests whether the EFLAGS ID bit can change before executing CPUID.
It tests the SSE2 feature bit from leaf 1. If set, it calls the
OS support probe at `FUN_20cc6f67`. The initializer at `0x20cc7021`
records the result at `0x20cc83f4`.

The OS probe attempts `MOVAPD XMM0,XMM1` inside an EH4 scope.
Its filter handles access violations and illegal instructions.
The handler returns zero for either exception, while the normal path
returns one. The comparison covers all 80 contiguous bytes through
the final return, including the embedded filter and handler. Ghidra's
stored function body counts 45 bytes.

| Address | Linked bytes | Status | Address fields |
| --- | ---: | --- | ---: |
| `0x20cc6f67` | 80 | Normalized | 3 |
| `0x20cc6fbc` | 96 | Normalized | 1 |

Both comparisons reuse the retained VC8 SP1 startup probe linked
against the pinned RTM CRT. The linker map identifies both routines
as part of `cpu_disp.obj`. The 28-byte SSE2 scope table at `0x20cc75a8`
matches after separately validating the two HIGHLOW fields for the
filter and handler. The original and linked fields resolve to the
same offsets in their respective probe bodies.

- [Ghidra listings and scope table](../../../artifacts/simscheduler.dll/sse2-detection-ghidra.json)
- [OS support probe comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-dll-startup-20261116-003/comparison-20cc6f67.json)
- [CPU feature comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-dll-startup-20261116-003/comparison-20cc6fbc.json)

The linked startup, retained Flight save and GPS comparisons were
replayed. The SSE2 instruction and exception paths were compared
statically. Runtime behavior remains unverified. Ghidra was inspected
read-only and reported no unsaved changes. See the
[CRT exit registration area](simscheduler-crt-onexit-area.md)
for the path at `0x20cc69c2` and `0x20cc6a66`.
