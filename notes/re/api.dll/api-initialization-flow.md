# API initialization: reachable flow and bounded work packages

The readiness and multimedia timer tail now has a separate source
implementation and verification note. See api-startup-timer.md.

## Splash image/pathed-string package checkpoint (2026-11-03)

User explicitly authorizes following all routine steps without confirmation;
use larger package checkpoints rather than stopping after each helper. Structured
source/goto preference and bounded questions remain in force.

Image2013afdd raw445-byte body verifies LoadImageA EAX result preserved through
cookie check and RET. Literal200f34ac is uires\\dlgsplash.bmp,200f3134 is colon.
It loads IMAGE_BITMAP0 with flags2010h (LR_LOADFROMFILE|LR_CREATEDIBSECTION),
default size andNULL module. Original code contains a generic low-word string-
resource path, but fixed literal address bypasses it. Decompiler unreachable
warnings reflect this static specialization, not evidence that paths should be
patched or that generic source never existed. Loader20138550 is stack3/RET12,
resource callback dispatch then LoadStringA fallback. Trim20135114 uses EAX path,
CharNextA and _mbsicmp to remove one final backslash (multibyte-aware traversal).

Added startup_image.cpp with structured path construction and explicit unresolved
custom trim/copy source interfaces. No unnecessary local zero buffer was invented
to match original frame. Source488 versus445 in api-splash-image-20261103-001;
stop initial source-shape question at1, no whole image-body/runtime claim.

Two new complete matches from natural source: path_append.cpp append20137ec0
62 bytes after one validated REL32 copy target20137e17; length20137fb0 raw34
bytes, NULL returns0. Append computes NULL-aware length, calls copy only when
length<capacity, returns original destination regardless of copy result. It does
not validate NULL destination before such a copy. Run api-path-append-20261103-001
first compiled append; later image run adds length and both gates succeed.
Comparator scripts/compare-api-path-string-helpers.cjs checks full bodies, zero
addend/target, and rejecting opcode mutations in both plus wrong copy target.
Final object SHA2565BCFD70083BDC748B7557ACD83211CBBA5016A1A31B256866DF86AE33C152912.
All six comparison scripts pass on that object:840 disjoint complete code bytes,
44 metadata; Initialize partial123 separate. Stop successful helper questions.

Ghidra actual changes after GZF: image return void* (opaque returned GDI handle),
no args/cdecl; append char* stdcall3; length uint stdcall1; trim void custom EAX
input/no stack words with cdecl cleanup model. Preserved established FUN names,
code bodies, bytes and other program. Script TypeApiSplashImage.java checks all
four target names/body sizes/old parameter counts. Readback now exposes image
HANDLE return and trim argument; parameter locations checked in script output
and saved snapshots. No table edits or navigation.
Backup:artifacts/api.dll/ghidra-backups/api-before-splash-image-types-20261103.gzf,
8541139 bytes. Saved revision1793738170185:636d9e2e:567, unsaved=false.
Post-edit image445 bytes equal pinned original. Evidence artifacts:
api-splash-image-evidence-20261103.json, api-splash-image-typing-20261103.json,
api-splash-image-post-type-memory-20261103.json. Source search still has no goto.

This is a larger splash/path source-and-interface checkpoint; matched stop98 and
string96 are accepted, image/start/thread/procedure remain explicitly unmatched.
Next main initialization/window entry2012205d: inspect actual flow/return across
its misleading small stored function before source. Continue routine work under
existing authorization; no additional installation input required at checkpoint.

## Structured source and splash procedure model (2026-11-03)

User preference: avoid goto in maintained pilot source where structured control
flow expresses the same behavior. Search found exactly one in src/api-pilot,
startup_window.cpp ready-event branch. Replaced with explicit wait-result tests:
ready event keeps thread handle, thread termination closes it, other results post
WM_QUIT then close. No goto statements remain in current src C++/header search.
Historical build snapshots and Ghidra decompiler-generated gotos remain evidence;
they are not canonical source and are not rewritten retroactively.

Added startup_window_procedure.cpp model for475-byte original2013b1c1. Handles
WM_CREATE (compatible memory DC/select image), WM_DESTROY (restore object,
delete image/DC, clear globals, PostQuitMessage), WM_PAINT (BitBlt rcPaint with
SRCCOPY), WM_ERASEBKGND0, WM_MOUSEACTIVATE4, WM_NCHITTEST1 and WM_NCCALCSIZE
(rgrc[2]=rgrc[0] for nonzero wParam). CREATE/DESTROY and visible DISPLAYCHANGE
then fall through to DefWindowProcA as observed; do not turn them into early0
returns. DISPLAYCHANGE uses original flags2455h/800x600. Source relies on an
explicit unresolved rectangle source boundary, because original2013b19a uses
EAX rectangle, then2013e5f0 uses ESI rectangle/ECX monitor. No ABI equivalence.

Run api-splash-procedure-20261103-001 uses fixed factory native profile; object
SHA256226DBE6C1AC3CA3F8250B582C2A5CDEFB6E635EAC7D14FD6E002B9BB1F7DB6C4.
Structured start225 bytes (earlier goto226, original227), procedure620 versus475.
No accepted new match. Single trial for new procedure, no register/flag/padding
permutations; stop on explicit source-boundary/dispatch/scheduling differences.
Five existing gates and all their negative controls pass;744 complete code/44
metadata unchanged. Diagnostic contains raw emitted bodies/references. No runtime,
database changes or navigation; last checked saved revision507.

