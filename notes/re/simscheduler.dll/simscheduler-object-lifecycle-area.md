# simscheduler.dll scheduler object initialization and teardown

Live verified in simscheduler.dll. The CRT initializer pointer at
`0x20cc111c` points to `0x20cc72b0`. That entry calls initialization
at `0x20cc1600` and registers `0x20cc7300` with `_atexit`
at `0x20cc6a66`. Ghidra's dry-run listings establish the three
code ranges and their boundaries through the final returns.

The initializer zeros time, both step values, the time-state flag,
saved time and the name index's count, capacity and array pointer.
It installs table `0x20cc11cc` in `DAT_20cc8370`, sets SimSpeed
to 256, the speed bound to 32768 and the index growth and flags to
5 and 4. Its return value is the scheduler object address.
The name index starts at object `+0x30` and the array pointer at
`+0x3c` establishes a minimum object extent of `0x40` bytes.

The exit callback passes the index at `0x20cc83a0` in ESI to
`FUN_20cc21b0`. That helper checks for an array, ownership flag 1
and nonzero capacity. It releases each non-null context through
table slot `+0x04`, reduces a positive count to zero, frees the
array through the API slot at `0x20cc10d8`, sets ownership flag 1
and zeros the pointer and capacity. The C++ model keeps the original
guard and count handling and uses an explicit stack argument.
Names and object layout are reconstructed.

| Original entry | Operation | Original / VC8 bytes | Native COFF |
| --- | --- | --- | --- |
| `0x20cc1600` | Initialize the scheduler object | 102 / 102 | Normalized |
| `0x20cc72b0` | Run initialization and register the exit callback | 17 / 17 | Normalized |
| `0x20cc7300` | Exit callback | 15 / 12 | Diagnostic |
| `0x20cc21b0` | Release the owned name index | 135 / 134 | Diagnostic |

The two normalized comparisons validate 18 address fields, including
the object's member offsets and the call and callback references.
The two cleanup diagnostics validate all three address fields. Run
`002` tested a zero-based cleanup loop with a separately retained
count and emitted a 130-byte helper. Run `001` retains the workspace
source and the 134-byte helper. The remaining differences cover the
original ESI argument, register allocation and branch scheduling.

- [Ghidra listings and CRT initializer table](../../../artifacts/simscheduler.dll/scheduler-object-lifecycle-ghidra.json)
- [Initializer comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-object-lifecycle-20261116-001/comparison-20cc1600.json)
- [CRT startup entry comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-object-lifecycle-20261116-001/comparison-20cc72b0.json)
- [Name index cleanup comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-object-lifecycle-20261116-001/comparison-20cc21b0.json)

The 31 preceding native gates and retained Flight save and GPS
comparisons pass. Linked CRT registration, DLL linkage and runtime
behavior remain unverified. Ghidra reported no unsaved changes after
the read-only inspection and dry-run disassembly. The next area is
pool startup and teardown, starting with the neighboring CRT entries
at `0x20cc72d0` and `0x20cc72e0`.
