# Turbine preparation, state and complete iteration

The reconstructed loop at `sim1.dll:0x20c2b6d0` now calls the engine
preparation, automatic start, common turbine controls, jet or shaft
update, thrust publication and final state updates in original order.
The source names, provider interfaces and minimum layouts are reconstructions.

## Preparation and start

| Original entry | Finding | Status |
| --- | --- | --- |
| `0x20c214b0` | A positive starter drive consults starter circuits 18–21 for the first four engines unless the configuration bypass is set. The circuit result writes engine `+0x2c`. The common electrical query is the existing implementation at `0x20bfee30`. | live verified |
| `0x20bf77c0`, `0x20bf8170` | The conventional turbine branch derives fuel demand from the preceding thrust and torque outputs. The shaft branch adds the power-dependent term using configured torque and rated horsepower, or the RPM and gear-ratio fallback for type 5. The common wrapper applies the fuel selector and clears availability and tank selection when the engine gate is off. The shared dispatch also has a separate piston callee. | live verified |
| `0x20bf79e0`, `0x20bf7fbc` | The fuel selector uses the original 21-way dispatch, priority within each internal tank bank, external tank priority, crossfeed mode and per-tank selection masks. This implementation is shared by the turbine fuel stage. | live verified |
| `0x20c2d410` | Automatic start sets the electrical fallback flag, advances the scheduled engine's lever and starts cranking. The original enables fuel after 20 seconds, reports a failed start after 25 seconds and moves to the next engine after five seconds of running. The completion command carries event `0x1048d`, argument 1 and reserved value 0 to the owning adapter. | live verified |
| `0x20c2a130`, `0x20c211c0` | The shared control advance saturates at its target. The autostart toggle counts running engines, applies the owner availability gate before transitions and messages, and publishes the original `0x9612`–`0x9618` message IDs through the shared message adapter. | live verified |

## Turbine and jet controls

| Original entry | Finding | Status |
| --- | --- | --- |
| `0x20c2bc90` | Selects the signed throttle schedule by engine type. Positive throttle can select and truncate the unsigned afterburner stage. Its final value uses the auxiliary level and the penalty at the engine effect record. | live verified |
| `0x20c2bdd0` | Blends the table results at turbine data `+0x4cc` and `+0x7a4` by inlet Mach using their stored reference Mach values. Publishes the governor target at `0x20c7ba68` and applies the shaft-engine idle floor. | live verified |
| `0x20c2beb0` | Updates turbine `+0x20` from governor target and the corrected spool selected by engine type. Uses the prior pressure indication as a gate, preserves the minimum schedule value and applies the stopped-jet limit. | live verified |
| `0x20c2c040`, `0x20c2bff0`, `0x20c205d0` | Combustion depends on the fuel schedule, ignition request, auxiliary level and turbine inhibit provider. The strict sustain thresholds are 20 for type 1 and 10 for types 3 and 5. Successful ignition sets the running flag, passes the mount position converted to meters to the shared effect adapter, then clears the ignition timer. | live verified |
| `0x20c2c410` | Selects the reverser target from signed command or the four-point Mach schedule and applies a rise rate of 8 or fall rate of 2. | live verified |
| `0x20c2c350`, AIR `0x1521` | Updates turbine `+0x58` using the optional four-byte afterburner gain and inline two-axis table. A running reverse command selects zero. Missing data selects one. | live verified |
| `0x20c2c120`, `0x20c2c230` | The jet high spool uses the fuel schedule and turbine data `+0xa7c`. It publishes speed, fraction and RPM using the inlet temperature ratio and rated high spool RPM. The low spool uses turbine data `+0x4c` with capped Mach and corrected high spool, limits the rate to ±33 and publishes corrected and temperature-scaled speed. | live verified |

## Indications and post-thrust state