Evidence:artifacts/api.dll/api-splash-procedure-evidence-20261103.json includes position
helper and image2013afdd decompilation. Image's existing void decompilation drops
LoadImageA result and warns of unreachable blocks: inspect raw return/control
before reconstructing445-byte image function. Image load uses flags2010h; no
resource/file origin claim from helper names alone. Next address2013afdd, with
path helpers20135114/20137ec0 and return flow first. Suggested procedure rename
FUN_2013b1c1 -> SplashWindowProcedure, not applied; class/thread evidence supports
it. User autonomy authorization continues, no per-step confirmation required.

## Autonomous continuation: disk query and splash lifecycle (2026-11-03)

User authorizes continuing routine packages without per-step confirmation.
Same trial/backup/provenance rules remain. Central address/count ledger is now
[api-code-ledger.md](api-code-ledger.md). Arithmetic correction: historical648
after507+139 must be646. With newly verified98 bytes the correct total is744,
not746. Individual comparisons/addresses unchanged. Metadata44 stays separate.

Disk query20133e19: implemented disk_space_query.cpp, stdcall3 with total then
available outputs. Path splitter20133b3b has eight stack words/RET32, including
optional root output/capacity; UNC handling and CharNextA traversal observed,
not yet reconstructed. Query preserves ignored helper returns, root extraction,
backslash append, zero outputs and cached query ordering. Source interface for
original custom copy remains explicit. Trial api-disk-query-20261103-001 builds
363 bytes versus318; copy inlined, local/register scheduling differs. One trial,
no acceptance; stop this bounded ABI/inlining question. Object SHA256
BC9EC3E906893C4CAFDFEC1A91F254B7DDC6C0A45AD42E5E3F8E4521A6522694.
Native diagnostics in run; splitter evidence api-disk-query-split-evidence file.

Window phase proven to be splash lifecycle, not main-window creation. Thread
2013b444 registers FlightSimSplash, creates800x600 window2020e0a4, records
monitor2020e0ac, signals supplied event, runs GetMessageA>0 loop and destroys
remaining valid window. Rendering window procedure2013b1c1 remains unresolved.
Start2013b5c1 creates manual-reset nonsignaled event, image2013afdd, and thread;
wait array order is thread then ready event. Event wins: retains thread handle.
Thread wins: closes handle and resets global. Other wait result: posts WM_QUIT,
then closes/resets. It closes ready event and deletes image when thread global
isNULL, otherwise returns splash HWND. Thread readiness and parameter lifetime
are original observed structure, not runtime safety guarantees.

Source startup_window.cpp in api-startup-window-20261103-001 produces226 versus
227 bytes; register-save shrink wrapping/frame placement differ. Stop at1 without
frame padding/forced register instructions. Source startup_window_thread.cpp in
api-startup-thread-20261103-001 produces354 versus381 bytes; first natural model
retained diagnostically, no whole match claimed. Stop that source-shape question
at1; dependency/rendering analysis can supply new evidence later.

Complete new match: stop2013b6a4,98 bytes, source startup_window_stop.cpp.
Checks HWND/IsWindow, posts WM_CLOSE, waits thread indefinitely, closes nonNULL
thread handle and clears global, unregisters FlightSimSplash with instance2020e024.
No invented image deletion or HWND reset in this helper. First run
api-startup-stop-20261103-001, object SHA256
E590E4818C6E83B5A073008A4BCD446B7818BA3D4304C2AD87DB2A5133AF93FF.
Full98-byte comparator scripts/compare-api-startup-stop.cjs validates12 DIR32
fields and native/original class-string content. Opcode/global/WM_CLOSE-to-WM_QUIT
mutations reject. Same gate survives real thread definition in subsequent run.

Latest shared regression api-startup-thread-20261103-001 object SHA256
6510829959EB4115B9D89B5773B3A90F31ECF6A7DF72FF5E02239A6F8E649906:
all five comparison scripts pass,744 complete code/44 metadata. Source/commands/
receipts retained under module runs/jobs. No candidate execution/database edits,
last checked saved API revision507; build work does not refresh DB save evidence.
Original/decompiled/assembly snapshots:api-startup-window-evidence-20261103.json.

Show2013b706 additionally invokes center-rectangle2013e5f0 with ECX monitor and
ESI rectangle, then SetWindowPos flags6455h. This is another custom register ABI;
do not normalize to a regular one-argument helper. Next useful analysis is splash
procedure2013b1c1/image2013afdd and shared teardown, then main-window2012205d.
Suggested provisional renames, not applied: FUN_2013b5c1 -> StartSplashWindow,
FUN_2013b444 -> SplashWindowThread, FUN_2013b6a4 -> StopSplashWindow, supported
by exact FlightSimSplash class identity. User's existing names remain untouched.

## Legacy disk-space fallback source (2026-11-03)

