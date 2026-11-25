# Sim1 airplane aerodynamic moment

The airplane force callback `sim1.dll:0x20c45510` writes the result
of the VTable slot `+0x14` to its first moment output component.
The verified table at `0x20bc2160` resolves this slot to
`0x20c013a0`. The C++ reconstruction covers the contributions and
both return paths of that 1,258-byte pitch body.
[Model entry points](sim1-forces-and-moments.md#larger-model-entry-points).

## Inputs and calculation

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c2df8d`, `0x20c0056f` | The airspeed path stores a pressure term computed as half the density factor times relative speed squared at runtime `+0x1dc`. The airplane callback multiplies this by the wing area and geometry `+0x10` before storing the resulting scale at runtime `+0x1f4`. `0x20c40020` binds `wing_area` to geometry `+0x00`. | live verified |
| `0x20c2e120`, `0x20c01414` | The angle update stores the degree value or its timestep-filtered form at runtime `+0x19c`. The moment calculation converts this field to radians when invoking coefficient slot `+0x54`, then multiplies the scalar result by runtime `+0x1f4`. The curve identity is recorded in the angle-dependent coefficient section. | live verified |
| `0x20c47ba0` | Updates each propeller record's signed RPM at `+0x00` using the shaft speed and a sign selected from the propeller state. The record stride is `0x54`. | live verified |
| `0x20be8470`, `0x20c22b28`, `0x20bfb611` | The airplane stores the configuration owner four bytes before the settings payload. The parsed `propeller_moi` at payload `+0x214` and `gyro_precession_on_pitch` at payload `+0x11ac` thus appear at owner offsets `+0x218` and `+0x11b0` in the moment calculation. | live verified |
| `0x20c0153e`–`0x20c016ac` | The gyroscopic term uses the count at runtime configuration `+0x458`. A nonzero control flag at `+0x20` or a gain at `+0x10` that fails the ordered positive comparison gives a zero contribution. | live verified |
| `0x20c015bb`–`0x20c0169e` | For each propeller, multiplies signed RPM by the binary64 constant `0.1047197551`, propeller inertia, flight angular rate `+0x1a8`, the pitch gyro factor, and the gain. Sums the contributions in encounter order. The original has a four-record loop and a remainder loop and stores the subtotal as a double. | live verified |
| `0x20c01876`, `0x20bf68f5` | Adds the gyroscopic subtotal before the force-reference term. The earlier attribution of that add to `0x20c0187c` is superseded. The latter adds the supplemental term. Control `+0x04` holds the normalized `REALISM/SimpleAero` selection for the separate pitch return path. | live verified |

The source defines minimum layouts. The term's field names reflect
the available anchors, while the original Ghidra type information and
function signature were preserved. A provisional Ghidra rename is
`param_2` → `airplane`, based on the ECX model instance and the
airplane pointer passed on the stack at the virtual call. A second
suggestion is `dStack_d8` → `propellerGyroMoment`, based on the
per-propeller sum and its use in the final return expression.

## Comparison scope

The selected Windows XP worker run uses VC8 RTM `14.00.50727.42`
with `/O2 /fp:precise` and emits native x86 COFF. The extracted
source builds to 343 bytes. The corresponding original in-function
slice occupies 336 bytes from `0x20c0155d` through `0x20c016ac`.
Register allocation, branches, and spill placement differ across
that scope. The original binary literal was validated individually
against the relocated COFF constant section. Conversion, gain
checks, and loop behavior remain visible in both listings.

The **20-byte exact range** at `0x20c01687` through `0x20c0169a`
covers multiplication by the angular rate, multiplication by both
gyro factors, accumulation, the remainder-loop back edge, and x87
stack cleanup. The comparator verifies both back-edge destinations
reach the per-propeller load and next-record step. The source retains
the original double-rounding boundary before passing the term back
to the caller. The rest of the 1,258-byte original function, linked
code comparison for this term, and simulator behavior remain open.

Ghidra tags `Aerodynamics` and `Scoped reconstruction` were saved
and read back in revision `1795545337374:962445d7:158` with
`unsaved_changes=false`.

- [C++ reconstruction](../../../src/sim1/aerodynamics.cpp)
- [Native COFF comparison](../../../artifacts/sim1.dll/aerodynamic-moment-comparison.json)
- [Original function listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c013a0.asm)
- [Ghidra and input provenance](../../../artifacts/sim1.dll/aerodynamic-moment-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-aerodynamic-moment-20261124-004)

## Force transfer to the moment reference

All three normal aerodynamic moment paths use the force vector
already computed by the airplane callback. The following labels are
component order in the output vector. The source uses the established
minimum `MassProperties::centerOfGravity` layout. Let `a` denote
`CG.x - highAlphaReferenceXShift`, and `b0` and `b1` denote
`CG.z - referenceZ - highAlphaReferenceZShift` with the corresponding
reference query. The component formulas preserve their original signs.

| Output | Original slice | Force transfer contribution | Status |
| --- | --- | --- | --- |
| X | `0x20c013a0`, `0x20c01733`–`0x20c01774` | `b0 * Fy - CG.y * Fz` | live verified |
| Y | `0x20c020a0`, `0x20c02645`–`0x20c0267f` | `a * Fz - b1 * Fx` | live verified |
| Z | `0x20c01890`, `0x20c02025`–`0x20c0203c` | `CG.y * Fx - a * Fy` | live verified |

The CG fields at mass-properties offsets `+0x24`, `+0x28` and
`+0x2c` are binary32. The force and correction values are binary64.
The Y slice stores the first product as a double before the
reference query and stores the difference before aggregation. The
alternate return paths selected by the control field at
`(*(FlightDynamics+0x2d8))+4` omit all three force corrections.
The X and Y slices may calculate the term before the branch.
The extracted C++ functions represent the force-dependent subtotal.

Airplane VTable `0x20bbecd0` slots `+0x11c` and `+0x120`
resolve to the tail adapters `0x20be8420` and `0x20be8440`.
They forward the configuration model at owner offset `+0x19fc`
to child slots `+0x1c` and `+0x20` of table `0x20bc213c`.
Their concrete targets `0x20c454f0` and `0x20c45500` load
binary32 fields at `+0x1404` and `+0x1408` into ST0 and return
without stack arguments. The two caller sites and getter signatures
were checked independently. `0x20c50f70` initializes both fields
using configuration geometry and coefficient samples. The names
`firstMomentReferenceZ` and `secondMomentReferenceZ` describe
their role in these slices. The meaning of all initializer inputs
and sentinel cases remains open.

The runtime correction values at `0x20c7b92c` and `0x20c7b93c`
come from `0x20c00530`, which runs before the force and moment
queries. Its model-dependent branch uses a filtered angle, table
values, geometry and the settings `hi_alpha_on_roll` and
`hi_alpha_on_yaw`. Its other branch clears the correction values.
The corresponding COFF operands were checked as external DIR32
references to the two reconstructed storage symbols.

### High-alpha reference correction

The second block of `sim1.dll:0x20c00530` starts at `0x20c005f3`.
When airplane slot `+0x50` is nonzero, it converts the filtered
alpha from runtime `+0x19c` to radians and divides by the reference
angle returned by configuration slot `+0x10`. Both lookups at
`0x20c00671` and `0x20c006d5` pass this relative angle to
`0x20bcbf60`. The source names below describe the verified axes.

| Relative alpha | Longitudinal curve | Vertical curve |
| ---: | ---: | ---: |
| 0 | 0 | 0 |
| 0.5 | 0 | 0 |
| 0.83 | 0.5 | -0.08 |
| 1 | 0.73 | -0.26 |
| 1.25 | 1.4 | -1 |

`0x20c00676` and `0x20c006da` store the raw curve results to
`0x20c7b92c` and `0x20c7b93c`. `0x20c006ea` clears the middle
component at `0x20c7b934`. The later block (`0x20c006f2` through
`0x20c00754`) obtains rudder deflection from control offset `+0xe4`
and `rudder_limit` from the control owner's geometry `+0x144`.
It scales both saved results by `-(rudder / rudder_limit) *
hi_alpha_on_roll * hi_alpha_on_yaw * realismWeight`. A zero limit
substitutes zero for the ratio. `0x20bfb330` binds the settings
at owner offsets `+0x1194` and `+0x119c`. The control object's
owner supplies its binary32 realism weight at `+0x38`. A zero
result from airplane slot `+0x50` instead clears all three globals
at `0x20c0075b`. These findings are live verified.

The VC8 native COFF comparison covers the 386-byte original block
and the extracted 448-byte source body. Eleven disjoint ranges
match, with 5 exact bytes and 70 bytes after address validation.
The x87 scheduling, stack layout and ABI differ. The scoped Ghidra
tags were saved and read back clean at revision
`1795610815925:62a95bf0:9`. Both arithmetic blocks of the original
function now have source reconstructions. A whole-body match and
in-simulator verification remain open.

- [Ghidra and curve evidence](../../../artifacts/sim1.dll/high-alpha-reference-ghidra.json)
- [Native COFF comparison](../../../artifacts/sim1.dll/high-alpha-reference-comparison.json)
- [Build, snapshots and receipt](../../../vm-share/modules/sim1/runs/sim1-high-alpha-reference-20261125-001)

### Force-reference comparison

The precise VC8 RTM build produces 67, 84 and 33 native COFF bytes
for the three extracted functions. The original in-function slices
are 66, 59 and 24 bytes. The diagnostic retains frame, register and
instruction-order differences. Four disjoint arithmetic ranges match.
The X correction and multiply normalize across 9 bytes. In Z,
the first product setup and final subtract match across 8 and
5 exact bytes, and the longitudinal correction normalizes across
6 bytes. The Y comparison verifies both correction operands and
the original virtual query, with arithmetic code differences
remaining. A fast-profile trial retained the same results. The
existing 20-byte exact gyro range also reproduced in the selected build.

Ghidra tags for all three moment functions were read back clean at
revision `1795546154148:962445d7:163`. Provisional Ghidra names
`param_3` → `aerodynamicForce` and `fVar44` →
`firstMomentReferenceZ` follow the first component's two verified
inputs. The original Ghidra names and signatures were preserved.
The remaining aerodynamic coefficient terms and simulator behavior
are still open.

- [C++ force transfer calculation](../../../src/sim1/aerodynamics.cpp#L305)
- [Native COFF comparison](../../../artifacts/sim1.dll/moment-arm-comparison.json)
- [Ghidra and reference dispatch evidence](../../../artifacts/sim1.dll/moment-arm-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-moment-arm-20261124-002)
- [Fast profile comparison](../../../vm-share/modules/sim1/runs/sim1-moment-arm-20261124-003/moment-arm-comparison.json)

## Angle-dependent moment coefficient

The concrete coefficient view retrieved by the airplane moment
callback resolves slot `+0x54` to the curve lookup at
`sim1.dll:0x20c4fcb0`. This resolves the previously open identity
for the angle-dependent term. The lookup uses an AIR table loaded
separately from the baseline coefficient supplied by slot `+0x58`.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c14e50`, `0x20c4f350` | The airplane model's `+0x20` getter reads the view at model offset `+0x0c`. The constructor installs table `0x20bc2618` on this view and sets its data pointer at `+0x1c`. | live verified |
| `0x20c013f7`, `0x20c0141d` | The model calls slot `+0x58` for a separate baseline, and slot `+0x54` with the angle converted from degrees to radians. Each return is multiplied by runtime `+0x1f4`, the pressure and geometry factor, and stored as a double before the full moment sum. | live verified |
| `0x20c4fcb0`, `0x20c4fcce` | The `+0x54` target receives the view in ECX and a stack double, passes its data block `+0x1104` in ESI to `0x20bcbf60`, and leaves the coefficient in ST0. The caller uses that result immediately. The target ends with `ret 8`. | live verified |
| `0x20c50f8a`, `0x20c50ff1`, `0x20c51027` | The loader clears 756 bytes at data `+0x1104`, reads AIR record `0x1546` into that buffer, and tries `0x473` if the first lookup fails. The adjacent table at `+0xe10` is separately loaded from `0x1545` or `0x404`. | live verified |
| `0x20bcbf60` | The curve starts with a 32-bit count and contains consecutive pairs of doubles at stride 16. The lookup returns zero for an absent or empty table, clamps beyond the endpoints, searches the ordered sample angles, and linearly interpolates adjacent coefficients. The specific 756-byte allocation can hold 47 pairs. | live verified |

The original input at runtime `+0x19c` is derived from the
air-relative velocity angle calculated at `0x20c2e0c9`.
The calculation uses the longitudinal and negated vertical
components through `atan2`, then converts to degrees and
applies the timestep-dependent filter on the applicable path.
The scoped reconstruction preserves the radians conversion,
the AIR-table interpolation and the subsequent double store.
A provisional variable suggestion in `FUN_20c013a0` is
`piVar29` → `coefficientModel`, based on the verified view getter.
`unaff_ESI` → `curve` in `FUN_20bcbf60` follows the caller ABI.
The Ghidra names and signatures were preserved.

The selected precise VC8 RTM native COFF build retains an exact
**16-byte interpolation range** at `0x20bcc013`–`0x20bcc022`.
It performs the interval division and combines the coefficient
difference with the lower sample. The whole lookup is 199 bytes
in the original and 204 in the rebuild. The original takes the
curve in ESI. The native reconstruction uses ECX and different
search and epilogue code. For the extracted moment contribution,
the conversion at `0x20c01414` matches across **6 normalized bytes**
with the binary64 constant individually verified in the PE and
COFF. The 45-byte original caller slice uses the concrete virtual
lookup path. The 84-byte standalone contribution has its own
entry, direct lookup call and return. The separate baseline is
covered under [Zero-alpha moment coefficient](#zero-alpha-moment-coefficient).

Ghidra revision `1795546801864:962445d7:167` was saved and read
back with `unsaved_changes=false`. The dashboard contains both
complete listings for the lookup and the scoped caller comparison.
The prior exact gyro and force-transfer scopes reproduced in the
selected build. Simulator verification remains open.

- [C++ curve and moment contribution](../../../src/sim1/aerodynamics.cpp#L336)
- [Native COFF comparison](../../../artifacts/sim1.dll/angle-curve-comparison.json)
- [Ghidra and AIR record evidence](../../../artifacts/sim1.dll/angle-curve-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-angle-curve-20261124-003)

### Alpha filter

The live listing for `sim1.dll:0x20c2e0b0` shows a first-order
low-pass filter using forward Euler. At `0x20c2e10b` through
`0x20c2e120`, the update is
`alphaFiltered += 8.0 * stepSeconds * (alphaRaw - alphaFiltered)`.
Both alpha values are in degrees at this stage. The current angle
is at runtime `+0x194` and the retained angle at `+0x19c`. The
gain 8.0 is verified from the original PE double at `0x20bc3a38`.
A step of 1/60 s gives the current alpha a weight of 2/15. The
continuous-time equation approximated by this recurrence has a
time constant of 0.125 s.

`0x20c30cbe` passes the flight step unchanged. The airplane
VTable slot `+0x50` resolves to `0x20bd0c60`, which reads the
instance field at `+0xb4`. When that field is zero the update
stores the raw alpha directly. The initialization path at
`0x20bfaf80` uses a zero step and explicitly copies the raw
angle into the history at `0x20bfaf8d`.
This verification used read-only Ghidra and the pinned PE.
[Alpha filter evidence](../../../artifacts/sim1.dll/alpha-filter-ghidra.json).

## Zero-alpha moment coefficient

The concrete coefficient view supplies the baseline from slot
`+0x58`, whose relocated entry at `0x20bc2670` points to
`sim1.dll:0x20c4ff30`. The function takes the view in ECX,
has no stack arguments and returns an x87 scalar in ST0.
The enclosing moment callback scales the result before adding it
to the sum.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c4f350`, `0x20c4ff30` | The coefficient view stores the airplane, runtime, owner configuration and coefficient data pointers at `+0x10`, `+0x14`, `+0x18` and `+0x1c`. A missing data pointer returns zero. The base binary64 coefficient is at data `+0x6c`. | live verified |
| `0x20c51290`, `0x20c519a0`–`0x20c519b6` | The loader reads the base from AIR record `0x1539` or from the corresponding field of legacy record `0x1101` scaled by `1/2048`. It clears 34 bytes at data `+0x5b8` before reading AIR record `0x433` into that region. The baseline calculation reads consecutive signed 16-bit samples there. | live verified |
| `0x20c2dca0`, `0x20c45700`, `0x20c4f240`, `0x20c4f270` | The runtime publishes air-relative speed divided by the speed of sound at `+0x15c`. The coefficient view receives that Mach value through slot zero and stores the index at `+0x04` and fraction at `+0x08`. For finite inputs the producer truncates the clamped product `Mach * 20480` to `[0, 65535]`, divides by 4096 and truncates again to get the index. The fraction uses the adjacent knots at `0x20bc25c0`. The last interval spans 4095 units. The x87 control word selects truncation for both conversions, resolving the decompiler's generic `ROUND` display. | live verified |
| `0x20c4ff5b`–`0x20c4ffa7`, `0x20bd0c60` | A nonzero result from the airplane query at slot `+0x50` enables the Mach and provider correction branch. The signed samples are linearly interpolated, multiplied by the binary64 constant `0.0004885197851` at `0x20bb18e8` and added to the base. The result is stored to a double before the modifier callback. | live verified |
| `0x20c45990`, `0x20c4ffb9`–`0x20c4ffd7` | Runtime `+0x250` holds an optional provider registered under `0xd07e`. Slot `+0x18` accepts the provider pointer on the stack, cleans that argument and returns a scalar in ST0. The coefficient is multiplied by `1 - 0.3 * (1 - value)` when present, with the binary64 literal at `0x20bc3658`. The missing provider uses a factor of one. The provider's physical meaning remains open. | live verified |
| `0x20bf6890`, `0x20c42940`, `0x20c4ffdd`–`0x20c50007` | Control pointer `airplane+0x2d8` leads through its first field to the flight-dynamics owner and its float `r` at `+0x38`. Configuration `+0x19c4` points to the double pair loaded from `RealismConstants` / `PitchMomentZeroAlpha`. Loader payload `+0x19c0` maps to this same owner field. Defaults are scale 1 and bias 0. The blend is `(scale + (1 - scale) * r) * coefficient + (1 - r) * bias`. | live verified |
| `0x20c013f7`–`0x20c0140a` | The original caller invokes slot `+0x58`, multiplies by the pressure and geometry factor at runtime `+0x1f4` and stores the result as a double. | live verified |

The source uses minimum reconstructed views for the coefficient data,
the coefficient model, the realism pair and the optional provider.
The provider dispatch structure represents the observed stack-based
callback ABI. The Mach index producer was traced and remains a
separate source reconstruction target. The suggested Ghidra parameter
rename `param_1` → `coefficientModel` follows the ECX argument and
verified view offsets. No Ghidra symbol or type edit was applied.

The selected precise VC8 RTM native COFF build has three disjoint
matches. The **20-byte blend** at `0x20c4fff4` is exact. The
**6-byte Mach scale** at `0x20c4ff99` and **16-byte modifier
calculation** at `0x20c4ffc3` each normalize after verifying
one original PE address and the corresponding COFF binary64 data.
The 220-byte original and 207-byte rebuilt body still differ in
stack layout and operand registers. The explicitly aligned local
trial produced 232 bytes with the same matched ranges. The
37-byte original caller slice and 57-byte extracted C++ function
also differ. The original uses virtual dispatch and reads the
pressure and geometry factor through EDI. The standalone build uses
a checked direct call and reads the same offset through ESI.

The selected build reproduced the prior gyro, lever-arm and angle
comparisons. Fresh linked checks reproduced the 215-byte normalized
thrust helper, the 83-byte exact propulsion copy and the existing
force-accumulation and acceleration scopes. The runtime and linked
comparisons for the new baseline remain open. Ghidra tags and
saved revision `1795548514295:962445d7:174` were read back clean.

- [C++ zero-alpha moment coefficient](../../../src/sim1/aerodynamics.cpp#L397)
- [Original baseline listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c4ff30.asm)
- [Original caller listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c013a0.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/baseline-moment-comparison.json)
- [Ghidra and loader evidence](../../../artifacts/sim1.dll/baseline-moment-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-baseline-moment-20261124-001)
- [Selected build disassembly](../../../vm-share/modules/sim1/runs/sim1-baseline-moment-20261124-001/worker-output/disassembly.txt)
- [Explicit alignment trial](../../../vm-share/modules/sim1/runs/sim1-baseline-moment-20261124-002)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-baseline-propulsion-20261124-001)
- [Linked forces regression](../../../vm-share/modules/sim1/runs/sim1-baseline-forces-20261124-001)

## Elevator contribution

The concrete coefficient view at VTable `0x20bc2618` supplies the
elevator coefficient through slot `+0x5c`. The relocated entry at
`0x20bc2674` targets `sim1.dll:0x20c4fce0`. The target receives the
view in ECX, takes no stack arguments and returns the coefficient
in ST0. It returns zero when coefficient data is missing.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c45c90`, `0x20be83e0`, `0x20c01429`–`0x20c0144b` | The moment model constructor stores the airplane owner at `+4`. Its concrete airplane table entry at `0x20bbede8` resolves slot `+0x118` to a getter for airplane `+0x334`. The caller obtains that control-surface block and reads `+0xcc` before invoking the elevator coefficient slot. | live verified |
| `0x20c04f50`, `0x20c05f70`, `0x20c419c0`, `0x20c41a30`, `0x20c41bf0` | The control block holds elevator deflection at `+0xcc` and elevator trim at `+0xfc`. The writers use the configured `elevator_up_limit`, `elevator_down_limit` and `elevator_trim_limit`. The geometry readers convert those settings from degrees to radians. | live verified |
| `0x20c51290`, `0x20c51928`–`0x20c5193e`, `0x20c4fd00`–`0x20c4fd4a` | The base coefficient is at data `+0x54`, loaded from AIR `0x1539` or from the scaled legacy field in AIR `0x1101`. When airplane slot `+0x50` returns nonzero, the function interpolates signed 16-bit samples from AIR `0x420` at data `+0x530` and adds their result scaled by `0.0004885197851`. | live verified |
| `0x20c4fd59`–`0x20c4fd7b`, `0x20c51290` | Subtracts the absolute trim times the coupling value at data `+0xac`. The loader derives this value from the elevator and trim coefficients and geometry limits when the trim limit is positive. It stores zero when this calculation is skipped. | live verified |
| `0x20c4fd96`, `0x20c4fdb8`, `0x20c51bac`, `0x20c51cba` | Applies two linearly interpolated factors through `0x20bcbf60`. AIR `0x341` at data `+0xcb4` takes the signed elevator deflection in radians. AIR `0x536` at data `+0xb10` takes raw alpha at runtime `+0x194` in degrees. The 116-byte and 84-byte buffers can hold seven and five samples. The common loader defaults to the factor 1. The original rounds to a double after the trim subtraction and after applying the first curve. | live verified |
| `0x20bfb330`, `0x20c4fdc4`–`0x20c4fde3` | Multiplies by `0.9 + 0.09999999999999998 * realismWeight` and by `elevator_effectiveness` from the `flight_tuning` payload at owner `+0x114c`. Its configuration default is 1. | live verified |
| `0x20c0144d`–`0x20c01461` | The caller multiplies the coefficient by signed deflection and the pressure and geometry factor at runtime `+0x1f4`, then stores the contribution as a double. | live verified |

The source uses minimum reconstructed control and table layouts.
Both new tables reuse the existing interpolation routine, with their
verified array sizes sharing the same four-byte count header.
`param_1` → `coefficientModel` is a suggested Ghidra rename based on
ECX and the verified table and runtime offsets. The saved Ghidra
revision `1795549312928:962445d7:178` was read back clean with
`Aerodynamics`, `COFF differences` and `Reconstructed`.

The selected precise VC8 RTM native COFF build contains **20 exact
bytes** across the coefficient's deflection load and the caller's
control-surface fetch and pressure multiplier. Three normalized
ranges total **30 bytes**, covering the Mach multiplier and
the realism and configured effectiveness calculation. Each range
checks its own binary64 reference. The original 269-byte
coefficient body and the 250-byte rebuild differ in stack layout,
register allocation and the lookup ABI. The original passes the
curve pointer in ESI. The reconstruction passes it in ECX.
The 279-byte explicit stack-alignment trial loses the longest
normalized range. The 42-byte original caller slice and its
89-byte standalone reconstruction also differ, including the
virtual coefficient call versus the verified direct COFF call.

The prior gyro, lever-arm, angle-curve and baseline comparisons
reproduced in the selected build. The linked force accumulation
and acceleration comparison also retained its established results.
Simulator verification remains open.

- [C++ elevator coefficient and moment](../../../src/sim1/aerodynamics.cpp#L442)
- [Original coefficient listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c4fce0.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/elevator-moment-comparison.json)
- [Ghidra and AIR evidence](../../../artifacts/sim1.dll/elevator-moment-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-elevator-moment-20261124-003)
- [Selected build disassembly](../../../vm-share/modules/sim1/runs/sim1-elevator-moment-20261124-003/worker-output/disassembly.txt)
- [Explicit alignment trial](../../../vm-share/modules/sim1/runs/sim1-elevator-moment-20261124-002)
- [Linked forces regression](../../../vm-share/modules/sim1/runs/sim1-elevator-forces-20261124-001)

## Pitch-rate contribution

Slot `+0x60` of the concrete coefficient view at VTable
`0x20bc2618` resolves through its relocated entry `0x20bc2678`
to `sim1.dll:0x20c4fe00`. The function receives the view in ECX,
takes no stack arguments and returns an x87 scalar in ST0. It
returns zero when there is no coefficient data.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c51290`, `0x20c51964`–`0x20c5197a`, `0x20c4fe25`–`0x20c4fe88` | The base coefficient at data `+0x64` comes from AIR `0x1539` or the corresponding signed legacy value from AIR `0x1101` scaled by `1/2048`. The loader clears the 34 bytes at data `+0x574` and reads AIR `0x422` there. The coefficient calculation interpolates adjacent signed 16-bit Mach samples and adds the correction scaled by `0.0004885197851`. | live verified |
| `0x20c51c01`–`0x20c51c12`, `0x20c4fe3a`–`0x20c4fe51` | The coefficient also multiplies by the AIR `0x539` curve evaluated at the raw angle of attack in degrees from runtime `+0x194`. The curve begins at data `+0xc0c`, with an 84-byte allocation, five-sample capacity and default factor 1. The lookup is `0x20bcbf60`. | live verified |
| `0x20bfb330`, `0x20c4fe8a`–`0x20c4fead` | The coefficient is multiplied by `1 + 0.15 * (1 - realismWeight)` and by `pitch_stability`, loaded from `flight_tuning` payload `+0x1178` at configuration owner `+0x117c`. The latter defaults to 1. The product is rounded to double before the control-surface query. | live verified |
| `0x20c4feb1`–`0x20c4fef3` | The airplane's control-surface getter supplies the provider at control block `+0x0c`. Its slot `+0x18` accepts the provider pointer on the stack and returns an x87 scalar. The function rounds `loss = 1 - value` to single precision and uses `loss = 1` without the provider. The factor is `1 - 0.15 * loss`. The product is then rounded to double. | live verified |
| `0x20c45990`, `0x20c4fee5`–`0x20c4ff26` | The distinct runtime `+0x250` modifier registered as `0xd07e` supplies a factor of `1 - (1 - value)`, with both subtractions present in the original x87 code. The absent provider gives a factor of one. The physical meaning of both provider results remains open. | live verified |
| `0x20c43110`, `0x20c4342d`, `0x20c01464` | The angular integrator publishes its first component to runtime `+0x134`. The moment callback multiplies the coefficient by this signed rate. | live verified |
| `0x20c2df37`, `0x20c00575`–`0x20c005b7`, `0x20c01471`–`0x20c0147a` | The aerodynamic scale at runtime `+0x760` is one quarter of the air-density factor times air-relative speed, wing area and the square of the geometry chord at `+0x10`. The moment callback multiplies by this scale and stores the contribution as a double. | live verified |

The layouts for the rate coefficient, AIR tables and modifier
providers are minimum reconstructions. `param_1` →
`coefficientModel` is a suggested Ghidra parameter rename based
on ECX and the verified coefficient view fields. Ghidra tags
`Aerodynamics`, `COFF differences` and `Reconstructed` were
saved and read back clean at revision `1795550031888:962445d7:182`.

The precise VC8 RTM build gives **14 exact bytes** at
`0x20c4febc` for the control modifier call and complement.
The Mach multiplier at `0x20c4fe7f`, realism arithmetic at
`0x20c4fe8f` and control loss multiplier at `0x20c4fedf`
normalize across **6, 18 and 6 bytes**. Each match verifies
an original PE literal address and the rebuilt COFF literal
contents. The selected 270-byte body preserves the single-precision
store with a typed float local. The 276-byte first trial used a
volatile float local and retained the same meaningful scopes.
The complete 295-byte original differs in its stack frame and
register allocation. The curve lookup takes the table in ESI
in the original and ECX in the reconstructed source.

The enclosing 25-byte original caller slice and 63-byte
standalone rebuild differ. The rebuild uses a verified direct call
to the concrete coefficient and reads the two runtime fields through
ESI instead of the original EDI. All established aerodynamic
comparisons reproduced. The separate native integration run
retained the three transform bodies and the exact 144-byte
history-copy range. Simulator verification remains open.

- [C++ pitch-rate coefficient and moment](../../../src/sim1/aerodynamics.cpp#L487)
- [Original coefficient listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c4fe00.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/pitch-rate-comparison.json)
- [Ghidra and rate producer evidence](../../../artifacts/sim1.dll/pitch-rate-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-pitch-rate-20261124-002)
- [Selected build disassembly](../../../vm-share/modules/sim1/runs/sim1-pitch-rate-20261124-002/worker-output/disassembly.txt)
- [Volatile local trial](../../../vm-share/modules/sim1/runs/sim1-pitch-rate-20261124-001)
- [Native integration regression](../../../vm-share/modules/sim1/runs/sim1-pitch-rate-integration-20261124-001)

## Angle-of-attack rate contribution

Slot `+0x68` of the airplane coefficient view at VTable
`0x20bc2618` resolves through its relocated entry at `0x20bc2680`
to `sim1.dll:0x20c50020`. The function receives the coefficient
view in ECX and returns an x87 scalar in ST0 without stack
arguments. It returns zero when coefficient data is absent.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c2dca0`, `0x20c2e0bb`–`0x20c2e0d4` | The airflow update publishes the longitudinal component at runtime `+0x104` and the vertical component at `+0x114`. The raw angle is `atan2(-vertical, longitudinal)`. | live verified |
| `0x20c2e1e2`–`0x20c2e1e9`, `0x20bfb840`–`0x20bfb88f` | The angle update passes the airplane in EDX to the two-rate function. For positive planar speed squared, the alpha-rate numerator is longitudinal acceleration `airplane+0x1c8` times vertical airflow minus vertical acceleration `airplane+0x1c0` times longitudinal airflow. The quotient divides by the sum of the two squared airflow components and is stored at runtime `+0x1cc`. Nonpositive planar speed squared gives zero. | live verified |
| `0x20bfb860`–`0x20bfb8db`, `0x20bc3fb0`, `0x20bbf2b0` | The estimate is limited to `±0.4363323129985824` rad/s, equivalent to `±25°/s`. The binary64 limits were checked separately. The following computation at `0x20bfb8dc` writes the separate sideslip rate at runtime `+0x1d4`. | live verified |
| `0x20c51290`, `0x20c51946`–`0x20c5195c`, `0x20c5003c`–`0x20c50086` | The base coefficient at data `+0x74` is loaded from AIR `0x1539` or the corresponding signed legacy field in AIR `0x1101` scaled by `1/2048`. The loader clears 34 bytes and reads AIR `0x421` into data `+0x552`. A nonzero airplane mode query at slot `+0x50` enables the interpolated signed Mach correction scaled by `0.0004885197851`. The adjusted coefficient is stored to double. | live verified |
| `0x20c51be3`–`0x20c51bf4`, `0x20c5008d`–`0x20c500ab` | The adjusted coefficient is multiplied by the AIR `0x538` curve at data `+0xbb8`, sampled using raw alpha in degrees from runtime `+0x194`. The common loader defines an 84-byte allocation, a five-sample capacity and a default factor of one. | live verified |
| `0x20c0147b`–`0x20c01493` | The moment callback multiplies the coefficient by the signed angle-of-attack rate and the aerodynamic rate scale at runtime `+0x760`, then stores the result as a double. | live verified |

The source reconstructs the alpha output of the 352-byte two-rate
function. The extracted method receives the runtime object in ECX
and the airplane in EDX. The original obtains the runtime object
from the airplane. Suggested Ghidra names are `param_1` →
`coefficientModel` in `FUN_20c50020` and `param_2` → `airplane` in
`FUN_20bfb840` based on the verified registers and fields. Ghidra
symbols and signatures were preserved. Both sets of tags were saved
and read back clean at revision `1795550562132:962445d7:188`.

In the selected precise VC8 RTM comparison, **62 exact bytes** of
the rate producer cover the speed-squared and zero comparison,
the numerator and the store and absolute-value sequence. The
two limit loads contribute **12 normalized bytes**. In the
coefficient, four exact ranges total **26 bytes**, covering
the base and airplane reads, mode query, signed Mach difference
and raw alpha load. The Mach multiply adds **6 normalized bytes**.
Each reference was verified against the original PE and the
rebuilt COFF data.

The complete coefficient body is 144 original and 135 rebuilt
bytes. The trial with an explicit volatile copy produced 140
rebuilt bytes with the same matched ranges. The selected build
preserves the double store before the curve call. Stack layouts
and lookup argument registers differ. The alpha-rate scope is
156 original and 132 rebuilt bytes. Division stack positions
and branches differ, with the original retaining state for the
sideslip calculation. The extracted caller is 63 bytes against
25 original in-function bytes and uses a verified direct call
and ESI for runtime fields. The previous aerodynamic and native
integration checks reproduced. Simulator verification remains open.

- [C++ angle-of-attack rate coefficient and producer](../../../src/sim1/aerodynamics.cpp#L532)
- [Original coefficient listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c50020.asm)
- [Original two-rate listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20bfb840.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/alpha-rate-comparison.json)
- [Ghidra and AIR evidence](../../../artifacts/sim1.dll/alpha-rate-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-alpha-rate-20261124-002)
- [Selected build disassembly](../../../vm-share/modules/sim1/runs/sim1-alpha-rate-20261124-002/worker-output/disassembly.txt)
- [Volatile local trial](../../../vm-share/modules/sim1/runs/sim1-alpha-rate-20261124-001)
- [Native integration regression](../../../vm-share/modules/sim1/runs/sim1-alpha-rate-integration-20261124-001)

## Horizontal-tail incidence contribution

Slot `+0x64` of the airplane coefficient view at VTable
`0x20bc2618` resolves through the relocated entry at `0x20bc267c`
to `sim1.dll:0x20c500c0`. The function takes the view in ECX
without stack arguments and returns an x87 scalar in ST0. The
result is zero when the coefficient data is missing.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c013c2`, `0x20c3ff30`, `0x20c40730`, `0x20c41790` | Configuration owner `+0x1980` points to the airplane geometry. Geometry `+0xcc` holds `airplane_geometry/htail_incidence` in radians. The configuration reader converts degrees to radians. The geometry loader can supply the value from legacy AIR record `0x1205` when the setting is absent. | live verified |
| `0x20c51290`, `0x20c51982`–`0x20c51998`, `0x20c500de`–`0x20c50128` | The base coefficient is stored at data `+0x7c` and comes from AIR `0x1539` or the corresponding signed field in legacy AIR `0x1101` scaled by `1/2048`. When the airplane query at slot `+0x50` returns nonzero, the function interpolates signed samples from AIR `0x423` at data `+0x596`, scales the result by `0.0004885197851` and adds it to the base. | live verified |
| `0x20c51bc5`–`0x20c51bd6`, `0x20c5012f`–`0x20c50158` | The coefficient is multiplied by the AIR `0x537` curve at data `+0xb64` using raw alpha in degrees. The table loader defines 84 bytes, enough for five samples, and initializes the default factor to one. The original stores the curve product as a double before calling the control-surface getter. | live verified |
| `0x20c5015c`–`0x20c50189` | The getter returns the block containing the optional provider at `+0x0c`. The provider's `+0x18` callback receives the provider on the stack and returns a scalar in ST0. The loss is the single-precision value of `1 - returnValue`, with a default loss of one. The coefficient is multiplied by `1 - 0.15 * loss`. The physical quantity returned by the provider remains open. | live verified |
| `0x20c01494`–`0x20c014bb` | The caller multiplies the coefficient by the signed horizontal-tail incidence and the pressure and geometry factor at runtime `+0x1f4`, then stores the result as a double. The next coefficient slot `+0x6c` resolves to `0x20c501a0` and its result is multiplied by the control block's elevator trim at `+0xfc`. | live verified |

The source uses minimum reconstructed views of the airplane
geometry and aerodynamic tables. A suggested Ghidra rename in
`FUN_20c500c0` is `param_1` → `coefficientModel`, based on the
verified ECX receiver and its data and state fields. Names and
signatures were preserved. Ghidra tags were saved and read back
at revision `1795565103483:d6cfccaa:7`.

The selected precise VC8 RTM object gives **49 exact bytes** across
five scopes. The largest is the **25-byte modifier selection** at
`0x20c5015c`, covering both branches through the join at the
single-precision store. The Mach multiplier and modifier arithmetic
normalize across **6 and 10 bytes**, with each binary64 reference
verified against the retained original and COFF data.

The complete coefficient differs across 209 original and 193
rebuilt bytes. Frame layout and the curve argument register differ.
The caller scope is 40 original and 77 rebuilt bytes. The extracted
caller uses a checked direct call and a different geometry pointer
register. Its pressure and geometry multiply matches exactly.
Both source variants produce identical body bytes, including the
two original rounding boundaries. The selected source uses a typed
local for the alpha-adjusted coefficient. Earlier aerodynamic
comparisons and the linked forces regression reproduced. Simulator
verification remains open.

- [C++ horizontal-tail incidence calculation](../../../src/sim1/aerodynamics.cpp#L587)
- [Original coefficient listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c500c0.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/tail-incidence-comparison.json)
- [Ghidra and geometry evidence](../../../artifacts/sim1.dll/tail-incidence-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-tail-incidence-20261125-002)
- [Selected build disassembly](../../../vm-share/modules/sim1/runs/sim1-tail-incidence-20261125-002/worker-output/disassembly.txt)
- [Volatile local trial](../../../vm-share/modules/sim1/runs/sim1-tail-incidence-20261125-001)
- [Linked forces regression](../../../vm-share/modules/sim1/runs/sim1-tail-incidence-forces-20261125-001)

## Elevator trim contribution

Slot `+0x6c` of the airplane coefficient view at VTable
`0x20bc2618` resolves through its relocated entry at `0x20bc2684`
to `sim1.dll:0x20c501a0`. The target takes the view in ECX with
no stack arguments and returns an x87 scalar in ST0. Missing
coefficient data gives zero.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c05f70`, `0x20c41bf0`, `0x20c014be` | The caller reads signed elevator trim from control block `+0xfc`. The control writer limits it using `elevator_trim_limit` at geometry `+0x14c`. The configuration reader converts the limit from degrees to radians. | live verified |
| `0x20c51290`, `0x20c51bac`–`0x20c51bb8`, `0x20c501c5`–`0x20c501d7` | The base at coefficient data `+0x5c` is loaded from AIR `0x1539` or the corresponding unscaled binary64 field in legacy AIR `0x1101`. AIR `0x536` supplies the same curve at data `+0xb10` used for elevator deflection. Its input is raw alpha in degrees at runtime `+0x194`. | live verified |
| `0x20bfb330`, `0x20c501df`–`0x20c501f6` | The function multiplies the base by `flight_tuning/elevator_trim_effectiveness` and the alpha factor. The tuning default is one. The loader writes payload `+0x1160`, which is owner `+0x1164`. The original stores this product as a double. | live verified |
| `0x20c2df8d`, `0x20c501ee`–`0x20c50219` | A second interpolation uses the curve at configuration owner `+0x57c` and dynamic pressure at runtime `+0x1dc`. The airflow update computes this pressure as one half of the air-density factor times air-relative speed squared. The second curve product is stored as a double before the control-surface query. The pressure curve writer and maximum sample count remain open. | live verified |
| `0x20c06d5a`–`0x20c06d98`, `0x20c5021d`–`0x20c50261` | The control block `+0x0c` provider comes from the query using identifier `0xd098` and GUID at `0x20bbe6a8`. Its `+0x18` callback accepts the provider on the stack and returns a scalar in ST0. The coefficient is multiplied by `1 - float(1 - value)` when the provider exists. The absent path substitutes a loss of one, producing a zero multiplier even though the original still performs the multiplication. The provider's physical output remains open. | live verified |
| `0x20c014cf`–`0x20c014e5` | The caller multiplies the coefficient by the signed trim and the pressure and geometry factor at runtime `+0x1f4`. It stores the result as a double in the moment sum. | live verified |

The source uses minimum layouts and reuses the existing curve
lookup. The configuration layout represents the verified count
header of the pressure curve. The capacity remains unspecified.
`param_1` → `coefficientModel` is a suggested Ghidra rename based
on the ECX receiver and verified coefficient data and configuration
fields. The original name and signature were preserved. Ghidra
tags were saved and read back at revision
`1795565874672:d6cfccaa:11`.

Five ranges total **35 exact bytes**, covering the provider
query and arithmetic, the signed trim read and the caller's
pressure and geometry multiplication. Two ranges total **16
normalized bytes** at `0x20c501d4` and `0x20c50200`. Each
checks the stack argument store and the relocation to `0x20bcbf60`
with the curve and input offsets identified separately. The pinned
original has 194 bytes and the selected native VC8 build has 176.
The 187-byte explicit-alignment trial reproduces the same scopes.
The full bodies differ in frame, register allocation and the
curve pointer register, with the original using ESI and the rebuild
using ECX. Both coefficient rounding points and the provider's
float store appear in the selected assembly.

The extracted caller comparison covers 42 original and 89 rebuilt
bytes. Its rebuild calls the verified concrete coefficient target
directly. The established aerodynamic and native integration
comparisons reproduced. Simulator verification remains open.

- [C++ trim coefficient and moment](../../../src/sim1/aerodynamics.cpp#L627)
- [Original coefficient listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c501a0.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/elevator-trim-comparison.json)
- [Ghidra and provider evidence](../../../artifacts/sim1.dll/elevator-trim-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-elevator-trim-20261125-003)
- [Selected build disassembly](../../../vm-share/modules/sim1/runs/sim1-elevator-trim-20261125-003/worker-output/disassembly.txt)
- [Explicit alignment trial](../../../vm-share/modules/sim1/runs/sim1-elevator-trim-20261125-002)
- [Native integration regression](../../../vm-share/modules/sim1/runs/sim1-elevator-trim-integration-20261125-001)

## Complete pitch moment calculation

The reconstruction of `sim1.dll:0x20c013a0` now covers the
normal airplane pitch callback from the coefficient-view check
through both return paths. Other flight model callbacks and
producers supply inputs through their verified storage and
dispatch interfaces.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c4f356`, `0x20bc2618`, `0x20c50270`–`0x20c502f0` | The constructor installs the table at `0x20bc2618`. Its slots `+0x70`, `+0x74`, `+0x78`, `+0x7c` and `+0x80` point to `0x20c50270`, `0x20c50290`, `0x20c502b0`, `0x20c502d0` and `0x20c502f0`. Their binary64 data fields are `+0x84`, `+0x8c`, `+0x94`, `+0x9c` and `+0xa4`. All return zero without a coefficient table. The caller uses the four slots starting at `+0x74`. | live verified |
| `0x20bc2688` | The earlier scope paired `0x20c50270` with `+0x74` and shifted all four pitch caller targets by one slot. That interpretation and the direct calls in the earlier pitch build are superseded by the constructor-relative offsets and corrected build. Its four exact getter matches remain byte evidence for their original addresses. | superseded |
| `0x20c00840`, `0x20c014e6`–`0x20c0150d`, `0x20c1c5d0` | The flap-group update publishes the pitch coefficient at `0x20c7b958`, multiplied here by the main pressure and geometry scale. The flap term multiplies slot `+0x74` by the flap deployment factor at runtime `+0x6e4` and by that scale. The deployment writer derives the factor from the active surface records. | live verified |
| `0x20c061e0`, `0x20c40da0`, `0x20c01519`–`0x20c01550` | The two spoiler angles at control block `+0x10c` and `+0x114` are averaged, then multiplied by the slot `+0x78` coefficient and the main scale. | live verified |
| `0x20c45760`, `0x20c005df`, `0x20c016ad`–`0x20c01727` | The first tail term multiplies elevator deflection minus fixed tail incidence by slot `+0x7c`, wing area, chord and the slipstream pressure increment at runtime `+0x1fc`. That increment uses `0.5 * density * (2 * airspeed * slipstreamSpeed + slipstreamSpeed²)`. The second uses slot `+0x80`, the signed pitch-rate field and runtime scale `+0x768`, derived from airflow plus slipstream speed. Their sum is multiplied twice by the cosine of raw alpha from the angle update at `0x20c2e13d`. | live verified |
| `0x20be8d56`, `0x20be8d85`, `0x20c01707`–`0x20c01741` | The supplemental object at airplane `+0x2f0` supplies a scalar through slot zero. The default table at `0x20bbefd8` targets the zero return at `0x20c0d7d0`. The other verified table at `0x20bbefe8` targets `0x20c2daf0`, which scales provider state `+0x2c` by a curve and 0.002. The pitch callback multiplies the result by the main scale. | live verified |
| `0x20bf68f5`, `0x20bd17a0`, `0x20c0177b`–`0x20c01847` | `REALISM/SimpleAero` selects this separate result through control `+0x04`. It uses three times the pitch-rate term plus 0.4 times the elevator term. When absolute pitch exceeds 20° and signed elevator deflection times pitch is below one, it doubles the rate term and scales the elevator term using a curve from 1 at 20° to 0.2 at 45°, clamped at its endpoints. | live verified |
| `0x20c01848`–`0x20c01889` | The normal sum order is angle, baseline, elevator, pitch rate, alpha rate, tail incidence, elevator trim, flap-group pitch, flaps, spoilers, gyro, force-reference transfer, supplemental, and tail slipstream. The comparator follows the original stores and adjusts the load offsets for the stack pops. | live verified |

The concrete supplemental callback signatures and slot targets
were checked independently. The source keeps their virtual
interface and uses minimum layouts for the runtime, geometry and
coefficient data. Suggested Ghidra names for `FUN_20c013a0` are
`param_2` → `airplane` and `param_3` → `aerodynamicForce`, based on the
two stack arguments and their verified field and vector reads.
The 17-byte Ghidra definitions retain their generated names.
The original four definitions were read back clean at revision
`1795566771278:d6cfccaa:205`. The fifth was saved with the roll
scope and read back clean at revision `1795568104133:d6cfccaa:350`.

The corrected precise VC8 RTM build matches all **85 bytes** of
the five accessors exactly. The conditional null-return form
recovers the original branch layout. The earlier ternary form
produces a different branch layout. The new instruction ranges
add **52 exact bytes** and **49 normalized bytes**. They cover the
spoiler loads and weighting, the published flap-group input,
three tail-slipstream arithmetic ranges, and the `SimpleAero`
tests, interpolation call and final weighted addition.

The full pitch body comparison is **1,258 original / 555
rebuilt bytes**. The reconstruction uses extracted routines,
a different stack frame, and direct calls to verified concrete
coefficient targets. Those differences remain visible in the
full listings. The established aerodynamic, state integration and
linked propulsion checks reproduced. The pitch scope is ready for a reconstruction
commit. Linked pitch and simulator verification remain open.
The normal roll sum at `0x20c01890` is documented in
[Sim1 airplane roll moment](sim1-roll-moment.md). The normal yaw
sum is documented in [Sim1 airplane yaw moment](sim1-yaw-moment.md).
The alternate airplane model remains open.

- [C++ pitch calculation](../../../src/sim1/aerodynamics.cpp#L765)
- [Original pitch listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c013a0.asm)
- [Corrected native COFF comparison](../../../artifacts/sim1.dll/pitch-sum-corrected-comparison.json)
- [Superseded caller mapping and original byte evidence](../../../artifacts/sim1.dll/pitch-sum-comparison.json)
- [Ghidra and input evidence](../../../artifacts/sim1.dll/pitch-sum-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-roll-sum-20261125-002)
- [Selected build disassembly](../../../vm-share/modules/sim1/runs/sim1-roll-sum-20261125-002/worker-output/disassembly.txt)
- [Ternary getter trial](../../../vm-share/modules/sim1/runs/sim1-pitch-sum-20261125-001)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-pitch-sum-propulsion-20261125-001)
- [Native state integration regression](../../../vm-share/modules/sim1/runs/sim1-pitch-sum-integration-20261125-001/state-integration-comparison.json)
