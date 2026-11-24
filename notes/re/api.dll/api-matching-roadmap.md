# API.DLL reconstruction and matching work packages

2026-11-09 export 85 file stream selection and CAB fallback build. Fifteen
gates retain 4935/288. Next decoder 20132e20 with seek/size helper 20133611
and util.dll ordinal 195, followed by the independent stream cache.
See api-file-stream-open.md.

2026-11-09 CAB stream extraction and FDI open callback build. The former
size-consumer hypothesis for 20144f5f is superseded by a stream return.
Fifteen gates retain 4935/288. Next trace the caller at 20133137, including
direct-file versus CAB selection and wrapper ownership. See
api-cab-extraction.md.

2026-11-09 CAB owner Stat failure outputs verified. Entry lookup and fallback
attributes build together. Fifteen gates retain 4935/288. Next entry-open
and size consumers through shared FDI dispatch 20144924. See
api-cab-owner-metadata.md.

2026-11-09 buffer cache container and lifetime source builds, with 17 bytes
accepted for the registration body. Fifteen gates pass at 4935/288.
Owner reload stays open after checking the resolver and Stat failure paths.
Next bound the metadata contract through 20135378 and its two Stat targets.
See api-cab-buffer-containers.md.

2026-11-09 stream VTables and remaining methods build. Five bodies and both
table spans add86/112, totals4918/288 with fourteen gates. Next buffer cache
container operations and owner reload contracts. See api-cab-stream-tables.md.

2026-11-09 pool registration, owner tables and pre-reset statistics source
build together. Eleven bodies and three table spans add97/132, totals4832/176
with thirteen gates. Next actual stream VTables200f2c70/200f2d50 and remaining
slots. CRT placement and native EH remain separate integration work.
See api-cab-pool-registration.md.

2026-11-09 fixed pool acquisition modes0/1/3 build and replace the external
source dependency at five call sites. Twelve gates retain4735/44. Next pool
registration and pre-reset callback chain201128d7. General acquisition and
runtime validation remain separate. See api-cab-pool-acquire.md.

2026-11-09 lock release/lifetime and both pool owner lifecycles build together.
Twelve gates retain4735/44. Next verify acquisition queue, wait retry and
rollback for pool modes0/1/3 before source integration. See
api-cab-lock-lifecycle.md for verified branch contracts and trial limits.

2026-11-09 slot transfer201462fc/201464c3 now builds with both x86 atomic forms.
Twelve gates retain4735/44. Next pool lock release20147aa6 and acquisition
20146e07, scoped to pool-used modes first. See api-cab-slot-transfer.md.

2026-11-09 slot state and aligned storage source builds. Twelve gates retain
4735/44. Next tagged-head operations and slot transfer201462fc/201464c3.
See api-cab-slot-storage.md for the verified fields and remaining dependencies.

2026-11-09 shared pool source covers allocation, return and block initialization.
Twelve gates pass at4735/44. Next trace slot state creation20145fe3, destruction
201460ff and node storage201461b4. See api-cab-pool-locks.md for the verified pool
lock modes and remaining atomic dependencies.

2026-11-09 buffer cache source builds and both object pools share verified
control flow. Twelve gates pass at4735/44. Next recover lock upgrade and slot
return contracts. See api-cab-buffer-pools.md for the bounded trial and addresses.

Stream methods2026-11-09: six source methods added, three full matches57+64+92
raise total4735/44, twelve gates. Buffer field semantics corrected in source.
See api-cab-stream-methods.md. Next shared object-pool20124903/20127a04 slot/
reservation contracts and buffer cache20127642/2012772b, before duplicate pool
source or further method tuning. No matrix; owner/EH blockers remain open.

Stream producers2026-11-08: four structured producer/lifetime bodies compile;
ownership verified, no new body matches. Eleven gates retain4522/44. Original
pool unwind return differs from owner heap free; source native EH still absent.
See api-cab-stream-producers.md. Next functional read/seek/stat methods behind
wrapper/file VTables, then allocation pool/buffer dependencies. Owner reload's
undefined failure edge remains open; no guessed reachability or NULL repair.