Original125-byte201325f1, pinned binary/hash below. Live assembly retained in
artifacts/api.dll/api-disk-fallback-evidence-20261103.json. stdcall4/RET16 calls
GetDiskFreeSpaceA through IAT200e1854, using its own path argument slot for the
sectors-per-cluster output. Failure returns original BOOL with outputs untouched.
On success: multiply sectors-per-cluster and bytes-per-sector at32-bit width,
sign-extend product and free/total cluster counts separately with CDQ, use
201ec9ea (__allmul) twice, write free amount to available output, optionally copy
it to freeBytes output, then write total. available and total are unconditionally
dereferenced on success; only freeBytes is optional in this fallback. Do not
generalize modern API optional-output behavior onto the legacy implementation.
It reports the same physical free amount for available and freeBytes, without
the separate per-caller/quota distinction of the modern target. No runtime test.

Added disk_space_fallback.cpp preserving these operations, explicit32-bit wrap
before signed widening, and argument-slot reuse. First native trial
api-disk-fallback-20261103-001 produces125 bytes with3 reference fields but
loads bytes-per-sector then IMULs sectors-per-cluster; original loads sectors
first then multiplies bytes. Trial002 exchanges source multiplication operands
to test whether ordinary expression order changes the compiler schedule. Emitted
helper body remains identical; stop at2, no more variants on this question.
Other arithmetic operand identities are retained, but exact compare fails.

Comparator scripts/compare-api-disk-fallback.cjs deliberately rejects full-body
mismatch; no byte masks or accepted partial coverage added. Diagnostics with raw
helper bytes/references retained in both runs' disk-fallback-diagnostic.json.
Final object SHA256DEF38A2FC1A4835FEE85F546EBD6EC246C229D880D24D24EEE57489701E7AC25.
Prior648 complete-code/44 metadata gates pass, including dispatch139, error48,
path49 and lifecycle/factory410. No new fallback matching acceptance.

No Ghidra changes/navigation or candidate execution; last checked saved revision
507 remains the database reference. Next source query20133e19 (318 bytes), using
actual output order and unresolved path-splitting20133b3b interface. Do not reopen
the fallback operand-order question without new compilation-context evidence.

## API-05c disk dispatch matched and gate semantics (2026-11-03)

Addresses: disk query318-byte20133e19; lazy dispatch139-byte2013266e;
fallback201325f1; cached pointer201f7538. Binary/hash pinned below. Live
query/dispatch assembly and pointer snapshot:artifacts/api.dll/api-disk-space-analysis-
20261103.json. Initial pointer is2013266e in live memory and original file.
Pointer xrefs: query READ20133efa, dispatch WRITEs201326c3/201326dd/201326e8.

Dispatch resolves GetDiskFreeSpaceExA from GetModuleHandleA("KERNEL32"). Missing
API caches fallback then tail-jumps to it. Resolved API returns success or a
failure other than GetLastError()==78h: caches API and forwards BOOL result.
Failure78h caches fallback and invokes it with all four original stack arguments.
No added module-NULL guard or locking: retain observed behavior. Runtime pointer
race/lifetime and fallback implementation remain outside current acceptance.

Added disk_space_dispatch.cpp in existing factory COFF profile. Trial001139 bytes
had reordered string pushes and early missing-API path. Explicit second hypothesis:
separate module lookup statement and nest non-NULL success branch to reproduce
observed lookup/branch ordering. Trial002 matches full139-byte body after12
relocations. Native/original string contents checked independently; original
initial pointer checked. Comparator:scripts/compare-api-disk-dispatch.cjs.
Opcode, relocation-field and error78h-constant mutations reject.
Final run api-disk-dispatch-20261103-002, object SHA256
5DBF9F111F85D8449A06C6D97AFE2CF55C2DFC607CB72EDACDED8198C71D1063.
Retain both controlled source forms/commands/receipts. Stop on trial2 success.
Prior507 complete-code/44 metadata gates pass; new disjoint complete-code total
648 bytes, metadata44. Initialize123 partial bytes separate. No linked DLL,
fallback body or runtime equivalence claim. No Ghidra edits; saved revision507
is last checked state, unrelated to code-byte counts.

Query20133e19 uses default base2020d360 forNULL path, otherwise resolver selector0.
It ignores copy/resolve status, extracts drive/root with20133b3b, appends backslash
200f1ce4 if capacity permits, initializes optional64-bit outputs to0, and calls
cached API(root,&available,&total,NULL). Caller output2 receives total bytes;
caller output3 receives available-to-caller bytes. Query returns-1 on API failure,
0 on success. No initialization-return test is made for earlier path helpers.

Startup20120ad7 calls query(NULL,NULL,&local64), hence tests available-to-caller
bytes, not total volume capacity: high DWORD nonzero or low>=1e00000h (30MiB)
passes; otherwise nonzero EDI bypasses rejection, zero EDI reports error14/returns0.
EDI override provenance remains open. The previous provisional disk-space
interpretation is now resolved by actual API lookup name and argument ordering.
Startup20120aa3 GlobalMemoryStatusEx uses struct atESP+34h: total-physical low at
ESP+3ch/high atESP+40h, threshold4000000h (64MiB), error6 on rejection. It does
not inspect the API BOOL result before reading fields. This documents original
control flow, not a runtime validation or improved failure policy.

Next fallback201325f1 and cached-pointer initialization/data ownership, then
source for query20133e19. Keep separate bounded questions and prior gates. Naming
suggestions, not applied: FUN_2013266e -> ResolveDiskSpaceQuery;
FUN_20133e19 -> QueryVolumeSpace. Current existing FUN/PTR names preserved.

