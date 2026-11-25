# Turbine inlet conditions

`sim1.dll:0x20c2b9d0` runs for each turbine in the common engine loop.
It stores inlet Mach in that turbine's record and publishes three shared
ratios for the remaining calculations in the current engine iteration.
The names and minimum source layouts are reconstructions.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c2b780`–`0x20c2b784` | The caller supplies the turbine record in ECX and the current integration history in ESI. The function returns without popping any arguments. | live verified |
| `0x20c2683a`–`0x20c2685f`, `0x20c2b9d0`–`0x20c2ba0c` | AIR record `0x1523` loads the dword at turbine performance data `+0x40`. The loader clears the field when the record is absent. A nonzero value selects a cap of 0.5, otherwise the cap is 1.0. History Mach at `+0x15c` is retained if ordered and strictly below the cap. The cap is used for unordered input. The result is stored at turbine state `+0x40`. The descriptive flag name `useHalfMachLimit` reflects its observed use. | live verified |
| `0x20c2dc34`–`0x20c2dc73` | The atmospheric update converts pressure from environment `+0x30` and temperature from `+0x40` and rounds each converted value to binary32 before dividing by its reference. The resulting binary64 ratios are stored at history `+0x25c` and `+0x254`, respectively. | live verified |
| `0x20c2ba0f`–`0x20c2ba25` | The original reloads stored inlet Mach and calculates the temperature factor as `1 + (Mach * 0.2) * Mach`. It multiplies the factor by the ambient temperature ratio at history `+0x254` and writes the binary64 inlet temperature ratio to `0x20c7ba80`. | live verified |
| `0x20c2ba2b`–`0x20c2ba3c` | The original preserves the temperature factor on the x87 stack, raises it to 3.5 with `MSVCR80.dll!_CIpow`, multiplies by ambient pressure ratio at history `+0x25c`, and writes inlet pressure ratio to `0x20c7ba78`. | live verified |
| `0x20c2ba42`–`0x20c2ba53` | The original reloads the binary64 inlet temperature ratio and writes its square root from `MSVCR80.dll!_CIsqrt` to `0x20c7ba70`. All three shared doubles are initially zero. | live verified |
| `0x20c2b855`, `0x20c2c828`, `0x20c2c31c`–`0x20c2c322` | The temperature ratio feeds corrected shaft speed and spool-speed publication. The downstream calculation at `0x20c2c2e0` multiplies its table-derived value by the stored pressure ratio and divides by the stored temperature root. The earlier open label for that output is superseded by its verified use as [ram drag](sim1-turbine-thrust-and-tail.md). | live verified |

Suggested Ghidra names are `param_1` → `turbine` from the per-engine
store and `unaff_ESI` → `history` from the configuration and atmospheric loads.
The existing prototype remains as read back. Ghidra revision
`1795634906543:62a95bf0:520` is saved.

## Reconstruction and verification

[`TurbineEngineState::UpdateInletConditions`](../../../src/sim1/turbine.cpp)
lives with the spool and shaft updates and retains the three shared
original publications. The inlet, spool and torque fixture calls the updates
in their original order and passes the published temperature ratio into
the existing explicit spool argument.

| Original range, inclusive | Native VC8 precise comparison |
| --- | --- |
| `0x20c2b9d4`–`0x20c2b9dd` | 10 exact bytes for the data pointer load and Mach-cap flag test. |
| `0x20c2b9e0`–`0x20c2b9e5` | 6 normalized bytes for the half-Mach cap load, with its literal verified. |
| `0x20c2b9e6`–`0x20c2b9f0` | 11 exact bytes for the Mach comparison and its x87 flag test. |
| `0x20c2b9f3`–`0x20c2b9fa` | 8 exact bytes for selecting the uncapped Mach input. |
| `0x20c2ba0c`–`0x20c2ba52` | 71 normalized bytes covering the full publication sequence. The comparison verifies both math imports, both literal targets and all references to the three globals, including the temperature reload. |

The complete bodies differ at 132 original and 123 rebuilt bytes.
Original code receives history in ESI and compares Mach on two branches.
The rebuilt __fastcall method receives history in EDX, preserves ESI and
shares the comparison after selecting the cap. A precise control with
separate cap branches produced 140 bytes and matched fewer ranges.
The 120-byte fast-profile control reverses the two multiplications in
the temperature factor and emits `FSQRT`. The retained precise build
preserves the original multiplication order and CRT square-root call.

Eight host x86 cases passed, including both caps, ordered negative and
unordered Mach, per-engine Mach with shared ratio publication, and the
handoff through spool speed to normalized torque. The atmospheric
producer and full turbine loop remain outside the reconstructed scope.
The native propeller and aerodynamics checks and linked propulsion
comparison passed. The spool and shaft torque object bodies are
byte-identical to their retained precise baselines. Simulator runtime
verification remains open.

[Live Ghidra evidence](../../../artifacts/sim1.dll/turbine-inlet-ghidra.json) ·
[Retained VC8 comparison](../../../artifacts/sim1.dll/turbine-inlet-vc8/turbine-inlet-comparison.json) ·
[Fast-profile control](../../../artifacts/sim1.dll/turbine-inlet-vc8-fast/turbine-inlet-comparison.json) ·
[Host x86 results](../../../artifacts/sim1.dll/turbine-inlet-host-check/results.txt)

[Native propeller regression](../../../artifacts/sim1.dll/sim1-turbine-inlet-propeller-regression-20261125-001.log) ·
[Native aerodynamics regression](../../../artifacts/sim1.dll/sim1-turbine-inlet-aero-regression-20261125-001.log) ·
[Linked propulsion regression](../../../artifacts/sim1.dll/sim1-turbine-inlet-linked-regression-20261125-001.log)
