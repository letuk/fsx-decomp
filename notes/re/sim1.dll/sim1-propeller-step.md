# Mapped propeller step and speed synchronization

`sim1.dll:0x20c20430` schedules `0x20c47420` after the engine-type
update when the propeller count is nonzero. The original `__stdcall`
entry takes the airplane and the time step. It uses `engine_map` at
configuration payload `+0x45c` to pair each `0x54`-byte propeller
record with its `0xec`-byte engine record.

| Address | Verified operation | Status |
| --- | --- | --- |
| `0x20c47441`, `0x20c47451` | Call `0x20c48270` to prepare the per-propeller `+0x34` flag and `0x20c48710` to update the bounded per-propeller load factor at `+0x3c`. | live verified |
| `0x20c47456`, `0x20c474bc` | Prepare a `0x14`-byte scratch record per engine and a `0x30`-byte record per propeller. Count the propellers mapped to each engine and keep the last propeller index seen for each engine. The initialization and setup cap their respective counts at four. | live verified |
| `0x20c474e1`, `0x20c4751d` | Set disk area from the configured diameter and initialize the propeller's angular speed from the mapped engine speed fraction, the runtime RPM reference at `+0x61c`, the ratio at configuration `+0x240`, and the radians-per-second conversion. | live verified |
| `0x20c47528`, `0x20c47536` | Call `0x20c47720` to prepare the blade pitch and advance ratio, then `0x20c47ae0` to calculate the aerodynamic torque load for the step. | live verified |
| `0x20c47580`, `0x20c475bd` | Divide the engine output at `+0x44` by its mapped propeller count and pass the result to `0x20c47ba0`. That routine integrates angular speed and writes signed RPM and normalized speed on the propeller. | live verified |
| `0x20c475f8`–`0x20c47648` | For each engine, copy the last mapped propeller's normalized speed at `+0x08` to engine `+0x5c`. Reverse its sign when the propeller's rotation factor at `+0x44` is strictly negative. Store the RPM reference times the resulting fraction at engine `+0x4c`. | live verified |
| `0x20c47698`, `0x20c476c3`, `0x20c476e1` | Per propeller, publish thrust at `0x20c47c90`, update the synchronization correction at `0x20c48150`, then publish induced airspeed to the mapped engine at `0x20c480b0`. These later passes use the configured counts directly. | live verified |

### Integrated mapped propeller passes

`FlightDynamics::UpdateMappedPropellersAfterPreparation` reconstructs
`sim1.dll:0x20c47456`–`0x20c47713` within the original
`0x20c47420` propeller step. The scope starts after the two
preparation calls to `0x20c48270` and `0x20c48710`. Those calls
provide the automatic-feather arm flags and propeller load factors
used by the integrated passes. `FlightDynamics::UpdateMappedPropellers`
now runs both preparation functions before those passes.

The first pass associates each propeller with its configured engine.
The `0x14`-byte reconstructed engine scratch record holds the count,
the last propeller index, and the shared shaft torque. The `0x30`-byte
step scratch record holds each propeller's disk area, angular speed,
advance ratio, aerodynamic torque and aerodynamic power. Disk area
comes from `diameter × (π/4) × diameter`. Initial angular speed comes
from the mapped engine's existing fraction and RPM reference, divided
by `gear_reduction_ratio`.

The pitch and aerodynamic torque passes complete before speed
integration starts. Integration divides the engine output at `+0x44`
by the number of mapped propellers. Once all propeller speeds are
integrated, the last propeller mapped to each engine supplies the
speed fraction. The original direction correction applies when
its rotation factor is negative. Engine `+0x4c` receives that
published fraction times the RPM reference. The final propeller
pass writes thrust, updates synchronization, and publishes induced
airspeed. Where multiple propellers share an engine, the last publish
to that engine supplies the induced airspeed downstream. These
relationships are live verified. The scratch and field names are
reconstructed.

The source validates the four-slot topology before updating records.
It leaves an engine with no mapped propeller unchanged. The original
publication pass would read the scratch index even for an unmapped
engine, and its later loops use raw counts after the bounded setup.
Those edge cases account for part of the structural difference.

The native VC8 RTM `/fp:precise` run verifies **21 exact and 42
normalized bytes** in twelve ranges, including all six downstream
call targets and both matching conversion literals. The complete
extracted body differs (702 original, 1,072 rebuilt bytes), including
topology validation, scratch initialization and register layout.
Twelve host x86 checks verify the split torque and published values
for two oppositely rotating propellers on one engine. The host check
uses a local ordinal-199 phase fixture. The original import binding
remains verified independently in the speed-integrator comparison.
The existing linked propulsion scopes pass. Simulator verification
remains open.

Suggested names in the original include `local_130` → `propellerSteps`
and `local_70` → `engineSteps` at their verified stack offsets. The
names remain proposals. Ghidra tags were saved and read back clean at
revision `1795632255946:62a95bf0:349`.

- [Original mapped step listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c47420.asm)
- [Native comparison](../../../artifacts/sim1.dll/propeller-mapped-pass-comparison.json)
- [Ghidra and topology evidence](../../../artifacts/sim1.dll/propeller-mapped-pass-ghidra.json)
- [Selected VC8 build](../../../vm-share/modules/sim1/runs/sim1-propeller-mapped-pass-20261125-003)
- [Aggregate initialization trial](../../../vm-share/modules/sim1/runs/sim1-propeller-mapped-pass-20261125-001)
- [Host x86 test log](../../../artifacts/sim1.dll/propeller-mapped-pass-host-check/result.txt)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-propeller-mapped-pass-propulsion-20261125-001)

### Distinct `+0x3c` fields

