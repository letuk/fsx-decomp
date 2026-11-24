# sim1.dll flight-state integration

The aircraft update at `0x20c30840` can enter `0x20c30bc0` with a
double-precision step. On that path, `0x20c30e07` calls the acceleration
stage, `0x20c30e17` calls the velocity integrator, and `0x20c30e25` calls a
subsequent low-motion check. When the flag at the object reached through
`this+0xb0`, offset `+4`, is clear, `0x20c30e47` invokes slot `+0x00` on
the transform at `this+0x24`. This caller passes the flight object and
the step on the stack with the transform object in ECX.

| Original VA | Finding and evidence status |
| --- | --- |
| `0x20c30bc0` | Live verified flight-step caller and call order above. |
| `0x20c42d90` | Live verified acceleration stage. Copies the old linear and angular accelerations to history, calculates new values from forces, mass, gravity, rotating-frame terms, moments and inertia, and publishes transformed linear acceleration through virtual slot `+0x04`. The original receives the flight object in ESI. |
| `0x20c43110` | Live verified velocity integrator. ECX holds the instance, the callee consumes an eight-byte step, and each velocity advances by `(previousAcceleration + currentAcceleration) * 0.5 * step`. |
| `0x20c4312d` | Live verified 144-byte conditional history-copy block through `0x20c431bc` inside the integrator. Exact in native COFF. |
| `0x20c43530` | Live verified post-integration low-motion check. Uses velocity and angular-velocity squared magnitudes and a timer before invoking the observed callback. |
| `0x20bdec10` | Live verified target of slot `+0x00` in the reviewed concrete table. Converts body velocity through slot `+0x04`, advances altitude and geodetic position with the step, and advances attitude subject to three freeze fields. The full method has been inventoried. |
| `0x20bdeec0` | Live verified forward transform, slot `+0x04`. Preserves Y, rotates X and Z, and snapshots the input for an in-place call. The inputs are doubles and the coefficients are floats, as verified from the x87 memory operand widths. |
| `0x20bdef30` | Live verified inverse transform, slot `+0x08`. Uses the same layout and the inverse signs. |
| `0x20bdefa0` | Live verified coefficient refresh, slot `+0x0c`. Converts doubles from `FlightDynamics+0x100` and `+0xf8` into the two float coefficient members. |

The integration is the trapezoidal update of velocity using two samples
of its derivative. The history reset makes the two samples equal for
that step. Linear velocities saturate at ±4500. Angular rates at
`this+0x1a0`, `+0x1a8` and `+0x1b0` saturate at ±4.363, ±3.491 and ±7.33
respectively. The scale at `history+0x4f8` is multiplied by 1.01 when an
angular limit is hit. Otherwise the method scales it by 0.98 with a floor
of 1. The method then transforms the angular-rate components and stores
them at `history+0x134`, `+0x13c` and `+0x144`.

`FlightDynamics`, `IntegrationHistory`, `MassProperties` and
`HeadingTransform` are reconstructed minimum C++ layouts. `this+0x270`
is the history pointer. `this+0x188` and `+0x1a0` are the linear and
angular velocity triples, and `this+0x1b8` and `+0x1d0` are their current
derivative triples. `history+0xbc` and `+0xec` are the preceding samples.
Axis, frame and scale names are interpretations of the operations.
`sim1.dll:0x20c7c180` is live verified as an eight-byte gravity scale read
by the acceleration calculation and referenced from initialization at
`0x20bd06d6`. The native object retains an external reference to it.

The table at `sim1.dll:0x20bbdb1c` is installed by the paths at
`0x20bd156a` and `0x20c58181`. The original PE and Ghidra pointer reads
agree on slots `+0x00` through `+0x0c`. The slot calls and the targets'
individual signatures were checked separately. Slot `+0x00` takes the
flight object and a double and returns with `ret 0xc`. Slots `+0x04` and
`+0x08` take two vector pointers and return with `ret 8`. Slot `+0x0c`
takes one flight-object pointer and returns with `ret 4`. The reviewed
extent is a prefix. Other tables, target sets, source type names and
complete object extents remain open.

## VC8 comparison

| Scope | Original bytes | Selected rebuilt bytes | Result |
| --- | ---: | ---: | --- |
| `0x20c42d90` acceleration stage | 889 | 795 | Different. Native member entry uses ECX where the original caller uses ESI. x87 stack and branch scheduling also differ. |
| `0x20c43110` velocity integrator | 837 | 769 | Different. The integrator retains the original ECX input and callee-cleaned double. Stack allocation, clamp and x87 scheduling differ. |
| `0x20c4312d` history-copy block | 144 | 144 | Exact. The 24 paired dword loads and stores are relocation-free. This scope excludes the surrounding conditional branch. |
| `0x20bdeec0` forward transform | 101 | 101 | Exact. |
| `0x20bdef30` inverse transform | 101 | 101 | Exact. |
| `0x20bdefa0` coefficient refresh | 30 | 30 | Exact. |

Four XP worker runs used VC8 RTM `14.00.50727.42` in native x86 COFF
mode. The first two controlled `/fp:fast` versus `/fp:precise` and
produced acceleration/integrator lengths of 795/756 and 803/756. The
third changed the clamp expression and scale floor, producing 795/752.
The fourth uses an integer saturation flag and emits the same ESI flag
register and initial comparison as the original, with lengths 795/769.
It is the canonical replay. All three transform bodies match in every
run. The comparator also validates the gravity reference and each
referenced floating-point literal by its bytes. The selected run pairs
all five acceleration references and all thirteen integrator references.
The changed field offsets prevent normalization of the complete bodies.
Linked module verification, runtime verification and a C++ reconstruction
of the position and attitude callback remain open.

- [C++ source](../../../src/sim1/state_integration.cpp)
- [Original Ghidra inventory](../../../artifacts/sim1.dll/sim1-inventory/manifest.json)
- [Call path, VTable and save evidence](../../../artifacts/sim1.dll/state-integration-ghidra.json)
- [Selected native COFF comparison](../../../vm-share/modules/sim1/runs/sim1-state-integration-20261124-004/state-integration-comparison.json)
- [Precise floating-point control](../../../vm-share/modules/sim1/runs/sim1-state-integration-20261124-002/state-integration-comparison.json)

Ghidra tags were read back after a module-specific save. The saved
revision is `1795526504922:962445d7:25`, with `unsaved_changes=false`.
The pre-edit backup is retained in `artifacts/sim1.dll/ghidra-backups/`.
The original Ghidra names and types are preserved. Suggested local names
are `param_1` → `step` in `FUN_20c43110` (multiplies the derivatives),
`iVar3` → `history` (loaded from `this+0x270`), and `bVar7` →
`angularRateLimited` (set by a saturated angular component). These names
are provisional suggestions and were not applied in Ghidra.
