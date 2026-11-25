# FSX shared knowledge

2026-11-25 — Moved the `sim1.dll` electrical circuit query and
bus lookup to their own translation unit. The independent VC8
objects, linked propulsion comparison and x86 fixtures preserve
the existing verification scopes. Ghidra remains saved at
`1795634906543:62a95bf0:520`.
[Electrical circuit availability](sim1.dll/sim1-propeller-step.md#electrical-circuit-availability).

2026-11-25 — Reconstructed `sim1.dll` automatic-feather arming
(`0x20c48270`) and its torque predicate (`0x20c48250`). Connected
arming and icing to the mapped propeller step. VC8 verifies 15 exact
and 27 normalized arming bytes. The x86 fixture passed 3,072
arming scenarios plus boundary and integration checks. Simulator
verification remains open. Ghidra revision
`1795634906543:62a95bf0:520` was saved and read back clean.
[Automatic feather arming](sim1.dll/sim1-propeller-step.md#automatic-feather-arming).

2026-11-25 — Reconstructed the electrical circuit query
(`0x20bfee30`) and bus lookup (`0x20bfef40`) in `sim1.dll`.
Propeller deice now calls the shared query. VC8 verifies 22 exact
and 12 normalized bytes in the query. Automatic-feather preparation
at `0x20c48270` was reconstructed subsequently. Ghidra revision
`1795634311730:62a95bf0:512` was saved and read back clean.
[Electrical circuit availability](sim1.dll/sim1-propeller-step.md#electrical-circuit-availability).

2026-11-25 — Reconstructed the propeller icing load arithmetic
at `sim1.dll:0x20c48710`. VC8 verifies 32 exact and 35 normalized
instruction bytes and 40 exact rate-table bytes. Eighteen host x86
checks passed. That build used a Boolean result supplied by the
caller. The current circuit-integrated build supersedes it.
Ghidra revision `1795633100703:62a95bf0:354` was saved and read
back clean.
[Propeller icing load factor](sim1.dll/sim1-propeller-step.md#propeller-icing-load-factor).

2026-11-25 — Integrated the mapped propeller passes at
`sim1.dll:0x20c47456`–`0x20c47713`. VC8 verifies 21 exact and
42 normalized bytes, including all six downstream calls. Twelve
host x86 checks passed. That build omits both preparation calls,
which were connected in the later mapped-step reconstruction.
Simulator verification remains open. Ghidra
revision `1795632255946:62a95bf0:349` was saved and read back clean.
[Integrated mapped propeller passes](sim1.dll/sim1-propeller-step.md#integrated-mapped-propeller-passes).

2026-11-25 — Reconstructed the ground selector at
`sim1.dll:0x20c1cbf0`, resolved its concrete virtual slot, and
integrated the pitch stages into `0x20c47720`. Native VC8 verifies
44 exact selector bytes, 14 exact dispatcher bytes, and a five-byte
normalized ground-selector call in the governor. The enclosing
propeller step `0x20c47420` and simulator verification remain open.
Ghidra revision `1795631628753:62a95bf0:346` was saved and read
back clean.
[Ground contact selector and integrated blade-pitch update](sim1.dll/sim1-propeller-step.md#ground-contact-selector-and-integrated-blade-pitch-update).

2026-11-25 — Reconstructed feather-state transitions at
`sim1.dll:0x20c485c0` and its low-speed latch caller range at
`0x20c47818`. VC8 confirms 93 exact and 44 normalized bytes across
the two scopes. The ground-limit decision and complete blade-pitch
dispatcher remain open. Ghidra revision `1795630948540:62a95bf0:239`
was saved and read back clean.
[Feather state and low-speed latch](sim1.dll/sim1-propeller-step.md#feather-state-and-low-speed-latch).

2026-11-25 — Reconstructed advance ratio, the governor calculation,
and fixed-pitch cruise selection from `sim1.dll:0x20c47720`. Native
VC8 verifies 43 exact and 89 normalized bytes. The feather-state
helper, latch update and ground-limit query remain open. The 17
host x86 checks passed. Ghidra revision `1795629219460:62a95bf0:234`
was saved and read back clean.
[Blade pitch and advance ratio](sim1.dll/sim1-propeller-step.md#blade-pitch-and-advance-ratio).

2026-11-25 — Reconstructed propeller thrust publication at
`sim1.dll:0x20c47c90`. Native VC8 COFF verifies 14 exact and
108 normalized instruction bytes plus 380 exact curve-data bytes.
Host x86 tests and the propulsion and aerodynamics comparisons pass.
The full body and simulator verification remain open. Ghidra
revision `1795615726054:62a95bf0:230` was saved and read back clean.
[Propeller thrust publication](sim1.dll/sim1-propeller-step.md#propeller-thrust-publication).

2026-11-25 — Reconstructed the propeller aerodynamic torque
at `sim1.dll:0x20c47ae0` and its AIR coefficient lookup at
`0x20bcc030`. Native VC8 COFF verifies 26 exact and 39 normalized
bytes for the torque and 29 exact bytes for the lookup. Full-body
and simulator verification remain open. Ghidra revision
`1795615039980:62a95bf0:227` was saved and read back clean.
[Aerodynamic propeller torque](sim1.dll/sim1-propeller-step.md#aerodynamic-propeller-torque).

2026-11-25 — Reconstructed propeller angular-speed integration at
`sim1.dll:0x20c47ba0` and verified the phase wrap through `util.dll`
ordinal 199. Native VC8 COFF confirms 37 exact and 161 normalized
bytes. Full-body and simulator verification remain open. Ghidra
revision `1795614553364:62a95bf0:219` was saved and read back clean.
[Propeller speed integration](sim1.dll/sim1-propeller-step.md#propeller-speed-integration).

2026-11-25 — Mapped the complete original propeller step at
`sim1.dll:0x20c47420` and reconstructed its synchronization
helper at `0x20c48150`. Native VC8 COFF verifies 36 exact and
21 normalized bytes. The full helper differs. The step phase map
also corrects the attribution of the integrator's `+0x3c` read.
Ghidra revision `1795613966931:62a95bf0:214` was saved and read
back clean. The full step and simulator verification remain open.
[Mapped propeller step and speed synchronization](sim1.dll/sim1-propeller-step.md).

2026-11-25 — Reconstructed the propeller induced-airspeed writer
at `sim1.dll:0x20c480b0`, which feeds the tail slipstream update.
Native VC8 COFF provides 72 exact and 33 normalized bytes.
The full body differs and simulator verification remains open.
Ghidra revision `1795613240321:62a95bf0:207` was saved and read
back clean. The enclosing propeller step remains open.
[Propeller induced airspeed](sim1.dll/sim1-engine-branches.md#propeller-induced-airspeed).

2026-11-25 — Reconstructed the seven flap-group coefficients
at `sim1.dll:0x20c00840`. The flap parser and runtime list
supersede the earlier slipstream attribution. Three coefficient
getters give 45 exact bytes and ten further ranges give 96
normalized bytes. The complete body differs. Ghidra revision
`1795612398258:62a95bf0:203` was saved and read back clean.
[Flap-group aerodynamic coefficients](sim1.dll/sim1-flap-groups.md).

2026-11-25 — Reconstructed the tail slipstream speed and pressure
update at `sim1.dll:0x20c45760`. The native VC8 comparison gives
48 exact and 12 normalized bytes. The full body differs. Ghidra
revision `1795611579878:62a95bf0:13` was saved and read back clean.
Simulator verification remains open. The separately analyzed
`0x20c00840` was identified as the flap-group coefficient producer.
[Tail slipstream speed and pressure](sim1.dll/sim1-aerodynamic-forces.md#tail-slipstream-speed-and-pressure).

2026-11-25 — Reconstructed the high-alpha reference correction
in `sim1.dll:0x20c00530`, including both curves, rudder scaling
and the zeroing path. Native COFF comparison gives 5 exact and
70 normalized bytes. Ghidra revision `1795610815925:62a95bf0:9`
was saved and read back clean. Both blocks of the function are
reconstructed, with full-body matching and simulator verification open.
[High-alpha reference correction](sim1.dll/sim1-aerodynamic-moment.md#high-alpha-reference-correction).

2026-11-25 — Reconstructed nine pressure and rate scales in
`sim1.dll:0x20c00530`, from `0x20c0054f` through `0x20c005f2`.
The VC8 COFF comparison verifies all output stores and contains
19 exact bytes in five ranges. The full extracted body differs.
Ghidra revision `1795609326019:62a95bf0:6` was saved and read back
clean. Simulator verification remains open. The high-alpha
reference correction was reconstructed next.
[Pressure and rate scales](sim1.dll/sim1-aerodynamic-forces.md#pressure-and-rate-scales).

2026-11-25 — Reconstructed the normal airplane lift, drag and
side-force calculations in `sim1.dll` and their body-axis
publication in `0x20c45510`. The lift calculation publishes four
wing loads shared with roll. The native COFF comparison contains
72 exact and 64 normalized bytes. The full force bodies and the
extracted output scope differ. Ghidra revision
`1795569949604:d6cfccaa:649` was saved and read back clean.
Linked force matching and simulator verification remain open.
[Normal airplane aerodynamic forces](sim1.dll/sim1-aerodynamic-forces.md).

2026-11-25 — Reconstructed normal airplane yaw composition in
`sim1.dll:0x20c020a0`, including propeller gyroscopic reaction,
P-factor, external tanks and both return paths. Native COFF
comparison gives 69 exact and 79 normalized bytes. The full yaw
body differs. Ghidra revision `1795569073366:d6cfccaa:495` was
saved and read back clean. All three normal-model moment sums
now have C++ reconstructions. The alternate airplane model,
linked yaw matching, and simulator verification remain open.
[Normal airplane yaw moment](sim1.dll/sim1-yaw-moment.md).

2026-11-25 — Reconstructed the normal airplane roll composition in
`sim1.dll:0x20c01890`, including wing asymmetry, propeller reaction
torque, force transfer and the `SimpleAero` return. Native COFF
comparison gives 82 exact and 94 normalized bytes. The full roll
body differs. This analysis corrected four pitch callback bindings
to their actual slots and verified five 17-byte pitch accessors.
Ghidra revision `1795568104133:d6cfccaa:350` was saved and read back
clean. Linked and simulator roll verification and the alternate
model remain open. The yaw composition was reconstructed next.
[Normal airplane roll moment](sim1.dll/sim1-roll-moment.md).

2026-11-25 — Completed the C++ reconstruction of the normal
airplane pitch moment in `sim1.dll:0x20c013a0`, including the
original summation order and the `SimpleAero` path. Four
coefficient accessors match all 68 bytes in native VC8 COFF.
The new comparisons total 120 exact and 49 normalized bytes.
The complete pitch body still differs and runtime verification
remains open. The normal-model roll and yaw bodies retain
their scoped force-transfer reconstructions. Ghidra revision
`1795566771278:d6cfccaa:205` was saved and read back clean. The
pitch slot bindings in this build were superseded by the roll work above.
[Complete pitch moment calculation](sim1.dll/sim1-aerodynamic-moment.md#complete-pitch-moment-calculation).

2026-11-25 — Reconstructed the `sim1.dll` elevator trim
coefficient at `0x20c501a0` and its contribution to `0x20c013a0`.
The native VC8 comparison contains 35 exact bytes and 16
normalized bytes across seven disjoint ranges. The complete
bodies still differ. The dynamic-pressure curve writer and
simulator verification remain open. Ghidra revision
`1795565874672:d6cfccaa:11` was saved and read back clean.
[Elevator trim contribution](sim1.dll/sim1-aerodynamic-moment.md#elevator-trim-contribution).

2026-11-25 — Reconstructed the `sim1.dll` horizontal-tail
incidence coefficient at `0x20c500c0` and its contribution to
`0x20c013a0`. The native VC8 comparison gives 49 exact bytes
and 16 normalized bytes across seven disjoint ranges. Both
complete comparisons differ. The elevator trim term was reviewed
in the subsequent entry. Simulator verification remains open.
Ghidra revision
`1795565103483:d6cfccaa:7` was saved and read back clean.
[Horizontal-tail incidence contribution](sim1.dll/sim1-aerodynamic-moment.md#horizontal-tail-incidence-contribution).

2026-11-25 — The `fsx.exe` build now stages source from `src/fsx/`.
The broad `api.dll` source pass has been retired from active source.
The opt-in `-SourcePass` profile stages the archived snapshot for the
existing COFF regressions. The scope of the recorded comparisons is
unchanged. This workspace cleanup did not open or save a Ghidra
database. [API source pass](api.dll/api-source-pass.md).

2026-11-24 — Reconstructed the `sim1.dll` angle-of-attack rate
from the airflow and acceleration states, the coefficient at
`0x20c50020` and its scaled contribution to `0x20c013a0`.
Seven disjoint scopes total 88 exact bytes. Three literal-backed
scopes total 18 normalized bytes. The complete comparisons,
sideslip-rate reconstruction and simulator verification remain open.
Ghidra revision `1795550562132:962445d7:188` was saved and read
back clean.
[Angle-of-attack rate contribution](sim1.dll/sim1-aerodynamic-moment.md#angle-of-attack-rate-contribution).

2026-11-24 — Reconstructed the `sim1.dll` pitch-rate coefficient
at `0x20c4fe00` and its scaled contribution to `0x20c013a0`.
Traced the integrated X rate and the airflow and geometry scale.
The control-modifier call and complement match across 14 exact
bytes. Three literal-backed scopes total 30 normalized bytes.
Both complete comparisons still differ. The modifier providers'
physical outputs and simulator verification remain open. Saved
Ghidra revision `1795550031888:962445d7:182` was read back clean.
[Pitch-rate contribution](sim1.dll/sim1-aerodynamic-moment.md#pitch-rate-contribution).

2026-11-24 — Reconstructed the `sim1.dll` elevator moment
coefficient at `0x20c4fce0` and its contribution to `0x20c013a0`.
Verified elevator and trim inputs, AIR records and the configuration
scalar. Three disjoint ranges total 20 exact bytes. Three further
ranges total 30 normalized bytes. Both full comparisons still differ.
Ghidra revision `1795549312928:962445d7:178` was saved and read back
clean. The other control and rate terms and simulator verification
remain open.
[Elevator contribution](sim1.dll/sim1-aerodynamic-moment.md#elevator-contribution).

2026-11-24 — Reconstructed the `sim1.dll` zero-alpha moment
coefficient at `0x20c4ff30` and its scaled contribution to
`0x20c013a0`. The 20-byte realism blend is exact. The Mach
scale and optional modifier have two disjoint normalized ranges
of 6 and 16 bytes. The complete function and extracted caller
still differ. The optional provider's physical meaning, the
remaining moment terms and simulator verification are open.
Ghidra revision `1795548514295:962445d7:174` was saved and read
back clean. [Zero-alpha moment coefficient](sim1.dll/sim1-aerodynamic-moment.md#zero-alpha-moment-coefficient).

2026-11-24 — Verified the `sim1.dll` alpha filter at `0x20c2e0b0`
as a first-order forward-Euler update with gain 8.0 and a
conditional raw-angle path. The dashboard now opens scoped
disassembly directly on the parent function and links to the
retained build dump. Ghidra was read-only and saved revision
`1795546801864:962445d7:167` was read back clean.
[Alpha filter](sim1.dll/sim1-aerodynamic-moment.md#alpha-filter).

2026-11-24 — Traced the `sim1.dll` angle-dependent moment coefficient
to AIR record `0x1546` or fallback `0x473`. Reconstructed the
curve interpolation and scaled contribution to `0x20c013a0`.
The final 16 interpolation bytes match exactly. The 6-byte
angle conversion matches after validating its constant address.
The separate baseline is covered under the zero-alpha coefficient.
The remaining coefficient terms and simulator verification are open.
Ghidra revision `1795546801864:962445d7:167` was saved and read back clean.
[Angle-dependent moment coefficient](sim1.dll/sim1-aerodynamic-moment.md#angle-dependent-moment-coefficient).

2026-11-24 — Reconstructed the force-vector transfer terms in all
three `sim1.dll` airplane aerodynamic moment components. Confirmed
both model reference queries and the high-angle correction sources.
The X comparison has a 9-byte normalized scope. The Z comparison
has 13 exact bytes in two ranges and a 6-byte normalized range.
The Y arithmetic still differs. The gyro scope reproduced. Ghidra
revision `1795546154148:962445d7:163` was saved and read back clean.
[Force transfer to the moment reference](sim1.dll/sim1-aerodynamic-moment.md#force-transfer-to-the-moment-reference).

2026-11-24 — Reconstructed the propeller gyroscopic contribution
inside `sim1.dll` aerodynamic moment function `0x20c013a0`.
Twenty bytes of the remainder loop and cleanup match exactly in
VC8 native COFF. The 336-byte original slice and 343-byte extracted
body still differ overall. Traced both the propeller settings and
the pressure and geometry input chain. Ghidra revision
`1795545337374:962445d7:158` was saved and read back clean.
[Airplane aerodynamic moment](sim1.dll/sim1-aerodynamic-moment.md).

2026-11-24 — Confirmed the airplane model table at `0x20bc2160`
and three concrete aerodynamic moment calculations. Reviewed the
ground-contact and helicopter rotor producers as larger reconstruction
targets. Their equations and runtime coverage remain open. Ghidra
was read-only and the saved revision was read back clean.
[Flight model entry points](sim1.dll/sim1-forces-and-moments.md#larger-model-entry-points).

2026-11-24 — Reconstructed the `sim1.dll` turbine output helper at
`0x20c2bc20`. The 21-byte high-speed threshold branch matches in
native COFF with one verified literal address. The entire body
still differs across 94 original and 91 native COFF bytes.
Ghidra revision `1795543912564:962445d7:153` is saved and read back
clean. The measurement name, linked comparison and runtime remain open.
[Turbine speed-scaled output](sim1.dll/sim1-engine-branches.md#turbine-speed-scaled-output).

2026-11-24 — Reconstructed the `sim1.dll` propulsion force collector
at `0x20c010f0`, including the optional propeller arm offset. The
83-byte output copy and return match exactly. The complete callback
still differs across a 674-byte original and 626-byte linked body.
The 215-byte projection helper retained its complete normalized
match in the reconstructed caller. Ghidra revision
`1795533674228:962445d7:149` was saved and read back clean.
[Propulsion force collector](sim1.dll/sim1-propulsion-collector.md).

2026-11-24 — Reconstructed the `sim1.dll` thrust projection at
`0x20c01010`. The complete 215-byte linked body matches after
verifying four CRT call displacements. Its 70-byte force and moment
accumulation block is exact. Force, position and integration
regression comparisons preserved the reviewed scopes. Ghidra
revision `1795531656408:962445d7:146` was saved and read back clean.
Runtime verification and the larger engine update bodies remain open.
[Thrust projection](sim1.dll/sim1-engine-branches.md#thrust-projection-and-accumulation).

2026-11-24 — Mapped the `sim1.dll` engine-type reader and all six
main dispatch slots. Piston selects a separate update. Jet,
Turboprop and Helo-Turbine with a null rotor configuration pointer
share the turbine core. Verified the separate rotor callback and
the engine and propeller thrust inputs to the force collector.
The 23 tagged functions and original dispatch table are in the
dashboard. Ghidra revision `1795530407241:962445d7:143` is saved
and read back clean. The thrust projection helper is the next
matching target, with runtime verification still open.
[Engine branch mapping](sim1.dll/sim1-engine-branches.md).

2026-11-24 — Reconstructed the `sim1.dll` force and moment accumulator,
callback adapter and mass update. The linked adapter matches all 36
bytes exactly. The 192-byte collection body matches with both API
ordinal 109 references validated. Mass publication matches across
59 normalized bytes and the acceleration history copy across 66 exact
bytes. The wider mass and acceleration bodies and the individual
producer equations remain open. Position and integration regressions
passed. Ghidra tags were saved and read back clean.
[Forces and moments](sim1.dll/sim1-forces-and-moments.md).

2026-11-24 — Reconstructed the `sim1.dll` world-velocity, longitude
and altitude writers. The 86-byte velocity helper matches the linked
LTCG body with one validated CRT call. Longitude has a 31-byte
normalized linked range and altitude has a 22-byte exact COFF range.
Whole-body coordinate differences and the complete position and
attitude caller remain open. The integration regression preserved
three full exact matches and the history-copy range. Ghidra tags
were saved and read back clean. [Position writers](sim1.dll/sim1-position-writers.md).

2026-11-24 — Exported the complete current `sim1.dll` function model,
with 2,496 internal functions and 246 external declarations. Verified
25 PE exports and 250 import slots and reconciled the four imports
outside Ghidra's external function table. The dashboard carries the
full internal inventory and prior comparison evidence. The first
proposed reconstruction is `0x20bd1d90` and the position helpers
called by `0x20bdec10`. Ghidra was read-only and read back saved.
[Function inventory](sim1.dll/sim1-function-inventory.md).

2026-11-24 — Located aircraft state integration in `sim1.dll`.
The selected VC8 RTM object matches three complete heading-transform
methods (232 bytes) and the integrator's 144-byte acceleration-history
copy. Acceleration preparation and velocity integration remain
different at full-body scope. The position and attitude step at
`0x20bdec10` is inventoried. The dashboard includes source, original and
rebuilt listings, and Ghidra tags. `sim1.dll` is saved and read back clean.
[State integration](sim1.dll/sim1-state-integration.md).

2026-11-24 — Traced the `sim1.dll` magnetic-variation call into
`fe.dll` ordinal 4 and reconstructed the FE query family. Ordinal 3
matches 17 linked bytes after target validation. Four other bodies
differ. The live `fe.dll` was already unsaved before tagging and remains
unsaved. The tagged model is retained in a GZF. Linked full modules and
runtime verification remain open for both work areas.
[FE lookup](fe.dll/fe-magnetic-variation.md).

2026-11-24 — Three previously Inventoried `api.dll` mapped-file
and interface cleanup bodies match across 63 native COFF bytes
with three verified calls and one verified tail jump. Inventoried
falls to 4,257 and Normalized rises to 109. The handle predicate,
linked DLL and runtime remain open. Ghidra was read-only and its
current save state was not independently verified.
[Mapped file and interface cleanup adapters](api.dll/api-pointer-owner-updates.md#mapped-file-and-interface-cleanup-adapters).

2026-11-24 — Four previously Inventoried `api.dll` buffer owner
forwarding and reset bodies match across 89 native COFF bytes with
four verified direct calls. Inventoried falls to 4,260 and
Normalized rises to 106. The dashboard has both listings and
explicit source links. The reset's two state-field roles, linked
DLL and runtime remain open. Ghidra was read-only and the current
save state was not independently verified.
[Buffer owner forwarding and reset](api.dll/api-pointer-owner-updates.md#buffer-owner-forwarding-and-reset).

2026-11-24 — The `api.dll` COM pointer chain at `0x20161815`,
`0x201611be` and `0x20161203` was compared against stock VC8 RTM
ATL and `_com_ptr_t` headers. Both reproduce the transfer caller
encoding with its call operand aligned. Neither reproduces the
complete chain. Original library provenance remains open. Ghidra was
read-only and no new save-state readback was obtained.
[VC8 COM smart-pointer provenance check](api.dll/api-pointer-owner-updates.md#vc8-com-smart-pointer-provenance-check).

2026-11-24 — Five previously Inventoried `api.dll` activity and
lock lease bodies normalize across 203 code bytes. The 35-byte wait
helper at `0x2014eb01` has four register-selection byte differences
after both import fields are validated. Inventoried falls to 4,264,
Normalized rises to 102 and Different rises to 459. Prior gates and
all 20 source-pass checks pass. Read-only Ghidra reported saved
revision `1793741866691:0852fe08:3`. Linked DLL and runtime remain open.
[Activity and critical-section leases](api.dll/api-pointer-owner-updates.md#activity-and-critical-section-leases).

2026-11-24 — Seven previously Inventoried `api.dll` mapped-view
and handle-transfer bodies match all 208 native COFF bytes through
ten validated relocation fields. Inventoried falls to 4,270 and
Normalized rises to 97. Previous owner and constructor gates and
all 20 source-pass checks pass. The predicate at `0x20173c46`,
linked DLL and runtime remain open. Read-only Ghidra reported saved
revision `1793741866691:0852fe08:3`.
[Mapped view and handle transfer](api.dll/api-pointer-owner-updates.md#mapped-view-and-handle-transfer).

2026-11-24 — Seven `api.dll` pointer-owner adapters match all 171
native COFF bytes after validating five direct calls and two imports.
Inventoried falls from 4,283 to 4,277, Different to 458 and Normalized
rises to 90. The prior pointer-owner and constructor groups and all
20 source-pass checks pass. Read-only Ghidra reported saved revision
`1793741866691:0852fe08:3`. Linked DLL and runtime remain open.
[Adjacent adapters](api.dll/api-pointer-owner-updates.md#adjacent-pointer-owner-adapters).

2026-11-24 — Four `api.dll` heap bridges match all 110 native COFF
bytes after seven validated relocation fields. Both previously
unresolved owner-free endpoints have source and match evidence.
Inventoried falls to 4,283, Different to 459 and Normalized rises to
83. The ten earlier pointer-owner matches and all 20 source-pass
checks pass. Read-only Ghidra reported saved revision
`1793741866691:0852fe08:3`. Linked DLL and runtime remain open.
[Heap bridge endpoints](api.dll/api-pointer-owner-updates.md#heap-bridge-endpoints).

2026-11-24 — Six `api.dll` owner release and replacement functions
match 136 native COFF bytes, with one Exact and five Normalized
bodies. Inventoried falls from 4,287 to 4,284 and Different drops
from 465 to 462. The four prior update bodies retain their matches
with the new helpers defined. All 20 source-pass checks pass.
Ghidra remained saved at `1793741866691:0852fe08:3`. Two heap-free
endpoints, linked DLL and runtime remain open.
[Release follow-up](api.dll/api-pointer-owner-updates.md#release-and-replacement-follow-up).

2026-11-24 — Four previously Inventoried `api.dll` pointer-owner
updates at `0x20161815`–`0x20161878` match all 100 native COFF bytes
after validating four REL32 fields. Inventoried falls from 4,291 to
4,287 and Normalized rises from 70 to 74. Twenty source-pass checks,
the eight constructor matches and seven earlier gates pass. Read-only
Ghidra reports saved revision `1793741866691:0852fe08:3`. Linked DLL
and runtime verification remain open.
[Pointer owner updates](api.dll/api-pointer-owner-updates.md).

2026-11-24 — Eight previously Different `api.dll` default-copy
routines at `0x20161267`–`0x20161356` match all 240 native COFF
bytes after validating eight DIR32 fields against 196 bytes of initial
zero-filled data. Normalized rises from 62 to 70, Different drops from
473 to 465, and Inventoried remains 4,291.
The separate factory replay retains all eight constructor matches,
seven earlier comparison gates and 20 source-pass checks. Read-only
Ghidra reported saved revision `1793741866691:0852fe08:3`. Linked DLL
and runtime verification remain open.
[Default-copy block](api.dll/api-construction-records.md#default-copy-block-at-0x20161267).

2026-11-24 — Eight previously Inventoried `api.dll` constructors
at `0x2016198a`–`0x20161a29` match all 160 native COFF bytes after
validating eight REL32 fields. The Inventoried count drops from
4,299 to 4,291. All 20 source-pass checks and seven prior gates passed.
Copy routine implementations, linked DLL and runtime remain open.
Read-only Ghidra reported saved revision `1793741866691:0852fe08:3`.
[Constructor block](api.dll/api-construction-records.md).

2026-11-24 — The previously Inventoried `api.dll` range check at
`0x201513ba` matches all 21 bytes in VC8 RTM x86 COFF with zero
relocations. The module's Inventoried count drops from 4,300 to
4,299. All 20 source-pass checks and six prior gates passed.
Read-only Ghidra reported saved revision `1793741866691:0852fe08:3`.
Linked DLL and runtime verification remain open.
[Signed-to-unsigned range check](api.dll/api-checked-arithmetic.md#signed-to-unsigned-range-check-at-0x201513ba).

2026-11-24 — The `api.dll` unsigned comparison at `0x20153784`
has a retained 33-byte native COFF diagnostic with ten byte differences
and zero relocations. Pinned SP1 and linked RTM LTCG reproduce the same
differences. The complete match remains open. All 20 source-pass checks
and six prior gates passed. Read-only Ghidra reported saved revision
`1793741866691:0852fe08:3`.
[Unsigned comparison](api.dll/api-checked-arithmetic.md#unsigned-32-bit-comparison-at-0x20153784).

2026-11-19 — The `api.dll` 64-bit subtraction forwarder at
`0x201711d2` matches all 29 bytes in VC8 RTM x86 COFF after validating
the sole REL32 to `0x20171131`. The 20 source-pass checks and five
earlier exact gates passed. Linked DLL and runtime verification remain
open. Read-only Ghidra reported saved revision `1793741866691:51f17f6c:3`.
[64-bit subtraction forwarder](api.dll/api-checked-arithmetic.md#64-bit-subtraction-forwarder-at-0x201711d2).

2026-11-19 — `api.dll` checked 64-bit subtraction at `0x20171131`
matches all 50 bytes in VC8 RTM x86 COFF with zero relocations. All 20
source-pass checks and four previous exact gates passed. Linked DLL and
runtime verification remain open. Read-only Ghidra reported saved
revision `1793741866691:51f17f6c:3`.
[Checked 64-bit subtraction](api.dll/api-checked-arithmetic.md#unsigned-64-bit-subtraction-at-0x20171131).

2026-11-19 — `api.dll` checked subtraction at `0x2015347a` matches
all 34 bytes in VC8 RTM x86 COFF with zero relocations. The 20 source-pass
checks and all three previous exact gates passed. Linked DLL and runtime
verification remain open. Read-only Ghidra reported saved revision
`1793741866691:51f17f6c:3`.
[Checked subtraction](api.dll/api-checked-arithmetic.md#unsigned-subtraction-at-0x2015347a).

2026-11-19 — `api.dll` checked addition at `0x2014e674` matches all
35 bytes in VC8 RTM x86 COFF with zero relocations. The 20 source-pass
checks and both prior exact gates passed. Ordinal 160 remains five
alignment bytes longer in the closest native and linked builds.
Ghidra reported saved revision `1793741866691:51f17f6c:3`.
[Checked arithmetic](api.dll/api-checked-arithmetic.md).
[String comparison](api.dll/api-string-compare.md).

2026-11-19 — `api.dll` ordinal 172 at `0x2013bdcc` matches all
34 bytes in VC8 RTM x86 COFF with zero relocations. All 20 source-pass
checks and the `ErfSetCodes` exact gate passed. Linked DLL and runtime
verification remain open. Read-only Ghidra reported saved revision
`1793741866691:51f17f6c:3`. [Callback array lookup](api.dll/api-dialog-registry.md#ordinal-172-record-lookup).

2026-11-19 — `api.dll` matches all 30 bytes of `ErfSetCodes` at
`0x2014a51b` in VC8 RTM x86 COFF with zero relocations. All 20 source-pass
regression checks passed. Linked DLL and runtime verification remain open.
Read-only Ghidra reported saved revision `1793741866691:51f17f6c:3`.
[FDI error record](api.dll/api-cab-error.md).

2026-11-19 — Live verified for fsx.exe. The packaged VC8 RTM replay and independent repeat reproduce all 137 IAT identities at their original slots and all 80 six-byte import thunks at their original addresses. The 146 dashboard scopes now contain 145 Exact and one Different. ShowErrorMessage retains its single JL/JS opcode difference at 0x01001e0e. [Linked-byte audit](../../artifacts/fsx.exe/iat-complete-20261119/audit.json).

2026-11-19 — Live verified for fsx.exe at 0x010021ac and 0x01002c38. The packaged RTM getter build also reproduces both six-byte MFC thunks at their original VAs using the separate AfxWinMain import and thunk archive. All seven getter references remain exact. Three CRT imports rotate, affecting InitInstance, the CRT exception thunk and AfxInitialize. The dashboard retains the stronger overall build. [Comparison](../../artifacts/fsx.exe/mfc-getter-pair-20261119/result.json).

2026-11-19 — `fsx.exe` now links `pfnGetApplication` at its original
VA `0x01004384`. All seven reference instructions match byte for byte.
The packaged RTM comparison has 143 Exact, two Normalized and one
Different across 146 code scopes. The two MFC thunk operands and the
`ShowErrorMessage` condition byte remain open. Read-only Ghidra reported
saved revision `1795013130661:dcf4182e:3`.
[Getter placement and verification](fsx.exe/data-contribution-alignment.md#2026-11-19-application-getter-at-0x01004384).

2026-11-18 — `fsx.exe` matches all 668 read-only bytes from
`0x0100127c` through `0x01001517`, including literals, both startup
vtables and the MFC message map. InitInstance has four remaining raw
bytes, all in references to the application getter cell. The packaged
link retains the 135 aligned import slots, 80 thunk addresses and exact
initial `.data`. The dashboard reports 136 Exact, nine Normalized and
one Different. Read-only Ghidra reported saved revision
`1795013130661:2514814d:58`.
[Startup read-only block](fsx.exe/exe-data-constants.md#2026-11-18-exact-startup-read-only-block).

2026-11-18 — Isolated the final MFC IAT pair for `fsx.exe`. An
independent import-pointer and native jump-stub trial reproduces both
MFC slots, all 80 thunk addresses and initial `.data`. Three CRT
slots move in that trial, so the admitted 135-slot profile remains
the dashboard comparison. Read-only Ghidra revision remains
`1795013130661:2514814d:58`.
[MFC pair isolation](fsx.exe/import-selection.md#2026-11-18-remaining-mfc-pair-isolation).

2026-11-18 — MFC support-library precedence resolves the remaining
CRT IAT pair for `fsx.exe`. Updated `IatOrder` reproduces 135 of 137
slots, all 80 thunk addresses, and initial `.data`. The MFC pair
`#5230` / `#1207` remains swapped. The dashboard now reports 126 Exact,
19 Normalized and one Different. Read-only Ghidra reports saved
revision `1795013130661:2514814d:58`.
[Final import-pair trials](fsx.exe/import-selection.md#2026-11-18-final-import-pair-trials).

2026-11-18 — `fsx.exe` opt-in `IatOrder` reduces shifted IAT slots
from 90 to four in the repeated packaged ATL-first RTM link. All 80
thunk addresses and initial `.data` bytes still agree. Seventy-seven
thunks are byte-exact. The live dashboard now reports 124 Exact,
21 Normalized and one Different. Remaining IAT differences are one
MFC and one CRT pair. Ghidra was read-only and reports saved revision
`1795013130661:2514814d:58`.
[VC8 IAT order](fsx.exe/import-selection.md#2026-11-18-vc8-iat-order).

2026-11-18 — Recompared the 146 `fsx.exe` dashboard code entries and
ranges against the packaged ATL-first RTM image. The dashboard now shows
55 Exact, 90 Normalized and one Different. The original and rebuilt PE
both strip base relocations. The linked IAT still moves 90 of 137 imports,
and 67 of 80 address-aligned thunks reference different slots. All
512 stored `.data` bytes agree while thirteen mapped object addresses
still differ. Read-only Ghidra reported saved revision
`1795013130661:2514814d:58`.
[Linked audit](fsx.exe/pe-imports-and-directories.md#2026-11-18-packaged-link-and-dashboard-audit).

2026-11-18 — The VC8 RTM link and Microsoft BinPlace reproduce the
`fsx.exe` CodeView basename, 40 zero bytes, and SafeSEH address and
contents in the same build. A fixed intermediate PDB path reproduced
the result in two differently named XP jobs. The `.data` initial image
and 80 thunk entry addresses still match. Original GUID and timestamps,
resource payload and shipping tool provenance remain open. The
packaging trial uses diagnostic `VERSIONINFO`; standard defaults are
retained. Read-only Ghidra reported saved revision
`1795013130661:2514814d:58`.
[BinPlace reproduction](fsx.exe/codeview-gap.md#2026-11-18-binplace-reproduction-and-repeat).

2026-11-18 — A resource control added `.rsrc` without moving the
`fsx.exe` SafeSEH or ATL thunk. The RTM ATL-first link with the worker
PDB path placed both at their original addresses, along with all 80
import thunks and the image entry. The entire `.data` initial image
matches byte for byte in that control. Thirteen mapped data objects
remain shifted. The original 32-byte CodeView record and following
40 zero bytes remain to be reproduced together with this placement.
Read-only Ghidra reported saved revision `1795013130661:2514814d:58`.
[Resource and PDB path controls](fsx.exe/codeview-gap.md#2026-11-18-resource-and-pdb-path-controls).

2026-11-18 — `fsx.exe` link-order trials kept PE base relocations absent.
Explicit CRT/MFC object order aligned the twelve CRT targets from argc
through fmode, but regressed raw data equality and thunk placement.
ATL-first remains the best writable-image result. Its ATL code target
is still 32 bytes early. Source and build defaults were retained.
Read-only Ghidra reported saved revision `1795013130661:2514814d:58`.
[Link-time address trials](fsx.exe/data-contribution-alignment.md#2026-11-18-link-time-address-ordering-trials).

2026-11-18 — The `fsx.exe` ATL-first direct-object link reproduces
`.data` geometry and initial bytes after one validated pointer field.
Thirteen mapped object addresses differ by four bytes. The XP replay
passed InitInstance, regressions and all 80 thunk placement checks.
The profile is opt-in. Read-only Ghidra state was saved at
`1795013130661:2514814d:58`.
[Writable section comparison](../../artifacts/fsx.exe/writable-object-order-20261118/match.json).

2026-11-18 — Rechecked the current `fsx.exe` writable layout using the
EXE-paired PDB, MAP and live references. Getter-only non-GL and ATL ordering
controls pass all gates at 1200 virtual bytes. Startup-only non-GL produces
1188 bytes but fails the application initializer constructor-target check.
Canonical data remains 1200 versus original 1184. Read-only Ghidra save
state verified at `1795013130661:2514814d:58`.
[Writable layout follow-up](fsx.exe/data-contribution-alignment.md#2026-11-18-current-rtm-layout-and-isolated-controls).

2026-11-18 — Verified the remaining `fsx.exe` dashboard code ranges
at `0x010024dd`, `0x01002529` and `0x010029c2` in a fresh XP RTM
build. Corrected three historical rebuilt data addresses in the function
view and paired six original ranges in Global data. The CRT, InitInstance
and established regression gates passed. Read-only Ghidra reported a saved
database at `1795013130661:2514814d:58`.
[Code evidence](../../artifacts/fsx.exe/crt-documented-remainder-20261118/code-matches.json).

2026-11-18 — The embedded CRT PE image filter range in `fsx.exe`
at `0x010028fd` is raw-exact across 23 bytes in the fresh XP RTM link.
The CRT scope table and existing regression gates passed. Ghidra
readback reported a saved database at `1795013130661:2514814d:54`.
[CRT image helper evidence](fsx.exe/crt-image-helpers.md#2026-11-18--embedded-filter-dashboard-comparison).

2026-11-18 — The D3DX export lookup and call in `fsx.exe` at
`0x01001a51` match across 26 linked bytes after validating two operands.
The full `InitInstance` comparison passed on the retained RTM build.
Ghidra block comment saved at `1795013130661:2514814d:53`.

2026-11-18 — The `fsx.exe` D3DX check at `0x01001a44` matches across
67 linked bytes after five address fields are validated. The fresh RTM
build also passed the 769-byte parent method and controller regressions.
Ghidra block comment saved at `1795012585183:2514814d:51`.
[D3DX evidence](fsx.exe/initinstance-matching.md#2026-11-18--d3dx-check-at-0x01001a44).

2026-11-18 — `fsx.exe` `AfxInitialize` at `0x01002c0d` matches across
all 38 linked bytes after validating two import references. VC8 RTM
`/OPT:NOICF` independently verifies the reconstructed source body.
Both ICF modes passed `InitInstance` and controller regressions. Ghidra
name, prototype and plate comment saved at `1795011990050:2514814d:49`.
Runtime and COFF comparisons remain open.
[MFC module initialization](fsx.exe/fsx-mfc-initialize.md).

2026-11-18 — Verified job-local VC8 SP1 for `fsx.exe`. The pinned SP1
integration passed the complete `InitInstance` match and the established
regression checks. In `ShowErrorMessage`, SP1 and RTM leave the same
one-byte residual at `0x01001e0e`. The tested control-flow and optimization
variants gave no closer match. A source form that shares the test block
and emits `JL` remains open. Ghidra readback reported saved revision
`1793671788023:2514814d:3`.
[Pinned SP1 and control-flow evidence](fsx.exe/startup.md#pinned-sp1-and-control-flow-trials).

2026-11-18 — The `fsx.exe` `ShowErrorMessage` follow-up reproduced the
one-byte RTM residual at `0x01001e0e`. Signed comparisons emitted the
original `JL` with a different preceding compare. The best C++ source
was retained, and the linked `InitInstance` comparison passed again.
The guest currently reports RTM for the installed SP1 profile. Ghidra
readback reported saved revision `1793671788023:2514814d:3`.
[Follow-up](fsx.exe/startup.md#follow-up-on-the-branch-opcode).

2026-11-18 — fsx.exe ShowErrorMessage at 0x01001d30 now has one residual byte in the canonical RTM build. The original JL and rebuilt JS follow TEST EAX,EAX. InitInstance retains its complete normalized match. Ghidra readback was saved. [Callback comparison](fsx.exe/startup.md#rtm-callback-comparison-2026-11-18).

2026-11-17 — `fsx.exe` `FSXApp::FSX_InitInstance` at `0x010019e0` is
**Normalized** across all 769 linked bytes in both VC8 SP1 and RTM.
All 44 address fields are target-validated and all 15 internal jumps
are byte-identical. This supersedes the 777-byte November 3 result.
The original compiler and SDK revision remain open. Runtime testing is
outstanding. Read-only Ghidra readback confirmed saved revision
`1793671788023:3045b49a:3`. [Method and evidence](fsx.exe/initinstance-matching.md).

2026-11-17 simscheduler.dll `FUN_20cc15f0` remains Diagnostic.
Linking the reconstructed pool acquisition restores CALL/RET.
RTM and SP1 emit 10 bytes with one residual byte after validating
the call target, PUSH EAX versus PUSH ECX. Stack versus EBX
storage passing is open. Splitting the LTCG units retains the
same difference. Ghidra read-only, save state unverified.
See [function coverage](simscheduler.dll/simscheduler-function-coverage.md).

2026-11-17 simscheduler.dll `FUN_20cc1400` is Normalized across
all 88 VC8 RTM LTCG bytes after verifying three calls and one HIGHLOW
object address. The source-identical SP1 control stays Diagnostic at
86 bytes. Full DLL linkage and runtime are open. Ghidra read-only,
save state unverified. See [event dispatch](simscheduler.dll/simscheduler-event-dispatch-match.md).

2026-11-17 simscheduler.dll live-list removal at 0x20cc3b70
matches all 78 linked VC8 LTCG bytes after validating the sole
api.dll ordinal 110 operand. Native COFF remains Diagnostic.
Ghidra read-only, save state unverified.

2026-11-17 simscheduler.dll propagation helper 0x20cc2fa0
is Exact across all 72 linked VC8 LTCG bytes, including loop
alignment. The caller at 0x20cc2c40 remains Normalized.
Ghidra read-only, save state unverified.

2026-11-17 simscheduler.dll update setter 0x20cc2c40 matches
all 50 linked VC8 SP1 LTCG bytes after validating one REL32 call.
The propagation helper now converts the time once before its entry
loop and copies the resulting representation. LTCG selects EDX
without setting ECX. The 72-byte helper at 0x20cc2fa0 remains
Diagnostic. Ghidra read-only, save state unverified.
See [context accessors](simscheduler.dll/simscheduler-context-accessors-area.md).

2026-11-17 simscheduler.dll update setter 0x20cc2c40 now has a
50-byte linked Diagnostic with five differing bytes after checking
the call target. A reconstructed time value copy constructor
reproduces the outgoing stack store. Only ECX versus EDX differs.
Ghidra read-only, save state unverified.

2026-11-17 simscheduler.dll update setter at 0x20cc2c40 has a
49-byte linked diagnostic against 50 original bytes. Stack frame
and propagation target verified. The context register and outgoing
double store differ. Ghidra read-only, save state unverified.

2026-11-17 simscheduler.dll time helper 0x20cc15d0 is Exact
across all 19 linked VC8 SP1 LTCG bytes. An aligned local time
value reproduces the original stack alignment with EAX and RET 8.
Ghidra read-only, save state unverified.

2026-11-17 simscheduler.dll 0x20cc28f0 follow-up recovered ECX
for the saved buffer in a linked trial. The trial shifted the count
load and spill and did not improve the complete comparison.
The earlier 71-byte Diagnostic remains the dashboard comparison.
Ghidra read-only, save state unverified.

2026-11-17 simscheduler.dll container cleanup at 0x20cc28f0
remains Diagnostic at 71 original / 71 linked bytes. The ESI
member input and unconditional count write agree. Initial
storage-pointer register and spill scheduling differ.
Ghidra read-only, save state unverified.

2026-11-17 simscheduler.dll linked constructor cleanup actions
0x20cc7090, 0x20cc709b and 0x20cc70a6 are Normalized. Each full
body matches after its checked REL32 jump. The destination bodies
remain separate. Two more base pool wrapper variants emitted
the same code and a shorter matching suffix. The previous best
wrapper source was restored. Ghidra read-only, save state unverified.

2026-11-17 simscheduler.dll constructor EH handler 0x20cc70b4
matches all 27 linked VC8 LTCG bytes after three address mappings.
Base pool deleting wrapper 0x20cc3d60 remains Diagnostic at
96 / 96 bytes with a 76-byte matching suffix after four mappings.
Three distinct source forms retained. Ghidra read-only, save state unverified.
See [EH handlers](simscheduler.dll/simscheduler-eh-funclets-area.md)
and [pool metrics](simscheduler.dll/simscheduler-pool-metrics-area.md).

2026-11-17 simscheduler.dll time getter at 0x20cc2ff0 is Exact in
both free and virtual VC8 SP1 COFF bodies. All 20 bytes match.
The reconstructed return type is double*, supported by EAX at RET 8.
The native context profile now has 14 Exact and two Normalized
virtual bodies. Ghidra read-only, save state unverified.
See [context accessors](simscheduler.dll/simscheduler-context-accessors-area.md).

2026-11-17 simscheduler.dll 0x20cc27a0 now has an internal VC8 LTCG
comparison with a verified direct call and two stack arguments.
The 274-byte body uses RET 8 with no extra EBP+8 store. Three spans
cover 225 bytes. The other 49 bytes are in two scheduling regions.
Direct export changes the ABI and fast FP specializes the time helper.
Whole body Diagnostic. Ghidra read-only, save state unverified.
See [context lifecycle](simscheduler.dll/simscheduler-context-lifecycle-area.md).

2026-11-17 simscheduler.dll 0x20cc27a0 now maps its Ghidra label
to ConstructSchedulerContext and the inlined member initializer in the
dashboard. A direct placement pointer leaves the EBP+8 store in place.
VC8 omits it in a separate 274-byte member body reached from an
11-byte forwarding wrapper. The inlined entry remains Diagnostic.
Ghidra read-only, save state unverified.

2026-11-17 simscheduler.dll constructor scheduling follow-up at
0x20cc27a0 found byte-identical output after swapping cursor and
substep assignments. /Os uses out-of-line EH prologue and epilogue
calls and was rejected as a matching candidate. The combined container
build remains Diagnostic, with 225 bytes in three verified spans.
Ghidra read-only, current save state unverified.
See [context lifecycle](simscheduler.dll/simscheduler-context-lifecycle-area.md).

2026-11-17 simscheduler.dll constructor at 0x20cc27a0 has a stronger
linked VC8 /Oy- diagnostic. Borrowed storage reproduces three unwind
states and state 2 at name copy. An early name member and pointer-first
container initialization reproduce a 104-byte block with one verified
import address. Whole body 274 original / 277 linked, with remaining
store and address-calculation scheduling differences outside the block.
Ghidra read-only, current save state unverified.
See [context lifecycle](simscheduler.dll/simscheduler-context-lifecycle-area.md).

2026-11-17 simscheduler.dll native context class reproduces all 21
slot mappings at 0x20cc1200 in VC8 SP1 COFF: 13 Exact and two
Normalized virtual bodies, six Diagnostic. The constructor at
0x20cc27a0 remains Diagnostic (274 original / 277 COFF bytes);
the 276-byte linked LTCG result remains the best constructor comparison.
The native profile is opt-in and legacy accessors passed their
regression checks. Next useful comparison is the native deleting
destructor and its pool call, separately from the constructor.
Ghidra read-only, current save state unverified.
See [context lifecycle](simscheduler.dll/simscheduler-context-lifecycle-area.md).

2026-11-16 simscheduler.dll native VC8 class ABI probe reproduces
the mixed stack-this and ECX-this dispatch used at 0x20cc1200.
The release dispatch differs only in the deleting-entry slot
displacement in the four-slot probe. Full table declaration and
pool deallocator remain open. Ghidra read-only and saved.

2026-11-16 simscheduler.dll context class-layout review verified the
name initialization order and sampled mixed VTable ABIs at 0x20cc1200.
An initialized name member remains a source-layout hypothesis.
Ghidra read-only and saved. See [context lifecycle](simscheduler.dll/simscheduler-context-lifecycle-area.md).

2026-11-16 simscheduler.dll 0x20cc27a0 complete matching follow-up:
best linked VC8 LTCG /Oy- diagnostic 274 original versus 276
linked bytes. Seven address fields verified. Prologue and tail match
in 44- and 75-byte spans. LTCG reproduces the EAX pointer argument
and ret 8 at 0x20cc15d0. Native COFF 277 remains a separate
diagnostic. Three approaches retained. Ghidra read-only and saved.
See [context lifecycle](simscheduler.dll/simscheduler-context-lifecycle-area.md).

2026-11-16 simscheduler.dll constructor at 0x20cc27a0 now preserves
the upper mode and status bits in the VC8 build. Both mask
instructions match. Retained native COFF diagnostic is 274 original
versus 277 rebuilt bytes. Three exact mode setters are unchanged.
Full DLL linkage is deferred until the function-level pass.
Ghidra read-only and saved. See [context lifecycle](simscheduler.dll/simscheduler-context-lifecycle-area.md).

2026-11-16 simscheduler.dll registry cleanup at 0x20cc7040,
0x20cc7059 and 0x20cc70e0 verified in native COFF. Added the
missing published-reference guard to the C++ lookup. All 19
original cleanup actions now have individual dashboard comparisons.
Ghidra read-only and saved. Runtime exception execution remains open.
See [name registry](simscheduler.dll/simscheduler-name-registry-area.md).

2026-11-16 simscheduler.dll context constructor unwind actions
at 0x20cc7090, 0x20cc709b and 0x20cc70a6 agree in order and
member targets. The inlined placement factory's additional delete
is a verified no-op. Byte comparisons diagnostic. Ghidra read-only
and saved. Next are registry actions at 0x20cc7040, 0x20cc7059
and 0x20cc70e0.

2026-11-16 simscheduler.dll context destructor unwind actions
at 0x20cc7260, 0x20cc726b and 0x20cc7276 have the same static
cleanup order and activation as the retained VC8 COFF build.
Byte comparisons remain diagnostic. Ghidra read-only and saved.
Next are the context constructor actions at 0x20cc7090,
0x20cc709b and 0x20cc70a6.

2026-11-16 simscheduler.dll SimConnectFallback now adopts both
temporary references after successful return, matching the
original unwind activation points. Actions at 0x20cc7220,
0x20cc7228 and 0x20cc7230 are statically verified and byte
comparisons remain diagnostic. Ghidra read-only and saved.
Next are context destructor actions at 0x20cc7260, 0x20cc726b
and 0x20cc7276. See [fallback context](simscheduler.dll/simscheduler-fallback-context-area.md).

2026-11-16 simscheduler.dll context factory at 0x20cc1670 now
emits the three original unwind transitions in native VC8 COFF.
Actions at 0x20cc71d0, 0x20cc71e9 and 0x20cc71f1 are statically
verified and byte comparisons remain diagnostic. Ghidra read-only
and saved. Next related actions are at 0x20cc7220, 0x20cc7228
and 0x20cc7230. See [EH funclets](simscheduler.dll/simscheduler-eh-funclets-area.md).

2026-11-16 simscheduler.dll guard actions at 0x20cc7170 and
0x20cc71a0 now have a scope-guard C++ reconstruction verified in
native COFF. Both clear bit 0 during unwind via one cleanup state
across the original protected calls. Byte comparisons diagnostic.
Ghidra read-only and saved. Next is the context factory cleanup
group at 0x20cc71d0, 0x20cc71e9, and 0x20cc71f1.
See [EH funclets](simscheduler.dll/simscheduler-eh-funclets-area.md).

2026-11-16 simscheduler.dll pool destructor cleanup at 0x20cc7140
has equivalent static unwind behavior in both retained COFF owners.
Eight-byte comparison diagnostic after validating REL32. The guard
destructor performs the dereference moved out of the funclet.
Source retained. Ghidra read-only and saved. Next are 0x20cc7170
and 0x20cc71a0. See [EH funclets](simscheduler.dll/simscheduler-eh-funclets-area.md).

2026-11-16 simscheduler.dll pool constructor cleanup at 0x20cc7110
matches both native COFF actions over eight bytes with a validated
REL32 jump. The additional rebuilt state calls a verified no-op
placement delete after unlinking. Source retained. Ghidra read-only
and saved. Next related funclet is 0x20cc7140.
See [EH handlers and funclets](simscheduler.dll/simscheduler-eh-funclets-area.md).

2026-11-16 simscheduler.dll inventory follow-up reconciled all 62
external function objects with original PE IAT slots. The native
cleanup probes at 0x20cc15f0 and 0x20cc1ef0 are diagnostic due to
internal register ABIs. Current internal dashboard status counts
are 22 Exact, 45 Normalized, 81 Diagnostic, and 19 cleanup funclets
without individual comparisons. Ghidra read-only and saved.
See [function coverage](simscheduler.dll/simscheduler-function-coverage.md).

2026-11-16 simscheduler.dll live Ghidra count is 229, consisting of
167 internal functions with dashboard coverage and 62 external
function objects. The five CRT
thunks at 0x20cc6e30 through 0x20cc6e60 have normalized linked
matches. Cleanup helpers at 0x20cc15f0 and 0x20cc1ef0 have verified
original listings and open standalone matching. Ghidra read-only
and saved. See [function coverage](simscheduler.dll/simscheduler-function-coverage.md).

2026-11-16 simscheduler.dll C++ EH block through the CRT initializer
boundary at 0x20cc72b0 verified against Ghidra and the original PE.
Ten handlers, 19 original cleanup funclets, and 12 parent references
recorded. Three 27-byte handler bodies match modulo relocations in
retained native COFF builds. Seven remain diagnostic. Full unwind
matching and runtime verification remain open. Ghidra read-only and
saved. The next block overlaps the already analyzed startup entries.
See [EH handlers and funclets](simscheduler.dll/simscheduler-eh-funclets-area.md).

2026-11-16 simscheduler.dll CRT conversion at 0x20cc6ea0 matches the
full 171-byte region, including the secondary entry at 0x20cc6ebc.
Thunks at 0x20cc6e86, 0x20cc6f50 and 0x20cc6f5c match all six bytes
after validated IAT normalization. Ghidra read-only and saved.
Runtime execution unverified. The next address block follows the
already matched SSE2 initializer at 0x20cc7021.
See [CRT conversion](simscheduler.dll/simscheduler-crt-conversion-area.md).

2026-11-16 simscheduler.dll cookie check at 0x20cc69ae, GS failure
reporter at 0x20cc6d27, and debugger thunk at 0x20cc6e6c match all
15, 260, and 6 linked bytes after validated address normalization
in three retained builds. Ghidra read-only and saved. Runtime
execution remains unverified.
See [CRT PE sections](simscheduler.dll/simscheduler-crt-pe-sections-area.md).

2026-11-16 simscheduler.dll EH4 handler at 0x20cc6c66 matches all
35 linked bytes after three checked address fields. Ghidra reports
no unsaved changes. Next cookie check at 0x20cc69ae.
See [CRT PE sections](simscheduler.dll/simscheduler-crt-pe-sections-area.md).

2026-11-16 simscheduler.dll four CRT import thunks at 0x20cc6bd8,
0x20cc6be4, 0x20cc6bf0 and 0x20cc6bfc match all six instruction bytes
after IAT normalization. Ghidra read-only and saved. Next EH4 handler
at 0x20cc6c66. See [CRT PE sections](simscheduler.dll/simscheduler-crt-pe-sections-area.md).

2026-11-16 simscheduler.dll CRT PE helpers at 0x20cc6af0 and
0x20cc6b20 match exactly. The section check at 0x20cc6b67 matches in
the RTM link probe with its EH4 scope checked separately. Ghidra read-only.
Next CRT initializer import thunks at 0x20cc6bd8 and 0x20cc6be4.
See [CRT PE sections](simscheduler.dll/simscheduler-crt-pe-sections-area.md).

2026-11-16 simscheduler.dll RTC callbacks at 0x20cc6a9a and 0x20cc6ac3
both match in the existing linked probes with two checked fields each.
Both callback ranges are empty. Ghidra read-only and saved.
Next CRT PE image and section lookup at 0x20cc6af0.
See [CRT exit registration](simscheduler.dll/simscheduler-crt-onexit-area.md).

2026-11-16 simscheduler.dll TypeInfo cleanup at FUN_20cc6a89 matches
12 linked RTM bytes with two checked fields. Ghidra inspection was
read-only. Next CRT callbacks at 0x20cc6a9a and 0x20cc6ac3.
See [CRT exit registration](simscheduler.dll/simscheduler-crt-onexit-area.md).

2026-11-16 simscheduler.dll CRT exit registration, unlock helper and
atexit wrapper match in the retained RTM-linked probe. The on-exit
EH4 table was checked separately. Ghidra read-only and saved. Next
CRT lifecycle callbacks at 0x20cc6a89, 0x20cc6a9a and 0x20cc6ac3.
See [CRT exit registration](simscheduler.dll/simscheduler-crt-onexit-area.md).

2026-11-16 simscheduler.dll CRT SSE2 detection and OS support probe
match in the retained RTM-linked image at 0x20cc6fbc and 0x20cc6f67.
Both fields in the SSE2 EH4 table were verified separately.
Ghidra read-only and saved. Next CRT exit registration at 0x20cc69c2
and 0x20cc6a66. See [CRT SSE2](simscheduler.dll/simscheduler-crt-sse2-area.md).

2026-11-16 simscheduler.dll DLL startup compared in a VC8 SP1 linked
probe with pinned RTM CRT. Nine normalized bodies, one exact body
and one DllMain diagnostic. EH4 table checked separately. Existing
gates pass. Ghidra reports no unsaved changes. Next CRT SSE2 support
at 0x20cc6fbc and FUN_20cc6f67.
See [DLL startup](simscheduler.dll/simscheduler-dll-startup-area.md).

2026-11-16 simscheduler.dll shared pool lock reconstructed. Five
native VC8 diagnostic bodies cover construction, acquisition, release,
destruction and backoff. Concurrent runtime behavior remains unverified.
Previous gates pass. Ghidra read-only and saved. Next PE startup at
0x20cc6988. See [pool lock](simscheduler.dll/simscheduler-pool-lock-area.md).

2026-11-16 simscheduler.dll shared pool free list reconstructed across
eleven native diagnostic bodies. VC8 emits tagged locked cmpxchg8b
for the pop path. Previous 49 native gates and retained regressions
pass. Ghidra read-only and saved. Next shared pool lock at FUN_20cc54b0.
See [free list](simscheduler.dll/simscheduler-free-list-area.md).

2026-11-16 simscheduler.dll pool metrics and remaining VTable operations
reconstructed. Nine native exact bodies, two normalized deleting
wrappers and five diagnostics. Existing gates pass. Ghidra reports
no unsaved changes. Next shared free list at FUN_20cc4d90 and FUN_20cc4eb0.
See [pool metrics](simscheduler.dll/simscheduler-pool-metrics-area.md).

2026-11-16 simscheduler.dll context and timed-entry pool lifecycle
reconstructed across fourteen bodies. Four CRT wrappers are normalized,
the shared used-count helper is exact, and nine are diagnostic.
Existing gates pass. Ghidra reports no unsaved changes. Next pool
metrics and remaining VTable slots at 0x20cc3a60 and 0x20cc45a0.
See [pool lifecycle](simscheduler.dll/simscheduler-pool-lifecycle-area.md).

2026-11-16 simscheduler.dll scheduler object lifecycle reconstructed.
0x20cc1600 and 0x20cc72b0 match in native COFF with eighteen checked
address fields. 0x20cc7300 and FUN_20cc21b0 remain diagnostic.
The previous gates pass. Ghidra reports no unsaved changes.
Next pool lifecycle at 0x20cc72d0 and 0x20cc72e0.
See [scheduler object lifecycle](simscheduler.dll/simscheduler-object-lifecycle-area.md).

2026-11-16 simscheduler.dll context reset at FUN_20cc2e20 is reconstructed.
The retained 88-byte VC8 SP1 COFF body is diagnostic against the full
86-byte original range with its checked pool call. Existing gates pass.
Ghidra read-only and saved. Next scheduler object initialization and
teardown around the VTable store at 0x20cc1630.
See [context reset](simscheduler.dll/simscheduler-context-reset-area.md).

2026-11-16 simscheduler.dll eight scheduler object accessors compared.
Six native VC8 SP1 COFF bodies are exact. FUN_20cc19a0 and FUN_20cc19e0
differ in register assignment and load order. Existing gates pass.
Ghidra read-only and saved. Next context reset FUN_20cc2e20.
See [scheduler object accessors](simscheduler.dll/simscheduler-object-accessors-area.md).

2026-11-16 simscheduler.dll context accessors and mode setters have
thirteen exact VC8 SP1 COFF bodies and three diagnostics at FUN_20cc2c40,
FUN_20cc2fa0 and FUN_20cc2ff0. Existing regressions pass. Ghidra read-only
and saved. Next scheduler object accessors at FUN_20cc19a0 and FUN_20cc19c0.
See [context accessors](simscheduler.dll/simscheduler-context-accessors-area.md).

2026-11-16 simscheduler.dll SimConnectFallback transition reconstructed
at FUN_20cc1cc0. The aligned VC8 SP1 COFF run emits 526 of 542 bytes
with twelve checked references and remains diagnostic after three
approaches. Existing regression comparisons pass. Ghidra read-only
and saved. Next FUN_20cc2a80 and FUN_20cc2aa0.
See [fallback context](simscheduler.dll/simscheduler-fallback-context-area.md).

2026-11-16 simscheduler.dll named context registry reconstructed and
compared across nine native COFF bodies. All nine remain diagnostic
after the bounded search. Existing twelve native gates, retained Flight
save and GPS comparisons pass. Runtime and exceptional paths unverified.
Ghidra read-only and saved. Next FUN_20cc1cc0.
See [named context registry](simscheduler.dll/simscheduler-name-registry-area.md).

2026-11-16 simscheduler.dll context factory and storage have four
C++ diagnostics. FUN_20cc2720 and FUN_20cc2740 match 22 and 49 COFF
bytes with one checked Interlocked operand each. Ghidra read-only,
save state unverified. Next named registry FUN_20cc17d0 and FUN_20cc22e0.
See [context factory and storage](simscheduler.dll/simscheduler-context-factory-storage-area.md).

2026-11-16 simscheduler.dll context lifecycle has five native COFF
diagnostics. The C++ placement factory restores the constructor's
outer stack ABI. Original table slot and EH references checked.
Existing gates pass. Ghidra read-only, save state unverified.
Next context storage and factory FUN_20cc1670, FUN_20cc1f10,
FUN_20cc20b0. See [context lifecycle](simscheduler.dll/simscheduler-context-lifecycle-area.md).

2026-11-16 simscheduler.dll container insertion has seven native and
linked C++ diagnostics. Original API imports, memcpy and direct calls
checked. Existing gates pass. Ghidra read-only, save state unverified.
Next context lifecycle FUN_20cc27a0 and FUN_20cc2940.
See [entry insertion](simscheduler.dll/simscheduler-entry-insertion-area.md).

2026-11-16 simscheduler.dll entry pool has C++ diagnostics for
FUN_20cc35d0, FUN_20cc3770 and FUN_20cc3e00. Verified block layout
and 22 reference operands. Existing gates pass. Ghidra read-only,
save state unverified. Next entry-container insertion.
See [entry pool](simscheduler.dll/simscheduler-entry-pool-area.md).

2026-11-16 simscheduler.dll immediate registration has native C++
diagnostics for FUN_20cc2ce0 and FUN_20cc2e80 and linked tests for
three helpers. Verified context slots +0x38/+0x3c. Existing gates pass.
Ghidra read-only, save state unverified. Next entry pool FUN_20cc35d0
and FUN_20cc3770. See [immediate registration](simscheduler.dll/simscheduler-immediate-registration-area.md).

2026-11-16 simscheduler.dll deferred registration boundary reached.
FUN_20cc3490 and FUN_20cc41d0 have three C++ comparisons. Best paired
LTCG diagnostic 174/221 and 225/235 bytes. Original imports verified.
Existing gates pass. Ghidra read-only, save states unverified.
Next area immediate registration, FUN_20cc2ce0 and FUN_20cc2e80.
See [deferred registration](simscheduler.dll/simscheduler-deferred-registration-area.md).

2026-11-16 simscheduler.dll internal time-update area reached the
registration boundary. FUN_20cc1b90, FUN_20cc30d0 and FUN_20cc3050
have linked C++ diagnostics. LTCG restores the original context
input and stack cleanup in the EBP profile. Existing gates pass.
Ghidra read-only, save state unverified. Next area FUN_20cc3490.
See [context time-update area](simscheduler.dll/simscheduler-context-time-update-area.md).

2026-11-16 simscheduler.dll internal time-update area started.
FUN_20cc1b90 C++ diagnostic 241 COFF bytes against 284 original.
Seven reference mappings checked. DAT_20cc8364 refined to an
observer pointer. Export, callback, linked save and GPS gates pass.
Ghidra read-only, save state unverified. Next FUN_20cc30d0.
See [internal time update](simscheduler.dll/simscheduler-internal-time-update.md).

2026-11-16 simscheduler.dll 0x20cc13c0 matched all 47 COFF bytes
with four validated DIR32 operands. The export/callback/Flight
persistence pass ends here. Nine exports, the group 0x20 callback,
and linked Flight save match. Four C++ diagnostics remain.
Ghidra read-only, save states unverified. Next area FUN_20cc1b90.
See [group 0x20 callback](simscheduler.dll/simscheduler-group20-callback-match.md).

2026-11-16 simscheduler.dll FUN_20cc4790 three C++ LTCG approaches
retained. Best diagnostic 267 linked bytes against 308 original.
Verified Flight import and propagation call. Save, nine COFF exports
and GPS checks pass. Ghidra read-only, save state unverified.
Next 0x20cc13c0. See [Flight load handler](simscheduler.dll/simscheduler-flight-load-match.md).

2026-11-16 simscheduler.dll FUN_20cc48d0 matched all 164 linked bytes
with eight validated HIGHLOW operands. Original Flight IAT identity
preserved. Nine COFF exports and GPS regression pass. Ghidra read-only,
save state unverified. Next FUN_20cc4790.
See [Flight save handler](simscheduler.dll/simscheduler-flight-save-match.md).

2026-11-16 simscheduler.dll FUN_20cc1a40 three C++ LTCG approaches
retained. Best diagnostic 116 linked bytes against 138 original.
EBX argument, ret 8 and paired stores recovered. Both listings verified.
Ghidra read-only, save state unverified. Next FUN_20cc48d0.
See [time propagation](simscheduler.dll/simscheduler-time-propagation-match.md).

2026-11-16 simscheduler.dll FUN_20cc1b30 three C++ LTCG approaches
retained. Best diagnostic 87 linked bytes against 81 original. EAX
object argument and word-copy shape recovered. Register selection,
stack frame, and cleanup differ. Ghidra read-only, save state unverified.
Next FUN_20cc1a40. See [time-state handler](simscheduler.dll/simscheduler-time-state-match.md).

2026-11-16 simscheduler.dll FUN_20cc1400 linked C++ LTCG probe emits 81
of 88 original bytes. Three direct calls resolve. Stack alignment returns,
but EDI replaces ESI and the object input is specialized. Three C++
approaches retained. Dashboard Diagnostic. All nine COFF export and GPS
regressions pass. Ghidra read-only, save state unverified.
See [event dispatcher](simscheduler.dll/simscheduler-event-dispatch-match.md).

2026-11-16 simscheduler.dll FUN_20cc1400 maintained as C++. Two VC8
COFF runs emitted 72 and 78 bytes against 88 original. The dashboard
records the 78-byte diagnostic. Two handler register ABIs remain open.
The assembly match is historical. All nine export regressions pass.
Ghidra read-only, save state unverified.
See [event dispatcher](simscheduler.dll/simscheduler-event-dispatch-match.md).

2026-11-16 superseded implementation. simscheduler.dll FUN_20cc1400 matched all 88 bytes in native
VC8 x86 COFF using inline assembly with four verified relocation targets.
All nine earlier comparisons pass. Ghidra read-only, save state unverified.
Next FUN_20cc1b30. See [event dispatcher](simscheduler.dll/simscheduler-event-dispatch-match.md).

2026-11-15 simscheduler.dll FUN_20cc1490 matched all 55 bytes in native
VC8 x86 COFF with four validated DIR32 operands. All nine exported
bodies pass. Ghidra read-only, save states unverified. Next FUN_20cc1400.
See [callback unregistration](simscheduler.dll/simscheduler-callback-unregistration-match.md).

2026-11-15 simscheduler.dll FUN_20cc1460 matched all 29 bytes in native
VC8 x86 COFF with three validated DIR32 references to the IAT and
two callbacks. All seven prior comparisons pass. Ghidra read-only,
save states unverified. Next 0x20cc1490.
See [callback registration match](simscheduler.dll/simscheduler-callback-registration-match.md).

2026-11-15 simscheduler.dll FUN_20cc1500 matched all 32 bytes in native
VC8 x86 COFF with two DIR32 operands and the REL32 call to FUN_20cc1cc0
validated. All six prior comparisons pass. Ghidra read-only, save state
unverified. Next 0x20cc1460.
See [SimConnectFallback wrapper](simscheduler.dll/simscheduler-simconnect-fallback-match.md).

2026-11-15 simscheduler.dll FUN_20cc1530 matched all 34 bytes in native
VC8 x86 COFF with two validated DIR32 operands. Verified slot +0x20,
FUN_20cc1ad0 machine ABI and x87 return in the wrapper. Five earlier
comparisons pass. Ghidra read-only, save state unverified.
Next 0x20cc1500. See [SimTime wrapper](simscheduler.dll/simscheduler-sim-time-match.md).

2026-11-15 simscheduler.dll FUN_20cc1580 matched all 29 bytes in native
VC8 x86 COFF with two validated DIR32 operands. Verified slot +0x14 and
FUN_20cc19e0. The four existing SimScheduler comparisons pass.
Ghidra read-only, save state unverified. Next target 0x20cc1530.
See [SimSpeed setter wrapper](simscheduler.dll/simscheduler-set-sim-speed-match.md).

2026-11-15 simscheduler.dll FUN_20cc15b0 matched all 16 bytes in native
VC8 x86 COFF with two validated DIR32 operands. Verified VTable slot
+0x18, target FUN_20cc1a00, and its use in Flight SimSpeed persistence.
All three prior matches pass. Ghidra read-only, save state unverified.
Next 0x20cc1580. See [SimSpeed wrapper](simscheduler.dll/simscheduler-sim-speed-match.md).

2026-11-15 simscheduler.dll FUN_20cc1560 matched all 16 bytes in native
VC8 x86 COFF with two validated DIR32 operands. Verified the original
VTable slot +0x1c and FUN_20cc1a20 ABI. Both existing matches pass.
Ghidra read-only, save state unverified. Next target 0x20cc15b0.
See [wrapper match](simscheduler.dll/simscheduler-field-min-match.md).

2026-11-15 simscheduler.dll FUN_20cc14e0 matched all 17 bytes in native
VC8 x86 COFF with one validated DIR32 operand. The 0x20cc14d0 regression
passes. The role of 0x20cc8368 remains provisional. Both listings are in
the dashboard. Ghidra read-only, save state unverified. Next 0x20cc1560.
See [setter match](simscheduler.dll/simscheduler-dispatch-key-match.md).

2026-11-15 simscheduler.dll FUN_20cc14d0 matched all six bytes in native
VC8 x86 COFF after validating its DIR32 operand. Both listings are in the
dashboard. Ghidra read-only, save state unverified. Next target 0x20cc14e0.
See [object getter match](simscheduler.dll/simscheduler-object-getter-match.md).

2026-11-15 simscheduler.dll first pass verified nine ordinal-only exports,
Flight time and rate persistence, and a six-byte matching target at
0x20cc14d0. Build profile, object layout and comparison are pending.
Ghidra read-only with save state unverified.
See [first pass](simscheduler.dll/simscheduler-first-pass.md).

2026-11-15 G2D DX10 depth pool allocation wrapper connected.
VC8 x86 run g2d-depth-allocate-20261115-001 passed without warnings.
Three call targets checked. Dashboard records Diagnostic with both listings.
Width getter Exact regression passed. Ghidra read-only.
Next is depth texture construction and initialization at 206d87a0,
206d8830 and 206d89f0. See [depth allocation](g2d.dll/g2d-dxgi-depth-allocate.md).

2026-11-15 G2D DX10 depth pool return and target finish reconstructed.
VC8 x86 run g2d-depth-return-20261115-001 passed without warnings.
Four call mappings checked. Dashboard records two Diagnostic entries.
Width getter Exact regression passed. Ghidra read-only.
Next is depth pool allocation at 206d9270.
See [depth resource return](g2d.dll/g2d-dxgi-depth-return.md).

2026-11-15 G2D DX10 quad constant width getter 2067e570 matches the
complete 16-byte original body in VC8 x86 COFF with inline assembly.
The pure C++ trial emits 20 bytes. Dashboard records Exact with both
listings. Ghidra read-only. Depth pool allocation and return remain next.
See [quad size match](g2d.dll/g2d-dxgi-quad-size-match.md).

2026-11-15 G2D DX10 back-buffer description, depth pool lookup and view setup connected.
VC8 x86 run g2d-clear-target-depth-20261115-001 passed without warnings.
Dashboard records three Compiled entries with both listings. Ghidra read-only.
Next is depth resource allocation and return at 206d9270 and 206d9190.
See [clear-target depth view](g2d.dll/g2d-dxgi-clear-target-depth.md).

2026-11-15 G2D DX10 target binding and presentation-window membership connected.
VC8 x86 run g2d-clear-target-bind-20261115-001 passed without warnings.
Dashboard records two Compiled entries with both listings. Ghidra read-only.
Next is back-buffer description and depth-view setup at 206db540,
206d9060 and 206d8c40. See [target binding](g2d.dll/g2d-dxgi-clear-target-bind.md).

Technical notes are organized by target in [fsx.exe](fsx.exe/),
[api.dll](api.dll/), [g2d.dll](g2d.dll/) and [gps.dll](gps.dll/). [Shared notes](shared/)
cover the EXE/DLL application boundary and matching workflows.

2026-11-15 API registry 200f6451 snapshot trial has four differing
bytes across the 318-byte linked body after fourteen address mappings.
Remaining differences are growth-size register selection. Existing
gates pass. Ghidra saved, mod 3.

2026-11-15 API registry 200f6451 internal SP1 probe emits ESI and
RET 0x10. Public probe callers match 200f63df and 200f6403 completely.
Registry body remains diagnostic at 319/318 bytes. Ghidra saved, mod 3.

2026-11-15 API registry 200f6451 linked diagnostic. The allocation
failure path matches 67 bytes and the reallocator call resolves.
The owner register ABI remains open. Prior linked and COFF checks
pass. Ghidra saved, mod 3. See api.dll/api-callback-registry.md.

2026-11-15 API reallocator 20135e98 matches all 185 linked bytes
after fourteen verified mappings. Prior linked bodies retained.
Ghidra saved, mod 3. See api.dll/api-callback-registry.md.

2026-11-15 API heap initializer 2013623e matches all 144 linked bytes
after nine verified address mappings. Adopted the flag snapshot and
tested-zero reuse in regular source. Prior linked and twenty fresh
COFF gates pass. Ghidra saved, modification 3. Runtime remains open.

2026-11-15 API standalone allocator 20135dd9 matches in the retained
SP1 linked build. All 174 bytes checked after 13 address mappings.
Both inlined callers retain their matches. Next verify initializer
2013623e. Ghidra saved, modification 3.

2026-11-15 API 20137efe and 201322e2 complete linked matches.
The shared allocator flag read reproduces both original inlined
callers. 491 linked bytes added with 34 verified address mappings.
Three prior linked and twenty fresh COFF checks pass. Runtime and
standalone allocator body remain open. Ghidra saved, modification 3.
See [heap string duplicate](api.dll/api-heap-string-duplicate.md).

2026-11-15 API 20137efe uses the shared allocator in regular source.
The SP1 linked entry matches 29 bytes and the 67-byte failure path
matches after verified address mapping. That path is shared by the
201322e2 original. Full 20137efe body remains open. Three linked and
twenty fresh COFF checks pass. Ghidra saved, mod 3.
See [heap string duplicate](api.dll/api-heap-string-duplicate.md).

2026-11-15 API copy helper 20137e17 capacity parameter trial restores
the original SUB EBX,1 encoding with only two byte changes to the
175-byte probe. Asymmetric pointer variants still move increments
before the lead test. Append and prior gates pass. Ghidra saved, mod 3.

2026-11-15 API copy helper 20137e17 first 42 bytes match with two IAT
mappings. The 175-byte SP1 probe shares both exits and preserves the
lead branch before pointer updates. Remaining capacity and trail-load
differences block full body acceptance. Append and prior gates pass.
Ghidra saved, modification 3.

2026-11-15 API copy helper 20137e17 shares the zero-capacity return
in the 188-byte SP1 probe. It preserves the prior 42-byte entry and
27-byte lead path against the 191-byte control. The full 169-byte
body remains open. Append and prior gates pass. Ghidra saved, mod 3.

2026-11-15 API 20137e17 and simpropace 00440740 bounded copy compared.
The original pair has the same trail-copy instructions outside two
branch displacements. SP1 now reproduces the 42-byte API entry path
with the destination register exchanged and both IAT mappings.
The full body remains diagnostic. Ghidra reported both saved.
See [copy comparison](shared/api-simpropace-string-copy.md).

2026-11-15 API copy helper 20137e17 NUL-exit trials. The 171-byte SP1
variant shares the final store, but moves the destination increment
before the lead-byte test. Both trials retain the linked 20137ec0
match and prior gates. Ghidra was read-only, save state unverified.

2026-11-15 API ordinal 157 combined shape trial. SP1 emits the
original lead-byte instruction order and a normal lstrcpynA CALL.
The 27-byte lead branch agrees after EDI/ESI exchange only; the full
188-byte probe remains diagnostic. Ordinal 218 and prior gates pass.
Ghidra was read-only, with save state unverified.

2026-11-15 API ordinal 157 trail-advance trial. Direct trail-byte
access matches the original strategy, but SP1 hoists the first
increments before the branch again. Retain the early-exit probe as
branch-order reference. Ordinal 218 and prior gates pass. Ghidra was
read-only, with save state unverified.

2026-11-15 API ordinal 157 SP1 copy sweep. The early-exit source
reproduces the local count and branch ordering. /Og- restores a normal
lstrcpynA CALL but diverges elsewhere. No complete 157 match. All three
variants retain linked 218. Three prior linked and twenty COFF gates
pass on replay. Ghidra was read-only, with save state unverified.

2026-11-15 API public string exports compared under SP1 with and
without LTCG. The 157 and 218 bodies are raw equal across profiles.
The 62-byte linked 218 matches the original with a validated call to
157, whose body remains diagnostic. Three prior linked and twenty
COFF gates pass. Ghidra was read-only, with save state unverified.

2026-11-15 API copy helper verified as unnamed ordinal 157. Adjacent
218, 158 and 159 expose append, private-heap duplication and length.
Next controlled build is the public 157 export under SP1 with and
without LTCG. No new build or Ghidra edit. Save state unverified.

2026-11-15 API copy helper 20137e17 split advance trial. SP1 still
hoists both increments before the branch. Only two bytes differ from
the prior 152-byte integer probe. Three linked and twenty COFF gates
pass. Ghidra was read-only, with save state unverified.

2026-11-15 API copy helper shared return experiment on SP1.
The source rewrite emits the same 152 bytes as the integer-test
variant and retains the lstrcpynA tail jump. Three linked and twenty
COFF gates pass. Ghidra save state unverified.

2026-11-15 API copy helper 20137e17 lead-byte type trial. SP1 direct
integer test removes SETNE but moves the test past the copy and pointer
updates. Regular source stays at the 157-byte baseline. The requested
RTM control was rejected because the guest reported SP1. RTM remains
open. Ghidra was read-only, with save state unverified.

2026-11-15 API copy helper 20137e17 compared with VC8 RTM static CRT.
Both candidate entries equal SP1. The workers differ only in call
displacements to the same named targets. Neither matches the original.
The three linked and twenty COFF checks pass on replay. Ghidra was
untouched, with save state unverified.

2026-11-15 API copy helper 20137e17 compared with static VC8 CRT.
The _mbsnbcpy and _mbsncpy bodies differ. Corrected the original extent
to 169 bytes including the final RET operand. Three linked and twenty
COFF checks pass. The helper's source provenance remains open. Ghidra
was read-only, with save state unverified.

2026-11-15 API ordinal 206 call provenance reviewed. StrSafe and the
VC8 /GS helper are identified, PE imports verified, and the bounded copy
helper's library origin remains open. No new build or Ghidra edits.
Ghidra save state unverified. See api.dll/api-numbered-names.md.

2026-11-15 API ordinal 206 and its copy helper integrated in regular source.
Linked VC8 verification retains the 60-byte match for 206. The wrapper
and copy remain diagnostic; heap and copy call targets are verified.
Three existing linked and twenty COFF gates pass. Runtime is open.
Ghidra was read-only, save state unverified.

2026-11-15 API ordinal 206 automatic inlining verified in a linked probe.
The wrapper emits 314 bytes versus 313 and matches the 60-byte formatting
and length region after two address mappings. The 207, 208 and StrSafe
linked gates still pass. Next inspect heap check and copy scheduling.
The dashboard includes both 206 listings. Ghidra save state unverified.

2026-11-15 API numbered name SDK source adopted for ordinals 207 and 208.
The linked VC8 /GL and /LTCG DLL fragment matches all 172 bytes across
both wrappers and the StrSafe formatter after five address mappings.
The /GL- control retains stack passing. All twenty regular COFF gates
pass. Dashboard listings are present. Whole DLL and runtime remain open.
Ghidra was read-only, save state unverified.

2026-11-15 API ordinal 207 register ABI tested in a VC8 COFF probe.
The full 38-byte wrapper agrees after two relocation mappings. Twenty
existing gates pass. The formatter remains unresolved in the probe and
the regular build uses the callable C++ implementation. The dashboard
retains both listings. Ghidra was read-only, save state unverified.
See [numbered names](api.dll/api-numbered-names.md).

API formatter provenance follow-up: `200fb1ab` agrees with the archived
VC8 SDK StrSafe formatter through the verified EXE body comparison.
Next candidate is a linked C++ SDK build of ordinals 207 and 208 with
/GL and /LTCG. No new API build. Ghidra save state unverified.

2026-11-15 G2D DX10 clear-target registry selection and reset reconstructed.
VC8 x86 run g2d-clear-target-registry-20261115-001 passed without warnings.
Dashboard records three Compiled entries with both listings. Ghidra read-only.
Next is target binding and update at 206db1e0 and 206db8f0.
See [clear-target registry](g2d.dll/g2d-dxgi-clear-target-registry.md).

2026-11-15 G2D DX10 upper clear entry points connected to the pilot selector.
VC8 x86 run g2d-clear-entrypoints-20261115-001 passed without warnings.
Dashboard records three Compiled entries with both listings. Ghidra read-only.
Next is the clear target lifecycle behind window +0x15c. Final table
binding and linked/runtime verification remain open.
See [clear entry points](g2d.dll/g2d-dxgi-clear-entrypoints.md).

2026-11-15 G2D DX10 compositor clear connected to the target selector.
VC8 x86 run g2d-compositor-clear-20261115-001 passed without warnings.
Dashboard records Compiled with both listings. Ghidra read-only.
Next are the upper clear callers, starting at 20690570.
See [compositor clear](g2d.dll/g2d-dxgi-compositor-clear.md).

2026-11-15 G2D DX10 target clear dispatch connected to local quad drawing.
VC8 x86 run g2d-quad-clear-20261115-001 passed without warnings.
Dashboard records Compiled with both listings. Ghidra read-only.
Next is alternate-target clear at 2067a270. Upper caller integration,
DLL linking and runtime verification remain open.
See [target clear](g2d.dll/g2d-dxgi-quad-clear.md).

2026-11-15 G2D DX10 quad render-state cache connected.
VC8 x86 run g2d-quad-draw-state-20261115-001 passed without warnings.
Dashboard records six Compiled entries with both listings. Ghidra
read-only. Next is the higher-level quad caller at 206db390.
See [quad render state](g2d.dll/g2d-dxgi-quad-draw-state.md).

2026-11-15 G2D DX10 quad drawing and constant upload reconstructed.
VC8 x86 run g2d-quad-draw-20261115-001 passed without warnings.
Dashboard records three Compiled entries with both listings. Ghidra
read-only. Next is state entry preparation at 2067eb50. The caller
at 206db390 still needs source integration.
See [quad drawing](g2d.dll/g2d-dxgi-quad-draw.md).

The local dashboard reads retained module inventories and comparison evidence.
See [dashboard usage](../../tools/re-dashboard/README.md) for generation and
adding new results. The dashboard also imports API global observations and
the saved ApplicationVTable layout, plus EXE data comparisons and VTable
slots. G2D currently has a partial inventory.

API build job evidence is now retained under `vm-share/modules/api/runs/`.
This supersedes the earlier API job snapshot layout. Existing run artifacts
and shared queue jobs remain available.

EXE builds and baselines are under `vm-share/modules/fsx/`. The shared
worker queue and controller lock remain at VM-share level. The EXE queue
audit links removed transport duplicates to their retained run copies.

Analysis artifacts and packed Ghidra backups are under
`artifacts/<binary-name>/`. Shared toolchain and multi-module inventories
are under `artifacts/shared/`. The dashboard reads the module directories.

2026-11-15 GPS legacy lifecycle and record initialization connected.
VC8 x86 run gps-legacy-lifecycle-20261115-003 checked 35 callback
relocations and matched the 147-byte reset and publish block. The
initializer body and EAX adapter have a separate diagnostic.
Prior GPS gates passed. Next is the refresh callback's count and
allocation path at 2086aac0 and 2086ab20. Ghidra was read-only and the
prior save state is unverified.
See [GPS legacy lifecycle](gps.dll/gps-legacy-lifecycle.md).

2026-11-15 GPS provider and legacy storage cleanup reconstructed.
VC8 x86 run gps-provider-cleanup-20261115-004 matches the 78-byte
entry-release method after one validated relocation. The destructor
unwind and legacy ESI boundary have separate COFF diagnostics.
Prior GPS gates passed. Next is the 2086a380 lifecycle callback and
its 2086aa10 initializer. Ghidra was read-only and the prior save
state is unverified.
See [GPS provider cleanup](gps.dll/gps-provider-cleanup.md).

2026-11-15 GPS ordinal 1 and provider Release reconstructed.
VC8 x86 run gps-provider-init-20261115-003 checked 13 relocations across
the new bodies and recorded code-generation differences. Ordinal 2
retained its normalized match. The dashboard also inventories the
legacy callbacks and their cleanup helper. Next are destructor 208560e0
and the ESI cleanup boundary at 2086a4c0. Ghidra was read-only and the
prior save state is unverified.
See [GPS provider lifetime](gps.dll/gps-provider-lifetime.md).

2026-11-15 GPS provider registration entered the dashboard with both
ordinal export listings. Ordinal 2 at 20855fb0 matches all 42 bytes
after five target-validated COFF relocations in
gps-registration-20261115-002. Next is ordinal 1 at 20855f20,
including provider layout and virtual dispatch. DLL linking and
runtime verification remain open. Ghidra was read-only with the
prior save state unverified.
See [GPS provider registration](gps.dll/gps-provider-registration.md).

2026-11-15 G2D window shared-state reset and resource entry cleanup connected.
VC8 x86 run g2d-window-state-reset-20261115-001 passed without warnings.
Dashboard records three Compiled entries with both listings. Ghidra
read-only. Next is quad state application at 2067ea50 and its resource
consumer 2067f500. See [window state reset](g2d.dll/g2d-dxgi-window-state-reset.md).

2026-11-15 G2D window shared-state and quad constant teardown connected.
VC8 x86 run g2d-quad-teardown-20261115-001 passed without warnings.
Dashboard records four Compiled entries with original and rebuilt listings.
Ghidra read-only. Next is holder reset at 2067e9c0 and its entry cleanup.
See [quad teardown](g2d.dll/g2d-dxgi-quad-teardown.md).

2026-11-15 G2D quad constant construction and GPU buffer creation connected.
VC8 x86 run g2d-quad-constant-20261115-001 passed without warnings.
Dashboard records both functions as Compiled. Ghidra read-only.
Next is shared-state and constant-buffer teardown at 2067e7b0,
2067e860, 2067ec80 and 2067e7e0.
See [quad constant](g2d.dll/g2d-dxgi-quad-constant.md).

2026-11-14 G2D shader binding lookup and size check connected for GsQuadCb.
VC8 x86 run g2d-shader-lookup-20261114-001 passed without warnings.
Ghidra read-only, with no new unsaved edits. Next is quad constant
construction and GPU buffer creation at 2068d3f0 and 2068d460.
See [shader lookup](g2d.dll/g2d-dxgi-shader-lookup.md).

2026-11-14 G2D reflected shader binding collection connected in source.
VC8 x86 run g2d-shader-bindings-20261114-001 passed without warnings.
Ghidra read-only, with no new unsaved edits. Next is the shader binding
lookup consumer at 2067fad0 and 20680590. DLL linking and runtime
verification remain open. See [shader bindings](g2d.dll/g2d-dxgi-shader-bindings.md).

2026-11-14 G2D DX10 shader lifetime and macro copying connected in source.
VC8 x86 run g2d-shader-lifetime-20261114-001 passed without warnings.
Ghidra read-only, with no new unsaved edits. Next is reflected-binding
storage at 20680710 and 20680880. DLL linking and runtime verification
remain open. See [shader lifetime](g2d.dll/g2d-dxgi-shader-lifetime.md).

2026-11-14 G2D DX10 shader device creation connected in source.
VC8 x86 run g2d-shader-creation-20261114-001 passed without warnings.
Ghidra read-only, with no new unsaved edits. Next are shader object
lifetime and define storage at 2067fa30, 2067f710 and 20680210.
DLL linking and runtime verification remain open. See g2d-dxgi-shader-creation.md.

2026-11-14 G2D shader compilation and resource reflection connected.
VC8 x86 run g2d-shader-loading-20261114-002 passed without warnings.
Ghidra read-only, with no new unsaved edits. Next are the per-stage
device creation targets 206800b0, 20680100 and 20680150. Resource
insertion, final linking and runtime verification remain open.
See g2d-dxgi-shader-loading.md.

2026-11-14 G2D PostProcess shader catalogue connected in source.
VC8 x86 run g2d-shader-catalogue-20261114-002 passed without warnings.
Ghidra read-only. Next source boundary is 2067fe30 and 2067fbb0.
See g2d-dxgi-shader-catalogue.md.

2026-11-12 G2D DXGI resource-holder source build. Isolated VC8 x86
compiled without warnings. Ghidra read-only. Next is shader catalogue
construction and initialization at 206d68a0 and 206d7940.
See g2d-dxgi-window-resources.md.

2026-11-12 G2D DXGI window device and output setup source build.
VC8 x86 compiled without warnings. Ghidra read-only, changed=false.
Next are the device-resource constructors at 206d8d70, 206d74c0
and 2067e620. See g2d-dxgi-window-device.md.

2026-11-12 G2D per-window initialization source build. VC8 x86
compiled without warnings. Ghidra read-only, changed=false.
Next are the DXGI prerequisites at 20690d40, 20690f00 and 20691100.
See g2d-window-initialization.md.

2026-11-12 G2D window construction and controller-list publication
source build. Isolated VC8 object compiled without warnings. Ghidra
read-only. Next are the +0xbc window initialization targets
20685a60 and 2068fc80. See g2d-window-construction.md.

2026-11-12 G2D per-backend restart and initialization source build.
The isolated VC8 object compiled without warnings. Ghidra read-only,
no new unsaved edits. Next is backend window construction and
publication at 206857d0, 2068f710 and 2068f8f0.
See g2d-controller-backends.md.

2026-11-12 G2D shared controller transition and fatal report build.
G2D source is separate in src/g2d-pilot. VC8 object compiled; Ghidra
read-only, no new unsaved edits. Next are per-backend slots +0x44 and
+0x4c. See g2d-controller-transition.md.

2026-11-12 FS98MAIN 0x46f and 0x470 graphics transition source build.
Twenty comparators pass. Ghidra read-only, no new unsaved edits.
Next is the G2D controller behind ordinal 4, slots +0x10, +0x14
and +0x38, and the ordinal 65 failure path. Original table placement
and runtime verification remain open. See api-main-window-graphics-transition.md.

2026-11-12 API main-window modal nesting, focus and close override
source build. Twenty comparators pass. Ghidra read-only, changed=false.
Next is the FS98MAIN 0x46f graphics transition at 2012f4f7.
Original table placement and shutdown-prefix integration remain open.
See api-main-window-modal.md.

2026-11-12 API WinTrust provider binding, signer classification and
release source build. Twenty comparators pass. Ghidra unchanged, no
new unsaved edits. Next is the main-window modal entry/exit pair
20125979 and 201259d8. Protected key and source shutdown-prefix
integration remain open. See api-addon-trust-provider.md.

2026-11-12 API add-on trust decision, profile and key source build.
Twenty comparators pass. Ghidra unchanged, no new unsaved edits.
Next is the WinTrust provider lifecycle and signer classification
at 20147f31, 201238c8, 20123536 and 20123694. The protected key
region and panels.dll caller remain open. See api-addon-trust.md.

2026-11-12 API ordinal 322, EXE.xml and DLL.xml startup and module
lifetime source build. Twenty comparators pass. Ghidra unchanged,
with no new unsaved edits. Next is trust ordinal 305 at 20123c41.
The source shutdown-prefix call and MAIN caller context remain open.
See api-addon-startup.md.

2026-11-12 API listener configuration and default fallback build.
Twenty comparators pass. Ghidra unchanged. Next is API ordinal 322
startup at 200fb207. See api-service-listener-configuration.md.
The caller in main.dll is verified at 20949c24. See
handoffs/HANDOFF-20261112-api-startup.md for the module boundary and resume point.

2026-11-12 API application registration and service startup entry build.
Twenty comparators pass. Ghidra unchanged. Next is listener configuration
and startup callers at 200fb303 and 200fb4d7.
See api-service-application-registration.md.

2026-11-12 API connection reset, close and status callbacks build.
Twenty comparators pass. Ghidra unchanged. Next is application-side
registration removal and startup callback integration.
See api-service-client-connection.md.

2026-11-12 API transport-tree insertion and balancing build.
Twenty comparators pass. Ghidra unchanged. Next is connection reset
and close. See api-service-transport-tree.md.

2026-11-11 API service listener client construction and activation build.
Twenty comparators pass. Ghidra unchanged. Next is transport-tree insertion
at 201165f2. See api-service-client-construction.md.

2026-11-11 API socket and pipe listener startup builds.
Twenty comparators pass. Ghidra unchanged. Client construction,
configuration and transport-tree insertion remain open boundaries.
See api-service-transport-listeners.md.

2026-11-11 API service transport setup and teardown build.
Twenty comparators pass. Ghidra unchanged. Next is socket and pipe
listener startup and registry publication. See api-service-transport-lifecycle.md.

2026-11-11 API outbound service packet delivery builds.
Twenty comparators pass. Ghidra unchanged. Next is transport
tree setup and teardown. See api-service-packet-send.md.

2026-11-11 API service client open and exception response build.
Twenty comparators pass. Ghidra unchanged. Next is outbound
delivery at 2011628d. See api-service-packet-open.md.

2026-11-11 API service packet routing and size checks build.
Twenty comparators pass. Ghidra unchanged. Next is client open
and exception response. See api-service-packet-dispatch.md.

2026-11-11 API receive message handoff targets build. Twenty
comparators pass. The base stub is raw-equal in a direct check.
Table integration remains open. Next is packet dispatch at 200fa39d.
See api-service-message-handoff.md.

2026-11-11 API service client buffering and I/O continuation build.
Twenty comparators pass. Ghidra unchanged. Next is message
interpretation at client slot +0x0c. See api-service-client-buffers.md.

2026-11-11 API service client I/O completion and reference release
build. Twenty comparators pass. Ghidra unchanged. Next is receive
buffering and I/O continuation. See api-service-client-completion.md.

2026-11-11 API service client transport lifecycle builds.
Twenty comparators pass. Ghidra unchanged. Next is I/O completion
and client reference lifetime. See api-service-client-transport.md.

2026-11-11 API client delivery and sorted index build. Twenty
comparators pass. Ghidra read-only. Next is client connection
and teardown at 20114ee9 and 20114e71.
See api-service-client-delivery.md.

2026-11-11 API service dispatch export list and client registration
build. Twenty comparators pass. Ghidra unchanged. Next is client
delivery and index ownership. See api-service-shutdown.md.

2026-11-11 API main-window service callback and dispatch enable build.
Twenty comparators pass. Ghidra read-only. Next are dispatch-export
storage and client registration. See api-service-shutdown.md.

2026-11-11 API service window initialization and queue routing
build. Twenty comparators pass. Ghidra unchanged. Next is the
main-window consumer 200f724b and its client dispatch.
See api-service-shutdown.md.

2026-11-11 API service-window and queue shutdown builds.
Twenty comparators pass. Ghidra unchanged. Next is service-window
initialization and routing at 200f74ab and 200f7426.
See api-service-shutdown.md.

2026-11-11 API callback dispatch and deferred removal build.
Twenty comparators pass. Ghidra unchanged. Next is service-window
and queue cleanup at 200f757e. See api-service-shutdown.md.

2026-11-11 API service shutdown and callback removal build.
Twenty comparators pass. Ghidra unchanged. Next is callback
dispatch and compaction at 200f67bd and 200f6688.
See api-service-shutdown.md.

2026-11-11 API main-window module shutdown builds and is connected
to the shutdown body. Twenty comparators pass. Ghidra unchanged.
Next is API service cleanup 200f7c66. See api-main-window.md.

2026-11-11 API session registration and keyboard hook installation
build. Twenty comparators pass. Ghidra unchanged. Next is module
shutdown at 2012260f. See api-main-window.md.

2026-11-11 API FS98MAIN creation and graphics startup phase builds.
Twenty comparators pass. Ghidra unchanged. Next is startup
completion at 20122587. See api-main-window.md.

2026-11-11 API monitor geometry loop and display-error dependencies
build. Twenty comparators pass. Ghidra unchanged. Next is SIMPROP
initialization and FS98MAIN creation at 20122437.
See api-window-geometry.md.

2026-11-11 API MAIN startup profile and language gate build.
Twenty comparators pass. Ghidra unchanged. Next is monitor geometry
and retry at 20122412. See api-main-window.md.

2026-11-11 API startup window class sequence and error reports
build. Twenty comparators pass. Ghidra unchanged. Enclosing startup
integration remains open. Next is profile setup at 20122332.
See api-main-window.md.

2026-11-11 API FS98FLOAT class registration and message routing
build. Twenty comparators pass. Ghidra read-only. Startup class
registration caller 2012205d remains open. See api-panel-float-window.md.

2026-11-11 API panel window record creation and finalization build.
Twenty comparators pass. Ghidra unchanged. Next is FS98FLOAT
registration and message routing. See api-panel-window-record.md.

2026-11-11 API menu host initialization, focus and canvas targets
build. Twenty comparators pass. Table placement and window
record creation and finalization remain open.
See api-menu-host-init.md.

2026-11-11 API menu host table comparison and teardown targets build.
Twenty comparators pass. Ghidra read-only. Table placement and
host initialization 20138fee and 201391d1 remain open.
See api-menu-host-teardown.md.

2026-11-10 API menu host creation and factory callbacks build.
Twenty comparators pass. Ghidra read-only. Next are API tables
200f3370 and 200f3410 and host teardown. See api-menu-host-create.md.

2026-11-10 API panel menu commands and context-click route build.
Twenty comparators pass. Ghidra read-only. Host creation
2013a534 and 2013a5be and special click route 2012cfc5 remain
open. See api-panel-menu-command.md.

2026-11-10 API panel menu construction and WM_INITMENU state build.
Twenty existing comparators pass at 5263 code and 288 metadata
bytes. Ghidra read-only. Next is menu command execution 20141853
and the pointer-to-menu caller 2012d08c. See api-panel-menu.md.

2026-11-10 API menu-host lookup, root property attachment/removal
and panel handle release build. Twenty comparators pass at 5263
code and 288 metadata bytes. The former graphics-context reading
of 20138f47 is superseded by the MenuHostWindow interface evidence.
Next is menu construction 2012b637 and menu state 20141195.
Ghidra read-only, no new unsaved edits. See api-panel-menu-host.md.

2026-11-10 API child refresh 0x467, private DIB rebuild and shared
surface callbacks build. Twenty existing comparators pass at 5263
code and 288 metadata bytes. Ghidra read-only with no new unsaved
edits. Graphics-context release integration and runtime verification
remain open. See api-panel-surface-refresh.md.

2026-11-10 API panel geometry and FS98SHADOW drag build. Twenty
existing comparators pass at 5263 code and 288 metadata bytes.
Ghidra read-only with no new unsaved edits. Next package starts at
0x467 and surface refresh 2012b9af. Shadow-container integration
and runtime verification are open. See api-panel-geometry-shadow.md.

2026-11-10 API FS98CHILD and FS98DLG3D procedures, context lifetime
and both dispatchers build. Twenty gates pass at 5263 code and 288
metadata bytes. Ghidra read-only, no new unsaved edits. Next proposed
package panel geometry and drag. See api-panel-message-routing.md.

2026-11-10 API WM_SETCURSOR, hit-test fallback, cursor override and
pointer polling build. Main route connected and original panel routes
verified. Twenty gates pass at 5263 code and 288 metadata bytes.
Ghidra read-only, no new unsaved edits. Cursor topic ends at larger
panel dispatch and external controls ordinal 45. See api-cursor-events.md.

2026-11-10 API panel hover text setter and refresh build. Twenty
gates pass at 5263 code and 288 metadata bytes. Ghidra read-only,
no new unsaved edits. Next cursor consumer 2012caa7 and fallback
2011fb07. See api-panel-hover-text.md.

2026-11-10 API hit-group removal, recursive cleanup and tooltip
destruction build. Twenty gates pass at 5263 code and 288 metadata
bytes. Ghidra read-only, no new unsaved edits. Next hover text
replacement at 20136409 and 20136ae6. See api-panel-hit-removal.md.

2026-11-10 API panel hit registration, recursive bounds and exports
146/147/186/187 build. Twenty gates pass at 5263 code and 288 metadata
bytes. Ghidra read-only, no new unsaved edits. Next hit removal and
cleanup at 201368ce. See api-panel-hit-registration.md.

2026-11-10 API cursor conversion and tooltip creation build.
The hover receiver is tooltips_class32. Twenty gates pass at 5263
code and 288 metadata bytes. Ghidra read-only, no new unsaved edits.
Next hit registration 20136645 and 2013670f. See api-cursor-conversion.md.

2026-11-10 API panel cursor selection, cache and hover packet build.
Twenty gates pass at 5263 code and 288 metadata bytes. Ghidra
read-only, no new unsaved edits. Next cursor conversion 2011fbc1
and hover message receivers. See api-panel-cursor-hover.md.

2026-11-10 API panel hit visitor and capture dispatch build. Twenty
gates pass at 5263 code and 288 metadata bytes. Ghidra read-only,
no new unsaved edits. Next cursor and hover at 20136dec and 20136532.
See api-panel-input-hits.md.

2026-11-10 API window input dispatcher and repeat checks build.
Twenty gates pass at 5263 code and 288 metadata bytes. Ghidra
read-only, no new unsaved edits. Next hit visitor 20137248 and
captured input 20136e98. See api-window-input-dispatch.md.

2026-11-10 API capture/repeat publication and release build. Twenty
gates pass at 5263 code and 288 metadata bytes. Ghidra read-only,
no new unsaved edits. Next input dispatcher 2013754f.
See api-timer-interaction-lifecycle.md.

2026-11-09 API timer window-thread input reconciliation and callback
dispatch build. Twenty gates pass at 5263 code and 288 metadata bytes.
Ghidra read-only, no new unsaved edits. Next capture/repeat lifecycle
20137891, 201378f2, 20137988 and 201379d1. See api-timer-window-work.md.

2026-11-09 API startup readiness tail, multimedia timer and both
message routes build. Twenty gates pass at 5263 code and 288 metadata
bytes. Ghidra read-only, no new unsaved edits. Larger Initialize body
and 0x465 work remain boundaries. Next 20137b6b and 20137a9a.
See api-startup-timer.md.

2026-11-09 API resource module registration/removal and both string
loaders build. Splash, language and WM_COMMAND share the loader.
Twenty gates pass at 5263 code and 288 metadata bytes. Ghidra read-only,
no new unsaved edits. Return to API initialization readiness and timer
setup at 20120d8d..20120ed4, then 201228f0. See api-resource-strings.md.

2026-11-09 API product text and +0x64 shell target build. Original
product-text flow verified at 691 instruction and 16 descriptor
bytes. Protected query and vtable placement remain boundaries.
Twenty gates pass at 5263 code and 288 metadata bytes accepted.
Ghidra read-only, no new unsaved edits. Next resource loader 20138550.
See api-main-window-product-text.md.

2026-11-09 API WM_COMMAND, prefilter and default-size request build.
0x110a translates to callback event 0x100c4 and is consumed. Twenty
gates pass at 5263 code and 288 metadata bytes. Ghidra read-only,
no new unsaved edits. Next 201205a6 product text and +0x64 launch
target. See api-main-window-command.md.

2026-11-09 API WM_CLOSE and exports 4/5 build. FS98MAIN, the message
loop and clip code share quit flag 2020e044. The termination call
uses demo.dll ordinal 7 as an external boundary. Twenty gates pass
at 5263 code and 288 metadata bytes. Ghidra read-only, no new unsaved
edits. Next WM_COMMAND 2012e96f and helper 2012fded.
See api-main-window-close.md.

2026-11-09 API FS98MAIN registration, controller construction, teardown
and procedure build. Sizing messages route to source handlers. Twenty
gates pass at 5263 code and 288 metadata bytes. Both cleanup paths
emitted in the adopted SEH model. Ghidra read-only, no new unsaved edits.
Next remaining main dispatch, starting with WM_CLOSE at 2012edbb and
shutdown gate 2020e044. See api-main-window-controller.md.

2026-11-09 API main-window sizing capture, exit, snapshot paint and
child visitors build. FS98MAIN controller aliases at 2020de9c/2020de98
and +0x3d sizing writes resolved. Twenty gates pass at 5263 code and
288 metadata bytes. Ghidra read-only, no new unsaved edits.
Next connect controller construction and teardown at 20125350 and
2012e506 to the FS98MAIN procedure. See api-main-window-sizing.md.

2026-11-09 API WM_PAINT and clip drawing at 20140d00 and 2012c588
build. Both message dispatchers traced. Twenty gates pass with 5263
code and 288 metadata bytes accepted. Ghidra read-only, no new unsaved
edits. Next trace 2020de9c and the writers of its +0x3d paint byte.
See api-panel-paint.md.

2026-11-09 API full and partial bitmap drawing, graphics update request,
and two message callers build. Twenty gates pass at 5263 code and
288 metadata bytes. Stack-overflow handler references are emitted.
Next paint/clip path 2012c588. Live api.dll saved, modification 3.
See api-panel-bitmap-draw.md.

2026-11-09 API alternate panel transfer, shared memory DC producer,
surface registry and bitmap blit helpers build. Twenty gates pass at
5263 code and 288 metadata bytes. The 36-byte notification wrapper
matches and was already counted. Next bitmap drawing caller 2012c719.
Live api.dll saved, modification 3. See api-panel-surface-lifecycle.md.

2026-11-09 API panel transfer, format kernels, palette cache and shared
DC wrappers build. Twenty gates pass. The acquire wrapper adds 88 bytes,
for 5263 complete code and 288 metadata bytes. The release wrapper was
already counted. Next alternate transfer caller 2012d3fb and DC producer.
Live api.dll saved, modification 3. See api-panel-pixel-transfer.md.

2026-11-09 API panel update region, sole caller and transfer preparation
build. Nineteen gates pass at 5175 complete code and 288 metadata bytes.
Both shared HRGNs now have source producers and last-group cleanup.
Next API rectangle transfer 2011f7d3. Graphics imports remain external.
Live api.dll saved, modification 3. See api-panel-update-regions.md.

2026-11-09 API deferred window ordering, shared overlap/update region,
raise and window resource teardown build. Nineteen gates pass. Export 28
adds 106 complete code bytes, for 5175 code and 288 metadata.
Next related region consumer 2011e6cb. g2d.dll remains external.
Live api.dll saved, modification 3. See api-window-region-order.md.

2026-11-09 API clip-region rebuild, graphics index cache and window-group
storage/lifecycle build. Both runs pass 18 gates at 5069 complete code
and 288 metadata bytes. Related region consumer 2011d580 and registration
callers remain open. g2d.dll remains external. Live api.dll saved,
modification 3. See api-window-clip-regions.md.

2026-11-09 API controller mode update, visibility refresh, clip guard
and floating-window visitor build. Eighteen gates pass at 5069 code
and 288 metadata. Next API clip-region rebuild 2011d836. The g2d.dll
ordinal remains an external boundary. Live api.dll saved, modification 3.
See api-window-controller-mode.md.

2026-11-09 API dialog registry, message dispatch, deferred-removal
iterator and controller hide build. All 18 gates pass at 5069 code and
288 metadata. Next controller mode update 20125b7f and callbacks.
Controller table placement, shutdown prefix and runtime remain open.
Live api.dll saved, modification 3. See api-dialog-registry.md.

2026-11-09 API numbered-name exports and private-heap release build.
Eighteen gates pass. The complete 43-byte release helper adds coverage,
for 5069 code and 288 metadata bytes. Name bodies remain outside
acceptance. Next dialog registry/message dispatch around 2013246b.
Live api.dll saved, modification 3. See api-numbered-names.md.

2026-11-09 selected and default API configuration writes, parser
invalidation and profile flush build. The final run passes 17 gates.
WORD/BYTE wrappers add 54 complete bytes, for 5026 code and 288 metadata.
Shared writer bodies and runtime remain open. Next numbered-name helpers
around 201322e2. Live api.dll saved, modification 3.
See api-stream-parser-writes.md.

2026-11-09 API configuration enumeration, numbered-section count,
typed scalar/list reads and string postprocessing build. Sixteen gates
pass at 4972 complete code and 288 metadata bytes. Native linking,
runtime edge cases and parser construction unwind emission remain open.
Live api.dll saved, modification 3. See api-stream-parser-queries.md.

2026-11-09 API parser tokenizer, section/key indices, five-entry parser
cache and default-configuration queries build. Both runs pass 16 gates
at 4972 complete code and 288 metadata bytes. Original parser allocation
unwind verified; native emitted cleanup and resolver failure output remain
open. Next parser enumeration/typed queries. Live api.dll saved,
modification 3. See api-stream-parser.md.

2026-11-09 stream-cache registration/lock callback, application shutdown
continuation, CAB and suffix teardown, file create/copy and parser stream
load/cleanup build. Sixteen comparators pass on both runs. Two complete
registration bodies add 37 bytes, for 4972 code and 288 metadata. Verified
util.dll JM token and integer-exhaustion paths statically. CRT placement,
shutdown prefix/subsystems, parser interpretation, transform verification
and native linking remain open. Live api.dll saved, modification 3.
See api-file-stream-lifecycle-consumers.md.

2026-11-09 decoder 20132e20, size helper 20133611 and independent file-stream
cache build and connect to export 85. Both builds pass all 15 gates at
4935 complete code bytes and 288 metadata bytes. Original util.dll ordinal
195 forwarding and API-facing contract are verified. The transform core,
cache startup/teardown wiring and native linking remain open. Live api.dll
readback was saved (changed=false, modification 3). See
api-file-stream-decode-cache.md.

2026-11-09 export 85 at 20133137 builds with direct-file, CAB fallback and
buffered wrapping connected. Decoder and separate stream cache are explicit
boundaries. Fifteen gates retain 4935/288. Next decoder 20132e20, seek/size
helper 20133611 and util.dll ordinal 195. Ghidra remains saved and unchanged.
See api-file-stream-open.md.

2026-11-09 CAB extraction and the Initialize FDI open callback build together.
20144f5f returns a stream, superseding the size-consumer hypothesis. Both
consumers key on the close notification. Fifteen gates retain 4935/288.
Next trace the 20133137 direct-file/CAB selection and ownership transitions.
Ghidra remains saved at revision 1793741866691:2a9d629f:3.
See api-cab-extraction.md.

2026-11-09 CAB owner Stat behavior verified for raw file, wrapped file and
memory streams. Reload failure outputs stay explicit. Entry lookup 20144b9b
and attribute fallback 2014507f now build and connect to path attribute
query. Fifteen gates pass at 4935/288. Ghidra saved revision unchanged.
Next entry-open and size consumers through FDI dispatch 20144924.
See api-cab-owner-metadata.md.

2026-11-09 CAB buffer arrays and cache lifetime build. Allocation and return
use typed container helpers. Registration at 201ee724 adds 17 code bytes.
Fifteen gates pass at 4935 code and 288 metadata bytes. Owner reload at
201449c4 remains an explicit boundary after confirming both unwritten output
paths. Next trace owner metadata Stat at 20135378. Ghidra remains saved at
revision 1793741866691:2a9d629f:3. See api-cab-buffer-containers.md.

2026-11-09 both 14-slot stream VTables and remaining methods build in
api-cab-stream-tables-20261109-002. Five methods add86 code bytes and two
tables add112 metadata bytes. Fourteen gates pass at4918/288. Shared AddRef
counted in prior generated coverage. Private copy/clone and unwind limits
remain. See api-cab-stream-tables.md. Next buffer container and owner reload
integration. Live Ghidra saved state confirmed.

2026-11-09 stream pool registration and owner tables. Source now connects
guarded getters, atexit callbacks, owner storage and eleven slots per table.
The pre-reset callback is capacity minus available with a shared-lock scan.
Eleven raw bodies add97 code bytes and 33 validated table fields add132
metadata bytes. Thirteen gates pass at 4832 code and 176 metadata bytes on
api-cab-pool-registration-eh-20261109-001. CRT placement and emitted unwind
remain open. See api-cab-pool-registration.md. Next actual file/wrapper stream
VTables200f2c70/200f2d50 and remaining slots. Ghidra saved state confirmed.

2026-11-09 pool acquisition modes0/1/3 are modeled and connected to all five
pool call sites. Verified distinct queue, wake retry and rollback transitions.
Twelve gates pass on api-cab-pool-acquire-20261109-001 at 4735 code bytes and
44 metadata bytes. Source is pool-specific and outside body acceptance.
Runtime contention remains unverified. See api-cab-pool-acquire.md. Next
owner registration and pre-reset callback chain201128d7. Ghidra saved state
confirmed.

2026-11-09 lock and owner lifecycle package. Lock release/create/destroy plus
both owner lifecycles, reset, trim and registry unlink compile in
api-cab-pool-lifecycle-20261109-001. Twelve gates pass at 4735 code bytes and
44 metadata bytes. Registry unlink stopped at two unmatched trials and first
source retained. Pool acquisition modes0/1/3 mapped, including upgrade errors
and wait helper register arguments. Acquisition source remains external.
See api-cab-lock-lifecycle.md. Ghidra saved state confirmed.

2026-11-09 slot publication and reservation source integrated in
api-cab-slot-transfer-20261109-001. Three x86 primitives preserve emitted
LOCK and unprefixed forms, with distinct mode thresholds. Twelve gates pass
at 4735 code and 44 metadata bytes. Both transfer bodies remain outside
acceptance. Next pool lock wait/wake pair20146e07/20147aa6. See
api-cab-slot-transfer.md. Ghidra saved state confirmed.

2026-11-09 slot state, growth and backing storage source compile in
api-cab-slot-storage-20261109-001. Eight bodies remain outside acceptance.
Twelve gates pass at 4735 code bytes and 44 metadata bytes. Nine original
listings cover 1544 verified bytes. Next tagged-head push/pop and transfer
contracts for 201462fc and 201464c3. See api-cab-slot-storage.md.
Ghidra saved state confirmed.

2026-11-09 shared pool allocation, return and block initialization compile in
api-cab-pool-blocks-20261109-001. Six entry points use three shared helpers.
Twelve gates pass at 4735 code bytes and 44 metadata bytes. Mode 3 upgrade and
mode 1 retry flow verified. Slot transfer and lock operations remain source
boundaries. Next recover slot state creation, destruction and node storage.
See api-cab-pool-locks.md. Ghidra saved state confirmed.

2026-11-09 buffer cache and pool checkpoint. Cache search, allocation and return
compile in api-cab-buffer-cache-20261109-001. All twelve gates pass at 4735 code
bytes and 44 metadata bytes. The three cache bodies remain unmatched. The second
search trial produced75/74 bytes and the clearer first source was retained.
File/wrapper pool listings correspond across167 instructions and23 branches.
See api-cab-buffer-pools.md. Next recover lock upgrade and slot return pairing
before translating shared pool source. Ghidra saved state confirmed.

2026-11-09 stream methods: six structured read/seek/stat methods compile. File
read57/file seek64 plus wrapper seek92 add213 complete bytes: current4735 code,
metadata44 separate. Twelve gates pass on api-cab-wrapper-seek-20261109-001.
Remaining stat/read bodies198/202,325/350,98/96 native/original unmatched;
stop initial questions, seek succeeded trial2. Wrapper fields reconciled from
usage: bufferStart+c, validBytes+10, position+28 (source renamed, DB unchanged).
See api-cab-stream-methods.md. Next shared pool/reservation/buffer dependencies;
owner failure edge and emitted-EH blockers remain. No runtime/tab/DB changes,
live unsaved_changes=false/modification3. Six listings861 original bytes verified.

2026-11-08 stream producers: file/wrapper producer, wrapper initialize/destroy
structured source builds in api-cab-stream-producers-20261108-001, native
157/151,59/102,215/227,94/77 (native/original), all unmatched. Verified AddRef/
Release ownership, original file-handle leak on pool failure, and distinct pool
return on constructor unwind. Fixed profile omits emitted wrapper EH. Twelve
listings1624 bytes verified; eleven gates retain4522/44. Owner reload failure
still not proven unreachable. See api-cab-stream-producers.md. Next wrapper/file
read/seek/stat functional methods, then pool/buffer dependencies; no ABI matrix.
No DB edits/tab changes/runtime; live unsaved_changes=false/modification3.

2026-11-08 CAB image/stream checkpoint: image2014420b source671/700 builds;
prefix length comes from following8-byte record, not whole-CAB loading. Release
201333ac33 and read201333cd39 full raw matches add72: current4522 complete code,
44 metadata separate. Eleven gates pass on api-cab-stream-read-20261108-001.
Owner reload201449c4 remains external: path-failure branch reads uninitialized
stream stack slot; no invented NULL repair. See api-cab-image.md. Eight listings
1568 bytes verified. Next stream producers201246c4/20127440/20126be0 and owner
failure reachability/ownership; image question stopped at1, read at2/success.
No Ghidra edits/tab changes/runtime; live unsaved_changes=false/modification3.

2026-11-08 CAB owner/cache follow-up: structured path, initialize, release/unlink
and normal resolver source compile in api-cab-cache-20261108-001; ten gates
retain4450/44. Original constructor unwind action201ee2d3 and one-entry map
verified; frees raw allocation through200f40a9, no cache-lock release. Fixed
pilot profile omits emitted resolver unwind cleanup despite placement-delete
source intent; explicit blocker, no further matching trials. Four new bodies
remain unmatched (443/376,106/105,144/128,259/311 native/original). See
api-cab-owner-cache.md. Next201449c4/2014420b CAB load/stream/error ownership
source package. No DB edits/tab switches/runtime; final unsaved_changes=false.

2026-11-08 CAB enumeration checkpoint: consolidated dependency/resource evidence
in api.dll/api-module-references.md; original language strings corroborate title/24-bit
warning. Backend identified from .CAB discovery + MSCF checks. Three structured
routines added (20145110/20144d5d/20144df0), native96/144/441 versus89/147/367;
unmatched, stop initial questions at1 on known register ABI blocker. Eight
original ASM listings byte-reverified. Cursor0x318/record stride and unchanged
last-write timestamp documented; constructor2014483b actually returns owner.
Ten gates pass on api-alternate-enumeration-20261108-001, complete total4450 and
metadata44 unchanged. See api-alternate-enumeration.md. Next CAB path/cache and
exception ownership contracts before another match trial. No Ghidra mutation,
tab switch or runtime execution; live unsaved_changes=false/modification3.

2026-11-07 broad API source pass (partial; whole DLL not finished): all5,077
internal model functions exported as decompilation/ASM plus789 external
declarations. All321,805 exported instructions/879,524 bytes verify against
the pinned original.7,406 unassigned instructions remain; model/decompiler
completeness is not real-flow completeness.567 generated C++ candidates and
nine new hand-written routines compile on VS2005 SP1. Latest run
api-source-message-loop-20261107-001, object141E6DC83FC6DF94F394F333CE96E889607A7CEED903DD331383597822C72623.
Ten comparison gates pass: prior1120 code/44 metadata retained;88 generated
bodies add3116 disjoint bytes (46/1935 raw-equal), foreground-idle hook20122a0b
adds214 target-validated bytes. Complete-body total4450; metadata44 and
Initialize partial123 separate. Enumeration, suffix registry, idle/message
loop source added; eight of nine manual additions unmatched. Generator keeps
all deferred reasons and193 native-contract blockers, no guessed arguments.
See api.dll/api-source-pass.md, source manifest and fixed accepted-body JSON. Whole
DLL/source/link work remains open: custom ABI, indirect calls/types, native
flows/teardown, globals, entry/CRT, exports/resources. No Ghidra mutation/tab
switch/runtime execution; live final readback unsaved_changes=false,
modification3. Worker ready. Broad first pass is not completion of user request.

2026-11-07 interruption recovery: font/enumeration build and all eight accepted
body gates pass on api-font-resources-20261103-001 (rechecked today). Close
20134929 adds92 complete bytes: total1120 code,44 metadata separate. Font
20121083 source891/733 remains unmatched; initial question stopped at1.
Five Ghidra signatures/two observed types survived; fresh readback revision
1793741866691:19c8b8ae:3 has unsaved_changes=false. No manual save needed.
See api.dll/api-font-resources.md and ledger. Next enumeration/custom ABI dependencies;
newly loaded modules are reference material, not evidence of completed coverage.
Module/resource cross-reference documentation from the interrupted session still
needs consolidation. No runtime or whole-DLL match; layout/import search deferred.

2026-11-03 geometry/language checkpoint: observed MONITORINFOEXA72 and context184
offsets verified by assembly/SDK assertions. Monitor callback309/336, selector
407/409, geometry575/585 and error233/233 source builds remain unmatched; stop
initial questions at1, no alignment/padding matrix. Four Ghidra interfaces and
two reconstructed record types applied after GZF; saved revision710 unsaved=false,
four live spans equal original. Error handler returns1/E_FAIL, no observed0
despite caller retry edge. Details in api-window-geometry.md.
Language2013e64f old50-byte/no-op view superseded by success inventory199
instructions/729 bytes+128 descriptor bytes, full interval2013e64f..2013e9a7.
Normal-flow source357 builds; integrity/formatter boundaries explicitly open.
Seven gates pass on api-language-module-20261103-001:1028 complete code/44
metadata unchanged. Flow-reader short-LEA extension regresses prior inventories.
See api-language-module.md. Next font lifecycle20121083 and enumeration contracts,
then dependency integration. No candidate execution/tab switches; source goto-free.

2026-11-03 main-window checkpoint:2012205d old76-byte view superseded as whole
behavior; read-only normal/success walk398 instructions/1346 code bytes plus
seven16-byte integrity descriptors covers2012205d..2012260e, seven realRET4 exits.
Inventory verifies pinned original and prior init465/1596 inventory regresses clean.
Source FS98CHILD20121360100 bytes differs only frame reserve28h/2ch; rejected,
no padding. Structured keyboard hook20122834 matches full188 bytes on trial3,
8 mapped fields/three controls; all seven accepted-body gates pass in
api-main-keyboard-hook-20261103-003. Central ledger1028 complete code/44 metadata.
Ghidra hook/class types + main warning comment applied after GZF; names/bodies
preserved. Saved1793739333501:636d9e2e:603 unsaved=false, five live spans equal
original. See api-main-window.md. Next geometry20121796/201215fd,72-byte monitor
layout and error-response201213c4, then font/2013e64f contracts. No runtime/tab
changes; canonical source goto-free. Routine autonomy remains authorized.

2026-11-03 larger splash/path checkpoint: append20137ec062 and length20137fb034
full object matches; six regression scripts pass in api-splash-image-20261103-001.
Central ledger840 complete code/44 metadata, Initialize partial123 separate.
Image2013afdd returns LoadImageA file bitmap uires\\dlgsplash.bmp; source488/445
unmatched after1 trial, custom copy/trim/inlining residuals retained. Four image/
path signatures actually corrected after GZF, existing names preserved, saved
revision1793738170185:636d9e2e:567 unsaved=false. Image bytes unchanged; no tab
navigation/runtime. Next main-window2012205d true flow before reconstruction.
User authorizes all routine next steps; pause only at meaningful checkpoints or
specific material/decision need. Maintained source remains goto-free.

2026-11-03 user requests avoiding goto where practical. Removed sole canonical
source goto in startup_window.cpp; no src C++/header goto matches remain. Added
splash procedure2013b1c1 source;620 native versus475 original, explicit custom
rectangle ABI unresolved. Structured start225 versus original227. Five gates
pass in api-splash-procedure-20261103-001;744 complete code/44 metadata unchanged.
Next raw image2013afdd return/control and path helpers before source (decompiler
void/unreachable warnings). No Ghidra edits/runtime, saved revision507 last checked.
Autonomous continuation persists; new source should use structured control flow.

2026-11-03 autonomous continuation: disk query source363/318, splash start226/227,
thread354/381 remain unmatched; stopped each native source question at1. Splash
stop2013b6a4 matches full98 bytes/12 fields with string checks and three rejecting
controls. All five gates pass in api-startup-thread-20261103-001. Correct total
744 complete code/44 metadata; previous648 was arithmetic error (507+139=646).
Central address ledger:api-code-ledger.md. Thread class FlightSimSplash proves
separate splash HWND2020e0a4, not main HWND2020e010. Next procedure2013b1c1/
image2013afdd and teardown, then main window2012205d. No DB edits/runtime;
last checked saved revision507. Routine continuation no longer needs user OK.

2026-11-03 fallback201325f1 source builds125/125 bytes, full comparator rejects
operand load/order difference. Trial002 operand swap leaves identical helper
code: stop at2. Preserved32-bit product then signed64 widening, BOOL forwarding,
mandatory available/total outputs and optional freeBytes; no new coverage.
Prior648 code/44 metadata gates pass. Next query20133e19 source/path-split
interface20133b3b. Runs api-disk-fallback-20261103-001/002, topic details and
diagnostics retained. No database edits/runtime; last checked saved revision507.

2026-11-03 API-05c dispatcher2013266e full139-byte match on trial2;12 mapped
fields/string identities and initial pointer201f7538 checked. Three mutations
reject, prior507/44 gates pass:648 complete object-code bytes,44 metadata.
GetDiskFreeSpaceExA lookup/fallback78h established; startup query output is
available-to-caller space>=30MiB, RAM gate total physical>=64MiB. See
api.dll/api-initialization-flow.md for exact return/error/bypass behavior. Next fallback
201325f1, then query20133e19 source. No database edits/runtime/linked DLL claim;
last verified saved revision507. Stop successful dispatch question at2 trials.

2026-11-03 error wrapper201230da matches complete48 bytes on first trial
api-init-error-20261103-001, three individually validated global fields, original
export264 independently checked; three negative controls reject. Prior459/44
gates pass; new complete object-code total507, metadata44, partial Initialize123
separate. No linked DLL/export/runtime claim, no Ghidra edits; last checked saved
database revision507. Next API-05c memory/free-space gates20133e19 and target
201f7538. See api.dll/api-initialization-flow.md; stop successful wrapper trials at1.

2026-11-03 suffix source boundary built in api-path-suffix-20261103-001:
251 native versus182 original bytes; copy inlined and custom original EAX/ECX
ABI differs. Exact source branches/special cases retained; no new match.
Prior459 code/44 metadata gates pass. Stop at1 without compiler permutations;
bounded path source slice closed for now. Next regular-stack error-callback
wrapper201230da48, then API-05c memory/free-space checks. No database mutation,
last checked saved revision507; artifacts/details in api-initialization-flow.md.

2026-11-03 copy source boundary implemented:82 native versus86 original bytes,
custom original EAX/EDX/RET4 versus source stack3/RET12, unmatched. Runs
api-path-copy-20261103-001/002 retain diagnostics; empty-conditional cleanup
leaves identical helper code, stop at2. Prior459 code/44 metadata gates pass.
No runtime/database edits; last verified saved API revision507. Next selector
20134fc4 source, then leave custom-ABI infrastructure for other startup helpers.
See api.dll/api-initialization-flow.md; no new matching coverage or extra input needed.

2026-11-03 natural resolver source built in api-path-resolver-20261103-001:
238 native bytes, no match claim; explicit unresolved source boundaries preserve
visibility of two custom-register ABI residuals. Copy20137e17 stack3/RET12 and
pointer result inspected. Prior459 code/44 metadata gates pass; attribute202
still unmatched. No DB changes, last saved revision507. Next bounded86-byte copy
source200f79d5, then selector dependency; no flag/buffer sweeps. Final module
architecture remains separate fsx.exe/api.dll; current trial is COFF only.

2026-11-03 cookie helper20148b71 model repaired after backup: void ECX argument,
zero purge and built-in security_check_cookie call fixup. Exact15 original/live
bytes and EAX/EDX preservation on returning path verified; mismatch target remains
existing noreturn20149017. Resolver now decompiles correct0/-1 returns; previous
artifact superseded. Saved readback unsaved=false in api-cookie-model-saved.
No byte/body/tab/build/coverage changes. Next resolver source with explicit
custom ABI dependencies;459 complete code/44 metadata retained.

2026-11-03 applied seven path interfaces with explicit custom storage/stack
cleanup and EAX results; established FUN names preserved. Pre-edit GZF and
transaction/readback script retained; API saved and unsaved=false confirmed.
See api.dll/api-initialization-flow.md and api-path-typing-{readback,saved}-20261103.json.
No code-byte/body/table edits or build;459/44 scope unchanged. Next inspect
cookie helper20148b71 stored signature/EAX preservation: resolver decompilation
still incorrectly attributes its0/-1 result to that helper. No tab switches.

2026-11-03 resolver/selector contract resolved:20134fc4 uses EAX selector,
ECX input and stack output/RET4, suffix table200efd98 (1 .BGL,36 .CFG), dynamic
suffix extension table and special-2.20133d17 stdcall4 combines relative paths
against2020d360, canonicalizes, returns0/-1. Copy200f79d5 and append201203f4
also have custom register ABIs. See api.dll/api-initialization-flow.md and retained
assembly/table snapshots. Next coordinated Ghidra custom-storage typing before
natural source matching; no ABI-substitution builds. No new coverage/build/DB
edit;459 complete code/44 metadata retained, API revision314 saved.

2026-11-03 API attribute dependency source compiled202/202 bytes in trial
api-path-attributes-20261103-001; unmatched frame/buffer offsets and scheduling,
no new coverage. Seven native references inventoried diagnostically. Prior459
complete object code/44 metadata gates retained. ResolvePath20133d17 stack4/
RET16 and fallback2014507f stack1/RET4 verified independently of misleading
decompiled signatures. Source-shape question stopped at1 without buffer/flag
guessing. Next path resolver/selector20134fc4 dependency analysis. Details in
api-initialization-flow.md. No Ghidra edits; revision314 remains saved.

2026-11-03 API-05b wrapper trial1 succeeds:20132c8e complete49-byte COFF body,
one validated call relocation to unresolved2013341b; opcode/target controls
reject. Prior410 code/44 metadata gates pass, disjoint complete code total459;
partial Initialize123 unchanged. See api.dll/api-initialization-flow.md milestone and
api-path-wrapper-20261103-001 run. Both helper/wrapper0/-1 return contracts now
assembly-verified; Ghidra stored undefined signatures not yet edited. No database
change; revision314 saved. Next202-byte attribute dependency2013341b. Stop
successful wrapper question at1 trial; no linked DLL/runtime claim.

2026-11-03 API-05a initialization flow inventory complete within explicit native,
call-opaque bounds20120869..20120ed4:465 instructions/1596 original-verified
bytes, three8-byte descriptors and three skipped8-byte ranges. Three RET4 stack
transfers resolved through201ecc2f/201ecc87; both helpers advance saved return
by8. Initial133-instruction probe extent superseded. No unresolved transfer in
this bounded walk; detached EH/callee/runtime completeness not claimed. Handler
201ee510 reaches __CxxFrameHandler3, not outer wrapper EH4. No new body match,
build or Ghidra edits. See api.dll/api-initialization-flow.md for phases/subpackages.
Next API-05b:49-byte path wrapper20132c8e/export75 and202-byte attribute helper
2013341b/export68; resolve stored return/signature contradictions before source.
Native trials limited to3 per explicit question; infrastructure matrices stay
closed. API saved revision1793724055614:636d9e2e:314, unsaved=false; no navigation.
No installation input needed; object410/metadata44 and partial Initialize123
coverage scopes unchanged. Final linked factory/export144 remains open.

2026-11-03 factory milestone: ordinary virtual bridge/placement construction now
matches complete GetApplication72-byte COFF body after7 validated relocations;
compiler-generated observed VTable16 has four correct defined method targets.
Trial1 was75 bytes; one source-only returned-pointer change gives72 on trial2.
Stopped successfully. Scope in final factory object410 unique complete code bytes,
metadata44; Initialize still only123 scoped bytes/unmatched whole. Window21 also
retains linked DLL regression and overlaps object evidence (do not double-count).
Source /GR- matches no-COL bridge shape; not original whole-DLL flag proof.
Final linked factory/export144/runtime remains open. See api-factory-matching.md.
Ghidra getter evidence comment added after GZF, existing unknown convention kept;
saved revision1793724055614:636d9e2e:314, unsaved_changes=false. Original bytes
unchanged, API current as user selected, no agent navigation. Next API-05 callback
20120869 reachable-flow/phase analysis; no renewed native infrastructure matrix.

2026-11-03 callback registry/realloc source slice: real16-byte header/16-byte
record registration and185-byte reallocator models compile in API-owned native
objects. Realloc14 references and mode4 restoration/NULL retry checked; no new
whole-body match (register/inline/ABI differences remain). Source registry uses
explicit stack owner; original ESI model retained. Artificial internal-helper
export removed with all4 generated function bodies unchanged. Stop this bounded
infrastructure question; existing317 full object-code/28 metadata and Initialize
123 scoped-byte gates pass regression. See api-callback-registry.md.
Ghidra realloc renamed/typed stdcall3 args; pre-edit GZF/readback, saved revision
1793722735273:636d9e2e:312, unsaved_changes=false. Both live helper byte ranges
equal original, no tab/code-byte changes. Next return to GetApplication/export144
and four-slot VTable/RTTI-prefix evidence, rather than more native heap/ESI trials.

2026-11-03 Initialize/FDI slice: four IStream-ABI-compatible FDI adapters now
raw-equal native COFF bodies,140 bytes; prior lifecycle177/metadata28 retained
(317 complete-body object code bytes total, separate from linked window21).
Initialize source builds158 versus original160: all21 reference fields mapped,
two disjoint spans123 bytes verified; whole function remains unmatched due to
native ECX versus original ESI registry owner/result/save scheduling. Stopped
this native source question without register forcing or another GL matrix.
See api.dll/api-initialize-fdi.md for two retained initial compiler failures, successful
002 jobs/hashes, negative controls and exact scope. Current header DLL window
regression passes21-byte gate. Ghidra four FDI functions named/typed; helper
RegisterCallbackInList now explicitly models ESI owner +4 stack words/RET16,
fixing missing call argument. Pre-edit GZF/readback saved; latest API revision
1793721567359:636d9e2e:281, unsaved_changes=false. API current as user selected,
no agent navigation or code-byte changes. Next real callback-list registration
and realloc20135e98 dependencies; no candidate runtime or baseline promotion.

2026-11-03 next API-04 slice completed: lifecycle.cpp ordinary C++ produces
COFF relocation-validated equality for Shutdown11, RunMessageLoop29 and SEH
wrapper137 bytes (normal90 plus EH47), plus28-byte EH4 table.177 unique code
bytes at object evidence level, separate from linked GetWindowHandle21. Three
negative controls reject; no dependency stubs/link/runtime claim. Header-regression
DLL retains21-byte window gate. See api.dll/api-lifecycle-matching.md for hashes/runs.
Ghidra exact11-byte Shutdown created; RunMessageLoop named/typed; VTable+4/+8
function-pointer types filled. Initialize target and table definition now both
uint startupArgument instead of the superseded char* model; this types repaired.
Observed layouts4/16 unchanged. Pre-edit GZF and readback retained; saved API
revision1793720305185:636d9e2e:135, unsaved_changes=false. No tab changes.
Next:160-byte Initialize outer source, inspect helper200f6451 ESI convention;
then dependency expansion/link integration. Heap context permutations remain closed.

2026-11-03 artifact organization: three initial API runs relocated to
vm-share/modules/api/runs; completed jobs copied and hash-verified under
modules/api/jobs while live queue transport remains intact. Canonical outputs
api-pilot.dll/map/pdb; legacy worker artifacts under worker-output. API dispatcher
and comparator use these module-owned paths. Relocation manifest and module READMEs
record provenance; original build/source/binary bytes and EXE paths preserved.
No rebuild or Ghidra action. Saved-state evidence remains the preceding API save,
not freshly rechecked in this filesystem-only task. See vm-share/README.md.

2026-11-03 API progress: see api.dll/api-first-pilot.md and updated boundary/roadmap.
First complete DLL body match: GetWindowHandle21 bytes, one validated global
mapping, both initial zeros and two rejected negative controls. Native comparison
DLL run api-window-20261103-001, SHACE0C3E803BFCBA376549FA190134CE9748C1766F409CED4941F4C0760614414B.
Separate /DLL /NOENTRY pilot uses existing worker without changing EXE source or
worker; no runtime execution/full DLL startup/export144 claim. API-01 normal and
handled-EH return resolved; shutdown frees/resets application and decrements heap
counter. Ghidra wrapper renamed/typed int stdcall with explicit callback definition;
commandLine renamed startupArgument, legacy char* left flagged for coordinated
VTable review. Backup api-before-boundary-20261103-1601.gzf; saved revision
1793718640047:636d9e2e:31, unsaved_changes=false, no tab switch.
API-03 native initializer146 versus144 and allocator174/174 remain unmatched.
Source-identical GL/LTCG control produces same124 symbolic instructions; stopped
that context hypothesis. Window match retained in both controls. Next API-04
wrapper/outer methods and honest VTable/GetApplication integration; remaining
roadmap packages still open. Worker ready, no additional installation input needed.

2026-11-03 current user direction: proceed systematically with API.DLL matching.
See [api.dll/api-matching-roadmap.md](api.dll/api-matching-roadmap.md) for API-00..08, gates and
trial limits. API-00 initial PE/boundary inventory complete:288 exports (one named,
GetApplication ordinal144),828 IAT entries; hash9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
API-01 in progress: getter/four targets verified; wrapper2013e3c1 forwards EAX
on normal path despite void decompilation; exception return and teardown at
2012103f/2012105a remain open. Next resolve these and prepare a separate DLL
pilot profile, first GetWindowHandle then GetApplication. No DLL body match yet.
Live api.dll revision1793405534817:636d9e2e:3 saved, no edits/save/backup/navigation.
Both programs open; MCP reports fsx.exe current, API explicitly targeted.
Worker ready; no extra installation files needed yet. EXE near-match is parked;
layout/import polishing remains deferred. This supersedes the EXE next-code
direction in chronological entries below, while retaining their findings.

2026-11-03 current continuation: baseline41 range consolidation is complete;
see [fsx.exe/coverage-baseline41.md](fsx.exe/coverage-baseline41.md) and its JSON ledger.
Unique code union:3962 verified bytes plus1206 known-different original bytes
(InitInstance/ShowErrorMessage); no unchecked snapshot instructions. This is
not a whole-EXE percentage. One subsequent InitInstance trial succeeded:
explicit HINSTANCE local restores original load ordering at01001c5d..01001c61,
candidate01001c45..01001c49.43->41 differing nonbranch positions, still777 versus
769 bytes. Canonical startup_context_pilot.cpp updated after regression and
whole-section delta checks; candidate retained in run initinstance-module-local-
20261103 (SHA71AC546145A95B8E79BF5E972729DFEFDB0D0928089DD5050BC0FCD4EA3EB5E8).
Archive41 remains the immutable ledger/default inventory baseline, not the new
source snapshot; no archive/replay promotion. Stop this successful one-trial
hypothesis; do not continue local-variable permutations. Next code question may
address the remaining InitInstance ACTCTXA scheduling/register differences with
fresh evidence and a maximum of3 trials. Layout/IAT polishing stays deferred.
Read-only fsx.exe revision1793671788023:335a223f:3, unsaved_changes=false;
fsx.exe current and api.dll open. No database edits/navigation/save/backup,
no new variable rename suggestions or candidate execution; worker ready.
This entry supersedes the pending-consolidation instruction immediately below.

Current continuation policy (2026-11-03, user-requested handoff): read
[handoffs/HANDOFF-20261103-coverage-and-code.md](handoffs/HANDOFF-20261103-coverage-and-code.md).
Selected baseline41 remains. Next consolidate same-hash range coverage, then
choose the largest useful unchecked code range or evidence-led InitInstance work.
Defer16-byte data,32-byte placement gap and103 IAT positions pending new evidence.
This supersedes older chronological "next layout/toolchain trial" suggestions.
Bound each hypothesis to at most3 targeted trials; no regression-based adoption.

2026-11-01 recompilation planning: see [shared/manual-recompilation-workflow.md](shared/manual-recompilation-workflow.md). Live on-disk x86 PE/linker 8.0 and VC80 strings support VS2005-family experiments, not an exact compiler revision. fsx.exe/api.dll SHA-256 identities recorded; Ghidra MCP hash semantics remain unresolved, so verify input identity before address-based comparison. Only fsx.exe currently open, superseding older open-program inventories. Pilot proposal: helpers 0x01001760/0x01001800 and linked build/PDB; reccmp VC8 compatibility needs testing. No compiler/VM installed or Ghidra edits/navigation/save. Last documented unsaved state was not freshly checked.

Updated: 2026-10-31 (Europe/Berlin). Canonical project: G:/Projects/Reversing/fsx-decomp. This knowledge base lives in notes/re; AGENTS.md is at the project root.

## Reading guide

| Topic | Document |
|---|---|
| EXE startup, FSXApp, errors, security cookie | [fsx.exe/startup.md](fsx.exe/startup.md) |
| Application, API.DLL, VTable, heap management | [shared/fsx-api-interface.md](shared/fsx-api-interface.md) |
| COM/OLE and message filter | [fsx.exe/mfc-message-filter.md](fsx.exe/mfc-message-filter.md) |
| Sources and evidence limitations | [SOURCES.md](SOURCES.md) |
| Historical and new changes | [CHANGELOG.md](CHANGELOG.md) |

Existing notes/progress_001.md and notes/chat-files contain earlier material and are retained unchanged.

## Verified state

- ghidra-bethington has fsx.exe open: x86 little endian, 32-bit Windows, image base 0x01000000, 188 functions.
- Recorded executable path: /G:/Projects/Reversing/fsx-decomp/original/fsx.exe.
- FSXApp::FSX_InitInstance at 0x010019e0 displays typed Application calls and correct returns.
- /FSXApp has a reconstructed minimum layout of 0xa8 bytes; /Application is 4 bytes; /ApplicationVTable is 0x10 bytes.
- Cookie checker: void __fastcall __security_check_cookie(uint cookie), modeled by an injection in its caller.
- Current cookie names: local securityCookie, global __security_cookie. Earlier renaming suggestions are obsolete.
- Ghidra reported unsaved_changes: true, revision 1793406139459:5073cd9e:12. Consolidation neither saved nor edited Ghidra.
- api.dll was not open. Its findings were retained from chats, not reverified live.

## Current investigation

S2 is walking through FSX_InitInstance from its beginning. Security cookie and AfxEnableControlContainer have been explained. Do not restart by searching for the entry point. The next code section is loading language.dll and checking Windows versions, if the user wishes to continue.

## Open questions

1. When the user next opens api.dll, verify names, signatures, Application types, and possible .conflict types.
2. Investigate Application VTable slots +0x04/+0x08; historical targets are in fsx-api-interface.md.
3. Determine a proper error-callback function-pointer type instead of provisional void *; inspect the stored type first.
4. Verify version-specific MFC80 layouts, especially AFX_MODULE_STATE +0x08.
5. Investigate filter fields +0x28/+0x2c and the heap counter.
6. Stored signatures of InitializeGlobalFSXApp and InitializeOleAndMessageFilter still report undefined although the decompiler infers returns. Correct only after checking evidence.
7. FSX_InitInstance's old comment broadly warns about incomplete calling-convention/return recovery. Specific documented errors are now fixed; refine that comment during a relevant edit.
8. Add binary hashes/version identity. Do not transfer historical addresses to other builds without verification.

## MCP

Bethington Ghidra MCP 7.0.0 connected successfully; data-type tools are available. Dynamically loaded tools initially failed to appear in the client, then became visible after reconfiguration. It is not proven that the app ignores tools/list_changed.


## S2 update: version helpers (2026-10-31)

Live verified 0x01001760 and 0x01001800: both preserve and return VerifyVersionInfoA BOOL in EAX; current undefined return types cause misleading void decompilation. See fsx.exe/startup.md for evidence and unapplied type/name suggestions. Inspect imported VerSetConditionMask signature next. No Ghidra edits; current revision 1793460953482:5073cd9e:13 is saved (supersedes prior unsaved-state report).

Follow-up: imported VerSetConditionMask at fsx.exe EXTERNAL:00000068 is live verified as `undefined VerSetConditionMask(void)`. The missing prototype explains unreliable argument recovery; correcting it is the next suggested step, not yet applied. UI remains at 0x01001760; Ghidra is unchanged and saved at the revision above.
`nLatest live check: fsx.exe helper 0x01001760 has versionInfo/securityCookie names already present. Verified memset clears 0x9c bytes of stack-resident versionInfo; see startup.md. Ghidra revision 1793460953482:5073cd9e:43 reports unsaved_changes=true, superseding the earlier saved-state report. No Ghidra edits made by this check.

Latest verification: fsx.exe 0x01001760 now has user-applied ULONGLONG conditionMaskMajor/conditionMaskMajorMinor; names and types fit the mask stages. Imported VerSetConditionMask prototype and helper BOOL return remain to be corrected. Revision 1793460953482:5073cd9e:100 is unsaved; no Ghidra edits by this check.

Latest completed edit: VerSetConditionMask Condition is now BYTE (live readback); other signature components preserved. ULONGLONG definition still needs correction/verification: helper shows floating 0.0 and extraout registers. Revision 1793460953482:5073cd9e:188 remains unsaved.

Latest review: VerSetConditionMask parameters now ulonglong/dword/byte, return still ULONGLONG; helper mask locals still ULONGLONG. First argument recovered as integer zero, but return data flow remains broken. Typedef repair remains open. Revision 1793460953482:5073cd9e:310 unsaved; read-only review.

Latest: first local VerSetConditionMask override verified effective at helper 0x01001760; remaining two calls still need return recovery. Root cause of imported signature propagation remains open. Revision 1793466450390:5073cd9e:357 unsaved.

Latest rundown: fsx.exe 0x01001760 is cleanly decompiled, BOOL return and three ulonglong mask stages; 0x01001800 still needs equivalent cleanup. Imported mask signatures now use ulonglong. Typedef repair and root cause of indirect-call signature propagation remain open. fsx.exe active; api.dll also open. Revision 1793467972012:5073cd9e:418 saved. No Ghidra edits in review.

Latest edit: English learning plate comment added and readback verified at fsx.exe 0x01001760. Ghidra has an unsaved comment change; no save performed. Result-variable spelling correction observed. Second helper/type-definition questions remain open.

Latest: second helper 0x01001800 cleanup live verified complete; BOOL return and all mask data flow recovered cleanly. Revision 1793468800776:5073cd9e:490 saved. Supersedes pending-second-helper reports. Typedef repair/indirect prototype propagation questions remain open.

Latest investigation: fsx.exe FSX_InitInstance 0x010019e0 D3DX loading/check flow live verified; see startup.md. LoadLibraryA, GetProcAddress, then explicit D3DXCheckVersion(32,34); failure selects 0x10, success releases the module and continues to path/manifest handling. Naming/type suggestions remain unapplied. Both programs open, fsx.exe active. Revision 1793468935670:5073cd9e:498 saved; no Ghidra edits/navigation.

Latest naming review: FSX_InitInstance 0x010019e0 user changes verified (path arrays char[260], exePathBuffer separated from errorDetailText, named D3DX pointer/result). Remaining naming suggestions in fsx.exe/startup.md; dwExePathSize still combines capacity and length. No Ghidra edits; revision 1793481262086:5073cd9e:575 saved.

Latest assembly review: drive buffer in FSX_InitInstance resolves to baseline ESP+0x10; apparent +0x2c accounts for seven pending PUSH arguments. _splitpath_s receives capacity 3; displayed char[4] does not prove original array size. See startup.md. Revision 1793481455350:5073cd9e:608 unsaved; no Ghidra edits by review.

Latest readback: path renames verified in FSX_InitInstance (exeDrive, exeDirectoryPart, exePath, exeDirectory); capacity/result split now complete (dwExePathBufferSize/dwExePathSize). Optional result-name precision remains; see startup.md. No Ghidra edits; revision 1793483097771:5073cd9e:637 saved.

Latest: manifest memset stack offsets verified in fsx.exe: buffer B+0x350, memset destination B+0x351 with baseline B defined in startup.md. No Ghidra edits; revision 1793483217796:5073cd9e:652 saved.

Latest: manifest _makepath_s destination verified as EAX=B+0x350 from LEA at 0x01001b50; current decompiler correct. Reported stale Listing name likely register markup (rendered UI not inspected). No Ghidra edits; revision 1793484006691:5073cd9e:665 saved.

Latest: ACTCTXA AStack_550 usage reviewed in FSX_InitInstance; suggested name actCtx, not applied. Descriptor points to manifestPath, size 0x20; created handle immediately released. Revision 1793484006691:5073cd9e:675 saved; no Ghidra edits.

Latest full-function review: FSX_InitInstance decompilation and complete 246-instruction listing checked. Names and recent variable splits consistent. Main remaining cleanup is fragmented API.DLL path buffer; generic function pointers and stored undefined helper/error-handler prototypes remain open. See fsx.exe/startup.md full review for suggestions and evidence. No Ghidra edits/navigation; revision 1793485977652:5073cd9e:707 saved. API.DLL target signatures not reverified.

Latest 2026-11-01: user-applied D3DXCheckVersionFunc* and apiDllPath[260] verified in InitInstance. Function-definition internals not exposed by inspected MCP endpoints; full signature verification remains limited. No Ghidra edits; revision 1793486487350:5073cd9e:729 saved.

Latest 2026-11-01: api.dll active at GetApplication 0x20120442; pointer/NULL return and lazy four-byte object allocation live verified. Unknown stored calling convention remains; zero-argument RET does not resolve cdecl/stdcall. See fsx-api-interface.md. No edits/navigation; api.dll revision 1793405534817:4a42ead2:3 saved.

Latest 2026-11-01: fsx.exe GetApplicationFunc* application verified via screenshot and MCP; both Application* return casts removed. Convention remains chosen __cdecl model, not original-convention proof. No Ghidra edits; revision 1793490968639:5073cd9e:747 saved.

Latest 2026-11-01: ShowErrorMessage 0x01001d30 and both string helpers live reviewed via MCP, including assembly. Identified MSGBOXPARAMSA, resource/direct/fallback text paths, synchronous wait for message-box thread, cleared user-icon flag and overwritten owner. See fsx.exe/startup.md; all type/name changes remain suggestions. API.DLL callback callers/resource contents remain open. No Ghidra edits/navigation; fsx.exe active and saved, revision 1793490968639:5073cd9e:747.

Latest 2026-11-01 readback: user split hErrorRessourceModule in InitInstance correctly; English spelling hErrorResourceModule suggested, not applied. ShowErrorMessage prototype still pending. No agent Ghidra edits/navigation; revision 1793492540064:5073cd9e:757 saved.

Latest provenance hypothesis: FUN_01001590 strongly matches optimized StrSafe StringCchPrintfA-style code; exact source/version and Cch/Cb identity unproven. Recorded in startup.md. No Ghidra edits.

Latest completed edit 2026-11-01: fsx.exe 0x01001590 renamed StringCchPrintfA_optimized; English provisional-identification/ABI comment added and verified. Prototype remains incomplete. Revision 1793493280309:5073cd9e:828 unsaved; database not saved by agent. No program switch.

Latest 2026-11-01: formatter custom parameter storage/return/varargs verified correct in callee, but ShowErrorMessage calls still display obsolete stack-only arguments; propagation/overrides need inspection. Argument 6 misnamed hMessageBoxThread; hStringResourceModule suggested. No agent Ghidra edits; revision 1793493918376:e38ea5f1:31 unsaved.

Latest inspection limitation: MCP script execution disabled; dedicated override reader not available. Presence of local call overrides remains unverified at 0x01001dd3/0x01001e03. No Ghidra changes; use UI inspection or enable server scripts to proceed with exact stored-override inspection.

Latest 2026-11-01: script capability works. Read-only inspection confirms correct formatter custom storage and varargs; no matching local override symbols found for ShowErrorMessage calls. Previous script-disabled state superseded. Caller handling remains unresolved; no Ghidra database edits.

Latest completed repair 2026-11-01: formatter 0x01001590 now uses program-local __fsx_edi_eax_varargs; both ShowErrorMessage calls correctly show destination, 1024 capacity, format and varargs. __cdecl+custom-storage recommendation superseded for this function. Root cause confirmed in installed decompiler source; see fsx.exe/startup.md and scripts/ghidra/fsx-edi-eax-varargs.xml. Existing ShowErrorMessage warnings / copy-helper ABI remain separate open cleanup. Revision 1793536182953:c57cedf3:57 unsaved; no save/backup performed, no program navigation.

Latest 2026-11-01: ShowErrorMessage messageText char[1024] applied at Stack[-0x410], replacing local_410 char[1028]. Separate resourceText[512] preserved. Revision 1793546575673:c57cedf3:77 unsaved; no database save.

2026-11-01 build setup: XP guest smoke build/execution verified from shared report: cl 14.00.50727.42 and link 8.00.50727.42. Supersedes earlier no-toolchain-installed report. MFC presence unresolved due to corrected probe path. Version-helper reconstruction and manual pilot build staged, not yet compiled/compared. Ghidra MCP currently refuses connection; database save state and input identity remain open. See manual-recompilation-workflow.md.

2026-11-01: Ghidra MCP reachable again; fsx.exe is the only open/current program. Pilot 0x01001760 read_memory returned 144 bytes; exact equality against original/fsx.exe PE section bytes verified. This establishes this pilot range only, not full input identity or MCP hash semantics. No pilot-output files present yet. No Ghidra edits/navigation/save; save state not rechecked.

2026-11-01 pilot experiment 1: guest build successful, MFC headers verified. Both rebuilt helpers are 144 bytes but ESP-based, while originals use EBP frames and MOV EDI,EDI entries. See pilot-comparison.md. Baseline archived; experiment 2 adds /Oy- /hotpatch, pending guest execution. Ghidra live revision 1793546795596:8eb5e96a:3 is saved, superseding older unsaved reports. No Ghidra edits/navigation/save.

2026-11-01 pilot experiment 2 verified: /Oy- /hotpatch recovered entry/frame instruction forms; both 144-byte helpers differ at only seven non-address byte positions, consistently representing eight-byte frame/structure placement difference. Exact match not achieved. See pilot-comparison.md. Archived results; experiment 3 /O1 instead of /O2 staged for guest execution. No Ghidra edits/save; last verified saved state unchanged by agent.

2026-11-01 pilot experiment 3: O1 build verified; helper lengths 139/137 bytes, changed registers/scheduling and LEAVE, same 0xa0 frame. Experiment 2 remains closest. O1 artifacts archived; experiment 4 O2+GL/LTCG staged, pending guest run. See pilot-comparison.md. No Ghidra edits/save.

2026-11-01 milestone: experiment 4 /O2 /Oy- /hotpatch /GS /MD /Zi /GL + /LTCG yields exact address-normalized matches for both 144-byte version helpers. Five reference fields per function independently mapped to known original targets and pilot MAP symbols; zero remaining byte differences. Raw difference 16 bytes/function, so not literal byte equality or whole-file match. See fsx.exe/pilot-comparison.md and scripts/compare-version-pilot.ps1. Matching artifacts/source/commands archived in vm-share/pilot-ltcg-match/. No further guest build required for this milestone. No Ghidra changes/save; last verified saved state remains prior readback.

2026-11-01 next pilot staged: SDK StringCchCopyA candidate for FUN_01001600 (72 bytes, ECX destination/EAX source, fixed 1024 capacity). Live assembly verified; current incomplete Ghidra prototype preserved. Source and batch prepared, guest build required. Version-helper matching baseline preserved in pilot-ltcg-match; regression comparison pending. Current fsx.exe revision saved, no Ghidra edits/save by agent. See startup.md.

2026-11-01 experiment 5: version helpers retain exact normalized matches. SDK emits near-original StringCopyWorkerA but specializes global destination; no copy match yet. Archived in pilot-copy-05; experiment 6 stack-local destination staged, guest build pending. See startup.md. No Ghidra changes/save.

2026-11-01 milestone experiment 6: third matching body verified. Original copy helper 0x01001600 equals SDK StringCopyWorkerA at pilot 0x00401180 across 72 RAW bytes, no normalization. EAX source/ECX destination generated by GL/LTCG with stack-local caller buffer. Both version helpers remain normalized exact. Archived pilot-copy-06-match; reproducible scripts/compare-copy-pilot.ps1 added. Strong SDK worker identification; exact original header version and Cch/Cb entry still unproven. Rename suggestion documented, not applied. No Ghidra edits/save; no new database-state query. Next candidate: SDK formatter 0x01001590 with caller context.

2026-11-01 shared-folder organization and formatter preparation: active launcher vm-share/BUILD.bat; input/07-formatter; outputs results/07-formatter (pending guest run); prior outputs results/06-three-matches; matching archives archive/04-version-matches and archive/06-three-matches. Older historical paths in notes refer to pre-organization names. Three obsolete input directories remain VMware-locked despite user closing guest Explorer; finishing their archival requires released guest handles/shutdown. Added README.txt. Formatter source experiment 7 staged; no Ghidra changes. Current live revision 1793554472087:8eb5e96a:5 saved.

2026-11-01 experiment 7: three prior matches retained; formatter ABI recovered automatically but SDK selects _vsnprintf_s whereas original uses _vsnprintf. Header conditional verified locally. Experiment 8 macro _STRSAFE_USE_SECURE_CRT=0 staged; BUILD.bat outputs results/08-formatter-legacy-crt. No Ghidra edits/save. See startup.md.

2026-11-01 milestone experiment 8: FOUR matching function bodies verified, total 452 bytes. Formatter 0x01001590 matches SDK legacy-CRT StringCchPrintfA after one verified _vsnprintf IAT mapping; two version helpers remain normalized exact, copy helper raw exact. Archive vm-share/modules/fsx/archive/08-four-matches includes source, commands, header and three reports. No build needed now. Provenance/configuration supports matching implementation, not exact original source/header identity. No Ghidra edits/save or fresh saved-state check. Next: ShowErrorMessage reconstruction and ABI review.

2026-11-01 experiment 9 staged: ShowErrorMessage reconstruction with SDK StringCchPrintfA/StringCchCopyA, actual MFC headers and minimal Application interface boundary. Full original function range verified 437 bytes 0x01001d30..0x01001ee4, six stack args, DWORD result. Current stored undefined return and fragmented MSGBOXPARAMSA remain Ghidra cleanup suggestions; no edits applied. BUILD.bat needs guest run, outputs results/09-show-error. Four prior matching functions will be regression-checked. Current fsx.exe revision 1793554472087:8eb5e96a:5 saved. See startup.md.

2026-11-01 experiment 9 failed linking due to C++ definition versus C-style pilot export name. No binary comparison performed. Experiment 10 extern C correction staged; BUILD.bat outputs results/10-show-error-c-linkage. Existing four-match baseline preserved. No Ghidra changes/save.

2026-11-01 experiment 10: ShowErrorMessage builds (444 bytes vs original437), four prior matches retained. Main differences shared vs duplicated HRESULT check and final owner/style store order. Archived first successful build; experiment 11 common check staged, BUILD.bat outputs results/11-show-error-common-check. No Ghidra changes/save; see startup.md.

2026-11-01 experiment 11: ShowErrorMessage now 437 bytes, four previous matches retained. Remaining reviewed differences sign-test encoding and final two field-store order. Experiment 12 field-order-only adjustment staged, outputs results/12-show-error-store-order. No exact ShowErrorMessage match yet; no Ghidra edits/save.

2026-11-01 experiment 12: final field-store ordering recovered, four prior helpers remain matching. ShowErrorMessage still has CMP-vs-TEST difference; not yet certified exact. Experiment 13 explicit HRESULT high-bit test staged, outputs results/13-show-error-sign-bit. No Ghidra edits/save.

2026-11-01 experiment 13: certified ONE byte remaining in full mapped ShowErrorMessage comparison (JS vs original JL at offset0xde), 16 address targets verified. Four previous matches retained. Added compare-show-error.ps1. Experiment 14 signed int result staged, results/14-show-error-signed-int. No Ghidra changes/save.

2026-11-01 experiment14: ShowErrorMessage two-byte CMP-vs-TEST difference after 16 checked mappings, four helper matches retained. Archived reports; experiment15 successful-format block/shared return staged, outputs results/15-show-error-success-block. No Ghidra edits/save; no exact fifth match yet.

2026-11-01 experiment15 saved as near-match: ShowErrorMessage 437 bytes, two non-reference byte differences at0xdb/0xdc (CMP EAX,EBX vs TEST EAX,EAX; EBX zero); no exact fifth match. Four prior matches retained. Full source/reports archived archive/15-show-error-near-match; no guest run pending. Remaining investigation needs compiler/TU/context evidence; Ghidra signature/MSGBOXPARAMSA cleanup still unapplied, API.DLL callback argument2 semantics unresolved. No Ghidra changes/save.

2026-11-01 completed: transferred verified reconstruction types/names/evidence into fsx.exe Ghidra and saved database. ShowErrorMessage DWORD stdcall six typed params; 40-byte MSGBOXPARAMSA at -0x63c and separate DWORD dialogResult at -0x640; meaningful locals. Copy helper now explicit ECX destination/EAX source and EAX HRESULT, caller arguments recovered. Formatter custom varargs convention preserved, locals improved. Five plate comments document four matches vs near-match. Fresh readback checked. Remaining maximum-restarts warning, argument2 callback semantics/API.DLL validation and two-byte ShowErrorMessage comparison difference remain open. No independent backup created; no program navigation or binary patches. See fsx.exe/startup.md transfer section.

2026-11-01 experiment16 no-source-goto variant staged, BUILD.bat outputs results/16-show-error-no-goto. Direct-copy status remains ignored, local success value permits common display condition. Pending guest run and all-five comparisons. Prior Ghidra transfer remains saved; no new Ghidra changes.

2026-11-01 experiment16 no-goto gives same two-byte near-match, four helpers retained. Archived. Original callback address pass reverified at fsx.exe0x01001c65; database saved. Experiment17 removes artificial export and introduces non-GL opaque callback retention boundary, outputs results/17-show-error-callback-escape. This is limited caller-context experiment, not full API.DLL reconstruction. Pending guest run; no Ghidra edits.

2026-11-01 experiment17 verified: removing artificial export and retaining callback through opaque non-GL boundary leaves same two-byte ShowErrorMessage difference; four helpers retain matches. Archive17-callback-escape-near-match includes all reports/source/build. No guest run pending. No-goto and limited callback-context hypotheses did not resolve it; real InitInstance/TU context and compiler revision remain open. No Ghidra changes/save; prior saved transfer retained.

2026-11-01 SP1 comparison18 staged: user reports installed VM running; exact experiment17 source/flags reused and hash-checked, fresh guest work path. BUILD.bat outputs results/18-sp1-same-source; compiler revision pending banner verification. Host VMware log confirms ulm.disableMitigations TRUE loaded, ULM mode remains; performance improvement unmeasured. No Ghidra changes/save.

2026-11-01 experiment18 complete: SP1 compiler14.00.50727.762 verified. Same source/flags retains four helper matches and same two-byte ShowErrorMessage difference. Archived18-sp1-four-matches-near-show-error; RTM17 baseline preserved. SP1 did not fix mismatch; original compiler revision still unproven. No guest run pending, no Ghidra changes/save.

2026-11-01 experiment19 staged: reconstructed CWinApp InitInstance flow and ShowErrorMessage now shared TU, actual virtual Initialize callback escape/direct error calls, replacing synthetic retention. SP1 flags retained; prior baselines safe. Explicit non-GL OLE/filter failure stub and minimal constructor make this comparison-only, not fully reconstructed startup. Guest BUILD.bat required, outputs results/19-initinstance-shared-tu; inspect helper inlining/boundaries before comparing. Current live Ghidra saved, no edits.

2026-11-01 experiment19 compile failed from missing afxdisp.h for AfxEnableControlContainer. Archived; experiment20 header-only correction staged in BUILD.bat, outputs results/20-initinstance-mfc-header. No new comparisons yet; prior SP1/RTM baselines retained. No Ghidra changes/save.

2026-11-01 experiment20 complete: shared TU InitInstance caller built successfully. Four helpers retained matches; ShowErrorMessage same two-byte difference after16 mappings. InitInstance pilot758 bytes/frame0x564 vs original769/frame0x554, NOT matching and still explicit OLE/event scaffolding. Archive20-startup-context-near-match holds full results/source. No guest run pending, no Ghidra changes/save. Remaining context and original TU/options questions documented in startup.md.

2026-11-01 experiment21 staged: replaced event constructor/destructor placeholders using reverified original95/70-byte functions; OLE/filter stub remains until matching MFC header evidence captured. BUILD.bat additionally copies four installed MFC header/inl files into results/21-startup-event-and-mfc-evidence. Guest run needed; no Ghidra edits, live saved state verified.

2026-11-01 experiment21 verified four helper matches and unchanged two-byte ShowErrorMessage; archived21-event-and-mfc-evidence with actual SP1 MFC headers. Experiment22 replaces OLE failure stub with ordinary COM/MFC filter reconstruction; compile asserts flag offsets/size and uses /EHsc plus ole32.lib. Derived virtual overrides still need verification. BUILD.bat results/22-ole-filter-reconstruction pending guest run; no Ghidra changes/save. See mfc-message-filter.md.

2026-11-01 experiment22 complete: OLE placeholder removed, filter size0x44 and flags0x28/0x2c compile-verified. OLE helper237 bytes matches instruction skeleton outside12 address fields, but EH metadata/full derived VTable NOT validated: no certified fifth match. Four prior helpers retained, ShowErrorMessage same two-byte near-match. Archive22-ole-layout-verified; no guest run pending. Next: validate filter virtual targets/EH metadata, then InitInstance differences. Ghidra current saved, no edits.

2026-11-01 validation found real missing filter override: original vtable+0x60 ->0x010018d0 returns TRUE, corresponding OnMessagePending. Candidate22 inherited MFC implementation was incomplete; added override for experiment23. EH FuncInfo/unwind/delete structure agrees at reviewed scope; exact full mapping still open. BUILD.bat results/23-filter-pending-override needs guest run. No Ghidra edits/save; future function creation/name suggestion documented only.

2026-11-01 experiment23 milestone: OLE initializer237 bytes normalized exact, filter deleting destructor35 normalized exact, OnMessagePending8 raw exact. All28 VTable slots and EH FuncInfo/unwind/cleanup checked with actual import identities by new scripts/compare-ole-filter.ps1. Prior candidate22 missing override superseded. Four earlier helpers still matching; ShowErrorMessage same two-byte near-match. Archive23-ole-filter-verified-matches saved. RTTI/external DLL body identity excluded. No new build needed, no Ghidra edits/save this turn. Next: InitInstance source/layout comparison and optionally transfer new filter findings to Ghidra.

2026-11-01 matching system implemented: see shared/matching-system.md and scripts/matching/README.md. Shared root cleaned after user shut XP down: RUN-WORKER.bat, worker.vbs, README, queue/runs/setup/archive only. Previous input/results moved under archive; baseline23 preserved. Host bounded search show-error-search-001 queued24 finite source proposals, waiting for user to launch XP worker. All-five comparison/regression and actual-source hash gates implemented. Replay/timeout/build-failure tests passed; live guest end-to-end not yet verified, no new match claimed. Ghidra unchanged; last verified database state remains saved.

2026-11-01 live matching search show-error-search-001 completed/exhausted: 24 real XP/SP1 builds, 12 strict target comparisons at437 bytes all retain2 differences, 6 variants438 bytes and6 variants439 bytes rejected by fixed-offset target mapper. No exact target match; source/receipt/compiler validations completed. Representative rejected002/004 retain all helper/OLE regressions when checked separately. Outputs retained under vm-share/modules/fsx/runs/show-error-search-001; REVIEW.md/review.csv explain mapper limitation and function lengths. No guest run pending. Continuous worker protocol now validated end-to-end; direct vmrun guest launch intentionally deferred at user request. No Ghidra/source changes.

2026-11-01 actual cleanup completed: 1,482 files / 188.02 MiB deleted, including completed queue copies, hash-verified legacy duplicates, disposable replay fixtures and non-representative generated outputs. Full search candidates001/002/004 and named manual baselines retained; all candidate sources/logs/reports/receipts preserved. Offline recheck of candidate001 retained all established helper/OLE matches and ShowErrorMessage437-byte/two-byte near-match; canonical src equals candidate001 source. See artifacts/fsx.exe/verification-after-cleanup for the retained comparison results. Queue empty; no new guest build dispatched. Next: InitInstance0x010019e0 current boundaries and instruction-aligned diagnostics before source search. Existing script/XP worker retained; reccmp not integrated. No Ghidra inspection/navigation/edit/save; current database save state not rechecked.

2026-11-01 remote-build setup interrupted at user request: VMware Tools running and VM listed, but authenticated guest execution rejected Robin's empty password. No guest process was launched. Proposed Startup-folder workaround was NOT installed; installer files and controller autostart integration removed. Waiting smoke controller stopped before staging: no test run, no queued jobs, no heartbeat, no guest build. Standalone start-guest-worker.ps1 direct-vmrun helper remains unvalidated for password-bearing credentials; worker heartbeat and receipt retention added. No desktop UI automation or guest authentication-policy changes performed. Existing manual worker workflow remains available. No Ghidra changes/save.

2026-11-02 host-dispatched worker smoke test passed: user started RUN-WORKER.bat, live heartbeat changed building -> ready, worker-smoke-20261102-001 completed one real XP/SP1 compile/link (receipt ExitCode0, cl14.00.50727.762). Source hash/receipt/output validations and all established helper/OLE comparisons passed, including destructor, pending override,28 vtable slots and EH metadata. ShowErrorMessage0x01001d30 remains437 bytes with only offsets0xdb/0xdc different after16 validated address fields. Run ends exhausted because its deliberate one-candidate test limit was reached, not build failure. Output SHA256532CBFF7A404ACF3AE6F93BE7EAE863114A4D7C6D2769DF0F299DB21FE13CFF7. Worker remains ready for host jobs; no vmrun credentials/autostart/UI automation involved. No Ghidra/source changes, no fresh database-state inspection. Next: InitInstance0x010019e0 instruction-aligned baseline diagnostics.

2026-11-02 InitInstance investigation: fresh saved fsx.exe readback at0x010019e0 (769 bytes/246 instructions), no Ghidra edits/navigation/save. Added instruction-aligned diagnostic JSON/HTML and reviewed-source build mode. Ten InitInstance attempts, nine actual successful compiles; final canonical proposal/run010 is777 bytes/245 instructions with matching0x554 frame. Corrected initialization phases, path scratch capacities and startup argument: original+0x4c is consistent with SDK m_nCmdShow, superseding prior command-line interpretation; API.DLL target signature and Ghidra layout repair remain open/suggestions only. Existing helper/OLE regressions pass; ShowErrorMessage stilltwo bytes. Fixed controller Target collision/lock disposal and guest failure exit code; actual bad-source regression returns3/build_failed. See fsx.exe/initinstance-matching.md, artifacts/fsx.exe/initinstance/experiments.csv and final run010 HTML. No exact InitInstance match/byte score; next D3DX block placement and verified target mapping. Worker ready, no search active.

2026-11-02 follow-up: user opened api.dll, independently verified vtable0x200f2b24 and Initialize0x2012048a five-word/RET0x14 ABI. Argument2 stored asDWORD2020de6c; inherited commandLine/char* type does not prove pointer semantics. SDK m_nCmdShow caller interpretation remains supported but target semantic type open; rename suggestion only. No Ghidra edits, DLL saved revision1793405534817:bb3b8861:3. Three real builds011/012/013 pass prior regressions; lengths857/777/777,013 listing identical to selected010. Added19-target reference inventory with equal occurrence counts, explicitly not exact-byte or positional validation. Canonical source unchanged, InitInstance still777/769, ShowErrorMessage two bytes, worker ready. See shared/fsx-api-interface.md and fsx.exe/initinstance-matching.md.

2026-11-02 block-comparison milestone: added conservative CFG extraction/anchor/edge pairing and navigable HTML; both selected baseline and new canonical have21 blocks, all21 paired, no conflicts or successor-set mismatch (structural diagnostic only). Isolated ACTCTXA DWORD-vs-WORD zeroing; two real aggregate variants014/015 retain regressions and reach246 instructions, matching original count. Canonical now proposal12 aggregate, validated with fresh integrated build016:777 bytes vs original769, frame0x554; all19 reference counts and helper/OLE/28-vtable/EH checks pass, ShowErrorMessage remains2 bytes. Reports in run016. No exact match, no Ghidra edits/navigation/save or fresh saved-state inspection. Worker ready, no run active. Next: mapped per-block operand/encoding comparison and branch-width/layout differences.

2026-11-02 class/encoding audit: live fsx.exe constructor/thunk/IAT ordinal566 plus vtable+0x58 reverified; rebuilt CWinApp-derived vtable slot independently points to InitInstance. Strong support for current inheritance/override model; original class name/layout not proven. Exact length accounting: all+8 bytes are branches (+12 D3DX conditional, -4 manifest, -5/+5 error-tail jumps), zero non-branch length delta in each paired block. Added actual PE per-block non-branch operand comparison:231 positions,188 normalized equal,43 positional differences,44 validated target/string fields. Not a match percentage/byte count. Canonical/source/compiler unchanged, no new guest build. No Ghidra edits/navigation/save; api.dll stays current. Next targeted register/LTCG context and failure-block placement, not class reassignment. See fsx.exe/initinstance-matching.md and updated run016 reports.

2026-11-02 lifecycle milestone: certified three further normalized-exact complete bodies: fsx.exe0x01001660 constructor/global initializer95 bytes (13 validated fields),0x010016d0 deleting destructor70 (4 fields),0x01001720 ExitInstance58 (5 fields). Added ordinary C++ ExitInstance; live original slot+0x70 and API slot+4 wrapper verified read-only. Fresh SP1 worker run lifecycle-20261102-003 ends matched; prior7 matches retained, total10 counted bodies/955 bytes. Golden archive24-application-lifecycle-matches preserved. New lifecycle comparator/import identity checks and automatic regression gates; negative mutated-byte test rejected. ShowErrorMessage still2 differences, InitInstance777/769 and246 instructions/frame0x554. Ghidra missing ExitInstance function creation and provisional API Shutdown rename are suggestions only, not applied. No Ghidra edits/navigation/save; api.dll stays current, save state not rechecked. See app-lifecycle-matching.md. Worker ready, no run active.

2026-11-02 EXE vtable inventory complete:48 observed application slots1398..1457,40 MFC DLL+ordinal/name matches,3 raw-equal SDK-stub entries,5 own entries. Found two real missing overrides in current reconstruction: GetMessageMap0x01001650 (empty static map) and Run0x01001d00 (API slot+8 with current-message callback, then virtual ExitInstance). Callback0x01001cf0 JMP thunk to MFC80#1074 AfxGetCurrentMessage. Original Serialize/Dump share RET4 target1ef0; rebuild separate => folding hypothesis, not proven /OPT:ICF. Added reusable CSV/JSON/HTML inventory. No new build/source/flags/Ghidra changes, api.dll remains current. Next stay in EXE: GetMessageMap, Run/callback/API slot+8 signature, then isolated ICF test. See exe-vtable-inventory.md.

2026-11-02 dispatch milestone: canonical GetMessageMap via empty MFC message-map macros, Run override and current-message wrapper added. Independent api.dll0x20122817/20122ae1 inspection confirms one callback parameter and MSG* use; source UnknownSlot08 corrected to reconstructed RunMessageLoop(MSG* callback). New bodies normalized-exact: GetMessageMap6, Run35, callback5; empty8-byte map/24-byte terminator and base import verified. Fresh worker app-dispatch-20261102-001 passed all13 explicitly counted bodies/1001 bytes; new mandatory dispatch comparator and negative target test pass. Golden archive25 preserved, inventory updated: no missing application overrides, InitInstance alone not matched in app vtable. InitInstance777/769,246/frame0x554 and ShowErrorMessage2 differences unchanged; all earlier OLE/EH checks retained. No Ghidra edits/navigation/save, api.dll current with saved revision returned by readback. Next isolated ICF hypothesis test; canonical linker stillNOICF. See app-dispatch-matching.md.

2026-11-02 ICF milestone: source hashes identical to baseline25, actual SP1 experiment icf-20261102-001 passes all13 bodies with /OPT:ICF. Across app48/filter28 slots, all2850 address-sharing relations agree with original (NOICF control had4 mismatches). ShowErrorMessage still2 bytes, InitInstance777/769 unchanged. Adopted ICF default after reporting result; fresh icf-default-20261102-001 passes13 gates, alias check0 differences offline. Golden26 preserved; NOICF25 kept. Added mode/command hash metadata, guest-local command logging and reusable alias check/future ICF gate. No Ghidra/source changes or candidate execution, worker ready. Next: actual Windows/MFC startup context instead of console pilot. See linker-folding.md.

2026-11-02 Windows/MFC startup profile implemented and adopted as default. Source hashes unchanged vs26; actual builds mfc-startup-001/mfc-default-001 omit console main and synthetic helper TUs, compile actual application/getter TUs, linkGUI subsystem2. New static startup gate verifies WinMainCRTStartup/MFCS80 WinMain/E9->AfxWinMain/MFC#1207, rejects console control. All13 existing body matches/1001 bytes and vtable sharing/OLE/EH gates pass without artificial helper callers. InitInstance777/769,246/frame0x554 and ShowErrorMessage2 differences unchanged. Fresh default ends diagnostic_complete for InitInstance. Golden27 preserved; ConsolePilot control and baseline26 retained. No canonical C++ edits or binary execution. Read-only Ghidra open/import inventory, no navigation/edit/save; api.dll current. See mfc-startup-profile.md.

2026-11-02 CUT at user's usage-limit request: both requested steps completed. GL-only and SameTU getter tests retain13 matches, open encodings unchanged. Canonical getter now ordinary /GL TU, obsolete retention helper removed; golden28-gl-getter-context preserved (B255FB5ECFBCC848A1D3760EF64589B221949997C51E0689A956B2F0C3DEA6F4). Read-only EXE export/inventory:188 model count=56 internal+132 external;174 unassigned instructions/70 ranges;80 raw import thunks,3 not disassembled;130 inventory rows are mixed regions, NOT function count. Executable9728 bytes:4628 instructions/3150 defined data/1950 undefined,15 nonzero undefined ranges remain open. Resources99.25% of file; original .rdata merged/organization differs (hypothesis, no exact flag proof). Reports: artifacts/fsx.exe/exe-inventory/inventory.html and notes/re/fsx.exe/getter-context-and-exe-inventory.md. No Ghidra changes/save/navigation or candidate execution. No job active; worker ready. Stop here, next static initializer/teardown glue and section/resource layout, avoid blind near-match rewrites.

2026-11-02 static initialization follow-up: see static-initialization-matching.md.
Original XCU table proves CWnd construction before application construction;
canonical declarations now produce that order. Five complete wrapper bodies/113
bytes match with18 validated references, all13 earlier body gates retained
(total18/1114 at their documented scopes). Golden29-static-initialization-matches;
new mandatory MfcWindows gate and StaticInitialization target. Full C initializer
table still differs: rebuilt XIY registers ___CxxSetUnhandledExceptionFilter,
absent from original checked table. InitInstance/ShowErrorMessage remain open.
Ghidra: five bounded functions and CRT table comments applied, pre-edit GZF backup
exported, fsx.exe saved (1793623738938:8bd7bef0:69, unsaved_changes=false).
Current inventory195 model functions=63 internal+132 external,145 unassigned
instructions/63 ranges,130 mixed rows,15 nonzero undefined ranges. Two additional
stored import thunks observed after changes; not new matched source bodies.
No API edits/tab navigation/candidate execution. Worker ready, no active run.
Next: investigate extra XIY CRT initializer from library/exception context before
blind flag changes; then remaining runtime ranges or API behavior.

2026-11-02 CRT context resolved: see crt-library-context.md. Installed SP1
msvcrt.lib crtexew.obj owns .CRT$XIY's DIR32 reference to
___CxxSetUnhandledExceptionFilter; unhandld.obj supplies the routine. Extracted
release x86 RTM msvcrt.lib from the existing VS2005 DVD CAB without installation
or VM/snapshot changes. SP1 compiler/linker and installed MFC/headers retained.
Explicit MFC libraries must precede the RTM CRT; first incorrect-order experiment
was rejected by allocation-import gates and remains baseline_failed. Corrected
source-identical build passes all18 existing body matches/1114 bytes and both C
and C++ initializer table roles/order. This supersedes the open XIY discrepancy
for the selected default, not full CRT-body/original-compiler identification.
CrtLibrary Auto now selects RTM for MfcWindows, InstalledSP1 for ConsolePilot;
InstalledSP1 explicit control retained. RTM library SHA256 pinned B1456D9A7E17865E3DC35663E0377BE4C7B8BE97A372210CD064F9F5635DFFEA.
Fresh default crt-default-20261102-001 passes; golden30-rtm-crt-context SHA256
D8973AC14E07DA44C01F0FA4AE207023E290361C2B2125460BB57FF902E17A7B.
InitInstance777/769,246/frame0x554/43 positional differences and ShowErrorMessage
2 bytes unchanged. Updated two existing Ghidra table comments and saved fsx.exe,
readback1793624696372:8bd7bef0:72 unsaved_changes=false. No API edits/navigation,
function creation/type changes, or candidate execution. Inventory counts unchanged;
default library/inventory/replay references30. Worker ready; no active run.
Next: inspect actual pre_c_init/runtime bodies against identified counterparts
before defining undecoded functions, or continue API behavior; no blind variants.

2026-11-02 pre_c_init completed: see pre-c-init.md. Live original0x01002547..01002628
(226 bytes/56 instructions) normalized-exact with27 validated fields against
baseline30 _pre_c_init from msvcrt:crtexew.obj. Three raw PE dependencies checked:
RTC walker36, FP precision41, shared setargv/matherr zero stub3. Same body also
matches SP1 control29, so it does not distinguish original compiler revision.
New mandatory MfcWindows PreCInit gate; real worker pre-c-init-20261102-001 passes
all18 existing application/helper/wrapper body gates. Those counts remain18/1114;
CRT library identification is reported separately. No C++ source/flags changed.
Pre-edit GZF exported, bounded disassembly/function int __cdecl pre_c_init(void)
and comment applied; saved readback1793625481881:8bd7bef0:291 unsaved_changes=false.
No API edits/navigation/candidate execution. Follow-on model contains three
one-byte FUN_* dependency placeholders, not complete stored dependency bodies;
full raw PE matches are separate evidence. Current model202=67 internal+135
external;135 mixed inventory rows;4857 code/3150 data/1721 undefined bytes,
14 nonzero undefined ranges. Baseline30 retained and its matching report extended.
Negative PUSH2->3 control rejected; mutated copy deleted. Worker ready/no active job.
Suggestions only: DAT_01004030 -> g_crtManagedImageFlag; puVar1 -> pCrtModeValue.
Next: complete three bounded dependency definitions/alias documentation, then
pre_cpp_init or API behavior. No blind near-match variants.

2026-11-02 pre_cpp_init/helper completion: see fsx.exe/pre-cpp-init-and-exe-gaps.md and
exe-matching-roadmap.md. Original0x01002313 complete75 bytes normalized-exact
with12 validated reference fields; RTC termination0x01002802 complete36 verified.
Mandatory PreCppInit MfcWindows gate added; real pre-cpp-init-20261102-001 passes
all earlier18 body gates plus CRT gates. Negative _amsg_exit8->9 control rejected.
No canonical C++/flags/library changes; baseline30 retained with extended report.
Ghidra pre-edit GZF exported; bounded full definitions/comments/signatures applied
for pre_cpp_init, _RTC_Initialize, _setdefaultprecision, CrtReturnZeroFallback,
_RTC_Terminate. Three prior one-byte helper placeholders superseded. Approved
DAT_01004030 -> g_crtManagedImageFlag applied exactly, existing undefined4 type
preserved. Readback saved1793626307309:8bd7bef0:445, unsaved_changes=false.
No API edits/navigation/candidate execution. New follow-on one-byte placeholders
at known import thunks2be0/2bec remain incomplete; raw identities already verified.
Current model206=71 internal+135 external;110 unassigned instructions/61 ranges,
134 mixed inventory rows;4934 code/3158 data/1636 undefined bytes,12 nonzero
undefined ranges. These are model counts, not matching percentages. Worker ready.
Remaining scope includes full CRT/security/SEH/stack/ATL bodies, data/RTTI/EH,
import/PE/linker layout, and explicit InitInstance/ShowErrorMessage nonmatches.
Next bounded block: full CRT entry/main startup and undecoded gaps near24dd/2529.

2026-11-02 full CRT startup region milestone: native Ghidra access recovered after
user restart. Explicit fsx.exe calls with api.dll user-selected; no tab switching.
Complete region01002363..01002541 (479 bytes/157 instructions) normalized-exact
with39 validated address fields, including54 previously undecoded EH4 bytes.
40-byte scope table/four target pointers, PE entry10, cookie init148, SEH prologue69
and raw epilogue20 plus MFC entry wrapper checked. Region is NOT a single stored
479-byte function:425-byte normal-flow parent ownership deliberately preserved.
New mandatory CrtStartup MfcWindows gate; actual crt-full-startup-20261102-001
passes all earlier18 application/helper/wrapper and CRT gates. SP1 control also
passes this region; no unique compiler identification. Canonical source/baseline30
unchanged; known InitInstance/ShowErrorMessage nonmatches remain open.
Ghidra pre-edit GZF exported;54 listing bytes filled, four embedded-EH labels and
scope refs/comments added; parent typed int __cdecl. Saved readback
1793646807668:5e7b737f:88 unsaved_changes=false. No API edits or candidate execution.
New _XcptFilter import-thunk one-byte placeholder observed, full stored definition
still pending. Inventory207=72 internal+135 external;136 mixed rows;131 unassigned
instructions/63 ranges;4988 code/3158 data/1582 undefined bytes,10 nonzero ranges.
Negative body-literal and EH-pointer controls rejected; temporary copies deleted.
Unneeded denied socket-fallback helper/request/schema scratch removed; native tools
used for actual edits. Worker ready/no job active. See full-crt-startup.md.
Suggestions only: Exchange->startupLockOwner; pvVar2->previousStartupLockOwner;
local_24->commandLineTail. Next: exception-dispatch wrapper and PE image helpers,
then remaining security/stack/ATL and full data/linker layout.

2026-11-02 CRT image/dispatch milestone: see crt-image-helpers.md. PE validation41
and section lookup66 raw-exact; nonwritable full region108 normalized-exact with6
fields, embedded EH23 and scope28/two pointers checked; dispatch35 with3 fields
and cookie checker15 with2 fields normalized-exact. Imported MSVCR80 dispatcher
and failure-reporter body remain outside this report. New mandatory CrtImageHelpers
gate; actual crt-image-helpers-20261102-001 retains all prior18/CRT gates. No C++,
flags/library/baseline30 change; known application nonmatches remain open.
Ghidra pre-edit GZF exported; defined dispatch35 with four-pointer minimum ABI,
filled23 EH listing bytes, preserved85-byte parent/prototypes, added two labels/
scope refs and evidence comments. Corrected false uninitialized pImageBase by
analysis-only inline on the verified41-byte leaf: actual EDX is preserved. This
is NOT original compiler/source inlining evidence or any byte/prototype change.
Saved readback1793647354097:5e7b737f:171 unsaved_changes=false. api.dll stays selected,
no tab navigation/API edits/candidate execution. Inventory208=73 internal+135
external;137 mixed rows,5011 code/3158 data/1559 undefined bytes,9 nonzero ranges.
Negative MZ-literal/EH-pointer controls rejected; altered copies deleted. Worker
ready/no active job. Suggestion only p_Var1->sectionHeader (PE Characteristics use).
Next: report_gsfailure and stack probes, then onexit/ATL and data/linker layout.

2026-11-02 GS failure/stack milestone: see crt-failure-stack.md. Reporter0x01002a6d
260 bytes normalized-exact with33 validated references, standard x86 CONTEXT716/
EXCEPTION_RECORD80 initial data and EXCEPTION_POINTERS links checked. Stack probes
22/22 normalized-exact plus43 raw-exact. New mandatory CrtFailureStack gate; actual
crt-failure-stack-20261102-001 passes all prior18/CRT gates. No source/flag/library
or baseline30 change; application nonmatches unchanged. Negative page-step/data-
pointer controls rejected and copies deleted. Ghidra backup exported; applied
existing Windows structures80/716/8, added data-role/reporter/special-ABI comments.
No user-defined symbols overwritten; automatic labels now reflect data types.
Existing function prototypes/injections/code bytes preserved; no guessed C ABI
for EAX/ESP stack helpers. Saved readback1793647925456:5e7b737f:196 unsaved=false.
api.dll still selected; no navigation/API edits/candidate execution. Inventory208
=73 internal+135 external;137 mixed rows;5011 code/3162 data/1555 undefined bytes,
8 nonzero undefined ranges. Worker ready/no active job. Data-name suggestions only
listed in topic file. Next: __onexit registration/locking/EH, then ATL and layout.

2026-11-02 exit-registration milestone: see crt-onexit.md. Complete __onexit150,
lock cleanup9 and atexit adapter18 normalized-exact with14/1/1 validated fields;
finally table28 with same cleanup target and lock8/import/global relationships
checked. New CrtOnexit gate; actual crt-onexit-20261102-001 retains all prior18/CRT
gates. No source/flags/library/baseline30 change; known nonmatches unchanged.
Negative lock-number/finally-pointer controls rejected; modified copies deleted.
Ghidra pre-edit GZF exported; existing __onexit name preserved and standard
/stdlib.h/_onexit_t reused for callback argument/return. FUN26e7 renamed
ReleaseCrtExitLock, FUN26f5 renamed atexit; separate void callback definition
/FSX/CRT/CrtAtexitCallback, comments and finally-table reference applied.
Text prototype parser failed on callbacks; datatype-backed Java script succeeded.
Readback saved1793648448204:5e7b737f:247 unsaved=false. Encoded global types/code
bytes/bounds preserved. api.dll still selected, no navigation/API edits/execution.
Inventory208=73 internal+135 external,137 mixed rows,8 nonzero undefined ranges;
worker ready/no active job. Suggestions only: p_Var1->registrationResult and encoded
begin/end global descriptions in topic file. Next: ATL thread-codepage helpers,
remaining runtime fragments, then data/RTTI/linker layout.

2026-11-02 ATL milestone: see atl-codepage.md. Fallback101 and selecting thunk100
normalized-exact with5/8 fields; real getter01001f89 raw-exact4 bytes returns3.
Actual selector target plus atls:atltrace.obj symbol resolve the prior fragment
hypothesis; now stored as ATL::_AtlGetThreadACPReal, uint stdcall(void). Cached
getter bootstrap/alias uses/imports checked. New AtlCodepage gate; real
atl-codepage-20261102-001 retains all prior18/CRT gates. No source/flags/library
or baseline30 change; application nonmatches unchanged. Two negative controls
rejected, copies deleted. Original5-byte CC gaps after fallback/real differ from
rebuilt0/0: concrete layout difference. /FUNCTIONPADMIN:5 is a hypothesis supported
as a valid option by local VC8 help; not yet enabled/tested or claimed original.
Ghidra pre-edit GZF exported; real getter defined in existing ATL namespace,
generic pointer refined via /FSX/ATL/ThreadCodePageGetter and default label
replaced with ATL::g_pfnGetThreadACP. Other names/signatures/OSVERSIONINFOA local
and code preserved. Readback saved1793649165631:5e7b737f:275 unsaved=false. api.dll
still selected, no navigation/API edits/execution. Inventory209=74 internal+135
external;137 mixed rows,128 unassigned instructions/62 ranges;8 nonzero undefined
ranges. Worker ready/no active job. Suggestions only Value->selectedGetter and
local_9c->versionInfo. Next classify remaining runtime/data ranges, then isolated
padding test and data/RTTI/section/linker reconstruction.

2026-11-02 runtime-remainders milestone: see runtime-remainders.md. GS SEH
prologue0100270c/epilogue01002754 normalized-exact72/15 (two targets each),
multiply01002c50 raw-exact52; three six-byte import identities independently
checked. Mandatory RuntimeRemainders gate and fresh XP job
runtime-remainders-20261102-001 pass all prior gates. Baseline30/source/flags/
libraries unchanged; prior18/1114 body count and two application nonmatches stay
open. Two negative controls rejected and mutated copies deleted. Pre-edit GZF
exported; three functions created, three placeholders completed/linked to existing
external signatures, known initializer/message-map pointers, five strings and
three EH4 metadata models typed. No user-defined names overwritten/API edits/tab
navigation/runtime execution. Corrected inspection IAT119c vs unrelated1214.
Post-link noreturn analysis stores precision helper38 normal-flow bytes; complete
comparison remains41 incl three unreachable cleanup bytes now listed. Multiply
second path27 bytes also explicitly listed. Saved fsx.exe and native readback
unsaved_changes=false. Current212 model=77 internal+135 external,140 mixed rows,
129 unassigned instructions/63 ranges;5168 instruction/3290 data/1270 undefined
bytes. Zero nonzero/nonCC undefined ranges; not full recovery/match proof. All80
raw thunks listed:29 complete stored,51 unassigned. Inventory distinguishes stored
completeness from raw/listing discovery. Worker ready/no active job. Next isolated
/FUNCTIONPADMIN:5 experiment (still hypothesis), then RTTI/data/section layout.

2026-11-02 function-padding milestone: see function-padding.md. Source-identical
actual XP control and /FUNCTIONPADMIN:5 builds differ only by that command token.
Ten missing five-byte CC gaps restored; twelve of twelve comparable sampled
pairs match versus two without option. Four additional pairs are not adjacent
in rebuilt ordering, remain open. Prior untested option hypothesis superseded by
measured reconstruction support, NOT original-command/whole-image proof. Initial
padded job linked but hit old zero-gap assumption; retained. Five boundary checks
now separate unchanged exact body checks from explicitly checked CC gaps0/5 and
report FollowingGapBytes; gap-only NOP mutation rejected, altered binary removed.
Verified job padding-five-verified-20261102-001 passes all regressions; InitInstance
still769/777,246 instructions,43 nonbranch differences; ShowErrorMessage still2.
Default FunctionPadding5, explicit0 control. New golden/replay/inventory31-function-padding
preserves real output/source/request/log/PDB/MAP/diagnostics. EXE SHA256
6F43081D0A0256092F3A10F107F3377E6ABE25F89D7060880C19575361B5FF90.
Baseline30/control runs retained. No source/compiler/library/worker changes or
Ghidra DB edits/navigation; last saved model unchanged. Worker ready/no active job.
Next RTTI/data coverage, then section/import/base/function-order/header layout.

2026-11-02 RTTI milestone: see exe-rtti.md. Live original vtable predecessor1394
is string tail00000073;1454 is prior app slot010022a8/import JMP, neither local
COL. No scanned decorated class/struct names in original non-resource sections.
Baseline31 has two local COLs/eight type names. Source-identical /GR- experiment
removes these plus two original-absent CRT imports; build command differs only
by /GR- on both compiler lines. Rebuilt file12800->11264, not a match percentage.
Real rtti-disabled and rtti-default-20261102-001 pass prior18/1114/CRT/ATL gates
and new RttiMetadata gate; enabled31 rejected by required-shape control. Default
Rtti Disabled, Enabled control retained. New golden/replay/inventory32-rtti-disabled,
EXE E0E82FCFA5885B26692B9DA7C43E9D3BBB5883158605A715E65368BCA455650F;
31 and real runs retained. /GR- experimentally supported, not unique original
flag proof. ShowError2/InitInstance769vs777 and43 nonbranch differences unchanged.
Ghidra pre-edit GZF2237777 exported; appended evidence comments only at existing
FSXApp_vftable1398 and FSXMessageFilter_vftable1458, names/types preserved. Saved
fsx.exe, native readback unsaved=false; no tab navigation/API edits/execution.
Inventory model counts unchanged; reference is32 and RTTI absence shape passes.
Next constants/strings, MFC runtime-class/global data and other metadata, then
base/sections/imports/order/headers. MFC runtime classes are distinct from C++ RTTI.

2026-11-02 constants/state milestone: see exe-data-constants.md. Verified18
terminated literals/183 bytes via MAP identities; complete eight-token pointer
group32 at12b0 matches rebuilt4032b0, resolving prior open counterpart. Load-config
14d0/72 matches with two validated pointers; PE directory Size64 versus structure
Size72 agrees in both. SafeSEH table1560/8 has two independently checked handler
RVAs. Window88/app168/getter4 observed initial zero spans and12 named CRT DWORDs
agree. This is pre-initializer minimum storage, not full original object sizes.
First virtual runtime-class slots use inherited MFC imports#3333/#3169, separately
from C++ RTTI. Mandatory DataConstants gate, fresh data-constants-20261102-001
passes all previous gates; source/flags/libraries/golden32 unchanged. Fresh EXE
127D6E48578A2A152D5943B572AADF61844F4F778C685FACDA83D9DD967DDC65.
Three literal/pointer/SafeSEH mutations rejected; altered binaries removed. Pre-
edit GZF2237514; comments appended only to known data/globals, existing names/types
preserved. Saved fsx.exe, readback unsaved=false; no tab/API/code edits/execution.
Suggestion only: currently unlabeled12b0 -> g_mfcTokenStringPointers, reconstructed
name; original identifier/parser semantics unproven. Prior18/1114 count and
ShowError/InitInstance differences unchanged. Inventory reference32/new data scope
passes; worker ready. Next remaining data coverage and PE directories/imports,
then base/section placement/order/header reconstruction.

2026-11-02 imports/PE milestone: see pe-imports-and-directories.md. Full137
ordinary import identities from five DLLs, name hints, ordinal values, descriptor
order/spelling/fields and relative IAT module blocks agree.120 descriptor/568 IAT
bytes shape/termination verified; initial ILT/IAT equal within each image. Entry
order DOES NOT match:128 different slots,9 retained, four DLL blocks reordered;
ole32 order agrees. Mandatory PeImports content gate and separate rejecting
RequireExactOrder preserve distinction. Native135 external functions+two imported
data (_adjust_fdiv IAT11cc, _acmdln11e0) explain137. All16 PE directories inventoried;
original resource/certificate absent in build, debug GUID/path/timestamp differ.
Certificate9280 file-offset record, not RVA/mapped data; no crypto verification.
Header differences: base01000000 vs00400000, COFF0123/0103, DLL8100/0, OS5.2/4.0,
image5.2/0, stack40000/2000 vs100000/1000. Alignments/header size/heap agree.
Fresh pe-imports-20261102-001 passes content/all prior gates; EXE
0B7BD395AED23E685BA8342AD3BF3DDE9671FF0AB6DB6098CDF1FE4A99F36BB0.
Two binary controls and actual-order control rejected; mutated copies deleted.
Baseline32/source/flags/libraries unchanged. GZF2237723; hash-bounded comments
only at IAT1000/descriptors2e10/debug1260, saved/readback unsaved=false. No tab/API
changes/execution. Worker ready; no new naming suggestions or body match. Next
base experiment after hardcoded-comparator audit, then merged section layout,
header fields and unresolved import/code ordering; debug/certificate gaps stay open.
PE debug extent correction in the same milestone: raw CodeView32 versus Ghidra
model31 differs by the PDB path null terminator at1537, not extra padding. Own
initial comment corrected by CorrectPeDebugExtent.java; explicitly saved and
read back unsaved=false. No original bytes/types changed.

2026-11-02 image-base milestone: see image-base.md. Actual XP original/controlx
jobs pass all regressions and InitInstance diagnostics. Source hashes identical;
sole command delta /BASE:0x01000000. Equal-length PDB paths control debug record
size92; all343 image MAP symbols retain RVA, no missing/extra. Baseline32 debug
record85 causes an additional path-length layout effect; do not attribute that
to /BASE. PE base now original01000000; source locations/sections/import ordering
still differ. Four legacy comparators read actual PE base instead of00400000,
including ShowError fallback-string read; lengths/target checks/masks unchanged.
Updated checks also pass old32. Startup gate records/enforces requested base;
real old/new wrong-base controls rejected. Default ImageBaseMode Original,
explicit Default control. New golden/replay/inventory33-original-image-base,
EXE F6008B207A89E9EC20C95C4EC36FE237C044A3E812E1AB98A6FE7D8F3682AB61;
32 and actual controls retained. Prior18/1114 count, ShowError2/InitInstance769vs777,
43 nonbranch differences unchanged. No source/library/other-flag/worker/VM UI
changes or candidate execution. No Ghidra edits/save/navigation needed; original
last saved state unchanged. Inventory/vtable gates pass33; worker ready. Next
controlled section merge; keep PDB-length, header-field and import-order effects
separate. No new naming suggestions.

2026-11-02 section-merge milestone: see section-merge.md. Source-identical actual
XP sections-mergedx/control-20261102-001 differ only by /MERGE:.rdata=.text;
equal-length PDB paths give89-byte records. Both complete all regressions and
InitInstance diagnostics. Merged .text/.data names/order/RVAs/raw sizes/raw offsets/
flags match original non-resource sections. Virtual .text9686 vs9654 (+32),
.data1204 vs1184 (+20), not mismatch-byte counts. IAT1000/debug1260/loadcfg14d0
now exact RVAs; eight MFC token strings exact VAs; entry263b vs262e (+13), not
whole-function order proof. Import contents pass but128 entry slots remain
reordered. New SectionLayout gate enforces requested topology/flags; actual old/
new wrong-mode controls rejected. Default SectionLayout RdataIntoText, Separate
control retained. New golden/replay/inventory34-merged-readonly, EXE
5EED511BE9650E052B579B195DC594662696A7CE0A1AF048F84549C966E98670;
33 and real control outputs retained. No source/library/other-flag/worker changes.
Prior18/1114 count, ShowError2 and InitInstance769vs777/43 nonbranch differences
unchanged. Inventory/vtable checks pass34. No Ghidra edits/save/navigation/API
changes or candidate execution; prior saved original model unchanged. Worker
ready. Next debug path/size and independently observed header flags/version/stack,
then remaining code/import/data order, virtual extents and metadata contents.

2026-11-02 bounded cut: see stack-header.md. Actual stack-original-20261102-001
passes prior gates/InitInstance diagnostics; source-identical sole command delta
/STACK:0x40000,0x2000. Reserve256KiB/commit8KiB now match original. Startup gate
checks requested mode; actual old/new wrong-stack controls rejected. Default
StackMode Original; Default control retained. Golden/replay/inventory35-original-stack,
EXE465CFF301B8FE7DA2B084110C1D9878E23D992297D0CA796F4FF06454D35119E;
34 retained. Inventory/vtable checks pass. Known body nonmatches unchanged.
No Ghidra/source/library/other-flag/worker changes or candidate execution. Prior
original DB save unchanged; worker ready/no active job. Stopped at requested
usage cut. Next basename-only PDB/debug record, then PE flags/versions; maintain
explicit import/code/data order and metadata/layout nonmatches.

2026-11-03 PDB-path milestone: see pdb-path.md. Installed SP1 linker actually
accepts /PDBALTPATH:fsx.pdb; no ignored-option warning. Embedded path fsx.pdb,
Type2/RSDS/Age1/32-byte extent and record RVA1518/file918 match original shape;
GUID/timestamps remain unmatched. Source-identical actual pdb-basename and
longer-ID repeat jobs pass all regressions/InitInstance diagnostics. Sole control
command delta is altpath option; repeat commands identical. Different-length
worker directories now produce byte-identical EXEs outside parsed COFF/debug
timestamps and RSDS GUID (18 raw differing bytes entirely within those fields).
No arbitrary masks or original/debug identity claim. .text virtual9622 vs9654
(-32), .data1204 vs1184 (+20), entry25fb vs262e (-51): older distances/size deltas
superseded for36, not mismatch-byte counts. Mandatory PdbDebug gate enforces mode
and actual PDB presence; real35/36 wrong modes rejected. Default PdbPathMode
Basename, explicit Worker control. Golden/replay/inventory36-basename-pdb, EXE
1A3FB2A1E4E491778A77E6FC163567F8AD98028096C583C53FB3E7FE76254BB7;
35/two actual runs retained. Prior source/libraries/other flags/body counts and
ShowError/InitInstance nonmatches unchanged. Inventory/vtable checks pass36;
worker ready/no active job. No Ghidra edits/save/navigation/API changes/execution.
Next observed COFF/DLL flags, OS/image versions; keep order/virtual extent/debug
GUID/timestamp/certificate/resource differences open. Prior usage cut resumed
by user's current instruction.

2026-11-03 PE-flags/version milestone: see pe-header-settings.md. Five actual
single-option stages add LARGEADDRESSAWARE, NXCOMPAT, TSAWARE, VERSION:5.2,
OSVERSION:5.2. Final COFF0123/DLL8100/image5.2/OS5.2 match original; subsystem4.0
preserved. Installed linker actually emits OSVERSION; no ignored flag warning.
Source hashes same and sole command delta checked per stage; whole-file step
comparisons show zero changes outside the intended header field and parsed
COFF/debug timestamps/RSDS GUID. All prior gates and InitInstance diagnostics
pass; known body nonmatches/counts unchanged. New PeHeaderSettings gate enforces
requested independent modes; actual36/37 wrong-mode controls rejected. Defaults
allOriginal; intermediate/default controls retained. Golden/replay/inventory37-
original-header, EXE787831F3E09F7A490269686069598D7FEE8CFE2304871A762D724D73A64AAC09;
36 and five actual stages retained. No source/library/other-option/worker/VM UI
changes or Ghidra edits/save/navigation/API changes/execution. Prior saved model
unchanged. Inventory/vtable checks pass37; worker ready. New on-disk lead: CodeView
ends1538 in both; original SafeSEH1560 gives40 zero bytes, rebuilt1540 gives8,
a32-byte gap difference. Allocation/padding/release-postprocess cause unknown;
record as hypothesis, not recovered source. Next inspect this range and linker/
function/data ordering. .text virtual-32/.data+20, entry-51,128 import slot order
changes, debug GUID/timestamps/certificates/resources remain open. No new names.

2026-11-03 CodeView gap investigation: see codeview-gap.md. Native original
1538..155f is40 zero bytes with no defined data/instructions/symbols/incoming
model refs; SafeSEH1560.37 has8 zero bytes/SafeSEH1540. MAP identifies empty
.rdata$r between debug and sxdata. Optional DebugGapProbeBytes32 generates an
explicitly synthetic data-only object there; real probe job passes all prior
gates/InitInstance diagnostics, restores SafeSEH/RTC placement and .text virtual
9654. It is NOT recovered original source/variable.343 common symbols:147 +32,
192 unchanged, four SDK symbols (three leaf bodies) reorder; blanket shift model
false and reported. .data+20/entry-19 and code/import order remain unmatched.
Probe EXE A3A116882DB81944178B17C095F4A865CB3469094796F595490B9384A711BE5E.
Actual default control0 job also passes; no probe source in default. Default0/
golden37 retained, no38 promotion. Nonzero-gap mutation rejected/copy deleted.
Original allocation/postprocess cause unresolved. General BinPlace docs do not
prove any FSX release step; no release tool run or binary postprocessing applied.
GZF2238507 before append-only gap comment; saved fsx/readback unsaved=false.
No names/types/bytes/functions/API changes/navigation/execution. Worker ready.
Next actual function-order experiment while retaining gap origin as open and
optional synthetic layout model clearly separate. Known body nonmatches/counts
unchanged; no justified variable rename for unidentified zeros.

2026-11-03 roadmap consolidation (documentation only): rewrote
fsx.exe/exe-matching-roadmap.md as current37 priorities, superseding accumulated stale
baseline recommendations. Next function placement/order, import entry order,
writable data placement, evidence-led residual body work, then exhaustive range
reconciliation. Gap origin stays open with optional probe disabled. No claim that
all useful work is exhausted or a percentage/completion date is known. Final
metadata comparison policy remains explicit work. No build/default/Ghidra change.

2026-11-03 application-order milestone: see application-function-order.md. Actual
FirstThree and corrected ApplicationPrefix /ORDER builds accepted under current
GL/LTCG and retain prior gates.12 entries have exact original relative distances,
uniform-32; separate gap32 diagnostic matches all12 original entry VAs, synthetic
gap remains disabled. Header/source/other settings unchanged; response data is
hashed/returned by unchanged worker. Initial length check falsely consumed an
unlabeled static callback after destructor; bounded next-MAP parser keeps exact
70-byte/ref checks and passes old37/new builds. First expanded list had incorrect
OnMessagePending decorated name; LNK4037 ignored it. Corrected actual const MSG*
name, added mandatory warning/placement gate; old unordered37/bad-warning controls
reject. Default FunctionOrder Auto resolves ApplicationPrefix for MfcWindows,
Default for Console. Golden/replay/inventory38-application-order, actual default
EXE14413012056017DD01059F7EA9C7D8A80B527243DF1100F9189F49731C5B5469;
37/all trials retained. Default no probe. Import slots still128 reordered; known
body counts/nonmatches unchanged. Inventory/vtable checks pass; worker ready.
No Ghidra edits/save/navigation/API changes/candidate execution. Next unlisted
functions/callback and import/data order. Build-environment-factors.md answers
host OS versus SDK/target/toolchain/environment question with official primary
references; no evidence requires Vista host. Observed original FileVersionInfo
10.0.61637.0 (FSX-Xpack.20070926-1421), resource read only, not host OS proof.

2026-11-03 extended-order milestone: see extended-function-order.md. Actual static
callback All trial rejected LNK4037. Separately built external-linkage callback
with old prefix retains5-byte body/stdcall/import checks; original linkage unknown.
Initial All response with only Serialize fails actual order due folded Dump
representative. Including both aliases restores order; equality allowed only for
reviewed Serialize/Dump pair,76-slot alias/body gates preserved.22 symbols/21
entries through app/SDK/ATL now original relative placement, uniform-32. Separate
gap32 diagnostic gives exact22 original VAs; synthetic gap stays disabled. All
prior gates/InitInstance diagnostics pass; ShowError2/InitInstance769vs777/43
nonbranch differences and18/1114 count unchanged. Other C++/headers and command
same as38; only staged callback static removal and response contents change.
No export introduced. Defaults FunctionOrder Auto->ApplicationAll for MfcWindows,
CallbackLinkage Auto->External forAll/Static otherwise; console Default/Static.
Golden/replay/inventory39-extended-function-order, actual default EXE
F649657A9C559DC2B9CF95DC81A8E56F28972968078A34F3A49E8FC8D60FB29B;
38/trials retained. Selected prefix is not whole code order: entry26dd vs262e
(+175), .text virtual-32/.data+20 and128 import slot differences remain. Inventory/
vtable checks pass39; worker ready. No Ghidra edits/save/navigation/API changes,
VM UI operations or candidate execution. Next remaining CRT/library groups and
import-member selection, then data ordering; no original linkage/name claim.

2026-11-03 MFC thunk order experiment: see mfc-thunk-order.md. Original fsx.exe
01001ffc..0100230d:66 FF25 thunks mapped by exact import identity and unique MAP
symbol/member. Actual optional MfcThunks XP build accepts88 symbols (prior22 plus
66); all66 now original relative placement at uniform-32. Prefix22 unchanged,
all other C++/headers and build.cmd identical to39. Dedicated comparator validates
FF25 bytes and exact IAT targets, source/control inputs. All prior gates and
InitInstance diagnostics pass. EXE6F2DE3881ECE606C1A352A061CBA322DC484AB14523B5D97FC06F38821CE2A70.
IAT slots128 still differ and entry26dd still+175; thunk placement is distinct
from import-table order/remaining CRT order. Known body differences/count unchanged.
Auto remains ApplicationAll/golden39; successful experimental run retained.
Next CRT public/static contribution eligibility and library-member/IAT ordering.
No Ghidra edits/save/navigation, candidate execution, VM UI or API changes.

2026-11-03 CRT selection milestone: see crt-selection-order.md. COFF inspection
shows separate COMDAT startup sections, but actual /ORDER static-name probe
rejects exactly3 names (LNK4037); rejection retained, no gate weakened. Controlled
/INCLUDE:_WinMainCRTStartup with same88-symbol response/compiled inputs moves
crtexew before cookie/onexit.36 independently mapped runtime/framework symbols
now original-32, including entry0260e vs0262e; prior88 placements unchanged.
All body/reference/header/import/data gates and InitInstance diagnostics pass.
Import slot differences128->103, contents137 unchanged. Auto MfcWindows now
MfcThunks plus EntryInclude (RTM); console Default. Default actual build retained
as golden/replay/inventory40-crt-entry-selection, SHA256
323E3F7F56CD0A4423C08605DECCDFF5B8B3CF610EC9FA90670A198B3625F393.
39/trials retained; gap0, .data+20 and known application near-matches unchanged.
Inventory/vtable checks pass40; worker ready. Next remaining CRT thunks/selection
and IAT ordering, then data layout. Selected placement is not exhaustive code
equality. No Ghidra edit/save/navigation, names/types/API changes or VM UI/runtime
execution. Original linkage/command and zero-gap cause remain unresolved.

2026-11-03 complete thunk/import-selection controls: see import-selection.md.
Corrected prior unsupported remaining-CRT-thunk/debugger-hook placement claim:
all80 original FF25 identities in40 are unique and original-32; all80 immediate
CC runs match. Dedicated strict opcode/IAT/placement/padding report recorded.
IAT103 differences split KERNEL32 32/MFC64/CRT4/USER32 3/ole32 0; CRT has two
adjacent swaps. Full137 IAT INCLUDE response builds but fails existing placement
gate (134 slots differ,14 thunks move). Targeted3 direct CRT INCLUDE trial passes
all gates/all80 thunk checks but worsens slot differences to105. Neither adopted;
all C++/prior response hashes controlled, one added input/command response only.
Initial manifest preflight failed before publication; marked preparation_failed,
not a guest build. Baseline/default40 unchanged; ImportSelection Default, optional
controls retained. No blind permutations planned. Next writable-data+20 mapping;
remaining IAT selection mechanism needs targeted contribution evidence. Ghidra
save/backup/name/type/API state unchanged; no candidate runtime/VM UI. Worker ready.

2026-11-03 writable-data ledger/probes: see writable-data-layout.md.30 disjoint
reference/initial-state mappings cover1153 bytes in each image; original .data
1184 leaves31 unassigned,40's1204 leaves51. Offsets vary (-8,-4,+12,+16,+20,+32),
not a uniform+20. Unassigned includes values/unreferenced state, not proven padding.
Existing original MFC one-byte flag437c preserved; no full class-size inference.
Actual never-linked COFF diagnostic probes establish cookie/ATL input alignment4,
natstart data8/alignment4 and failure BSS804/alignment8. Read-only fsx Ghidra
metadata/xrefs checked; no refs to400c/4010/4014/4380, event4484 established.
ATL INCLUDE changes no layout; explicit atls before CRT puts cache4000 but cookie
4010 instead of4004, retains+20 (IAT102). Explicit .bss=.data produces+32/raw1536
and LNK4254; not adopted. All three retain compiled inputs and pass existing
normalized-body/all80 thunk/CC gates, which do not override measured layout
regressions. Defaults/baseline40 unchanged, IAT103, gap0, body near-matches remain.
Read-only existing RTM-media MFC member inspection retained; no MFC substitution
built/installed and no original library version inferred. Next installed/media
force/data contribution comparison and remaining original gap ownership. No
Ghidra edit/save/navigation or names/types/API change, no VM UI/runtime execution.
Worker ready; archive40 SHA323E3F7F56CD0A4423C08605DECCDFF5B8B3CF610EC9FA90670A198B3625F393.

2026-11-03 MFC support milestone: see mfc-support-library.md. Captured installed
libraries with guest fc /b verification, no installed replacement. Relevant RTM/
installed MFC data shapes/symbols identical; appmodul directive differs: SP1
includes __forceMFCManifest, RTM directly requests MFC8.0.50608.0. Actual isolated
RTM mfcs80.lib removes the reconstruction's4-byte common symbol; .data1200 versus
original1184 (+16), raw512 retained. Matching RTM PDB extracted; initial4099 trial
retained, PDB/default builds have no linker warnings.30 spans1153 bytes retained,
unassigned31 original/47 rebuilt. Prefix88/IAT entries unchanged (103 slot diffs),
all80 thunk placements/CC and prior body/data/header gates pass. Original embedded
manifest24/1/1033 requests MFC+CRT8.0.50608.0; supporting context, not original
compiler/library proof. Generated sidecar still has extra SP1 dependencies and
is not claimed matched; no resource reconstruction/runtime test. Default Auto
RTM support for RTM MfcWindows, InstalledSP1 otherwise; pinned library+PDB hashes.
Golden/replay/inventory41-rtm-mfc-support SHA256
6953DEA3584BECDFBE03A0D55511D8B4093A593F421603B97EE3938214E8580B.
40/trials retained; inventories pass41. Next remaining16-byte data placement.
Known body differences/gap0 unchanged. No Ghidra edit/save/navigation/name/type/
API or VM UI changes; worker ready.

2026-11-03 data contribution diagnostic: see data-contribution-alignment.md.
Read-only MSF7/DBI reader pairs41 PDB/EXE GUID+age;20 public data owners cross-check
MAP,3 known pilot globals lack table contributions (not padding).913 data bytes
reported, unreported ranges4018/8,4024/12 (raw zero) and438d/267 (includes pilot
storage). No original PDB/owner map or original-gap inference. Wrong-PDB override
negative control rejects GUID/age; both genuine pairs parse. Native application
TU control changes only contextGL->GL- and getterGL omission, retaining linkLTCG
and identical source/response hashes. .data1192 (+8) but .text9786/raw10240 and
global-app-initializer appCtor mapping fails; not adopted, no full comparison.
Import137 contents still pass. Data layout responds to context, but specific
alignment rule/cause of remaining16 not isolated. Defaults GL/GL/golden41 intact;
ContextCompilation NonGL diagnostic mode recorded. Next evidence-led toolchain/
section-order control preserving bodies; no blind packing/global removal.
No Ghidra activity, names/types/save/navigation/API change or candidate runtime/
VM UI/installed toolchain changes. Existing gaps/near-matches remain; worker ready.