The caller loads ESI at `0x20c4759a` from `IntegrationHistory+0x438`
and adds the `0x54`-stride index before calling `0x20c47ba0`. At
`0x20c47bd1` the integrator reads the propeller load factor. Its
writer is `0x20c48710`, and the same field reduces published thrust
via `1 − 0.2 × factor` at `0x20c4807f`. The earlier identification
of this read as induced airspeed is **superseded**. The separate
engine field `EngineForceRecord+0x3c` is written by `0x20c480b0`
and read by the tail slipstream routine `0x20c45760`.

### Propeller icing load factor

`sim1.dll:0x20c48710` updates propeller `+0x3c` before the mapped
propeller calculations. The step caller at `0x20c47451` passes the
airplane in ECX and a double step on the stack. The reconstructed
`FlightDynamics::UpdatePropellerLoadFactor` queries circuit `0x11`
when the deice switch is on. This supersedes the earlier Boolean
input used to isolate the icing arithmetic.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20bd02ab`, `0x20c48750`–`0x20c48764` | The environment holds temperature in degrees Celsius at `+0x40`. The update rounds it to float and sends ordered temperatures at or below zero through the freezing-condition checks. An unordered temperature takes the fast decay path. | live verified |
| `0x20c48720`, `0x20c48766`–`0x20c48790` | At a freezing temperature with a nonzero severity at `+0x54`, environment flag `+0x50` selects accumulation or slow decay. The four accumulation rates are `0.0004`, `0.0008`, `0.0012` and `0.0016` per second. Slow decay is `−0.0016`. A positive temperature, unordered temperature, or zero severity uses `−0.016`. | live verified |
| `0x20c48790`–`0x20c487bf`, `0x20c48870`, `0x20bfefa0` | The propeller deice switch at state `+0x610` gates the electrical query `0x20bfee30` with selector `0x11`. The switch handler reads configuration `prop_deice_available`. The electrical configuration reads `propeller_deice`. | live verified |
| `0x20c487da`–`0x20c48853` | A gated positive circuit result clears each propeller load to zero. Otherwise add the binary64 increment and clamp the rounded propeller result to `[0, 1]`. An unordered load becomes zero. | live verified |
| `0x20c47bd1`, `0x20c4807f` | The integrator uses `1 + 0.3 × load` on aerodynamic torque and the thrust writer uses `1 − 0.2 × load` on thrust. | live verified |

The environment flag and severity field names describe the reads
in this function. Their upstream weather-field producers remain
open. The reconstructed rate selection uses fast decay for a
severity outside the five-entry table. The original indexes the
table directly on the accumulation path.

The current native VC8 RTM `/fp:precise` run verifies **30 exact and
40 normalized instruction bytes** across twelve ranges, including
the relocated call to the shared query. All **40
accumulation-rate data bytes** are exact. The original constructs
the table on the stack. The reconstruction uses a constant
array. The complete body differs (340 original, 241 rebuilt bytes),
including argument setup for the circuit call and the clamping layout.
The linked propulsion regression preserves its earlier scopes.
Eighteen host x86 checks cover rates, temperature rounding, bounds,
switch and circuit inputs, and unordered values. Simulator testing
remains open.

Suggested Ghidra names include `param_1` → `step`, `local_8` →
`temperatureCelsius`, and `bVar5` → `deiceActive`. These remain
proposals. Ghidra tags were saved and read back clean at revision
`1795634311730:62a95bf0:512`.

- [Original listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c48710.asm)
- [Current native comparison](../../../artifacts/sim1.dll/propeller-load-factor-with-circuit-comparison.json)
- [Ghidra, weather and switch evidence](../../../artifacts/sim1.dll/propeller-load-factor-ghidra.json)
- [Current VC8 build](../../../vm-share/modules/sim1/runs/sim1-propeller-circuit-20261125-002)
- [Superseded Boolean-input comparison](../../../artifacts/sim1.dll/propeller-load-factor-comparison.json)
- [Earlier rate selection trial](../../../vm-share/modules/sim1/runs/sim1-propeller-load-factor-20261125-001)
- [Host x86 test log](../../../artifacts/sim1.dll/propeller-load-factor-host-check/result.txt)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-propeller-load-factor-propulsion-20261125-001)

### Electrical circuit availability

The shared query `sim1.dll:0x20bfee30` takes the airplane in EAX,
the electrical runtime in ESI and the circuit selector in EDI.
`ElectricalRuntime::CircuitAvailability` uses ordinary C++ argument
passing. `ElectricalRuntime::FindBusVoltage` reconstructs the lookup
at `0x20bfef40`. The type and member names are reconstructed.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20bfee30`–`0x20bfee51`, `0x20bd1040` | The airplane fields at `+0x64` and `+0xb4` are installed from constructor arguments. With `+0xb4` zero, the query calls owner slot `+0x114` with owner in ECX and tests EAX against one. Other results return the unmodified runtime DWORD at `+0x244`. The slot's concrete target and its broader interface remain open. | live verified |
| `0x20bfee52`–`0x20bfee6c`, `0x20bfef40`–`0x20bfef8f` | Circuit records start at runtime `+0xb0`, stride `0x0c`. A descriptor pointer at record `+4` is required. Descriptor `+0x10` selects one of eight voltage addresses, starting at runtime `+4` with stride `0x10`. An out-of-range bus selects the available result. The complete circuit record count remains open. | live verified |
| `0x20bfee6e`–`0x20bfeeac`, `0x20bfefa0`, `0x20c000ca`, `0x20c7c008` | A valid bus passes when its voltage is strictly above the descriptor minimum and the circuit flag is zero. Zero `max_battery_voltage` or nonzero `electric_always_available` also makes a configured circuit available. The option is zero in the loaded image. An unordered voltage or battery comparison cannot pass that comparison. | live verified |
| `0x20c4828a`–`0x20c482b0`, `0x20c48797`–`0x20c487b8` | Automatic feathering calls circuit `0x09` for engine type 5 with history `+0x520` set. Propeller deice tests history `+0x610` and calls circuit `0x11`. Both get the electrical runtime at history `+0x534`. | live verified |