CAB image2026-11-08: prefix reader plus four stream helpers source added;
release/read33+39 raw matches increase total4522/44, eleven gates. Owner reload
source deferred on uninitialized stream failure branch. Next stream producers
201246c4/20127440/20126be0, ownership and failure reachability. See api-cab-image.md.

CAB follow-up2026-11-08: API-05 path/owner/normal cache source added; original
unwind free verified but fixed profile emits none for source resolver. Ten gates
retain4450/44 on api-cab-cache-20261108-001. See api-cab-owner-cache.md. Next
owner/image loading201449c4/2014420b and stream/error ownership. ABI/EH blockers
remain explicit; no byte tuning at these boundaries without new evidence.

2026-11-08 API-05: dependency/resource consolidation complete for the retained
bounded findings; CAB backend identified and three more structured routines
compile. First sizes96/89,144/147,441/367 remain unmatched; ten gates preserve
4450/44. See api-alternate-enumeration.md and api-module-references.md. Next
20145169 path discovery,201452e1 cache and exception ownership; no repeated
font/open/next tuning until original private register conventions are resolved.
Whole-module work and linking remain open. No DB edits/runtime execution.

2026-11-07 broad-pass update: user now requests source-first whole-API breadth,
leaving detailed byte differences for later. Read-only whole-model export and
automatic-route classification are complete; **whole-DLL reconstruction is
not**.567 generated candidates plus nine new manual routines compile on SP1.
API-05 enumeration/suffix source expanded; API-06 idle/hook/message loop source
added (shutdown still open); API-07 broad source pipeline now exists.88 new
generated bodies/3116 bytes and foreground-idle hook214 pass, prior1120 retained:
4450 complete code/44 metadata, ten gates. API-08 linking/data/export work remains
open. See api-source-pass.md for exact scope and fixed regression snapshot.

Latest factory milestone2026-11-03: source/object gate for GetApplication72 and
generated observed four-slot VTable16 succeeds on second targeted source trial;
all prior gates retained. Final factory object complete-body scope410/metadata44,
with overlapping window21 evidence tracked separately. Linked factory/export144
still open. See [api-factory-matching.md](api-factory-matching.md). Next API-05
entry20120869 reachable-flow/phase analysis, before large function creation or
startup source. Native heap/ESI residuals remain bounded/deferred.

Latest source dependency slice2026-11-03: callback registration and realloc now
have actual compiled C++ models; unusual original NULL-retry behavior retained.
No additional helper body match is claimed; fourteen realloc references and
critical paths checked. Prior complete/partial gates retained. Bounded native
question closed; return to GetApplication/export144 and VTable integration,
starting with original RTTI/prefix evidence. See
[api-callback-registry.md](api-callback-registry.md). This supersedes the previous
'next reconstruct registry/realloc' action; original register residual stays open.

Latest Initialize/FDI slice2026-11-03: raw native COFF equality for four stream
adapters adds140 bytes; complete-body object scope317 plus EH metadata28.
Initialize source158/original160 is only partially verified (123 bytes/21 mapped
references), with explicit ECX/ESI receiver/register residual. Ghidra custom ESI
owner model and FDI signatures saved. See [api-initialize-fdi.md](api-initialize-fdi.md).
Next callback registry registration/realloc20135e98 source and genuine context;
no further native source/GL permutations without new evidence. This supersedes
the now-completed 'inspect ESI and prepare Initialize source' next action below.

Latest API-04 result2026-11-03: one native object trial verifies complete outer
Shutdown11, RunMessageLoop29 and wrapper137 code ranges plus28-byte scope table.
This is relocation-validated COFF equality, separate from linked DLL evidence.
Ghidra targets/VTable types are coordinated and saved; Initialize argument2's
legacy pointer type is now neutral uint on both sides. Details and limits:
[api-lifecycle-matching.md](api-lifecycle-matching.md). Next the Initialize outer
body and helper200f6451's register ABI; callback/filter implementation and final
GetApplication/VTable linking remain open. This supersedes older next-step cells.

