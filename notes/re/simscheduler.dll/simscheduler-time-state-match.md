# simscheduler.dll time-state handler at 0x20cc1b30

Live verified in simscheduler.dll. `FUN_20cc1b30` receives the
object in EAX and a 32-bit flag in ESI. On transition to active it
reads the time through slot `+0x20` and copies both 32-bit words
to object offsets `+0x28` and `+0x2c`. On transition to inactive
it calls `FUN_20cc1a40` with the saved double on the stack and
object in EBX. It records the flag at object offset `+0x24`.
`UpdateSchedulerTimeState` is a reconstructed identifier.

Three VC8 x86 SP1 C++ approaches used the same /GL and /LTCG
profile with a second externally visible caller to keep the
object parameter live. This restored EAX for the object in
all three builds without assembly.

| Approach | Linked body | Difference |
| --- | ---: | --- |
| Double assignment | 81 bytes | Captures via x87 instead of two word moves |
| Two-word representation | 102 bytes | Restores word moves, expands conversion before restore |
| Byte-preserving memcpy | 87 bytes | Restores word moves and direct x87 load for restore |

The third build preserves both copy shapes and is the dashboard
diagnostic. It still uses EDI for the flag and ESI for the local
object, adds 64-byte stack alignment, and cleans the restore
call's stack argument in the caller. Original size is 81 bytes.
The comparator validates the complete linked body and decodes
the direct call to the rebuilt counterpart of `FUN_20cc1a40`.
The nine exported COFF bodies and GPS regression passed.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/time-state-ghidra.json)
- [87-byte comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-time-state-20261116-003/comparison-20cc1b30.json)
- [81-byte control](../../../vm-share/modules/simscheduler/runs/simscheduler-time-state-20261116-001/comparison-20cc1b30.json)
- [102-byte control](../../../vm-share/modules/simscheduler/runs/simscheduler-time-state-20261116-002/comparison-20cc1b30.json)

Matching stopped at the three approaches. The linked output is
a probe fragment. Runtime verification remains open. Ghidra was
read-only, save state unverified. Next candidate `FUN_20cc1a40`.
