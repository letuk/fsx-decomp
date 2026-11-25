# Sources and verification limits

The electrical and propeller VC8 runs now retain separate object
files. The electrical run verifies the circuit body and bus
lookup. The propeller run verifies both electrical call relocations.
The linked propulsion build combines the two translation units.
[Electrical circuit availability](sim1.dll/sim1-propeller-step.md#electrical-circuit-availability).

Automatic feather arming uses the live `sim1.dll` listings at
`0x20c48270`, `0x20c48250`, the switch handler and configuration
bindings, and native VC8 COFF. The comparator verifies the
threshold literals, static calls and four switch targets in each
artifact. The exact spool designation of turbine record `+0x00`
and simulator runtime verification remain open.
[Automatic feather arming](sim1.dll/sim1-propeller-step.md#automatic-feather-arming).

The `sim1.dll` electrical query uses the live listings at
`0x20bfee30` and `0x20bfef40`, the two propeller call sites,
the configuration reader at `0x20bfefa0`, its cross-reference
to `0x20c7c008`, and native VC8 COFF. The original jump table
and the original call targets were verified against the PE image.
The concrete owner dispatch target at slot `+0x114` remains open.
[Electrical circuit availability](sim1.dll/sim1-propeller-step.md#electrical-circuit-availability).

The propeller icing calculation uses the original listing at
`sim1.dll:0x20c48710`, the caller, the environment temperature
writer, the deice configuration and switch bindings, and native
VC8 COFF. The earlier host x86 fixture supplied an electrical
result. The current fixture exercises the reconstructed query.
The original weather producers for the two icing fields remain
open.
[Propeller icing load factor](sim1.dll/sim1-propeller-step.md#propeller-icing-load-factor).

The integrated mapped propeller passes use the live `sim1.dll`
`0x20c47420` listing and configuration mapping, the pinned PE and
native VC8 COFF. The comparator checks the six called bodies by
their original targets and rebuilt symbols. Host x86 execution
uses a local phase utility fixture. The two original preparation
calls remain outside the reconstructed scope.
[Integrated mapped propeller passes](sim1.dll/sim1-propeller-step.md#integrated-mapped-propeller-passes).

The ground selector and integrated blade-pitch reconstruction use
the original call sites, the ground-contact writer, the installed
reference object's constructor, the VTable bytes, and raw target
instructions. The native COFF and host x86 checks cover the
reconstructed invocation and pitch stages. The original surface
mode values remain unnamed.
[Ground contact selector and integrated blade-pitch update](sim1.dll/sim1-propeller-step.md#ground-contact-selector-and-integrated-blade-pitch-update).

The feather-state and low-speed latch reconstruction uses the live
`sim1.dll:0x20c485c0` and `0x20c47720` listings, the turbine torque
writer and parser bindings. VC8 covers explicit ranges, including
the feather call relocation. Independent host x86 checks exercise
the reconstructed state transitions and timer rounding.
[Feather state and low-speed latch](sim1.dll/sim1-propeller-step.md#feather-state-and-low-speed-latch).

The scoped propeller pitch reconstruction uses live `sim1.dll`
listings and the parser's pitch, cruise and governor bindings.
VC8 object evidence covers three extracted ranges. The host x86
tests verify calculated values for the reconstructed source.
[Blade pitch and advance ratio](sim1.dll/sim1-propeller-step.md#blade-pitch-and-advance-ratio).

The propeller thrust reconstruction uses live `sim1.dll:0x20c47c90`
and caller listings, the AIR `0x511` binding, configuration parser
evidence, and x87 literal-store replay for the three low-speed
curves. The pinned PE and native VC8 COFF validate call and data
identities. Host x86 tests and linked propulsion checks cover the
reconstructed source and the existing comparison gates separately.
[Propeller thrust publication](sim1.dll/sim1-propeller-step.md#propeller-thrust-publication).

The aerodynamic propeller torque and table lookup use live Sim1
listings at `0x20c47ae0` and `0x20bcc030`, the feather setting
bindings, AIR record `0x512`, the `D⁵` writer, the pinned original
PE and VC8 object evidence. The lookup scope verifies finite
coefficient table behavior by static analysis. The comparison
covers specified instruction ranges.
[Aerodynamic propeller torque](sim1.dll/sim1-propeller-step.md#aerodynamic-propeller-torque).

The propeller speed integration at `sim1.dll:0x20c47ba0` uses the
live caller and callee listings, verified configuration bindings,
the pinned Sim1 and Util PE files and native VC8 COFF. Original
`util.dll` export and disassembly evidence verifies the phase wrap.
The import library and existing linked propulsion scopes were
verified separately from the integrator's native COFF comparison.
[Propeller speed integration](sim1.dll/sim1-propeller-step.md#propeller-speed-integration).

The mapped propeller step and synchronization correction use the
live `sim1.dll:0x20c47420` and `0x20c48150` listings, the
`prop_sync_available` parser, the blade-pitch reader, and native
VC8 COFF. The call-site register assignment resolves the two
distinct `+0x3c` fields. Full-body and simulator verification remain open.
[Mapped propeller step and speed synchronization](sim1.dll/sim1-propeller-step.md).

The propeller induced-airspeed writer in `sim1.dll:0x20c480b0`
uses the live writer and caller listings, the `propeller_diameter`
parser binding, native VC8 COFF, and the pinned PE. The original
listing supersedes the decompiler's omission of the thrust clamp.
Comparison validates both relocated literals and the `_CIsqrt` import.
[Propeller induced airspeed](sim1.dll/sim1-engine-branches.md#propeller-induced-airspeed).

The aerodynamic pressure and rate scales use the live
`sim1.dll:0x20c00530` listing and input writers, the pinned PE,
and native VC8 COFF. The comparison verifies nine stores, the
quarter-density literal and five exact instruction ranges.
[Pressure and rate scales](sim1.dll/sim1-aerodynamic-forces.md#pressure-and-rate-scales).

The normal airplane aerodynamic force reconstruction in `sim1.dll`
uses live Ghidra listings for the three scalar queries and their
producer, the installed coefficient table, runtime load writers,
and the pinned PE and native VC8 COFF. The vector rotation and
the original sums were checked through the x87 listings. Three
getter bodies and fifteen disjoint instruction ranges match
exactly or after address validation. Full-body byte matching,
linked matching and simulator verification remain open.
[Normal airplane aerodynamic forces](sim1.dll/sim1-aerodynamic-forces.md).

The normal airplane yaw reconstruction in `sim1.dll` uses the
live `0x20c020a0` listing, coefficient slot loads and targets,
the external tank bindings, and the pinned PE and native VC8
COFF. Three complete getters and fifteen instruction ranges
match exactly or after validating addresses. The return ledger
checks the original order of the thirteen contributions.
Linked yaw and simulator verification remain open.
[Normal airplane yaw moment](sim1.dll/sim1-yaw-moment.md).

The normal airplane roll reconstruction in `sim1.dll` uses the
live `0x20c01890` listing, the coefficient constructor and slot
targets, runtime global writers and the pinned PE and native VC8
COFF comparison. The original term order was checked through the
x87 return sequence. The match covers two complete roll accessors
and seventeen disjoint regions. Full-body and linked-image matching
and simulator behavior remain open. The same live constructor
also supersedes the previous pitch slot assignments and the
corrected build retains the original comparison evidence.
[Normal airplane roll moment](sim1.dll/sim1-roll-moment.md).

The normal airplane pitch moment in `sim1.dll` uses the live
coefficient and supplemental dispatch tables, term producers,
and the pinned PE and native VC8 COFF. The original return
sequence was checked against the stack slots where the terms
are stored. Five complete accessor bodies and eleven additional
ranges match exactly or after individually verifying references.
Full-body byte matching, linked pitch comparison and simulator
verification remain open.
[Complete pitch moment calculation](sim1.dll/sim1-aerodynamic-moment.md#complete-pitch-moment-calculation).

The `sim1.dll` elevator trim contribution uses Ghidra listings
and memory, geometry and AIR readers, the provider registration,
and VC8 RTM native COFF against the pinned PE. The arguments,
curve offsets and destinations of both curve calls were checked
individually. The dynamic-pressure curve writer and sample
capacity remain open, along with simulator verification.
[Elevator trim contribution](sim1.dll/sim1-aerodynamic-moment.md#elevator-trim-contribution).

The `sim1.dll` horizontal-tail incidence contribution uses live
Ghidra listings, geometry and AIR loaders, and VC8 RTM native
COFF against the pinned PE. The comparison checks the coefficient
slot, curve call target and both binary64 constants. Simulator
verification remains open.
[Horizontal-tail incidence contribution](sim1.dll/sim1-aerodynamic-moment.md#horizontal-tail-incidence-contribution).

The `sim1.dll` angle-of-attack rate contribution uses live Ghidra
listings and memory, airflow and acceleration field reads, AIR
loaders and VC8 RTM native COFF against the pinned PE. The Mach
multiplier and both angular-rate limits were validated using their
binary64 values. The producer comparison covers the alpha portion
of the two-rate function. Simulator verification remains open.
[Angle-of-attack rate contribution](sim1.dll/sim1-aerodynamic-moment.md#angle-of-attack-rate-contribution).

The `sim1.dll` pitch-rate contribution uses original Ghidra
listings and memory, the velocity integrator, the airflow and
geometry scale producer, and the pinned PE and VC8 RTM native
COFF. The coefficient's source table, both modifier callback
sites and three literal references were reviewed individually.
The physical meanings of the modifier returns and simulator
behavior remain open.
[Pitch-rate contribution](sim1.dll/sim1-aerodynamic-moment.md#pitch-rate-contribution).

The `sim1.dll` elevator coefficient and moment contribution use
live Ghidra listings, control and geometry writers, configuration
and AIR loaders, the hash-pinned PE and VC8 RTM native COFF.
The comparison validates both VTable entries, both lookup target
references and three binary64 literal identities. Simulator
verification remains open.
[Elevator contribution](sim1.dll/sim1-aerodynamic-moment.md#elevator-contribution).

The `sim1.dll` zero-alpha moment coefficient uses live Ghidra
listings and memory, AIR loading and configuration evidence,
and VC8 RTM precise native COFF builds against the pinned PE.
Both normalized ranges validate the original and rebuilt binary64
literals. The optional provider's callback ABI is verified and its
physical quantity remains open. Simulator verification is open.
[Zero-alpha moment coefficient](sim1.dll/sim1-aerodynamic-moment.md#zero-alpha-moment-coefficient).

The `sim1.dll` alpha filter recurrence, step argument and airplane
mode gate were checked using the live Ghidra listing and pinned
PE. The 0.125 s time constant describes its continuous-time
equivalent. No simulator measurement was used.
[Alpha filter](sim1.dll/sim1-aerodynamic-moment.md#alpha-filter).

The `sim1.dll` angle-dependent moment term uses live data-loading,
dispatch and x87 instruction evidence, the pinned PE, and native
VC8 RTM precise COFF builds. Verified scopes cover the interpolation
arithmetic and degree-to-radian conversion. The full lookup ABI,
other coefficients and runtime behavior remain open.
[Angle-dependent moment coefficient](sim1.dll/sim1-aerodynamic-moment.md#angle-dependent-moment-coefficient).

The `sim1.dll` force transfer terms use live instruction and
configuration evidence, the hash-pinned PE, and precise and fast
VC8 RTM native COFF builds. Both reference query tables and their
getter ABIs were checked. Exact and normalized matches are limited
to the documented arithmetic ranges. Simulator verification is open.
[Force transfer to the moment reference](sim1.dll/sim1-aerodynamic-moment.md#force-transfer-to-the-moment-reference).

The `sim1.dll` propeller gyroscopic moment uses live Ghidra
instruction listings, configuration readers, the pinned PE and
VC8 RTM native COFF. The exact range covers arithmetic, the loop
back edge and x87 cleanup inside the original moment function.
The other coefficient terms and simulator behavior remain open.
[Airplane aerodynamic moment](sim1.dll/sim1-aerodynamic-moment.md).

The larger `sim1.dll` flight model entry points use live Ghidra
registrations, constructor and target listings, and the original PE
verification of the airplane model VTable prefix. Individual
aerodynamic terms and runtime target coverage remain open.
[Flight model entry points](sim1.dll/sim1-forces-and-moments.md#larger-model-entry-points).

The `sim1.dll` turbine output helper uses live caller, callee and
query listings and the hash-pinned PE and VC8 RTM native COFF.
The normalized scope includes the high threshold and its branch.
Five original PE and COFF constants were checked individually.
[Turbine speed-scaled output](sim1.dll/sim1-engine-branches.md#turbine-speed-scaled-output).

The `sim1.dll` propulsion collector uses live Ghidra listings and
memory, the hash-pinned original PE and VC8 RTM precise LTCG builds.
The exact linked scope covers the callback's output copies and
return. Both projection calls and the two data operands were
identified for the full-body diagnostic. Runtime verification
remains open.
[Propulsion force collector](sim1.dll/sim1-propulsion-collector.md).

The `sim1.dll` thrust projection uses the live Ghidra caller and
callee listings, a hash-pinned PE and two VC8 RTM precise LTCG
builds. The complete linked comparison verifies four individual
CRT call displacements and the original EBX, ESI and EDI inputs.
Runtime and native COFF verification for this helper remain open.
[Thrust projection](sim1.dll/sim1-engine-branches.md#thrust-projection-and-accumulation).

The `sim1.dll` engine branch mapping uses live Ghidra listings,
callers and configuration references, plus the hash-pinned PE
verification of the six table entries and ten direct calls.
The FSX Acceleration SDK supplies the published engine type names.
Verification covers static branch selection and identified force
paths. The later linked result below the branch mapping covers
`0x20c01010`. Runtime behavior and the larger engine bodies remain open.
[Engine branch mapping](sim1.dll/sim1-engine-branches.md).

The `sim1.dll` force and moment work uses live callback and constructor
cross-references, independently checked slot and target signatures,
the hash-pinned original PE and VC8 RTM LTCG DLL comparisons.
Verification covers the adapter and collector bodies and two scoped
ranges. The dynamic producer set and runtime results remain open.
[Forces and moments](sim1.dll/sim1-forces-and-moments.md).

The `sim1.dll` world-velocity and position writers use live Ghidra
listings, call sites and IAT references, a hash-pinned original PE,
and VC8 RTM native COFF and linked LTCG builds. The whole-body
velocity comparison verifies its CRT import thunk. The two
coordinate matches are bounded ranges. Dynamic virtual targets
and runtime verification remain open. [Position writers](sim1.dll/sim1-position-writers.md).

The complete `sim1.dll` inventory uses a read-only live Ghidra function
manager export, live import and export symbols, and the original
hash-pinned PE. It records the model's saved revision, defined function
bodies and external declarations. Decoded instructions outside those
bodies and subsystem anchors have their stated verification limits.
[Function inventory](sim1.dll/sim1-function-inventory.md).

The `sim1.dll` flight-state path uses live Ghidra MCP function listings,
memory and cross-references, the hash-pinned original PE VTable and
targets, and four VC8 RTM native COFF builds from retained snapshots.
Exact coverage is three whole methods and one explicitly bounded code
range. Source type names and minimum layouts are reconstructed.
[Sim1 state integration](sim1.dll/sim1-state-integration.md).

The `fe.dll` magnetic-variation boundary uses live Ghidra MCP listings
and cross-references, both pinned PE import and export identities, and
two successful VC8 RTM no-entry DLL builds. The 17-byte normalized
comparison covers ordinal 3 and its call target identity. Runtime
equivalence and original source names remain open.
[FE and Sim1 import boundary](shared/fe-sim1-magnetic-variation.md).

The `fsx.exe` application getter at `0x01004384` was checked against
the original PE, seven live Ghidra cross-references, and the packaged
VC8 RTM worker image and MAP. The exact caller bytes and repeated linker
placement are verified. The original ownership of `0x01004380` is
provisional. The new report supersedes the getter displacement in the
November 18 packaged build.
[Linked-byte audit](../../artifacts/fsx.exe/getter-placement-20261119/audit.json).
[Original Ghidra labels and save state](../../artifacts/fsx.exe/getter-placement-20261119/ghidra-state.json).

`fsx.exe` `AfxInitialize` at `0x01002c0d` uses a live Ghidra MCP
listing, a hash-pinned original PE, compiler-checked MFC member offsets,
and a VC8 RTM linked source probe with ICF disabled. Two import
identities establish the normalization. Runtime and native COFF
comparison remain open.
[MFC module initialization](fsx.exe/fsx-mfc-initialize.md).

The `fsx.exe` callback at `0x01001d30` was rechecked against a live
Ghidra listing and the pinned original PE. The job-local VC8 SP1 result
uses compiler binaries extracted from the hash-checked Microsoft
`KB926601` update, the guest's installed headers and the selected RTM
CRT and MFC libraries. The reported residual is for linked machine code.
Runtime behavior and the original toolchain revision remain open.
[Callback evidence](fsx.exe/startup.md#pinned-sp1-and-control-flow-trials).

The `fsx.exe` method at `0x010019e0` uses a live Ghidra MCP listing and
memory export, the hash-checked original executable, and SP1 and RTM
linked worker builds from the canonical C++ source. Original and rebuilt
PE imports, MAP symbols and literal contents establish the 44 normalized
operands. The two-byte OLE control tests the comparator against an actual
source variant. The scope is the complete linked method. The original build
environment and runtime behavior remain open.
[InitInstance evidence](fsx.exe/initinstance-matching.md).

The simscheduler.dll dispatcher at `0x20cc1400` uses live Ghidra
listing and references, a pinned original PE, two VC8 RTM LTCG
builds and a source-identical SP1 control. Three direct call
targets and one HIGHLOW object operand are independently checked.
The match covers the callback body in a linked fragment.
See [event dispatch](simscheduler.dll/simscheduler-event-dispatch-match.md).

The simscheduler.dll internal constructor comparison checks the live
factory call site and the retained VC8 SP1 PE export, call target,
argument setup, constructor extent and unwind actions. The projected
caller models the construction branch only. Runtime and full DLL
linkage remain unverified.

The simscheduler.dll constructor ownership follow-up uses live Ghidra
listings, the original PE unwind maps and VC8 SP1 linked projection
images. It verifies cleanup targets, state maps, and scoped bytes.
Full DLL linkage and runtime behavior were outside the projection.

The simscheduler.dll native context class comparison uses fresh Ghidra
function extents, the pinned PE bytes, base relocations and import table,
and worker-built VC8 SP1 x86 COFF. It checks table slot mappings and scoped
method bodies; DLL linkage and runtime remain unverified.
See [context lifecycle](simscheduler.dll/simscheduler-context-lifecycle-area.md).

The simscheduler.dll first pass uses the original PE export and relocation
directories and live Ghidra listings and decompilation. COFF matching and
runtime behavior remain unverified.
See [first pass](simscheduler.dll/simscheduler-first-pass.md).

G2D depth allocation uses a 409-byte PE-verified original body and
three checked factory and insertion call mappings.
See [depth allocation](g2d.dll/g2d-dxgi-depth-allocate.md).

G2D depth resource return uses 240 PE-verified original bytes and
four COFF call mappings, including main.dll ordinal 61.
See [depth return](g2d.dll/g2d-dxgi-depth-return.md).

G2D DX10 quad constant byte width uses a 16-byte Ghidra export checked
against the pinned PE and a complete relocation-free COFF COMDAT.
See [quad size match](g2d.dll/g2d-dxgi-quad-size-match.md).

G2D DX10 clear-target depth setup uses 735 bytes checked against the
pinned PE, the original texture IID and Windows SDK slot order.
See [clear-target depth view](g2d.dll/g2d-dxgi-clear-target-depth.md).

G2D DX10 target binding uses 366 bytes checked against the pinned PE
and a live read of the second registry tree lookup.
See [target binding](g2d.dll/g2d-dxgi-clear-target-bind.md).

G2D DX10 clear-target registry uses 612 bytes checked against the
pinned PE, original IAT identities and window lifecycle call sites.
See [clear-target registry](g2d.dll/g2d-dxgi-clear-target-registry.md).

G2D DX10 clear entry points use 564 bytes checked against the pinned
PE, original table pointers and Windows SDK COM slot order.
See [clear entry points](g2d.dll/g2d-dxgi-clear-entrypoints.md).

G2D DX10 compositor clear uses 163 bytes checked against the pinned PE,
the profiling thunks and the original wide event name.
See [compositor clear](g2d.dll/g2d-dxgi-compositor-clear.md).

G2D DX10 target clear dispatch uses 302 bytes checked against the
pinned PE, Windows SDK COM slot order and live caller references.
See [target clear](g2d.dll/g2d-dxgi-quad-clear.md).

G2D quad render-state preparation uses 1,137 bytes checked against the
pinned PE, Windows SDK COM slot order and the original quad caller.
See [quad render state](g2d.dll/g2d-dxgi-quad-draw-state.md).

G2D quad drawing uses 579 bytes checked against the pinned PE,
Windows SDK COM slot order and live call references.
See [quad drawing](g2d.dll/g2d-dxgi-quad-draw.md).

GPS legacy lifecycle uses 400 original bytes checked against the
pinned PE, separate string and IAT validation, and live references.
The scoped COFF results describe a block match and the initializer's
register mapping. See [GPS legacy lifecycle](gps.dll/gps-legacy-lifecycle.md).

GPS provider cleanup adds 297 Ghidra listing bytes checked against
the pinned PE, including the original destructor and unwind cleanup
body. The exception metadata and call references limit the scope
of the unwind check to static evidence.
See [GPS provider cleanup](gps.dll/gps-provider-cleanup.md).

GPS provider lifetime uses 1,695 additional Ghidra listing bytes
checked against the pinned PE, separate table reads, and live cross
references. The two new builds have scoped COFF diagnostics.
See [GPS provider lifetime](gps.dll/gps-provider-lifetime.md).

GPS provider registration uses live Ghidra MCP listings and raw bytes
for 179 bytes across two exports, checked against the pinned gps.dll
PE. Export ordinals and IAT identities come from the PE directories.
The 42-byte ordinal 2 result compares to native COFF with
target-validated relocations. See [GPS provider registration](gps.dll/gps-provider-registration.md).

G2D window shared-state reset uses 358 bytes checked against the
pinned PE and separate caller and consumer listings.
See [window state reset](g2d.dll/g2d-dxgi-window-state-reset.md).

G2D window shared-state and quad constant teardown use 287 bytes
checked against the pinned PE and separate table entries.
See [quad teardown](g2d.dll/g2d-dxgi-quad-teardown.md).

G2D DX10 quad constant construction and initialization use 245 bytes
checked against the pinned PE and Windows SDK device slot order.
See [quad constant](g2d.dll/g2d-dxgi-quad-constant.md).

G2D DX10 shader lookup uses 420 bytes checked against the pinned PE
and live call references. See [shader lookup](g2d.dll/g2d-dxgi-shader-lookup.md).

G2D DX10 reflected shader bindings use 663 bytes checked against the
pinned PE and live references for the separate lookup consumer.
See [shader bindings](g2d.dll/g2d-dxgi-shader-bindings.md).

G2D DX10 shader lifetime uses 681 bytes checked against the pinned PE
and live listings of catalogue and define-copy helpers.
See [shader lifetime](g2d.dll/g2d-dxgi-shader-lifetime.md).

G2D DX10 shader creation uses 180 bytes checked against the pinned PE,
separate shader table reads and Windows SDK device vtable order.
See g2d-dxgi-shader-creation.md.

G2D shader compilation and reflection use 1,246 bytes checked against
the pinned PE, two D3DX import thunks, and separate shader table reads.
See g2d-dxgi-shader-loading.md.

Service listener configuration uses 2,059 PE-verified instruction
bytes, eleven PE-verified property GUIDs and the simprop import table.
See api-service-listener-configuration.md.

Application service registration uses 2,203 PE-verified instruction bytes
and three direct startup call sites located in the PE.
See api-service-application-registration.md.

Service connection reset and close use 1,362 PE-verified listing bytes
and 46 bytes read directly for the callback adapters.
See api-service-client-connection.md.

Service transport-tree insertion uses 1,742 PE-verified instruction bytes.
See api-service-transport-tree.md.

Service listener client construction uses 3,859 PE-verified instruction bytes,
both client table slot lists and the auxiliary-list initializer.
See api-service-client-construction.md.

Service transport listeners use 4,376 PE-verified instruction bytes
and original configuration and registry strings.
See api-service-transport-listeners.md.

Service transport lifecycle uses 3,598 PE-verified instruction bytes
and the original string comparison import and registry value names.
See api-service-transport-lifecycle.md.

Outbound packet delivery uses 685 PE-verified instruction bytes.
See api-service-packet-send.md.

Service client open and exception responses use 1002
PE-verified instruction bytes and the original product string.
See api-service-packet-open.md.

Service packet dispatch uses 3379 PE-verified instruction bytes
and separate selector and jump-table checks.
See api-service-packet-dispatch.md.

Receive message handoff uses 830 PE-verified listing bytes,
the base stub and both table slots.
See api-service-message-handoff.md.

Service client buffering uses 1035 PE-verified instruction bytes
and original IAT identities. See api-service-client-buffers.md.

Service client completion uses 632 PE-verified instruction bytes
and separate status selector and jump-table checks.
See api-service-client-completion.md.

Service client transport uses 1517 PE-verified instruction bytes,
API-side import identities and original pipe path evidence.
See api-service-client-transport.md.

Service client delivery uses 1662 PE-verified instruction bytes,
original index header data and live mutation cross-references.
See api-service-client-delivery.md.

Service client registration uses 601 PE-verified instruction bytes
and the original export list header. See api-service-shutdown.md.

Main-window service callback uses 805 PE-verified instruction bytes,
live cross-references and original export strings.
See api-service-shutdown.md.

Service window initialization and routing use 970 PE-verified
instruction bytes. See api-service-shutdown.md.

Service window shutdown uses 805 PE-verified instruction bytes
and the enqueue and initialization callers for layout evidence.
See api-service-shutdown.md.

Callback dispatch and compaction use 420 PE-verified instruction
bytes and stack argument and owner range checks.
See api-service-shutdown.md.

Service shutdown uses 939 PE-verified instruction bytes and
compares owner and callback pairs with the startup registration.
See api-service-shutdown.md.

Main window module shutdown uses 289 PE-verified instruction
bytes and API-side import checks. See api-main-window.md.

Main window completion uses 136 PE-verified instruction bytes
and API-side import and string checks. See api-main-window.md.

Main window creation and graphics startup use 304 PE-verified
instruction bytes and API-side IAT checks. See api-main-window.md.

Startup geometry integration uses 1360 PE-verified instruction
bytes and API-side IAT checks. See api-window-geometry.md.

MAIN startup profile uses 208 PE-verified instruction bytes from
the main-window inventory plus import and string checks.
See api-main-window.md.

Startup class sequencing uses the pinned main-window flow inventory,
including 534 verified instruction bytes in the class phase.
See api-main-window.md.

FS98FLOAT uses live API decompilation, 2170 PE-verified
instruction bytes and the original class string.
See api-panel-float-window.md.

Panel window record creation uses 677 PE-verified instruction
bytes and original class strings. See api-panel-window-record.md.

Menu host initialization uses 901 PE-verified instruction bytes
and API-side import and thunk checks. See api-menu-host-init.md.

Menu host teardown uses live API decompilation, 356 PE-verified
instruction bytes and both 39-slot table spans from the PE.
See api-menu-host-teardown.md.

Menu host creation uses live callback decompilation, 676
PE-verified instruction bytes and API-side import and identifier
checks. See api-menu-host-create.md.

Panel menu commands use live API decompilation, 1296 PE-verified
instruction bytes and separate switch-table and import checks.
See api-panel-menu-command.md.

Panel menu construction and state use live API decompilation,
cross-references, 6888 PE-verified instruction bytes and API-side
IAT and initial-data checks. See api-panel-menu.md.

Panel menu-host lookup uses live API decompilation and xrefs,
1126 PE-verified instruction bytes, the property string and
interface-ID bytes, and IAT identities. Register arguments and
the query output follow assembly. See api-panel-menu-host.md.

Panel surface refresh uses live API decompilation, 5957
PE-verified instruction bytes and API-side import checks.
See api-panel-surface-refresh.md.

Panel geometry and shadow drag use live API decompilation, 5539
PE-verified instruction bytes and the class string and IAT checks.
The COFF result covers compilation and existing regression gates.
See api-panel-geometry-shadow.md.

Panel message routing uses live API decompilation and xrefs, 7962
instruction bytes checked against the PE, and class and IAT evidence.
See api-panel-message-routing.md.

Cursor events use live API decompilation and xrefs, 1792 PE-verified
instruction bytes, the hit-test table and checked IAT identities.
See api-cursor-events.md.

Panel hover text uses live API decompilation and xrefs, 356
PE-verified instruction bytes and the strncmp import identity.
See api-panel-hover-text.md.

Panel hit removal uses live API decompilation and xrefs, 542
PE-verified instruction bytes and the DestroyWindow import identity.
See api-panel-hit-removal.md.

Panel hit registration uses live API decompilation and xrefs and
782 PE-verified instruction bytes. See api-panel-hit-registration.md.

Cursor conversion uses live API decompilation and xrefs, 876
PE-verified instruction bytes and checked API imports and class data.
See api-cursor-conversion.md.

Panel cursor and hover use live API decompilation, 976 PE-verified
instruction bytes and a 32-byte jump table checked against the PE.
See api-panel-cursor-hover.md.

Panel hit and capture dispatch use live API decompilation and 1699
PE-verified instruction bytes. See api-panel-input-hits.md.

Window input dispatch uses live API decompilation and xrefs, 721
PE-verified instruction bytes and the original alternate-window
import identity. See api-window-input-dispatch.md.

Capture and repeat lifecycle use live API decompilation, xrefs,
405 PE-verified instruction bytes and two checked USER32 imports.
See api-timer-interaction-lifecycle.md.

Timer window work uses live API decompilation, 560 listing bytes and
the 84-byte input table checked against the PE. Native COFF verifies
integration. See api-timer-window-work.md.

Startup timer uses the PE-verified initialization inventory, live
callback bytes checked against the PE, and original IAT identities.
Native COFF verifies source integration. See api-startup-timer.md.

Resource strings use live API decompilation, xrefs, registry data and
1337 listing bytes checked against the PE. The API import identity and
native source integration were checked. See api-resource-strings.md.

Product text and shell launch use live API reads, a contiguous
original-byte flow inventory, three PE-verified helper listings and
API import identities. COFF verifies source and boundary emission.
See api-main-window-product-text.md.

Main-window commands use live API decompilation and table/xref reads
and 1201 listing bytes checked against the original PE. Native COFF
verifies source integration. See api-main-window-command.md.

Main-window close uses live API decompilation and xrefs, 2065 listing
bytes checked against the original PE, and PE import identity checks.
Native COFF verifies source integration. See api-main-window-close.md.

FS98MAIN lifetime uses original-verified listings, live constructor
strings and unwind records, and 93 registration bytes checked against
the PE. Native COFF verifies scope and cleanup emission.
See api-main-window-controller.md.

Main-window sizing uses live api.dll xrefs and decompilation plus nine
retained listings totaling 2428 bytes checked against the original PE.
Native COFF verifies source integration. See api-main-window-sizing.md.

Panel WM_PAINT findings use live api.dll disassembly and xrefs plus
524 live bytes checked against the original PE. Native COFF verifies
source integration. See api-panel-paint.md.

Panel bitmap drawing findings use 1082 api.dll instruction bytes
checked against the PE, original EH4 scope and handler, IAT identities
and live xrefs. COFF verifies source integration and emitted handler
references. See api-panel-bitmap-draw.md.

Panel surface lifecycle findings use 1006 api.dll instruction bytes
checked against the PE, original registry and property bytes, import
identities and live xrefs. Native COFF verifies integration and the
previously accepted notification wrapper. See api-panel-surface-lifecycle.md.

Panel-pixel findings use 3229 api.dll instruction bytes checked against
the PE, live xrefs, format and import bytes, and an exhaustive check
of the 16-bit conversion expression. Native COFF validates both shared
DC wrapper bodies and seven relocation operands. See api-panel-pixel-transfer.md.

Panel-update findings use 1279 api.dll instruction bytes checked against
the original PE, live caller/global xrefs, IAT identities and the original
EH4 filter and handler. Native COFF verifies source integration and emitted
handler references. See api-panel-update-regions.md.

Window ordering and update-region findings use 1150 api.dll instruction
bytes checked against the PE, export/import identities and live xrefs.
The COFF gate validates the whole update-region body and six relocation
fields. See api-window-region-order.md.

Clip rebuild and group findings use 1767 api.dll instruction bytes
checked against the PE, export and IAT identities, and live API references.
Graphics receiver slots are described from API call sites. See
api-window-clip-regions.md.

Controller mode findings use 490 api.dll instruction bytes, PE import
and string checks, and live VTable field/target reads. Native COFF verifies
source integration. The g2d.dll ordinal is identified from the api.dll
IAT. See api-window-controller-mode.md.

Dialog-registry findings use 1939 api.dll instruction bytes checked against
the PE, export/import tables, initial registry words and live controller
field and target reads. Native COFF verifies source integration and
regressions. See api-dialog-registry.md.

The 2026-11-19 ordinal 172 follow-up checks a new Ghidra listing and memory
export against the PE export slot and raw bytes, then compares the full
native COFF body and the generated control. This scope covers the lookup
body in `api.dll` at `0x2013bdcc`.
[Callback array lookup](api.dll/api-dialog-registry.md#ordinal-172-record-lookup).

Numbered-name and private-heap release findings use 694 api.dll
instruction bytes checked against the PE, export ordinals, format literals,
import identities and live xrefs. The COFF gate validates the release
body and both relocation targets. See api-numbered-names.md.

Configuration write findings use 5193 api.dll instruction bytes checked
against the PE, export ordinals, import identities, formatting literals
and live references. The COFF gate validates both narrow writer bodies
and their call targets. See api-stream-parser-writes.md.

Configuration query findings use 2303 api.dll instruction bytes rechecked
against the PE, export ordinals, import identities, the numbered-section
format and live callers. Native build verifies integration and regressions.
See api-stream-parser-queries.md.

Stream parser findings use 2245 bytes of api.dll instruction exports
rechecked against the original PE, live caller xrefs, and PE unwind
funclet inspection. Native builds verify source integration and existing
gates. See api-stream-parser.md.

Keep reconstruction sources, native runs and accepted-byte ledgers
separate by binary. Record a cross-DLL dependency at its caller, and
attribute callee reconstruction evidence to its own binary.

Stream lifecycle and size-consumer evidence uses original PE bytes, listing
exports and live caller and CopyTo slot/target checks. Two complete code
bodies are accepted by the cache lifecycle COFF gate. The util.dll bit
reader, back-reference and exception paths are static PE findings.
See api-file-stream-lifecycle-consumers.md.

Decoder and cache evidence uses original-verified api.dll listings, live
caller and lock xrefs, and local original util.dll PE disassembly. Native
COFF inspection and both 15-check runs establish source integration and
regression coverage. No runtime or complete transform equivalence claim.
See api-file-stream-decode-cache.md.

File stream selection uses original-verified instructions, pinned PE mode
table and API export/import bytes, local util.dll export bytes and live
caller checks. Source build verifies integration and existing gates.
See api-file-stream-open.md.

CAB extraction uses live FDI callback and consumer xrefs, original-verified
instructions, pinned PE strings and VC8 SDK declarations. Callback completion
is distinguished from library return status. See api-cab-extraction.md.

CAB owner metadata uses original-verified Stat, reload and lookup listings
with live slot and caller xrefs. Static failure analysis distinguishes the
file Stat return from the wrapper's unconditional success. The native build
verifies source integration and prior gates. See api-cab-owner-metadata.md.

CAB buffer containers use original-verified listings, pinned PE bytes for
the registration body, table, suffix and spin-count import, plus live checks
for the cleanup callback and owner xrefs. The 17-byte body gate verifies
three target fields. Owner failure findings are static. See
api-cab-buffer-containers.md.

Stream table evidence uses eleven original-verified listings1486 bytes,
PE table and identifier bytes, live signature checks and native COFF gates.
Five bodies add86 code and28 target-validated fields add112 metadata.
Fourteen gates pass at4918/288. See api-cab-stream-tables.md.

Pool registration uses live read-only callback/table/startup spans, pinned PE
bytes, CRT and unwind anchors and actual COFF builds. Eleven raw bodies97
bytes and 33 target-validated table fields132 bytes add separate coverage.
Thirteen gates pass at4832/176. Source cleanup intent is distinct from absent
emitted unwind handlers. See api-cab-pool-registration.md.

Pool acquisition uses the original-verified20146e07/20145ca9/20145d2a listings,
branch-level transition review and emitted wait/spin diagnostics. Twelve
gates retain4735/44. Source covers fixed pool call arguments. Runtime
concurrency and general acquisition behavior remain outside verification.
See api-cab-pool-acquire.md.

Lock/owner lifecycle uses fifteen original-verified listings covering5606
bytes and actual VC8 builds. Original release and lifecycle paths are source
models with shared CAS64. Acquisition evidence covers selected pool mode
contracts. Twelve gates retain4735/44. See api-cab-lock-lifecycle.md.

Slot transfer uses six original-verified listings covering1552 bytes and
compiled COFF checks for both forms of three atomic instructions. These checks
establish emitted instruction forms and twelve existing body gates. Runtime
concurrency equivalence remains open. See api-cab-slot-transfer.md.

Slot storage analysis verifies nine original listings covering1544 bytes.
Lifecycle, accounting and chunk handling build as source models with explicit
atomic primitives. Twelve gates retain4735/44. Equal body lengths at20145d4c
and20145e2f do not establish equivalence. See api-cab-slot-storage.md.

Shared pool source follows original block and caller lock contracts, including
failure and upgrade paths. Seven listings covering 4902 bytes are original
verified. Actual COFF builds retain twelve regression gates. The shared helper
design is outside body acceptance. See api-cab-pool-locks.md.

2026-11-09 cache and pools. Eight listings covering2357 bytes reverify against
the original PE. Pool comparison checks instruction mnemonics, corresponding
branch destinations and operand differences. Actual COFF builds establish
cache source compilation and twelve regressions. See api-cab-buffer-pools.md.

2026-11-09 stream methods: six listings861 original bytes, selected VTable targets
and two IAT identities verified. Full57+64 import-normalized and92 raw bodies,
rejecting controls and generated-body overlap checks add213 (total4735/44).
Twelve gates pass; source field semantics corrected by read/seek usage, no DB
renames. Other three methods compile unmatched. See api-cab-stream-methods.md.

Stream producer checkpoint: twelve original-byte listings1624 bytes, selected
VTable and unwind metadata, live state/signatures, actual source/COFF/receipt
support observed ownership/return contracts. Four new bodies compile unmatched;
eleven gates preserve4522/44. File-handle leak and owner undefined edge are not
repaired. See api.dll/api-cab-stream-producers.md for source ABI/EH limitations and hashes.

CAB image/stream: eight original-byte listings1568 bytes plus live state/signature
check.33+39 raw-equal complete wrappers, six rejecting controls and generated
overlap checks; eleven gates retain4522 complete code/44 metadata. Image source
671/700 unmatched. Owner reload uninitialized-stack failure edge is assembly
evidence, not silently fixed source. See api.dll/api-cab-image.md for hashes and limits.

CAB owner/cache follow-up: five listings949 bytes original-verified plus raw
FuncInfo/unwind map/action, live read-only action readback. Source normal-flow
builds and native disassembly distinguish source placement-delete intent from
absent emitted EH cleanup. Ten gates retain4450/44; no newly accepted bodies.
See api.dll/api-cab-owner-cache.md for hashes, argument contracts and explicit limitations.

2026-11-08 CAB enumeration: .CAB discovery/MSCF reader and eight original-byte
verified listings support backend identity and cursor/entry offsets. Three real
source implementations compile; ordinary source ABI is not original ESI/EDI/EAX
ABI. Ten gates preserve4450/44; no new accepted bodies. Evidence artifact and
object hash are in api-alternate-enumeration.md. Original RT_STRING extraction
rerun corroborates title/monitor resources; see api.dll/api-module-references.md for
hashes and configuration limits. No DB edits, live saved state confirmed.

The 2026-11-19 `api.dll` checked arithmetic and ordinal 160 comparisons use
fresh Ghidra listings and memory reads checked against the pinned PE.
The checked 32-bit add and both checked subtraction bodies are byte-exact
in native COFF. The 64-bit subtraction forwarder has one validated
REL32 to the checked subtraction, and its full body matches after that
field is resolved. For ordinal 160 the PE export slot was checked
independently, and the COFF and linked fragment both retain the
alignment difference.
[Checked arithmetic](api.dll/api-checked-arithmetic.md).
[String comparison](api.dll/api-string-compare.md).

The 2026-11-24 `api.dll` unsigned comparison follow-up uses Ghidra
listing and memory bytes checked against the pinned PE, native
VC8 RTM and pinned SP1 COFF objects, and a linked RTM LTCG fragment.
The selected native and linked bodies have the same ten differences.
[Unsigned comparison](api.dll/api-checked-arithmetic.md#unsigned-32-bit-comparison-at-0x20153784).

The 2026-11-24 `api.dll` range check at `0x201513ba` uses fresh
Ghidra function and caller listings and a memory read checked against
the pinned PE. The native VC8 RTM COFF comparison verifies all 21
bytes. The caller and `RET 8` establish the two stack slots.
[Signed-to-unsigned range check](api.dll/api-checked-arithmetic.md#signed-to-unsigned-range-check-at-0x201513ba).

The 2026-11-24 construction-record block in `api.dll` uses fresh
Ghidra listings for eight constructors and eight copy targets. The
PE verifies all 400 instruction bytes and 196 initialized data bytes.
The COFF comparison covers the 160 constructor bytes and their
individual call relocations. The copy targets are external build
dependencies. [Constructor block](api.dll/api-construction-records.md).

The 2026-11-24 default-copy follow-up matches the eight original
target bodies using fresh Ghidra and pinned PE bytes, VC8 RTM COFF
symbols and individually validated DIR32 relocations. Native `.bss`
and the original data bytes establish the initial zero values for the
eight copied ranges. The independent factory replay verifies the prior
caller bodies. [Default-copy block](api.dll/api-construction-records.md#default-copy-block-at-0x20161267).

The 2026-11-24 `api.dll` pointer-owner update block uses fresh
Ghidra function and dependency listings verified against the pinned
PE, live release-path reads and native VC8 RTM COFF with four verified
REL32 fields. The comparison certifies the four caller bodies and
only the in-scope callee body at `0x2016184b`.
[Pointer owner updates](api.dll/api-pointer-owner-updates.md).

The release follow-up verifies six `api.dll` bodies and the two
original free endpoints from live Ghidra and the pinned PE. Four
direct call targets and the `UnmapViewOfFile` IAT identity are
verified in the native COFF comparison. The exact virtual-call
body establishes its slot and call sequence. The runtime dispatch
target remains open.
[Release follow-up](api.dll/api-pointer-owner-updates.md#release-and-replacement-follow-up).

The heap bridge follow-up checks four fresh Ghidra listings and the
private heap cell against pinned `api.dll` PE bytes. Native VC8 RTM
COFF validates seven fields against the original global, named PE
imports and internal call. The earlier callers are replayed with the
new endpoint symbols defined.
[Heap bridge endpoints](api.dll/api-pointer-owner-updates.md#heap-bridge-endpoints).

The adjacent owner adapters use fresh `api.dll` function and target
listings checked against the pinned PE. The native VC8 RTM object
validates five internal REL32 targets and the named DIR32 imports
`InterlockedIncrement` and `LeaveCriticalSection`.
[Adjacent adapters](api.dll/api-pointer-owner-updates.md#adjacent-pointer-owner-adapters).

The mapped-view and handle transfer scope uses fresh Ghidra function
and dependency listings verified against the pinned `api.dll` PE.
Native COFF validates nine call targets and the named `CloseHandle`
IAT entry. The predicate body is outside the reconstructed scope.
[Mapped view and handle transfer](api.dll/api-pointer-owner-updates.md#mapped-view-and-handle-transfer).

The activity and lock lease comparison uses fresh `api.dll` Ghidra
listings checked against the pinned PE and native VC8 RTM COFF
with five verified REL32 fields and ten named IAT fields. The wait
helper's two IAT fields are verified separately from the remaining
register-selection differences.
[Activity and critical-section leases](api.dll/api-pointer-owner-updates.md#activity-and-critical-section-leases).

The buffer owner forwarding and reset comparison verifies four
live `api.dll` listings and four dependency bodies against the
pinned PE. The XP VC8 RTM object and previous profile gates verify
the four direct call targets. The reset establishes the two write
offsets and their order. Field roles remain provisional.
[Buffer owner forwarding and reset](api.dll/api-pointer-owner-updates.md#buffer-owner-forwarding-and-reset).

The mapped file and interface cleanup comparison verifies three
live `api.dll` bodies and three dependencies against the pinned
PE and the native XP VC8 RTM object. The comparator validates three
direct calls and one direct tail jump. Return values of the stack
adapters and the mapping handle predicate remain unverified.
[Mapped file and interface cleanup adapters](api.dll/api-pointer-owner-updates.md#mapped-file-and-interface-cleanup-adapters).

The VC8 COM pointer diagnostic compares live `api.dll` listings and
bytes checked against the pinned PE with native VC8 RTM /Ob0 and /Ob2
objects. The XP logs identify the installed headers, and the report
retains the source, command and object hashes. Aligned call operands
establish caller encoding only when the callee differs.
[VC8 COM smart-pointer provenance check](api.dll/api-pointer-owner-updates.md#vc8-com-smart-pointer-provenance-check).

2026-11-07 broad source pass: live read-only Ghidra exports for all5077 internal
functions and789 external declarations; raw original-byte verification of
321805 instructions/879524 bytes. Full listing separately records7406 unassigned
instructions. Raw decompilation is evidence, not compilable-source/completeness
proof. Exported inferred global types are not original declarations. See
api.dll/api-source-pass.md and artifacts/api.dll/api-source-pass-20261107.
Actual VM source/log/receipts and COFF show567 generated candidates plus nine
manual additions compile.88 fixed generated body gates cover3116 bytes,
including46 raw-equal bodies/1935 bytes; foreground-idle hook214 separately
checks13 fields and opcode/addend/callee rejecting controls. Direct PE import
parsing verifies828 IAT identities. Ten regression comparators pass on run
api-source-message-loop-20261107-001; combined4450 complete code/44 metadata.
Compilation, model-body equality, dependency implementation and final DLL
equivalence remain separate.193 native-contract blockers and other deferred
categories are retained; no placeholders certify missing behavior. Final live
API readback has unsaved_changes=false/modification3; no database changes.

Font/enumeration checkpoint: original api.dll pinned hash, inspected assembly,
native source objects and full close comparator with rejecting controls support
the additional92 bytes (1120 complete code total). All eight gates rechecked
2026-11-07. Fresh live signature readback confirms five edits persisted and
unsaved_changes=false; earlier save timeouts are superseded by this check.
See api.dll/api-font-resources.md for addresses, object hash, backup and limitations.

Geometry/language: raw target/callback/dependency assembly, SDK sizeof/offsetof
assertions, actual native source builds and saved type readbacks in geometry
artifacts/topics. Four post-edit code spans equal original; no source matches
accepted for the new routines. Language flow729+128 bytes original-verified,
integrity-success assumptions explicit; source357 is normal-flow model only.
Seven gates retain1028/44 on language run. Color-depth/monitor layout and actual
1/E_FAIL response supersede earlier decompiler shape/retry interpretations.

Main-window phase: raw bounded success-flow inventory1346 bytes/112 descriptor
bytes and five live/original spans; integrity helper success+16/failure redirection
distinguished. New188-byte hook native match with8 mappings/negative controls;
100-byte child class rejects sole frame-byte difference. Seven accepted-body gates
retain1028 complete code/44 metadata. GZF, actual helper typing/evidence comment,
readback/save revision603 retained; main body expansion/decompiler completeness
not claimed. See api.dll/api-main-window.md for runs/hashes and next contracts.

Splash image/path checkpoint: raw image return/control/literals, source trial
and helper storage evidence retained. Full append62/length34 comparator with
rejecting controls; six gates retain840 disjoint complete code/44 metadata.
Image488/445 source is unmatched. Four actual Ghidra interface edits with GZF,
readbacks/save revision567 and post-edit image-byte equality; no runtime proof.

Structured splash slice: retained original procedure/image decompilation and
position-helper assembly, actual source build and raw body diagnostics. Procedure
620/475 and start225/227 remain unmatched; rectangle custom ABI explicit. Five
gates retain744/44. Removing canonical goto changes source, not historical runs
or decompiler output. Image return/control requires raw verification next.

Autonomous disk-query/splash slice: actual source builds and retained original
assembly/decompiler snapshots. Full98-byte splash-stop comparator12 fields,
class string and three rejecting controls; latest five-script regression covers
744 disjoint complete bytes. api.dll/api-code-ledger.md corrects prior arithmetic648 to
646 before adding98. Query/start/thread source builds remain unmatched; no
coverage for compile-only work, runtime or entire linked module. See topic.

Legacy fallback: live125-byte original assembly, two retained native125-byte
trials with identical emission and full rejecting comparator. Signed-widening
and output-pointer behavior are assembly verified; runtime/API edge behavior
unexecuted. No new coverage;648/44 gates retained. See initialization topic.

Disk dispatch: live assembly/pointer, original-file initial pointer, actual two
native source forms, full139-byte/12-field comparator with string checks and
opcode/reference/error-constant controls. Run/hash and gate64MiB/30MiB dataflow
in api-initialization-flow.md. No fallback implementation/runtime/link claim.
Complete disjoint code648; metadata44, Initialize partial123 separate.

Initialization error wrapper: live48-byte assembly and actual retained COFF trial,
three DIR32 mappings, raw original PE ordinal264 lookup and three rejecting
mutations in compare-api-initialization-error.cjs establish complete object body.
See api.dll/api-initialization-flow.md for hash/run; unresolved callback/data semantics
and final linked export remain outside scope. Total disjoint complete code507.

Suffix source trial: live typed/decompiled original evidence, retained native
251-byte COFF body/reference diagnostics and source snapshot. Custom ABI and
copy inlining differ from original182-byte body; no new coverage. See topic
milestone for full run/hash/special cases.459/44 previous gates retained.

Copy boundary: live86-byte original assembly and explicit storage, retained
native82-byte objects from two trials and copy-diagnostic.json bytes show identical
emission after empty-condition cleanup. Source/assembly semantics inspected;
custom ABI is unmatched, no runtime or coverage acceptance.459/44 gates retained.

Resolver source trial: actual native238-byte COFF and eleven reference fields,
retained source/command/hash and original copy20137e17 assembly documented in
api-initialization-flow.md. Two source interfaces explicitly differ from custom
original register ABI: no equivalence mapping/coverage acceptance. Existing
459/44 gates retained; no candidate execution or linked DLL assertion.

Cookie model: exact original/live15-byte check, failure-target inspection,
compiler-spec fixup inventory and before/after caller decompilation retained in
api-cookie-model artifacts/topic. Built-in fixup affects decompiler successful
return modeling; it does not certify failure impossible or remove original check.
GZF, applied script, ECX storage/save readback retained. No new matching bytes.

Seven path signatures actually updated by TypeApiPathInterfaces.java after GZF;
explicit register/stack storage assertions plus target/decompiled and saved-state
readback retained. Names and code bodies unchanged. Remaining cookie-helper
return artifact is flagged independently, not accepted as source behavior.

Resolver/selector contracts: live API assembly, original-file70-entry suffix
inventory and live560-byte table snapshot retained; see api-initialization-flow.md.
Custom EAX/ECX/EDI/EDX interfaces verified separately from misleading decompiler
prototypes. Startup .CFG/.BGL paths inferred from static transformation; runtime
file existence/dynamic suffix table values remain unverified. No new match.

API attribute dependency: actual retained202-byte native source build, seven
reference diagnostics and live dependency assembly in api-initialization-flow.md.
Equal length is not equality: original/native frame offsets and scheduling differ.
No new coverage;459 complete object bytes retained by existing gates. Decompiler
fastcall/void contracts are superseded by stack/RET/EAX evidence where described.

API-05b wrapper: original assembly and actual retained native trial establish
49-byte20132c8e body equality with one validated2013341b relocation and rejecting
opcode/target controls; scripts/compare-api-path-wrapper.cjs. Both0/-1 return
contracts verified from assembly, independently of stored undefined signatures.
See api.dll/api-initialization-flow.md milestone for run/hash and unresolved dependency.

## API initialization native flow inventory (2026-11-03)

Read-only PseudoDisassembler worklist, exact observed stack-transfer templates,
independent gateway/helper assembly and pinned PE/IAT identities documented in
api-initialization-flow.md. All465 instruction rows and seven live memory spans
equal original bytes; descriptors and skipped ranges are excluded from native
instructions. Both helpers' saved-return +8 writes verified in assembly. Existing
helper decompilation supports provisional region-processing interpretation only.
No decoded image/protection-product/full-function/runtime/source match claim.
Preliminary133-instruction extent superseded by465 after transfer resolution.
Stored void/undefined return declarations remain separately flagged, not repaired
from decompiler output alone. No database changes; saved revision rechecked.

## GetApplication native factory / four-slot table (2026-11-03)

Live original getter/table-prefix/zero-flag bytes and PE export144 association;
actual two controlled native COFF source forms, source/command hashes and full
72-byte/7-relocation comparison plus generated16-byte table references in
api-factory-matching.md. Negative allocation-size/target/slot controls reject;
prior object and linked window regression gates retained. RTTI-less bridge shape
does not prove full DLL flags/class/source. Unresolved globals/callees and linked
export identity remain outside object scope. GZF/comment/save readback recorded.

## Callback registry / reallocation dependency source (2026-11-03)

Live original185-byte realloc plus318-byte registry/related dispatch reads,
actual native COFF builds and reference/critical-path diagnostics recorded in
api-callback-registry.md. Fourteen reference identities and selected NULL-retry/
mode4 shapes checked; no new whole-body or runtime equivalence claim. Source
visibility control leaves all4 native code sections unchanged. Prior exact/scoped
gates pass shared-reader regression. Pre-edit GZF, actual realloc name/prototype
edit, saved readback and live/original byte equality documented in that topic.

## API cabinet FDI error record (2026-11-19)

The 2026-11-19 `api.dll` `ErfSetCodes` result uses a fresh Ghidra
listing and memory read checked against the pinned original PE, the VC8
SDK `fdi.h` record, and a native RTM COFF byte comparison. The Ghidra
signature and C++ source reconstruction are recorded separately.
[FDI error record](api.dll/api-cab-error.md).

## Initialize / FDI stream object evidence (2026-11-03)

Installed SDK IStream/fdi.h declarations, actual native source builds and raw
COMDAT checks establish140 complete adapter bytes, keeping prior177-byte gates.
Initialize21 relocation mappings and123 disjoint bytes are partial evidence only;
remaining ESI/ECX ABI/scheduling visible. Original helper/related list-dispatch
assembly supports explicit ESI owner and callback-list interpretation. Scoped
Ghidra edits, pre-edit GZF, storage/readback/save and post-edit byte identity in
api.dll/api-initialize-fdi.md; concrete stream class/IID/original source names unproven.

## API lifecycle native object / coordinated types (2026-11-03)

Actual VC8 native COFF sections/symbols/relocations and pinned original PE bytes
in compare-api-lifecycle-object.cjs establish177 code/28 metadata bytes at object
evidence level, with opcode/addend/target negative controls. No linked-callee
implementation proof. Independent original EH4 import identity and table targets
checked. Header DLL regression retains existing21-byte gate. Scoped Ghidra script,
pre-edit GZF, separate VTable/target readbacks and saved revision in
api-lifecycle-matching.md. Earlier char* retained-state statements superseded.

## API first body pilot / EH and lifetime (2026-11-03)

Actual VS2005 SP1 .762 worker DLL builds and byte/MAP/global checks in
compare-api-window.ps1 establish a complete21-byte normalized body match;
opcode/target mutations reject. Native/GL heap controls use identical sources
and controlled flags, with unchanged symbolic schedules, not heap match evidence.
ApiBoundaryEvidence.java and live/raw snapshots verify EH zero-return and
shutdown reset; all new memory spans equal pinned original bytes. GZF/readback/
save and actual versus suggested edits are in api-application-boundary.md.

## API.DLL initial work packages (2026-11-03)

Pinned original PE parsed by scripts/inventory-api.ps1 using existing pe-image.ps1;
export144 and VTable independently checked on disk and live. Saved Ghidra function,
memory, type and xref snapshots in artifacts/api.dll/api-boundary-*-20261103.json; all
three captured memory ranges match the original file. See api.dll/api-application-boundary.md
for normal-path EAX evidence and its exception-path limit. Existing names/types
remain analysis state, not original declarations. No rebuild/body match or DB edit.

## Baseline41 consolidation and bounded code trial (2026-11-03)

Exact original/archive41 SHA256 checks, archived same-binary comparator scopes,
fresh raw range/vtable checks and overlap-preserving partitions are recorded in
coverage-baseline41.md/JSON. Ghidra snapshot classifications remain historical,
separate from binary evidence; live InitInstance revision read confirms saved state.
Actual one-job module-instance-local trial, compiled-source/command comparison,
full regression reports and whole-section raw delta verification establish the
five-byte load-order improvement; see initinstance-matching.md. The new candidate
hash is separate from41's ledger. No whole-function/runtime/original-source proof.

Consolidated 2026-10-31. IDs support subsequent read_thread calls. Original titles are retained verbatim for identification.

| ID | Original title | Reviewed coverage |
|---|---|---|
| S1: 01a1850b-9375-7842-a467-8bed745042ba | Finde Startfunktion in fsx.exe | All available pages through hasMore=false: 12 pages, 116 turns |
| S2: 01a1926f-a685-70e0-8b75-b04fa364897d | local_c und Wertzuweisung erklären | All 4 available turns |
| S3: 6abee7a8-41f4-83eb-90d7-13550c710801 | FSX Decomp Starten | 5 available turns; hasMore=false |
| S4: 01a1924d-e3c1-78c3-ba00-6da99771432d | Project Config | Current chat: MCP setup and consolidation |

S3 refers to attachments whose contents were not returned. Its statements are historical reports, not freshly verified assembly evidence. hasMore=false does not establish completeness of the original ChatGPT conversation.

The current chat list and local archived Codex history were checked. This does not claim exhaustive coverage of all historical chats across every host/project. Unrelated topics were excluded.

## Live verification L1

Read-only ghidra-bethington checks on 2026-10-31:
- list_open_programs and get_metadata for fsx.exe.
- get_functions with signatures/decompilation for 010019e0, 01001660, 0100263d, 010018e0, 0100262e, 01002363.
- get_struct_layout for Application, ApplicationVTable, FSXApp, CWinThread, COleMessageFilter, COleMessageFilterVTable, AFX_MODULE_STATE.
- find_data_types for full structure paths.

Only fsx.exe was open. No api.dll verification, Ghidra edits, or save operations. Metadata supplied no binary hash. Existing names establish the analysis state, not independently their semantic correctness.

## Evidence labels

- Live verified: visible in the current Ghidra model/decompilation; respect stated scope.
- Historically reported: extracted from S1–S3, not currently rechecked.
- Hypothesis: plausible interpretation; original name, intent, or complete layout unproven.
- Superseded: subsequently corrected interpretation; do not reuse as a starting assumption.

## Finding template

Date; binary/hash if known; address or full type path; finding; evidence (instruction, data flow, xref, tool); status; actual edit; open questions; source chat ID.


## Static initialization evidence (2026-11-02)
Live fsx.exe CRT startup, wrapper listing, initializer-table and __onexit reads; actual XP worker jobs static-order-20261102-001 and static-default-20261102-001; target-validated PE/MAP/import comparisons in compare-static-glue.ps1. Source order correction, negative control, extra XIY difference, bounded Ghidra edits, pre-edit GZF and saved revision are documented in static-initialization-matching.md. Earlier cut/inventory counts are historical and superseded by that update; no API changes or runtime execution.

## CRT library context evidence (2026-11-02)
Live original table/import/model reads; installed SP1 library member COFF headers/symbols/relocations returned by XP diagnostic jobs, especially crt-table-probe-20261102-001. Local VS2005 DVD CAB extraction and pinned release library hash, source-identical failed-order and corrected actual builds, fresh default and negative SP1 table control. See fsx.exe/crt-library-context.md for binary/address/member provenance, diagnostic path corrections, Ghidra comment changes/save and limitations. Resolves the previous open XIY entry; does not prove original compiler revision or complete CRT body matching.

## pre_c_init evidence (2026-11-02)
Live original memory/pseudo decoding at01002547/010027d9/0100299e/010029cc; PE/MAP/import/state comparisons in compare-pre-c-init.ps1 against30 and SP1 control29. Real worker pre-c-init-20261102-001 and rejected literal mutation control. Bounded database script, exact live-body hash, backup, readback and saved revision in pre-c-init.md. Raw dependency matching does not imply complete stored Ghidra dependency functions; new one-byte placeholders explicitly remain open.

## pre_cpp_init and EXE gap assessment (2026-11-02)
Live fsx.exe original listing/pseudo decoding, PE/MAP/import/state checks in compare-pre-cpp-init.ps1, real XP pre-cpp-init-20261102-001 and rejected error-literal mutation. Hash-bounded Ghidra completion script, approved rename, backup/readback/save in pre-cpp-init-and-exe-gaps.md. EXE roadmap separates normalized body evidence from exact data/linker layout; inventory rows and byte classifications are not match percentages.

## Full CRT startup region evidence (2026-11-02)
Native Ghidra tool access restored after restart; live explicit fsx.exe pseudo decode and boundary/state checks, original PE/MAP/import comparisons in compare-crt-startup.ps1, baseline30 and SP1 control29, real XP crt-full-startup-20261102-001, two rejected body/EH pointer mutations. Backup, scoped listing edits, parent-versus-funclet distinction and saved revision documented in full-crt-startup.md. Entire CRT/callee/layout/runtime equality remains outside scope.

## CRT image/dispatch evidence (2026-11-02)
Live fsx.exe memory/listing/prototypes and read-only pseudo decode; compare-crt-image-helpers.ps1 PE/MAP/import/state/EH checks; real XP crt-image-helpers-20261102-001 and two rejected mutations. Bounded script plus native minimum-ABI prototype, backup/readback/save in crt-image-helpers.md. Documented EDX preservation and analysis-only inline model; not original source/compiler inline proof. Imported dispatcher and report_gsfailure body remain out of scope.

## GS failure/stack evidence (2026-11-02)
Live fsx.exe assembly and standard Ghidra Windows layouts _CONTEXT716/_EXCEPTION_RECORD80/_EXCEPTION_POINTERS8; PE/MAP/import/data comparisons in compare-crt-failure-stack.ps1, real XP crt-failure-stack-20261102-001, two rejected code/data mutations. Pre-edit GZF, bounded typing/comment script, automatic label effects, saved readback and exclusions in crt-failure-stack.md. Specialized stack-helper ABI preserved; no full EXE/layout/runtime claim.

## CRT exit-registration evidence (2026-11-02)
Live fsx.exe code/types, standard _onexit_t shape, PE/MAP/import/state/EH checks in compare-crt-onexit.ps1; real XP crt-onexit-20261102-001 and two rejected mutations. Backup, renamed helpers, separate callback type paths, parser fallback, saved readback and scope limits in crt-onexit.md. Imported runtime implementations/runtime execution and full layout remain outside scope.

## ATL codepage evidence (2026-11-02)
Live fsx.exe listing/state/type readbacks, PE/MAP/import comparisons in compare-atl-codepage.ps1, atls:atltrace.obj symbol mapping and VC8 local linker help. Real XP atl-codepage-20261102-001 and two rejected code/pointer mutations. Backup, namespace/type/cache changes and saved revision documented in atl-codepage.md. Prior real-getter fragment interpretation superseded; padding/linker option remains an explicitly untested hypothesis.

## Remaining runtime/metadata evidence (2026-11-02)
Live explicit fsx.exe pseudo/listing/external-signature/data reads, original PE
import bytes, baseline30 MAP/member provenance and compare-runtime-remainders.ps1;
fresh XP runtime-remainders-20261102-001 and two rejected mutations. Pre-edit GZF,
hash/model-bounded completion/type script, instruction-by-instruction readback,
save and unsaved=false recorded in runtime-remainders.md. Prior precision41 stored
body statement superseded by38 reachable/41 compared distinction after noreturn
propagation. No original typedef-name, padding-option or whole-image proof.

## Function-padding evidence (2026-11-02)
Original PE bytes and baseline30 known-body extents; actual XP jobs padding-control,
padding-five and padding-five-verified-20261102-001 (full IDs in topic); identical
source hashes and single-token generated-command delta, MAP-resolved CC gap
measurement in compare-function-padding.ps1. Updated full regression reports,
InitInstance diagnostic and rejected gap-only mutation. See function-padding.md.
Option hypothesis now experimentally supported; original command, full ordering
and whole image remain unproven. No Ghidra database changes in this experiment.

## EXE RTTI absence and /GR- evidence (2026-11-02)
Live explicit fsx.exe listing/anchor values1394/1398/1454/1458, original PE scans,
actual enabled31 and rtti-disabled/default-20261102-001 XP builds, source hashes
and exact compiler-command delta; MAP/type-name/COL/import comparisons through
compare-rtti.ps1. Real enabled control rejected; all body regressions retained.
Pre-edit packed DB, bounded comment script, explicit save/unsaved=false readback
in exe-rtti.md. Supports reconstruction RTTI mode, not unique original flags or
external DLL metadata/full layout. MFC runtime-class metadata remains separate.

## EXE constants and initial-data evidence (2026-11-02)
Original PE and live Ghidra token/load-config/SafeSEH/zero-storage anchors and
existing types; baseline32 MAP literal/member/symbol identities, original/rebuilt
PE directory fields and function/import mappings. compare-data-constants.ps1,
actual XP data-constants-20261102-001 and three rejected binary controls. Pre-edit
GZF, bounded comment-only script and save/unsaved=false readback recorded in
exe-data-constants.md. Token group counterpart now verified; original name/parser
semantics and complete data/layout coverage remain unproven. No new body match.

## Complete imports and PE inventory evidence (2026-11-02)
Original PE versus baseline32/fresh actual XP pe-imports-20261102-001, full
bounded descriptor/ILT/IAT/hint parsing and PE header/directory/section/raw-debug/
certificate-range reads via compare-pe-imports.ps1. Native original hashes for
IAT/descriptors/debug and two external-data references. Two rejected mutations
and actual-order rejection; pre-edit GZF, comment-only edit, save/unsaved=false
in pe-imports-and-directories.md. Contents match; order/header/layout/debug/cert
remain unmatched. Certificate cryptographic status/signer not verified.

## Original image-base evidence (2026-11-02)
Original PE base versus actual XP image-base-original/controlx-20261102-001;
source hashes/exact command delta, matched-length PDB records and343 matched MAP
RVAs via compare-image-base.ps1. Full existing gates and both wrong-base controls;
legacy comparator dynamic-base updates also checked on32. Backup of original DB
not needed because no DB edits; last saved state unchanged. See image-base.md.
This proves reconstruction base equality, not source address/order/full-layout
or original complete-command identity. Debug path/GUID/timestamp differences stay
explicitly open.

## Read-only section merge evidence (2026-11-02)
Original PE section/directory fields versus actual XP sections-mergedx/control-
20261102-001, source hashes/single-option commands and equal CodeView record
lengths. Full existing gates, compare-section-merge.ps1 target/data/import checks,
verify-section-layout.ps1 and two actual wrong-mode rejections. Details/retained
baseline34 in section-merge.md. Raw extents/placement match, virtual sizes/content
and import/code order do not. No Ghidra DB changes or runtime execution.

## PE stack header evidence (2026-11-02)
Original PE fields, actual XP stack-original-20261102-001, identical source hashes
and sole /STACK command delta against34; startup field check, actual old/new
wrong-mode controls, full regressions and diagnostics. See stack-header.md. No
whole-layout/runtime/unique original-command claim or Ghidra edit.

## Basename PDB debug-record evidence (2026-11-03)
Actual installed SP1 linker logs and PE output from pdb-basename/longer-ID repeat
jobs; original and baseline35 controls. Identical source hashes/single-option
commands, full regressions, verify-debug-record.ps1 and compare-pdb-path.ps1
structure-derived timestamp/GUID repeat comparison, actual wrong-mode rejection.
Details/hashes/default36 in pdb-path.md. Older RTM help absence does not disprove
measured installed-linker support. Original GUID/timestamp and full layout remain
unmatched; no Ghidra DB edits/runtime execution or PDB content-identity claim.

## PE flags/version settings evidence (2026-11-03)
Original fixed PE fields versus actual header-step1-large/2-nx/3-ts/4-image/5-os-
20261103-001; source hashes/exact one-option commands, full regressions, independent
requested-field verification and structure-bounded whole-file step comparisons.
Actual old/new wrong-mode controls, retained37 and hashes in pe-header-settings.md.
Original/37 on-disk CodeView end1538 and SafeSEH1560/1540 plus zero gap bytes are
verified through pe-image.ps1; origin of32-byte gap difference remains unknown.
No Ghidra DB changes/runtime test/unique original command or full header identity.

## Exact startup read-only layout (2026-11-18)

Live `fsx.exe` Ghidra memory and pinned PE bytes agree across
`0x0100127c..0x01001517`. The
[read-only range proof](../../artifacts/fsx.exe/rdata-order-20261118/rdata-layout.json)
records the packaged and regular XP replays, exact data records and
residual writable references. Original source organization and the
private caption name remain reconstructed. The comparison is static.

## MFC IAT pair isolation (2026-11-18)

Read-only Ghidra memory at `0x010010f8`, `0x010021ac` and
`0x01002c38`, plus XP build logs, archive inputs, PE import comparisons
and verified thunk bytes. The [trial ledger](../../artifacts/fsx.exe/iat-mfc-final-20261118/trials.json)
distinguishes the supported 135-slot result from the diagnostic split
and its CRT regression. The historical build and runtime are unverified.

## Final IAT pair trials (2026-11-18)

Live original memory at `0x010010f8` and `0x0100120c`, read-only
Ghidra listing, XP linker variants, pinned input hashes and static PE
comparisons. The [trial ledger](../../artifacts/fsx.exe/iat-pairs-20261118/trials.json)
retains repeated packaged evidence and rejected MFC changes. The
ordinary `iatnew` controller run confirms the opt-in response and
library precedence. The shipping link invocation is unverified.

## VC8 IAT order (2026-11-18)

Live `fsx.exe` Ghidra memory `0x01001000` through `0x01001237` agrees
with the pinned original EXE. The [XP trial ledger](../../artifacts/fsx.exe/iat-order-20261118/trials.json)
retains stock library hashes, worker receipts, commands and exact
import operand deltas. Two packaged runs and an isolated normal
controller run reproduce the selected result. The
[linked range audit](../../artifacts/fsx.exe/iat-order-20261118/audit.json)
links the file-backed comparisons and fresh original listings. No
candidate runtime or historical Microsoft link command was verified.

## Packaged linked-code audit (2026-11-18)

Read-only Ghidra `fsx.exe` listing for 146 scoped dashboard ranges,
paired with original and packaged-RTM PE bytes and the worker dumpbin
listing. IAT identities, data targets and selected procedure-specific
comparators validate changed operands. The
[audit ledger](../../artifacts/fsx.exe/linked-content-audit-20261118/audit.json)
retains prior comparison provenance and each remaining byte or operand.
The comparison is static and scoped to the listed ranges.

## CodeView/SafeSEH packaging reproduction (2026-11-18)

Three XP link-and-package builds retain their source hashes, logs,
MAPs, receipts, and before and after images. The original range at
`0x01001518` was read live again in Ghidra. The
[trial ledger](../../artifacts/fsx.exe/codeview-packaging-20261118/trials.json)
identifies the [Microsoft WDK 7.1 distribution](https://www.microsoft.com/en-us/download/details.aspx?id=11800)
and extracted x86 binary hashes. The demonstrated operation uses
[Microsoft's BinPlace symbol packaging options](https://learn.microsoft.com/en-us/windows-hardware/drivers/devtest/using-binplace-from-the-command-line).
The historical FSX packaging command and runtime behavior are unverified.

## CodeView/SafeSEH gap controls (2026-11-18)

Original Ghidra read-back at `fsx.exe 0x01001518` was compared to 80 bytes
from the hash-pinned original EXE. Six XP worker receipts, commands,
PE/MAP reads, source snapshot hashes and control comparisons are indexed
in [the trial ledger](../../artifacts/fsx.exe/codeview-layout-20261118/trials.json).
The resource control used one `RCDATA` entry from a diagnostic `.rc`.
The worker-path result compares the initial linked image, addresses and
verified references. The original packaging steps and runtime behavior
are outside that verification.

## CodeView/SafeSEH gap investigation (2026-11-03)
Native original memory/listing/reference/symbol inspection and37 MAP contribution
layout; actual debug-gap-probe32/default-control0 builds, original-source hash
checks and explicit added-object command delta. compare-debug-gap-probe.ps1
records exact placement and four SDK leaf shift exceptions; nonzero control
rejected. Backup, comment-only edit/save/readback and limits in codeview-gap.md.
Microsoft BinPlace syntax docs consulted for general release-tool capabilities;
they do not prove FSX used it or explain this gap. Origin remains hypothesis,
probe is synthetic/optional and golden37 remains selected.

## Application order and build-environment factors (2026-11-03)
Original known-entry/body/slot evidence versus actual FirstThree/ApplicationPrefix
MAP outputs, response/source hashes and single-option command changes. Corrected
full-regression/default and separate gap32 diagnostic builds, actual unordered/
ignored-symbol negative controls. Lifecycle listing/MAP read proves old70-byte
boundary overrun into unlabeled callback; no byte/reference tolerance added.
Reports/hashes/38 in application-function-order.md. Microsoft /ORDER, Windows
Headers and GL/LTCG docs linked in that topic/build-environment-factors.md provide
mechanism context; current VC8 behavior verified by builds. Host OS remains
unidentified; Vista release timing and header version are not host evidence.
Original FileVersionInfo resource metadata read as10.0.61637.0 (FSX-Xpack.20070926-1421).
No OS migration, Ghidra DB edit or candidate runtime execution.

## Extended callback/SDK/ATL ordering evidence (2026-11-03)
Actual original vtable13a0/13a4/13a8 and existing cleanup/ATL body mappings; actual
static/isolated-external/folded-pair/default/gap32 XP runs and MAP warnings/positions.
Source/command/exports/callback controls in compare-tail-order.ps1, existing full
regression gates and actual rejected static/folded-representative controls. Hashes
and limitations in extended-function-order.md. Original callback linkage remains
unknown; externalness is a reconstruction choice, not a recovered fact. No Ghidra
DB edit or runtime execution.

## MFC thunk ordering (2026-11-03)

Original fsx.exe executable bytes/IAT, baseline39 and actual MfcThunks job MAP,
compiled-input hashes, response, worker receipt, build log and regression reports.
66 original FF25 targets in01001ffc..0100230d match unique rebuilt identities and
public MFC import-library symbols. compare-mfc-thunk-order.ps1 checks opcodes,
IAT identities, controlled source/command inputs and placements; reports and
limitations in mfc-thunk-order.md. Actual acceptance demonstrates these thunks
are orderable here; it does not establish original linker flags/member order,
static CRT eligibility or IAT ordering. No Ghidra edit or candidate execution.

## CRT selection/order (2026-11-03)

Pinned RTM archive member/COFF section reads (crt-order-members-20261103.json),
actual rejected static-name /ORDER job warnings/MAP and actual controlled entry
INCLUDE/default XP jobs. compare-crt-selection.ps1 proves identical compiled
inputs, single-option command change, unchanged88 prefix and36 previously mapped
runtime symbol positions; existing full body/reference/data gates retained.
Default40 comparisons/inventories pass; import slot count103 from actual ordered
identities, not inferred from function placement. See fsx.exe/crt-selection-order.md for
hashes, paths and limits. No original command claim, Ghidra edit or runtime test.

## Complete import thunk and selection evidence (2026-11-03)

Original/baseline40 executable-section FF25 scan, actual IAT identities and MAP
symbols. compare-import-thunk-placement.ps1 independently checks all80 identities,
operands, positions and immediate CC runs; supersedes unsupported residual
debugger-hook placement claim. Actual137/3 import-INCLUDE jobs plus controlled
source hashes, commands, manifest, failed/passed existing gates and full ordered
import reports. compare-import-selection.ps1 checks input isolation; topic
fsx.exe/import-selection.md records hashes,103/134/105 counts and limits. Initial failed
preparation was not a build. No adoption, Ghidra change or runtime execution.

## Writable-data ledger and probes (2026-11-03)

Original PE bytes, baseline40 MAP/provenance and previously independently checked
CRT/ATL/static-initializer references. inventory-writable-data.ps1 retains the
full data/body gate chain and records30 spans/1153 bytes with explicit unassigned
regions31/51. Actual XP nonlinked object/installed-header diagnostics and three
source-identical selection/merge builds; compare-writable-data-selection.ps1
checks command/input isolation and measured layout/import/thunk outcomes. Native
Ghidra fsx metadata/xrefs read only. Existing RTM installation media CAB/member
reads and generalized COFF inspector report; no original library attribution.
Hashes, failed-improvement decisions and limitations in writable-data-layout.md.
No promotion, Ghidra database edit, library installation or candidate execution.

## MFC support-library replacement and original manifest (2026-11-03)

Actual installed-library copies checked by guest fc /b; host library/member
hashes, section-owned symbols/common records/directives versus RTM ISO member.
Controlled support-library path-only XP runs, missing-PDB warning control,
matched-media-PDB and fresh default builds. compare-mfc-support.ps1 retains input,
prefix/IAT and full body/data/thunk gates; selected41 actual binary hash/report.
Original PE resource type24 tree/XML explicitly parsed, not an unlocated text
search: main manifest requests MFC/CRT8.0.50608.0; extra Common-Controls resource
recorded. Generated sidecar remains different. Hashes/provenance and limitations
in fsx.exe/mfc-support-library.md; no original compiler/library-file attribution,
installation, resource rebuild, Ghidra change or candidate runtime execution.

## PDB data contributions and native-TU control (2026-11-03)

Actual41 PDB/EXE GUID+age pairing, MSF7/DBI-v60 records and20 public-owner MAP
cross-checks. Wrong-PDB rejection and genuine41/native pairs; contribution table
omits three known pilot globals, explicitly not classified as padding. LLVM MSF/
DBI/RawTypes primary references define the reader; Microsoft OPT/LTCG docs only
background, not evidence of the16-byte cause. Actual same-source native-TU XP
trial changes two GL compile settings, reduces size delta to8 but fails existing
initializer mapping and is rejected. Files/hashes/limits in
data-contribution-alignment.md. No promotion, Ghidra activity or runtime test.

## API add-on startup evidence (2026-11-12)

Retained API instruction listings were checked against the pinned
original PE. The original MAIN IAT and API export table establish
the ordinal 322 and 323 call boundaries. The API PE also confirms
the shutdown-prefix call to add-on teardown. The reader contracts,
source substitutions and VC8 COFF regression run are recorded in
api-addon-startup.md. Ghidra received no edits.

## API add-on trust evidence (2026-11-12)

Fifteen API listings matched the pinned PE bytes. Additional PE reads
establish ordinal 305, the protected-region descriptor records, the
provider dependency strings and the panels.dll IAT call. The original
key cache and trust decision are documented in api-addon-trust.md.

## API WinTrust provider evidence (2026-11-12)

Ten retained listings matched 2,274 instruction bytes in the pinned
API PE. Additional reads confirmed the phase callback, the relative
shutdown call, the controller slots, import identities and binding
strings. VC8 SDK layout checks and limitations are recorded in
api-addon-trust-provider.md.

## API main-window modal controller evidence (2026-11-12)

Read-only Ghidra pseudo-decode and pinned PE checks cover 2,357 API
instruction bytes plus the original controller table and IAT identities.
The pinned MAIN and WINDOW export reads establish the external edges.
Source decisions and the VC8 object are recorded in api-main-window-modal.md.

## FS98MAIN graphics transition evidence (2026-11-12)

Seven listings matched 2,044 instruction bytes against the API PE.
Additional reads verified the API controller target and pinned G2D
and WINDOW exports. Original source-pass C drops the populated refresh
vector in the parent restore path. See api.dll/api-main-window-graphics-transition.md
for assembly evidence and the VC8 build.

## G2D controller transition evidence (2026-11-12)

Read-only Ghidra pseudo-decode agrees with 1,347 bytes in the pinned
G2D PE. Additional PE reads verify both controller tables, export and
import identities and the fatal dialog literals. The isolated G2D
VC8 object and its limits are recorded in g2d-controller-transition.md.

## G2D controller backend evidence (2026-11-12)

Read-only Ghidra pseudo-decode matches 1,518 pinned G2D PE bytes.
Table cells, dynamic library strings and the API ordinal 216 import
were checked separately. Source scope and the isolated VC8 build
are recorded in g2d-controller-backends.md.

## G2D window construction evidence (2026-11-12)

Read-only pseudo-decode matched 2,349 listed bytes against the
pinned G2D PE. Separate table and import reads establish the
window targets and allocator boundary. The verified source build
and its limits are recorded in g2d-window-construction.md.

## G2D per-window initialization evidence (2026-11-12)

Read-only pseudo-disassembly matched 2,814 pinned G2D PE bytes.
The verified status exits, source boundaries and VC8 build are recorded
in g2d-window-initialization.md.

## G2D DXGI window device evidence (2026-11-12)

Read-only pseudo-disassembly matched 965 bytes in the pinned G2D PE.
The resolver, Interlocked imports, source integration and isolated VC8
object are documented in g2d-dxgi-window-device.md.

## G2D DXGI resource-holder evidence (2026-11-12)

Nine live G2D memory ranges totaling 4,817 bytes matched the pinned
PE. Original instruction reads establish holder ownership and status
returns. See g2d.dll/g2d-dxgi-window-resources.md for the isolated build and limits.