Build ownership: API runs and completed job snapshots are separated under
vm-share/modules/api/{runs,jobs}; canonical outputs api-pilot.dll/map/pdb.
Original fixed-name worker files are under each run's worker-output directory.
See vm-share/modules/api/README.md and relocation-20261103.json. Existing EXE
runs/archives, shared toolchains/worker queue and controller lock remain in place.

Current execution update2026-11-03: API-01 initial boundary gate established
(normal/handled-exception return, resettable object lifetime, explicit original
type limits); Ghidra wrapper ABI/name and provisional parameter name corrected,
backed up and saved. API-02 has its first accepted body: GetWindowHandle21 bytes,
including target/initial-state and negative-control gates. GetApplication/full
VTable integration remains pending. API-03 initializer/allocator source builds,
but both heap bodies remain unmatched; a source-identical GL/LTCG control gives
the same symbolic schedules, so that context question is closed. See
[api-first-pilot.md](api-first-pilot.md). Next independent package is API-04's
outer-method/wrapper reconstruction; no blind heap permutations. This update
supersedes the initial state cells below, not their completion requirements.

2026-11-03. User-authorized next module after fsx.exe. Project documentation is
English. Goal: reconstruct ordinary C++ with independently checked ABI, then
target-validated linked body matches, expanding toward module code/data coverage.
No promise of whole-file identity or a runnable replacement from partial pilots.

## Fixed input and inherited evidence

Original `original/api.dll`, SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003;
x86 PE32, base0x200e0000, linker version8.0. Live Ghidra ranges at20120442,
200f2b24 and2013e3c1 match this exact file byte-for-byte. Database5865 functions
is a model count, not recovered-function or match coverage.

Fresh original inventory:288 nonempty export entries, only1 named export
(`GetApplication`, ordinal144, VA20120442),828 ordinary IAT entries. Preserve
ordinal exports even where original names are unavailable. `.text` virtual1132115
bytes, `.data`101193, plus resources and relocations. These are section extents,
not counts of application code. No whole-module range classification yet.

Use the existing XP VS2005 SP1 .762 worker as the first controlled compiler
candidate. Its settings/library selections matched parts of fsx.exe; they are
not proven original API.DLL settings. Do not blindly inherit MFC startup,
function-order responses, CRT/MFC RTM library choices, /GR-, section merging,
EXE image flags or linker gap probes. DLL entry/exports/relocations need their
own controls. Preserve the accepted EXE source and immutable41 ledger.

## Ordered work packages

| ID | Scope and inputs | Deliverable / completion gate | State |
|---|---|---|---|
| API-00 | Pin DLL, PE sections/directories, imports/exports and live identity | Reproducible inventory, export ordinal/name map, byte anchors and saved-state record; no generic match percentage | Complete for initial PE/boundary scope |
| API-01 | EXE/DLL Application boundary: getter, four slots, globals, callback ABI | Cross-binary argument/return ledger; independently inspect stored VTable types and target signatures; document unresolved exception returns/lifetime paths | Initial boundary gate established; zero-argument original convention and deeper lifetime/concurrency questions remain explicit |
| API-02 | Isolated comparison-only DLL build and first small bodies | Separate DLL job profile, effective source/command/library hashes, EXE regressions protected; first target GetWindowHandle21 bytes, then GetApplication72 bytes; validate references and export144 independently | Linked window21; factory72/table16 object gates complete; final linked factory/export table pending |
| API-03 | Private heap dependency slice:2013623e,20135dd9 and direct helpers | Initialization/alloc/free/error-mode ABI, data ownership and initial state; bounded body pilots with no fake dependency equivalence claims | Allocator/initializer/reallocator/registration source compiled; helper whole matches remain open, native permutations stopped |
| API-04 | Lifecycle wrappers: Initialize2012048a, slot042012052a, slot0820122817, common wrapper2013e3c1 | Five-word initializer ABI, callback return/SEH behavior, exit and message callback contract, full body/reference checks | Shutdown11/loop29/SEH137 and FDI140 object code verified; Initialize123 partial bytes only; implementations/link integration open |
| API-05 | Initialization implementation rooted at20120869 | Establish real reachable instruction ranges across unusual transfers, identify phases and outputs, split into dependency-sized subpackages; then reconstruct each | Init/main-window/language success-flow inventories and geometry source/type checkpoint established. New monitor/geometry/language source remains unmatched; complete-code ledger1028. Next font lifecycle20121083/enumeration, then dependency integration; see api-window-geometry.md, api-language-module.md and api-code-ledger.md |
| API-06 | Shutdown20120ed5 and message loop20122ae1 | Object teardown, callback/global lifetime, event/window operations and return behavior; code/data regression gates | Splash-stop2013b6a498-byte gate verified as shared lifecycle dependency; main shutdown/loop implementation pending |
| API-07 | Expand remaining exports/subsystems by verified call graph | Rank substantial code by callers, outputs and dependencies; maintain same-hash range ledger as coverage grows | Planned after first accepted DLL body baseline |
| API-08 | Module integration | Export/ordinal/import/data/relocation reconciliation and explicit resource/debug/certificate policy; layout work only after useful body coverage | Deferred |

