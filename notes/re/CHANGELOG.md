# Analysis change log

2026-11-25 — Reconstructed turbine preparation, jet spools, starter,
fuel delivery and post-thrust state and connected the common turbine
loop. Retained the 24 original and rebuilt listings, native VC8
comparisons and host x86 loop checks.
[Turbine preparation and state](sim1.dll/sim1-turbine-preparation-and-state.md).

2026-11-25 — Reconstructed gross thrust, ram drag, the reverser
and afterburner adjustments, net turbine thrust and the last inline
turbine speed blend. Mapped the independent propeller handoff and
retained original listings, native VC8 comparisons and host x86 evidence.
[Turbine thrust and the engine-loop boundary](sim1.dll/sim1-turbine-thrust-and-tail.md).

2026-11-25 — Reconstructed the common turbine inlet correction
and shared ratio publications. Verified the AIR Mach cap selector,
retained the VC8 and host x86 results, and mapped the original
and rebuilt instructions and three ratio globals in the dashboard.
[Turbine inlet conditions](sim1.dll/sim1-turbine-inlet.md).

2026-11-25 — Reconstructed the type-3 and type-5 turbine spool
update, traced the inlet temperature ratio and retained both native
VC8 profiles and host x86 checks for the spool-to-torque handoff.
Recorded the original listing and scoped comparisons in the dashboard.
[Shaft turbine spool speed](sim1.dll/sim1-turbine-spool-speed.md).

2026-11-25 — Reconstructed turbine shaft torque in a separate
translation unit, preserved the original binary64 publication boundary,
and corrected unordered input handling in the shared curve evaluator.
Retained native VC8 and host x86 evidence and added dashboard listings.
[Turbine shaft torque](sim1.dll/sim1-turbine-shaft-torque.md).