Native VC8 confirms **22 exact and 12 normalized instruction bytes**
in the query (125 original, 148 rebuilt). The validated operands
are the relative call to the bus lookup and the address of the
electrical option. The lookup differs (47 original, 30 rebuilt
bytes). The original uses a jump table. All eight original
table targets and their offsets were checked individually. The
reconstruction uses an indexed array. The host x86 fixture passes
25 electrical and 18 icing checks. The linked propulsion
regression preserves its existing comparison scopes. Simulator
verification remains open.

The electrical runtime and its methods now live in
`src/sim1/electrical.h` and `src/sim1/electrical.cpp`.
The separate native VC8 build retains the same instruction bytes
for the two electrical functions, with the same validated call and
global operands. The native propeller build still resolves the
two caller relocations by external symbol. The linked propulsion
build and both host fixtures passed with the two translation units.

Suggested Ghidra names for `FUN_20bfee30` are `unaff_ESI` →
`electrical`, `unaff_EDI` → `circuitIndex`, and `pdVar2` →
`busVoltage`. These remain proposals. Ghidra tags were saved
and read back clean at revision `1795634311730:62a95bf0:512`.

- [Original circuit query](../../../artifacts/sim1.dll/sim1-inventory/functions/20bfee30.asm)
- [Original bus lookup](../../../artifacts/sim1.dll/sim1-inventory/functions/20bfef40.asm)
- [Ghidra evidence](../../../artifacts/sim1.dll/electrical-circuit-ghidra.json)
- [Native comparison](../../../artifacts/sim1.dll/electrical-circuit-comparison.json)
- [Native VC8 build](../../../vm-share/modules/sim1/runs/sim1-propeller-circuit-20261125-002)
- [Standalone electrical VC8 comparison](../../../artifacts/sim1.dll/electrical-split-comparison.json)
- [Standalone electrical VC8 build](../../../vm-share/modules/sim1/runs/sim1-electrical-split-20261125-001)
- [Propeller VC8 build with external electrical calls](../../../vm-share/modules/sim1/runs/sim1-propeller-split-20261125-001)
- [Linked build with both translation units](../../../vm-share/modules/sim1/runs/sim1-propeller-split-linked-20261125-001)
- [Host x86 test log](../../../artifacts/sim1.dll/electrical-circuit-host-check/results.txt)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-propeller-circuit-linked-20261125-001)

### Automatic feather arming

