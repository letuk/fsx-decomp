# simscheduler.dll scheduler object accessors

Live verified in simscheduler.dll. The eight targets below occupy
scheduler object VTable `0x20cc11cc` slots `+0x0c` through `+0x28`.
The comparison checks the PE and live Ghidra table targets separately
from the body and stack cleanup of each function. Names and source
signatures are reconstructed.

| Entry | Slot | Observed operation | Bytes | Native COFF |
| --- | --- | --- | ---: | --- |
| `0x20cc19a0` | `+0x0c` | Write signed speed bound at object `+0x20` | 18 | Diagnostic |
| `0x20cc19c0` | `+0x10` | Read speed bound at `+0x20` | 15 | Exact |
| `0x20cc19e0` | `+0x14` | Write SimSpeed at `+0x1c` | 18 | Diagnostic |
| `0x20cc1a00` | `+0x18` | Read SimSpeed at `+0x1c` | 15 | Exact |
| `0x20cc1a20` | `+0x1c` | Return the signed minimum of the two speed fields | 24 | Exact |
| `0x20cc1ad0` | `+0x20` | Copy SimTime at `+0x04` to the output pointer | 20 | Exact |
| `0x20cc1af0` | `+0x24` | Copy the last scaled step at `+0x0c` to the output pointer | 20 | Exact |
| `0x20cc1b10` | `+0x28` | Return the last input step at `+0x14` through x87 | 15 | Exact |

The speed-bound name follows the signed minimum operation and remains
provisional. The scaled-step accessor leaves the output pointer in EAX
with the same instruction sequence as the SimTime accessor. The
reconstructed signature returns it. Caller use of that return value
for the scaled-step accessor remains unverified.

VC8 SP1 emits full exact native x86 COFF bodies for six targets using
the existing frame-preserving block. In both setters it loads the
value before the object and interchanges their EAX/ECX assignments.
The stack ABI, field offsets, hotpatch prologues and body lengths agree.
Stopped matching those two at the user's register-allocation stopping
point. Full linked image and runtime behavior remain unverified.

- [Original Ghidra listings and VTable](../../../artifacts/simscheduler.dll/scheduler-object-accessors-ghidra.json)
- [Worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-object-accessors-20261116-001/receipt.json)
- [Effective-speed match](../../../vm-share/modules/simscheduler/runs/simscheduler-object-accessors-20261116-001/comparison-20cc1a20.json)
- [SimSpeed setter comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-object-accessors-20261116-001/comparison-20cc19e0.json)

The previous twelve normalized native gates and thirteen exact context
gates pass. Retained GPS and Flight save comparisons pass. Ghidra was
read-only and reported no unsaved changes. The next open context VTable
operation is clearing entries and resetting context state at
`FUN_20cc2e20`.
