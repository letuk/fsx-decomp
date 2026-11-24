# simscheduler.dll SimConnectFallback context

Live verified in simscheduler.dll. `FUN_20cc1cc0` takes a context
pointer and an observer pointer on the stack and returns with `ret 8`.
It looks up the name at `0x20cc11b8` in the registry owned by
`DAT_20cc8370` and retains its own reference across the transition.
An observer without a matching context triggers `FUN_20cc1670` and
calls slots `+0x24/+0x2c` with value 1. A null observer with an existing
context triggers `FUN_20cc18c0`, which consumes the reference added for
that call. The routine retains one reference until its own exit.

The previous context at `0x20cc8360` is read before the registry calls.
The previous observer at `0x20cc8364` is read after them if there is a
fallback context. On a change, the fallback removes the old observer if
there was no previous active context. It registers the requested
observer with flags 0 if there is no requested active context. When
the active context changes to a non-null pointer, it copies the step
bounds at `+0x28/+0x30` and forwards bits 1 and 2 of the mode word at
`+0x38` as 0 or -1. The callers perform the global pointer stores.

Table `0x20cc1200` resolves the observed slots to `0x20cc2aa0`
(step bounds), `0x20cc2bc0` (primary mode), `0x20cc2c40` (update
enable), `0x20cc2c80` (second mode bit), `0x20cc2cb0` (third mode
bit), `0x20cc2ce0` (entry registration) and `0x20cc2e80` (entry
removal). Target stack cleanup was checked separately. Names and
layouts are reconstructed. The C++ uses the existing registry view
for the scheduler object's storage and preserves the original
creation path's assumption of a non-null result before its virtual calls.

| VC8 SP1 COFF run suffix | Rebuilt bytes | Approach |
| --- | ---: | --- |
| `001` | 512 | Scoped C++ reference ownership with default alignment |
| `002` | 526 | Same source with a 64-byte-aligned lookup local |
| `003` | 506 | Aligned local, removal branch first and separate null exit |

All three remain diagnostic against the full 542-byte original body.
Run `002` was closest by size among these three attempts. Its
source snapshot was superseded by the unwind reconstruction below.
Its twelve address fields map to original symbols and
targets with zero COFF addends. Differences include the aligned stack
frame, the use of EBP instead of ESP in the cookie calculation, and
branch and ownership-cleanup layout. The comparison covers the EH
reference. Unwind equivalence, linked image and runtime behavior
remain unverified.

- [Original Ghidra listing and slot evidence](../../../artifacts/simscheduler.dll/fallback-context-ghidra.json)
- [Best COFF comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-fallback-context-20261116-002/comparison-20cc1cc0.json)
- [Worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-fallback-context-20261116-002/receipt.json)

All twelve native COFF regression gates pass, including the wrapper at
`0x20cc1500` with a local callee definition. Retained Flight save and
GPS registration comparisons pass. Ghidra was read-only and reported
no unsaved changes. The next area is the context accessors and mode
setters at `FUN_20cc2a80` and `FUN_20cc2aa0`.

## Unwind ownership

Live verified in simscheduler.dll at `0x20cc1cc0`. The persistent
fallback reference is state 0 (`0x20cc7220`). The lookup result is
state 1 (`0x20cc7228`) and the factory result is state 2
(`0x20cc7230`). Both temporary states unwind to state 0.
The original enters each temporary state after the producing call
returns. The older reconstruction armed both guards before their
calls, exposing an output that a failing callee could already have
released. The current C++ takes ownership from the returned slot
after successful return and keeps the same normal cleanup order.

In `simscheduler-fallback-unwind-20261116-001` the native VC8 build
emits the same transitions and arms both temporary references after
the corresponding calls. The verified shared guard destructor
releases through VTable slot +0x04. The three original eight-byte
funclets compare against 8, 11 and 11 rebuilt bytes. All remain
diagnostic due to frame layout, register ABI and target placement.
The parent comparison is diagnostic at 542 original versus 524
rebuilt bytes. The wrapper at `0x20cc1500` retains its normalized
match. Ghidra was inspected read-only and reported no unsaved
changes. Runtime exception execution remains unverified.

- [Unwind comparison](../../../artifacts/simscheduler.dll/eh-funclets-comparison.json)
- [Updated parent comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-fallback-unwind-20261116-001/comparison-20cc1cc0.json)