`FlightDynamics::UpdateAutomaticFeatherArming` reconstructs
`sim1.dll:0x20c48270` and
`TurbineEngineState::HasAutomaticFeatherTorque` reconstructs
`0x20c48250`. The caller at `0x20c47441` runs arming before
the ice-load update at `0x20c47451`. The new mapped-step entry
recreates that order before calling the extracted mapped passes.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c48220`, `0x20c22dbb`, `0x20beacd5` | The switch handler writes history `+0x520`. It turns on only if configuration `+0x278` (`prop_auto_feathering_available`) is enabled and the switch was off. The panel state reader reads the same field. | live verified |
| `0x20c48250`–`0x20c48268`, `0x20bc35a0`, `0x20c2c850` | The torque gate returns one only for an ordered value strictly above 0.17 at turbine record `+0x28`. The turbine update writes normalized shaft torque there. | live verified |
| `0x20c4827b`–`0x20c482c4`, `0x20bc4270`, `0x20c2c2c5` | Type 5 uses the switch and circuit 9. Each arming test requires the designated turbine record's first double to be strictly above 88.0. The turboprop update writes a turbine speed value there. The name `turbineSpeed` describes that observed use, and the precise spool label remains open. The count dispatch covers 1 through 4. | live verified |
| `0x20c48497`, `0x20c484d5`, `0x20c48516`, `0x20c48551`, `0x20c48563` | The four-propeller path writes propeller 1 and writes all three later results to propeller 2. The last result uses turbine 4 speed and turbine 2 torque. Propeller 3 and 4 retain their previous flags. | live verified |
| `0x20c4856d`–`0x20c4859d` | Other engine types clear all configured propeller flags. Type 5 with a count outside 1 through 4 leaves the flags as they were. | live verified |

The case table below uses one-based turbine and propeller indices.
Each listed condition also requires switch and circuit availability.

| Count | Writes in order, with speed turbine and torque turbine(s) |
| --- | --- |
| 1 | Propeller 1 uses speed 1 and torque 1. |
| 2 | Propeller 1 uses speed 1 and torque 2. Propeller 2 uses speed 2 and torque 1. |
| 3 | Propeller 1 uses speed 1 and torque 3. Propeller 2 uses speed 2 and torque 1 and 3. Propeller 3 uses speed 3 and torque 1. The original repeats the speed 2 check before torque 3. The source folds the repeated read into one comparison. |
| 4 | Propeller 1 uses speed 1 and torque 3. Successive writes to propeller 2 use speed 2 and torque 3, speed 3 and torque 2, then speed 4 and torque 2. |

VC8 confirms **15 exact and 27 normalized bytes** in the arming
routine. The original spans 814 bytes including three bytes of
alignment inside that span. The rebuilt instruction span is 648
bytes before the case table. The comparison checks all four
entries in both switch tables individually. The torque gate
matches **16 exact and 6 normalized bytes**. Both copies are 25
bytes and the remaining instruction uses EAX in the original and
ECX in the C++ call convention. The mapped-step preparation
confirms **8 exact and 13 normalized bytes**, including both
original preparation calls, and checks the rebuilt call to the
extracted passes separately. The native regression and linked
propulsion comparison passed their established scopes.

The x86 fixture covers 3,072 combinations of propeller count,
power, speed and torque, plus unordered inputs, the disabled switch,
the non-turboprop clearing path and the reconstructed step entry.
Simulator verification remains open. Suggested Ghidra names for
`FUN_20c48270` are `in_EAX` → `airplane`, `piVar2` → `history`
and the initial `iVar3` value → `circuitAvailable`. The latter
would require splitting the reused decompiler variable. These are
proposals. Ghidra tags were saved and read back clean at revision
`1795634906543:62a95bf0:520`.

- [Original arming listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c48270.asm)
- [Original torque-gate listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c48250.asm)
- [Ghidra evidence](../../../artifacts/sim1.dll/automatic-feather-ghidra.json)
- [VC8 comparison](../../../artifacts/sim1.dll/automatic-feather-comparison.json)
- [Native VC8 build](../../../vm-share/modules/sim1/runs/sim1-propeller-automatic-feather-20261125-002)
- [Host x86 test log](../../../artifacts/sim1.dll/automatic-feather-host-check/results.txt)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-propeller-automatic-feather-linked-20261125-001)

### Propeller speed integration

`sim1.dll:0x20c47ba0` is reconstructed as
`PropellerForceRecord::IntegrateSpeed`. The call at `0x20c475bd`
passes a `0x30`-byte propeller step record in ECX, the shared state
in EDX, and the propeller record in ESI. The stack carries the
engine torque share and the time step. The C++ member keeps the
state in EDX and takes the step record on the stack. Its receiver
arrives in ECX and the compiler copies it to ESI.

| Address | Evidence | Status |
| --- | --- | --- |
| `0x20c47ba6`–`0x20c47bc7` | Save aerodynamic torque times initial angular speed at step `+0x18` for the thrust calculation. For engine type zero (piston), scale the engine torque share by `gear_reduction_ratio` from configuration payload `+0x240`. | live verified |
| `0x20c47bca`–`0x20c47bfa` | Calculate `((engine torque) − (1 + 0.3 × load factor) × aerodynamic torque) / propeller_moi`. When state `+0x488` is nonzero, substitute `−initialAngularSpeed / step`, which drives the Euler update to zero. The state flag's writer remains open. | live verified |
| `0x20c47bfa`–`0x20c47c28` | Advance `angularSpeed += acceleration × step`. Set it to zero when both absolute quantities are strictly below their respective thresholds, `2 rad/s` and `8 rad/s²`. The x87 branch keeps the computed value if either comparison is unordered. | live verified |
| `0x20c47c2a`–`0x20c47c5f` | Convert angular speed to RPM with the original literal `9.549296586` and reverse its sign if the propeller's rotation factor is strictly negative. Divide the signed result by `(RPM reference / gear reduction ratio)` for the normalized speed. | live verified |
| `0x20c47c62`–`0x20c47c7c` | Compute the signed phase increment as `RPM × 0.1047197551 × step`. Call `util.dll` ordinal 199 through IAT slot `0x20bb13e8` and store the result as a float at propeller `+0x10`. | live verified |

The pinned `util.dll` PE resolves ordinal 199 to `0x20f51990`.
That body adds the phase and increment and wraps the sum with a
`2π` period. Its large-increment path calls `msvcr80.dll!modf`.
The comparison import library binds reconstructed name
`AdvanceWrappedAngle` to the same DLL and ordinal. The matching
report verifies the original IAT operand and the rebuilt COFF
relocation. The linked propulsion regression also verifies the
generated import library and preserves the established thrust
and vector-publication scopes.

The native VC8 RTM `/fp:precise` COFF run verifies **37 exact and
161 normalized bytes** across nine separate ranges. Normalization
validates six binary64 constants and the import individually.
The complete function differs (229 original, 232 rebuilt bytes).
Remaining differences include stack offsets, register choices in
the engine-type check and the epilogue. Linked equivalence for
this integrator and tests in the simulator remain open.

Suggested Ghidra names include `param_1` → `stepInputs`,
`unaff_ESI` → `propeller`, and `dVar1` → `angularAcceleration`
within the integration section. These names remain proposals.
The tags were saved and read back clean at revision
`1795614553364:62a95bf0:219`.

- [Original integrator listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c47ba0.asm)
- [Native comparison](../../../artifacts/sim1.dll/propeller-speed-comparison.json)
- [Ghidra and PE evidence](../../../artifacts/sim1.dll/propeller-speed-ghidra.json)
- [Original utility ordinal listing](../../../artifacts/util.dll/propeller-phase-ordinal-199.asm)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-propeller-speed-20261125-002)
- [Linked propulsion regression and import binding](../../../vm-share/modules/sim1/runs/sim1-propeller-speed-propulsion-20261125-002)
- [Linked thrust regression](../../../vm-share/modules/sim1/runs/sim1-propeller-speed-thrust-20261125-001/thrust-comparison.json)

### Blade pitch and advance ratio

`sim1.dll:0x20c47720` runs before the aerodynamic propeller torque
calculation. The original caller at `0x20c47528` passes the propeller
index in EAX and the airplane, step record, mapped engine index and
time step on the stack. Three code ranges have separate C++ definitions.

| Range | Verified behavior | Status |
| --- | --- | --- |
| `0x20c4772e`–`0x20c477e2` | Write `n = angularSpeed / (2π)` to step `+0x10`. Write `999` to step `+0x28` if `abs(n) < 0.0001`. Otherwise calculate the advance ratio from positive axial speed divided by `diameter × n`. For `abs(normalizedSpeed) < 0.25`, multiply by `max(0, (abs(normalizedSpeed) − 0.125) × 8)`. | live verified |
| `0x20c477e3`–`0x20c47878` | Dispatch variable pitch for configuration `propeller_type == 0`. Update the low-speed latch at propeller `+0x24` using `min_rpm_for_feather`, the governor input and the current feather state. Call `0x20c485c0` for the feather-state update. The latch and helper call are reconstructed at `0x20c47818`. | live verified |
| `0x20c47879`–`0x20c47a2e` | The latch or an eligible governor command selects the governed RPM target. Interpolate between `min_gov_rpm` and the runtime RPM reference using the command plus synchronization correction, capped at `1`. Convert to radians per second. Otherwise select zero target speed and use `beta_feather` as the upper limit. | live verified |
| `0x20c47902`–`0x20c47a2e` | Use `(gearReductionRatio × angularSpeed − targetSpeed) × prop_tc` as the pitch rate, or `−0.2` for commanded reverse. Apply the defeathering increment on its enabled path. Compute the lower limit from the configured normal, ground, or interpolated reverse value. Integrate by `step` and apply the upper limit. Apply the lower limit only for a negative pitch rate. | live verified |
| `0x20c47a2f`–`0x20c47ad0` | For `propeller_type == 1`, reset to `fixed_pitch_beta` when the optional cruise record is absent. Select cruise pitch from fixed pitch only above the configured axial speed. With cruise pitch already selected, reset when airplane `+0x1f4 < 5` and the propeller's normalized speed is exactly zero. Unrecognized current pitch resets to the fixed pitch. | live verified |

`0x20c22a20` parses `fixed_pitch_beta_auto_cruise` into a `0x0c`-byte
record reached through configuration `+0x1984`. The pitch value is a
double rounded through float after conversion to radians. The speed is
a float after multiplying by `1.6880556`. The runtime comparison uses
this float against double axial speed at state `+0x104`.

The initial `ApplyGovernorPitch` took both selectors as inputs. The
integrated reconstruction now gets feather state from
`UpdateGovernorFeatherState` and calls `0x20c1cbf0` as
`UsesGroundContactLimits` only on the original ground-limit path.
Field names for the controls and latch are reconstructed from their reads.

The initial native VC8 `/fp:precise` comparisons for the three extracted
C++ definitions confirmed **43 exact and 89 normalized bytes**. The ranges
contain 181, 438 and 162 original bytes and each complete range
differs, including the extracted functions' new call interfaces.
The host MSVC x86 smoke test passes 17 checks across the advance
ratio, cruise-pitch transitions and governor bounds. Simulator
verification of the enclosing function remains open.

Suggested Ghidra names include `param_2` → `stepInputs`,
`param_3` → `engineIndex`, and `in_EAX` → `propellerIndex`. These
are proposals based on the original call site. The scoped Ghidra tags
were saved and read back clean at revision `1795629219460:62a95bf0:234`.

- [Original listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c47720.asm)
- [Native comparison](../../../artifacts/sim1.dll/propeller-pitch-comparison.json)
- [Ghidra and parser evidence](../../../artifacts/sim1.dll/propeller-pitch-ghidra.json)
- [Selected VC8 build](../../../vm-share/modules/sim1/runs/sim1-propeller-pitch-20261125-003)
- [Host x86 smoke-test log](../../../artifacts/sim1.dll/propeller-pitch-host-check/result.txt)
- [Aerodynamic regression](../../../vm-share/modules/sim1/runs/sim1-propeller-pitch-aero-20261125-002)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-propeller-pitch-propulsion-20261125-001)

### Feather state and low-speed latch

`sim1.dll:0x20c485c0` updates the receiver's feather state at
propeller `+0x38`. The original call at `0x20c47874` supplies the
propeller in ECX, the mapped **engine index** in EAX, the shared
state in EDI, and a double time step on the stack. The receiver is
selected by propeller index. The lookup for normalized speed uses
the mapped engine index into the propeller array.

| Address | Verified operation | Status |
| --- | --- | --- |
| `0x20c485c8`–`0x20c48600` | For a nonzero request timer at propeller `+0x50`, round the time step to float, subtract from the timer, and round the result to float before clamping. A nonpositive or unordered remaining time becomes zero. | live verified |
| `0x20c48607`–`0x20c48646`, `0x20c2c850` | From the inactive state, automatic feathering requires engine type `5`, the receiver's arm flag at `+0x34`, and mapped normalized shaft torque strictly below `0.10000000149011612`. The turbine update writes torque normalization to its `+0x28` slot in the `0x80`-byte array at shared state `+0x434`. | live verified |
| `0x20c4864f`–`0x20c48682` | A remaining request timer also activates feathering when absolute normalized speed of the indexed propeller is greater than `0.5`. | live verified |
| `0x20c48685`–`0x20c48705` | An active type-5 feather state clears if the arm flag clears or normalized torque rises strictly above the activation threshold. With `feathering_switches` enabled at configuration `+0x298`, it also clears when both the absolute blade-pitch error and absolute indexed normalized speed are below `0.0001`. | live verified |
| `0x20c47818`–`0x20c47878` | The variable-pitch caller sets the low-speed latch when ordered RPM is below `min_rpm_for_feather`, the command is positive and the receiver is inactive. A latched receiver clears the latch when ordered RPM reaches or exceeds the threshold and command remains positive. It then calls the feather-state routine. | live verified |

The native VC8 `/fp:precise` comparison confirms **85 exact and
33 normalized bytes** for `UpdateFeatherState` and **8 exact and
11 normalized bytes** for the extracted latch and call. The latter
validates the COFF relocation to the reconstructed feather-state
routine. Full bodies differ, including the register ABI and timer
stack offsets. The host x86 smoke test passes 17 cases, including
the equal-torque boundary, both sides of the RPM threshold, a distinct mapped index, float
timer rounding, and unordered values. The linked propulsion
regression preserves its prior scopes. Simulator verification of
the enclosing caller remains open.

Suggested names for `0x20c485c0` include `in_EAX` → `engineIndex`,
`unaff_EDI` → `state`, and the two stack words → `step` as a single
double. These names remain proposals. Ghidra tags were saved and
read back clean at revision `1795630948540:62a95bf0:239`.

- [Original feather listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c485c0.asm)
- [Native comparison](../../../artifacts/sim1.dll/propeller-feather-comparison.json)
- [Ghidra and field evidence](../../../artifacts/sim1.dll/propeller-feather-ghidra.json)
- [Selected VC8 build](../../../vm-share/modules/sim1/runs/sim1-propeller-feather-20261125-002)
- [Host x86 smoke-test log](../../../artifacts/sim1.dll/propeller-feather-host-check/result.txt)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-propeller-feather-propulsion-20261125-001)

### Ground contact selector and integrated blade-pitch update

`sim1.dll:0x20c1cbf0` is reconstructed as
`FlightDynamics::UsesGroundContactLimits`. It returns one only when
the qualified contact count is nonzero, the flap deflection factor
is ordered and at least `1.0`, and the reference object's surface
mode differs from `2`. The equality case at `1.0` follows the original
`FCOMP` and parity branch. This supersedes the ambiguous decompiler
expression. The physical meaning of individual mode values remains open.

| Original address | Evidence | Status |
| --- | --- | --- |
| `0x20c48add`, `0x20c48fef` | The ground reaction update clears the counter at state `+0x214`. Its per-contact path increments the counter when the calculated clearance is nonpositive and the contact carries flag `0x10`. | live verified |
| `0x20c1cbf6`–`0x20c1cc16` | The original checks the count and state `+0x6e4`, loads the reference object from airplane `+0x14`, supplies that object in ECX and calls table slot `+0x0c` with no stack arguments. | live verified |
| `0x20bd1072`, `0x20bcefc0`, `0x20bcf2d7` | The base constructor receives the airplane subobject at `+0x0c` and installs the allocated reference at subobject `+0x08`, or airplane `+0x14`. The reference constructor writes table `0x20bbd938`. | live verified |
| `0x20bbd940`, `0x20bceec0` | Slot `+0x08` points to a no-stack-argument method that reads reference `+0x14` into ST0. This is the altitude reference slot used by `0x20bd1c90`. | live verified |
| `0x20bbd944`, `0x20bceed0` | Slot `+0x0c` points to a no-stack-argument method that returns the reference's `+0x1c` word in EAX. The caller compares all 32 bits with `2`. | live verified |

The minimum C++ table projection preserves the observed surface
mode and height signatures. The shared `GroundReferenceDispatch`
layout leaves the other slots opaque. The table name, fields and
C++ signatures are reconstructed descriptions of the verified ABI.

`FlightDynamics::UpdatePropellerBladePitch` now represents the full
`0x20c47720` calculation with extracted helpers. It initializes
advance ratio for the current propeller, selects fixed or variable
pitch from `propeller_type`, updates the variable-pitch latch and
feather state, then applies the governed pitch and its limits. The
governor calls the virtual selector after computing its rate and
before storing pitch, with the original reverse-capability and
low-control gates. The enclosing propeller step `0x20c47420` remains
to be reconstructed. The new pitch entry remains callable from it.

The native VC8 RTM `/fp:precise` selector comparison verifies **44
exact bytes** of the 54-byte body. The remaining differences are
the initial zero branch and EAX return materialization. The combined
pitch dispatcher with extracted calls verifies **14 exact bytes**
and resolves its four stage calls. Its COFF entry is 152 bytes versus
945 original bytes that contain the stages directly. The fresh
governor comparison adds a **five-byte normalized call** whose target
is verified on both sides. The earlier pitch, feather and propulsion
scopes replayed. A linked position regression verifies the altitude
path with the extended table projection. Thirteen host x86 checks
exercise the real dispatch slot, ground gating and combined pitch
paths. Simulator verification remains open.

Suggested Ghidra names are `param_1` → `airplane` and `iVar2` →
`surfaceMode` in `0x20c1cbf0`. These remain proposals. The two entry
tags were saved and read back clean at revision
`1795631628753:62a95bf0:346`.

- [Original ground selector listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c1cbf0.asm)
- [Original pitch listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c47720.asm)
- [Constructor, table and target evidence](../../../artifacts/sim1.dll/propeller-ground-selector-ghidra.json)
- [Native comparison](../../../artifacts/sim1.dll/propeller-ground-selector-comparison.json)
- [Selected VC8 build](../../../vm-share/modules/sim1/runs/sim1-propeller-ground-selector-20261125-003)
- [Host x86 smoke-test log](../../../artifacts/sim1.dll/propeller-ground-selector-host-check/result.txt)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-propeller-ground-selector-propulsion-20261125-001)
- [Linked position regression](../../../vm-share/modules/sim1/runs/sim1-propeller-ground-selector-position-20261125-001)

### Aerodynamic propeller torque

`sim1.dll:0x20c47ae0` returns the aerodynamic torque used by the
propeller speed integrator. The caller at `0x20c47536` provides the
propeller in EDX, the airplane in EDI, and the step record in EBX.
It stores the x87 return value at step `+0x08` before calling
the integrator. The reconstructed member is
`PropellerForceRecord::CalculateAerodynamicTorque`.

| Address | Evidence | Status |
| --- | --- | --- |
| `0x20c47ae6`–`0x20c47b12`, `0x20c22a20` | If the absolute blade-pitch difference from `beta_feather` is strictly below `0.0001 rad`, use `power_absorbed_cf`. The configuration payload stores those two inputs at `+0x284` and `+0x28c`. | live verified |
| `0x20c47b14`–`0x20c47b44` | Otherwise look up the coefficient with advance ratio and blade pitch in degrees. A negative blade pitch uses its magnitude and zero advance ratio. An unordered feather comparison also takes the lookup path. | live verified |
| `0x20c47b50`–`0x20c47b98`, `0x20c488a0` | Reverse the coefficient's sign for strictly negative angular speed. With revolutions per second `n`, density `ρ`, diameter `D`, and the selected signed coefficient `C`, return `ρ × C × n² × D⁵ / (2π)`. The initializer at `0x20c488a0` stores `D⁵` at shared state `+0x550`. | live verified |
| `0x20c251f0`, `0x20c23b60`, `0x20bcc030` | AIR record `0x512` supplies the table at configuration `+0xbac`, with storage size `0x548`. The defaults contain 7 rows and 14 columns including headers. Advance ratio is the column axis and blade pitch is the row axis. | live verified |

The shared lookup `0x20bcc030` is reconstructed as
`PropellerCoefficientTable::Evaluate`. Its two dimensions precede
the row-major `double` values. It clamps out-of-range coordinates
to the nearest edge, interpolates along the pitch axis before the
advance-ratio axis, and returns the first interior cell if either
coordinate is NaN. The C++ reconstruction uses the final cell for
both ends of the interval at an exact upper-axis hit. The original
uses a zero-weight next index. The reconstructed case preserves the
finite endpoint value without accessing beyond the logical axis.
The type and field names describe a minimum reconstructed layout.

The selected VC8 RTM `/fp:precise` comparison for `0x20c47ae0`
contains **26 exact and 39 normalized bytes** across seven ranges.
It validates the three binary64 constants and the lookup call.
The full body differs (185 original, 163 rebuilt bytes), including
the calling convention's register assignments and the original
inline integer-power loop for the square. The lookup comparison
contains **29 exact bytes** across five ranges, including the final
weighted sum. The two `_isnan` calls share one loaded CRT address
in the rebuild. The full lookup differs (519 original, 706 rebuilt
bytes), including the compiler's inlined axis scans. The out-of-line
trial and selected inline trial are retained. Simulator testing
remains open for both functions.

A host MSVC x86 smoke test passed ten checks on a known table.
It covers both axis clamps, the upper endpoint, both NaN inputs,
the two-axis interpolation, negative pitch, feathered torque, and
negative rotation. This test exercises the reconstructed source
independently of the VC8 COFF comparison.

Suggested Ghidra names include `unaff_EBX` → `stepInputs`,
`unaff_EDI` → `airplane`, and `fVar6` → `powerCoefficient` at
`0x20c47ae0`. At `0x20bcc030`, `unaff_ESI` refers to the
coefficient table. These names remain proposals. The tags were
saved and read back clean at revision `1795615039980:62a95bf0:227`.

- [Original torque listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c47ae0.asm)
- [Original lookup listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20bcc030.asm)
- [Native comparison](../../../artifacts/sim1.dll/propeller-load-comparison.json)
- [Ghidra and configuration evidence](../../../artifacts/sim1.dll/propeller-load-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-propeller-load-20261125-003)
- [Out-of-line interval trial](../../../vm-share/modules/sim1/runs/sim1-propeller-load-20261125-001)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-propeller-load-propulsion-20261125-001)
- [Host x86 smoke-test log](../../../artifacts/sim1.dll/propeller-load-host-check/result.txt)

### Synchronization correction

The final pass calls `0x20c48150` with the propeller in ESI, shared
state in EDX, and the mapped engine pointer and time step on the
stack. The reference is **engine index zero**. The original compares
the reference's speed fraction with the mapped engine's signed speed
fraction after engine speeds have been published. It requires the
configuration flag `prop_sync_available` at payload `+0x268`, a
nonzero runtime flag at state `+0x51c`, a propeller other than the
first `0x54`-byte record, and an ordered absolute speed difference
strictly below `0.088`. When enabled, it accumulates
`(referenceSpeed − mappedSpeed) × (propeller_moi × 0.04 × step)` at
propeller `+0x2c`. It clears that field otherwise. The blade-pitch
routine at `0x20c47720` reads the correction when it forms the
governor's speed command on a later step. These offsets and roles
are live verified. The C++ member and field names are reconstructed.
The event or control that writes state `+0x51c` remains open.

`PropellerForceRecord::UpdateSynchronization` uses a VC8 `__fastcall`
member interface. It keeps the state in EDX and the two stack arguments
in their original positions. The compiler passes `this` in ECX, while
the original call supplies ESI. The original expects a valid
configuration and installed reference engine. The selected source
keeps the signed engine-count selection and the two configuration
fields at their verified offsets.

The selected native VC8 `/fp:precise` COFF comparison contains
36 exact and 21 normalized bytes across ten separate ranges.
The normalized ranges validate the binary64 `0.04` gain and the
`0.088` threshold individually. The full synchronization helper
differs (126 original, 120 rebuilt bytes), including register use,
the configuration null check, branch layout, and field stores.
The mapped step at `0x20c47420` has a verified listing and phase map.
Its complete source and linked or simulator verification remain open.

Suggested Ghidra names include `unaff_ESI` → `propeller` and
`dVar2` → `speedDifference` at `0x20c48150`, based on the call site
and the two speed-fraction loads. The suggestions are provisional.
Ghidra tags were saved and read back clean at revision
`1795613966931:62a95bf0:214`.

- [Mapped step original listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c47420.asm)
- [Synchronization original listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c48150.asm)
- [Ghidra evidence](../../../artifacts/sim1.dll/propeller-step-ghidra.json)
- [Native COFF comparison](../../../artifacts/sim1.dll/propeller-sync-comparison.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-propeller-sync-20261125-003)
- [Member-call trial](../../../vm-share/modules/sim1/runs/sim1-propeller-sync-20261125-001)
- [Fastcall trial without the reference selection](../../../vm-share/modules/sim1/runs/sim1-propeller-sync-20261125-002)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-propeller-sync-propulsion-20261125-001/propulsion-comparison.json)

### Propeller thrust publication

`sim1.dll:0x20c47c90` is reconstructed as
`PropellerForceRecord::UpdateThrust`. The original call at
`0x20c47698` passes shared state in EDI and the step pointer,
propeller pointer and engine torque share on the stack. Its output
is propeller `+0x14`, which feeds the propulsion vector collector
and the induced-airspeed writer.

| Address | Verified operation | Status |
| --- | --- | --- |
| `0x20c47e3f`–`0x20c47ef6` | Use zero for the airflow term when absolute axial speed is below `1e-7`. Otherwise use aerodynamic power times efficiency divided by axial speed. At nonnegative blade pitch, AIR record `0x511` at configuration `+0x664` supplies efficiency from advance ratio and pitch in degrees. | live verified |
| `0x20c47e6d`–`0x20c47ea5`, `0x20c22a20` | At negative blade pitch, use `−0.3 × prop_reverse_available`. Backward axial flow multiplies that efficiency by `max(0, 1 − axialSpeed / −6)`. The parser stores the reverse factor as a float at configuration payload `+0x29c`. | live verified |
| `0x20c47eff`–`0x20c47f8f` | Below `low_speed_theory_limit` at payload `+0x260`, estimate static thrust from shaft horsepower and the curve for aerodynamic horsepower per unit disk area. Both conversions divide by `550`. | live verified |
| `0x20c47f92`–`0x20c4802d` | Below 10 degrees of pitch, scale the static term by the pitch curve. Scale negative static thrust on this path by the reverse factor. At or above 10 degrees, a pitch within `0.0001 rad` of `beta_feather` suppresses the static term. At nonnegative pitch with nonpositive axial flow, the ordered-positive static term is retained and other results are cleared. | live verified |
| `0x20c4802d`–`0x20c4807c` | Apply the low-speed axial-flow curve to static thrust and blend it with the airflow term. The respective weights are `(limit − axialSpeed) / limit` and `axialSpeed / limit`. At or above the limit, store the airflow term directly. | live verified |
| `0x20c4807f`–`0x20c48098`, `0x20c2325b` | Apply `1 − 0.2 × propellerLoadFactor`, then multiply by `[propeller] thrust_scalar` at configuration payload `+0x11c8`. The parser initializes the scalar to `1.0`. | live verified |

The original constructs a 13-point power-density curve and two
five-point curves for axial flow and blade pitch on the stack. The
reconstruction keeps the same 23 points in constant C++ aggregates.
The comparator replays the original x87 literal stores and compares
the counts and all coordinates against the referenced COFF data.
The **380 curve-data bytes are exact**. The comparisons validate
the three curve calls and the AIR efficiency table call separately.

Native VC8 RTM `/fp:precise` verifies **14 exact and 108 normalized
instruction bytes** across 14 ranges. The complete function differs
(1,041 original, 534 rebuilt bytes). Remaining differences include
the stack-built curves, register and argument allocation, and x87
evaluation order at the blend and final stores. The linked propulsion
checks passed. Ten independent host x86 checks cover the airflow and
static paths, the blend and its boundary, pitch and feathering, and
reverse thrust. Simulator verification remains open.

The shared one-axis evaluator `0x20bcbf60` remains owned by the
aerodynamics translation unit. It uses one definition with a separate
link context for the propulsion comparison. An inline trial changed
the roll-sum comparator output and was superseded. The final replay
reproduces all 54 prior aerodynamic COFF `.text` sections with their
relocations and passes the aerodynamic comparisons.

Suggested Ghidra names are `param_1` → `stepInputs`, `param_2` →
`propeller`, `param_3` → `engineTorque` at entry, and `unaff_EDI` →
`state`, based on the call at `0x20c47698`. The `param_3` stack slot is
reused by the original for the static term. The suggestions remain
provisional. Ghidra tags were saved and read back clean at revision
`1795615726054:62a95bf0:230`.

- [Original thrust listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c47c90.asm)
- [Native comparison and verified curves](../../../artifacts/sim1.dll/propeller-thrust-comparison.json)
- [Ghidra and parser evidence](../../../artifacts/sim1.dll/propeller-thrust-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-propeller-thrust-20261125-002)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-propeller-thrust-propulsion-20261125-001)
- [Aerodynamics regression](../../../vm-share/modules/sim1/runs/sim1-propeller-thrust-aero-20261125-003)
- [Host x86 smoke-test log](../../../artifacts/sim1.dll/propeller-thrust-host-check/result.txt)
