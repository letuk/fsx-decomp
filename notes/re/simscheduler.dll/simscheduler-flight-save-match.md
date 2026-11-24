# simscheduler.dll Flight save handler at 0x20cc48d0

Live verified in simscheduler.dll. `FUN_20cc48d0` reads the time
via VTable slot `+0x20` (`FUN_20cc1ad0`) and the speed via
slot `+0x18` (`FUN_20cc1a00`). It passes the two descriptors,
their terminator, and its payload through the import at `0x20cc1104`.
The original import resolves to `flight.dll:0x2049e860`. The wrapper
uses `ret 4`. The descriptor's source names are reconstructed.

The VC8 SP1 LTCG default-frame control produced 184 bytes. Local
EBP-frame preservation brought the body to 164 bytes. That control
used an exported transfer pointer, leaving its import identity open.
The third approach built a minimal import library from the original
PE identity and emitted the correct Flight import with the same
164-byte instruction layout.

The comparison independently validates all eight HIGHLOW offsets in
both PEs. Four operands refer to the scheduler object, three refer
to the strings SimScheduler, SimSpeed and SimTime, and the eighth
refers to the Flight IAT. The resolved full bodies are identical.
The matched scope is the linked Flight save handler. All nine
SimScheduler export COFF gates and the GPS regression passed.

- [Original listing and import evidence](../../../artifacts/simscheduler.dll/flight-save-ghidra.json)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-flight-save-20261116-002/comparison-20cc48d0.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-flight-save-20261116-002/worker-output/disassembly.txt)

The linked image is a probe fragment. Full-module linking and
runtime verification are pending. Ghidra read-only, save state
unverified. Next candidate `FUN_20cc4790`.