## Initialization error wrapper matched (2026-11-03)

Original api.dll201230da,48 bytes, export264; pinned hash below. Live assembly
and old stored undefined(void) signature retained in
artifacts/api.dll/api-initialization-error-evidence-20261103.json. Assembly establishes
stdcall3/RET12, default EAX0, optional six-word callee-cleanup callback through
2020de78, forwarding EAX result unchanged. Callback arguments: errorCode, detail,
address2020d468, third wrapper word, constant65h, value at2020e028. Source names
and char-pointer interpretation are reconstructed; no callback implementation or
meaning of every word is claimed.

Added src/api-pilot/initialization_error.cpp in factory COFF translation unit.
First trial api-init-error-20261103-001 succeeds at complete48-byte equality after
three individually verified DIR32 global mappings (offsets7,18,36). Original PE
export264 association independently checked from pinned EAT. No linked rebuilt
ordinal/export claim. Object SHA256
77789F2C19ABD8DAB81888A144BC909051C06E960E1DFE5903251C737802087E.
Comparator:scripts/compare-api-initialization-error.cjs; result retained as
initialization-error-comparison.json in the run. Negative opcode, callback-global
and fixed callback-argument mutations reject. Prior410 code/44 metadata and
49-byte path wrapper gates pass all existing negative controls.

New disjoint complete object-code total507 bytes; metadata44 unchanged, partial
Initialize123 still separate. This48-byte wrapper does not certify its callback,
global initial values/lifetime or complete initialization. Stop trial question at1
on success. No database mutation/navigation or candidate execution; last checked
saved API database revision507 (a revision number, unrelated to code-byte total).
Next API-05c memory/free-space gates and20133e19 dynamic query target201f7538,
without reopening custom-register path infrastructure.

## Suffix source boundary built (2026-11-03)

Added path_suffix.cpp with original signed selector branches, fixed8-byte table
stride, dynamic table offset70, special-2, strict inputLength>suffixLength and
case-insensitive suffix comparison. Empty suffix suppresses append; matching
suffix suppresses append without changing case of the input. Equal-length suffix
strings still take append path. Copy/append failures become-1; success0. As in
original assembly, other negative indices are not clamped and NULL input is not
validated. Built-in/dynamic table storage remains unresolved, with names that
explicitly identify source boundaries. No invented original-data layout beyond
the observed8-byte entry stride.

Run api-path-suffix-20261103-001 uses unchanged native factory COFF profile,
object SHA2568345703CC8B5E952C83552DC62887762DB1D3ED37E20E36ABEF1ED18E74664CB.
Native suffix251 bytes versus original182: ordinary stdcall3 source differs from
original EAX/ECX plus stack1 ABI; compiler also inlines source copy loop, visible
from absence of copy relocation and loop instructions. suffix-diagnostic.json
retains body bytes and references. Resolver238 and copy82 bodies remain emitted.
Do not map source calls/tables as if a linked original-equivalent implementation
exists. Append remains an unresolved source interface; no runtime candidate run.

Existing410 code/44 metadata and49-byte wrapper pass all existing negative
controls, leaving459 complete object-code bytes. No suffix matching acceptance.
Stop this source question at1: custom ABI and inlining residuals are explicit;
no noinline/flag/array-size matrix. Assembly/storage evidence retained in
artifacts/api.dll/api-path-suffix-source-evidence-20261103.json. No Ghidra changes;
last checked API revision507 saved. Source step does not refresh database state.

The bounded path source slice is now sufficient to expose its implementation and
ABI gaps. Next return to initialization201230da error-callback wrapper (48 bytes,
regular stack interface), then memory/free-space gates under API-05c, rather than
continue custom-register infrastructure permutations. Final module remains a
separate api.dll PE; this run emits only a native COFF object.

## Copy source boundary built; bounded question closed (2026-11-03)

Original200f79d5 live assembly/storage retained in
artifacts/api.dll/api-path-copy-evidence-20261103.json, pinned API hash below. Added
path_copy.cpp implementation of the previously unresolved source boundary.
Zero capacity returns80070057 without reading/writing strings. Otherwise copies
bytes until NUL or capacity exhaustion; exhausted output pointer moves back one
byte, writes NUL, returns8007007a. Success writes NUL and returns0. No added
NULL/overlap checks or replacement by a CRT routine. Source assumes valid
nonzero-capacity pointers just as the observed loop does. Semantic correspondence
is by assembly/source inspection, not runtime validation.

Trials api-path-copy-20261103-001 and002 use unchanged factory native profile.
Trial002 removes an empty conditional left in trial001 source; both emit exactly
the same82-byte relocation-free helper. Original86-byte helper uses EAX/EDX plus
one stack word/RET4; this source uses three stack words/RET12. Also native
termination/control scheduling differs. No whole-body/ABI match or new coverage.
Stop at2 on identical emitted code; do not chase flags/register forcing.
Final object SHA256DA2227B86E715A9C261C974D81693735472BB99EC4AB112131A47C16716A5A60.
Each run retains copy-diagnostic.json with bytes/counts. Existing410/44 and49-byte
wrapper comparisons/negative controls pass, complete scope stays459 code bytes.

No candidate execution/database mutation/navigation. Saved API revision507 is
the last verified database state, not a fresh save in this build-only task.
Next selector source dependency20134fc4; keep its EAX/ECX original ABI distinct
from the source interface. After this bounded path slice, return to noncustom
initialization helpers rather than attempt an infrastructure compiler matrix.

