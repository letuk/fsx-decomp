# simscheduler.dll callback unregistration at 0x20cc1490

Live verified in simscheduler.dll. `FUN_20cc1490` removes
`FUN_20cc1400` from group `0x10` and `0x20cc13c0` from
group `0x20` through the IAT at `0x20cc10d0`. The original
API target at `0x200f658f` reads two stack arguments and returns
with `ret 8`. The wrapper discards any return value.

The wrapper then walks the list headed by `DAT_20cc83b8`.
It passes each node in ECX to VTable slot `+0x04`, then reads
the next pointer from node offset `+0x04`. The insertions in
`FUN_20cc3eb0` and `FUN_20cc46f0` install tables at `0x20cc1280`
and `0x20cc12ac`. Their slot targets `FUN_20cc38c0` and
`FUN_20cc4430` preserve their ECX object argument and return
with a plain RET. This verifies representative machine call shapes.
The full set of possible dynamic targets remains open.
The C++ node declaration is a minimum layout. Its virtual method
names and `UnregisterSchedulerCallbacks` are reconstructed.

The first VC8 x86 XP build matched the complete 55-byte COFF body.
The comparator validates all four DIR32 operands independently
against original HIGHLOW fixups, PE import identity, callback entry
bytes and writable list-head storage. The indirect call and list
loop match byte for byte. The eight prior SimScheduler comparisons
and the GPS regression pass.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/callback-unregistration-ghidra.json)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-unregister-20261115-001/comparison-20cc1490.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-unregister-20261115-001/worker-output/disassembly.txt)

The nine exported wrappers now have scoped COFF matches.
Linked image and runtime verification are pending. Ghidra was
read-only for both programs. Save states were unverified.
Next candidate `FUN_20cc1400`.
