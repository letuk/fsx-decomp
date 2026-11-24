# sim1.dll forces, moments and mass properties

The aircraft step at `0x20c30bc0` calls the mass update at
`0x20c30cae`, collects force and moment contributions at
`0x20c30db0` and calls the acceleration stage at `0x20c30e07`.
Other flight-step calls run between these stages. The checks below
use live Ghidra listings and the hash-pinned original PE.

| Entry VA | Live verified behavior |
| --- | --- |
| `0x20bccae0` | Zeros two 24-byte output vectors through `api.dll` ordinal 109. Walks the signed count at registry offset `+0x00` and the array at `+0x0c`. Entries have stride `0x84` and the callback object is at `+0x80`. For each non-null object, calls slot zero with the step and two zeroed temporary vectors, then adds all six returned components. |
| `0x20bf6a20` | Callback adapter in slot zero of table `0x20bbefd4`. Loads the bound context from `+0x04` and the target from `+0x08`, forwards the step and the two output pointers and returns with `ret 0x10`. |
| `0x20bccef0` | Recalculates weight, mass, center of gravity and inertia from the registered weight and position pointers. Runs when its stack flag is set or the dirty field at `+0x0c` is nonzero. Publishes through the owner's virtual slot `+0xe4` and clears the dirty field. |
| `0x20c42d90` | Consumes the force triple at flight-object offset `+0x300`, the moment triple at `+0x318` and the mass and inertia object at `+0x2f8`. Copies the earlier acceleration into history before writing new linear and angular accelerations. |

The original force-collection call supplies the registry in EBX,
force storage in EDI, moment storage in ESI and the step on the stack.
The callee returns with `ret 8`. The callback object is in ECX when
the collector calls slot zero. The table entry and the target's
independent machine signature agree with three stack arguments
`(double, Vector3*, Vector3*)`. The binding adapter then transfers
its stored context into ECX for the concrete target.

## Verified producer bindings

The installation at `0x20be88f0` uses table `0x20bbefd4` for the
embedded objects at `+0x240` and `+0x24c`. The airplane constructor
at `0x20be8800` also uses it at `+0x340`. The helicopter branch
at `0x20be86f0` uses it at `+0x33c`. The registration helper at
`0x20bccbb0` reaches `0x20bceb40`, which stores the passed object
at the entry's `+0x80`.

| Registration path | Object offset | Bound target |
| --- | --- | --- |
| `0x20c046c0` | `+0x240` | `0x20c010f0` |
| `0x20c046c0` | `+0x340` | `0x20c45510` |
| `0x20c046c0` and `0x20c047e0` | `+0x24c` | `0x20c48a90` |
| `0x20c047e0` | `+0x33c` | `0x20c176f0` or `0x20c4c360`, selected by the field at configuration offset `+0x1994` |

Each of these five concrete targets has the observed ECX instance
and an epilogue with `ret 0x10`. The full producer equations, remaining
registrations and runtime target set remain open.

### Larger model entry points

| Entry | Live verified scope in `sim1.dll` |
| --- | --- |
| `20c45510` | Airplane force and moment producer. Prepares inputs, updates a model and queries three force and three moment results through virtual calls. |
| `20c013a0`, `20c01890`, `20c020a0` | Three concrete moment calculations installed at `20bc2160+0x14`, `+0x18` and `+0x1c`. The callback writes their results to moment components 0, 2 and 1 on this path. The aerodynamic meaning of individual terms is provisional. |
| `20c48a90`, `20c48dc0` | Ground-contact producer and per-contact calculation. Walks active contact records and feeds the contact force and moment results into the same collector used by the airplane model. |
| `20c4c360` | Separate helicopter rotor model entry. Calls the rotor calculation stages and writes three force and three moment components. |

`20c45c90` initializes table `20bc2160` on the model installed by
`20c458e0` at airplane offset `+0x34c`. The eight-pointer prefix
agrees with the original PE, including all eight HIGHLOW fixups.
The three moment-slot call sites pass the model in ECX, the airplane
pointer and force-vector pointer on the stack, and consume an x87
return value. The three target listings independently show
`ret 8` and their use of the stack inputs. The alternate `+0x350`
model also supplies moment results on a separate configuration path.
Its full layout and equations remain open. Ghidra was read-only,
with revision `1795543912564:962445d7:153` read back clean.
[Model entry evidence](../../../artifacts/sim1.dll/flight-model-entrypoints-ghidra.json).
[Airplane aerodynamic moment](sim1-aerodynamic-moment.md) records the
first reconstructed term inside `0x20c013a0`.

## Mass and inertia inputs

