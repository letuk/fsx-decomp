# simscheduler.dll SimTime wrapper at 0x20cc1530

Live verified in simscheduler.dll. `FUN_20cc1530` calls slot `+0x20`
through the object at `0x20cc8370`. The original slot at
`0x20cc11ec` points to `FUN_20cc1ad0` and has a HIGHLOW fixup.
The target takes an object pointer and an eight-byte output buffer
on the stack, copies the double from object offset `+0x04`, leaves
the buffer address in EAX and returns with `ret 8`. The wrapper
loads the double through EAX into x87 ST(0). The Flight save handler
at `0x20cc48d0` uses this slot for `SimTime`.

`GetSchedulerSimTime` is a reconstructed source name. The explicit
output-pointer signature reproduces the verified ABI. Whether the
original source expressed the argument as hidden return storage
remains open.

The first VC8 x86 build matched the complete 34-byte COFF wrapper
with local EBP-frame preservation. The comparator verifies both
DIR32 operands at offsets 7 and 22 independently against the original
HIGHLOW fixups and writable object at `0x20cc8370`. The five earlier
SimScheduler comparisons and retained GPS regression pass.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/sim-time-ghidra.json)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-time-20261115-001/comparison-20cc1530.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-time-20261115-001/worker-output/disassembly.txt)

Linked image and runtime verification are pending. Ghidra read-only,
save state unverified. Next target `0x20cc1500`, whose callee uses
`SimConnectFallback`.
