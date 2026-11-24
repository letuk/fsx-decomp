# simscheduler.dll pool initialization and teardown

Live verified in simscheduler.dll. The CRT pointers at `0x20cc1120`
and `0x20cc1124` start the context and timed-entry pools. Each wrapper
publishes its static pool address to `0x20cc83b0` or `0x20cc83b4`.
Both accessors test bit 0 of a one-time guard and register an exit
callback with `_atexit` at `0x20cc6a66`. The original unwind entries
clear the corresponding guard bit on failure.

The pool objects at `0x20cc83d4` and `0x20cc83bc` have an observed
24-byte extent through the peak counter. Construction links the
object into the list at `0x20cc83b8`, installs its VTable, clears
the block list, creates a lock via `FUN_20cc6580`, and zeros the
two counters. A null lock triggers DebugBreak. The C++ registration
subobject unlinks on unwind if pool construction throws.

The destructors drain their blocks under the pool lock when acquisition
succeeds, destroy the lock through `FUN_20cc63e0`, set the base VTable
at `0x20cc1254`, and unlink their node. The drain first calls table
slot `+0x0c`, acquires in mode 1, detaches the block list, destroys
each free list and returns storage to the API release routine. It
zeros the block count and releases the lock. The peak count is retained.
The shared slot at `0x20cc3a30` calls capacity first and subtracts the
available-slot count. Names and minimum layouts are reconstructed.

| Context pool | Timed-entry pool | Operation | Original / VC8 bytes | Native COFF |
| --- | --- | --- | --- | --- |
| `0x20cc72d0` | `0x20cc72e0` | CRT startup entry | 11 / 11 | Normalized |
| `0x20cc3cf0` | `0x20cc4650` | Guarded pool accessor | 100 / 127 | Diagnostic |
| `0x20cc3eb0` | `0x20cc46f0` | Construct pool | 145 / 149 | Diagnostic |
| `0x20cc7320` | `0x20cc7330` | CRT exit callback | 11 / 11 | Normalized |
| `0x20cc3810` | `0x20cc4380` | Destroy pool | 171 / 184 | Diagnostic |
| `0x20cc38c0` | `0x20cc4430` | Drain blocks | 143 / 147 | Diagnostic |
| `0x20cc3580` | — | Unlink registration | 63 / 63 | Diagnostic |
| `0x20cc3a30` | — | Count used slots | 31 / 31 | Exact |

The four normalized matches validate two fields each. Both drain
comparisons include the six skipped alignment bytes before the loop.
In run `001` the constructors called an out-of-line template with
explicit exception cleanup. Run `002` embedded the placement
constructors and C++ registration subobject. Run `003` also embedded
normal unlink cleanup in the destructors and is the retained build.
Remaining differences cover EH code, register assignments, alignment
and the original internal lock/free-list argument conventions.
The emitted one-time accessors retain additional rethrow and guard
references. Low-level lock allocation/destruction and free-list APIs
are modeled as calls to rebuilt counterparts. Their implementation
comparison and linked CRT registration are outside this scope.

- [Original Ghidra listings, CRT pointers and raw PE code evidence](../../../artifacts/simscheduler.dll/pool-lifecycle-ghidra.json)
- [Best worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-pool-lifecycle-20261116-003/receipt.json)
- [Context pool startup match](../../../vm-share/modules/simscheduler/runs/simscheduler-pool-lifecycle-20261116-003/comparison-20cc72d0.json)
- [Context pool constructor comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-pool-lifecycle-20261116-003/comparison-20cc3eb0.json)
- [Timed-entry pool teardown comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-pool-lifecycle-20261116-003/comparison-20cc4380.json)

The previous 33 native match gates and retained Flight save and GPS
comparisons pass. Runtime behavior and exception-path equivalence
remain unverified. Ghidra reported no unsaved changes after inspection.
The next area is the pool metrics and remaining VTable operations at
`0x20cc3a60`, `0x20cc3ac0`, `0x20cc45a0` and `0x20cc4600`.