## Resolver source model built (2026-11-03)

Added src/api-pilot/path_resolver.cpp to the existing factory COFF translation
unit. Trial api-path-resolver-20261103-001 uses the unchanged fixed native profile;
object SHA2566839A27DC137AD5EC0FBC209B8639590BA7D7EEFC2CA56AFF8022B297E3535B2.
Retained compiled sources/command/receipt under the module-owned run/job paths.
Natural source captures selector0 NULL rejection, nonzero selector processing,
relative base2020d360, combine/canonicalize, capacity260 gate and ignored copy
result. No helper substitutions are linked or executed.

Original copy20137e17 has three stack arguments/RET12, returns destination in
EAX, and includes a multibyte-aware branch; its return is ignored at this caller.
Source declares the pointer result without replacing implementation by strcpy.
Assembly snapshot:artifacts/api.dll/api-resolver-source-evidence-20261103.json.

Native resolver238 bytes, versus258 stored original body addresses (the latter
is Ghidra body cardinality, not an independently certified contiguous extent).
resolver-diagnostic.json records eleven native reference fields. Two dependencies
SelectSuffixSourceBoundaryPilot and CopyPathSourceBoundaryPilot deliberately use
ordinary source stack interfaces instead of the original custom register ABIs.
They remain undefined; do not normalize their calls into original targets or count
resolver bytes as coverage. Explicit source-boundary names distinguish this model
from the observed machine interface. Original suffix/copy helpers remain open.

Existing410/44 lifecycle/factory gates and49-byte wrapper pass with negative
controls; attribute routine remains202 bytes/unmatched. No complete coverage
increase. Stop resolver source-model trial at1: ABI and scheduling differences
are already visible; flag/array-size sweeps cannot establish the missing boundary.
Next bounded source question is the86-byte copy200f79d5, whose exact copy/error
behavior is known, followed by selector182-byte dependency integration. Preserve
custom-ABI residuals explicitly; no assembly forcing or opaque target masking.
No database edits/navigation in this source step; last saved revision507.

Module clarification: final architecture remains separate fsx.exe and api.dll PE
images. This trial emits only a COFF object (canonical api-factory.obj). The
worker transport filename fsx.exe does not change object format or combine DLL
logic into the EXE. Existing small DLL pilot and final linked API/export gates
remain separate from this object evidence.

## Cookie-helper model repaired (2026-11-03)

Live/original20148b71 bytes3b0d84621f207502f3c3e997040000 establish CMP
ECX,[201f6284], unequal jump, prefixed REP RET on equality, and tail JMP to
20149017. EAX/EDX are untouched on successful return. Existing target
___report_gsfailure is already noreturn; its inspected body reports failure and
calls TerminateProcess. Preserved both established names and failure target.

Applied void __fastcall __security_check_cookie(uint cookie), ECX storage,
zero stack purge, and existing compiler-spec call fixup security_check_cookie.
The fixup was present in the compiler specification and absent on the function.
This is decompiler modeling of the returning path, not a binary patch or proof
the validation/failure path can be discarded. Resolver readback now returns0/-1
directly, and attribute/fallback callers no longer obtain a fabricated cookie
result. Source/raw assembly remain authoritative for failure behavior.

Pre-edit GZF:artifacts/api.dll/ghidra-backups/api-before-cookie-model-20261103.gzf,
8540511 bytes. Script:scripts/ghidra/TypeApiCookieCheck.java; retained before/
after and saved-state snapshots api-cookie-model[-saved]-20261103.json. Initial
script byte guard rejected before mutation because RET's F3 prefix was mistaken
for a padding byte; raw read corrected this to exact full15-byte validation.
Final transaction, ECX readback and save succeeded, unsaved_changes=false.
No changes to code/body/table/navigation, no build or coverage gain. The previous
cookie-return artifact is superseded. Next resume resolver source work with
explicit custom ABI boundaries rather than more type scaffolding.

## Applied path interface typing (2026-11-03)

Actual Ghidra changes: seven existing functions20133d17,20134fc4,200f79d5,
201203f4,2013341b,20132c8e,2014507f now have explicit EAX result types and
assembly-verified parameter storage/purge sizes. Original FUN names preserved;
no previously suggested renames applied. Script TypeApiPathInterfaces.java
checks original names/body sizes/zero stored parameters before its transaction,
then asserts each stored parameter location after update. No body/table/byte
changes. __stdcall on register-bearing functions is only the cleanup model;
custom storage is part of the signature and must be retained.

Pre-edit api.dll current, fsx.exe also open, saved revision314. GZF backup
artifacts/api.dll/ghidra-backups/api-before-path-typing-20261103.gzf (8531846 bytes).
Readbacks: artifacts/api.dll/api-path-typing-readback-20261103.json and
api-path-typing-saved-20261103.json. Database save completed and saved-state
readback confirms unsaved_changes=false. No user tab navigation or build.

Decompiled selector now shows all three real arguments and copy/append operands.
Resolver and other cookie-checked routines still incorrectly decompile their
return as __security_check_cookie's result. This is separate from corrected path
storage: original assembly establishes0/-1 before that call. Next inspect the
stored cookie-helper target/calling convention and actual EAX preservation before
changing any compiler-helper type. Do not replace assembly return evidence with
this remaining decompiler artifact. No additional code coverage;459/44 retained.