The allocation path at `0x20be8c85` requests `0x70` bytes and
stores the result of `0x20be7f10` at flight offset `+0x2f8`.
The constructor installs the owner pointer at accumulator `+0x04`.
`0x20bcce90` allocates an eight-byte pair of pointers and appends it
to the accumulator. `0x20bccef0` walks the count at `+0x60` and
pointer array at `+0x6c`, skips missing entries or values, and reads
single-precision weight and position components.

The recalculation initializes six inertia fields from the six base
values at `+0x48` through `+0x5c`. For each entry it adds weighted
position and incorporates the squared and mixed displacements from
the stored center. The new center at `+0x24` through `+0x2c` is the
sum of weighted positions divided by total weight. The mass at `+0x14`
is total weight divided by `GravitySeaLevel`. The gravity value is
the double at `sim1.dll:0x20c7c180`. The earlier statement that its
default was open is superseded by the `0x20bd0600` lookup and fallback
table. The fallback double is 32.174 and a positive configured value
can replace it.

The owner's `+0xe4` call passes the owner in ECX and no stack
arguments. The original PE and Ghidra reads agree that this slot
points to `0x20c2edc0` in the airplane table `0x20bbecd0` and to
`0x20c2ee60` in the helicopter table `0x20bbee38`. Both targets
read the accumulator and return with a plain `ret`. The base-table
slot at `0x20bbec6c` instead points to the six-byte jump at
`0x20c669a6` to `MSVCR80.dll!_purecall`. The analysis covers
these three slot instances. Remaining dynamic owner types remain open.

`ForceContributors`, `ForceContributorEntry` and
`BoundForceContributor` are reconstructed minimum layouts. The
mass-notification pointer is a minimum C++ ABI projection of the
already modeled flight-object dispatch table. Force-axis and inertia
member names describe the observed consumers and products.

## VC8 comparison

| Scope | Original bytes | Rebuilt bytes | Match |
| --- | ---: | ---: | --- |
| `0x20bf6a20` callback adapter | 36 | 36 | Exact, complete linked body. |
| `0x20bccae0` force and moment collection | 192 | 192 | Normalized, complete linked body. Both absolute fields were validated against the original and rebuilt `api.dll` ordinal 109 import identities. |
| `0x20bccef0` mass properties | 427 | 431 | Different. ESI and the original forced-update stack argument are restored. Loop register allocation and x87 local stores still differ. |
| `0x20bcd05a` center-of-gravity and mass publication | 59 | 59 | Normalized. The gravity reference is the only adjusted field. The range includes the notification and dirty reset. |
| `0x20c42d90` acceleration preparation | 889 | 798 | Different. The linked version restores the original ESI input. Floating-point scheduling, spills and clamps still differ. |
| `0x20c42da5` previous linear-acceleration copy | 66 | 66 | Exact, linked range with no address fields. |

The comparison uses VC8 RTM `14.00.50727.42` and the no-entry linked
profile. The build-only caller reproduces the reviewed relative call
sequence and register pressure. It supplies a variable mass-update
flag in the selected run so LTCG retains the original stack argument.
Its gravity storage supplies an address for comparison. The original
initializer and the intervening flight-step work are outside this
comparison image.

Runs 001 and 002 retained failed links during API import-name setup.
Run 003 got both full matches with precise floating-point and
specialized out a constant zero mass-update flag. Run 004 tested
fast floating-point with the same caller and preserved both full
matches. Its mass body shrank to 353 bytes and lost the matching
publication range. Run 005 uses precise floating-point and a
variable flag. Its 431-byte mass body restores the stack ABI and
retains the 59-byte normalized range. The 50-byte component-add
block at `0x20bccb4c` also matches inside the full collection body.

The shared layout changes preserved the three exact transform
methods and the 144-byte integration-history copy in the fresh
integration regression. The linked position regression retains the
86-byte normalized velocity writer and the 31-byte normalized
longitude range. Runtime verification remains open.

Suggested Ghidra names are `param_1` → `step`, `unaff_EDI` →
`forceSum` and `unaff_ESI` → `momentSum` in `FUN_20bccae0`.
In `FUN_20bccef0`, `param_1` → `forceUpdate` describes the dirty
gate override. These are provisional suggestions. Ghidra tags were
applied and read back after save. The saved revision is
`1795529521749:962445d7:55` with `unsaved_changes=false`.

- [C++ contributors and mass update](../../../src/sim1/flight_forces.cpp)
- [Original Ghidra callback, VTable and save evidence](../../../artifacts/sim1.dll/force-and-moment-ghidra.json)
- [Selected linked comparison](../../../vm-share/modules/sim1/runs/sim1-forces-20261124-005/forces-comparison.json)
- [Integration regression](../../../vm-share/modules/sim1/runs/sim1-state-integration-20261124-007/state-integration-comparison.json)
- [Position regression](../../../vm-share/modules/sim1/runs/sim1-position-20261124-007/position-comparison.json)
