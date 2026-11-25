# Bounded recompilation search

For `sim1.dll` flight-state integration, run
`./scripts/matching/start-sim1-state-integration.ps1 -RunId sim1-<unique-id>`
with the XP worker ready. The controller verifies the original and
compiled-source hashes and retains a native VC8 RTM object. The body
comparator checks the reviewed VTable targets, three exact transform
methods and the exact history-copy range. `-Profile Precise` changes
the floating-point control. Replay an archived result with
`node scripts/compare-sim1-state-integration.cjs <run-directory>`.
[State integration](../../notes/re/sim1.dll/sim1-state-integration.md).

For the position writers, pass `-Target PositionLinked` to
`start-sim1-state-integration.ps1` with an unused `sim1-` job ID.
The linked profile defaults to `Precise` and retains the no-entry
PE32 comparison DLL. `-Target Position` retains a separate native
COFF comparison under the same lock. Replay either result with
`node scripts/compare-sim1-position.cjs <run-directory>`.
[Position writers](../../notes/re/sim1.dll/sim1-position-writers.md).

For the force and moment path, pass `-Target ForcesLinked` to
`start-sim1-state-integration.ps1` with an unused `sim1-` job ID.
The default precise LTCG profile retains a no-entry PE32 comparison
DLL and verifies the imported fill operation against ordinal 109.
Replay with `node scripts/compare-sim1-forces.cjs <run-directory>`.
[Forces and moments](../../notes/re/sim1.dll/sim1-forces-and-moments.md).

