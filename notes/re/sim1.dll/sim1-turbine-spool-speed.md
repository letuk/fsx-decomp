# Shaft turbine spool speed

`sim1.dll:0x20c2c6d0` updates two spool-speed fields before
[`0x20c2c850` calculates shaft torque](sim1-turbine-shaft-torque.md).
The shared turbine loop uses this helper for engine types 3 and 5.
The source names and the minimum engine and turbine layouts are reconstructions.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c2b827`–`0x20c2b83f` | The caller loads axial air-relative speed from history `+0x104` and the step interval onto the stack. EBX carries the turbine record and EDI carries the engine record. The original callee removes 16 bytes of stack arguments. | live verified |
| `0x20c2c6e5`–`0x20c2c755` | When engine `+0x08` is nonzero and the absolute value at engine `+0x6c` is at least 0.0001 or unordered, query the local schedule with turbine `+0x20`. The four points are `(0, 0)`, `(100, 62)`, `(150, 50)` and `(610, 101.5)`. Otherwise the scheduled target is zero. The running response rate is 0.5. The physical identification and units of the two schedule inputs remain provisional. | live verified |
| `0x20c2c769`–`0x20c2c78e` | For a nonrunning engine, cap axial air-relative speed at 130 and use `0.15 - speed * responseDrop / 130` as the response rate. An unordered speed also selects the cap. The binary64 coefficient at `0x20bc3cb8` is `0.13999999999999999`. Negative air-relative speed passes through the cap comparison. | live verified |
| `0x20c2c78e`–`0x20c2c7e4`, `0x20c2c040` | Engine `+0x2c` selects a target floor of 12 and a response rate of 0.3. The ignition routine checks the same flag before starting the engine, supporting the reconstructed name `starterEngaged`. The original takes the larger powered target and applies a zero windmilling gain only for ordered positive axial speed. Positive infinite speed yields an unordered windmilling result. | live verified |
| `0x20c2c7e6`–`0x20c2c825` | Calculate `(target - old) * responseRate`, clamp to `[-20, 20]`, select +20 for an unordered rate, multiply by the step and add to the previous corrected speed. Store the binary64 result in turbine `+0x10`. The subsequent shaft torque helper uses this field as the AIR `0x1508` curve input. | live verified |
| `0x20c2dc52`–`0x20c2dc73`, `0x20c2ba0f`–`0x20c2ba25`, `0x20c2c828`–`0x20c2c837` | The environment update normalizes absolute temperature at history `+0x254`. The inlet calculation multiplies it by `1 + 0.2 * Mach²` using the Mach value in turbine `+0x40` after applying its configuration limit. The result is stored at `0x20c7ba80`. The spool update multiplies its rounded corrected speed by the square root of that ratio and publishes turbine `+0x00`. | live verified |

Suggested Ghidra names are `param_1` → `axialSpeed` from the history load,
`param_2` → `step` from the caller and final rate multiplication,
`unaff_EBX` → `turbine` from the per-engine state stride, and
`unaff_EDI` → `engine` from the engine flags and drive input. Ghidra
readback remains at saved revision `1795634906543:62a95bf0:520`.

## Build and verification

[`TurbineEngineState::UpdateSpoolSpeed`](../../../src/sim1/turbine.cpp)
shares the turbine translation unit with the shaft torque function.
The temperature ratio enters as a source argument. This keeps the caller-owned
inlet state explicit while the original reads a shared module global.
The [reconstructed inlet calculation](sim1-turbine-inlet.md) publishes the
value used by the combined inlet, spool and torque fixture.

| Original range, inclusive | Native VC8 precise comparison |
| --- | --- |
| `0x20c2c6d0`–`0x20c2c6d4` | 5 exact bytes for the hotpatch entry and frame setup. |
| `0x20c2c72c`–`0x20c2c738` | 13 normalized bytes for the absolute drive comparison with the validated threshold. |
| `0x20c2c755`–`0x20c2c759` | 5 normalized bytes for the call to the shared schedule lookup. |
| `0x20c2c780`–`0x20c2c78d` | 14 normalized bytes for the nonrunning response calculation, with both literal targets checked. |
| `0x20c2c7aa`–`0x20c2c7b0` | 7 exact bytes for the positive-speed check before the windmilling calculation. |
| `0x20c2c7eb`–`0x20c2c7f7` | 13 normalized bytes for the upper rate comparison, including its literal target. |
| `0x20c2c82e`–`0x20c2c832` | 5 normalized bytes for the square-root thunk targeting `MSVCR80.dll!_CIsqrt`. |

The four-point schedule is exact across 68 bytes as materialized by the
original instructions and stored in the rebuilt COFF. The complete bodies
differ at 367 original and 299 rebuilt bytes. The reconstructed method takes
the temperature ratio explicitly, uses a static schedule and has different
register and branch allocation. The 278-byte fast-profile control folds
the response division into the gain and emits `FSQRT`. The precise profile
retains the original division and CRT call.

Twelve host x86 cases passed, including the drive threshold, signed airflow,
starter override, rate limits, temperature correction and the handoff to
shaft torque. The native propeller regression suite passed and the shaft
torque comparison preserved its previous scopes. Simulator runtime and
the complete original turbine-loop calling context remain open.

[Ghidra evidence](../../../artifacts/sim1.dll/turbine-spool-ghidra.json) ·
[VC8 precise comparison](../../../artifacts/sim1.dll/turbine-spool-vc8/turbine-spool-comparison.json) ·
[VC8 fast control](../../../artifacts/sim1.dll/turbine-spool-vc8-fast/turbine-spool-comparison.json) ·
[Host x86 results](../../../artifacts/sim1.dll/turbine-spool-host-check/results.txt) ·
[Propeller regression](../../../artifacts/sim1.dll/sim1-turbine-spool-propeller-regression-20261125-001.log)
