# Sim1 airplane yaw moment

The normal airplane model in `sim1.dll` installs `0x20c020a0`
at slot `+0x1c` of `0x20bc2160`. The caller at `0x20c45510`
writes the result to moment component 1. The callback takes the
airplane and aerodynamic force vector on the stack and returns its
scalar in ST0. It returns zero if the coefficient view is absent.

## Terms and return paths

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c020e7`–`0x20c02113`, `0x20c00840`, `0x20c50e20` | Scales the published slipstream yaw coefficient at `0x20c7b948` by runtime `q*S` and the additional `+0xc4` coefficient by `q*S*b`. The latter samples filtered alpha and applies a model float at `+0x08`, realism weight, and the `hi_alpha_on_yaw` setting. | live verified |
| `0x20c02117`–`0x20c0216a`, `0x20c00530` | Yaw rate uses transformed rate `+0x13c` and `ρ*V*S*b²/4` at `+0x750`. Airplane slot `+0x50` enables the shared six-sample high-angle curve at `0x20c71d10`, evaluated at raw alpha divided by the configuration reference angle. The force-scale writer supplies the same rate scale for the roll-rate contribution. | live verified |
| `0x20c0217d`–`0x20c02261`, `0x20bfb330` | Rudder deflection, rudder trim, sideslip, and the mean of both aileron deflections use `q*S*b`. The binary32 trim at control `+0x108` uses `rudder_trim_effectiveness` at configuration owner `+0x1174`. Slot `+0xa4` supplies the separate coupling with the transformed roll rate at `+0x144`. | live verified |
| `0x20c02265`–`0x20c023ce`, `0x20bfb330`, `0x20c47ba0` | The gyroscopic reaction accumulates `-(signedRPM * 0.1047197551 * inertia * pitchAngularVelocity) * gyro_precession_on_yaw * gyroGain`. It uses pitch rate at airplane `+0x1a0`, yaw setting at owner `+0x11ac`, and the common propeller count, suppression flag and ordered-positive gyro gain checks. | live verified |
| `0x20c023cf`–`0x20c025dc`, `0x20c22a20`, `0x20bfb330` | P-factor uses signed propeller thrust with arm `diameter * 0.1875 * boundedAlpha / limit * p_factor_on_yaw * gain`. The limit is the configuration angle minus three degrees. The original bounds raw alpha between the negative and positive limits using ordered comparisons. The count, suppression flag, positive gain and positive raw alpha gate the loop. Each propeller contributes negative thrust times arm and reverses sign for negative rotation at record `+0x44`. The forward air-relative velocity fades the sum from zero at 20 to full at 60 in the original velocity units. | live verified |
| `0x20c025dd`–`0x20c02641`, `0x20c00530`, `0x20c45760` | The tail slipstream subtotal adds `12*b*S*C_rudder*rudder*q_slipstream` and `C_rate*rollRate*scale_770*cos(alpha)²`. Slot `+0xbc` couples the transformed roll rate into yaw. The first product is stored as a double before the rate contribution is added. | live verified |
| `0x20c02645`–`0x20c0267f` | Force reference transfer reuses `a*Fz - b1*Fx`, where `a = CG.x - highAlphaReferenceXShift` and `b1 = CG.z - referenceZ - highAlphaReferenceZShift`. The first product is stored before the reference query. | live verified |
| `0x20c02680`–`0x20c026f3`, `0x20bf97e0`, `0x20bfa8b0`, `0x20bf92c0` | With airplane slot `+0x50` enabled, each installed `fuel/External1` or `fuel/External2` tank contributes `q * 0.047 * lateralPosition` if capacity is nonzero and flag `0x02` is clear. The runtime records start at `+0x3ec` and `+0x40c`. The original jettison transition sets the tested flag. | live verified |
| `0x20c026f4`–`0x20c02720` | `SimpleAero` returns `0.75*rudder + yawRate + rollRate + sideslip` using the previously scaled contributions. | live verified |
| `0x20c02721`–`0x20c0275b` | The normal path sums rudder, yaw rate, rudder trim, roll rate, sideslip, ailerons, additional yaw, slipstream, propeller gyro, P-factor, force transfer, external tanks, and tail slipstream in that order. The comparator follows the stack stores, adjusts the loads for the pops, and verifies the external tank addition from ST1. | live verified |

The coefficient table at `0x20bc2618` supplies slots `+0xa4`
through `+0xc4`. The original slot loads were checked against
the table and each concrete target. The three reconstructed
17-byte targets are `0x20c50dc0`, `0x20c50de0`, and `0x20c50e00`.
They take the coefficient view in ECX, return zero for absent
data, and read binary64 fields `+0x110`, `+0x118` and binary32
field `+0x120` otherwise. Other aerodynamic coefficient bodies
retain their original typed dispatch in the reconstruction.
The layouts and source symbol names are reconstructions.

Suggested Ghidra variable names are `param_2` → `airplane` from
its configuration and runtime reads and `param_3` → `aerodynamicForce`
from the moment-reference calculation. The saved Ghidra functions
retain their generated names. The parent tags and three accessor
definitions and tags were read back clean at revision
`1795569073366:d6cfccaa:495`.

## Matching and verification

The selected native VC8 RTM COFF build matches **69 exact bytes**
and **79 bytes after address validation** across the three complete
getters and fifteen instruction ranges. The whole callback differs
at 1,724 original and 779 rebuilt bytes. The gyro, P-factor, external
tank, and `SimpleAero` extracted bodies differ at 362/349, 526/495,
116/142, and 22/27 bytes respectively. Stack frames, register
allocation, direct/virtual call structure and loop shape account
for differences in the listings. The existing pitch, roll, state
integration and linked propulsion comparisons were reproduced. Linked yaw matching, simulator
verification and the alternate airplane model remain open.

- [C++ yaw calculation](../../../src/sim1/aerodynamic_moment.cpp#L1020)
- [Original yaw listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c020a0.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/yaw-sum-comparison.json)
- [Ghidra bindings and save state](../../../artifacts/sim1.dll/yaw-sum-ghidra.json)
- [Selected build, source snapshot and receipt](../../../vm-share/modules/sim1/runs/sim1-yaw-sum-20261125-002)
- [Selected build disassembly](../../../vm-share/modules/sim1/runs/sim1-yaw-sum-20261125-002/worker-output/disassembly.txt)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-yaw-sum-propulsion-20261125-001/propulsion-comparison.json)
- [Native state integration regression](../../../vm-share/modules/sim1/runs/sim1-yaw-sum-integration-20261125-001/state-integration-comparison.json)
