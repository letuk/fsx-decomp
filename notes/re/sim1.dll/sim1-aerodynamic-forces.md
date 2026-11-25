# Sim1 airplane aerodynamic forces

The airplane force and moment producer at `sim1.dll:0x20c45510`
queries the normal model at airplane offset `+0x34c` for lift,
drag and side force. The installed `0x20bc2160` table binds slots
`+0x08`, `+0x0c`, and `+0x10` to `0x20c00a00`, `0x20c00d80`,
and `0x20c00f40`. Both configuration branches query these force
slots on the normal model. The reconstructed output scope starts
after the prerequisite updates and ends at the moment calls.

## Pressure and rate scales

`sim1.dll:0x20c00530` receives the airplane in EDI. Its first
arithmetic block (`0x20c0054f` through `0x20c005f2`) writes the
nine binary64 scales below before entering the high-alpha branch.
The force producer calls this step after updating airspeed and
slipstream. At `0x20c2df37` and `0x20c2df8d`, the air-data update
writes air-relative speed `V` at runtime `+0x14c` and dynamic
pressure `q` at `+0x1dc`. The slipstream update at `0x20c45760`
supplies the speed increment `w` at `+0x204`. Density `ρ` comes
from the environment object at airplane `+0x18`, offset `+0x134`.
Wing area `S`, span `b` and chord `c` are the first three binary64
values in the geometry reached through configuration `+0x1980`.

| Original store | Runtime offset | Equation | Status |
| --- | --- | --- | --- |
| `0x20c00557` | `+0x1e4` | `q*S` | live verified |
| `0x20c00560` | `+0x1ec` | `q*S*b` | live verified |
| `0x20c0056f` | `+0x1f4` | `round64(q*S)*c` | live verified |
| `0x20c00592` | `+0x748` | `ρ*V*S*b/4` | live verified |
| `0x20c005a0` | `+0x750` | `ρ*V*S*b²/4` | live verified |
| `0x20c005ab` | `+0x758` | `ρ*V*S*c/4` | live verified |
| `0x20c005b7` | `+0x760` | `ρ*V*S*c²/4` | live verified |
| `0x20c005df` | `+0x768` | `ρ*(V+w)*S*c²/4` | live verified |
| `0x20c005ed` | `+0x770` | `ρ*(V+w)*S*c*b/4` | live verified |

