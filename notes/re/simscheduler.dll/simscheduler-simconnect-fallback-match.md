# simscheduler.dll SimConnectFallback wrapper at 0x20cc1500

Live verified in simscheduler.dll. `FUN_20cc1500` loads the 32-bit
value at `0x20cc8360`, calls `FUN_20cc1cc0` with that value and its
own stack argument, then stores the argument at `0x20cc8364`.
The wrapper uses `ret 4`. Both callee exits use `ret 8` and the
callee reads both stack arguments. It references the
`SimConnectFallback` string at `0x20cc11b8`.

`SetSimConnectFallback` and `UpdateSimConnectFallback` are
reconstructed source names. The first parameter is modeled as a
pointer to an opaque `SchedulerContext`. The exact context type
and the semantics of the setting beyond its observed use remain open.

The integer model for the second argument is superseded. The setter's
stored value is dereferenced by FUN_20cc1b90 as an observer object
pointer. The updated opaque pointer type keeps the 32-bit ABI and
the verified setter match. See [internal time update](simscheduler-internal-time-update.md).

The first VC8 x86 XP build matched the complete 32-byte native COFF
wrapper with local EBP-frame preservation. The comparator validates
the DIR32 operands at offsets 6 and 23 against the writable PE
targets and HIGHLOW fixups. It separately validates the REL32
operand at offset 17 by decoding the original direct call to
`0x20cc1cc0` and checking the rebuilt external symbol and zero addend.
All six earlier SimScheduler comparisons and the GPS regression pass.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/simconnect-fallback-ghidra.json)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-fallback-20261115-001/comparison-20cc1500.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-fallback-20261115-001/worker-output/disassembly.txt)

Matched scope is the wrapper COFF body. Linked image and runtime
verification are pending. Ghidra read-only, save state unverified.
Next candidate `0x20cc1460` for callback registration.

The callee is now reconstructed and its context transitions and
comparison scope are recorded in the
[fallback context area](simscheduler-fallback-context-area.md).