API-03 precedes full matching of GetApplication dependencies, but API-02 can
first compare GetWindowHandle without rebuilding the heap subsystem. A matched
caller with mapped heap references does not certify the heap implementation.
Parallel package IDs indicate conceptual boundaries, not authorization to run
multiple agents or simultaneous worker controllers.

## First boundary evidence and immediate next steps

See [api-application-boundary.md](api-application-boundary.md). Start by inspecting
the wrapper's EH handler/scope table and the global teardown reads/writes at
2012103f/2012105a. Then define the minimum callback result contract and prepare
API-02's separate DLL profile. Do not create large functions across unknown
control transfers merely to make the decompiler emit C.

## Acceptance and stopping rules

- User source-style preference: avoid goto in maintained C++ where conditions,
  loops or ordinary returns preserve the observed flow. Decompiler output and
  historical snapshots may contain gotos; do not rewrite evidence. Recheck gates
  after source restructuring and disclose any changed matching result.

- Every finding records binary/hash, address/type path, evidence and status.
  Original bytes, current database declarations and hypotheses remain distinct.
- Before any database edit, check current state and take an appropriate backup;
  preserve names/types, compare VTable definitions and target signatures separately,
  verify readback and save. User controls tab changes. This phase made no DB edits.
- Each compiler/source question has a written hypothesis and predicted observable
  outcome, at most3 targeted trials. Stop earlier on success, identical output,
  lost reference mapping or regression. A new question requires new evidence.
- Use natural C++ and compiler output. No arbitrary byte masking, binary patching,
  fake original padding or guessed class sizes. Tiny isolated pilots are explicitly
  comparison-only; unresolved external dependencies remain unresolved.
- Retain unique job IDs, original/effective sources, compiler/linker command,
  receipt and EXE/DLL/PDB/MAP/disassembly hashes. No automatic baseline promotion.
- Compare raw bytes and individually validated target normalization separately;
  include entire stated ranges and negative controls for new acceptance checks.
  Source presence, token alignment and matching CFG counts are not coverage.
- Do not run a candidate as an installation replacement. Runtime validation, if
  later useful, needs a separate bounded plan using the original installation.

## Reuse and requested inputs

Reused `scripts/lib/pe-image.ps1` for PE bytes/import identity and the existing
worker queue design. `scripts/inventory-api.ps1` supplies original API evidence;
EXE-specific comparators/order/defaults remain EXE-specific. Evaluate additional
tools only against a concrete need (for example linked VC8/PDB compatibility),
consulting primary documentation then. Internet research is unnecessary for the
current boundary evidence.

Nothing additional is required from the installation yet. If a package needs a
missing dependency, matching header/import library, resource or runtime data,
request the specific file/version/hash and purpose. Do not copy the whole
installation without a scoped need. No VM setting/toolchain replacement needed.

Evidence:artifacts/api.dll/api-inventory/{original.json,exports.csv,imports.csv},
artifacts/api.dll/api-boundary-{live,memory,types,xrefs}-20261103.json.
Ghidra API revision1793405534817:636d9e2e:3, unsaved_changes=false. Both programs
open, fsx.exe current as reported by MCP; api.dll addressed explicitly. No tab
switch, edits/save/backup or candidate build/execution. Worker reports ready.