| Original entry | Finding | Status |
| --- | --- | --- |
| `0x20c2c980` | Uses a fixed two-axis schedule to update turbine `+0x30` with inlet Mach, corrected high spool and ambient pressure. A low-Mach floor can select one. | live verified |
| `0x20c2caa0`, `0x20c2ce50` | The two gas-temperature schedules use the corrected spool selected by engine type or rounded ambient Rankine when stopped. The ambient floor applies before the inlet correction for turbine `+0x38` and after integration for engine `+0x74`. | live verified |
| `0x20c2cbe0` | The spool drive schedule updates engine `+0x6c` and includes the configured high-RPM bias for type 5. It reads separate provider slots for supply level, wear level and wear rate. The availability decrement rounds through binary32. | live verified |
| `0x20c2cd80`, `0x20c2cf80` | The two oil indication calculations update engine `+0x64` and `+0x84`. The temperature uses a spool schedule or rounded ambient. The pressure uses the starter/spool target, a provider inhibit and a configured floor. The pressure step is binary32. The source indication names remain reconstructed. | live verified |
| `0x20c2b6d0` | The loop calls all the above stages, the existing inlet, spool, torque and thrust routines and the final speed blend. The two inline shaft-speed writes at `0x20c2b846`–`0x20c2b860` publish scaled and corrected shaft speed. The last shared effect call stays at the boundary documented in [Turbine thrust and the engine-loop boundary](sim1-turbine-thrust-and-tail.md#end-of-the-turbine-loop). | live verified |

Suggested Ghidra renames are `unaff_EDI` → `turbine` in
`FUN_20c2c980` from the inlet Mach and pressure state offsets and
`unaff_EBX` → `engine` in `FUN_20c2ce50` from the running read and
temperature store. The implicit EAX value in `FUN_20c2c410` is
`history` by its configuration and Mach reads. Ghidra names and
prototypes are preserved. Saved revision `1795634906543:62a95bf0:520`
was read back clean.

## Verification and boundaries

The native VC8 RTM comparisons cover 24 bodies. The 75-byte shared
control-advance helper is an entire exact match. In the other 23 bodies,
922 scoped bytes are exact and 644 are normalized against individually
checked references. Six materialized interpolation schedules match all
540 data bytes. The comparison includes original and rebuilt listings
and reports the remaining whole-body differences.

| VC8 object | Bodies | Exact scope bytes excluding the complete match | Normalized scope bytes | Exact table bytes |
| --- | ---: | ---: | ---: | ---: |
| `turbine-controls` | 10 | 267 | 286 | 68 |
| `turbine-indications` | 6 | 146 | 56 | 472 |
| `turbine-start` | 4 | 187 | 147 | 0 |
| `turbine-fuel` | 3 | 273 | 64 | 0 |
| `turbine-loop` | 1 | 49 | 91 | 0 |

The host x86 fixture passed 32 assertions, including a jet and a
shaft-producing iteration through the normal reconstructed loop.
The earlier inlet, spool, shaft torque and thrust host fixtures passed,
along with the established native propeller and linked propulsion comparisons.
Fuel, starter, indications and the loop are separate translation
units. The shared visual effects and message renderers enter through
the owning adapter. Their calls are exercised by the fixture.
Concrete external provider bindings, a linked original-image match
and simulator runtime remain unverified.

[Controls](../../../artifacts/sim1.dll/turbine-state-vc8/controls/turbine-controls-comparison.json) ·
[Indications](../../../artifacts/sim1.dll/turbine-state-vc8/indications/turbine-indications-comparison.json) ·
[Starter](../../../artifacts/sim1.dll/turbine-state-vc8/start/turbine-start-comparison.json) ·
[Fuel](../../../artifacts/sim1.dll/turbine-state-vc8/fuel/turbine-fuel-comparison.json) ·
[Loop](../../../artifacts/sim1.dll/turbine-state-vc8/loop/turbine-loop-comparison.json) ·
[Host x86](../../../artifacts/sim1.dll/turbine-state-host-check/results.txt) ·
[Ghidra export](../../../artifacts/sim1.dll/turbine-state-ghidra.json)

[Host regression](../../../artifacts/sim1.dll/turbine-state-host-regression/results.txt) ·
[Native turbine regression](../../../artifacts/sim1.dll/sim1-turbineshafttorque-20261125-002.log) ·
[Native propeller regression](../../../artifacts/sim1.dll/sim1-turbine-20261125-002.log) ·
[Linked propulsion regression](../../../artifacts/sim1.dll/sim1-propulsionlinked-20261125-002.log)
