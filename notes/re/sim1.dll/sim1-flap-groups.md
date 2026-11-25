# Sim1 flap-group aerodynamic coefficients

The airplane force producer calls `sim1.dll:0x20c00840` at
`0x20c45562`, passing the airplane in EAX. The routine uses the
coefficient view of the normal model at airplane `+0x34c` and
updates seven binary64 globals. The earlier attribution of
these globals to propeller slipstream is **superseded**. The
distinct tail slipstream producer is `0x20c45760`.

| Evidence | Finding | Status |
| --- | --- | --- |
| `0x20c1fff0`, `0x20c1f490`, `0x20c1eef0` | A runtime flap-group list at `+0x22c` is built from the configured flap chain. Each `0x28`-byte node points to its configuration at `+0x00`, holds side deflections at `+0x04` and `+0x0c`, and links to its successor at `+0x24`. The flap update at `0x20c1eef0` writes both side deflections. | live verified |
| `0x20c1f580` | The flap parser binds `lift_scalar`, `drag_scalar`, `pitch_scalar`, and `span-outboard` at configuration offsets `+0x188`, `+0x190`, `+0x198`, and `+0xc8`. | live verified |
| `0x20c00861`–`0x20c00896` | Coefficient slots `+0x18`, `+0x34`, and `+0x70` resolve through `0x20bc2618` to `0x20c4f620`, `0x20c4fa70`, and `0x20c50270`. The ECX-based accessors return binary64 fields `+0x24`, `+0x3c`, and `+0x84` through ST0 and return zero when the tables are missing. The caller halves each result. | live verified |
| `0x20c00899`–`0x20c008ee` | Clears all seven outputs. A missing list or empty head ends the update. | live verified |
| `0x20c008f3`–`0x20c0096b` | Adds each signed side deflection times its lift scale to the corresponding lift global. For drag it clamps each side's deflection to zero before scaling. Both lift sums and first-side drag are reread from binary64 storage. Second-side drag is carried in the x87 stack and also published each iteration. | live verified |
| `0x20c00971`–`0x20c009e5` | Pitch adds the two signed terms weighted by the flap pitch scalar. Roll and yaw use second minus first of the **running** lift and drag sums, respectively, weighted once per node by `0.5 * span-outboard * wingSpan`. The payload geometry pointer at `+0x197c` is the corresponding owner pointer at `+0x1980`; `0x20be8470` establishes the four-byte owner prefix. | live verified |

The seven output addresses are `0x20c7b948` (yaw), `0x20c7b950`
(roll), `0x20c7b958` (pitch), `0x20c7b960` and `0x20c7b968`
(side drag), and `0x20c7b970` and `0x20c7b978` (side lift).
The reconstructed names use first and second side to preserve
the verified storage order. Full runtime flap layout and
in-simulator behavior remain open. A proposed Ghidra rename
is `in_EAX` → `airplane` based on the original call site.

## Matching and evidence

The native VC8 COFF build matches 45 bytes across the three
complete accessors and 96 normalized bytes across ten additional
ranges. `0x20c00840` differs at 431 original and 440 rebuilt
bytes. The comparison retains both listings and verifies each
adjusted global operand and the `0.5` literal. The new `0x20c4f620`
function definition required a 14-byte disassembly repair before
comparison. The original PE bytes, generated names and verified
target addresses are preserved. Ghidra revision
`1795612398258:62a95bf0:203` was saved and read back clean.

- [Original update listing](../../../artifacts/sim1.dll/sim1-inventory/functions/20c00840.asm)
- [Native COFF comparison](../../../artifacts/sim1.dll/flap-groups-comparison.json)
- [Parser, dispatch, and Ghidra evidence](../../../artifacts/sim1.dll/flap-groups-ghidra.json)
- [Build, source snapshots, and receipt](../../../vm-share/modules/sim1/runs/sim1-flap-groups-20261125-001)
