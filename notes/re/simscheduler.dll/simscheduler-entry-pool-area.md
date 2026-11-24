# simscheduler.dll timed-entry pool

Live verified in simscheduler.dll. `DAT_20cc83b4` points to the
pool. Its observed offsets are the block list at `+0x08`, lock
at `+0x0c`, block count at `+0x10` and peak at `+0x14`.
Each `0x80c`-byte block contains 128 entries of 16 bytes, followed
by the free count, free-list handle and next pointer at
`+0x800/+0x804/+0x808`. Names and types are reconstructed minima.

- `FUN_20cc35d0` locks the pool and tries existing blocks. A
  successful pop atomically decrements the count. If it must grow,
  it allocates and initializes a block and attempts lock mode 3
  before linking it. A failed lock transition releases the
  prepared block and retries in mode 1. The first pop from a
  newly linked block uses a regular decrement.
- `FUN_20cc3770` takes the entry in EBX. It searches by unsigned
  address range and atomically increments the count after a
  successful push into the owning block's free list.
- `FUN_20cc3e00` creates or drains the free list and pushes all
  128 entry addresses. Free-list creation failure calls DebugBreak.
  The acquisition routine does not explicitly release the block
  on its initialization-failure edge.

| Function | Original | VC8 C++ COFF |
| --- | ---: | ---: |
| `0x20cc35d0` | 400 | 413 |
| `0x20cc3770` | 141 | 131 |
| `0x20cc3e00` | 163 | 158 |

The initial C++ probe verifies the full original spans, HIGHLOW
fields and 22 reference operands. It checks the API allocation
and release imports and the three Kernel32 import identities.
Several original lock and free-list functions take register
arguments. The explicit C++ argument models require rebuilt
counterparts. All ten earlier native COFF matches, the linked
Flight save match and the GPS regression passed.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/entry-pool-ghidra.json)
- [Acquisition comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-entry-pool-20261116-001/comparison-20cc35d0.json)
- [Return comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-entry-pool-20261116-001/comparison-20cc3770.json)
- [Block initialization comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-entry-pool-20261116-001/comparison-20cc3e00.json)

The pool pass stops at the free-list and lock implementations.
The [context storage pass](simscheduler-context-factory-storage-area.md)
shares the pool implementation template and retains refreshed
396-byte acquisition and 133-byte return diagnostics.
Next registration area is entry-container insertion at
`FUN_20cc3bd0`, `FUN_20cc3b00` and `FUN_20cc3c20`. Runtime
verification is pending. Ghidra read-only, save state unverified.