## Resolver/selector contract (2026-11-03)

Evidence: live explicit-api.dll assembly in
artifacts/api.dll/api-path-resolver-analysis-20261103.json; original pinned-file70-entry
table inventory api-path-suffixes-20261103.json and live560-byte table snapshot
api-path-suffixes-live-20261103.json. Original hash unchanged as specified below.
Status: assembly/file verified; names are provisional. No new build/match claim.

Selector is a suffix-table index, not an access mask. Table200efd98 has8-byte
entries; observed0 empty,1 .BGL,2 .STN,35 .SC1,36 .CFG,37 .MIS,69 .PNG.
20134fc4 selects this table for signed selector<70 except-2; selector>=70 and
below70+2020e008 uses pointer table2020e00c[index-70];-2 or indices beyond the
upper bound select empty string200f24c0. Other negative selectors are not range
checked in this assembly; do not silently add a safety clamp or infer support.
No runtime value or lifetime for the dynamic table has been established.

Custom20134fc4 ABI: EAX selector, ECX input string, one stack output pointer,
RET4, EAX status0/-1. It checks for an existing case-insensitive suffix only when
input length is strictly greater than suffix length, copies input to output with
capacity260, then appends suffix if needed. Equal-length strings do not take the
suffix-present case. Empty suffix produces no append. Keep these details rather
than replacing them with a general extension-changing API.

20133d17 itself is stdcall4: output, capacity, input, selector. Nonzero selector
uses20134fc4 with the original input even if NULL; selector0 checks input forNULL
and uses it directly. Relative paths combine with copied global2020d360, not a
fresh GetCurrentDirectory query. Capacity must be>=260; PathCanonicalizeA writes
to a local260-capacity buffer, then20137e17 copies to caller output. Resolver
returns0 on this path, ignoring the copy helper's result, otherwise-1.
The actual caller clears EAX after the final copy. No original local array-size
or prototype claim is derived from decompiler local names.

Copy200f79d5 uses EAX source, EDX destination, stack capacity/RET4, HRESULT-style
EAX:0,80070057 for zero capacity,8007007a for truncation; output is terminated
on truncation. Append201203f4 uses EDI destination, ECX capacity, stack suffix/
RET4 and calls the same custom copy. These custom interfaces explain missing
arguments and fabricated fastcall inputs in earlier decompilations. Existing
stored signatures have not been changed.

Consequently startup selector36 resolves terrain/scenery to .CFG names, selector1
resolves the four Scenery paths to .BGL names, then relative paths are based on
2020d360. This is a static transformation inference; actual file existence and
runtime canonical output have not been checked.

Next bounded implementation task: preserve these custom register interfaces in
Ghidra with separate target/storage verification and backup/save, then choose a
natural source model with explicit ABI boundaries. Do not map a standard-call
helper to the custom original as if the call instructions were equivalent.
No speculative native build was issued for this unresolved ABI question. No
tab navigation/database edits; revision314 remains saved, no unsaved changes.
Suggested renames (unapplied): FUN_20134fc4 -> CopyPathWithSelectedSuffix,
FUN_20133d17 -> ResolveSelectedPath, based on the flows above. No extra files
from the installation are needed for this scope.

## API-05b attribute dependency source (2026-11-03)

Original binary/hash as pinned below. Live assembly of20133d17 confirms four
stack arguments/RET16 despite its misleading fastcall decompilation. Caller
2013341b supplies buffer, capacity260, path, selector.2014507f uses one stack
argument/RET4 and returns attributes orFFFFFFFF; do not treat its void decompiler
output as the contract. Raw dependency listings retained in
artifacts/api.dll/api-path-attribute-dependencies-20261103.json; names remain unchanged.

Natural source in path_checks.cpp now implements the202-byte attribute routine:
resolution failure returns-1; Win32 attributes fall back when unavailable; selector
-2 requires a directory, retrying fallback for a non-directory. Unavailable/failing
paths leave output untouched. Success maps attribute bits80/20/01/02/10 to
01/04/02/08/10 and returns0. Buffer260 is the observed call capacity, not a claim
about original local-array declaration. Fallback is unresolved subsystem logic,
not a substituted GetFileAttributes implementation.

Trial api-path-attributes-20261103-001 (same fixed factory COFF profile) produces
202 bytes, but original frame114h versus native108h, buffer displacement-110h
versus-108h and early failure scheduling differ. No new match accepted. Diagnostic
scripts/inspect-api-path-attributes.cjs records seven native reference fields;
it does not claim offset-aligned target normalization or body equivalence.
Object SHA25682585610D20AB205B9BECC207C9CAA280B0E5A7735C2134094813C499447790D.
Prior410/44 gates and49-byte wrapper pass; complete-code scope stays459 bytes.
Wrapper comparator initially rejected the now-defined helper symbol (previously
external); updated to validate unique exact symbol/value with either defined or
external section. Full byte/opcode/target tests still pass. No emitted-code fix
was required for this comparator assumption.

