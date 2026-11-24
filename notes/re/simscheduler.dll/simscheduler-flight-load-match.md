# simscheduler.dll Flight load handler at 0x20cc4790

Live verified in simscheduler.dll. The handler receives its payload
in ESI. It reads `SimTime` and `SimSpeed` under `SimScheduler`
with fallback sections `DateTimeSeason` and `Options`. When
both reads for a value fail, time defaults to zero and speed to
`0x100`. It passes time to `FUN_20cc1a40` and speed through
VTable slot `+0x14`, which resolves to `FUN_20cc19e0`.
The import at `0x20cc1108` resolves to `flight.dll:0x2049e750`.
This refines the first-pass decompilation with the checked register
argument and return shape. C++ names and layouts are reconstructed.

| VC8 x86 LTCG approach | Linked body |
| --- | ---: |
| Default frame and transfer pointer from the dispatcher probe | 312 bytes |
| Preserved EBP frame and real Flight import | 270 bytes |
| Flight import and byte-preserving propagation helper profile | 267 bytes |

The last approach restores the ESI payload, repeated imported call
through EDI, and direct call to a callee-cleanup propagation helper.
Its EBP-based locals omit the original eight-byte stack realignment.
The descriptor stores and fallback branches are scheduled differently.
The comparator verifies all five string references, four Flight transfer
calls, the original and linked propagation targets, and the SimSpeed
slot reference. The 308-byte original remains unmatched after three
approaches. The closest ABI shape is retained as the diagnostic.

- [Original Ghidra and PE evidence](../../../artifacts/simscheduler.dll/flight-load-ghidra.json)
- [267-byte comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-flight-load-20261116-002/comparison-20cc4790.json)
- [270-byte control](../../../vm-share/modules/simscheduler/runs/simscheduler-flight-load-20261116-001/comparison-20cc4790.json)
- [312-byte control](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-ltcg-20261116-002/comparison-20cc4790.json)

The linked Flight save match, nine exported COFF matches and GPS
regression passed. Runtime verification remains open. Ghidra
read-only, save state unverified. Next candidate `0x20cc13c0`.
