# simscheduler.dll pool metrics and remaining VTable operations

Live verified in simscheduler.dll. All eleven targets in each typed pool
VTable at `0x20cc1280` and `0x20cc12ac` were checked against Ghidra,
the original PE and its HIGHLOW fields. The base deleting wrapper is
slot `+0x00` at `0x20cc1254`. Names and source signatures are reconstructed.

| Context | Timed entry | Operation | Native COFF |
| --- | --- | --- | --- |
| `0x20cc3960` | `0x20cc44d0` | Trim completely free blocks under the exclusive lock | Diagnostic |
| `0x20cc3a60` | `0x20cc45a0` | Sum block free counts, acquiring the shared lock when present | Diagnostic |
| `0x20cc3ac0` | `0x20cc4600` | Capacity in slots | Exact |
| `0x20cc3ad0` | shared | Current block count | Exact |
| `0x20cc4610` | shared | Peak block count | Exact |
| `0x20cc4640` | `0x20cc4620` | Slots per block, 16 or 128 | Exact |
| `0x20cc3ae0` | `0x20cc4630` | Allocated bytes in blocks | Exact |
| `0x20cc3af0` | `0x20cc4640` | Entry size, 144 or 16 bytes | Exact |
| `0x20cc3dd0` | `0x20cc46c0` | Typed deleting wrapper, 40 bytes each | Normalized |
| `0x20cc3d60` | shared base | Unlink registration and optionally free the node | Diagnostic |

The trim pass only removes a block whose free count equals its slot
capacity. It repairs the links, destroys an attached free list,
returns the block to the allocator and decrements the live block count.
The peak count stays intact. Available slots are summed using the pool's
mode 0 lock when one is present. Failed acquisition yields zero. The
absence of a lock still permits summing the list. Capacity and allocated
bytes are current block count times the known block capacity and
allocation size. `0x20cc4640` serves two slots because both context
slots per block and timed-entry size are 16.

Both typed deleting wrappers call their earlier teardown routines and
free storage when flags bit 0 is set. Each complete native body matches
with one checked release import and one checked direct call. The base
wrapper differs in initial register and store scheduling.

The native VC8 SP1 control using pointer-to-link traversal emitted
153/166 bytes for the two trims. The retained build `003` uses an
explicit predecessor and produces 176/179 bytes against contiguous
original ranges of 189/192 bytes. The ranges include three skipped
alignment bytes each. The free-count loops are 84 bytes each against
86 original. The base wrapper is 96 bytes in both images. The five
diagnostics retain all references. The lock and free-list argument
conventions and generated scheduling account for remaining differences.
The first worker request failed on a qualified member `sizeof`
unsupported by VC8. The successful builds use the block template's
compile-time capacity directly.

- [Original Ghidra listings and pool VTables](../../../artifacts/simscheduler.dll/pool-metrics-ghidra.json)
- [Retained worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-pool-metrics-20261116-003/receipt.json)
- [Context pool trim comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-pool-metrics-20261116-003/comparison-20cc3960.json)
- [Entry pool deleting wrapper match](../../../vm-share/modules/simscheduler/runs/simscheduler-pool-metrics-20261116-003/comparison-20cc46c0.json)

The preceding 38 native match gates and retained Flight save and GPS
comparisons pass. Ghidra reported no unsaved changes. DLL linkage and
runtime behavior remain unverified. The next area is the shared
free-list implementation starting at `FUN_20cc4d90` and
`FUN_20cc4eb0`.

The follow-up at `0x20cc3d60` compared the retained for loop, a
while loop that separates the search from the unlink, and a guarded
do loop. The while loop added a second null check on the found path
and emitted 100 bytes. The guarded do loop preserves the 96-byte
extent and extends the matching suffix from 70 to 76 bytes after
verifying four address fields. The remaining difference occupies
six bytes at offsets +0x0e through +0x13. The rebuild tests EDX
and moves the head to EAX after the test. The original moves it
before the test and tests EAX. The guarded do loop is the retained
Diagnostic. The comparison covers the original slot at
`0x20cc1254` and the complete PE and COFF bodies.

Live Ghidra suggestions, unapplied, are `pvVar3` to `head`,
`pvVar2` to `node`, `pvVar1` to `previous`, and `param_1` to
`flags`. The list snapshot, traversal, predecessor link and
deletion flag support these reconstructed names. Ghidra was
read-only and the current save state was not verified.

- [Live deleting wrapper listing and slot](../../../artifacts/simscheduler.dll/pool-base-delete-ghidra.json)
- [Deleting wrapper comparisons](../../../artifacts/simscheduler.dll/pool-base-delete-comparison.json)

Runs `simscheduler-pool-base-20261117-003` and `004` tested a
separate empty-list path and delayed traversal initialization.
They emit the same 96-byte wrapper with a 68-byte matching suffix.
The previous guarded do-loop source and 76-byte suffix remain
the best Diagnostic.