The extracted C++ method keeps the original pressure-force reload
before the chord multiplication. The native VC8 comparison verifies
all nine output stores and the original PE and COFF representations
of `0.25` at `0x20bc3388`. Five disjoint ranges give 19 exact bytes
within the 164-byte original scope and 156-byte rebuilt routine.
The addressing registers, x87 scheduling and surrounding calling
convention differ. The second block publishes the
[high-alpha reference correction](sim1-aerodynamic-moment.md#high-alpha-reference-correction).
Ghidra
tags were saved and read back clean at revision
`1795609326019:62a95bf0:6`. `unaff_EDI` → `airplane` is a proposed
name based on the call site and field reads.

- [Original setup listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c00530.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/load-scales-comparison.json)
- [Ghidra and input evidence](../../../artifacts/sim1.dll/load-scales-ghidra.json)
- [Build, snapshots and receipt](../../../vm-share/modules/sim1/runs/sim1-load-scales-20261125-001)

## Tail slipstream speed and pressure

The producer at `sim1.dll:0x20c45510` calls `0x20c45760` at
`0x20c45541`. When airplane slot `+0x50` returns zero, the update
preserves the two runtime fields. Otherwise it walks the engine
array at runtime `+0x43c` in `0xec`-byte records, with the count
at propulsion configuration `+0x454`. A missing array results
in both outputs being cleared.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c457a1`–`0x20c457db` | The engine contributes only when its mount has lateral `x == 0` and `z > 0`. The loop sums its induced-air-speed value at engine record `+0x3c`, with one count per qualifying engine. | live verified |
| `0x20c480b0`, `0x20c476e1` | The engine/propeller update writes the source field at `0x20c4812d` and `0x20c4813a`. Its positive-density path uses clamped forward speed and thrust in the square-root calculation. See [Propeller induced airspeed](sim1-engine-branches.md#propeller-induced-airspeed) for the reconstruction and comparison. | live verified |
| `0x20c457f1`–`0x20c45823` | Divides by the selected count and stores the mean `w` at runtime `+0x204`. With air-relative speed `V` and environment density `ρ`, computes `2*V*w + w*w` and stores it at `+0x1fc`, then overwrites it with `0.5*ρ*(2*V*w + w*w)`. | live verified |
| `0x20c45832`–`0x20c4583f` | Clears both outputs if no engine qualifies. | live verified |

The first VC8 trial unrolled the indexed loop and produced 460
bytes. The selected pointer-walking trial produced 223 bytes
against the 233-byte original. Ten disjoint ranges match with **48
exact bytes** and **12 bytes after address validation**. The
remaining differences include the C++ member ABI, register
scheduling and zeroing path. The original function receives the
airplane on the stack and returns with `ret 4`. The recovered
source layout and `UpdatePropellerSlipstream` are reconstructions.
`param_1` → `airplane` is a proposed Ghidra name. Tags were saved
and read back clean at revision `1795611579878:62a95bf0:13`.
The linked propulsion regression reproduced. Simulator verification remains open. See [flap-group aerodynamic coefficients](sim1-flap-groups.md) for the distinct update at 0x20c00840.

- [Original function listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c45760.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/tail-slipstream-comparison.json)
- [Ghidra and writer evidence](../../../artifacts/sim1.dll/tail-slipstream-ghidra.json)
- [Selected build and receipt](../../../vm-share/modules/sim1/runs/sim1-tail-slipstream-20261125-002)
- [Indexed-loop trial](../../../vm-share/modules/sim1/runs/sim1-tail-slipstream-20261125-001)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-tail-slipstream-propulsion-20261125-001/propulsion-comparison.json)

## Force calculations

| Original addresses | Finding | Status |
| --- | --- | --- |
| `0x20c00530`, `0x20c00a00`–`0x20c00a4b`, `0x20c00d80`–`0x20c00dc2`, `0x20c00f40`–`0x20c00f70` | The three scalar queries return zero if the coefficient view is absent. The geometry update writes `q*S` at runtime `+0x1e4` and the lateral and lift rate scales `ρ*V*S*b/4` and `ρ*V*S*c/4` at `+0x748` and `+0x758`. Here `q` is dynamic pressure, `S` is wing area, `b` is wing span and `c` is reference chord. | live verified |
| `0x20c00a4c`–`0x20c00b07`, `0x20c00780`, `0x20c00840` | Raw-alpha lift slot `+0x04` and the two scale slots `+0x20` and `+0x24` generate four wing loads. Each uses one quarter of the lift coefficient, its own wing multiplier, both scale factors, and `q*S`. The two inner sections add the corresponding flap-group lift coefficient before scaling. The four stores at `0x20c7b9c0`, `0x20c7b9b8`, `0x20c7b9b0` and `0x20c7b9a8` also feed the roll callback. | live verified |
| `0x20c00b08`–`0x20c00c7e`, `0x20bfb330` | The five-sample attenuation curve scales the two published loads on each side when the corresponding runtime inputs at `+0x62c` or `+0x634` enable the path. The four loads are summed and multiplied by `s + (1-s)*min(abs(rawAlpha/referenceAngle),1)`, with `s` from `cruise_lift_scalar` at owner `+0x112c`. The reference angle is returned by the configuration slot `+0x10`. | live verified |
| `0x20c00c82`–`0x20c00d65`, `0x20c03b10` | The lift sum adds pitch rate, wing loads, elevator, spoilers, alpha rate, tail incidence, then `-0.6 * state[+0x478] * q*S`. Spoilers use the mean angle. The adjustment input has an independently identified time-dependent writer at `0x20c03b10`. The callback publishes lift at runtime `+0x5c`. | live verified |
| `0x20c00dc3`–`0x20c00e86`, `0x20c00f06`–`0x20c00f2c` | Drag adds induced drag evaluated with both inner flap-group lift coefficients, scaled base drag, supplementary drag gated by geometry DWORD `+0x48`, both flap-group drag coefficients, flap and spoiler terms, and the external tank term. The original also multiplies runtime `+0x478` by zero before adding it. The coefficient sum is multiplied by `q*S` and published at runtime `+0x64`. | live verified |
| `0x20c00e9e`–`0x20c00f04`, `0x20bfa8b0`, `0x20bf92c0` | With airplane slot `+0x50` enabled, an external tank with nonzero capacity and cleared flag `0x02` adds `0.047/S` to the drag coefficient. In force units this is `0.047*q` per attached tank. The same records supply the yaw contribution through their lateral arms. | live verified |
| `0x20c00f71`–`0x20c00ffc` | The side-force sum adds roll-rate and yaw-rate terms multiplied by runtime `+0x748`, then rudder and sideslip terms multiplied by `q*S`. This is the only scalar published without rotation by alpha. | live verified |
| `0x20c455b8`–`0x20c4562b`, `0x20c4564f`–`0x20c456c4` | The producer stores `Fx = side`, `Fy = lift*cos(alpha) + drag*sin(alpha)` and `Fz = lift*sin(alpha) - drag*cos(alpha)` before passing the vector to the moment callbacks. The signs follow the x87 destination registers and final reverse subtraction. | live verified |

The source retains typed calls through the concrete coefficient
table at `0x20bc2618` for the remaining coefficient bodies.
`0x20c4f640`, `0x20c4fa50` and `0x20c4fa90` read binary64 table
fields `+0x2c`, `+0x44` and `+0x4c`, respectively, returning zero
without the coefficient data. The three scalar force targets have
the model in ECX and the airplane pointer on the stack. The original
coefficient targets for the alpha lift and induced drag also take a
binary64 stack argument. The callback roles of these fields and the
body-axis signs are verified. The physical interpretation of the
geometry gate and some upstream scale inputs remains provisional.

Suggested Ghidra variable names are `param_2` → `airplane` for the
three scalar bodies and `param_2` → `bodyForce` in `0x20c45510`,
based on the verified field reads and stores. The saved definitions
retain their generated names. The three new accessor definitions,
the force-function tags and the producer's scoped tag were read
back clean at revision `1795569949604:d6cfccaa:649`.

## Matching and verification

The VC8 RTM `/O2 /fp:precise` native COFF comparison gives **72
exact bytes** and **64 bytes after address validation**, including
the three complete 14-byte accessors. The full lift, drag and
side-force bodies differ at 890/778, 438/406 and 189/167 original
and rebuilt bytes. The extracted axis conversion and publication
scope differs at 118/110 bytes. The comparisons preserve the
remaining differences in stack layout, direct and virtual calls,
register scheduling and spill placement. The existing pitch, roll
and yaw comparisons reproduced, along with the linked force
collection regression. The alternate model
preparation, remaining provider bodies, linked image matching
and simulator verification are outside this scope.

- [C++ force publication](../../../src/sim1/aerodynamics.cpp#L1274)
- [Original force and moment producer](../../../artifacts/sim1.dll/sim1-inventory/functions/20c45510.asm)
- [Original lift](../../../artifacts/sim1.dll/sim1-inventory/functions/20c00a00.asm)
- [Original drag](../../../artifacts/sim1.dll/sim1-inventory/functions/20c00d80.asm)
- [Original side force](../../../artifacts/sim1.dll/sim1-inventory/functions/20c00f40.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/aerodynamic-force-sum-comparison.json)
- [Ghidra bindings and save state](../../../artifacts/sim1.dll/aerodynamic-force-sum-ghidra.json)
- [Selected build, source snapshot and receipt](../../../vm-share/modules/sim1/runs/sim1-aerodynamic-force-sum-20261125-002)
- [Selected build disassembly](../../../vm-share/modules/sim1/runs/sim1-aerodynamic-force-sum-20261125-002/worker-output/disassembly.txt)
- [Linked force collection regression](../../../vm-share/modules/sim1/runs/sim1-aerodynamic-force-linked-20261125-001/forces-comparison.json)
