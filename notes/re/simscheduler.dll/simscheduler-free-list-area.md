# simscheduler.dll shared pool free list

Live verified in simscheduler.dll. `FUN_20cc4d90` creates a `0x48`-byte
aligned control record. Both pool types use that record. It has tagged
heads at `+0x00` for spare nodes, `+0x08` for ready entries and
`+0x20` for the backing batch allocations. Optional semaphore counters
and handles are at `+0x10/+0x14` and `+0x18/+0x1c`. Batch size,
remaining allowance, allocated node count and byte accounting are at
`+0x28/+0x2c/+0x30/+0x34`. The signature is `0x4c665374` at `+0x38`.
The raw allocation and custom allocator are at `+0x3c/+0x40`.
An eight-byte node has a next link and an entry or allocation pointer.
Names and layouts are reconstructed minima.

`FUN_20cc5110` takes a spare node or refills a batch, stores the entry and
publishes the node on the ready list. `FUN_20cc52e0` takes a ready node and
returns its value while recycling the node onto the spare list. A pop
changes both the head pointer and sequence. A push changes the pointer.
With a corresponding semaphore, the original reserves or restores the
counter and signals waiters when publishing. Creation leaves both
semaphore handles clear. The refill at `0x20cc4fb0` reserves the quota,
tracks the raw batch allocation via a trailing node, publishes all
but its first node, and returns that first node to the caller.
`FUN_20cc4eb0` invalidates the signature, closes any handles, releases
the batches and then releases the control record's raw storage.

| Entry | Operation | Original / rebuilt bytes |
| --- | --- | --- |
| `0x20cc4a40` | Initialize processor mode | 88 / 69 |
| `0x20cc4aa0` | Allocate and account for raw storage | 144 / 114 |
| `0x20cc4b40` | Return process-heap storage | 83 / 65 |
| `0x20cc4ba0` | Allocate aligned storage | 147 / 134 |
| `0x20cc4c40` | Allocate and track a batch | 112 / 118 |
| `0x20cc4cc0` | Release tracked batches | 189 / 142 |
| `0x20cc4d90` | Create control record | 283 / 168 |
| `0x20cc4eb0` | Destroy control record | 238 / 145 |
| `0x20cc4fb0` | Refill spare list | 332 / 281 |
| `0x20cc5110` | Push entry | 455 / 330 |
| `0x20cc52e0` | Pop entry | 452 / 276 |

All eleven native VC8 SP1 comparisons are diagnostic. The comparison at
`0x20cc4cc0` includes ten alignment bytes skipped by its initial jump.
The retained build uses VC8's `_InterlockedCompareExchange64` for tagged
pops and Win32 pointer exchange for pushes. The XP disassembly verifies
that the intrinsic emits locked `cmpxchg8b`. The original also emits
unlocked atomic instructions for the uniprocessor paths selected by
`DAT_20cc8010`. The C++ uses interlocked operations in either mode.
Existing explicit C++ entry points replace register inputs such as EDI
for the list, EAX for the creation batch size and EAX for destruction.
The COFF comparison records all paired and unpaired references.

Run `001` failed to compile because the Win32 name for the 64-bit
primitive was missing from the XP SDK headers. Run `002` verified the
compiler intrinsic. Run `003` tracks batch bytes and uses 64-bit CAS
for both head operations. Run `004` uses the original push field width
and is the retained source snapshot. Linked compatibility, concurrency
and runtime behavior remain unverified.

- [Original Ghidra listings](../../../artifacts/simscheduler.dll/free-list-ghidra.json)
- [Retained worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-free-list-20261116-004/receipt.json)
- [Refill comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-free-list-20261116-004/comparison-20cc4fb0.json)
- [Push comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-free-list-20261116-004/comparison-20cc5110.json)
- [Pop comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-free-list-20261116-004/comparison-20cc52e0.json)

The 49 preceding native match gates pass, as do the retained Flight save
and GPS comparisons. Ghidra was read-only and reported no unsaved
changes. The next area is the shared pool lock at
`FUN_20cc54b0`, `FUN_20cc61c0`, `FUN_20cc63e0` and `FUN_20cc6580`.