Stop this source-shape question after1 trial: equal length alone is insufficient,
and no evidence justifies changing buffer sizes or flags. Next trace path resolver
20133d17 and its selector-dependent20134fc4 interface, then fallback2014507f;
keep a separate at-most3-trial budget for a new explicit source hypothesis.
No Ghidra edits/save/navigation or candidate execution. Latest API tool revision
1793724055614:636d9e2e:314 remains saved. Suggested provisional rename
FUN_2013341b -> QueryPathAttributes, supported by output bit conversion and
status returns; not applied. No additional installation input needed.

## API-05b wrapper milestone (2026-11-03)

First natural source trial api-path-wrapper-20261103-001 matches all49 bytes
at20132c8e after one REL32 mapping to2013341b. Comparator
scripts/compare-api-path-wrapper.cjs checks zero COFF addend, external stdcall3
symbol, original target and full body; opcode/wrong-target controls reject.
Run path:vm-share/modules/api/runs/api-path-wrapper-20261103-001; object SHA256
75B9CAB17B8FD9055FA805966A004A1208DC560C2BE95EBB64D671B67855F956.
Uses existing fixed factory COFF profile, adding path_checks.cpp to the real
source translation unit; profile name remains factory, run ID identifies scope.
Prior410 code/44 metadata gates pass; new disjoint total459 complete object code
bytes. Partial Initialize123 unchanged. Stop wrapper trials successfully at1.
2013341b remains unresolved, so this does not certify its implementation.

Assembly resolves both return contracts: wrapper stdcall2 returns-1 on helper
failure or outputFFFFFFFF, otherwise0; helper stdcall3 returns-1 at20133452
and0 at201334d7. Its output is written only on success. The wrapper deliberately
reuses its second argument slot as output storage; selector is passed unchanged
into the helper before overwrite. These are selector values, not proven masks.
Source names CheckPathExistsPilot/QueryPathAttributesPilot are reconstructed.
No Ghidra edits; revision314 remains saved. Next202-byte dependency reconstruction,
with path resolution20133d17 and fallback2014507f interfaces checked first.

2026-11-03. Binary: original/api.dll, x86 PE32, base0x200e0000,
SHA2569CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Addresses below are original VAs. Status: live verified where assembly/raw bytes
are cited; phase and proposed names are reconstructed interpretations, not original
symbols. No new compiler/body-match acceptance in this step.

## Result and scope

API-05a has a bounded native-flow inventory rooted at20120869, ending before
the known shutdown entry20120ed5. It contains465 decoded instructions/1596 unique
instruction bytes, three8-byte gateway descriptors, and three skipped8-byte
ranges. These exhaust the1644-byte interval without overlaps. No unresolved jump
or stack transfer remains in this inventory. Calls are opaque: their return paths,
indirect targets and detached exception handlers are not certified by this walk.
This is not complete-function or reconstructed-code coverage.

The first direct-flow probe stopped at133 instructions/438 bytes because it
treated20120a00 RET4 as an ordinary return. That preliminary extent is superseded.
The final reader handles only the exact observed stack-transfer instruction forms
and the two independently inspected gateways. It rejects unknown RET4 forms by
recording an explicit stop; no generic assumption that RET means CALL.

Evidence: artifacts/api.dll/api-initialization-flow-20261103.json,
api-initialization-memory-20261103.json, api-initialization-gateway-20261103.json,
api-initialization-dependencies-20261103.json and
api-initialization-verification-20261103.json. All465 instruction byte strings
and all seven live memory spans equal the pinned original file. Scripts:
InventoryApiInitializationFlow.java, InventoryApiInitializationEvidence.java
under scripts/ghidra, and scripts/verify-api-initialization-flow.ps1.

## Entry, descriptors and exception handling

20120869 jumps directly to20120873, skipping2012086b..72. Two more direct jumps
skip20120afc..b03 and20120d03..0a. Keep these bytes unclassified; they are not
reachable instructions on the inventoried paths. No filler/source interpretation
is established.

The real prologue aligns ESP to8 and allocates0x178 bytes plus saved state,
installs FS:[0], and uses security cookie201f6284. The pushed201ee510 is an
exception-handler thunk, not a scope-table address. Its two cookie checks call
20148b71; it loads EAX=201f097c and jumps via201ec94b to IAT200e1034,
msvcr80.dll!__CxxFrameHandler3. This is C++ EH evidence; do not carry over the
separate outer wrapper's EH4 interpretation to this function. Detached handler
coverage and the full201f097c metadata graph remain open.

| Transfer start / RET4 | Gateway | Descriptor | Native resume |
|---|---|---|---|
|201209d7 /20120a00|201ecc2f|20120a03|20120a0b|
|20120a27 /20120a4a|201ecc87|20120a4d|20120a55|
|20120a5c /20120a8b|201ecc87|20120a8e|20120a96|

Each sequence saves a scratch register twice, synthesizes a descriptor address
and gateway target through LEA pairs, then restores the register and RET4s into
the gateway with the descriptor address on the stack. PUSH and the exact
LEA ESP,[ESP-4]/MOV [ESP],reg expansion occur. All three descriptors contain
DWORDs00001db8,00000000. They are data, not MOV EAX,1d/etc. instructions.

201ecc2f and201ecc87 save flags and ECX/EDX/EBX/ESI/EDI, preserve GetLastError
through GetLastError/SetLastError, call201e7347 and201e6c4d respectively, and
return helper EAX. Each helper advances the saved return pointer by8:
201e7add and201e732f independently confirm it. Thus execution resumes beyond
the descriptor. The first helper result is explicitly tested for negative failure
at20120a0b. Register/flags preservation does not imply the helper has no memory
effects or that the original incoming EAX is restored.

