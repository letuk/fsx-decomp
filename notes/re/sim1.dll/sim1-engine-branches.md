# Sim1 engine branch mapping

The live Ghidra evidence and the original `sim1.dll` PE agree on the
six entries of the engine dispatch table. The Microsoft FSX
Acceleration SDK supplies the aircraft.cfg names used below.
Function descriptions and field names in this note are reconstructed.
[Engine branch evidence](../../../artifacts/sim1.dll/engine-branches-ghidra.json).
[FSX Acceleration SDK aircraft configuration](https://www.rikoooo.com/FSX_Acceleration_SDK/SimObject%20Creation%20Kit/SimObject%20Container%20SDK/Aircraft%20Configuration%20Files.htm).

## Configuration and dispatch

| Entry | Finding | Status |
| --- | --- | --- |
| `0x20c26d60` | Reads `[GeneralEngineData] engine_type` with API ordinal 49. The temporary starts at `-1`. A value other than `-1` is stored as a byte at configuration offset `+0x46c`. | live verified |
| `0x20c26f80` | Handles the `-1` sentinel. A successful AIR record `0x310` lookup selects piston and writes type 0 to the configuration and through API ordinal 50. | live verified |
| `0x20c26fe0` | Counts accepted engine definitions and stores the count at configuration offset `+0x454`. Type 2 skips the AIR fallback when no definitions were accepted. | live verified |
| `0x20c223e0` | Loads the common engine settings before dispatching the type-specific configuration readers. Type 3 checks configuration `+0x1994` before loading shared turbine settings. | live verified |
| `0x20c24270` | Reads `[piston_engine]`, including cylinder count, rated RPM and rated horsepower. | live verified |
| `0x20c26290` | Reads `[TurbineEngineData]`, including static thrust, inlet area and rated N2 RPM. | live verified |
| `0x20c26020` | Tries the `[turboprop_engine]` settings block from type 5 and from type 3 when the rotor data pointer is null. | live verified |
| `0x20c22a20` | Reads `[propeller]`, including `number_of_propellers` at `+0x458` and `engine_map` beginning at `+0x45c`. The configuration dispatcher reaches this reader for type 0 when `+0x98` is nonzero and for type 5. | live verified |
| `0x20c20430` | Updates propulsion from the flight step at `0x20c30d58`. The observed call passes the flight instance in EDI, the step as a stack double, and the target returns with `ret 8`. | live verified |

The flight instance points to the shared runtime block at `+0x270`.
The first pointer in that block leads to the configuration. The
jump table at `0x20c20544` contains six HIGHLOW-relocated addresses.
The Ghidra memory export and the pinned PE give the same 24 bytes.
These entries describe the main dispatch after the early mode gate.

| Type | SDK name | Table target | Runtime branch in the original PE |
| --- | --- | --- | --- |
| 0 | Piston | `0x20c20474` | Calls `0x20c27620`. |
| 1 | Jet | `0x20c2048b` | Calls `0x20c2b6d0`. |
| 2 | None | `0x20c2049a` | Continues at the independent propeller check. |
| 3 | Helo-Turbine | `0x20c20482` | Calls `0x20c2b6d0` when configuration `+0x1994` is null. A non-null pointer skips that call. |
| 4 | Rocket, marked unsupported | `0x20c2049a` | Continues at the independent propeller check. The separate simplified path below explicitly tests this type. |
| 5 | Turboprop | `0x20c2048b` | Calls `0x20c2b6d0`. |

Type bytes above 5 also continue at `0x20c2049a`. When the
flight-instance field `+0xb4` equals zero and the virtual call on
the object at `+0x64`, slot `+0x114`, returns a value other than 1,
the dispatcher exits through `0x20c20524`. It calls the simplified
engine update at `0x20c20bb0` only when configuration `+0x98` is
nonzero. That update uses `1 / step` for its thrust-rise coefficient
when the type byte equals 4 and `0.16` otherwise. This live verified
case is evidence of a code path for value 4. Support for a configured
rocket aircraft remains unverified.

On the main path, a nonzero propeller count invokes `0x20c47420`
after the type-specific update. The final loop advances
`engine[+0xac] += step * engine[+0x5c]` and replaces
`engine[+0x54]` with `engine[+0x4c]` on a strict ordered increase.
The latter follows the original x87 compare and branch, including
preservation of the old field on unordered input. Field names and
units for this postprocessing remain open.

## Force and moment outputs

`0x20c22140` allocates the per-engine records at runtime `+0x43c`
with stride `0xec` and the per-propeller records at runtime `+0x438`
with stride `0x54`. The force collector's registered propulsion
callback at `0x20c010f0` consumes both arrays. Its two calls to
`0x20c01010` are at `0x20c011a6` and `0x20c01281`. The first
passes each nonzero engine thrust at `+0x34`. The second passes
propeller thrust at `+0x14` and uses `engine_map` to select the
engine mount. The listing shows that the helper adds the oriented
thrust vector to the force sum and adds the cross product of the
lever arm and that vector to the moment sum.

| Entry | Finding | Status |
| --- | --- | --- |
| `0x20c27620` | Iterates the per-engine array for the piston path. Its helper `0x20c278b0` writes the engine output at `+0x44` used as an input by `0x20c47420`. | live verified |
| `0x20c2b6d0` | Shares the turbine loop across types 1, 3 and 5. Type 1 calls `0x20c2c120` and `0x20c2c230`. Types 3 and 5 call `0x20c2c6d0` and `0x20c2c850`. The latter writes the same engine offset `+0x44` consumed by the propeller update. | live verified |
| `0x20c2c620` | Called at `0x20c2b8cd` after the turbine sub-branches converge. Writes per-engine thrust at `+0x34` at instruction `0x20c2c6b8`. | live verified |
| `0x20c47420`, `0x20c47c90` | The propeller update consumes the mapped engine output at `+0x44`. Its call at `0x20c47698` reaches the helper that writes propeller thrust at `+0x14` using the configured thrust scalar. | live verified |
| `0x20c010f0`, `0x20c01010` | A callback and its projection helper carry the engine and propeller thrust into the force and moment collector. The caller supplies force sum in EBX, lever arm in ESI, moment sum in EDI, and a pointer to two orientation values plus a thrust double on the stack. | live verified |
| `0x20c4c7d0` | Checks AIR records `0x1400` through `0x1404`. With all five, it requests the `0x1b0`-byte rotor configuration allocation through `+0x1994`. A partial set clears that pointer. | live verified |
| `0x20c047e0` | Uses the same pointer test as type 3 of the engine dispatch. Registers `0x20c176f0` when it is null and `0x20c4c360` when it is non-null. This registration tests the pointer directly. | live verified |
| `0x20c176f0`, `0x20c4c360` | The null-pointer callback forwards to `0x20c17720`. The non-null callback runs a separate rotor calculation and publishes both force and moment through its output arguments. | live verified |

The propeller loop's initialization, speed integration, engine speed
publication and final outputs are mapped in [Mapped propeller step and
speed synchronization](sim1-propeller-step.md).

## Scope and follow-up

The prior inventory listed the engine-type mapping as a hypothesis.
The table and force-bus links above supersede that assessment for
the verified dispatch and outputs. The identities of the virtual
mode query's concrete targets, the full piston and turbine state
layouts, and per-aircraft runtime behavior remain open.

The initial mapping left the thrust helper's byte comparison pending.
The linked comparison below supersedes that status for `0x20c01010`.
The propulsion callback now has an exact 83-byte output range and a
full linked diagnostic. [Propulsion force collector](sim1-propulsion-collector.md).
The remaining engine update bodies are mapped at the original-code level.

Provisional Ghidra variable suggestions for `FUN_20c20430` are
`param_1` → `step` and `unaff_EDI` → `flightDynamics`, based on
the stack value and instance pointer at `0x20c30d58`. In
`FUN_20c01010`, `param_2` → `thrust`, `unaff_EBX` → `forceSum`,
`unaff_ESI` → `leverArm` and `unaff_EDI` → `momentSum` follow
the two call sites and the observed cross-product terms. Ghidra
names and types were preserved.

Ghidra tags were applied to 23 functions and read back after saving.
Revision `1795530407241:962445d7:143` reported
`unsaved_changes=false`. The dashboard includes the tagged functions,
the original dispatcher listing and the jump table.

- [Ghidra and PE evidence](../../../artifacts/sim1.dll/engine-branches-ghidra.json)
- [Ghidra backup before tagging](../../../artifacts/sim1.dll/ghidra-backups/sim1-before-engine-branch-tags-20261124.gzf)
- [Force collection](sim1-forces-and-moments.md)

## Thrust projection and accumulation

The complete 215-byte linked body of `sim1.dll` `0x20c01010`
matches after validating the four direct call displacements.
The 70-byte accumulation block at `0x20c0109b` through
`0x20c010e0` is exact without normalization. This scope uses a
VC8 RTM `14.00.50727.42` precise LTCG no-entry DLL. Native COFF
and runtime verification for this helper remain open.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c011a6`, `0x20c01281` | The collector puts the force sum in EBX, the lever arm in ESI and the moment sum in EDI. A pointer to two float angles and a thrust double are passed on the stack. The helper returns with `ret 0xc`. | live verified |
| `0x20c01015`–`0x20c01039` | When both angles compare equal to zero, the force components are zero, zero and the original thrust. The x87 tests direct unordered comparisons to projection. | live verified |
| `0x20c0103d`–`0x20c01099` | The projection uses float-rounded trig results, the cosine of the first angle times thrust as the horizontal component, and the sine of the negated first angle for the second force component. | live verified |
| `0x20c0109b`–`0x20c010e0` | Adds the thrust vector and the lever-arm cross product to the force and moment sums. The original and linked instructions are exact across all 70 bytes. | live verified |

| Original call instruction | REL32 field offset from entry | Original thunk | Verified import in both DLLs |
| --- | ---: | --- | --- |
| `0x20c0103f` | `0x30` | `0x20c670a0` | `msvcr80.dll!_CIcos` |
| `0x20c01056` | `0x47` | `0x20c670ac` | `msvcr80.dll!_CIsin` |
| `0x20c0106e` | `0x5f` | `0x20c670ac` | `msvcr80.dll!_CIsin` |
| `0x20c01085` | `0x76` | `0x20c670a0` | `msvcr80.dll!_CIcos` |

The first precise build reproduced the projection and the three force
adds. Twelve bytes still differed because LTCG exchanged ESI and EDI
for the lever arm and moment accumulator. Ordering the two accumulator
parameters together in the reconstructed signature restored the
original register allocation in the second build. The comparison
caller is reconstruction-only. The successful receipt, compiler log,
source snapshot, build command and hashes are retained with the run.

Ghidra kept `FUN_20c01010` and its signature. The tags
`Reconstructed` and `Normalized linked match` were saved and read
back in revision `1795531656408:962445d7:146` with
`unsaved_changes=false`. The dashboard contains the source mapping,
the complete original and rebuilt listings, and the exact inner block.

- [Reconstructed thrust projection](../../../src/sim1/engine_forces.cpp)
- [Original Ghidra listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c01010.asm)
- [Selected linked comparison](../../../artifacts/sim1.dll/thrust-projection-comparison.json)
- [Ghidra and caller evidence](../../../artifacts/sim1.dll/thrust-projection-ghidra.json)
- [First linked build comparison](../../../vm-share/modules/sim1/runs/sim1-thrust-20261124-001/thrust-comparison.json)
- [Force collection regression](../../../vm-share/modules/sim1/runs/sim1-thrust-regression-forces-20261124/forces-comparison.json)
- [Linked position regression](../../../vm-share/modules/sim1/runs/sim1-thrust-regression-position-20261124/position-comparison.json)
- [Native position regression](../../../vm-share/modules/sim1/runs/sim1-thrust-regression-position-coff-20261124/position-comparison.json)
- [State integration regression](../../../vm-share/modules/sim1/runs/sim1-thrust-regression-integration-20261124/state-integration-comparison.json)

## Turbine speed-scaled output

`sim1.dll` `0x20c2bc20` runs after the turbine thrust writer in the
shared turbine update. The call at `0x20c2b8d5` passes the common
`0xec`-byte engine record in ECX and the `0x80`-byte turbine record
on the stack. The helper returns with `ret 4`. This ABI and both
record strides are live verified. The source uses reconstructed
names and minimum layouts.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c2bc25`–`0x20c2bc7d` | Reads the engine double at `+0x5c` and writes it multiplied by 120 for values above 0.5, by 80 for values above 0.2, and by 10 otherwise. The tests use strict comparisons, so unordered input reaches the final multiply. The destination is turbine `+0x50`. | live verified |
| `0x20bebfba` | Reads turbine `+0x50` for the indexed engine query reached by selector 338 of `FUN_20be96e0`. The measurement name and units remain open. | live verified |

The selected VC8 RTM `/O2 /fp:precise` native COFF body has 91
bytes against the original 94. Its first 21 bytes, through the
branch for the 0.5 threshold, match after validating one DIR32
field. This scope also validates that both branches reach the
0.2 threshold in the lower-speed path. All five binary64 literals
were individually checked against the original PE and COFF data.
The original loads the multiplier and multiplies by the engine
field. The rebuilt body loads the engine field and multiplies by
the literal, with differing load placement and destination registers.
The fast profile also produced 91 bytes. The size profile folded the
output paths into a common tail, producing 75 bytes.

Ghidra tags were saved and read back in revision
`1795543912564:962445d7:153` with `unsaved_changes=false`.
Suggested rename `param_1` → `turbineState` follows the indexed
record at the call site. The database retains its current names and
types. The linked and simulator comparisons for this helper remain open.

- [C++ helper](../../../src/sim1/engine_forces.cpp#L162)
- [Native COFF comparison](../../../artifacts/sim1.dll/turbine-speed-output-comparison.json)
- [Original Ghidra listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c2bc20.asm)
- [Ghidra, call-site and literal evidence](../../../artifacts/sim1.dll/turbine-speed-output-ghidra.json)
- [Selected source snapshot, command, and receipt](../../../vm-share/modules/sim1/runs/sim1-turbine-20261124-005)
- [Propulsion collector regression](../../../vm-share/modules/sim1/runs/sim1-turbine-regression-propulsion-20261124/propulsion-comparison.json)
- [Thrust projection regression](../../../vm-share/modules/sim1/runs/sim1-turbine-regression-projection-20261124/thrust-comparison.json)
## Propeller induced airspeed

`sim1.dll:0x20c480b0` calculates the additional axial airspeed at
the propeller disk and publishes it in the mapped engine record.
Its reconstructed symbol is `EngineForceRecord::UpdateInducedAirSpeed`.

| Address or field | Evidence | Status |
| --- | --- | --- |
| `0x20c476e1` | The propeller loop at `0x20c47420` calls the writer with the engine in ESI, the propeller in EDI, the airplane in EDX and a step-record pointer on the stack. The callee returns with `ret 4`. | live verified |
| `0x20c480bd`, `0x20c480dc` | The original compares `IntegrationHistory+0x104` and `PropellerForceRecord+0x14` with zero and uses zero for nonpositive or unordered inputs. The decompiler omits the thrust clamp. | live verified |
| `0x20c480f2`, `0x20c4810c` | The density comes from `FlightEnvironment+0x134`. The step record provides disk area at `+0x20`. The caller computes that slot from `propeller_diameter` at configuration payload `+0x20c` as `diameter × (π/4) × diameter`. | live verified |
| `0x20c4811d`, `0x20c4812d`, `0x20c4813a` | The positive-density path calls `msvcr80.dll!_CIsqrt` and stores the result after subtraction at `EngineForceRecord+0x3c`. Nonpositive or unordered density stores zero. | live verified |
| `0x20c45760` | The tail slipstream update reads the engine induced-speed field. | live verified |
| `0x20c47bd1` | The earlier induced-speed attribution is superseded. The caller sets ESI to the propeller record and this instruction reads its distinct `+0x3c` field. | superseded |

With clamped axial speed `V`, clamped thrust `T`, density `ρ` and
disk area `A`, the positive-density result is
`sqrt(T / (2 × ρ × A) + V² / 4) − V / 2`. The callee does not
test disk area separately. `PropellerStepInputs` is a reconstructed
minimum C++ layout with the verified disk-area field.

The selected VC8 RTM `/fp:precise` COFF replay verifies 72 exact
bytes and 33 normalized bytes across nine nonoverlapping ranges.
The normalization validates both binary64 literals and the CRT
callee individually. The whole function differs (147 original,
158 rebuilt bytes). The C++ member uses the normal `__thiscall`
ABI for its reconstructed interface. The remaining differences
include original interprocedural register assignments, frame
layout, the encoding of density doubling and the epilogues.
The local-speed and `__fastcall` trials are retained for comparison.
The linked projection and vector-publication regressions reproduced
their 215 normalized and 83 exact bytes, respectively. This writer
has no linked match or simulator test. The full propeller step at
`0x20c47420` remains open.

Suggested Ghidra names `dVar1` → `forwardSpeed` for the clamped
local and `unaff_ESI` → `engine` for the verified destination.
These are provisional suggestions. The save at
`1795613240321:62a95bf0:207` contains the three tags and was read
back clean.

- [Original listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c480b0.asm)
- [Live Ghidra evidence](../../../artifacts/sim1.dll/induced-airspeed-ghidra.json)
- [Native COFF comparison](../../../artifacts/sim1.dll/induced-airspeed-comparison.json)
- [Selected build, logs and receipt](../../../vm-share/modules/sim1/runs/sim1-induced-airspeed-20261125-004)
- [Local-speed trial](../../../vm-share/modules/sim1/runs/sim1-induced-airspeed-20261125-001)
- [Direct-field trial](../../../vm-share/modules/sim1/runs/sim1-induced-airspeed-20261125-002)
- [Fastcall trial](../../../vm-share/modules/sim1/runs/sim1-induced-airspeed-20261125-003)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-induced-airspeed-propulsion-20261125-001/propulsion-comparison.json)
