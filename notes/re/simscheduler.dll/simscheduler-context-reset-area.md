# simscheduler.dll context reset

Live verified in simscheduler.dll. Context table slot `+0x40` at
`0x20cc1240` points to `FUN_20cc2e20`. The target takes the context
on the stack, returns 1 in EAX and exits with `ret 4`.
`ResetSchedulerContext` is a reconstructed source name.

It first clears the pending count at context `+0x80`. It then returns
every live record in forward order to `FUN_20cc3770`, reading the
live array at `+0x6c` and the live count at `+0x60`. It clears the
live and index counts at `+0x60/+0x70`, clears the substep and its
count at `+0x54/+0x5c`, and sets the cursor at `+0x40` to -1.
Array pointers, capacities, clock and mode are retained. The pool
return helper is covered separately in the
[entry pool analysis](simscheduler-entry-pool-area.md).

The original contiguous PE range is 86 bytes through the final return.
Ghidra counts 80 instruction bytes and excludes the skipped six-byte
alignment instruction at `0x20cc2e3a`. The three native VC8 SP1
approaches emitted 88 bytes with a for loop, 88 bytes with a guarded do
loop and 72 bytes with size optimization on the guarded loop. All
remain diagnostic. The retained for loop matches the workspace source
hash and keeps the original iteration and state updates. The original
passes entries in EBX and the COFF build uses a stack argument to
the rebuilt pool helper. The call target and zero addend are validated.
Register assignment and reset-store scheduling also differ.

- [Original Ghidra listing and slot](../../../artifacts/simscheduler.dll/context-reset-ghidra.json)
- [Retained native comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-reset-20261116-001/comparison-20cc2e20.json)
- [Worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-context-reset-20261116-001/receipt.json)

All 31 existing native match gates and the retained Flight save and GPS
comparisons pass. Full DLL linkage and runtime behavior remain
unverified. Ghidra was inspected read-only and reported no unsaved
changes. The next open area is the scheduler object's initialization
and teardown, starting with the known VTable store at `0x20cc1630`.
