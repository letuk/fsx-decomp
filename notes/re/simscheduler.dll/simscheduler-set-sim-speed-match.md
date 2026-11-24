# simscheduler.dll SimSpeed setter wrapper at 0x20cc1580

Live verified in simscheduler.dll. `FUN_20cc1580` passes its 32-bit
stack argument and object address `0x20cc8370` to VTable slot
`+0x14`, then returns with `ret 4`. The original slot at
`0x20cc11e0` points to `FUN_20cc19e0`, which writes the second
argument to object offset `+0x1c` and returns with `ret 8`.
The getter and Flight save path identify the field as SimSpeed.
`SetSchedulerSimSpeed` is the reconstructed source name.

The first C++ approach emitted the full 29-byte wrapper using the
existing VC8 x86 XP profile with local frame-pointer preservation.
The comparator independently validates the two DIR32 operands at
offsets 10 and 19 against the original HIGHLOW fixups and writable
object at `0x20cc8370`. It checks the original table initialization,
slot fixup, and target body separately. All four earlier SimScheduler
comparisons and the retained GPS regression pass.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/set-sim-speed-ghidra.json)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-set-speed-20261115-001/comparison-20cc1580.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-set-speed-20261115-001/worker-output/disassembly.txt)

Matched scope is the wrapper COFF body. Linked image and runtime
verification are pending. Ghidra read-only, save state unverified.
Next target `0x20cc1530`.