For the thrust projection, pass `-Target ThrustLinked` to
`start-sim1-state-integration.ps1` with an unused `sim1-` job ID.
The default precise LTCG profile compares the linked helper and
validates its four CRT call targets. Replay with
`node scripts/compare-sim1-thrust.cjs <run-directory>`.
[Thrust projection](../../notes/re/sim1.dll/sim1-engine-branches.md#thrust-projection-and-accumulation).

Pass `-Target PropulsionLinked` to build the thrust helper in its
reconstructed callback context. The comparison reviews the projection
target, gravity storage, the gain divisor and exact callback scopes.
Replay with `node scripts/compare-sim1-propulsion.cjs <run-directory>`.
[Propulsion force collector](../../notes/re/sim1.dll/sim1-propulsion-collector.md).

Pass `-Target Turbine` for the native VC8 COFF comparison of the
shared turbine output helper. Replay with
`node scripts/compare-sim1-turbine.cjs <run-directory>`.
[Turbine speed-scaled output](../../notes/re/sim1.dll/sim1-engine-branches.md#turbine-speed-scaled-output).

Pass `-Target AerodynamicMoment` for the native VC8 COFF comparisons
of the propeller gyroscopic contribution, force-vector terms,
angle-dependent coefficient curve, zero-alpha coefficient, elevator
moment, pitch-rate, angle-of-attack rate, horizontal-tail
incidence and elevator trim contributions, plus the complete normal
airplane pitch callback and its remaining terms. Replay with
`node scripts/compare-sim1-aerodynamic-moment.cjs <run-directory>`,
`node scripts/compare-sim1-moment-arm.cjs <run-directory>`,
`node scripts/compare-sim1-angle-curve.cjs <run-directory>`,
`node scripts/compare-sim1-baseline-moment.cjs <run-directory>`,
`node scripts/compare-sim1-elevator-moment.cjs <run-directory>`,
`node scripts/compare-sim1-pitch-rate.cjs <run-directory>`,
`node scripts/compare-sim1-alpha-rate.cjs <run-directory>`,
`node scripts/compare-sim1-tail-incidence.cjs <run-directory>`,
`node scripts/compare-sim1-elevator-trim.cjs <run-directory>` and
`node scripts/compare-sim1-pitch-sum.cjs <run-directory>`.
[Airplane aerodynamic moment](../../notes/re/sim1.dll/sim1-aerodynamic-moment.md).

For the `fe.dll` magnetic-variation functions, run
`./scripts/matching/start-fe-magnetic-variation.ps1 -RunId fe-<unique-id>`.
The controller retains a no-entry PE32 DLL and checks the linked adapter
match with `node scripts/compare-fe-magnetic-variation.cjs <run-directory>`.
`-Profile Precise` provides the floating-point control.
[Magnetic variation](../../notes/re/fe.dll/fe-magnetic-variation.md).

SimScheduler getter pilot. Run `./scripts/matching/start-simscheduler-pilot.ps1
-RunId simscheduler-<unique-id>` with the XP worker ready. The controller
uses the shared lock and verifies the complete COFF bodies at `0x20cc13c0`,
`0x20cc1460`, `0x20cc1490`, `0x20cc14d0`, `0x20cc14e0`, `0x20cc1500`, `0x20cc1530`, `0x20cc1560`,
`0x20cc1580` and `0x20cc15b0`.
The C++ dispatcher at `0x20cc1400` produces a separate diagnostic.
Pass `-DispatcherLtcg` to build and compare its separate no-entry
DLL probe with the internal handler definitions under VC8 LTCG.
Use `-TimeStateLtcg`, `-TimeStateWords`, or `-TimeStateCopy`
for the time-state handler comparison at `0x20cc1b30`.
Use `-TimePropagation Frame` or `-TimePropagation Copy` for
the linked comparison at `0x20cc1a40`.
Use `-FlightSave Import` for the normalized linked match at
`0x20cc48d0`. `-FlightSave Frame` retains the transfer-pointer control.
Use `-FlightLoad Import` or `-FlightLoad PropagationCopy` for the
linked diagnostics at `0x20cc4790`.
Use `-ContextLtcg` or `-ContextFrame` for the linked diagnostics
at `0x20cc1b90`, `0x20cc3050` and `0x20cc30d0`.
Use `-PendingRegistration Default` or `-PendingRegistration Frame`
for the linked diagnostics at `0x20cc3490` and `0x20cc41d0`.
Use `-EntryInsertionLtcg` for the seven linked insertion diagnostics.
  Use `-ContextLifecycleFactory` for the native placement constructor ABI probe.
  Use `-ContextConstructorLtcg Frame` for the linked constructor and time
  helper probe at `0x20cc27a0` and `0x20cc15d0`.
  `-ContextConstructorLtcg Default` retains frame-pointer omission.
Use `-FallbackContext Default` or `-FallbackContext Aligned` to compare
the native COFF fallback transition at `0x20cc1cc0`.
Use `-ContextAccessors FastFp` to reproduce the native context-accessor
comparisons. It applies fast floating-point code generation to the two
step setters that require that profile. `Default` uses the normal profile.
Use `-SchedulerObjectAccessors` for the eight scheduler object VTable
accessors. It can be combined with `-ContextAccessors FastFp`.
Use `-ContextReset` for the native context reset comparison at
`0x20cc2e20`.
Use `-SchedulerObjectLifecycle` for the four native scheduler
initialization and teardown comparisons.
Use `-PoolMetrics` for the native pool metrics, trim routines and
deleting wrappers.
Use `-FreeList` for the eleven native free-list and allocator comparisons.
Use `-PoolLock` for the five native shared pool lock comparisons.
Use `-DllStartup SP1`, `-DllStartup RTM` or `-DllStartup RTMSse2`
for the linked startup probe. The RTM profiles select the pinned
CRT library and the last also links the original SSE2 initializer member.
Use `-PoolLifecycle` for the native context and timed-entry pool
initialization and teardown comparisons.
Replay with `node scripts/compare-simscheduler-object.cjs <run-directory>`.
Use `-NativeContext` with a unique SimScheduler run ID to build the native
virtual context class in VC8 COFF. Replay its separate VTable and method-body
comparison with `node scripts/compare-simscheduler-native-context.cjs <run-directory>`.
See [object getter match](../../notes/re/simscheduler.dll/simscheduler-object-getter-match.md).

To replay the linked SimScheduler group 0x10 dispatcher match, use
`start-simscheduler-pilot.ps1 -RunId <unique-simscheduler-id> -DispatcherLtcg
-DispatcherVc8Rtm -FlightLoad PropagationCopy -FlightSave Import -TimeStateCopy`.
The controller checks the shared RTM tool identities and the XP job runs
them from its own directory. Omit `-DispatcherVc8Rtm` for the installed
SP1 control. See [event dispatch](../../notes/re/simscheduler.dll/simscheduler-event-dispatch-match.md).

Use `-ContextStorageCleanupLtcg -DispatcherVc8Rtm` with a fresh
SimScheduler run ID to build the class allocation probe for
`0x20cc15f0`. The report includes the compiler-generated factory
funclet and the checked pool-release targets. See
[function coverage](../../notes/re/simscheduler.dll/simscheduler-function-coverage.md).

GPS provider pilot. Run `./scripts/matching/start-gps-pilot.ps1 -RunId
gps-<unique-id>` from the project root with the XP worker ready.
The controller stages the source through the shared queue under the
controller lock, retains the receipt and artifacts in
`vm-share/modules/gps/runs/`, and checks the complete ordinal 2 body
against the pinned GPS PE. Current builds also verify the ordinal 1
and provider Release diagnostics, the entry-release match, and
the destructor unwind, legacy ESI, and lifecycle diagnostics. To replay the comparison, run
`node scripts/compare-gps-registration.cjs <run-directory>`.
See [GPS provider registration](../../notes/re/gps.dll/gps-provider-registration.md).

Factory bridge pilot: `start-api-window-pilot.ps1 -RunId <unique-api-id>
-FactoryObject` builds api-factory.obj/compiler PDB with the explicit RTTI-less
fragment profile. Use the lifecycle COFF comparator on that run for getter72,
observed table16 and prior gates. It does not link a complete DLL or certify a
rebuilt export144. See api-factory-matching.md for the bounded two source trials.

Registry dependency pilot: `start-api-window-pilot.ps1 -RunId <unique-api-id>
-RegistryObject` produces api-registry.obj/compiler PDB. Use
`node scripts/inspect-api-registry-object.cjs <run>` for diagnostic-only reference
and critical-path checks. This does not certify helper matches. Registry and
lifecycle profiles are mutually exclusive; no object profile accepts WholeProgram.
See api-callback-registry.md for actual source/visibility trials and stopping point.

Lifecycle object pilot: `start-api-window-pilot.ps1 -RunId <unique-api-id>
-LifecycleObject` compiles an unlinked native COFF object with actual unresolved
dependencies. Canonical api-lifecycle.obj/compiler PDB stay under API-owned runs;
use `node scripts/compare-api-lifecycle-object.cjs <run>` for scoped relocation
verification. This is distinct from a linked DLL match. See api-lifecycle-matching.md.

API.DLL pilot (2026-11-03): `start-api-window-pilot.ps1 -RunId <unique-id>` builds
a module-owned run under vm-share/modules/api/runs with canonical api-pilot.dll,
api-pilot.map and api-pilot.pdb. Original worker outputs live in worker-output/;
submitted source and job metadata in the same run directory.
See vm-share/README.md for ownership.
The controller continues to use the shared queue and lock. This builds
a separate no-entry DLL fragment using the same locked worker queue. Optional
`-IncludeHeap` adds reviewed heap source; `-WholeProgram` is the retained failed
GL/LTCG context control. Compare with `scripts/compare-api-window.ps1` using the
run directory. The worker's fixed fsx.exe output name transports a /DLL PE and
is copied to api-pilot.dll on the host; it must never replace either installation
binary. See notes/re/api.dll/api-first-pilot.md for exact scopes and retained trials.
No worker restart/output-list edit or EXE baseline modification is required.

The optional `-FactoryObject -SourcePass` profile stages the frozen broad
API snapshot from `artifacts/api.dll/api-source-pass-20261107/archived-source/`
for the existing native COFF regression checks. Source generation for that
experiment has been retired. [API source pass](../../notes/re/api.dll/api-source-pass.md).

The host generates ordinary C++ source variants and compares linked binaries;
the Windows XP guest only compiles jobs. No candidate binary is executed. This
first strategy targets the remaining ShowErrorMessage encoding difference,
not a complete FSX initialization match.

Default build profile is now MfcWindows: guest-build-mfc.cmd links the actual
MFC WinMain startup with /SUBSYSTEM:WINDOWS, compiling only the application TU
and getter-boundary TU. It does not compile the console main/synthetic helper
callers. Use -BuildProfile ConsolePilot for the retained guest-build.cmd control.
verify-startup-profile.ps1 checks the actual PE subsystem/entry, MFC wrapper/import
and absence of console symbols. Baseline27-mfc-windows-startup is current; see
notes/re/fsx.exe/mfc-startup-profile.md. Builds are compared, never executed as FSX.

`-Target AppLifecycle -MaxCandidates 1` verifies the application constructor,
deleting destructor and ExitInstance bodies with target-validated address
normalization. All three now participate in every build's regression checks.
The preserved baseline is archive/24-application-lifecycle-matches; see
notes/re/fsx.exe/app-lifecycle-matching.md for exact scope and exclusions.

Latest baseline: archive/25-application-dispatch-matches. Every build now also
checks GetMessageMap, the empty static map, Run and its current-message callback
through compare-app-dispatch.ps1. Source interface slot+8 is a typed message-loop
callback method; the independent API evidence is in app-dispatch-matching.md.

Run `./scripts/matching/start-search.ps1 -RunId <unique-id> -Target InitInstance -MaxCandidates 1`
to reproduce the linked match. The default uses the installed SP1 tools.
`-CompilerToolset RTM` uses the hash-checked RTM compiler and linker from the
shared toolchain for this job. Both use the selected profile's headers and
libraries. Pass `-SourceDirectory <snapshot>` to build a source proposal.
The controller emits alignment, control-flow and operand diagnostics. For the
original instruction layout it also emits `initinstance-body.json`. A
`matched` run passed the complete target-aware byte comparison and regression
checks. Replay the whole-body check with
`./scripts/compare-initinstance-body.ps1 -BuildDirectory <candidate-directory>`.
See [InitInstance](../../notes/re/fsx.exe/initinstance-matching.md).

Run on the host: `./scripts/matching/start-search.ps1`. In XP, launch
`RUN-WORKER.bat` from the shared folder once and leave its window open. No SSH,
guest password, network service or host-side legacy compiler is required.

`-CompilerToolset PinnedSP1` selects the VC8 SP1 x86 tools extracted from
the verified update in `vm-share/setup/`. The controller checks the shared
tool hashes, copies the tools into the guest job directory, and verifies
the compiler and dumpbin versions. The guest supplies the installed headers
and the profile selects its CRT and MFC libraries as usual.
[Tool provenance](../../vm-share/toolchains/vc80-sp1/README.txt).

The MfcWindows probe for the independent source-body comparison of
`AfxInitialize` can be run alongside the `InitInstance` regression.

```powershell
./scripts/matching/start-search.ps1 -RunId <unique-id> -CompilerToolset RTM -Target InitInstance -MaxCandidates 1 -AfxInitializeProbe -LinkFolding NOICF
```

The strategy enumerates at most 24 semantically equivalent HRESULT-result source
forms: signed int/HRESULT/long, optional zero initialization and four success
predicates. The rest of src, compiler flags and SP1 revision are pinned. It
does not write arbitrary assembly, patch binaries, alter Ghidra or overwrite
canonical sources. Baseline failure stops the run. A full verified target match
plus existing helper/OLE regression checks stops successfully. Exhaustion means
these implemented proposals are exhausted, not all possible source programs.

Each run snapshots source, hashes and candidate descriptions under `runs/`.
`status.json` and `results.csv` expose progress, best candidate and stop reason.
During execution every candidate retains source, build log, EXE/PDB/MAP/disassembly
and comparison report. Full outputs remain for representative
candidates and all candidate sources/logs/reports remain. Different layouts that the deliberately strict comparators cannot map
are rejected, never counted as matches. These comparators are not a general
assembly aligner: adapting them is required before exploring InitInstance.

The guest claims one ready job, uses a fresh `C:\fsx-build\matching-jobs` directory,
and publishes its completion marker only after copying outputs and receipt.
No old artifacts are reused. A 15-minute per-job timeout stops host dispatch;
it does not kill an already-running guest compiler. A crashed worker's claimed
job is not automatically retried. Stop the worker and start a fresh host run
after investigating; preserved job folders explain the failure.

Host stop: `./scripts/matching/stop-search.ps1 -RunId <id>`. Guest worker stop:
Ctrl+C in its window, or create `STOP-WORKER` in the shared root. Removing that
file is required before restarting the worker. No recurring Codex automation
is installed; this is an explicit local run with bounded proposals and duration.

Run only one host search at a time. Do not edit queued job folders while running.
Do not run historical BUILD.bat files against their old paths. Existing archives
remain evidence snapshots; moved historical paths are described in cleanup notes.


InitInstance diagnostics now also emit initinstance-references.json and initinstance-blocks.json/.html automatically. Block pairing and consistent successor sets describe structure only; they are not byte or semantic equivalence certificates. Latest selected reconstruction/limitations are in notes/re/fsx.exe/initinstance-matching.md.

Current linker default is /OPT:ICF after controlled source-identical builds retained all13 matches and recovered original address-sharing relationships across76 vtable slots. Use -LinkFolding NOICF for a staged control; default source/build command is not overwritten by that option. Requests retain mode and command hash; guest log prints the actual local build script. compare-vtable-aliases.ps1 gates ICF runs on sharing-pattern agreement. Golden/replay baseline is archive/26-icf-vtable-sharing-matches. Details and limitations: notes/re/fsx.exe/linker-folding.md.

Getter definition now compiles /GL by default (GetterCompilation GL); explicit NonGL control and SameTU proposal mode remain. SameTU requires an include of application_interface_pilot.cpp in the staged application source. Current golden/replay baseline28-gl-getter-context. Tests and EXE inventory documented in notes/re/fsx.exe/getter-context-and-exe-inventory.md.

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

Current CRT library default: -CrtLibrary Auto selects pinned RTM msvcrt.lib for MfcWindows and InstalledSP1 for ConsolePilot. Explicit -CrtLibrary InstalledSP1 retains the prior control. Compiler/linker and installed MFC/headers remain SP1. RTM is read from the established guest fsx-build share's toolchains/vc80-rtm directory, after explicit MFC library precedence. Keep that library available. Hash and effective mode recorded in request.json; RTM C initializer table roles/order must pass the gate. Golden/replay30-rtm-crt-context. Optional -ToolchainDiagnostics emits installed SP1 CRT member headers/symbols/relocations and nonlinked diagnostic object directives. See notes/re/fsx.exe/crt-library-context.md; table equality is not full CRT-body equality.

MfcWindows builds now additionally gate the complete226-byte CRT pre_c_init body with27 target-validated fields and three dependency checks through scripts/compare-pre-c-init.ps1. comparison.json includes PreCInit; this is a separate CRT library identification, not a change to the18 reconstructed/helper/wrapper count. Baseline30 and application sources unchanged. See notes/re/fsx.exe/pre-c-init.md.

MfcWindows comparison.json also includes PreCppInit: the complete75-byte CRT pre_cpp_init body with12 validated fields and RTC termination/atexit dependencies. This is separately reported library identification; prior18 reconstructed/helper/wrapper bodies unchanged. Baseline30 remains selected. See notes/re/fsx.exe/pre-cpp-init-and-exe-gaps.md and notes/re/fsx.exe/exe-matching-roadmap.md.

MfcWindows comparison.json now includes CrtStartup: complete479-byte region including embedded SEH code,39 target-validated fields,40-byte EH4 table, PE entry10, cookie initialization148, SEH prologue69/epilogue20 and MFC entry wrapper. No C++/flag/library change;18 application/helper/wrapper count remains unchanged. Full image helper/exception-dispatch bodies and whole CRT/EXE layout remain separate. See notes/re/fsx.exe/full-crt-startup.md.

MfcWindows comparison.json also gates CrtImageHelpers: PE validation41/section66 raw bodies,108-byte nonwritable region including EH23/scope28, exception wrapper35 and cookie checker15. Existing18 body count and baseline30/source choices unchanged. See notes/re/fsx.exe/crt-image-helpers.md; DLL dispatcher implementation/failure-reporter body remain separate.

MfcWindows comparison.json also includes CrtFailureStack: reporter260 with33 mapped fields, initial Windows context/exception buffers and pointer links, plus aligned stack probes22/22 and raw base probe43. Existing18 body count and baseline30/source choices unchanged. See notes/re/fsx.exe/crt-failure-stack.md.

MfcWindows comparison.json also includes CrtOnexit: full150-byte registration body,9-byte lock cleanup,18-byte adapter and28-byte EH4 finally record. Prior18 body count and baseline30/source/library choices unchanged. See notes/re/fsx.exe/crt-onexit.md.

MfcWindows comparison.json also includes AtlCodepage: three complete ATL101/4/100 bodies, cookie/import targets and cached getter state. Original five-byte inter-function gaps are reported separately from body matching; no padding option changed. Prior18 body count and baseline30/source/library choices unchanged. See notes/re/fsx.exe/atl-codepage.md.

MfcWindows comparison.json additionally gates RuntimeRemainders: GS SEH72/15
normalized bodies with four validated targets, multiplication52 raw bytes and
three import thunk identities. Prior18 application/helper/wrapper body count,
sources and baseline30 remain unchanged. See notes/re/fsx.exe/runtime-remainders.md for
listing/data changes, limits and next padding experiment.

Current default additionally uses -FunctionPadding 5 (/FUNCTIONPADMIN:5 injected
into staged linker command). Explicit -FunctionPadding 0 restores the omitted
option control. Golden/replay/inventory baseline31-function-padding supersedes30;
30 remains retained. Ten missing CC gaps reproduced, all body gates retained.
FollowingGapBytes distinguishes checked0/5 CC padding from exact body extents.
See notes/re/fsx.exe/function-padding.md for source-identical controls and layout limits.

Current default additionally uses -Rtti Disabled, injecting /GR- on staged
compiler commands. -Rtti Enabled retains the former control. RttiMetadata records
two vtable-anchor COL checks, scanned type names/MAP RTTI symbols and selected
imports; disabled mode requires the observed original absence shape. All earlier
body gates remain. Golden/replay/inventory32-rtti-disabled supersedes31, which is
retained. See notes/re/fsx.exe/exe-rtti.md; this does not disable MFC runtime classes or
prove a unique original compiler configuration.

MfcWindows comparison.json additionally includes mandatory DataConstants:
18 terminated literals/183 bytes, eight mapped token pointers,72-byte load-config
and8-byte SafeSEH table, selected initial global spans/CRT DWORDs and inherited
MFC runtime-class import dispatch. PE directory size64 is distinguished from
structure size72. Baseline32/source/flags unchanged; see notes/re/fsx.exe/exe-data-constants.md
for actual worker build, negative controls and incomplete whole-data/layout scope.

MfcWindows comparison.json additionally includes PeImports. Mandatory
-RequireImportContents checks complete137 identities/hints, descriptors, ILT/IAT
initial equality/termination and module-block coverage. It does NOT accept import
entry order as matched: ImportContentsAndOrderMatch is false on baseline32;
-RequireExactOrder rejects that build. PE directories/headers/sections/debug and
certificate ranges are inventory only. See notes/re/fsx.exe/pe-imports-and-directories.md.

Current default additionally uses -ImageBaseMode Original (/BASE:0x01000000).
Explicit -ImageBaseMode Default retains omitted-/BASE control. Startup gate
records/enforces the requested actual base. Four legacy body comparators now
read the PE base; no body/reference masking relaxed. Golden/replay/inventory33-
original-image-base supersedes32, retained as control. Worker PDB path length can
affect layout; matched-length actual control shows343 identical MAP RVAs. See
notes/re/fsx.exe/image-base.md for evidence and remaining section/header/order differences.

Current default additionally uses -SectionLayout RdataIntoText, staging
/MERGE:.rdata=.text; -SectionLayout Separate retains the omitted-option control.
SectionLayout report/gate enforces requested section topology/access flags and
reports original placement/extents separately. Golden/replay/inventory34-merged-readonly
supersedes33, retained. Non-resource RVA/raw-size/file-offset/flags match; virtual
sizes/content/order/debug identity remain open. See notes/re/fsx.exe/section-merge.md.

Current default -StackMode Original stages /STACK:0x40000,0x2000 and enforces
original reserve/commit in Startup; Default restores omitted-option control.
Golden/replay/inventory35-original-stack supersedes34, retained. See
notes/re/fsx.exe/stack-header.md for actual build and bounded usage cut/next steps.

Current default additionally uses -PdbPathMode Basename (/PDBALTPATH:fsx.pdb).
Explicit Worker retains omitted-option embedded worker path. Mandatory PdbDebug
verifies actual path/type/RSDS/age/terminated extent and PDB artifact presence;
GUID/timestamp differences remain reported. Golden/replay/inventory36-basename-pdb
supersedes35, retained. Different-length actual job paths now agree outside known
timestamp/GUID fields. See notes/re/fsx.exe/pdb-path.md; no whole-file identity claim.

For the verified `fsx.exe` IAT order, use `-ImportSelection IatOrder`
with `-CompilerToolset RTM -WritableDataSelection AtlObjectFirst`.
The response file contains 102 `/INCLUDE` directives for existing
import-library symbols. This option also puts `mfcs80.lib` before
`mfc80.lib`, which aligns the final two CRT slots. The following XP replay uses a six-character
run ID to keep the worker PDB allocation at the verified extent.

```powershell
pwsh -File scripts/matching/start-search.ps1 -RunId iatwin -Target InitInstance -ReviewedSource -MaxCandidates 1 -CompilerToolset RTM -WritableDataSelection AtlObjectFirst -PdbPathMode Worker -ImportSelection IatOrder
```

Use a fresh six-character run ID if `iatwin` already exists. A reference
replay with the same-length ID `iatbas` and the default import selection
isolates the response file. `compare-import-selection.ps1` accepts the
two candidate directories and `-ExpectedThunkDelta 0` for this pair.
The separately retained BinPlace trial has the short CodeView record.
See [the final-pair trials](../../notes/re/fsx.exe/import-selection.md#2026-11-18-final-import-pair-trials).

Current defaults HeaderFlagsMode/ImageVersionMode/OsVersionMode Original stage
/LARGEADDRESSAWARE /NXCOMPAT /TSAWARE /VERSION:5.2 /OSVERSION:5.2. Independent
Default and intermediate flag controls retained. Mandatory PeHeaderSettings
checks selected flags/version pairs and unchanged subsystem4.0. Five actual
single-option stages changed no bytes outside intended header fields and known
timestamps/GUID. Golden/replay/inventory37-original-header supersedes36, retained.
See notes/re/fsx.exe/pe-header-settings.md; full header/layout is not certified.

Optional -DebugGapProbeBytes 32 generates a clearly synthetic32-byte .rdata$r
object for the reviewed merged MfcWindows layout. Default0/golden37 unchanged.
Actual probe restores SafeSEH/RTC placement/.text virtual size but reorders four
SDK leaf symbols; it is not recovered original source or a verified release
process. Source hashing includes the added diagnostic file. See notes/re/fsx.exe/codeview-gap.md
and scripts/compare-debug-gap-probe.ps1 for retained results and unresolved cause.

Current FunctionOrder Auto resolves ApplicationPrefix for MfcWindows and Default
for ConsolePilot. Explicit Default omits /ORDER; FirstThree retains the small
control. function-order-input.h is plain response data, not included C++: .h lets
the unchanged worker return/hash the actual used file. Mandatory order gate rejects
LNK4037/4044 and verifies MAP placements. Twelve entries have original relative
distances (standard gap offset-32); optional synthetic gap remains off. Golden/
replay/inventory38-application-order supersedes37, retained. See
notes/re/fsx.exe/application-function-order.md and build-environment-factors.md.

Current FunctionOrder Auto now selects ApplicationAll for MfcWindows; CallbackLinkage
Auto selects External forAll and Static otherwise. The actual staged callback
removes only static; its5-byte body/ABI/import checks remain. Original linkage
unknown, no PE export added. Both folded Serialize/Dump names must be in response;
order equality is allowed only for this reviewed pair.22 symbols/21 entry positions
have original relative distances, still-32 without the optional disabled gap.
Golden/replay/inventory39-extended-function-order supersedes38, retained. Actual
compiled source is in archive39/source, canonical src unchanged. See
notes/re/fsx.exe/extended-function-order.md; unlisted runtime/import/data order remains open.

Optional `-FunctionOrder MfcThunks` retains ApplicationAll and appends 66 reviewed
MFC import thunk symbols from mfc-thunk-order.json. Auto callback linkage is
External for this mode too. Actual XP trial accepts all88 entries and preserves
prior checks; compare-mfc-thunk-order.ps1 validates original FF25/IAT identities,
uniform-32 placement, unchanged prefix and source/build-command controls. IAT
slot order is unchanged (128 differences). Auto/golden39 remain selected while
remaining CRT groups are investigated. See notes/re/fsx.exe/mfc-thunk-order.md.

Current default (supersedes the preceding chronological settings): FunctionOrder
Auto -> MfcThunks for MfcWindows, Default for console. CrtStartupSelection Auto ->
EntryInclude for MfcWindows/RTM, Default otherwise. EntryInclude adds only
/INCLUDE:_WinMainCRTStartup; all compiled inputs are unchanged versus the MFC
control. Actual default40 passes prior gates, retains88 prefix placements and
places36 mapped runtime symbols original-32; import slot differences are103.
Golden/replay/inventory is40-crt-entry-selection;39 retained. To restore39 settings,
use -FunctionOrder ApplicationAll -CrtStartupSelection Default. CrtStartupProbe
is a retained negative /ORDER test: exactly3 static names receive LNK4037 and the
mandatory order gate rejects it. See notes/re/fsx.exe/crt-selection-order.md and
scripts/compare-crt-selection.ps1. Synthetic gap remains disabled.

ImportSelection remains Default. Optional OriginalIncludes requests all137
reviewed IAT symbols in original ascending IAT-VA order; CrtDirectIncludes
requests only _splitpath_s/_vsnprintf/_setmbcp. Both add hashed response data
import-selection-input.h and one linker response reference; prior inputs are
unchanged. Actual full trial fails the existing placement gate and has134 IAT
slot differences. The direct trial passes all gates but has105 instead of103.
Neither is adopted. compare-import-selection.ps1 checks the controlled inputs
and reports the outcomes; import-selection.json records baseline40 MAP/identity
provenance. See notes/re/fsx.exe/import-selection.md. Complete baseline40 thunk checking
now verifies all80 FF25 targets/placements and immediate CC runs; earlier claims
of residual CRT thunk placement differences are superseded. Default40 unchanged.

WritableDataSelection remains Default. Optional AtlCacheInclude adds an INCLUDE
for the existing ATL cache; AtlLibraryFirst explicitly lists atls.lib after
mfcs80 and before pinned CRT. Actual source-identical controls preserve all80
thunks/body gates but do not reduce .data+20. Optional MergeBssIntoData adds
/MERGE:.bss=.data: actual trial passes normalized-body gates but worsens virtual
size to1216/raw1536 and emits LNK4254; not adopted. WritableDataDiagnostics builds
one non-GL probe that is never linked and logs installed header/member evidence.
All remain off by default. inventory-writable-data.ps1 reports reference-validated
spans/unassigned ranges; compare-writable-data-selection.ps1 checks isolated
inputs/commands and actual outcomes. See notes/re/fsx.exe/writable-data-layout.md.

Current default41 supersedes the preceding support-library setting:
MfcSupportLibrary Auto -> RTM for MfcWindows with pinned RTM CRT, InstalledSP1
otherwise. Only mfcs80.lib is replaced; installed MFC import/ATL libraries,
headers, compiler/linker remain. RTM library and matching PDB hashes are checked
and recorded. Actual default passes all gates without linker warnings; .data
delta16 instead of20, prefix88/IAT entries unchanged, all80 thunk checks pass.
Golden/replay/inventory is41-rtm-mfc-support,40 retained. Explicit
-MfcSupportLibrary InstalledSP1 restores40 support settings. The generated
sidecar has extra dependencies and is not original-resource equality. See
notes/re/fsx.exe/mfc-support-library.md and scripts/compare-mfc-support.ps1.
WritableDataDiagnostics additionally captures installed libraries read-only
with fc /b copy verification; those copies are analysis evidence, not installed
replacements. Diagnostics remain disabled by default.

For the VC8 RTM writable-section comparison, use
`-WritableDataSelection AtlObjectFirst`. It extracts the existing ATL
member into the guest job directory and links it before the libraries.
The probe uses an installed `atls.pdb` copy when available.
Run `scripts/compare-writable-image.ps1 -BuildDirectory <candidate-directory>`
to verify the complete initial writable section and its one validated pointer.
The diagnostics `AtlObjectCrtLibraryFirst`, `AtlObjectSplitMfcBss` and
`AtlObjectSplitMfcBssTail` exercise alternative CRT/MFC contribution order.
The split probes retain any explicitly linked startup members needed by the
provenance gates. Results and remaining address deltas are in the
[link-time address trial ledger](../../artifacts/fsx.exe/linked-address-order-20261118/trials.json).

For the RTM application getter at `0x01004384`, select
`-WritableDataSelection AtlObjectMfcNolib` together with `-ImportSelection IatOrder`.
This adds the guest's MFC `nolib.cpp` as a separate LTCG input to the
ATL-first profile. The following ordinary worker run reproduced the full
769-byte InitInstance. The six-character run ID preserves the tested
worker PDB path length. Choose an unused ID of that length for this replay.

```powershell
./scripts/matching/start-search.ps1 -RunId gtr002 -Target InitInstance -MaxCandidates 1 -CompilerToolset RTM -WritableDataSelection AtlObjectMfcNolib -ImportSelection IatOrder -PdbPathMode Worker
```

To reproduce the packaged dashboard image from its archived inputs on
the RTM snapshot, use the following command with an unused job ID.
The controller checks the guest source hashes, MAP getter, artifact
type, 146 code scopes, startup read-only block and file-backed writable
section against the accepted replay.

```powershell
./scripts/matching/start-fsx-getter-layout.ps1 -RunId fsx-getter-replay-02
```

ContextCompilation remains GL. Optional NonGL replaces the main context's /GL
with /GL-; GetterCompilation controls the other TU independently. Actual
-ContextCompilation NonGL -GetterCompilation NonGL trial retains /LTCG and all
compiled source hashes but fails existing app-initializer mapping, despite
reducing .data delta to8. It is rejected;41 and GL/GL remain selected.
inspect-pdb-contributions.ps1 reads paired MSF7/DBI-v60 data with GUID/age and
public-owner MAP checks. PdbFile can override the input for pair validation;
a wrong-PDB control rejects. Known pilot globals missing from this table are
not padding. No original PDB or inferred original section ownership. See
notes/re/fsx.exe/data-contribution-alignment.md for actual41 gaps and limitations.
