# Turbine thrust and the engine-loop boundary

## Shared thrust path

The common turbine loop converges after the jet and shaft-producing
branches. It evaluates gross thrust and inlet momentum drag, then applies
airframe configuration for afterburning and reverse thrust. The resulting
force enters the existing engine thrust record read by the force collector.
The source names and minimum layouts below are reconstructions.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c2b879`–`0x20c2b8be`, `0x20c266ee`–`0x20c26707` | AIR `0x1506` occupies the 0x740-byte table at turbine performance offset `+0xb20`. The original evaluates it at raw Mach and corrected spool speed from turbine `+0x10`, multiplies by `static_thrust` at data `+0x00` and by the inlet pressure ratio, and stores gross thrust at `0x20c7ba60`. | live verified |
| `0x20c2c2e0`–`0x20c2c343`, `0x20c2670c`–`0x20c26720` | AIR `0x1507` supplies the next 0x740-byte table at data `+0x1260`. Its first input is capped inlet Mach at turbine `+0x40`. The second is the same corrected spool speed. The original multiplies the table value by `inlet_area`, multiplies by inlet pressure ratio and divides by the stored inlet temperature root. It then multiplies by inlet Mach times speed of sound at history `+0x164`, divides by the shared gravity scale at `0x20c7c180` and stores ram drag at `0x20c7ba58`. | live verified |
| `0x20c2bb20`–`0x20c2bb6a`, `0x20c26290` | The afterburner helper divides the unsigned stage at turbine `+0x7c` by the unsigned configured stage count at turbine data `+0x24` and returns the binary32 result. Missing performance data or a zero count returns zero. The configuration binds the count to `afterburner_available`. | live verified |
| `0x20c2c4e0`–`0x20c2c619`, `0x20c26290` | The AIR `0x1522` selector at data `+0x48` selects the original 5-by-5 table, indexed by raw Mach and reverser position at turbine `+0x60`. Otherwise strictly positive `reverser_available` at data `+0x44` gives `1 - (1 + 0.375 * effectiveness) * position`. The remaining path gives one. | live verified |
| `0x20c2c620`–`0x20c2c6c0`, `0x20c26290` | The original rounds the reverser multiplier to binary64. With a nonzero stage count and strictly positive afterburner fraction, it evaluates AIR `0x1524` at raw Mach and applies `1 + max(0, fraction * (curve - 1))` to gross thrust. It multiplies the result by the reverser factor, subtracts ram drag, and stores the same net thrust in turbine `+0x48` and engine `+0x34`. | live verified |

The turbine source uses the verified general coefficient interpolation
with 231-value AIR storage and 25-value reverser storage. The prior
propeller evaluator stays at its existing symbol. All three native VC8
evaluator bodies contain the same 706 bytes.

## End of the turbine loop

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c2b8e9`–`0x20c2b94c` | After publishing thrust and the existing speed-scaled output, the loop calls further per-engine state updates at `0x20c2c980`, `0x20c2caa0`, `0x20c2ce50`, `0x20c2cbe0`, `0x20c2cd80` and `0x20c2cf80`. The earlier open reconstruction status is superseded by [the later state reconstruction](sim1-turbine-preparation-and-state.md#indications-and-post-thrust-state). | live verified |
| `0x20c2b951`–`0x20c2b97b` | The final inline turbine-state calculation targets `(shaftSpeed - turbineSpeed) * 0.01 + shaftSpeed * 0.023`. It applies response gain 2 and the step and stores the result at turbine `+0x68`. The source name `filteredSpeedBlend` describes the calculation. Its physical meaning remains open. | live verified |
| `0x20c2b989`, `0x20c207a0`, `0x20c208ed`, `0x20c20952` | The last call in the turbine iteration is shared with the piston and other paths. It passes the step and calls `visualfx.dll` ordinals 29 and 8. The effects routine is outside the turbine physics reconstruction. | live verified |
| `0x20c2b98e`–`0x20c2b9b9`, `0x20c20492`–`0x20c204ac` | The loop advances the 0x80-byte turbine and 0xec-byte engine records, then returns at `0x20c2b9b9` when all engines are processed. The caller resumes at `0x20c20497`, checks propeller count separately at `0x20c2049a`, and calls `0x20c47420` at `0x20c204ac` if that count is nonzero. This is the turbine-to-propeller boundary. | live verified |

The verified boundary identifies where the turbine section ends in the
dispatcher. The earlier open status for common preparation, the jet
spool paths and the post-thrust states is superseded by
[the complete reconstructed iteration](sim1-turbine-preparation-and-state.md).
The propeller and effects code remain separate from the turbine TUs.

Suggested Ghidra names include `unaff_EDI` → `turbine` in
`FUN_20c2c2e0` from its Mach and spool loads, and
`unaff_EBX` → `turbine` in `FUN_20c2c620` from the thrust store.
Ghidra revision `1795634906543:62a95bf0:520` remains saved.

## Verification

| Original entry or inline start | Native VC8 precise scope matches |
| --- | --- |
| `0x20c2b879` gross thrust | 26 normalized bytes |
| `0x20c2c2e0` ram drag | 5 exact and 34 normalized bytes |
| `0x20c2bb20` afterburner fraction | 7 exact bytes |
| `0x20c2c4e0` reverser factor | 10 exact and 26 normalized bytes |
| `0x20c2c620` net thrust | 18 exact and 14 normalized bytes |
| `0x20c2b951` final speed blend | 2 exact and 14 normalized bytes |

The original stack-built reverser table and the rebuilt static table
agree in all 208 bytes, including dimensions. The complete scopes still
differ because of calling conventions, register assignment, the static
table and control flow. The precise profile retains more matched
instructions than the fast profile.

Nine host x86 cases passed, including distinct Mach inputs to the two
tables, afterburner scaling and clamping, both reverser models, the
speed-blend update and the inlet-to-thrust handoff. Simulator runtime
verification and the complete turbine-loop reconstruction remain open.
The existing turbine host fixtures, native propeller suite and linked
propulsion comparison passed. The dashboard shows both instruction
listings for the six reconstructed entries and the boundary addresses.

[Ghidra and PE evidence](../../../artifacts/sim1.dll/turbine-thrust-ghidra.json) ·
[VC8 precise comparisons](../../../artifacts/sim1.dll/turbine-thrust-vc8/turbine-thrust-comparison.json) ·
[VC8 fast control](../../../artifacts/sim1.dll/turbine-thrust-vc8-fast/turbine-thrust-comparison.json) ·
[Host x86 results](../../../artifacts/sim1.dll/turbine-thrust-host-check/results.txt)

[Table evaluator comparison](../../../artifacts/sim1.dll/turbine-thrust-evaluator-regression.json) ·
[Turbine host regressions](../../../artifacts/sim1.dll/turbine-thrust-host-regression/results.txt) ·
[Propeller regression](../../../artifacts/sim1.dll/sim1-turbine-thrust-propeller-regression-20261125-001.log) ·
[Linked propulsion regression](../../../artifacts/sim1.dll/sim1-turbine-thrust-linked-regression-20261125-001.log)
