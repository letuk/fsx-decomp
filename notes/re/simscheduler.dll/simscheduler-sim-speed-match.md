# simscheduler.dll SimSpeed wrapper at 0x20cc15b0

Live verified in simscheduler.dll. `FUN_20cc15b0` calls slot `+0x18`
of the VTable installed by the initializer at `0x20cc1630` in the
object at `0x20cc8370`. The original slot at `0x20cc11e4` contains
`0x20cc1a00` and has a PE HIGHLOW fixup.

The listing of `FUN_20cc1a00` independently verifies the stack
object argument, `ret 4`, and a 32-bit return value in EAX from
object offset `+0x1c`. The Flight save handler at `0x20cc48d0`
stores the result of the same slot under `SimSpeed`. The wrapper
preserves EAX, superseding the first-pass void interpretation.
`GetSchedulerSimSpeed` is the reconstructed source name.

The existing VC8 x86 profile emitted the complete 16-byte wrapper
on the first C++ approach. The comparison validates both DIR32
operands independently at offsets 1 and 9 against the original
HIGHLOW fixups and writable object address `0x20cc8370`. The three
earlier SimScheduler COFF comparisons and retained GPS regression pass.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/sim-speed-ghidra.json)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-speed-20261115-001/comparison-20cc15b0.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-speed-20261115-001/worker-output/disassembly.txt)

The matched scope is the wrapper COFF body. Linked image and runtime
verification are pending. Ghidra read-only, save state unverified.
The adjacent setter wrapper at `0x20cc1580` is the next candidate.
