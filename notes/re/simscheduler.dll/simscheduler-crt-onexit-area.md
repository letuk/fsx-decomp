# simscheduler.dll CRT exit-handler registration

Live verified in simscheduler.dll. `__onexit` at `0x20cc69c2` decodes
the local exit-table pointer. It forwards to the CRT import if that
pointer is the `-1` sentinel. Otherwise it acquires CRT lock 8,
decodes both bounds, registers the callback and re-encodes the updated
bounds. `FUN_20cc6a58` releases lock 8 on the normal and EH4 unwind
paths. `_atexit` at `0x20cc6a66` returns zero for a non-null registration
result and -1 on failure. Callback execution is part of the previously
compared detach path at `0x20cc66c9`.

| Address | Original / retained linked bytes | Status |
| --- | --- | --- |
| `0x20cc69c2` | 150 / 150 | Normalized |
| `0x20cc6a58` | 9 / 9 | Normalized |
| `0x20cc6a66` | 18 / 18 | Normalized |

These bodies reuse the VC8 SP1 link probe with the pinned RTM CRT.
The installed SP1 CRT emits 156 bytes for `__onexit`. The 28-byte
EH4 table at `0x20cc7368` was checked separately, including the
HIGHLOW pointer to the lock-release helper in each image. Direct
import thunks were checked against their IAT identities. The linked
startup comparison passes with the expanded scope. No runtime
execution or full DLL linkage was verified. Ghidra reports no
unsaved changes.

- [Ghidra listings and scope table](../../../artifacts/simscheduler.dll/crt-onexit-ghidra.json)
- [Exit registration comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-dll-startup-20261116-003/comparison-20cc69c2.json)

`FUN_20cc6a89` is the TypeInfo cleanup callback registered by
`0x20cc6677`. Live verified in simscheduler.dll. It passes the root
node at `0x20cc8024` to the CRT thunk at `0x20cc6e54`.
All 12 linked bytes match after validating both address fields.
[Original listing](../../../artifacts/simscheduler.dll/crt-type-info-cleanup-ghidra.json).
[Comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-dll-startup-20261116-003/comparison-20cc6a89.json).

Live verified in simscheduler.dll. The RTC initialization callback at
`0x20cc6a9a` and termination callback at `0x20cc6ac3` both match
their complete 36-byte linked bodies with two checked address fields
each. Their loops start at the first slot after the leading sentinel
and stop at the trailing sentinel. Both bounds are equal in the original
and probe, so neither loop invokes a callback. The four original sentinel
words at `0x20cc139c` are zero. The existing SP1 and RTM probe comparisons
were replayed. Runtime behavior remains unverified. Ghidra was read-only
and reported no unsaved changes.
[Listings and sentinel words](../../../artifacts/simscheduler.dll/crt-rtc-ghidra.json).

The [CRT PE section checks](simscheduler-crt-pe-sections-area.md)
cover the next three routines.
