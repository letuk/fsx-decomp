# simscheduler.dll callback registration wrapper at 0x20cc1460

Live verified in simscheduler.dll. `FUN_20cc1460` loads the
api.dll import from the IAT at `0x20cc10fc` once into ESI.
It registers `0x20cc13c0` for group `0x20` and then
`FUN_20cc1400` for group `0x10`. Both callback entry paths
have `ret 8`. `0x20cc13c0` is a listing entry without a
defined Ghidra function. The complete callback bodies remain
outside the rebuilt scope.

The original api.dll export at `0x200f63df` reads two stack
arguments and returns with `ret 8`. It indexes the callback
registry with the group value and forwards the callback pointer
to `0x200f6451`. The SimScheduler wrapper discards any result.
The C++ import declaration models only the ABI this wrapper uses.
`RegisterSchedulerCallbacks` and the import and callback names
are reconstructed.

The first VC8 x86 XP run matched the complete 29-byte native
COFF wrapper. The comparator verifies the three DIR32 offsets
5, 10 and 19 against the original HIGHLOW fixups, the PE import
identity and the two executable callback entries. All seven
earlier SimScheduler comparisons and the GPS regression pass.

- [Original Ghidra and import evidence](../../../artifacts/simscheduler.dll/callback-registration-ghidra.json)
- [API registry evidence](../api.dll/api-callback-registry.md)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-register-20261115-001/comparison-20cc1460.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-register-20261115-001/worker-output/disassembly.txt)

Matched scope is the wrapper COFF body. Linked image and runtime
verification are pending. Both Ghidra programs were read-only.
Save states were unverified in this pass. Next target `0x20cc1490`.