2026-11-25 — Moved the electrical runtime and circuit checks into
a separate translation unit. Updated the native and linked build
workflows and the dashboard source and disassembly links.
[Electrical circuit availability](sim1.dll/sim1-propeller-step.md#electrical-circuit-availability).

2026-11-25 — Reconstructed automatic feather arming and its
turbine torque predicate, linked both preparation calls to the
mapped propeller entry, and preserved the original four-propeller
write destinations. Saved Ghidra tags and retained VC8 and x86
comparison evidence and dashboard listings.
[Automatic feather arming](sim1.dll/sim1-propeller-step.md#automatic-feather-arming).

2026-11-25 — Reconstructed the shared electrical circuit query
and bus lookup in `sim1.dll`, wired the propeller deice caller,
and identified the automatic-feather circuit selector. Saved
Ghidra tags and retained native VC8 comparisons, both listings
and host x86 checks.
[Electrical circuit availability](sim1.dll/sim1-propeller-step.md#electrical-circuit-availability).

2026-11-25 — Reconstructed the propeller icing load calculation,
verified the rate table, and retained VC8 listings and host x86
checks. Preserved the electrical query as an explicit input.
[Propeller icing load factor](sim1.dll/sim1-propeller-step.md#propeller-icing-load-factor).

2026-11-25 — Integrated the mapped propeller calculations following
preparation, including torque division, engine speed feedback and
per-propeller outputs. Kept VC8 evidence and saved scoped Ghidra tags.
[Integrated mapped propeller passes](sim1.dll/sim1-propeller-step.md#integrated-mapped-propeller-passes).

2026-11-25 — Reconstructed the ground contact selector, verified
its installed virtual dispatch target and wired the complete
blade-pitch entry to the extracted stages. Saved Ghidra tags and
retained VC8 comparisons and original and rebuilt listings.
[Ground contact selector and integrated blade-pitch update](sim1.dll/sim1-propeller-step.md#ground-contact-selector-and-integrated-blade-pitch-update).

2026-11-25 — Reconstructed propeller feather state and the
governor latch update with its call. Saved Ghidra tags and preserved
both listings, field evidence and VC8 comparisons.
[Feather state and low-speed latch](sim1.dll/sim1-propeller-step.md#feather-state-and-low-speed-latch).

2026-11-25 — Reconstructed the advance ratio and pitch calculation
ranges in the propeller update. Saved scoped Ghidra tags and preserved
native VC8 comparisons, original and rebuilt listings, and x86 tests.
[Blade pitch and advance ratio](sim1.dll/sim1-propeller-step.md#blade-pitch-and-advance-ratio).

2026-11-25 — Reconstructed propeller thrust publication and verified
all 23 low-speed curve points. Reused the shared coefficient lookup
and replayed the propulsion and aerodynamic comparisons. Saved Ghidra
tags and retained the original and rebuilt listings.
[Propeller thrust publication](sim1.dll/sim1-propeller-step.md#propeller-thrust-publication).

2026-11-25 — Reconstructed the propeller aerodynamic torque and
its coefficient table lookup. Saved Ghidra tags and preserved both
original and rebuilt listings, AIR record bindings and VC8 trials.
[Aerodynamic propeller torque](sim1.dll/sim1-propeller-step.md#aerodynamic-propeller-torque).

2026-11-25 — Reconstructed the propeller speed integrator and kept
its original and rebuilt listings. Verified and bound the phase-wrap
import by ordinal for the linked propulsion regression. Saved Ghidra tags.
[Propeller speed integration](sim1.dll/sim1-propeller-step.md#propeller-speed-integration).

2026-11-25 — Mapped the propeller step and reconstructed its
speed synchronization helper. Saved Ghidra tags and preserved the
listings and VC8 comparison trials. Corrected the earlier attribution
of the propeller load factor read in `0x20c47ba0`.
[Mapped propeller step and speed synchronization](sim1.dll/sim1-propeller-step.md).

2026-11-25 — Reconstructed the propeller induced-airspeed writer
in `sim1.dll:0x20c480b0`. Retained the VC8 trials, both dashboard
listings and Ghidra tags. Replayed the linked propulsion comparisons.
[Propeller induced airspeed](sim1.dll/sim1-engine-branches.md#propeller-induced-airspeed).

2026-11-25 — Reconstructed the flap-group coefficient writer,
verified its configuration and runtime flap bindings, and corrected
the earlier slipstream labels in source and dashboard records.
Preserved prior build evidence and saved Ghidra tags.

2026-11-25 — Reconstructed the tail slipstream velocity and
pressure update in `sim1.dll`, compared two VC8 loop forms,
and retained the original and rebuilt listings and Ghidra tags.

2026-11-25 — Renamed the shared airplane aerodynamics translation
unit to `src/sim1/aerodynamics.cpp` and updated the VC8 build input,
source mappings and documentation links. The VC8 replay reproduced
all 50 COFF `.text` sections with their relocations and all 15
comparison results.
[Rename verification run](../../vm-share/modules/sim1/runs/sim1-aerodynamics-tu-rename-20261125-001).

2026-11-25 — Reconstructed the high-alpha reference shift block
in `sim1.dll`, including both curves and the disabled branch.
Saved Ghidra tags and retained COFF comparisons and listings.

2026-11-25 — Reconstructed the shared aerodynamic pressure and
rate scales, retained VC8 comparison and dashboard listings,
and saved scoped Ghidra tags on `sim1.dll:0x20c00530`.

2026-11-25 — Reconstructed airplane lift, drag, side force and
normal body-axis force publication in `sim1.dll`. Retained native
comparisons and the original and rebuilt dashboard listings.
Saved Ghidra tags and three small coefficient definitions.
[Normal airplane aerodynamic forces](sim1.dll/sim1-aerodynamic-forces.md).

2026-11-25 — Reconstructed the normal yaw callback in
`sim1.dll:0x20c020a0`, including the external tanks, asymmetric
propeller thrust, gyroscopic reaction and SimpleAero return.
Retained native comparisons and both dashboard listings, and
saved the Ghidra accessor definitions and function tags.
[Normal airplane yaw moment](sim1.dll/sim1-yaw-moment.md).

2026-11-25 — Reconstructed the normal roll moment and SimpleAero
return from `sim1.dll:0x20c01890`. Added the torque and wing load
contributions, compared the native VC8 build, and saved the
Ghidra definitions and tags. Corrected the four pitch accessor
bindings using the live constructor and updated both sets of
listings in the dashboard.
[Normal airplane roll moment](sim1.dll/sim1-roll-moment.md).

2026-11-25 — Assembled the full normal-airplane pitch moment in
the `sim1.dll` reconstruction, including flap, spoiler, slipstream,
supplemental and `SimpleAero` terms. Corrected the original gyro
addition address and identified the `REALISM/SimpleAero` selector.
Matched four complete coefficient accessors, retained the comparisons
and dashboard listings, and saved the Ghidra definitions and tags.
[Complete pitch moment calculation](sim1.dll/sim1-aerodynamic-moment.md#complete-pitch-moment-calculation).

2026-11-25 — Reconstructed the `sim1.dll` elevator trim
coefficient and scaled caller contribution. Retained both
listings, VC8 trials, seven comparison scopes, dashboard
mappings and saved Ghidra tags. Recorded the control-surface
provider registration and its GUID.
[Elevator trim contribution](sim1.dll/sim1-aerodynamic-moment.md#elevator-trim-contribution).

2026-11-25 — Reconstructed the `sim1.dll` horizontal-tail
incidence coefficient and scaled caller contribution. Retained
the original and rebuilt listings, both VC8 trials, comparisons
and dashboard mappings. Saved and verified the Ghidra tags.
[Horizontal-tail incidence contribution](sim1.dll/sim1-aerodynamic-moment.md#horizontal-tail-incidence-contribution).

2026-11-25 — Initialized Git and added a root README covering the
verified FSX Acceleration reference version, matching workflow,
local dashboard, and contribution conventions.

2026-11-25 — Added Git exclusions for the original installation, VM
workspace, Ghidra database, local evidence and generated dashboard.
The local evidence directory contains binary and text exports.

2026-11-25 — Moved the `fsx.exe` sources into `src/fsx/` and updated
the build controller and dashboard paths. Archived the broad API source
pass and its generators with the original export. The opt-in regression
profile still uses the frozen snapshot, and the dashboard retains the
recorded comparisons. [API source pass](api.dll/api-source-pass.md).

2026-11-24 — Reconstructed the `sim1.dll` angle-of-attack
rate, its aerodynamic coefficient and its contribution to the
pitch moment. Retained both listings, two VC8 builds, comparison
reports, dashboard mappings and saved Ghidra tags.
[Angle-of-attack rate contribution](sim1.dll/sim1-aerodynamic-moment.md#angle-of-attack-rate-contribution).

2026-11-24 — Reconstructed the `sim1.dll` pitch-rate
coefficient and scaled moment contribution. Retained the AIR
and rate producer evidence, original and rebuilt listings, VC8
comparisons, dashboard rows and saved Ghidra tags. Shared the
COFF listing and instruction-range comparison code with the
elevator comparison and reproduced its retained report.
[Pitch-rate contribution](sim1.dll/sim1-aerodynamic-moment.md#pitch-rate-contribution).

2026-11-24 — Reconstructed the `sim1.dll` elevator moment
coefficient and its caller contribution. Retained the original
listing, VC8 comparisons, AIR and control evidence, dashboard
listings and saved Ghidra tags.
[Elevator contribution](sim1.dll/sim1-aerodynamic-moment.md#elevator-contribution).

2026-11-24 — Reconstructed the `sim1.dll` zero-alpha moment
coefficient and its scaled caller contribution. Retained the
original listings, two VC8 trials, exact and normalized ranges,
dashboard data and saved Ghidra tags.
[Zero-alpha moment coefficient](sim1.dll/sim1-aerodynamic-moment.md#zero-alpha-moment-coefficient).

2026-11-24 — Documented the alpha filter recurrence and bypass.
Exposed scoped rebuild listings on parent function pages and
added links to retained build disassembly files for Sim1 comparisons.

2026-11-24 — Reconstructed the `sim1.dll` moment coefficient curve
lookup and scaled angle contribution. Retained the AIR loading
evidence, native COFF comparisons, dashboard listings and saved tags.
[Angle-dependent moment coefficient](sim1.dll/sim1-aerodynamic-moment.md#angle-dependent-moment-coefficient).

2026-11-24 — Added the three `sim1.dll` aerodynamic force transfer
terms, both model reference adapters, and their native COFF and
dashboard evidence. Saved the Ghidra tags on the moment functions.
[Force transfer to the moment reference](sim1.dll/sim1-aerodynamic-moment.md#force-transfer-to-the-moment-reference).

2026-11-24 — Reconstructed the propeller gyroscopic contribution
from `sim1.dll` airplane moment slot `+0x14`. Retained the native
COFF comparison, original input and listing evidence, the dashboard
comparison, and saved Ghidra tags.
[Airplane aerodynamic moment](sim1.dll/sim1-aerodynamic-moment.md).

2026-11-24 — Recorded the airplane moment VTable prefix and larger
flight-model entry points in the `sim1.dll` evidence and dashboard.

2026-11-24 — Added the `sim1.dll` turbine output reconstruction
and native COFF comparison, plus original and rebuilt listings,
literal evidence and saved Ghidra tags.
[Turbine speed-scaled output](sim1.dll/sim1-engine-branches.md#turbine-speed-scaled-output).

2026-11-24 — Reconstructed the `sim1.dll` propulsion force collector,
verified its output copy and return, and retained the full comparison
and projection regression. Added dashboard listings and saved Ghidra tags.
[Propulsion force collector](sim1.dll/sim1-propulsion-collector.md).

2026-11-24 — Reconstructed and matched the `sim1.dll` thrust
projection at `0x20c01010`. Added its linked comparison, original
and rebuilt dashboard listings, source mapping and saved Ghidra tags.
[Thrust projection](sim1.dll/sim1-engine-branches.md#thrust-projection-and-accumulation).

2026-11-24 — Mapped the `sim1.dll` engine branches, verified the
original PE jump table and force output connections, saved and
read back Ghidra function tags, and added the evidence to the
dashboard.
[Engine branch mapping](sim1.dll/sim1-engine-branches.md).

2026-11-24 — Added the `sim1.dll` force and moment callback and mass
reconstruction, linked comparison workflow, reviewed callback
bindings, dashboard evidence and saved Ghidra tags.
[Forces and moments](sim1.dll/sim1-forces-and-moments.md).

2026-11-24 — Added the `sim1.dll` velocity, longitude and altitude
reconstruction with a shared minimum flight-state layout. Added
native and linked position comparisons, original listings,
dashboard source mappings and saved Ghidra tags.
[Position writers](sim1.dll/sim1-position-writers.md).

2026-11-24 — Added the complete stored `sim1.dll` function inventory,
PE boundary reconciliation, reviewed subsystem anchors, dashboard
coverage and a proposed reconstruction order.
[Function inventory](sim1.dll/sim1-function-inventory.md).

2026-11-24 — Added `sim1.dll` flight-state C++ reconstruction, native
VC8 build and comparator, verified original inventory, dashboard results
and VTable evidence. Applied and saved the Sim1 Ghidra function tags.
[State integration](sim1.dll/sim1-state-integration.md).

2026-11-24 — Added `fe.dll` magnetic-variation C++ reconstruction,
linked VC8 comparisons and dashboard results. Verified the Sim1 import
and retained the tagged FE model in a GZF.
[FE lookup](fe.dll/fe-magnetic-variation.md).

2026-11-24 — Reconstructed and matched three `api.dll` mapped-file
and interface cleanup bodies and published the original and rebuilt
listings and source mappings in the dashboard.
[Mapped file and interface cleanup adapters](api.dll/api-pointer-owner-updates.md#mapped-file-and-interface-cleanup-adapters).

2026-11-24 — Reconstructed and matched four `api.dll` buffer
owner forwarding and reset bodies and added their original and
rebuilt listings and source mappings to the dashboard.
[Buffer owner forwarding and reset](api.dll/api-pointer-owner-updates.md#buffer-owner-forwarding-and-reset).

2026-11-24 — Added VC8 RTM ATL and `_com_ptr_t` COFF controls for
the three `api.dll` COM-pointer wrapper bodies. Recorded separate
caller and callee findings in the topic and dashboard.
[VC8 COM smart-pointer provenance check](api.dll/api-pointer-owner-updates.md#vc8-com-smart-pointer-provenance-check).

2026-11-24 — Reconstructed six activity and lock lease bodies in
`api.dll`, matched five and retained the wait helper's four
non-relocation differences. Added all six listing pairs and source
mappings to the dashboard.
[Activity and critical-section leases](api.dll/api-pointer-owner-updates.md#activity-and-critical-section-leases).

2026-11-24 — Reconstructed seven `api.dll` mapped-view and handle
transfer bodies and verified 208 native COFF bytes. Added their
original and rebuilt listings, source links and ten relocation fields
to the dashboard.
[Mapped view and handle transfer](api.dll/api-pointer-owner-updates.md#mapped-view-and-handle-transfer).

2026-11-24 — Reconstructed seven `api.dll` pointer-owner adapters,
verified 171 native COFF bytes through seven relocation fields and
published both listings with the corresponding source mappings.
[Adjacent adapters](api.dll/api-pointer-owner-updates.md#adjacent-pointer-owner-adapters).

2026-11-24 — Reconstructed the two `api.dll` owner-free endpoints
and adjacent process-heap wrappers. Verified 110 native COFF bytes
with seven named relocations, updated the private heap evidence, and
published four source mappings and listing pairs.
[Heap bridge endpoints](api.dll/api-pointer-owner-updates.md#heap-bridge-endpoints).

2026-11-24 — Reconstructed six pointer-owner release and replacement
bodies in `api.dll`. Matched 136 COFF bytes and verified the prior
four update bodies with the new helper definitions. Updated listings
and source mappings in the dashboard.
[Release follow-up](api.dll/api-pointer-owner-updates.md#release-and-replacement-follow-up).

2026-11-24 — Reconstructed four previously Inventoried `api.dll`
pointer-owner updates and matched their 100-byte native COFF block
after validating four call targets. Published all source mappings and
listing pairs. [Pointer owner updates](api.dll/api-pointer-owner-updates.md).

2026-11-24 — Reconstructed the eight `api.dll` default copies at
`0x20161267`–`0x20161356` in a separate native COFF build. Verified
240 bytes of code with eight data references and 196 initial data bytes.
Retained the eight constructor matches and added function and data
evidence to the dashboard.
[Default-copy block](api.dll/api-construction-records.md#default-copy-block-at-0x20161267).

2026-11-24 — Reconstructed eight `api.dll` construction-record
initializers and verified their entire 160-byte native COFF block
through eight validated call relocations. Added source mappings and
both listings per entry. [Constructor block](api.dll/api-construction-records.md).

2026-11-24 — Reconstructed the previously Inventoried `api.dll`
range check at `0x201513ba` with its unused second stack argument.
Verified the complete 21-byte COFF match and added both listings
and the source mapping to the dashboard.
[Signed-to-unsigned range check](api.dll/api-checked-arithmetic.md#signed-to-unsigned-range-check-at-0x201513ba).

2026-11-24 — Reconstructed the unsigned comparison at `0x20153784`
in `api.dll`, retained the RTM, pinned SP1 and linked LTCG evidence,
and added both listings to the dashboard with status Different.
[Unsigned comparison](api.dll/api-checked-arithmetic.md#unsigned-32-bit-comparison-at-0x20153784).

2026-11-19 — Reconstructed the `api.dll` 64-bit subtraction
forwarder at `0x201711d2`. Verified the 29-byte native COFF body
after checking the direct call target, and added both listings and
the source mapping to the dashboard.
[64-bit subtraction forwarder](api.dll/api-checked-arithmetic.md#64-bit-subtraction-forwarder-at-0x201711d2).

2026-11-19 — Reconstructed the checked unsigned 64-bit subtraction
in `api.dll` at `0x20171131`, verified its complete 50-byte native COFF
body and added the source mapping and both listings to the dashboard.
[Checked 64-bit subtraction](api.dll/api-checked-arithmetic.md#unsigned-64-bit-subtraction-at-0x20171131).

2026-11-19 — Reconstructed the checked unsigned subtraction in
`api.dll` at `0x2015347a`. Verified its complete 34-byte native COFF
body and published the source mapping and both listings in the dashboard.
[Checked subtraction](api.dll/api-checked-arithmetic.md#unsigned-subtraction-at-0x2015347a).

2026-11-19 — Reconstructed the checked unsigned add at `0x2014e674`
and verified the 35-byte native COFF match. Retained the ordinal 160
COFF and linked layout controls. The dashboard includes both listings
for each result. [Checked arithmetic](api.dll/api-checked-arithmetic.md).
[String comparison](api.dll/api-string-compare.md).

2026-11-19 — Reconstructed the indexed callback-array lookup for
`api.dll` ordinal 172. Verified its PE export slot and 34-byte native
COFF body. Published the typed source and both listings in the dashboard.
[Callback array lookup](api.dll/api-dialog-registry.md#ordinal-172-record-lookup).

2026-11-19 — Reconstructed `api.dll` `ErfSetCodes` using the SDK's ERF
record and verified its complete 30-byte x86 COFF body. Added an explicit
RTM factory build option and published the matched source and both listings
in the dashboard. [FDI error record](api.dll/api-cab-error.md).

2026-11-19 — Live verified for fsx.exe. The packaged VC8 RTM replay and independent repeat reproduce all 137 IAT identities at their original slots and all 80 six-byte import thunks at their original addresses. The 146 dashboard scopes now contain 145 Exact and one Different. ShowErrorMessage retains its single JL/JS opcode difference at 0x01001e0e. [Linked-byte audit](../../artifacts/fsx.exe/iat-complete-20261119/audit.json).

2026-11-19 — Live verified for fsx.exe at 0x010021ac and 0x01002c38. The packaged RTM getter build also reproduces both six-byte MFC thunks at their original VAs using the separate AfxWinMain import and thunk archive. All seven getter references remain exact. Three CRT imports rotate, affecting InitInstance, the CRT exception thunk and AfxInitialize. The dashboard retains the stronger overall build. [Comparison](../../artifacts/fsx.exe/mfc-getter-pair-20261119/result.json).

2026-11-19 — Added the RTM `AtlObjectMfcNolib` profile and a packaged
replay for `fsx.exe`. Verified all seven getter references, the 30
mapped writable ranges and the 146 linked code scopes. Refreshed the
dashboard from new Ghidra and DUMPBIN listings and mapped the
`Run` and `ExitInstance` labels to their C++ definitions.
[Linked comparison](../../artifacts/fsx.exe/getter-placement-20261119/audit.json).

2026-11-18 — Reordered the reconstructed `fsx.exe` startup definitions
and gave the shared empty caption a named read-only array. Verified the
668-byte read-only match in XP and refreshed linked dashboard listings
and data comparisons.

2026-11-18 — Verified a separate `fsx.exe` MFC import-pointer and
thunk trial. Linked its exact MFC result and residual CRT cycle from
the dashboard while retaining the previous active comparison.

2026-11-18 — Updated the `fsx.exe` opt-in IAT profile to link the
MFC support library first. Verified both remaining CRT slots and
refreshed the dashboard with the new packaged code comparisons.

2026-11-18 — Added and replayed the opt-in `fsx.exe` `IatOrder` link
response. Reduced shifted IAT slots from 90 to four and refreshed the
dashboard with the verified packaged output and original listings.

2026-11-18 — Refreshed all 146 `fsx.exe` dashboard code comparisons
from the packaged link, with fresh Ghidra listings. Forty formerly
normalized ranges compare exact. Recorded remaining IAT, read-only
data and writable-object reference differences by operand.

2026-11-18 — Reproduced the `fsx.exe` CodeView basename and zero gap
with VC8 RTM and Microsoft BinPlace, repeated the fixed-path XP build,
and updated the ATL pointer dashboard comparison to the exact packaged
result.

2026-11-18 — Verified resource and PDB path layout controls for `fsx.exe`
in XP. Recorded the worker-path `.data` byte match and exact thunk entry
addresses in the comparison evidence and linked the evidence from the
ATL pointer dashboard entry.

2026-11-18 — Added isolated CRT/MFC ordering trials for the remaining
linked address deltas in `fsx.exe`. Explicit startup-member comparisons
check extracted object hashes as well as the MAP owner.

2026-11-18 — Added and verified the opt-in ATL-first link profile for
`fsx.exe` and updated its writable-data dashboard records.

2026-11-18 — Reconciled `fsx.exe` writable PE, PDB and MAP contributions
with the original range ledger. Preserved three isolated XP builds and
their comparison results.

2026-11-18 — Added three verified CRT range comparisons and six initial-state
byte comparisons for `fsx.exe`. Corrected the classification of three rebuilt
PDB contribution addresses that appeared in the function view.

2026-11-18 — Verified the 23-byte embedded CRT filter range at
`fsx.exe 0x010028fd` in a fresh XP build and added both listings with
status `Exact` to the dashboard.

2026-11-18 — Added four `fsx.exe` InitInstance range comparisons with
paired original and rebuilt listings and validated address fields from
the complete linked method comparison.

2026-11-18 — Reviewed documented dashboard entries across the five
inventoried modules and added 254 existing source references. Recorded
unresolved and non-code addresses in per-module audit files. Excluded
dates embedded in build IDs from the address-table importer.

2026-11-18 — Verified the 26-byte D3DX lookup and call at
`fsx.exe 0x01001a51`, added both scoped listings to the dashboard,
and saved a Ghidra block comment.

2026-11-18 — Added the verified D3DX range at `fsx.exe 0x01001a44` as a
direct dashboard entry with its original and rebuilt listings. The dashboard
shows the comparison status and describes the verified scope in the entry.

2026-11-18 — Verified the `fsx.exe` D3DX startup sequence at
`0x01001a44` in a fresh RTM build, retained its scoped byte comparison,
and saved a Ghidra block comment.

2026-11-18 — Reconstructed and matched `AfxInitialize` at `0x01002c0d`,
added the optional XP build probe, and saved the verified Ghidra
name and prototype.
[MFC module initialization](fsx.exe/fsx-mfc-initialize.md).

2026-11-18 — Added the job-local, hash-pinned VC8 SP1 compiler profile and
verified it on the XP worker. Recorded the `ShowErrorMessage` control-flow
trials and the one-byte result in both compiler profiles. The pinned SP1
integration also passed `InitInstance` and the existing regression gates.
[Evidence](fsx.exe/startup.md#pinned-sp1-and-control-flow-trials).

2026-11-18 — Rechecked the original `fsx.exe` `ShowErrorMessage` listing,
retained source and XP comparison evidence for the branch opcode trials,
and verified the one-byte baseline again.
[Follow-up](fsx.exe/startup.md#follow-up-on-the-branch-opcode).

2026-11-18 — fsx.exe ShowErrorMessage at 0x01001d30 now has one residual byte in the canonical RTM build. The original JL and rebuilt JS follow TEST EAX,EAX. InitInstance retains its complete normalized match. Ghidra readback was saved. [Callback comparison](fsx.exe/startup.md#rtm-callback-comparison-2026-11-18).

## 2026-11-17 — fsx.exe InitInstance matched

Reconstructed the MFC startup override and its inlined D3DX check. Added
the full 769-byte linked comparison, a retained two-byte negative control,
an optional pinned RTM toolset in the EXE controller, and the original and
rebuilt dashboard listings. Fresh SP1 and RTM builds passed the target and
regression checks. [Method and evidence](fsx.exe/initinstance-matching.md).

## 2026-11-17 - simscheduler.dll storage wrapper call restored

Extended the linked probe to use the reconstructed pool acquisition,
lock, free-list and destructor bodies. Recorded the single remaining
wrapper byte after call-target validation for both VC8 revisions.

## 2026-11-17 - simscheduler.dll linked storage cleanup

Added a class allocation probe for the factory unwind path and
verified both linked pool release edges. Recorded the remaining
wrapper and funclet byte differences in the dashboard.

## 2026-11-17 - simscheduler.dll RTM storage unwind control

Enabled the RTM tool profile for the native unwind probes, annotated
the storage wrapper entry, and verified identical RTM and SP1 body
bytes. Updated the dashboard source mapping and comparison.

## 2026-11-17 - simscheduler.dll normalized RTM dispatcher

Added a side-by-side RTM dispatcher profile, independently checked
the scheduler-object HIGHLOW fixup, and reproduced the complete
88-byte linked body in two builds. The source-identical SP1 control
reproduces the 86-byte diagnostic.

## 2026-11-17 - simscheduler.dll dispatcher handler ABI

Combined the linked handler profiles and extended the dispatcher
comparator to resolve calls and the scheduler-object address.
Verified an 86-byte diagnostic and updated its dashboard listing.

## 2026-11-17 - simscheduler.dll normalized live-list removal

Passed the live-list member directly and retained the guarded reverse
scan with a single exit. Verified the complete linked helper body.

## 2026-11-17 - simscheduler.dll exact propagation helper

Moved the index into the initial count check and advanced it when
selecting the entry. Verified the complete linked helper and its caller.

## 2026-11-17 - simscheduler.dll normalized update setter

Verified register selection across three build boundaries. Recovered
the propagation helper's separate rounded entry value and raw baseline
copy, then matched the full linked 50-byte setter.

## 2026-11-17 - simscheduler.dll update setter stack argument

Reconstructed the by-value time copy and verified the remaining five
register-encoding differences in the complete linked update setter.

## 2026-11-17 - simscheduler.dll linked update setter comparison

Added a linked update setter profile and recovered the original
aligned frame. The body remains Diagnostic.

## 2026-11-17 - simscheduler.dll exact time helper

Recovered the helper stack alignment and verified the complete
19-byte linked body at 0x20cc15d0.

## 2026-11-17 - simscheduler.dll cleanup register trials

Retained two linked comparisons for the context container cleanup
and verified the ECX allocation in the early count-read variant.

## 2026-11-17 - simscheduler.dll container count write

Recovered the unconditional count write and retained the 71-byte
linked comparison with the original ESI member input.

## 2026-11-17 - simscheduler.dll linked constructor cleanup matches

Verified three complete unwind action bodies and their cleanup
destinations. Retained two further pool wrapper builds as diagnostics.

## 2026-11-17 - Remove the fixed matching trial limit

Superseded the November 15 three-approach limit. Further matching trials
are guided by the evidence and the remaining plausible approaches.

## 2026-11-17 - simscheduler.dll linked EH handler match

Verified the complete constructor handler body and its three address
operands. Retained the improved pool deleting wrapper diagnostic and
published both original and rebuilt listings.

## 2026-11-17 - Document source identity and C++ class reconstruction

Added source conventions for original entry addresses and evidence-based
C++ class models. The dashboard now shows the original entry, Ghidra name
and reconstructed or build symbols together and accepts those names in search.

## 2026-11-17 - simscheduler.dll exact context time getter

Reconstructed the output pointer return and verified the 20-byte free
and virtual getter bodies. Published the corrected slot signature,
source links, and both disassembly listings in the dashboard.

## 2026-11-17 - simscheduler.dll internal constructor call

Verified a direct call to the internal 274-byte VC8 constructor with
the original stack argument placement. Retained export, scalar state
and fast FP trials and published the internal body in the dashboard.

## 2026-11-17 - simscheduler.dll constructor source mapping and argument slot

Added address comments and symbol-based dashboard links for the context
constructor, factory and time helper. Compared two linked experiments
and located the EBP+8 store in the inlined version.

## 2026-11-17 - simscheduler.dll constructor scheduling experiments

Retained source-order and size-optimization build evidence and recorded
the two remaining constructor ranges outside the verified spans.

## 2026-11-17 - simscheduler.dll constructor ownership and store order

Added opt-in linked VC8 constructor experiments for borrowed storage,
early name initialization and pointer-first container initialization.
Verified unwind state maps and cleanup targets, retained four builds,
and published the 104-byte matched block in the dashboard diagnostic.

## 2026-11-17 - simscheduler.dll native context class

Added a VC8 build profile with native virtual methods and pool deletion,
verified the 21-slot context table and compared each virtual body.
Retained the constructor diagnostic, original listings and dashboard evidence.

## 2026-11-16 - simscheduler.dll native virtual class probe

Built and checked compiler-generated VTable relocations and the
reference-release dispatch against the original ABI.

## 2026-11-16 - simscheduler.dll context class layout

Retained original name initialization, VTable and dispatch ABI evidence.

## 2026-11-16 - simscheduler.dll constructor LTCG comparison

Added the constructor-only VC8 LTCG probe and target checks.
Verified the custom time helper ABI and retained the 276-byte
frame-preserving constructor diagnostic. Published both linked listings.

## 2026-11-16 - simscheduler.dll context initialization

Reconstructed the context constructor's partial flag resets,
pre-copy name write and bitwise time copies. Captured both VC8
trials and published the 277-byte diagnostic in the dashboard.

## 2026-11-16 - simscheduler.dll registry cleanup

Added the lookup's published-reference guard, verified lookup and
removal unwind action targets, and completed the dashboard's
individual comparison coverage of the 19 original cleanup actions.

## 2026-11-16 - simscheduler.dll context constructor cleanup

Verified the constructor member cleanup order and the additional
placement-delete action. Published three diagnostic comparisons.

## 2026-11-16 - simscheduler.dll context destructor cleanup

Verified the destructor unwind order, state writes, member offsets
and cleanup targets. Added all three diagnostic listings.

## 2026-11-16 - simscheduler.dll fallback context cleanup

Moved temporary reference adoption after successful lookup and
factory returns. Verified the three native unwind actions and
published diagnostic comparisons.

## 2026-11-16 - simscheduler.dll context factory cleanup

Reconstructed all three factory cleanup actions in C++ scopes and
verified the emitted VC8 unwind transitions and action targets.
Retained diagnostic instruction comparisons and scope evidence.

## 2026-11-16 - simscheduler.dll pool initialization guard cleanup

Replaced catch-and-rethrow guard resets with a scope guard, verified
both VC8 unwind maps, and added the diagnostic action listings.

## 2026-11-16 - simscheduler.dll pool destructor cleanup

Verified the destructor unwind state, registration action, and both
REL32 targets. Recorded the MOV/LEA diagnostic in the dashboard.

## 2026-11-16 - simscheduler.dll pool constructor cleanup

Verified the extra placement-delete state and recorded the eight-byte
registration cleanup match for both pool constructors.

## 2026-11-16 - simscheduler.dll inventory follow-up

Built the two cleanup probes, added their diagnostic comparisons,
retained generated unwind action listings and relocations, and
verified all 62 external function entries against the original IAT.

## 2026-11-16 - simscheduler.dll function coverage

Captured all 167 internal Ghidra entries and reconciled the total of
229 with the 62 external function objects. Documented the remaining two
cleanup helpers, and added five verified CRT import thunk comparisons.

## 2026-11-16 - simscheduler.dll C++ EH block

Exported the original EH block, verified its parent and unwind-map
references, and added retained COFF comparisons for ten handler
bodies to the dashboard.

## 2026-11-16 - simscheduler.dll CRT conversion and import thunks

Added the RTM conversion link profile, exported the four original
listings, and recorded their normalized linked matches in the dashboard.

## 2026-11-16 - simscheduler.dll GS failure path

Exported the cookie checker, failure reporter, and debugger hook thunk
from Ghidra. Extended the linked startup comparator and dashboard with
three normalized matches across the retained SP1 and RTM builds.

## 2026-11-16 - simscheduler.dll EH4 handler

Verified the complete linked EH4 wrapper and its three address fields.

## 2026-11-16 - simscheduler.dll CRT import thunks

Verified four complete CRT import thunks and their IAT identities.

## 2026-11-16 - simscheduler.dll CRT PE section checks

Verified three linked PE section helpers and the section check EH4 scope
table.

## 2026-11-16 - simscheduler.dll RTC callbacks

Verified both 36-byte linked RTC loops and their empty sentinel ranges.

## 2026-11-16 - simscheduler.dll TypeInfo cleanup

Verified the linked TypeInfo cleanup wrapper at FUN_20cc6a89.

## 2026-11-16 - simscheduler.dll CRT exit registration

Verified three linked RTM CRT exit registration bodies and the EH4
unlock pointer. See [CRT exit registration](simscheduler.dll/simscheduler-crt-onexit-area.md).

## 2026-11-16 - simscheduler.dll CRT SSE2 detection

Extended the retained RTM-linked startup comparison to the two SSE2
helpers and their EH4 scope. See [CRT SSE2](simscheduler.dll/simscheduler-crt-sse2-area.md).

## 2026-11-16 - simscheduler.dll DLL startup and CRT dispatch

Reconstructed the raw loader hook and DllMain, then compared the PE
entry and CRT startup using installed SP1 and pinned RTM libraries.
Verified ten linked startup bodies and the EH4 scope table.
See [DLL startup](simscheduler.dll/simscheduler-dll-startup-area.md).

## 2026-11-16 - simscheduler.dll shared pool lock

Reconstructed packed-state pool lock transitions, lifecycle and spin
backoff. Retained five diagnostic comparisons and their original
listings. See [pool lock](simscheduler.dll/simscheduler-pool-lock-area.md).

## 2026-11-16 - simscheduler.dll shared pool free list

Reconstructed control-record lifecycle, batch allocation, push and pop
with supported VC8 atomics. Retained eleven native diagnostics and
their original listings. See [free list](simscheduler.dll/simscheduler-free-list-area.md).

## 2026-11-16 - simscheduler.dll pool metrics and remaining VTable operations

Reconstructed empty-block trimming, pool metrics and deleting wrappers.
Verified eleven native matches and retained five diagnostics.
See [pool metrics](simscheduler.dll/simscheduler-pool-metrics-area.md).

## 2026-11-16 - simscheduler.dll pool initialization and teardown

Reconstructed CRT entry wrappers, guarded construction, block drains,
teardown and registration unlinking for both pools. Verified five
native COFF matches. See [pool lifecycle](simscheduler.dll/simscheduler-pool-lifecycle-area.md).

## 2026-11-16 - simscheduler.dll scheduler object lifecycle

Reconstructed initialization, CRT exit registration and name index
teardown. Matched the initializer and startup entry in native COFF
and retained diagnostics for both cleanup bodies.
See [scheduler object lifecycle](simscheduler.dll/simscheduler-object-lifecycle-area.md).

## 2026-11-16 - simscheduler.dll context reset

Reconstructed the context reset and compared three native builds.
Verified its VTable slot and its call to the existing entry-pool return
helper. See [context reset](simscheduler.dll/simscheduler-context-reset-area.md).

## 2026-11-16 - simscheduler.dll scheduler object accessors

Reconstructed eight scheduler object VTable targets and matched six
native COFF bodies. Retained both setter comparisons and original listings.
See [scheduler object accessors](simscheduler.dll/simscheduler-object-accessors-area.md).

## 2026-11-16 - simscheduler.dll context accessors and mode setters

Reconstructed fifteen context VTable entries and their time propagation
helper. Matched thirteen native COFF bodies and retained three diagnostic
comparisons with their original listings.
See [context accessors](simscheduler.dll/simscheduler-context-accessors-area.md).

## 2026-11-16 - simscheduler.dll SimConnectFallback context

Reconstructed the transition routine and verified the context table
slots it uses. Retained three COFF comparisons and updated the wrapper
comparison for the local callee definition.
See [fallback context](simscheduler.dll/simscheduler-fallback-context-area.md).

## 2026-11-16 - simscheduler.dll named context registry

Reconstructed name lookup, insertion, removal and storage helpers.
Retained original listings and three VC8 runs with per-function native
COFF comparisons. See [named context registry](simscheduler.dll/simscheduler-name-registry-area.md).

## 2026-11-16 - simscheduler.dll context factory and storage

Modeled the named factory and context pool, reused the timed-entry
pool code, and matched both reference-count functions.
See [context factory and storage](simscheduler.dll/simscheduler-context-factory-storage-area.md).

## 2026-11-16 - simscheduler.dll context lifecycle

Reconstructed construction, destruction and cleanup helpers. Compared
the member constructor and placement factory and verified the original
deletion slot. See [context lifecycle](simscheduler.dll/simscheduler-context-lifecycle-area.md).

## 2026-11-16 - simscheduler.dll entry-container insertion

Reconstructed and built both insertion paths, index slot helpers,
and capacity changes. Preserved their native and linked comparisons.
See [entry insertion](simscheduler.dll/simscheduler-entry-insertion-area.md).

## 2026-11-16 - simscheduler.dll entry pool

Reconstructed allocation, return and block initialization. Verified
the diagnostic ranges and stored both listings.
See [entry pool](simscheduler.dll/simscheduler-entry-pool-area.md).

## 2026-11-16 - simscheduler.dll immediate registration

Reconstructed immediate add/remove and three lookup/removal helpers,
verified the context VTable and added five comparison listing pairs.
See [immediate registration](simscheduler.dll/simscheduler-immediate-registration-area.md).

## 2026-11-16 - simscheduler.dll deferred registration

Reconstructed enqueue/cancel and storage resizing, checked the
original allocator and transfer targets, and added paired diagnostics.
See [deferred registration](simscheduler.dll/simscheduler-deferred-registration-area.md).

## 2026-11-16 - simscheduler.dll context time-update area

Reconstructed context advancement and substep calculation, verified
their linked call targets and marked the registration boundary.
See [context time-update area](simscheduler.dll/simscheduler-context-time-update-area.md).

## 2026-11-16 - simscheduler.dll internal time update

Reconstructed the time-advance path, verified its COFF references,
refined the observer pointer model and retained both listings.
See [internal time update](simscheduler.dll/simscheduler-internal-time-update.md).

## 2026-11-16 - simscheduler.dll group 0x20 callback

Matched the C++ callback at 0x20cc13c0, checked both imports and
the original time-update slot, and marked the area boundary.
See [group 0x20 callback](simscheduler.dll/simscheduler-group20-callback-match.md).

## 2026-11-16 - simscheduler.dll Flight load handler

Compared three linked C++ variants for FUN_20cc4790 and recorded
the best ABI shape and both listings.
See [Flight load handler](simscheduler.dll/simscheduler-flight-load-match.md).

## 2026-11-16 - simscheduler.dll Flight save handler

Matched the linked C++ body of FUN_20cc48d0 with the original Flight
import and added both listings to the dashboard.
See [Flight save handler](simscheduler.dll/simscheduler-flight-save-match.md).

## 2026-11-16 - simscheduler.dll time propagation

Compared three linked C++ variants for FUN_20cc1a40 and added the
closest body and its original listing to the dashboard.
See [time propagation](simscheduler.dll/simscheduler-time-propagation-match.md).

## 2026-11-16 - simscheduler.dll time-state handler

Compared three linked C++ handler variants, verified the time propagation
call target, and retained the paired listings for the closest instruction
shape. See [time-state handler](simscheduler.dll/simscheduler-time-state-match.md).

## 2026-11-16 - simscheduler.dll dispatcher LTCG probe

Modeled the three handlers and time propagation helper in C++ and
verified the linked dispatcher and its three call destinations. The
dashboard now displays the 81-byte linked diagnostic.
See [event dispatcher](simscheduler.dll/simscheduler-event-dispatch-match.md).

## 2026-11-16 - simscheduler.dll dispatcher in C++

Replaced inline assembly with a C++ switch and retained both VC8
diagnostics and the archive of the assembly match.
See [event dispatcher](simscheduler.dll/simscheduler-event-dispatch-match.md).

## 2026-11-16 - Document source reconstruction conventions

Documented idiomatic C and C++ reconstruction with structured control
flow and evidence-based types. Matching trials use plausible source forms
and supported build options, with the existing per-target stopping rule.
Added guidance on grouping related functions and helpers into translation
units along evidenced subsystem or ownership boundaries.

## 2026-11-16 - simscheduler.dll event dispatcher

Matched FUN_20cc1400, verified its three handler call shapes, and
added both listings to the dashboard.
See [event dispatcher](simscheduler.dll/simscheduler-event-dispatch-match.md).

## 2026-11-15 - simscheduler.dll callback unregistration

Reconstructed and matched FUN_20cc1490 and verified representative
node VTable targets. Added paired listings and list evidence to the
dashboard. See [callback unregistration](simscheduler.dll/simscheduler-callback-unregistration-match.md).

## 2026-11-15 - simscheduler.dll callback registration wrapper

Reconstructed and matched FUN_20cc1460. Verified its API import and
local callback references and added both listings to the dashboard.
See [callback registration match](simscheduler.dll/simscheduler-callback-registration-match.md).

## 2026-11-15 - simscheduler.dll SimConnectFallback wrapper

Reconstructed and matched FUN_20cc1500, verified the original direct
call and two data references, and added both listings to the dashboard.
See [SimConnectFallback wrapper](simscheduler.dll/simscheduler-simconnect-fallback-match.md).

## 2026-11-15 - simscheduler.dll SimTime wrapper

Reconstructed and matched FUN_20cc1530, verified the original slot and
target ABI, and added both listings to the dashboard.
See [SimTime wrapper](simscheduler.dll/simscheduler-sim-time-match.md).

## 2026-11-15 - simscheduler.dll SimSpeed setter wrapper

Reconstructed and matched FUN_20cc1580, verified the original slot and
target ABI, and added both listings to the dashboard.
See [SimSpeed setter wrapper](simscheduler.dll/simscheduler-set-sim-speed-match.md).

## 2026-11-15 - simscheduler.dll SimSpeed wrapper

Reconstructed and matched FUN_20cc15b0, verified the slot and target
signature, and added the paired listings and slot evidence to the
dashboard. See [SimSpeed wrapper](simscheduler.dll/simscheduler-sim-speed-match.md).

## 2026-11-15 - simscheduler.dll wrapper at 0x20cc1560

Reconstructed and matched the wrapper, verified the original slot and
target signature, and added the paired listings and table evidence to
the dashboard. See [wrapper match](simscheduler.dll/simscheduler-field-min-match.md).

## 2026-11-15 - simscheduler.dll setter at 0x20cc14e0

Reconstructed the setter, verified its complete COFF body alongside the
getter regression and added the function listings and global reference
to the dashboard. See [setter match](simscheduler.dll/simscheduler-dispatch-key-match.md).

## 2026-11-15 - simscheduler.dll object getter

Added the C++ getter, shared-queue build profile, COFF comparator and
dashboard listings. Verified the complete body of FUN_20cc14d0 with
one validated relocation. See [object getter match](simscheduler.dll/simscheduler-object-getter-match.md).

## 2026-11-15 - simscheduler.dll first pass

Recorded the ordinal exports, callback and persistence anchors, and
original bytes and relocations for the first matching targets.
See [first pass](simscheduler.dll/simscheduler-first-pass.md).

## 2026-11-15 - G2D DX10 depth resource allocation

Connected resource allocation and both pool index boundaries. Verified
the compiled call targets and retained the original and rebuilt listings.
See [depth allocation](g2d.dll/g2d-dxgi-depth-allocate.md).

## 2026-11-15 - G2D DX10 depth resource return

Connected target finish to pool return and verified the COFF call
relocations against original targets. Both dashboard entries retain
original and rebuilt listings. See [depth return](g2d.dll/g2d-dxgi-depth-return.md).

## 2026-11-15 - Define the matching stopping rule

Documented exact or independently validated normalized equality as sufficient
for the stated scope and capped matching at three distinct approaches per target.

## 2026-11-15 - G2D DX10 quad constant byte-width match

Connected the local width getter and verified its complete COFF body
against 2067e570. Retained the differing C++ trial, the matching inline
assembly trial and both dashboard listings.
See [quad size match](g2d.dll/g2d-dxgi-quad-size-match.md).

## 2026-11-15 - Restore dashboard views and link reconstructed source

Stored the active filters, selection and listing mode in the dashboard URL.
Added current-source references for uniquely resolved definitions and an
explicit source reference in the per-module function schema.

## 2026-11-15 - G2D DX10 clear-target depth view

Connected back-buffer description, pooled depth resource lookup and
depth-stencil view setup. VC8 x86 build passed and the dashboard
retains both listings. See [clear-target depth view](g2d.dll/g2d-dxgi-clear-target-depth.md).

## 2026-11-15 - G2D DX10 target binding

Connected registered target binding and corrected the presentation-window
lookup interpretation. The VC8 x86 build passed. Both dashboard entries
retain original and rebuilt listings.
See [target binding](g2d.dll/g2d-dxgi-clear-target-bind.md).

## 2026-11-15 - API registry body trials

Reduced the linked registration diagnostic to four differing bytes
after target mapping. Retained all trials and updated the dashboard.

## 2026-11-15 - API registry internal ABI probe

Reproduced the ESI owner argument via LTCG and verified both complete
linked caller bodies. Preserved three dashboard listing pairs.

## 2026-11-15 - API registry linked probe

Linked the registration model against the matched heap helpers and
verified its inlined failure path and direct reallocation call.

## 2026-11-15 - API reallocator linked match

Adopted the single volatile flag read, verified the full linked body
and added both dashboard listings.

## 2026-11-15 - API heap initializer linked match

Adopted the tested-zero flag snapshot and added the 144-byte initializer
comparison with a rejecting control and both dashboard listings.

## 2026-11-15 - API standalone allocator linked match

Added a complete 174-byte comparison and regression gate for
20135dd9. Retained the original and rebuilt listings in the dashboard.

## 2026-11-15 - API shared heap flag and complete linked caller matches

Adopted the local volatile flag read after both complete SP1 bodies
matched. Added the full linked gate with a rejecting barrier control
and updated both API dashboard comparison pairs.

## 2026-11-15 - API private-heap string duplicate

Moved the string duplicate to its own source and reused the common
allocator. Verified that VC8 inlines it with the same linked instruction
body as the expanded control after import mapping. Recorded the
shared original failure path and added both duplicate listings.

## 2026-11-15 - API copy trail pointer and capacity trials

Tested asymmetric pointer advances and direct capacity decrement.
The capacity parameter reproduces the original decrement encoding.
Retained all three comparisons and updated the dashboard pair.

## 2026-11-15 - API copy combined exits

Verified the 42-byte entry match for three staged SP1 variants.
Retained the combined exit comparison and updated both copy listings.

## 2026-11-15 - API copy shared zero-capacity return

Removed the separate zero-capacity return in the staged SP1 source.
Verified the resulting shared return and retained both previous local
instruction sequences. Updated the side-by-side API listing.

## 2026-11-15 - API and simpropace string copy

Scanned the original images for the API copy helper, recorded the
similar simpropace function and measured four SP1 prologue variants.
Updated the API dashboard with the parameter-cursor comparison.

## 2026-11-15 - API copy NUL exit trials

Compared separate and shared NUL exits for 20137e17. Retained both
linked diagnostics and recorded the exit and branch-order tradeoff.

## 2026-11-15 - API copy branch and return shape trials

Preserved both successful local changes in one SP1 probe. Added the
comparison and a side-by-side listing for the combined variant.

## 2026-11-15 - API ordinal 157 trail advance probe

Tested early pointer advances in the DBCS branch and recorded the local
instruction tradeoff against the early-exit probe and original listing.

## 2026-11-15 - API ordinal 157 source and optimization sweep

Built three SP1 variants and retained their comparisons and the live
original listing. The early-exit variant improves local branch and
count ordering. /Og- changes the import jump to a call but introduces
stack locals and a shared epilogue.

## 2026-11-15 - API public string export build comparison

Added linked SP1 profiles for ordinals 157 and 218. Verified identical
output across profiles and confirmed the linked append body match
with a validated target. Added both listing pairs to the dashboard.

## 2026-11-15 - API string export context

Verified the copy helper's public ordinal and adjacent string exports.
Retained the export table evidence, cross-references and listings.

## 2026-11-15 - API copy pointer advance experiment

Split the pointer increments in a staged SP1 source variant. Retained
evidence that the compiler still emits them before the lead-byte branch.

## 2026-11-15 - API copy helper shared return experiment

Added the staged shared return variant and retained SP1 evidence that
the copy body remains byte-identical to the direct integer-test build.

## 2026-11-15 - API copy lead-byte type experiment

Added a staged int variant and retained the SP1 comparison. It removes
SETNE but differs in instruction order. The RTM request was rejected by
the version gate after the worker reported SP1.

## 2026-11-15 - API VC8 RTM static CRT comparison

Replayed the static CRT probe on the VC8 RTM guest and retained the
comparison against the original helper and the SP1 probe.

## 2026-11-15 - API static CRT copy comparison

Linked and compared the two VC8 libcmt multibyte copy candidates.
Corrected the original copy helper extent and updated the dashboard
listing. Retained both comparison reports in artifacts/api.dll.

## 2026-11-15 - API ordinal 206 call provenance

Retained live helper listings, PE import identities and the original
and rebuilt cookie-helper bytes. Recorded verified library targets and
the open provenance of the multibyte copy helper.

## 2026-11-15 - Organize artifacts by binary

Moved analysis exports, comparison inputs and packed Ghidra backups under
artifacts/<binary-name>/. Shared toolchain and multi-module evidence moved
to artifacts/shared/. Updated active path references, inventory discovery
and dashboard inputs. Reverified EXE baseline 41 and API flow evidence.

## 2026-11-15 - Move EXE build history into its module

Moved the EXE runs and archive to vm-share/modules/fsx and put the shared
controller lock at vm-share/controller.lock. Updated the host controllers,
comparators, baseline ledger and dashboard. Removed 2,673 completed EXE
queue payload files after verifying their retained run copies by SHA-256.
The module queue audit records the retained paths and hashes.

## 2026-11-15 - Consolidate API build job evidence

Verified all 28,414 files from 199 API job snapshots against retained
run copies or newly transferred files by SHA-256. Removed the redundant
snapshot directory and updated run metadata to point to the retained
evidence. The API dispatcher now writes job evidence into its run directory
and continues to use the shared queue for VM transport.

## 2026-11-15 - API ordinal 206 regular source integration

Integrated the StrSafe suffix call and corrected the multibyte copy
implementation using the original listing. Verified the linked copy and
heap targets and retained all linked and COFF regression gates.
Updated both dashboard entries. See [numbered names](api.dll/api-numbered-names.md).

## 2026-11-15 - API ordinal 206 automatic inlining

Added a linked C++ diagnostic and /GL- control. VC8 automatically inlines
the heap and append helpers. Verified the 60-byte formatting and length
range and retained the three linked StrSafe gates. Added the 206 listing pair.
See [numbered names](api.dll/api-numbered-names.md).

## 2026-11-15 - API numbered name C++ matches

Adopted the SDK StrSafe call in both numbered-name wrappers. Verified
their linked VC8 /LTCG bodies and the shared formatter over 172 bytes
with five address mappings. Ran the /GL- control and all twenty COFF
gates. Added linked comparison evidence and all three listing pairs.
See [numbered names](api.dll/api-numbered-names.md).

## 2026-11-15 - API ordinal 207 register ABI probe

Added an opt-in VC8 wrapper probe and comparator. The full 38-byte wrapper
agrees after two relocation mappings, and the formatter remains unresolved
in the probe. Twenty existing COFF gates pass. Added both dashboard listings.
See [numbered names](api.dll/api-numbered-names.md).

Verified the original API formatter against the original EXE and replayed
the archived VC8 StrSafe formatter comparison. The two original bodies
differ only in the verified CRT import operand. Recorded the source
provenance and the linked C++ candidate in the numbered-name notes.

## 2026-11-15 - G2D DX10 clear-target registry

Reconstructed registry lookup, active target selection and registry reset.
The VC8 x86 build passed and the dashboard retains both listings.
See [clear-target registry](g2d.dll/g2d-dxgi-clear-target-registry.md).

## 2026-11-15 - G2D DX10 clear entry points

Connected the viewport clear and both renderer clears to the local target
selector. The VC8 x86 build passed and the dashboard retains both listings
for all three. See [clear entry points](g2d.dll/g2d-dxgi-clear-entrypoints.md).

## 2026-11-15 - G2D DX10 compositor clear

Connected the alternate compositor clear and verified the profiling
import identities. The VC8 x86 build passed. The dashboard retains
both listings. See [compositor clear](g2d.dll/g2d-dxgi-compositor-clear.md).

## 2026-11-15 - G2D DX10 target clear dispatch

Connected the target clear selector and scissored quad branch.
The VC8 x86 object build passed. The dashboard retains both listings.
See [target clear](g2d.dll/g2d-dxgi-quad-clear.md).

## 2026-11-15 - G2D DX10 quad render-state cache

Connected per-index state preparation, Direct3D creation and application
to the quad draw path. The VC8 x86 object build passed. All six compiled
counterparts have original and rebuilt listings in the dashboard.
See [quad render state](g2d.dll/g2d-dxgi-quad-draw-state.md).

## 2026-11-15 - G2D DX10 quad drawing

Reconstructed the quad draw path, changed-constant upload and
pipeline state clearing. The VC8 x86 object build passed and all
three original and rebuilt listings were entered in the dashboard.
See [quad drawing](g2d.dll/g2d-dxgi-quad-draw.md).

## 2026-11-15 - GPS legacy lifecycle

Connected the 0x36/0x37 callback, legacy cleanup and data initializer.
Checked 35 callback relocations, the 147-byte reset and publish block,
and the initializer's EAX adapter. Replayed prior GPS gates.
See [GPS legacy lifecycle](gps.dll/gps-legacy-lifecycle.md).

## 2026-11-15 - GPS provider and legacy storage cleanup

Connected provider destruction, buffer cleanup and entry release.
Recorded a normalized 78-byte match, the destructor unwind
diagnostic and the legacy ESI adapter. Replayed prior GPS gates.
See [GPS provider cleanup](gps.dll/gps-provider-cleanup.md).

## 2026-11-15 - Add dashboard status chart and disassembly alignment

Added a module status distribution and an instruction comparison table
with an option to switch back to raw listings. The table separates raw
equality, validated relocation fields, differences and one-sided rows.

## 2026-11-15 - Add dashboard views for global data and types

Imported archived API global references and a stored VTable type, plus
EXE data and VTable comparisons. Added support for module data and type
exports and documented their use in the reconstruction workflow.

## 2026-11-15 - GPS provider lifetime

Reconstructed ordinal 1 and its Release target. Verified 13 COFF
relocations and documented the remaining code differences and the
legacy callback boundaries. Replayed the ordinal 2 match.
Fixed dashboard import of wrapped VC8 dumpbin byte lines so long
instructions retain their complete bytes.
See [GPS provider lifetime](gps.dll/gps-provider-lifetime.md).

## 2026-11-15 - GPS provider registration

Added GPS to the dashboard and shared XP build workflow. Recorded both
original export listings and reconstructed ordinal 2 as a 42-byte
target-validated COFF match with five relocations.
See [GPS provider registration](gps.dll/gps-provider-registration.md).

## 2026-11-15 - G2D DX10 window shared-state reset

Connected the holder reset and both resource entry cleanup routines.
The VC8 x86 object build passed. All three routines have original and
rebuilt listings in the dashboard.
See [window state reset](g2d.dll/g2d-dxgi-window-state-reset.md).

## 2026-11-15 - G2D DX10 quad constant teardown

Connected holder and inline quad constant destruction. The VC8 x86
object build passed and four functions were mapped in the dashboard.
See [quad teardown](g2d.dll/g2d-dxgi-quad-teardown.md).

## 2026-11-15 - Show original G2D quad constant listings

Added the original constructor and GPU buffer creation listings to the
dashboard with bytes from the pinned g2d.dll PE.

## 2026-11-15 - G2D DX10 quad constant buffer

Connected the quad constant constructor and D3D10 buffer creation.
The VC8 x86 object build passed and both functions were mapped in
the dashboard. See [quad constant](g2d.dll/g2d-dxgi-quad-constant.md).

## 2026-11-14 - Add local decompilation dashboard

Added an offline module and function browser backed by retained inventories,
topic notes, baseline 41 comparisons and API COFF source-pass reports.
Documented the module data format for future results.

## 2026-11-14 - G2D DX10 shader binding lookup

Connected the reflected lookup and size check for GsQuadCb to the
window shared-state initializer. The VC8 x86 object build passed.
See [shader lookup](g2d.dll/g2d-dxgi-shader-lookup.md).

## 2026-11-14 - G2D DX10 reflected shader bindings

Connected sorted insertion, growth and cleanup for the reflected
shader bindings. The VC8 x86 object build passed without warnings.
See [shader bindings](g2d.dll/g2d-dxgi-shader-bindings.md).

## 2026-11-14 - G2D DX10 shader lifetime

Connected shader construction, owned macro copying and cleanup for
unpublished shaders. The VC8 x86 object build passed without warnings.
See [shader lifetime](g2d.dll/g2d-dxgi-shader-lifetime.md).

## 2026-11-14 - Restructure project instructions

Reorganized AGENTS.md into general instructions, module-scoped notes,
Ghidra MCP and the host/XP worker build and verification workflow.
Generalized the notes layout to all modules and documented how to name
new single-module and cross-module topic files.

## 2026-11-14 - Organize reverse-engineering notes by binary

Moved technical notes into fsx.exe/, api.dll/, g2d.dll/ and shared/.
Updated documentation references and the output path of the
coverage script. Removed the obsolete November 1 cleanup report, audit logs and one-time cleanup script.
Moved the three chat handoffs into handoffs/ and updated their references.

## 2026-11-14 - G2D Direct3D 10 shader creation

Connected the verified vertex, geometry and pixel shader device calls
to the PostProcess catalogue. The VC8 x86 object build passed without
warnings. See g2d-dxgi-shader-creation.md.

## 2026-11-14 - G2D shader compilation and reflection

Connected HLSL compilation and resource reflection to the PostProcess
catalogue. The VC8 x86 object build passed without warnings.
See g2d-dxgi-shader-loading.md.

## 2026-11-14 - G2D PostProcess shader catalogue

Connected the twelve shader records and ordered initialization to
the DXGI holder. The fresh VC8 x86 object build passed without warnings. See g2d-dxgi-shader-catalogue.md.

## 2026-11-12 — G2D DXGI resource holders

Connected the three holder factories to the primary DXGI device
path and reconstructed window holder construction, reference-counted
publication and quad-shader selection. The isolated VC8 build passed.
See g2d-dxgi-window-resources.md.

## 2026-11-12 — G2D DXGI device and output setup

Connected descriptor preparation, device creation and sharing,
holder reference counts and output dimensions to the DXGI window
initializer. The isolated VC8 build passed.
See g2d-dxgi-window-device.md.

## 2026-11-12 — G2D per-window initialization

Connected D3D9 and DXGI initialization to both controller loops.
Reconstructed adapter checks, return status and failure routing.
The isolated VC8 build passed. See g2d-window-initialization.md.

## 2026-11-12 — G2D window construction and publication

Connected both backend loops to the shared and backend-specific window
constructors and controller-list append. The isolated VC8 object
compiled without warnings. See g2d-window-construction.md.

## 2026-11-12 — G2D backend initialization

Added the four D3D9 and DXGI controller slot targets, including
enumeration and shared initialization finalization. Built the separate
VC8 x86 G2D object. See g2d-controller-backends.md.

## 2026-11-12 — G2D shared controller transition

Added a separate G2D source unit for the shared controller slots,
window refresh order and ordinal 65 fatal report. Built a separate
VC8 x86 COFF object. See g2d-controller-transition.md.

## 2026-11-12 — FS98MAIN graphics transition and wait cursor

Connected 0x46f and 0x470 to the main-window procedure. Added temporary
window reparenting, refresh collection and modal suspension around the
G2D transition. Twenty comparators pass. See api-main-window-graphics-transition.md.

## 2026-11-12 — API main-window modal controller

Connected interactive WinTrust verification to the reconstructed
modal entry and exit. Added the focus, surface clear and close
override bodies and clarified shared controller field names.
Twenty comparators pass. See api-main-window-modal.md.

## 2026-11-12 — API WinTrust provider lifecycle and classification

Connected the trust decision to dynamic WinTrust and Crypt32 binding,
signature verification, signer classification and the shared group-6
release callback. Implemented the full release body. Twenty comparators
pass. See api-addon-trust-provider.md.

## 2026-11-12 — API add-on trust decision and profile integration

Added the ordinal 305 decision, ordinary key construction and
trust clearing. Connected both XML readers and reused the existing
profile parser and writer for Trusted. Twenty comparators pass.
See api-addon-trust.md.

## 2026-11-12 — API add-on startup and module lifetime

Added source for API ordinal 322, including Trusted-section reset,
listener configuration, both XML loaders and the reverse-order module
stop body. Verified the original call to the stop body from the
shutdown prefix. Twenty comparators pass. See api-addon-startup.md.

## 2026-11-12 — API service listener configuration

Connected configured and default listener selection to service startup,
including per-entry overrides and fallback suppression.
Twenty comparators pass. See api-service-listener-configuration.md.

## 2026-11-12 — API application service registration

Connected protocol-client lookup, removal and shutdown to the shared
registry. Implemented the service startup entry and its callback
registration. Twenty comparators pass.
See api-service-application-registration.md.

## 2026-11-12 — API service client connection

Connected reset and close to queue cleanup, connection generation,
status polling and transport restart. Twenty comparators pass.
See api-service-client-connection.md.

## 2026-11-12 — API service transport tree

Connected listener registration to signed-key insertion and red-black
balancing. Twenty comparators pass. See api-service-transport-tree.md.

## 2026-11-11 — API service listener client construction

Connected client construction, pipe-name ownership, derived dispatch and
cleanup. Resolved the socket setup address-family argument.
Twenty comparators pass. See api-service-client-construction.md.

## 2026-11-11 — API service transport listeners

Connected socket and named-pipe startup, listener groups, and local
port publication to the transport lifecycle. Twenty comparators pass.
See api-service-transport-listeners.md.

## 2026-11-11 — API service transport lifecycle

Connected transport tree and client-group ownership to service
window startup and shutdown. Restored the configuration argument.
Twenty comparators pass. See api-service-transport-lifecycle.md.

## 2026-11-11 — API outbound service packets

Connected transport lookup, direct callback delivery and queued
write submission. Twenty comparators pass.
See api-service-packet-send.md.

## 2026-11-11 — API service client open

Connected the packet dispatcher to client open and exception
response construction. Twenty comparators pass.
See api-service-packet-open.md.

## 2026-11-11 — API service packet routing

Reconstructed the packet routing entry and 72 command cases
with their size checks and handler boundaries.
Twenty comparators pass. See api-service-packet-dispatch.md.

## 2026-11-11 — API receive message handoff

Reconstructed both receive targets and reused the service
queue builder for the derived path. Twenty comparators pass.

## 2026-11-11 — API service client buffering

Connected receive framing, read submission and queued write
continuation to the I/O completion path. Twenty comparators pass.

## 2026-11-11 — API service client completion

Connected I/O completion, error dispatch and final reference
cleanup. Twenty comparators pass.
See api-service-client-completion.md.

## 2026-11-11 — API service client transport

Connected index maintenance to socket and pipe accept and
teardown, including the pipe wait callback. Twenty comparators pass.
See api-service-client-transport.md.

## 2026-11-11 — API service client delivery

Connected the registration callback to queue delivery and
reconstructed index storage, insertion and removal. Twenty
comparators pass. See api-service-client-delivery.md.

## 2026-11-11 — API service client registration

Connected dispatch export storage, client lookup and registration
writes, and the heap callbacks. Twenty comparators pass.
See api-service-shutdown.md.

## 2026-11-11 — API main-window service callback

Connected service queue draining, dispatch enablement and the
module and client registration routes. Twenty comparators pass.
See api-service-shutdown.md.

## 2026-11-11 — API service window initialization

Reconstructed queue setup, message-window registration,
enqueue routing and service-window queue dispatch.
Twenty comparators pass. See api-service-shutdown.md.

## 2026-11-11 — API service window shutdown

Connected message-window teardown, callback removal and both
queue drains. Twenty comparators pass. See api-service-shutdown.md.

## 2026-11-11 — API callback dispatch

Connected callback dispatch and deferred removal, sharing the entry
erase operation with immediate removal. Twenty comparators pass.
See api-service-shutdown.md.

## 2026-11-11 — API service shutdown

Connected the service teardown sequence and reconstructed callback
removal, including pending removal during dispatch. Twenty
comparators pass. See api-service-shutdown.md.

## 2026-11-11 — API main window shutdown

Connected the keyboard hook, class, module and font teardown
to the existing shutdown body. Twenty comparators pass.
See api-main-window.md.

## 2026-11-11 — API main window completion

Reconstructed session registration, common controls call and
keyboard hook installation. Connected the existing close callback
to the resolved WTS unregister entry. Twenty comparators pass.

## 2026-11-11 — API FS98MAIN creation

Connected SIMPROP startup, main HWND publication, the creation
error path and the graphics startup gate. Twenty comparators pass.
See api-main-window.md.

## 2026-11-11 — API startup monitor geometry

Connected the geometry resolver and error handler in the startup
loop and implemented message formatting and display applet launch.
Twenty comparators pass. See api-window-geometry.md.

## 2026-11-11 — API MAIN startup profile

Reconstructed the profile reads, debugger-sensitive default and
language loader result gate. Twenty comparators pass.
See api-main-window.md.

## 2026-11-11 — API startup window classes

Connected the five class registrations in native order with their
two error reports. Twenty comparators pass. See api-main-window.md.

## 2026-11-11 — API FS98FLOAT

Reconstructed class registration and floating panel message routing,
including deferred move and size updates. Twenty comparators pass.
See api-panel-float-window.md.

## 2026-11-11 — API panel window record

Connected window link allocation, child and auxiliary creation,
clip registration and finalization to menu host initialization.
Twenty comparators pass. See api-panel-window-record.md.

## 2026-11-11 — API menu host initialization

Reconstructed both shared initialization targets and the focus
callback, including the canvas setup calls. Twenty comparators pass.
See api-menu-host-init.md.

## 2026-11-11 — API menu host teardown

Reconstructed shared interface lookup, HWND forwarding and release
targets, and the separate window and default host detach paths.
Twenty comparators pass. See api-menu-host-teardown.md.

## 2026-11-10 — API menu host creation

Connected both window host creation routes and reconstructed
the default host creation entry and both factory callbacks.
Twenty comparators pass. See api-menu-host-create.md.

## 2026-11-10 — API panel menu commands

Connected WM_COMMAND and the pointer route to menu construction
and display. Twenty existing comparators pass.
See api-panel-menu-command.md.

## 2026-11-10 — API panel menu

Reconstructed panel menu construction, item insertion and
WM_INITMENU state dispatch. Twenty comparators pass.
See api-panel-menu.md.

## 2026-11-10 — API panel menu host

Connected menu-interface lookup, root property attachment/removal
and panel handle release. Corrected the prior graphics-context
interpretation. Twenty comparators pass. See api-panel-menu-host.md.

## 2026-11-10 — API panel surface refresh

Connected the 0x467 handler, private bitmap recreation and shared
surface sizing and publication. Corrected WM_SIZE frame queries
to use the auxiliary HWND. Twenty existing comparators pass.
See api-panel-surface-refresh.md.

## 2026-11-10 — API panel geometry and shadow drag

Connected panel geometry messages, related-panel sizing, shadow
registration and the preview-to-commit path. Twenty existing
comparators pass. See api-panel-geometry-shadow.md.

## 2026-11-10 — API panel message routing

Connected the child and dialog window procedures, context lifecycle,
paint, cursor, bitmap and inline input routes. Twenty existing
comparators pass. See api-panel-message-routing.md.

## 2026-11-10 — API cursor events

Connected main and panel cursor handling, controller fallback, cursor
override and pointer query. Shared key sampling with timer input.
Twenty existing comparators pass. See api-cursor-events.md.

## 2026-11-10 — API panel hover text

Connected the hover text setter, shared refresh and text release.
Twenty existing comparators pass. See api-panel-hover-text.md.

## 2026-11-10 — API panel hit removal

Connected export 148, recursive owner release and tooltip destruction.
Twenty existing comparators pass. See api-panel-hit-removal.md.

## 2026-11-10 — API panel hit registration

Connected the four registration exports, tooltip creation and
recursive bounds construction. Twenty existing comparators pass.
See api-panel-hit-registration.md.

## 2026-11-10 — API cursor conversion and tooltip receiver

Connected the cursor conversion body and added tooltip creation.
Identified the receiver class and original caller. Twenty existing
comparators pass. See api-cursor-conversion.md.

## 2026-11-10 — API panel cursor and hover

Connected cursor selection, lazy cursor caching and hover messages to
the input dispatcher. Twenty existing comparators pass.
See api-panel-cursor-hover.md.

## 2026-11-10 — API panel hits and capture dispatch

Connected nested hit selection, wheel routing and the captured-input
callback path. Twenty existing comparators pass.
See api-panel-input-hits.md.

## 2026-11-10 — API window input dispatch

Added capture selection, hit dispatch, repeat checks and click
fallback. Twenty existing comparators pass.
See api-window-input-dispatch.md.

## 2026-11-10 — API capture and repeat lifecycle

Connected the capture and repeat start and release routines to the
0x465 input path. Twenty existing comparators pass.
See api-timer-interaction-lifecycle.md.

## 2026-11-09 — API timer window work

Connected the 0x465 worker, input checks and countdown callbacks.
Twenty existing comparators pass. See api-timer-window-work.md.

## 2026-11-09 — API startup timer

Modeled startup readiness and timer setup, the multimedia callback and
its window-thread route. Connected floating-window startup refresh.
Twenty existing comparators pass. See api-startup-timer.md.

## 2026-11-09 — API resource strings

Connected the resource-module registry and the two string loaders.
Reused callback-array traversal and removal. Twenty existing
comparators pass. See api-resource-strings.md.

## 2026-11-09 — API product text and shell target

Reconstructed the product text formatter and controller shell-launch
body. Connected the command prefilter and retained the original
instruction inventory. Twenty existing comparators pass.
See api-main-window-product-text.md.

## 2026-11-09 — API main-window commands

Connected WM_COMMAND and all identified prefilter cases, including
the default-size export and the 0x110a event translation. Twenty
existing comparators pass. See api-main-window-command.md.

## 2026-11-09 — API main-window close

Implemented WM_CLOSE, synchronous close and posted command wrappers.
Shared the quit flag and corrected controller initialization at +0x5c.
Twenty existing comparators pass. See api-main-window-close.md.

## 2026-11-09 — API FS98MAIN controller lifetime

Added class registration, controller construction and teardown, and the
window procedure with three sizing/paint routes. Verified the emitted
cleanup scopes and passed twenty existing comparators.
See api-main-window-controller.md.

## 2026-11-09 — API main-window sizing

Implemented main-window snapshot capture, paint and sizing exit,
including child notifications. Connected panel paint suppression to
the shared controller layout. Twenty existing comparators pass.
See api-main-window-sizing.md.

## 2026-11-09 — API panel WM_PAINT

Connected paint suppression and clip drawing to the bitmap and graphics
helpers. Traced both WM_PAINT dispatchers. Twenty gates pass with
coverage unchanged. See api-panel-paint.md.

## 2026-11-09 — API panel bitmap drawing

Connected full and partial bitmap drawing, graphics update bounds and
both message callers. Modeled the stack-overflow exit and verified its
COFF references. Twenty gates pass with coverage unchanged.
See api-panel-bitmap-draw.md.

## 2026-11-09 — API panel surfaces and memory DC

Connected the alternate panel transfer, shared memory DC creation,
surface registration and removal, and two StretchBlt helpers.
Twenty gates pass with coverage unchanged. See api-panel-surface-lifecycle.md.

## 2026-11-09 — API panel pixel transfer

Connected panel update preparation to rectangle clipping and pixel transfer.
Added palette conversion and shared DC acquire/release source. The new gate
verifies both DC bodies and adds 88 previously uncounted code bytes.
See api-panel-pixel-transfer.md.

## 2026-11-09 — API panel update regions

Added the panel region rebuild, shared second region storage, the direct
panel update caller and rectangle preparation. Modeled the allocation-only
stack-overflow handler and verified its COFF references. Nineteen
comparators pass with coverage unchanged. See api-panel-update-regions.md.

## 2026-11-09 — API window ordering and update regions

Added deferred window ordering, update rectangle union, group raise and
window-link teardown. Connected the shared region storage to its existing
last-group cleanup. The new gate accepts 106 complete bytes at 2011d315.
See api-window-region-order.md.

## 2026-11-09 — API clip regions and window groups

Connected the controller clip rebuild to the graphics index cache and
observed window groups. Added group registration, member insertion and
removal with final shared-region cleanup. Both builds pass 18 gates with
coverage unchanged. See api-window-clip-regions.md.

## 2026-11-09 — API controller mode and visibility

Added source definitions for the controller mode setter, clip-refresh
guard, visibility refresh and floating-window visitor. Retained virtual
dispatch and explicit graphics and clip-region boundaries. Eighteen
comparators pass with coverage unchanged. See api-window-controller-mode.md.

## 2026-11-09 — API dialog registry

Added dialog visibility and dispatch exports, shared callback-array
iteration with deferred removal, and the controller hide path. Extended
the pilot controller slot interface while retaining the idle slot.
Eighteen comparators pass with coverage unchanged. See api-dialog-registry.md.

## 2026-11-09 — API numbered names and heap release

Added numbered-name buffer and heap exports, reused the bounded formatter
for numbered-section counting, and connected the original heap release
helper to decoder and CAB image paths. The new gate accepts its complete
43-byte body. See api-numbered-names.md.

## 2026-11-09 — API configuration writes

Connected parser invalidation, profile flush and selected/default
configuration writes. Reused the profile retry and typed list assembly
across the two families. The new gate accepts 54 complete WORD/BYTE
wrapper bytes. See api-stream-parser-writes.md.

## 2026-11-09 — API configuration queries

Connected selected and default configuration queries to the stream parser.
Added section enumeration, numbered-section count, scalar/list conversion,
profile string expansion and quote collapse. Sixteen gates pass with
coverage unchanged. See api-stream-parser-queries.md.

## 2026-11-09 — API stream parser and parser cache

Added in-place tokenization, sorted section/key indices, parser acquisition
and the first configuration query callers. Refined parser teardown fields
and connected it to the now-defined cache state. Both builds pass 16
comparators with coverage unchanged. See api-stream-parser.md.

## 2026-11-09 — File-stream lifecycle and size consumers

Defined stream-cache storage, registration and lock cleanup. Connected the
shutdown continuation through parser, stream-cache, CAB/FDI and suffix
teardown. Added file create/copy and parser input/cleanup source and typed
CopyTo in the pilot stream tables. The new relocation gate accepts 37 code
bytes. See api-file-stream-lifecycle-consumers.md.

## 2026-11-09 — File stream decoder and cache

Implemented the decoder and seek/size helper and connected the independent
stream cache to export 85. Added cache lookup, remember, prune, link, detach
and drain source. Both native object runs pass all 15 existing gates with
coverage unchanged. See api-file-stream-decode-cache.md.

## 2026-11-09 — File stream selection and CAB fallback

Added the export-85 source body and connected the existing stream and CAB
paths. Traced the decoder and separate stream cache boundaries. Fifteen
checks pass on api-file-stream-open-20261109-001. See api-file-stream-open.md.

## 2026-11-09 — CAB extraction and FDI callbacks

Added the shared extraction dispatcher, notification and two stream consumers.
Connected the FDI open callback already referenced by Initialize and shared
the observed CAB header fields with the prefix loader. Fifteen checks pass
on api-cab-extraction-20261109-001. See api-cab-extraction.md.

## 2026-11-09 — CAB owner metadata and attribute fallback

Verified raw and wrapped Stat failure behavior. Pinned the timestamp offsets
in the source layout. Added entry lookup and connected the existing attribute
fallback calls, with a source-only rename of owner +0x120 to lastExactEntry.
Fifteen checks pass on api-cab-owner-metadata-20261109-001. See
api-cab-owner-metadata.md.

## 2026-11-09 — CAB buffer containers and registration

Connected CAB cache allocation and return to the live-address and cached-size
containers. Added cache initialization, teardown and explicit atexit
registration source. Accepted 17 relocation-validated bytes at 201ee724.
Fifteen checks pass on api-cab-buffer-containers-20261109-003. Owner reload
failure outputs were traced and documented. Ghidra inspection was read-only.
See api-cab-buffer-containers.md.

## 2026-11-07 — Broad API source export, native source pass and 3330 new matching bytes

Added read-only whole-model source/ASM and inferred-global exporters; saved5077
internal functions and complete listing evidence. Added conservative C++ source
generator, explicit pointer-conversion/compile-blocker ledgers, SourcePass VM
profile, native comparator, fixed88-body acceptance snapshot and ten-check
regression runner.567 generated functions compile;88 full model bodies add3116
disjoint bytes. Added enumeration open/next, suffix lookup/register/copy/string
duplication, idle scheduler, foreground-idle hook and message pump sources.
The hook20122a0b matches214 bytes/13 fields; eight other manual additions remain
unmatched. All prior1120/44 checks pass; combined complete code4450, metadata44
separate. New source-only observed VTable slots preserve existing keyboard gate.
All runs/failed compiler diagnostics retained. No Ghidra names/types/code or
tab changes, no candidate execution, no whole-DLL completion claim. See
api.dll/api-source-pass.md for addresses, hashes, limitations and outstanding work.

## 2026-11-03 — Monitor/geometry source and language-loader flow checkpoint

Added monitor_selection.cpp, window_geometry.cpp, geometry_error.cpp and
language_module.cpp; retained three bounded native runs and diagnostics. New
routines unmatched; prior1028/44 seven gates pass. Created observed72/184 Ghidra
types, corrected four function interfaces after GZF, saved revision710; code
unchanged. Resolved geometry error1/E_FAIL and language-load flow beyond its
partial50-byte body. Reader handles signed short LEA form, prior flow inventories
regress; generic original-byte verifier added. No runtime or candidate execution.

## 2026-11-03 — Main-window flow,188-byte hook match and coordinated types

Extended bounded reader for16-byte integrity gateway and real frame epilogues;
inventoried398 instructions/1346 bytes across main-window normal paths, preserving
prior initialization inventory regression. Added class source (100-byte near
match rejected at frame byte) and structured hook source; third bounded hook
trial matches188 bytes. Seven gates pass, complete total1028/metadata44.
Added api-main-window.md/verifier/artifacts; backup, hook/class typing and main
incomplete-body warning applied and saved revision603. No code/table/runtime edits.

## 2026-11-03 — Splash image source,96 new helper bytes and interface repair

Added startup_image.cpp/path_append.cpp, retained native append/image trials.
Append62 and NULL-aware length34 match full bodies, six regression gates pass;
complete code840/metadata44. Image488/445 remains unmatched. Corrected four
image/path signatures after GZF; preserved names/bytes, saved revision567 with
unsaved=false. Larger splash/path checkpoint; main-window2012205d next.

## 2026-11-03 — Remove canonical goto and model splash procedure

Replaced sole current pilot goto with wait-result conditions; src search clean.
Added structured splash WndProc source; retained api-splash-procedure-20261103-001.
New procedure620/475 and start225/227 unmatched.744/44 five-script gates pass.
Preserved source-style preference and next image-control analysis; no DB/runtime.

## 2026-11-03 — Autonomous disk query/splash work and central code ledger

Implemented disk query, splash start/thread/stop source and retained four trials.
Splash stop2013b6a4 full98-byte match with12 validated fields/string and negative
controls. Query/start/thread remain unmatched with bounded stops. Five existing
comparison scripts pass in final thread run. Added api.dll/api-code-ledger.md and corrected
prior arithmetic648 to646; new complete total744, metadata44. No Ghidra/runtime
changes. User authorized routine package continuation without per-step OK.

## 2026-11-03 — Legacy disk fallback source; stop on identical output

Added disk_space_fallback.cpp and full125-byte comparator. Two native trials
remain unmatched due operand scheduling; swapping source operands emits identical
code. Retained diagnostics, stop at2.648/44 prior gates pass. Documented legacy
signed arithmetic and mandatory output semantics; no Ghidra/runtime changes.

## 2026-11-03 — Disk dispatch139-byte match and capacity-gate analysis

Implemented disk_space_dispatch.cpp, retained two native trials and full-body
comparator. Trial2 matches139 bytes/12 fields with rejecting controls; prior
507/44 gates pass, new complete code648. Resolved lazy GetDiskFreeSpaceExA and
startup available-space30MiB/total-physical64MiB checks. No Ghidra changes/runtime.

## 2026-11-03 — Complete48-byte initialization error wrapper match

Added initialization_error.cpp and full-body/global/export comparator. Trial
api-init-error-20261103-001 succeeds with three validated fields and three
rejecting mutations; prior459/44 gates retained, complete code total507 bytes.
No database edits or linked DLL/runtime claim. Next memory/free-space slice.

## 2026-11-03 — Suffix source model and path-slice stop

Implemented path_suffix.cpp and retained api-path-suffix-20261103-001 native
trial/diagnostics.251 versus182 bytes, custom ABI/copy-inlining residuals; no
coverage gain.459/44 gates pass. Stop source question at1 and return to regular
stack initialization helpers. No database changes or candidate execution.

## 2026-11-03 — Bounded path-copy source model

Implemented source copy boundary with observed status/termination behavior.
Retained api-path-copy-20261103-001/002; both82-byte helper bodies identical,
original86-byte custom ABI unmatched. Stop on identical output.459/44 existing
gates pass; no coverage increase, database changes or candidate execution.

## 2026-11-03 — Buildable API resolver source model

Added path_resolver.cpp with explicit unresolved source interfaces for custom ABI
dependencies; retained native trial api-path-resolver-20261103-001 and diagnostic.
238 bytes, no match acceptance; existing459/44 gates pass. Verified final copy
helper's stack ABI/pointer return. No Ghidra mutations or replacement execution.

## 2026-11-03 — API security-cookie decompiler model

Typed existing cookie check void/ECX and applied installed security_check_cookie
call fixup after GZF. Correct0/-1 resolver returns restored in decompilation;
failure target retained. Exact original bytes/readbacks/save checked. Initial
pre-mutation guard corrected for REP RET prefix; no code/body changes or builds.

## 2026-11-03 — Applied API path interface storage

Typed seven existing path functions with explicit argument locations, EAX results
and verified stack cleanup. Preserved names/bodies. Pre-edit GZF, transactional
script, readbacks and successful database save retained. Separate cookie-helper
decompiler return artifact remains open; no new matching/build evidence.

## 2026-11-03 — API path selector and custom ABI ledger

Recorded resolver/selector/copy/append assembly and70 suffix entries. Corrected
selector interpretation to suffix index (.BGL/.CFG) and documented exact custom
register interfaces, relative base, case comparison and error paths. No database
mutations/builds or additional coverage; next coordinated custom-storage typing.

## 2026-11-03 — API attribute dependency source and diagnostics

Implemented natural202-byte attribute-query source with unresolved resolver and
fallback, retained api-path-attributes-20261103-001 and diagnostic script.202/202
length but unmatched frame/scheduling; no coverage gain. Existing459/44 gates
retained. Wrapper comparator now accepts the exact helper symbol when defined
as well as external. Dependency assembly snapshot retained; no Ghidra changes.

## 2026-11-03 — API path wrapper source match

Added natural path_checks.cpp to existing factory COFF translation unit and
full49-byte relocation comparator. Trial api-path-wrapper-20261103-001 succeeds
with two rejecting negative controls; prior410/44 gates pass. Complete disjoint
object code459 bytes; dependency2013341b remains unresolved. Return contracts
verified in original assembly; no Ghidra edits or candidate execution.

## 2026-11-03 — API initialization flow and child packages

Added read-only initialization flow/evidence scripts and original-byte verifier,
retained snapshots and api.dll/api-initialization-flow.md phase/work-package ledger.
Resolved three observed RET4 stack transfers and8-byte descriptors through two
independently inspected gateways;465 instructions/1596 bytes within the stated
call-opaque bound, superseding preliminary133-instruction extent. Handler thunk
201ee510 reaches __CxxFrameHandler3; no outer-wrapper EH4 generalization.
Updated roadmap/status/provenance; selected49-byte path wrapper20132c8e and
202-byte attribute helper2013341b next. No new source/body-match acceptance,
build, candidate execution or Ghidra mutation. API saved revision remains
1793724055614:636d9e2e:314, unsaved=false, original bytes unchanged.

## 2026-10-31 — English documentation in the project

Translated the consolidated knowledge base and project instructions. Canonical location: G:/Projects/Reversing/fsx-decomp/notes/re; AGENTS.md at project root. Original chat titles remain verbatim in SOURCES.md for traceability. Existing older project notes retained unchanged.

## 2026-10-31 — Initial consolidation

Consolidated S1–S4; verified fsx.exe read-only. No Ghidra database edits or saves. api.dll was not open. Ghidra reported unsaved fsx.exe changes.

## Historical changes from S1/S2

Consolidated end state, not an exhaustive chronological audit. Exact times/authors of individual manual edits are not always recoverable.

| Area | Reported change | Verification |
|---|---|---|
| fsx.exe 0x010019e0 | FSX_InitInstance, FSXApp class, this/locals typed | Live |
| fsx.exe 0x01001660 | InitializeGlobalFSXApp | Live |
| fsx.exe globals | g_fsxApp, FSXApp_vftable, g_mainWindow; applied FSXApp | Names/accesses live; addresses S1 |
| fsx.exe /Application, /ApplicationVTable | Object/table pointers, Initialize, GetWindowHandle | Live |
| fsx.exe 0x010018e0 | InitializeOleAndMessageFilter; COM/filter/SEH variable names | Live |
| fsx.exe MFC types | CWinThread, COleMessageFilter, VTable, AFX_MODULE_STATE | Live, partial layouts |
| fsx.exe 0x01001d30 | ShowErrorMessage, errorId/errorDetailText | Caller name live; address S1 |
| fsx.exe 0x0100263d | void __fastcall cookie signature, inline model preserving EAX | Live |
| fsx.exe 0x01004004 / local | __security_cookie / securityCookie | Names live; address S2 |
| api.dll 0x20120442 | GetApplication return/variables refined to Application * | Historical |
| api.dll 0x20120535 / 0x2020e010 | GetWindowHandle, windowId, hWindow, g_mainWindowHandle | Historically reported completed |
| api.dll 0x2012048a | Application::Initialize and __thiscall signature recommended | Implementation currently unverified |
| api.dll heap | InitializeHeapManager/heap names; AllocatePrivateHeapMemory discussed/typed | Historical; check current signatures |

Topic files preserve corrections: three versus five API arguments, pure-stdcall-table hypothesis, filter array artifacts, and cookie return are no longer unresolved starting assumptions.


## 2026-10-31 — S2 Windows version checks

Documented live helper return-value evidence, hierarchical version conditions, and unapplied return-type/variable-name suggestions in fsx.exe/startup.md; updated STATUS.md including saved state. No Ghidra edits.

## 2026-10-31 - User mask variable refinements verified
Live verified in fsx.exe 0x01001760: conditionMaskMajor and conditionMaskMajorMinor typed ULONGLONG, applied by the user. Recorded current state; no Ghidra modifications by this review.

## 2026-10-31 - VerSetConditionMask BYTE parameter applied
Changed fsx.exe EXTERNAL:00000068 Condition from UINT to BYTE via MCP at user request; readback verified. No ULONGLONG typedef correction or database save performed.

## 2026-10-31 - First local version-mask override verified
User-applied call-site signature override in fsx.exe helper 0x01001760 restores first return assignment; verified read-only via MCP. Remaining two call sites unresolved.

## 2026-10-31 - Version helper cleanup reviewed
Verified user changes in fsx.exe 0x01001760: all mask stages now recover correctly, BOOL result returned, integer mask types used in both imports. Second helper remains pending. Review only; current database saved.

## 2026-10-31 - Learning notes embedded in first version helper
Added and verified English plate comment at fsx.exe 0x01001760 explaining the analysis and ABI/Ghidra lessons. No signatures, variable types or executable bytes changed. Database save remains pending.

## 2026-10-31 - Second version helper cleanup verified
Verified user-completed cleanup of fsx.exe 0x01001800: BOOL return, descriptive locals, ulonglong mask and clean call results. Database saved; no Ghidra edits in review.

2026-11-01: fsx.exe 0x01001590 renamed FUN_01001590 -> StringCchPrintfA_optimized via MCP; provisional provenance and custom register/stack ABI documented in plate comment. Readback verified; prototype unchanged. Database changes unsaved.

2026-11-01 completed: fsx.exe formatter 0x01001590 repaired with program-local __fsx_edi_eax_varargs convention and dynamic storage, preserving varargs/HRESULT/names. Both ShowErrorMessage calls now recover EDI destination and EAX capacity correctly; fresh decompilation and MCP readback verified. English plate updated; reproduction XML retained in scripts/ghidra/fsx-edi-eax-varargs.xml. No instruction edits or navigation. Database unsaved.

2026-11-01: fsx.exe ShowErrorMessage local_410 -> messageText, char[1028] -> char[1024], matching observed formatter/copy capacity. MCP readback verified; database unsaved.

2026-11-01: Added shared/manual-recompilation-workflow.md with live on-disk PE/version/runtime-string evidence, fsx.exe/api.dll SHA-256, proposed VM/compiler/function-comparison workflow and tool limitations. Updated STATUS.md. No build toolchain installed or Ghidra database edits/save; MCP hash semantics and input identity verification remain open.

2026-11-01: Verified guest toolchain smoke report. Added reconstructed version helper source, separate pilot harness, shared input copies and manual build batch producing EXE/PDB/MAP/disassembly. Fixed MFC probe path. Pilot compilation pending; no Ghidra edits/save.

2026-11-01: Compared first compiled version-helper pilot against live Ghidra original assembly; documented frame/entry differences in pilot-comparison.md. Preserved baseline outputs and staged /Oy- /hotpatch experiment with unchanged source. MFC headers confirmed present. No Ghidra database changes.

2026-11-01: Archived experiment 2 and documented exact PE range comparison with explicit address-field mapping. Staged experiment 3 changing only /O2 to /O1; reconstructed source unchanged. No Ghidra changes.

2026-11-01: Reviewed and archived O1 pilot; documented increased instruction differences and unchanged frame discrepancy. Staged GL/LTCG experiment based on best O2 baseline; source unchanged.

2026-11-01: Verified first two address-normalized exact function-body matches (version helpers) with GL/LTCG. Added reproducible PE comparison script validating five target mappings per function; archived matching artifacts, source and commands. Documented scope/limitations. No Ghidra database edits.

2026-11-01: Added string_copy_pilot.cpp SDK provenance experiment and expanded comparison harness/build batch with include tracing and strsafe.h capture. Reverified FUN_01001600 assembly and documented unapplied ABI/name suggestions. No Ghidra changes; guest build pending.

2026-11-01: Verified SDK copy pilot, retained version-helper matches, archived experiment 5 including actual SDK header. Staged stack-local destination harness to remove observed global specialization. No Ghidra edits.

2026-11-01: Verified 72-byte raw exact SDK copy-worker match, retained two normalized version-helper matches, added reproducible copy comparison and archived experiment 6 including sources/header/build command. Documented identification limits and unapplied rename suggestion. No Ghidra changes.

2026-11-01: Organized shared-folder artifacts into numbered archive/results/setup directories, replaced old root launcher with BUILD.bat and README.txt, updated comparison-script defaults. Added SDK formatter pilot and fresh input/07-formatter. Three obsolete input folders remain locked and were preserved; no deletion. Reverified formatter assembly, no Ghidra edits/save.

2026-11-01: Archived formatter experiment 7, verified three regressions remain matching, identified secure-CRT header branch difference. Staged experiment 8 legacy CRT selection without modifying SDK source.

2026-11-01: Verified SDK formatter normalized exact match with legacy CRT branch, retained three earlier matches. Added reproducible formatter comparator and archived complete four-function baseline/reports. No Ghidra changes.

2026-11-01: Added ShowErrorMessage C++ reconstruction and minimal API interface pilot boundary; prepared experiment 9 with MFC development libraries and retained SDK helpers. Updated shared build/readme and comparison defaults. Reverified original 437-byte body and current prototype; no Ghidra modifications/save. Guest build pending.

2026-11-01: Archived ShowErrorMessage link failure and corrected pilot export linkage with extern C; staged experiment 10. No original function/ABI conclusion from this failure; no Ghidra edits.

2026-11-01: Reviewed first successful ShowErrorMessage build and verified four regressions. Archived experiment 10; staged common HRESULT check in reconstructed source for experiment 11. No Ghidra edits.

2026-11-01: Reviewed and archived experiment 11 (original-size ShowErrorMessage), retained four helper matches. Staged isolated final field-store ordering experiment 12. No Ghidra changes.

2026-11-01: Archived experiment 12 and reviewed corrected field-store order, retained four helper matches. Staged isolated HRESULT high-bit-expression experiment 13, no Ghidra changes.

2026-11-01: Added full ShowErrorMessage PE comparator with 16 verified reference mappings; experiment13 has one opcode byte difference, not exact match. Archived artifacts and staged signed-int expression experiment14. No Ghidra edits.

2026-11-01: Archived experiment14 two-byte difference and four helper regression reports; staged successful-format-block source variant15. No Ghidra changes.

2026-11-01: Verified experiment15 unchanged two-byte ShowErrorMessage difference, archived complete near-match baseline with four passing helper comparisons. Documented plateau and remaining context/ABI questions; no new build staged, no Ghidra edits.

2026-11-01 completed Ghidra transfer: typed ShowErrorMessage return/parameters and MSGBOXPARAMSA local, separated dialogResult, applied proven local names; modeled copy helper ECX/EAX parameters and EAX HRESULT return; retained formatter convention and improved locals. Added matching evidence/limits to five plate comments. Fresh decompilation verified; corrected partial rename side effects. fsx.exe saved, no independent backup or program switch/instruction edits.

2026-11-01: Staged structured no-goto ShowErrorMessage variant16, preserving direct-copy behavior and previous archives. Updated active shared inputs/build/comparison paths. No Ghidra edits.

2026-11-01: Verified/archived no-goto experiment16 unchanged encoding, rechecked original callback escape in Ghidra. Staged experiment17 non-GL pointer-retention boundary without artificial export; no Ghidra edits.

2026-11-01: Verified/archived experiment17 callback escape, unchanged two-byte near-match and four passing helper comparisons. Documented tested hypotheses and remaining compiler/context axes. No Ghidra modifications.

2026-11-01: Prepared SP1 toolchain experiment18 with hash-identical experiment17 inputs and same flags, fresh object directory; prior RTM artifacts preserved. Verified VMware loads mitigation-disable setting. No Ghidra edits.

2026-11-01: Verified SP1 toolchain build18 and all five comparisons, archived SP1 separately. Four matching helpers retained; ShowErrorMessage two-byte difference unchanged. No Ghidra edits.

2026-11-01: Added startup_context_pilot.cpp real MFC InitInstance flow/TU experiment19, corrected minimum Application slot0 declaration, added explicit opaque OLE test stub, removed synthetic callback retention call. Rechecked original startup/helper decompilation read-only; no Ghidra edits. Guest build pending.

2026-11-01: Archived experiment19 C3861 failure; added required MFC Automation header and staged experiment20. No Ghidra changes.

2026-11-01: Verified experiment20 shared startup context build, retained four helper matches and unchanged two-byte ShowErrorMessage difference. Measured InitInstance pilot758 bytes/frame0x564 without claiming match; archived full baseline/reports. No Ghidra changes.

2026-11-01: Reverified event creation/lifetime in Ghidra and added ordinary C++ constructor/destructor reconstruction; staged experiment21 with MFC header capture for next OLE/filter step. No Ghidra changes.

2026-11-01: Archived event/MFC evidence build21 with four helper matches; added OLE/filter reconstruction22 and layout assertions, /EHsc and ole32.lib. Replaced failure stub, no Ghidra edits.

2026-11-01: Verified OLE/filter build22 and layout assertions, four helper regressions, and237-byte initializer instruction skeleton; explicitly withheld exact-match claim pending VTable/EH metadata verification. Archived complete baseline; no Ghidra edits.

2026-11-01: Validated original28-slot filter VTable and EH structure; found missing always-TRUE OnMessagePending override. Added supported override, staged experiment23, documented EH evidence/remaining ordinal checks. No Ghidra modifications.

2026-11-01: Verified experiment23 OLE initializer/destructor/OnMessagePending matches, all28 VTable target identities and EH cleanup metadata; added reproducible PE comparator and complete archive. Retained four prior helpers and documented unchanged ShowErrorMessage near-match. No Ghidra changes.

2026-11-01: Reorganized vm-share into archive/setup/queue/runs and one XP worker launcher; preserved all historical inputs/results. Added bounded matching controller, XP build worker, finite source generator, source/receipt/compiler validation, regression gates, stop/status reports and integration replay fixture. Validated staging/parsers, two-job replay exhaustion, timeout and build failure. First live ShowErrorMessage search queued (up to24 proposals), guest worker launch pending. No Ghidra edits.

2026-11-01: Reviewed completed live24-candidate XP search, measured target body lengths, documented12 valid near-matches versus12 layout rejections, wrote REVIEW.md/review.csv. No new match, no Ghidra edits; remote vmrun setup deferred.

2026-11-01: Deleted 1,482 redundant/disposable build files (188.02 MiB), retained hash-audited evidence and representative binaries; cleared completed queue, retained receipts/build commands, removed stale controller pointer. Added cleanup audit/report and post-cleanup offline comparison reports; updated retention/shared-folder documentation. Helper/OLE matches reverified; ShowErrorMessage remains two-byte near-match. InitInstance is next analysis target; no source/Ghidra changes or new guest build.

2026-11-01: Added direct-vmrun worker-launch helper (authentication blocked by passwordless XP account; not end-to-end verified), worker status heartbeat, and permanent receipt/build-command retention. User stopped proposed autostart approach; removed installer and controller autostart integration. Smoke wait stopped before queueing; no build or guest installation occurred.

2026-11-02: Completed real host-queued XP worker smoke build worker-smoke-20261102-001 with pinned SP1 compiler, source integrity and all existing helper/OLE comparison checks. Verified new heartbeat protocol and receipt/build-command retention; ShowErrorMessage remains two-byte near-match. No Ghidra/source changes.

2026-11-02: Investigated InitInstance0x010019e0 with real XP builds; added diagnostic aligner/HTML and reviewed-source controller mode. Adopted tested buffer lifetimes/capacities/order, explicit ACTCTXA setup, direct error paths and SDK-backed startupArgument correction in canonical source. Frame now0x554, body777 vs original769, not matched. Preserved helper/OLE matches and ShowErrorMessage near-match. Repaired guest failure return reporting, controller variable collision/lock cleanup and destructor gate; verified real compile-failure handling. Documented superseded command-line interpretation and unapplied Ghidra suggestion. No Ghidra changes.

2026-11-02: Read-only API.DLL vtable/Initialize ABI verification and correction of definite command-line type claims; recorded pending startupArgument rename suggestion without applying it. Tested three further InitInstance source variants; no improvement, canonical source unchanged. Added and verified19-target reference inventory script/report; no exact match claimed. No Ghidra edits/navigation by agent.

2026-11-02: Added block-aware InitInstance JSON/HTML diagnostics with reference/error anchors, branch-polarity-aware propagation and conflict/edge checks; integrated into build controller and verified via fresh build016. Adopted tested conventional ACTCTXA aggregate initialization:246 instructions,777 bytes/frame0x554, not exact. All established regressions retained; no Ghidra changes.

2026-11-02: Reverified CWinApp constructor/vtable evidence for InitInstance; added exact per-block encoded-length accounting and validated-reference non-branch PE comparison, integrated for future diagnostic builds. Explained+8 bytes entirely by branch widths/layout; recorded remaining43 positional instruction differences. Canonical source unchanged, no new guest build or Ghidra changes.

2026-11-02: Added and matched ExitInstance C++ reconstruction; certified existing application constructor/destructor bodies,223 additional normalized-exact bytes. Added compare-app-lifecycle.ps1 plus shared PE reader, AppLifecycle target/regression gates, preserved archive24 and updated replay fixture. Fresh worker run003 matched; negative control rejected. Fixed new target's status accounting after run002; InitInstance/ShowErrorMessage remain nonmatching. No Ghidra changes.

2026-11-02: Added reusable48-slot application-vtable inventory with exact inherited import checks and SDK leaf checks; identified missing GetMessageMap/Run overrides, callback and empty message-map data. Recorded original-vs-rebuilt identical-stub sharing as a folding hypothesis. Read-only analysis only; no canonical source/flags/Ghidra changes or new guest build.

2026-11-02: Added matching GetMessageMap/empty MFC message-map data, Run and callback C++ bodies; corrected source Application slot+8 callback contract using live API evidence. Added compare-app-dispatch mandatory gate, verified fresh worker build and negative callback-target rejection, preserved archive25 and refreshed inventory/replay fixture.13 matched bodies/1001 bytes; InitInstance/ShowErrorMessage still nonmatching. No Ghidra edits.

2026-11-02: Controlled ICF experiment and fresh default build retain13 matched bodies while eliminating4 vtable-sharing-pattern mismatches over76 slots. Adopted /OPT:ICF as default; explicit NOICF control retained. Added folding option, command provenance logging and alias comparison gate; preserved baseline26 and updated replay/inventory defaults. Source and Ghidra unchanged; remaining near-matches unaffected.

2026-11-02: Added MfcWindows build template/profile and verified GUI/MFC startup acceptance checks; adopted as default after two real XP/SP1 builds retained13 matches without console/helper caller TUs. Added negative console-profile check, preserved baseline27 and updated replay/inventory references. Near-matches unchanged; source/Ghidra unmodified, no candidate execution.

2026-11-02: Completed getter GL/SameTU experiments and adopted clean GL default, preserving13 matches; removed obsolete source retention helper, saved baseline28. Added reusable read-only Ghidra exporter and PE-backed EXE inventory/coverage report, including import thunks and undecoded regions. No Ghidra database changes. Stopped at user's usage-limit request.

2026-11-02: Verified actual CRT initializer order and corrected global CWnd/application declaration order. Added five-wrapper/113-byte comparator, table-call bounds/import checks, mandatory MfcWindows gate and StaticInitialization target; two real XP builds retain previous13 matches (18 total/1114 bytes), negative old-order control rejected. Preserved baseline29 and refreshed inventory/replay defaults. Explicit extra XIY exception-filter initializer remains unmatched. Backed up fsx.exe to GZF, created/named/commented five bounded functions and two CRT table comments, verified readback and saved; API untouched. See static-initialization-matching.md.

2026-11-02: Identified extra XIY initializer relocation in installed SP1 crtexew.obj; extracted pinned RTM msvcrt.lib from existing installation medium. Added optional toolchain diagnostics and CRT selection (Auto: RTM for MfcWindows, SP1 for ConsolePilot), preserved MFC allocation precedence after rejecting first incorrect-order build. Two accepted real builds retain18 body matches and recover original C-table roles/order; default/archive30 and inventories/replay updated. Existing near-matches unchanged. Updated and saved two Ghidra table comments; no function/type/API changes. See crt-library-context.md.

2026-11-02: Identified and certified complete226-byte pre_c_init CRT body with27 validated reference fields and three raw PE dependencies. Added mandatory MfcWindows comparator; real XP build and negative literal control pass. Backed up fsx.exe, disassembled only the bounded body, created int __cdecl pre_c_init(void), commented/read back/saved. Refreshed inventory and documented auto-created one-byte dependency placeholders as incomplete. Existing18 source/helper/wrapper count, C++ source and baseline30 unchanged. See pre-c-init.md.

2026-11-02: Verified75-byte pre_cpp_init with12 validated fields and36-byte RTC termination dependency; new gate passes a real XP build and negative literal control. Backed up fsx.exe, completed five bounded CRT definitions, applied approved g_crtManagedImageFlag rename preserving type, read back and saved. Refreshed inventory and added scope-separated EXE matching roadmap. Prior18 body count and canonical C++/baseline30 unchanged. See pre-cpp-init-and-exe-gaps.md.

2026-11-02: Verified full479-byte CRT startup region with39 mapped fields,40-byte EH4 table,10-byte entry,148-byte cookie initializer and69/20-byte SEH helpers. Added mandatory gate, verified real XP build and two negative controls. Backed up fsx.exe, decoded54 embedded-EH bytes, added labels/comments/scope refs, preserved425-byte parent ownership, set int cdecl prototype, read back and saved. Source/baseline30 unchanged; inventory/roadmap updated. See full-crt-startup.md.

2026-11-02: Verified CRT dispatch35, PE validation/section helpers41/66, nonwritable region108/EH table28 and cookie check15. Added mandatory gate; real XP build and two negative controls pass. Backed up fsx.exe, defined/typed minimum-ABI dispatcher, filled23 embedded-EH bytes with labels/refs/comments, preserved existing prototypes. Applied documented analysis-only leaf inlining to correct a false uninitialized EDX/imagebase artifact; no machine/C++ code change. Read back/saved; refreshed inventory/roadmap. See crt-image-helpers.md.

2026-11-02: Verified260-byte GS reporter with33 fields, record backing data/pointer pair and stack probes22/22/43. Added mandatory gate; real XP build and two negative controls pass. Backed up fsx.exe, applied standard Windows failure-record structures, documented special EAX/ESP helper ABI without guessing prototypes, read back/saved. Source and baseline30 unchanged; inventory/roadmap refreshed. See crt-failure-stack.md.

2026-11-02: Verified onexit150/cleanup9/atexit18 and EH4 finally28, added mandatory gate, passed real XP build and negative lock/pointer controls. Backed up fsx.exe, named cleanup/adapter, applied distinct callback datatype models via Java after text-parser failures, added evidence/finally refs, read back and saved. Source/baseline30 unchanged; inventory/roadmap updated. See crt-onexit.md.

2026-11-02: Verified ATL101/4/100 bodies/cache/imports, resolving real-getter fragment hypothesis. Added gate, real XP build and two negative controls pass. Backed up fsx.exe, created real getter, typed/named cached pointer, read back and saved. Observed two original5-byte CC gaps absent in rebuild; recorded isolated padding hypothesis without enabling flags. Source/baseline30 unchanged; inventory/roadmap updated. See atl-codepage.md.

2026-11-02 — Verified remaining GS72/15/multiply52 bodies and three import thunk
identities; integrated RuntimeRemainders gate and fresh successful worker build.
Two mutation controls rejected. Backed up/saved fsx.exe; defined bounded helpers,
completed/linked thunks, typed five tokens, known pointers and EH4 table models.
Completed alternate/unreachable listing paths; documented precision38 reachable
versus41 compared bytes after existing noreturn import propagation. Corrected
inspection IAT address. Refreshed inventory: undefined bytes now only00/CC,
without claiming whole-EXE matching. See runtime-remainders.md.

2026-11-02 — Tested and adopted /FUNCTIONPADMIN:5; ten missing original CC gaps
restored with all body regressions retained. Added source/command-controlled gap
report; separated five old adjacency assumptions from exact body lengths, with
strict CC checks and rejected gap-only mutation. Preserved real verified build as
31-function-padding; default/inventory/replay updated,30 retained. InitInstance
and ShowErrorMessage unchanged. No Ghidra edits. See function-padding.md.

2026-11-02 — Audited original RTTI anchors/names and removed extra reconstructed
C++ RTTI with source-identical /GR- builds. All existing regressions retained;
new RttiMetadata gate rejects actual enabled control. Preserved32-rtti-disabled,
updated defaults/inventory/replay, retained31. Backed up Ghidra and saved two
evidence comments without names/types/code changes. See exe-rtti.md.

2026-11-02 — Added DataConstants regression gate:18/183 literal scope, token
pointers32, load-config72/SafeSEH8, selected initial storage/state and inherited
MFC runtime-class dispatch. Fresh worker build and three negative controls pass.
Baseline32/source/flags unchanged. Ghidra backed up, evidence comments appended
without names/types/code changes, saved/read back. Resolved token counterpart
and documented directory64 versus structure72 distinction. See exe-data-constants.md.

2026-11-02 — Added complete ordinary-import content/hint/termination gate and
PE-directory/header/section/debug/certificate inventory.137 identities match,
128 entry slots differ; stricter actual-order control still rejects. Explained
135 Ghidra functions plus two imported data targets. Fresh XP build/all regressions
and negative controls pass. Backed up/saved Ghidra comment-only changes. Baseline32
and source/flags unchanged. See pe-imports-and-directories.md.
Same milestone: corrected own debug-record description: final byte1537 is the
PDB string terminator, not padding; persisted comment/readback updated.

2026-11-02 — Adopted original image base through source-identical /BASE experiment;
all current regressions retained. Removed four hardcoded comparator base
assumptions, added requested-base startup enforcement and wrong-base controls.
Matched-length real control shows343 identical symbol RVAs; documented worker
PDB-path layout effects. Preserved33-original-image-base and updated defaults,
retained32. No Ghidra or C++ edits. See image-base.md.

2026-11-02 — Adopted /MERGE:.rdata=.text after source-identical actual merge/control
builds retain all regressions. Non-resource names/order/RVAs/raw extents/flags
match; virtual extents and contents remain open. Added requested-layout gate,
actual wrong-mode controls and experiment report. Preserved34-merged-readonly,
updated defaults/inventory/replay, retained33. No Ghidra/C++ edits. See section-merge.md.

2026-11-02 — Matched original PE stack reserve/commit via single-option actual
build, all prior regressions retained. Added requested-stack gate and actual
wrong-mode controls; preserved35-original-stack,34 retained. Bounded usage cut;
no further job started. See stack-header.md.

2026-11-03 — Adopted measured /PDBALTPATH:fsx.pdb support; original32-byte
basename CodeView shape restored. Added requested-mode gate, actual controls and
longer-job-name repeat comparison (equal outside known timestamps/RSDS GUID).
All regressions retained; preserved36-basename-pdb,35 retained. No source or
Ghidra edits. See pdb-path.md.

2026-11-03 — Matched selected PE flags and version fields in five source-identical,
single-option actual builds; all prior gates retained. Added requested-mode gate,
whole-file step report and actual negative controls. Preserved37-original-header,
updated defaults/inventory/replay, retained36/stages. Identified32-byte zero-extent
difference between CodeView and SafeSEH; cause open. No Ghidra/source edits. See
pe-header-settings.md.

2026-11-03 — Investigated original zero gap and added optional diagnostic32-byte
.rdata$r object, default0. Actual probe restores table placement/.text size but
also reorders four SDK leaf symbols; all prior gates retained. Default control
passes, nonzero mutation rejected. Golden37 unchanged; original cause unresolved.
Ghidra backed up, evidence comment appended/saved without types/names/code edits.
See fsx.exe/codeview-gap.md; no new original variable or production padding claim.

2026-11-03 — Consolidated EXE roadmap into current37 status, prioritized remaining
experiments and completion criteria. Removed obsolete current-baseline advice
from roadmap; historical evidence retained in topic files. Documentation only.

2026-11-03 — Tested /ORDER first3 then12 application-prefix functions; all relative
entry distances match with32-byte standard gap offset. Adopted38-application-order,
retained37/diagnostic gap control. Added hashed response input and warning/MAP
order verification, corrected lifecycle parser's unlabeled-callback overrun and
one erroneous decorated name in rejected trial. Prior body gates unchanged.
Documented build-host/SDK/library/context factors; Ghidra unchanged.

2026-11-03 — Extended measured order to22 symbols/21 entries through SDK/ATL.
Isolated callback external-linkage reconstruction and folded Serialize/Dump
representative issue; retained strict warnings/order/body/alias gates. Adopted39,
38 retained; optional synthetic gap remains off. All prior comparisons pass,
known code differences remain. Staged source/linkage choice explicitly documented;
Ghidra unchanged. See extended-function-order.md.

2026-11-03 — Inventoried80 import thunks in each EXE and tested optional
MfcThunks /ORDER mode for66 MFC thunks before CRT startup. Actual88-symbol job
passes prior gates;66 thunk placements now uniformly original-32, prefix22
unchanged. Added manifest and controlled opcode/IAT/source/command comparator.
IAT128-slot differences and remaining CRT order persist. Auto/golden39 retained;
no Ghidra change. See mfc-thunk-order.md.

2026-11-03 — Verified CRT COFF section structure and actual rejection of three
static /ORDER names. Controlled /INCLUDE:_WinMainCRTStartup orders crtexew before
cookie/onexit;36 mapped runtime symbols now original-32, prior88 symbols unchanged.
Import slot differences128->103; all prior gates pass. Adopted actual default40
with MfcThunks/EntryInclude, updated inventory/replay defaults, retained39/trials.
Synthetic gap remains off; Ghidra unchanged. See crt-selection-order.md.

2026-11-03 — Verified all80 import thunk placements/targets and trailing CC runs
in40; explicitly corrected stale residual CRT/debugger-hook placement claim.
Inventoried IAT103 differences by module/two CRT swaps. Controlled137-symbol
INCLUDE trial rejected by existing placement gate; targeted3-symbol trial passes
but worsens IAT to105. Added strict reports/optional controls; default40 retained.
No Ghidra change; next independent work is data placement. See import-selection.md.

2026-11-03 — Added strict30-span writable-data ledger (1153 mapped bytes,31/51
unassigned). Inspected actual COFF alignments and original xrefs; tested isolated
ATL include/library precedence and explicit BSS merge. No default change: include
does nothing, library precedence partially moves globals without size improvement,
merge worsens extents. Full body/thunk gates retained. Read RTM MFC media members
without switching libraries. No Ghidra edit. See writable-data-layout.md.

2026-11-03 — Compared installed/RTM MFC data symbols/directives, identified
appmodul manifest-force difference. Isolated RTM support-library replacement
removes4-byte common allocation, reducing data delta20->16; matched PDB resolves
initial4099 warnings. Original embedded manifest corroborates50608 dependency
context without identifying compiler. Adopted actual default41, retained40;
prefix/IAT/body/thunk gates unchanged, sidecar/resource equality still open.
No Ghidra edit. See mfc-support-library.md.

2026-11-03 — Added paired MSF7/DBI PDB contribution reader with MAP-owner checks
and wrong-PDB negative control. Confirmed41's two small zero gaps; missing pilot
contributions are not padding. Same-source native application-TU trial shrinks
data delta to8 but fails existing initialization mapping; retained as rejected
control. Baseline41/GL defaults unchanged, remaining16 unresolved. Ghidra untouched.

2026-11-03 — User-requested new-chat wrapup: created
handoffs/HANDOFF-20261103-coverage-and-code.md with selected41 inputs/evidence, limits,
same-hash coverage-ledger deliverable, subsequent code-target selection and
bounded trial rules. Deferred layout/IAT polishing pending new evidence; updated
continuation priority. Documentation only, no build/default/binary/Ghidra change.
# 2026-11-03: baseline41 range ledger and bounded InitInstance improvement

- Added scripts/coverage-baseline41.cjs, artifacts/fsx.exe/coverage-baseline41-20261103.json
  and notes/re/fsx.exe/coverage-baseline41.md. Same-hash paired ranges, overlap-preserving
  interval unions, explicit unchecked/diagnostic/metadata policies; no whole-EXE
  percentage.3962 verified code bytes,1206 original bytes in known near-matches.
- One reviewed module-instance-local worker trial restored a five-byte original
  load sequence,43->41 differing InitInstance nonbranch pairs. Full prior gates,
  whole-section delta audit, thunk/CC and writable checks pass. Added the audit
  script/report and accepted the exact local change into canonical startup source.
- Archive41 and its ledger remain baseline-specific; new candidate retained in
  initinstance-module-local-20261103, no golden/replay promotion. Stopped after
  one successful trial; layout/import polishing remains deferred.
- Ghidra read-only, saved revision1793671788023:335a223f:3. No DB edits, renames,
  navigation/save/backup or candidate execution. Details in fsx.exe/coverage-baseline41.md
  and fsx.exe/initinstance-matching.md; STATUS updated.

# 2026-11-03: API.DLL work packages and boundary inventory

- Added api.dll/api-matching-roadmap.md (API-00..08) and api-application-boundary.md.
  Reused PE helper in scripts/inventory-api.ps1; pinned original export/import/
  section/body inventory and live snapshots stored under artifacts/api.dll/api-*.
- Verified GetApplication export144, four VTable targets, getter/initializer/
  window getter and normal wrapper EAX dataflow. Recorded unresolved exception
  return/teardown and unapplied naming suggestions; no body match claimed.
- STATUS changed to the user-requested DLL phase. No Ghidra edits, tab switches,
  save/backup, DLL build or candidate execution. No new installation input needed.
# 2026-11-03: first API DLL match, heap control and saved boundary corrections

- Added separate DLL pilot source, guest command, locked one-job dispatcher and
  complete21-byte GetWindowHandle comparator with two negative controls. Actual
  window/native-heap/GL-heap runs retain inputs, outputs, hashes and receipts;
  window gate passes all three. No candidate execution or EXE/default changes.
- Reconstructed initializer/allocator heap source. Native144/146 and174/174 remain
  unmatched; source-identical GL/LTCG changes no symbolic instruction schedule.
  Stopped this context question without blind third trial.
- Verified detached wrapper EH handler returns0 and normal path forwards EAX;
  verified shutdown frees/resets application. GZF before Ghidra edits; corrected
  wrapper name/prototype and provisional startupArgument parameter name, added
  comments, read back and saved at1793718640047:636d9e2e:31. VTable fields and
  legacy parameter type preserved pending coordinated review. No tab changes.
- Added api.dll/api-first-pilot.md and updated boundary/roadmap/STATUS; packages beyond
  these stated scopes remain open. See topics for hashes, addresses and limits.
# 2026-11-03: module-owned API build organization

- Relocated the three completed API runs into vm-share/modules/api/runs; added
  canonical api-pilot.map/pdb alongside DLL, moved fixed-name worker artifacts
  into worker-output, retained original file hashes/provenance.
- Worker-held queue folders could not be moved; completed jobs instead copied
  and verified into modules/api/jobs, with shared queue retained. No restart.
- Added relocation manifest, module metadata and build-storage READMEs. Updated
  API dispatcher/comparator/documentation paths; existing fsx.exe runs/archive,
  source and shared infrastructure preserved. No rebuild or Ghidra changes.
# 2026-11-03: API lifecycle object equality and coordinated Ghidra boundary types

- Added ordinary lifecycle source and separate native COFF profile using the
  API-owned worker directories. First trial verifies177 code bytes/28 EH4 bytes
  with explicit symbol/addend/label checks and three rejected negative controls.
  No linked-module/dependency/runtime claim. Fresh header DLL retains21-byte gate.
- Backed up API database, created bounded Shutdown function, named/typed loop
  outer method, filled VTable+4/+8 function-pointer fields and coordinated
  Initialize startupArgument uint plus this types. Read back and saved revision
  1793720305185:636d9e2e:135; no tab/code-byte changes. Rolled-back initial managed
  parameter-array exception documented in api-lifecycle-matching.md.
- Updated STATUS/roadmap/boundary and module build metadata; next Initialize
  outer-body source and nonstandard helper register ABI, not renewed heap trials.
# 2026-11-03: Initialize source, FDI stream matches and ESI registry model

- Reconstructed four SDK IStream-compatible adapters;140 complete raw native
  COFF bytes match, extending prior177-byte code/28-byte metadata gates. Read-to-
  Write slot mutation rejects. Initialize source compiles158 versus160;21 mapped
  references and123 scoped bytes verified, remaining register ABI residual explicit.
- Retained two initial compiler failures and successful corrected002 jobs; no
  third source/flag permutation. Added failed-job snapshot/status handling to
  dispatcher. Header DLL window regression passes. No runtime/baseline promotion.
- Pre-edit API GZF; four callback names/prototypes corrected, registry helper
  custom ESI input typed/named and separate four-stack-word cleanup verified.
  Initialize decompilation now includes owner argument. Readback/save at
  1793721567359:636d9e2e:281; no tab/code-byte changes. Details api-initialize-fdi.md.
# 2026-11-03: compiled callback-list/reallocator dependencies and bounded stop

- Added observed header/record types, real registry/reallocator source and API
  registry COFF profile. Preserved NULL retry and mode4 old-pointer restoration;
 14-reference/critical-shape diagnostic and negative controls retained. Neither
 helper is certified matched. Internal-helper export cleanup leaves all4 native
 function bodies unchanged; no further permutations.
- Shared native COFF parser extracted;317 complete-code/28 metadata and123 scoped
 Initialize bytes pass regression. No previous EXE/default changes or execution.
- GZF before realloc rename/prototype/comment, readback/save at
 1793722735273:636d9e2e:312; original bytes unchanged. STATUS/roadmap updated:
 next GetApplication/VTable bridge rather than continued native infrastructure
 polishing. Details/hashes in api-callback-registry.md.
# 2026-11-03: GetApplication object match and generated four-slot bridge

- Changed minimum API source interface to four virtual slots in observed order,
  added natural placement-construction factory and dedicated native /GR- object
  profile. RTTI-less prefix observation is specific to this bridge, not all DLL.
- Trial75 bytes rejected; single source returned-pointer correction recovers72
  exact normalized bytes/7 references, table16/4 symbols validated. Prior317 code
  and partial Initialize gates retained; window21 now also verified in object.
  Complete object scope410, metadata44, with no duplicate linked-window count.
- New allocation-size/VTable-pointer/slot-substitution negative controls reject;
  virtual-header DLL window regression passes. Original export144 parsed, final
  rebuilt linked export144 not claimed. No further source permutations.
- GZF then getter evidence comment only; preserved names/types/unknown convention,
  saved314 revision (full token in api.dll/api-factory-matching.md). Next API-05 reachable
  initialization entry mapping, not further native infrastructure polishing.
# 2026-11-07 interruption recovery / font enumeration

Confirmed saved API font/enumeration types and five signatures after prior save
timeouts. Retained structured font source and exact92-byte enumeration close
body; eight comparator gates pass, ledger1120 complete code/44 metadata.
Added api.dll/api-font-resources.md and reconciled STATUS/SOURCES/ledger. Font body
891/733 remains unmatched; no further shape trials or runtime execution.

# 2026-11-08 CAB enumeration and dependency consolidation

Added observed CAB cursor/entry source layouts and three structured backend
routines, integrated in factory TU. Extended optional source diagnostics for
their bodies. First source build96/144/441 remains unmatched; ten existing gates
pass, accepted4450 code/44 metadata unchanged. Retained eight original-listing
byte checks. Added api.dll/api-alternate-enumeration.md and api.dll/api-module-references.md;
corrected outdated language-resource uncertainty using actual extraction. No
Ghidra mutations/tab switches or runtime execution. Next cache/path/exception
ownership contracts; known register ABI blocker stops blind match trials.

# 2026-11-08 CAB owner/cache source

Added cab_owner.cpp path discovery, initialization, image/list destruction and
normal cache resolver; expanded observed owner124 layout with SDK assertions.
Verified constructor-failure free action and documented missing emitted native
EH under fixed profile. Two distinct source-addition builds pass ten gates;
4450/44 unchanged, four new bodies unmatched. Updated optional diagnostics,
evidence, topic/status/roadmap. No Ghidra edits/runtime or repeated shape trials.

# 2026-11-08 CAB prefix image and stream source

Added structured CAB prefix reader and four observed stream helpers/interfaces.
Release33/read39 match full raw bodies; new comparator/eleven-gate regression,
complete total4522/metadata44. Image671/700, seek43/57 and stat143/156 remain
unmatched. Owner reload translation deferred on verified uninitialized stream
slot path; no invented initialization. Added original-byte evidence/topic and
updated ledger/status/roadmap. No Ghidra changes/runtime or flag matrices.

# 2026-11-08 file/wrapper stream producers

Added source file/wrapper producers, wrapper initialize/destroy and observed
276/48-byte records; source AddRef slot typed. Verified reference ownership,
file-handle leak path and constructor-failure pool-return action. One build,
four unmatched bodies; eleven gates retain4522/44. Updated diagnostic/evidence/
topic/status/roadmap. No DB changes/runtime or native flag/body matrices.

# 2026-11-09 stream read/seek/stat methods

Added six structured methods and shared observed stream records. Corrected
source bufferStart/validBytes/position field names from actual access patterns.
File read57/seek64 and wrapper seek92 full matches add213 (total4735/44), twelve
gates pass. New full-body comparator validates imports/negative controls/overlap.
Other stat/read bodies unmatched; no matrix. Added evidence/topic and updated
ledger/status/roadmap. No Ghidra edits/runtime; owner/EH limitations retained.

# 2026-11-09 CAB buffer cache and object pool comparison

Added three structured buffer cache routines and connected the wrapper buffer
dependencies. Retained first source after a bounded unmatched search trial.
Recorded original byte evidence and shared pool correspondence. Twelve gates
pass at 4735 code bytes and 44 metadata bytes. Pool synchronization source is
deferred pending lock and slot return contracts.

# 2026-11-09 shared stream object pools

Added shared allocation, return and initializer C++ for both stream pools.
Recorded original lock upgrade, retry and slot publication contracts. Twelve
regression gates pass. Atomic slot and low-level lock source remains external.

# 2026-11-09 CAB slot state and node storage

Added source for slot creation, destruction, growth and aligned storage.
Recorded raw-pointer trailer and process-heap accounting contracts. Twelve
gates pass. Eight source bodies remain outside acceptance and atomic head
operations remain explicit dependencies.

# 2026-11-09 CAB atomic slot transfers

Implemented shared slot head operations, publication and reservation. The x86
primitives retain the original mode-dependent LOCK forms. Integrated both
transfer entry points with the pool source. Twelve gates pass. Transfer body
acceptance and the lower-level pool lock remain open.

# 2026-11-09 stream pool lock and owner lifecycle

Implemented lock release/create/destroy, both owner lifecycles, reset, trim
and registry unlink. Shared the tagged CAS primitive with slot pop. Verified
pool acquire modes and upgrade wait/rollback boundaries. Twelve gates pass.
Registry unlink stopped after two unmatched trials and first source retained.

# 2026-11-09 stream pool lock acquisition

Integrated source for reader, writer and upgrade acquisition at the five
pool call sites. Preserved the three queue rollback paths and shared CAS64
selection. Twelve gates pass. Added emitted wait/spin diagnostics and source
scope documentation. Pool acquisition is outside original body acceptance.

# 2026-11-09 stream pool registration and owner tables

Added owner globals, guarded getters, registration and atexit paths, scoped
cleanup, statistic methods and the three observed table spans. Eleven full
raw body matches add97 bytes. Target-validated tables add132 metadata bytes.
Thirteen gates pass at4832/176. CRT placement and native owner EH remain open.

# 2026-11-09 file and wrapper stream tables

Connected both 14-slot tables and modeled query, reference lifetime, write,
resize, copy, clone and fixed-result methods. Five full bodies add86 code
bytes and the table spans add112 metadata bytes. Fourteen gates pass.

