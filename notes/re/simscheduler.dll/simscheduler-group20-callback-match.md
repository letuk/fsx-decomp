# simscheduler.dll group 0x20 callback at 0x20cc13c0

Live verified in simscheduler.dll. The stored listing begins at
`0x20cc13c0` and ends at `0x20cc13ec` with `ret 8`. Ghidra
has no function object for the range. The registration and
unregistration references independently identify the entry.

The callback exits when the predicate at `main.dll:0x20955a1b`
returns nonzero. Otherwise it gets a pointer from
`api.dll:0x2012df27`, checks it for null, and passes the double
at pointer offset `+0x64` to the scheduler object's VTable slot
`+0x2c`. The slot at `0x20cc11f8` resolves to `FUN_20cc1b90`.
That target reads the object and double from the stack and
returns with `ret 0xc`. The C++ names and input structure are
reconstructed. The source meaning and units of the double are open.

The first VC8 x86 C++ worker build matched the full 47-byte COFF
function after four target validations. Two DIR32 operands identify
the original imports at `0x20cc1110` and `0x20cc10cc`. Two
identify object `0x20cc8370`. The PE HIGHLOW fixups and original
table slot are checked separately. Both registration wrappers
retain their matches with the now-defined local callback symbol.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/group20-callback-ghidra.json)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-group20-callback-20261116-001/comparison-20cc13c0.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-group20-callback-20261116-001/worker-output/disassembly.txt)

## Area boundary

The export, callback and Flight persistence pass ends here. All nine
exported COFF bodies, the `0x20cc13c0` callback and the linked
`FUN_20cc48d0` Flight save body have verified scoped matches.
The C++ diagnostics for `FUN_20cc1400`, `FUN_20cc4790`,
`FUN_20cc1b30` and `FUN_20cc1a40` retain their remaining differences.
The GPS regression passed. Full-module linking and runtime verification
are pending. Ghidra read-only, save states unverified.
The next area begins at `FUN_20cc1b90`, the internal time-update path.
