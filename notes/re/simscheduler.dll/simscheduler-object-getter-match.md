# simscheduler.dll object getter match

Live verified in simscheduler.dll at `0x20cc14d0` (`FUN_20cc14d0`).
The six-byte body returns the address of scheduler object storage at
`0x20cc8370`. `GetSchedulerObject` is the reconstructed source name.
The source uses a forward declaration because the object layout is open.

The first C++ approach used VC8 x86 14.00.50727.762 with
`/O2 /Oy /hotpatch /GL- /Gy` through the locked XP worker queue.
Both worker receipts returned exit code 0. The first controller run
reached comparison before the comparator was available and passed when
replayed. The second run completed the full controller workflow with
the same source and flags. The request and worker output retain the command,
source snapshot, hashes, build log and native COFF headers.

The original PE body is `B8 70 83 CC 20 C3`. The rebuilt COMDAT is
`B8 00 00 00 00 C3`. The comparator checks its sole DIR32 relocation
at offset 1 against the undefined external `_schedulerObject`, verifies
the zero addend, and maps it to the original writable VA `0x20cc8370`
from the original instruction and PE HIGHLOW fixup. Resolving that
operand makes the entire six-byte body identical.

- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-get-object-20261115-002/comparison.json)
- [Original Ghidra and PE evidence](../../../artifacts/simscheduler.dll/first-pass.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-get-object-20261115-002/worker-output/disassembly.txt)

The retained GPS COFF regression passed using
`gps-legacy-lifecycle-20261115-003`. No earlier simscheduler.dll matches
exist. Linked image and runtime verification are pending. Ghidra read-only,
save state unverified. The next candidate is `0x20cc14e0`.
