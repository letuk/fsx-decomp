# simscheduler.dll context factory and storage

Live verified in simscheduler.dll. `FUN_20cc1670` rejects names
whose API-reported length is at least 32, copies the name to a fixed
key, and performs a virtual registry lookup through slot `+0x04`.
It returns a found context through the output slot. On a miss it
obtains storage from `FUN_20cc1f10`, constructs via `FUN_20cc27a0`,
and registers the name and object through `FUN_20cc22e0`. The
reference-count operations use context table slots `+0x00/+0x04`.
The length import resolves to `api.dll:0x20137fb0` and the copy
import to `api.dll:0x20137e17`.

The pool pointer is `DAT_20cc83b0`. The context pool has the same
observed shape as the timed-entry pool, with 16 raw context slots
of `0x90` bytes per block. The free count, free-list handle, and
next pointer are at `+0x900/+0x904/+0x908`. The C++ template
shares the checked pool operations with the existing entry pool.
`FUN_20cc2240` populates the free list. `FUN_20cc20b0`
receives the context in EBX and returns its storage to the owning
free list. The C++ return helper takes an explicit argument instead.

Table `0x20cc1200` resolves its first two slots to `FUN_20cc2720`
and `FUN_20cc2740`. They update the reference count at context
`+0x04` with InterlockedIncrement and InterlockedDecrement.
On zero, `FUN_20cc2740` invokes the deleting wrapper through
slot `+0x50` with flag 1. Both bodies match in native COFF after
independently validating their single DIR32 import references.

| Function | Original | VC8 C++ COFF | Status |
| --- | ---: | ---: | --- |
| `0x20cc1670` | 341 | 361 | Diagnostic |
| `0x20cc1f10` | 400 | 396 | Diagnostic |
| `0x20cc20b0` | 141 | 133 | Diagnostic |
| `0x20cc2240` | 147 | 142 | Diagnostic |
| `0x20cc2720` | 22 | 22 | Normalized |
| `0x20cc2740` | 49 | 49 | Normalized |

The factory comparison validates the API imports and internal
targets. The C++ construction and cleanup produce additional
cookie/rethrow references inside the rebuilt range. Exceptional
path equivalence and runtime behavior are unverified. The lock,
free-list helpers and registry insertion use explicit source
arguments where the original uses internal register ABIs.
Names and layouts are reconstructed minima.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/context-factory-storage-ghidra.json)
- [Factory comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-storage-20261116-002/comparison-20cc1670.json)
- [Pool acquisition comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-storage-20261116-002/comparison-20cc1f10.json)
- [Reference increment match](../../../vm-share/modules/simscheduler/runs/simscheduler-context-storage-20261116-002/comparison-20cc2720.json)
- [Reference release match](../../../vm-share/modules/simscheduler/runs/simscheduler-context-storage-20261116-002/comparison-20cc2740.json)

The existing ten native COFF matches, linked Flight save and GPS
regression passed. Ghidra read-only, save state unverified.
The next area is the name registry, starting at `FUN_20cc17d0`
and `FUN_20cc22e0`.
