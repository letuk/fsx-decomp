# simscheduler.dll wrapper at 0x20cc1560

Live verified in simscheduler.dll. `FUN_20cc1560` loads the table pointer
from `DAT_20cc8370`, pushes that object's address and calls slot `+0x1c`.
The initializer instruction at `0x20cc1630` writes `0x20cc11cc` to
`0x20cc8370`. The PE slot at `0x20cc11e8` points to `FUN_20cc1a20`.
The original initializer operands and slot each have a HIGHLOW fixup.

The target listing independently establishes a stack object argument,
callee cleanup via `ret 4`, and the signed minimum of the 32-bit fields
at object offsets `+0x1c` and `+0x20` returned in EAX. The wrapper
preserves EAX, refining the first-pass decompiler's void signature.
`GetSchedulerFieldMinimum` is a reconstructed source name. The C++
object/table declarations are minimum layouts. The meaning of the
field at +0x20 and the other slot signatures remain open. The field
at +0x1c is the SimSpeed value used by the Flight save handler.
See [SimSpeed wrapper](simscheduler-sim-speed-match.md).

The first C++ approach through the VC8 x86 XP worker produced the
complete 16-byte wrapper. The comparator verifies the DIR32 operands
at offsets 1 and 9 independently against the original HIGHLOW fixups
and writable object address `0x20cc8370`. The initial replay exposed
a 23-byte verification window for the 24-byte original target. After
correcting that window, the first object passed without rebuilding.
The second run completed all three comparisons through the controller.
The retained GPS COFF regression also passed.

- [Original Ghidra export](../../../artifacts/simscheduler.dll/field-min-ghidra.json)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-field-min-20261115-002/comparison-20cc1560.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-field-min-20261115-002/worker-output/disassembly.txt)

Only the wrapper's rebuilt COFF body is matched. Linking and runtime
verification are pending. Ghidra read-only, save state unverified.
Next target `0x20cc15b0`.
