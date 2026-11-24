# Sim1 propulsion force collector

The callback at `sim1.dll` `0x20c010f0` is reconstructed as
`FlightDynamics::CollectPropulsionForces`. The selected VC8 RTM precise
LTCG build reproduces the final 83 bytes exactly. The complete bodies
are 674 original bytes and 626 rebuilt bytes. The projection helper
at `0x20c01010` retains its complete 215-byte normalized match in this
caller context.
[Retained comparison](../../../artifacts/sim1.dll/propulsion-collector-comparison.json).

## Original behavior

The callback is registered by `0x20c046c0` with the flight instance
at `+0x244` and the callable address at `+0x248`. At the function
entry, ECX carries the instance. The incoming stack contains the step
and the two output pointers. The step is unused. The original
function returns with `ret 0x10`.

| Original address | Operation | Status |
| --- | --- | --- |
| `0x20c010f0`–`0x20c01135` | Gets the runtime block from `FlightDynamics+0x270` and clears the two vectors at runtime `+0x6c` and `+0xa4`. | live verified |
| `0x20c0115e`–`0x20c011d1` | Steps through records of stride `0xec` at runtime `+0x43c`. Projects each nonzero thrust using the engine mount position minus the center of gravity. | live verified |
| `0x20c01205`–`0x20c012a9` | Steps through records of stride `0x54` at runtime `+0x438`. Uses the configured engine map and adds the three-double offset pointed to by record `+0x4c` when present. | live verified |
| `0x20c012b5`–`0x20c012cc` | Clears moment component 0 when either configuration `+0x1444` or the entry at `(*(FlightDynamics+0x2d8))+4` is nonzero. | live verified |
| `0x20c012d5`–`0x20c012e9` | Writes the ratio of propulsion force component 2 to mass times gravity at runtime `+0x14`. The original instructions use a float mass, a double gravity value, and a double force operand. | live verified |
| `0x20c012ce`, `0x20c012ec`–`0x20c0133e` | When runtime `+0x210` equals 2, divides the instance double at `+0x1f4` by 3.0 and clamps the gain to `[0, 1]`. Unordered input also takes the zero-gain path. The gain scales moment component 1. | live verified |
| `0x20c0133f`–`0x20c01391` | Copies all six components to the callback outputs and returns. The full sequence is exact in the rebuilt image. | live verified |

The original listing supersedes the decompiler rendering for the
propeller arm offset and the precision of the force-to-weight ratio.
The layout additions are reconstructed minima. The names of the
suppression flags, blend input and mode describe their use in this
callback. Their broader meanings remain open.

Provisional Ghidra suggestions for `FUN_20c010f0` are `param_3` →
`forceOutput` and `param_4` → `momentOutput` from the two final
copies. `piVar2` → `state` follows the shared runtime pointer at
`FlightDynamics+0x270`. Ghidra names and types were preserved.

## Linked comparison

The selected build uses VC8 RTM `14.00.50727.42` with precise floating
point and LTCG in a no-entry PE32 DLL. The comparator verifies the
original PE hash, both listings, the two projection call targets, the
gravity storage references and the double literal 3.0 at
`0x20bb1760`. The projection call targets are paired once the
reconstructed callee's complete match is verified. The paired
addresses occur at different instruction offsets in the two callback
bodies. Stack slots, register allocation, loop code and the moment
gain block still differ at whole-body scope.

The first successful build covers 624 linked bytes. The selected
626-byte build uses explicit vector initialization and the engine
count entry condition. Its 83-byte exact scope begins at
`0x20c0133f` in the original and includes both output copies
and the epilogue. Runtime verification and a native COFF comparison
for this callback remain open.

The dashboard keeps the full-body difference and exact subrange
alongside the reconstructed source. Ghidra tags `Reconstructed`
and `Linked differences` were saved and read back for the callback
in revision `1795533674228:962445d7:149` with
`unsaved_changes=false`.

- [C++ collector](../../../src/sim1/engine_forces.cpp#L98)
- [Live Ghidra listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c010f0.asm)
- [Ghidra and ABI evidence](../../../artifacts/sim1.dll/propulsion-collector-ghidra.json)
- [Selected run with source snapshot, build command and receipt](../../../vm-share/modules/sim1/runs/sim1-propulsion-20261124-004)
- [First successful linked comparison](../../../vm-share/modules/sim1/runs/sim1-propulsion-20261124-002/propulsion-comparison.json)
- [Standalone thrust projection regression](../../../vm-share/modules/sim1/runs/sim1-propulsion-regression-thrust-20261124-002/thrust-comparison.json)
- [Force collection regression](../../../vm-share/modules/sim1/runs/sim1-propulsion-regression-forces-20261124/forces-comparison.json)
- [Linked position regression](../../../vm-share/modules/sim1/runs/sim1-propulsion-regression-position-20261124/position-comparison.json)
- [Native position regression](../../../vm-share/modules/sim1/runs/sim1-propulsion-regression-position-coff-20261124/position-comparison.json)
- [State integration regression](../../../vm-share/modules/sim1/runs/sim1-propulsion-regression-integration-20261124/state-integration-comparison.json)
