# Sim1 airplane roll moment

The normal airplane model in `sim1.dll` installs `0x20c01890`
at slot `+0x18` of the table at `0x20bc2160`. The callback at
`0x20c45510` writes its x87 return value to output component 2.
The function reads the airplane and force vector from its two
stack arguments and returns zero if the coefficient view is absent.
The reconstructed source uses the verified concrete model instance
and keeps its unreconstructed coefficient providers as typed virtual
calls through their original table offsets.

## Terms and return paths

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c00530`, `0x20c018e1`–`0x20c01917` | The writer stores `q*S` at runtime `+0x1e4`, `q*S*b` at `+0x1ec`, and `ρ*V*S*b²/4` at `+0x750`. The callback scales the published flap-group coefficient at `0x20c7b950` by `q*S` and the sideslip coefficient by `q*S*b` and the signed sideslip in radians. The angle update at `0x20c2e0b0` writes sideslip at `+0x1a4`. | live verified |
| `0x20c0191e`–`0x20c019e4`, `0x20bfb330` | Adds rudder deflection, first spoiler minus second spoiler, and aileron trim contributions. The trim path reads binary32 at control `+0x104` and applies `aileron_trim_effectiveness` at configuration owner `+0x116c`. These three terms use `q*S*b`. | live verified |
| `0x20c019e4`–`0x20c01a93` | Airplane slot `+0x50` selects one common aileron coefficient for the mean deflection, replicated into both contributions, or separate coefficient calls using selectors 2 and 3 for control offsets `+0xd4` and `+0xdc`. All paths apply `q*S*b`. The combined path sends selector 1. | live verified |
| `0x20c01a97`–`0x20c01b26`, `0x20c71d10`, `0x20c454c0` | Roll and yaw rate terms use transformed rates at `+0x144` and `+0x13c` and scale `+0x750`. With airplane slot `+0x50` enabled, roll damping is multiplied by the six-sample curve at `0x20c71d10`, indexed by raw alpha divided by the binary32 reference returned through configuration slot `+0x10`. The curve ranges from `(0, 1)` to `(3.71, 0.45)` and peaks at `(1.4, 1.08)`. | live verified |
| `0x20c01b2a`–`0x20c01b4c`, `0x20c508b0`, `0x20c2db40` | The coefficient table's slot `+0xa0` and supplemental provider slot `+0x04` each contribute a term scaled by `q*S*b`. The default supplemental slot returns zero at `0x20c0d7d0`. The configured slot scales provider state `+0x30` by a curve and `0.004`. | live verified |
| `0x20c01b53`–`0x20c01c4a`, `0x20bf0b80` | If either runtime input `+0x62c` or `+0x634` is positive, evaluate the local curve `(0,1)`, `(0.21,0.15)`, `(0.4,0.08)`, `(0.6,0.05)`, `(1,0.03)` for each side. It scales the corresponding aileron term and the mean of both results scales the trim term. The inputs' physical names remain provisional. | live verified |
| `0x20c01c55`–`0x20c01f43`, `0x20bfb330`, `0x20c2c850`, `0x20c488a0` | Positive reaction-torque gain uses `(1 - Vx/(1.5*referenceSpeed)) * controlGain * torque_on_roll`. The count and suppression flag can bypass the contribution. For every propeller, read the mapped engine torque at record `+0x44` with stride `0xec`. Reverse its sign if the propeller's rotation factor at `+0x44` is negative. Divide each signed term by the number of propellers on its engine and accumulate in propeller order. The settings reader binds `torque_on_roll` to owner `+0x11a8`. | live verified |
| `0x20c00780`, `0x20c00a00`, `0x20c01f44`–`0x20c01fc1` | When the four wing multipliers sum to less than four, compute the outer lift difference times half span and the inner lift difference times quarter span. The combined control path halves both products again. The globals are produced before the roll callback. The source uses first and second wing identifiers in encounter order. | live verified |
| `0x20c01fd2`–`0x20c02009` | `SimpleAero` returns `0.75*firstAileron + 0.75*secondAileron + spoilers + 1.3*yawRate`. These are the already-scaled terms, including any aileron attenuation. | live verified |
| `0x20c02025`–`0x20c0208d` | The normal return sums first aileron, second aileron, trim, spoilers, yaw rate, roll rate, rudder, sideslip, flap-group roll, propeller torque, additional roll, supplemental roll, force transfer, outer wing, and inner wing. Force transfer is `CG.y*Fx - (CG.x - highAlphaReferenceXShift)*Fy`. The comparator checks the original stack stores, the adjustment for three pops, and the final x87 adds. | live verified |

The coefficient table base is `0x20bc2618`, installed at
`0x20c4f356`. Its roll offsets `+0x84` through `+0xa0` resolve to
`0x20c50790`, `0x20c506e0`, `0x20c50310`, `0x20c50630`,
`0x20c504b0`, `0x20c50870`, `0x20c50890` and `0x20c508b0`.
The aileron target consumes one stack argument. The other verified
targets receive the view in ECX and return a scalar in ST0. The
two 17-byte accessors read binary64 table `+0xdc` and binary32
table `+0xe4`, respectively, with a zero return for absent tables.
The remaining coefficient and supplemental bodies are dependencies
of this reconstruction. The field meanings behind the wing inputs
and the alternate model path remain open. The normal yaw path
was subsequently reconstructed. [Normal airplane yaw moment](sim1-yaw-moment.md).

Suggested Ghidra variable names are `param_2` → `airplane`, based
on its runtime and configuration loads, and `param_3` →
`aerodynamicForce`, based on the caller and the vector reads.
These are suggestions. Ghidra retains the generated names and
inferred signatures. The parent tags and the three short accessor
definitions and tags were saved and read back with clean revision
`1795568104133:d6cfccaa:350`.

## Matching and evidence

The precise VC8 RTM native COFF build matches **82 bytes exactly**
and **94 bytes after validating individual addresses** across the two
complete accessors and seventeen instruction ranges. The full roll
body differs at 2,046 original and 1,259 rebuilt bytes. The extracted
propeller reaction torque and `SimpleAero` bodies also differ,
at 748/666 and 37/39 bytes respectively. The differences include
stack frames, register allocation and loop/call structure. Linked
roll matching and in-simulator behavior remain open.

- [C++ roll calculation](../../../src/sim1/aerodynamics.cpp#L855)
- [Original roll listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c01890.asm)
- [Ghidra evidence and save state](../../../artifacts/sim1.dll/roll-sum-ghidra.json)
- [Native COFF comparison](../../../artifacts/sim1.dll/roll-sum-comparison.json)
- [Selected build, source snapshot and receipt](../../../vm-share/modules/sim1/runs/sim1-roll-sum-20261125-002)
- [Selected build disassembly](../../../vm-share/modules/sim1/runs/sim1-roll-sum-20261125-002/worker-output/disassembly.txt)
- [Linked propulsion regression](../../../vm-share/modules/sim1/runs/sim1-roll-sum-propulsion-20261125-001/propulsion-comparison.json)
- [Native state integration regression](../../../vm-share/modules/sim1/runs/sim1-roll-sum-integration-20261125-001/state-integration-comparison.json)
