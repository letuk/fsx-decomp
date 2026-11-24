# simscheduler.dll internal time-update area

Live verified in simscheduler.dll. `FUN_20cc1b90` scales and
dispatches the step. `FUN_20cc30d0` advances a context using either
all eligible entries in each substep or a rotating subset of entries.
It uses mode bits at `+0x38`, an update guard at `+0x3c`,
cursor at `+0x40`, timestamps and substep state at `+0x44..+0x5c`,
and the entry array at `+0x6c`. Callback records hold flags at
`+0x00`, time at `+0x04` and callback object at `+0x0c`.
Callbacks receive the current substep as a double. The reconstructed
names and packed C++ layouts represent only the observed fields.

`FUN_20cc3050` applies the lower and upper step bounds at context
offsets `+0x28` and `+0x30`. It receives the context in ESI,
the output step address in EDI, the output count address in EBX
and the input double on the stack. It calls the float-to-integer
runtime helper at `0x20cc6ea0` and returns with `ret 8`.
The C++ reconstruction passes the context and writes its observed
output fields directly. The compiler's runtime conversion is outside
this matching scope. Numerical edge cases have no runtime verification.

| Address | Original | LTCG default frame | LTCG EBP frame |
| --- | ---: | ---: | ---: |
| `0x20cc1b90` | 284 | 254 | 263 |
| `0x20cc30d0` | 955 | 774 | 757 |
| `0x20cc3050` | 108 | 116 | 122 |

The default and EBP-frame builds use VC8 x86 SP1 and link the three
functions with the original API data and dispatch import identities.
The EBP variant reproduces the EAX context parameter and `ret 0x10`
for `FUN_20cc30d0`. The complete bodies remain diagnostic. The
context's stack alignment, x87 lifetime, loop scheduling and access
patterns differ. The linked split helper uses ESI with direct context
stores instead of the separate EBX and EDI output addresses.
The comparator checks the full PE byte spans, direct call targets
and both import identities. An initial link attempt failed from
duplicate DLL export declarations. The preserved successful linked
probes report LNK4210 for a CRT section in the no-entry fragment.
Neither fragment was executed.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/context-time-update-ghidra.json)
- [EBP time update](../../../vm-share/modules/simscheduler/runs/simscheduler-context-ltcg-20261116-003/comparison-20cc1b90.json)
- [EBP per-context update](../../../vm-share/modules/simscheduler/runs/simscheduler-context-ltcg-20261116-003/comparison-20cc30d0.json)
- [EBP substep calculation](../../../vm-share/modules/simscheduler/runs/simscheduler-context-ltcg-20261116-003/comparison-20cc3050.json)
- [Default-frame per-context control](../../../vm-share/modules/simscheduler/runs/simscheduler-context-ltcg-20261116-002/comparison-20cc30d0.json)

## Area boundary

The time update includes the pending-list drain in `FUN_20cc30d0`.
It clears the update guard, visits 16-byte pending records from
`+0x8c` and calls context slots `+0x38` or `+0x3c` according
to each record. It then clears the count at `+0x80`. The concrete
registration handlers, list storage management and source lifecycle
begin the next area. `FUN_20cc3490` is a next candidate for that
area because it writes the same pending array, including the count
and 16-byte records. The complete dynamic VTable targets are open.

The nine exported COFF matches, group 0x20 callback, linked Flight
save and GPS regression passed. Ghidra was read-only, save state
unverified. Full DLL linking and runtime verification are pending.
