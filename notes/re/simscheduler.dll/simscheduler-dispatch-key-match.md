# simscheduler.dll setter at 0x20cc14e0

Live verified in simscheduler.dll at `0x20cc14e0` (`FUN_20cc14e0`).
The function writes its one 32-bit stack argument to `DAT_20cc8368`
and returns with `ret 4`, consistent with x86 `__stdcall`.
`FUN_20cc2ce0` compares its second argument with that value and propagates
the equality result into dispatch flags. `SetSchedulerDispatchKey` and
`schedulerDispatchKey` are provisional reconstructed names.

VC8 x86 14.00.50727.762 emitted the original hotpatch and EBP frame in
the first C++ approach with `/hotpatch` and a local
`#pragma optimize("y", off)`. The two locked XP worker runs preserve
their source snapshots, commands, hashes, receipts and build output.
The first run was replayed with the extended comparator. The second
ran both comparisons through the controller.

The complete 17-byte x86 COFF function matches after resolving the
sole DIR32 relocation at offset 9. The comparator verifies that its
zero-addend external symbol maps to writable original storage at
`0x20cc8368`, as identified by the PE operand and HIGHLOW fixup at
`0x20cc14e9`. The getter at `0x20cc14d0` and the retained GPS COFF
regression also pass.

- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-set-key-20261115-002/comparison-20cc14e0.json)
- [Original Ghidra and PE evidence](../../../artifacts/simscheduler.dll/first-pass.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-set-key-20261115-002/worker-output/disassembly.txt)

Linked image and runtime verification are pending. Ghidra read-only,
save state unverified. The next candidate is `0x20cc1560`.