The descriptor RVA resolves to200e1db8, a21-byte thunk filling a caller-provided
record with201f6008, count0x4c, and201f6000, then returning. Decompiler output
for the two existing helpers shows critical-section/refcount management,
VirtualProtect and reversible byte/range transformations, including relocation
handling. The role as region enter/leave is provisional; no protection product,
complete descriptor format, decoded image or runtime behavior is claimed. Static
native initialization flow can continue beyond these descriptors without a
runtime dump. Do not expand into reconstructing this entire mechanism now.

## Initialization phases and dependencies

| Original range | Evidence and working interpretation | Remaining gate |
|---|---|---|
|201208b7..20120966|Clears2020dea0; calls option processing201301ce and path/config/language setup201219df; FindWindowA; existing-instance handling20122c4a/20122f99; event2020de80 and error callback checks|Verify callee returns;201219df's stored void decompilation contradicts immediate EAX test|
|20120967..201209ce|Sets module-instance global; config query2013174d; GetSystemMetrics(1000h); event/thread/window helper2013b5c1, show helper2013b706, MonitorFromWindow; GlobalInitializer marker2014ec4e|Window/thread ownership and cancellation paths|
|201209cf..20120a95|Enter gateway, calls2014852a/20148112, two leave alternatives; negative enter status returns0|Identify protected check callees without expanding protection implementation|
|20120a96..20120b0a|GlobalMemoryStatusEx, threshold0x4000000 (64MiB);20133e19 output at least0x1e00000 (30MiB) unless override; errors6/14|Disk/free-space interpretation of20133e19 needs dynamic target201f7538 and return-contract verification|
|20120b0b..20120c0c|Zeroes43 DWORDs at2020bb30; calls200f684d/200f5d90/200f5eaf; SetCurrentDirectoryA; six20132c8e checks;2012205d result gate|API-05b path/dependency checks selected next|
|20120c0d..20120d8c|ShowWindow with0 then1; CoInitialize/CoCreateInstance, strings/conversions20120252/2012031f, COM method at+18h, release/cleanup, later2013a20b/201226b0|Identify COM class/IID and detached C++ cleanup; do not trust incomplete2012205d void decompilation|
|20120d8d..20120ed4|Sets readiness2020dea0, calls2013e4d4 and callback2020e034(1); cancellation2020e044; virtual calls; window operations; timeGetDevCaps, timeBeginPeriod, timeSetEvent callback201228f0; common epilogue20120e8a|Timer period/failure/lifetime and callbacks, shared with API-06|

The six path literals are terrain, scenery, Scenery\\BASE\\Scenery\\Dem4km,
Scenery\\BASE\\Scenery\\lclookup, Scenery\\BASE\\Scenery\\Seasons and
Scenery\\BASE\\Scenery\\worldlc at200f28e8..200f2954. First two pass24h;
the remaining four pass1. Failures select errors11h,9,10,12,11,13 respectively;
the first reports then continues, the remaining five return0. Mask meanings are
not yet established and must not be described as standard Win32 access modes.

## Work packages and stopping rules

1. API-05a: initial flow/phase inventory complete within the stated call-opaque
   bounds. No large Ghidra function creation was needed.
2. API-05b, next:20132c8e (49-byte stdcall2 wrapper, export75) and2013341b
   (202-byte attribute-query dependency, export68). Inspect target signatures and
   raw returns independently: both currently store undefined(void) signatures;
   the2013341b decompiler's void return contradicts20132c8e's EAX status test.
   Trace20133d17 path resolution and2014507f fallback only far enough to model
   their interfaces. Attempt natural C++ wrapper first with unresolved real
   dependencies; at most3 targeted source/compiler trials, stop on match,
   identical output or regression. Then attempt dependency source separately.
3. API-05c: memory/free-space gates and20133e19/201f7538 target provenance.
4. API-05d: thread/window creation and2012205d true flow/return contract.
5. API-05e: COM/string cleanup and detached EH ranges; align with API-06 teardown.
6. API-05f: ready/cancel/timer callback phases and code/data integration. Whole
   initialization comparison only after child contracts and gateway policy exist.

No new native flag matrix, layout/import polishing, arbitrary byte masks, patches
or replacement execution. Reuse separate API worker paths and existing COFF
comparison infrastructure. Prior410 unique complete object-code bytes/44 metadata
bytes and123 partial Initialize bytes remain unchanged. No new match percentage.

## Database state and naming suggestions

Read-only session: api.dll current as user selected, fsx.exe also open; all tool
calls explicitly target API. No tab switches, function creation, renames, type
changes, comment changes, memory patches or save required. Latest checked API
revision1793724055614:636d9e2e:314, unsaved_changes=false. Existing factory GZF
remains the last database backup; this inventory is not a database backup.

Unapplied suggestions: FUN_201e7347 -> EnterRegionBoundary and FUN_201e6c4d ->
LeaveRegionBoundary, both provisional, based on paired count increment/decrement,
range processing and shared descriptors. Global2020e010 is already named
g_mainWindowHandle, verified through GetWindowHandle readback; preserve it.
Check other uses before applying helper-name suggestions. No additional
installation input needed.
