# InitInstance recompilation investigation

## 2026-11-19 — Complete linked method at 0x010019e0

**Live verified.** All 769 bytes and 246 instructions now match at the
original VA, including the four operands that target `pfnGetApplication`
at `0x01004384`. This supersedes the previous four-byte residual for
the packaged comparison. The regular RTM worker also reproduced all
769 bytes using the same C++ method.
[Packaged comparison](../../../artifacts/fsx.exe/getter-placement-20261119/initinstance-body.json).
[Regular controller comparison](../../../artifacts/fsx.exe/getter-placement-20261119/standard-initinstance.json).

## 2026-11-18 — D3DX check at 0x01001a44

Live verified in `fsx.exe`. This is a block within the method at
`0x010019e0`. The scoped sequence through `0x01001a86` loads
`D3DX9_34.DLL` via the `LoadLibraryA` pointer populated at `0x01001a0b`,
resolves `D3DXCheckVersion` and calls it with arguments 32 and 34.
A missing library, missing export or false result reaches the error
path at `0x01001a6b` with error ID `0x10`. Success releases the module.
The reconstructed source boundary `HasRequiredD3DXVersion` is inlined.

A fresh XP VC8 RTM build reproduced the existing source match. The 67-byte
scope maps to `0x01001a24..0x01001a66` and has zero differences after
validating the three string operands and two IAT operands. The separate
upstream `LoadLibraryA` IAT load and all branch displacements are checked
in the full 769-byte comparison. The controller regressions passed.
Runtime execution remains open.
[Scoped comparison](../../../artifacts/fsx.exe/initinstance/d3dx-01001a44-20261118/match.json).

Added and read back a Ghidra PRE comment at `0x01001a44`. Saved revision
`1795012585183:2514814d:51`.

### D3DX export lookup at 0x01001a51

Live verified in `fsx.exe` at `0x01001a51..0x01001a6a`. This block receives
the D3DX module handle in ESI, resolves `D3DXCheckVersion` and calls it
with `32, 34`. A missing export or a false return goes to the error
path at `0x01001a6b`. The inlined C++ helper is
`FSXAppPilot::HasRequiredD3DXVersion`.

Rechecked the retained VC8 RTM build against the pinned original. The
26 bytes at rebuilt `0x01001a31` match after validating the export-name
literal and the `GetProcAddress` IAT operand. The full method comparator
passed again. This scope lies inside the preceding 67-byte comparison.
The Ghidra PRE comment was read back after saving revision
`1795013130661:2514814d:53`.
[Scoped comparison](../../../artifacts/fsx.exe/initinstance/d3dx-01001a51-20261118/match.json).

## 2026-11-17 — complete linked method

**Live verified.** The complete `fsx.exe` method at `0x010019e0..0x01001ce0`
matches after address normalization. The SP1 build places
`FSXAppPilot::InitInstance` at `0x010019c0..0x01001cc0`. Both bodies contain
769 bytes and 246 instructions. All 15 internal branches have identical
encodings and displacements. The comparison checks 13 relative call operands
and 31 absolute address operands. They account for all 33 unequal raw bytes
in the SP1 build. The second build with the VC8 RTM tools has 36 unequal raw
bytes and also has zero remaining differences. This supersedes the November 3
777-byte diagnostic as the current result for this method. The baseline 41
ledger continues to describe its archived image.
[SP1 certificate](../../../artifacts/fsx.exe/initinstance/structured-20261117/match.json)
and [RTM certificate](../../../vm-share/modules/fsx/runs/initinstance-full-RTM-20261117/candidates/initinstance-full-RTM-20261117-001/initinstance-body.json).

The C++ source uses the virtual `CWinApp` override and a private static D3DX
version check. The compiler inlines the check. The D3DX module is released
only after a successful version check. The manifest success block ends in
the application initialization result, leaving the manifest error at the end
of the method. `_MAX_DRIVE` and `_MAX_DIR` storage in the enclosing scope
recovers the original `0x554` stack reservation. Each `MAX_PATH` buffer
is initialized at its observed point of use. A zero-initialized `ACTCTXA`
followed by assignments to `cbSize` and `lpSource` recovers the original store
order. The class name, helper boundary and local names are reconstructions.
[Source](../../../src/fsx/startup_context_pilot.cpp).

Two actual XP builds isolate the last instruction at `0x01001c2a`. Testing
the OLE initializer against `FALSE` and returning a literal `FALSE` emits
`85 C0` (`TEST EAX,EAX`). Storing the returned `BOOL`, testing it and
returning that value on the failure path emits the original `3B C3`
(`CMP EAX,EBX`). The strict comparator reports exactly offsets `0x24a` and
`0x24b` for the former. Earlier guarded returns and a per-function size
optimization trial produced distinct layouts. The table of all 22 linked
trials retains source and build-command hashes and distinguishes layouts
where the positional diagnostic does not apply. Three additional staging
attempts in Windows PowerShell stopped at the manifest count before dispatch.
[OLE control](../../../artifacts/fsx.exe/initinstance/structured-20261117/ole-control.json)
and [trial ledger](../../../artifacts/fsx.exe/initinstance/structured-20261117/trials.json).

The two canonical builds used VC8 x86 `14.00.50727.762` and `14.00.50727.42`,
`/O2 /Oy- /hotpatch /GS /MD /Zi /GL /EHsc /GR-`, and `/LTCG /OPT:ICF`.
Both used the VM's installed headers and the profile's pinned RTM CRT and MFC
support libraries. D3DX is dynamically resolved and the original passes
`32, 34` to the version check. Neither the original SDK installation nor the
original compiler service pack can be distinguished from this result.
[SP1 command and receipt](../../../vm-share/modules/fsx/runs/initinstance-full-InstalledSP1-20261117/candidates/initinstance-full-InstalledSP1-20261117-001/).

The comparator verifies the Ghidra listing bytes against the pinned original
PE and checks the instruction offsets against the linked listing. It resolves
references against the rebuilt MAP, verifies IAT identities and import thunks,
and checks pointer targets for the exact string contents including the empty
literal. The SP1 and RTM workers returned x86 PE executable images and passed
the controller regression checks. The separate callback `ShowErrorMessage`
retains its two known differences. All 80 SP1 thunk targets and trailing
padding checks passed. The writable section still has the documented
16-byte size difference. The verification covers linked machine code.
Runtime execution and a separate native COFF comparison remain untested.
[Comparator](../../../scripts/compare-initinstance-body.ps1),
[thunks](../../../artifacts/fsx.exe/initinstance/structured-20261117/thunks.json)
and [writable data](../../../artifacts/fsx.exe/initinstance/structured-20261117/writable-data.json).

Ghidra MCP explicitly targeted `fsx.exe` with `api.dll` active. The original
listing and memory read cover the complete method. Final readback reports
saved revision `1793671788023:3045b49a:3`, `unsaved_changes=false`. This pass
used read-only access. The existing startup show-mode field interpretation is
documented below. [Original listing](../../../artifacts/fsx.exe/fsx-inventory/functions/010019e0.asm).

## Earlier investigations

2026-11-02. Original: fsx.exe SHA256
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735,
FSX_InitInstance at 0x010019e0..0x01001ce0: 769 bytes / 246 instructions.
Status: live verified assembly and offline rebuilt PE comparison; no exact match.

## Evidence and diagnostic boundary

Read-only Ghidra inspection found only fsx.exe open/current. Saved revision
1793559605675:8eb5e96a:275, no unsaved changes. No navigation, edits or save.
Fresh instruction/decompilation snapshots: artifacts/fsx.exe/initinstance/original-*.json.
Rebuilt function resolved from ?InitInstance@FSXAppPilot@@UAEHXZ in its MAP.
The fresh worker smoke baseline remained 758 bytes / 229 instructions and reserved
0x564 stack bytes. This is independently verified for the post-OLE reconstruction,
not merely copied from the older experiment20 result.

scripts/compare-initinstance.ps1 emits JSON and a side-by-side HTML instruction
alignment. It uses LCS with spelling normalization and abstracted address/branch
targets; registers and stack offsets remain meaningful. Large numeric immediates and width spellings are
abstracted too. It does NOT validate reference target identities or certify
equivalence, and aligned-token counts are NOT matching percentages. Rows show
the original instruction strings so abstractions can be inspected.

## Source corrections and experiments

- Original 0x01001a87 onwards initializes directory/path only after successful
  D3DX validation; manifest at 0x01001b27 and API path at 0x01001bae are initialized
  later. Previous source eagerly initialized all buffers before OS checks.
  Moved initialization to the observed phases; retained zeroing of all260 bytes.
- Direct error calls with branch-specific arguments remove reconstructed
  errorId/detail/errorModule spills. Common source-level failure locals were
  scaffolding, not recovered original variables. Changing this recovered much
  of the initial instruction sequence; compiler block placement still differs.
- _splitpath_s receives drive capacity3 and directory capacity256 at
  0x01001af1/0x01001ae4. Reconstructed arrays changed from4/260 to3/256. The
  resulting frame became0x554, matching original SUB ESP at0x010019e8. Exact
  original declarations are still a hypothesis; an inferred Ghidra array may
  include padding. Four path buffers remain260 bytes.
- Explicit ACTCTXA field initialization avoids zeroing cbSize/lpSource only to
  overwrite them; resulting code remains under review, not a complete match.
- Reordering exePath/manifest declarations recovered their original stack
  positions in the reviewed accesses; source declaration order is not proven.

### Startup argument correction: previous interpretation superseded

Original0x01001c50 loads saved this,0x01001c54 reads its+0xa4 event handle,
and0x01001c5a reads DWORD[this+0x4c] for the second explicit Initialize argument.
The previous reconstruction emitted m_lpCmdLine at+0x48. Captured installed
MFC header sdk-afxwin.h declares m_hInstance, m_lpCmdLine, m_nCmdShow in order;
real compiler experiments006/010 show m_nCmdShow accesses+0x4c, while the event
remains+0xa4. Canonical source now passes m_nCmdShow through ULONG_PTR and uses
a generic startupArgument word at the reconstructed API boundary.

The earlier claim that this argument was the command-line pointer is superseded
at this caller. Strong SDK-backed hypothesis: startup show mode. The actual
API.DLL target signature still needs independent verification. Suggested Ghidra
repair: current decompiled pFsxApp->m_lpCmdLine at this call -> m_nCmdShow / a
provisional startupShowMode interpretation, AFTER reviewing the stored FSXApp/
CWinApp layout. Suggestion only; no Ghidra field, name or signature edited.
Do not infer a complete original class layout from the minimum reconstruction.

## Results

artifacts/fsx.exe/initinstance/experiments.csv summarizes the runs using one diagnostic
implementation. Ten InitInstance job attempts: nine compiled successfully,
one source-generation error failed (run002). Local D3DX failure-label and common
return variants008/009 grew to857 bytes and were rejected as the baseline choice.
Selected proposal06-buffer-order is reproduced as final run010 and copied into
canonical src/fsx/startup_context_pilot.cpp and application_interface_pilot.h.

Final run: vm-share/modules/fsx/runs/initinstance-20261102-010, diagnostic_complete.
Rebuilt VA0x00401340,777 bytes /245 instructions, stack frame0x554. Eight bytes
longer is a LENGTH difference, NOT a count of mismatching bytes. Remaining
differences include D3DX failure-block placement, register allocation/scheduling,
and ACTCTXA stores. No instruction-aligned exact reference mapping exists yet.
Do not choose a proposal solely by length or LCS count.

Final EXE SHA256:
E28BC0581CE8E912D6F0A3CE00F580293D41D54F8432CFF29E80C26FA944A4FD.
All established version/copy/formatter/OLE/destructor/pending/vtable/EH checks
passed. ShowErrorMessage remains437 bytes with only0xdb/0xdc differing after16
validated reference fields. No candidate executable was executed as FSX.

## Controller repairs and limitations

- Added reviewed source-directory input and InitInstance one-proposal diagnostic
  mode to the existing host/XP queue controller. It does not reuse the24-form
  ShowErrorMessage search as an InitInstance generator.
- Fixed case-insensitive PowerShell Target/target variable collision discovered
  in run001: real compilation succeeded but controller reported baseline_failed.
  Its output was subsequently diagnosed offline. Early run003..008 status files
  contain the ShowErrorMessage regression's two differences in their generic
  Differences field; that is NOT an InitInstance byte score. Current controller
  keeps InitInstance Differences null and records ShowErrorRegressionDifferences.
- Run002 compilation failed but the old batch returned receipt ExitCode0;
  missing-output validation still stopped it safely. Guest batch now exits the
  dedicated cmd process with explicit codes. Replayed the actual invalid source
  as worker-failure-20261102-001: genuine compile failure, receipt ExitCode3,
  host build_failed, no further dispatch. No stale EXE accepted.
- Controller lock is disposed on terminal paths; destructor now explicitly
  participates in the regression acceptance condition.

## Next checks

Build a target-aware reference map for InitInstance before claiming any exact
bytes. Investigate D3DX branch/tail-merging without additional pointer/array
guesses. Independently inspect API.DLL Initialize's second explicit argument,
with user-controlled program navigation, before repairing Ghidra layouts.
Worker stays ready; no search is left running and no InitInstance match claimed.


## Follow-up builds and reference inventory (2026-11-02)

After the user opened api.dll, independently checked the vtable and Initialize
ABI; details in fsx-api-interface.md. The DLL's stored commandLine parameter is not
independent confirmation of the old pointer interpretation. No Ghidra changes.

Three additional real SP1 builds, all existing regressions passed:
- run011 / proposal09-d3dx-boolean: explicit boolean result,857 bytes; rejected.
- run012 / proposal10-context-source-order: lpSource assigned before zero fields,
  777 bytes, no improvement.
- run013 / proposal11-d3dx-flat-labels: explicit flat failure/success labels,
  777 bytes. Entire InitInstance DUMPBIN listing equals selected run010's listing;
  this tested source-label change did not alter generated instructions.

Canonical src remains the tested run010 reconstruction. No exact match found;
frame remains0x554, body777 versus original769, ShowErrorMessage stilltwo bytes.

Added scripts/inventory-initinstance-references.ps1 and ran it on selected010:
19 original known caller targets correspond to rebuilt MAP/DUMPBIN symbols,
with equal per-function reference counts for every entry. Includes MFC thunks,
version/OLE/string/security helpers, kernel/CRT imports and application getter.
Output initinstance-references.json beside the selected comparison. This is an
inventory, NOT positional operand validation, import/callee-body identity proof
or a byte-match certificate. Register-indirect calls and strings/data remain
outside its scope. The instruction diagnostic still abstracts targets.

Remaining exact-match blockers: D3DX and manifest failure-block placement,
register scheduling, ACTCTXA store sequence. Next work should combine reference
identity with block-aware alignment, rather than deriving a score from raw LCS
or repeatedly trying equivalent source labels. Worker ready, no run active.

## Block-aware diagnostics and ACTCTXA follow-up (2026-11-02)

Added scripts/compare-initinstance-blocks.ps1. It extracts block leaders from
entry/direct branches/branch fallthroughs/returns, checks all branch destinations
and verifies instruction coverage. Calls remain inside blocks; external/callee
control flow is not modeled. Reference and unique error-ID anchors seed pairs;
one-successor edges and equal/inverse JZ/JNZ propagate the pairing. Conflicts
are reported rather than silently overwritten. JSON retains instructions and
edges; HTML provides both block lists, anchor links and all propagated pairs.

Selected run010 and the newer aggregate runs each have21 blocks on each side;
all21 can be paired without anchor conflicts and all successor sets agree.
This is a structural diagnostic, NOT proof that branch predicates, instructions,
callee implementations or runtime behavior are equivalent. It is not a matching
percentage. A shifted failure block is therefore distinguished from a missing
startup branch without inventing an exact-match result.

Block comparison isolated ACTCTXA construction: original uses XOR EAX,EAX and
seven DWORD zero stores. Previous canonical explicit member assignments used
EBX/BX and separate WORD stores for architecture/language. Tested two ordinary
C++ formulations at the observed lifetime point:
- run014/proposal12: ACTCTXA context = { sizeof(ACTCTXA) }, then lpSource;
- run015/proposal13: ACTCTXA context = { 0 }, then cbSize and lpSource.
Both build successfully, retain all established regressions, and produce246
instructions with777 bytes/frame0x554. Both now use the XOR/DWORD zeroing form,
but store scheduling and register allocation still differ from original. Exact
original aggregate syntax is not established. Run014's conventional aggregate
form is now canonical, superseding run010 as the selected source snapshot.

Final canonical integration build016: diagnostic_complete, linked EXE SHA256
F82A443E3F7782137D8452ADE839C38688BF6B277D211B94059D492547DCF8A4.
Original769 bytes/246 instructions; rebuilt777 bytes/246 instructions. Both
frames0x554;21 paired blocks, no successor-set mismatch or pairing conflict.
All19 known reference inventory entries have matching occurrence counts.
All helper/OLE/destructor/pending/28-vtable/EH regression gates pass;
ShowErrorMessage still differs at0xdb/0xdc after its16 verified mappings.

Controller now automatically emits reference inventory and block reports after
each reviewed InitInstance build; integrated path validated with fresh run016.
No Ghidra inspection/navigation/edit/save in this follow-up; prior saved-state
readbacks remain the most recent database evidence. Worker ready, no run active.

Next: per-paired-block operand/encoding comparison with verified target mapping,
especially conditional-branch widths/inversions, ACTCTXA store scheduling and
D3DX-vs-manifest failure-tail layout. Equal instruction counts do not mean only
eight byte positions differ. Current generic LCS remains diagnostic only.

## Class-membership audit and encoded differences (2026-11-02)

Question investigated: could the InitInstance reconstruction belong to the wrong
class? Fresh read-only fsx.exe evidence, with api.dll still the current tab:
- Original vtable slot0x010013f0 (=0x01001398+0x58) contains0x010019e0.
- At0x01001660, constructor-like initializer passes global0x010043e0 asECX
  into thunk0x0100202c, then writes vtable0x01001398 into that same global.
  The thunk bytes FF25 7C110001 point to IAT0x0100117c; its80000236 value
  identifies ordinal566, listed by Ghidra as CWinApp's constructor.
- Selected rebuilt real CWinApp-derived class vtable0x00403404+0x58 contains
  InitInstance0x00401340 (read from actual linked PE, not inferred from names).

These independent associations strongly support the current CWinApp-derived
InitInstance override model. FSXApp/FSXAppPilot remain reconstructed class names,
not identified original names or complete layout proofs. A wrong base/layout/
virtual declaration could affect matching, whereas a Ghidra namespace label
cannot itself change compiled instructions. Source names can influence linkage,
RTTI and ordering; that is separate from the verified current slot association.
No class/type/name changes were made in Ghidra.

Encoded-size accounting now emitted by compare-initinstance-blocks.ps1 and its
HTML/CSV, based on disassembler instruction spans (validated1..15 bytes and
expected direct-branch lengths). All21 paired blocks have equal total NON-branch
encoded length. All eight extra bytes are attributable to branch encoding/layout:

| Original block | Rebuilt block | Branch length delta |
|---|---|---:|
|01001a44|004013a4|+4|
|01001a51|004013b5|+4|
|01001a61|004013c9|+4|
|01001a6b|0040161d|-5|
|01001b27|0040147e|-4|
|01001cb5|004014fa|+5|
|Total||+8|

Three D3DX conditional jumps grew2->6 bytes, because the error block is now far
away. The manifest conditional shrank6->2. One5-byte error-tail jump disappears
at D3DX and appears at manifest. Non-branch instructions contribute zero LENGTH
delta per paired block, but their bytes and scheduling do not all match.

Added compare-initinstance-operands.ps1. Within each CFG-paired block it compares
non-branch instructions by ordinal, reading their actual PE bytes. Known absolute
fields/relative call displacements are normalized only when the rebuilt target
matches the mapped symbol. Known string operands require exact null-terminated
byte content on both sides; cookie/window/global fields use explicit symbol maps.
No arbitrary immediate masking, instruction patching or source assembly used.
Branches excluded, instruction reordering is left visible, and external callee
body/import identity requires its separately recorded evidence.

Selected run016:231 non-branch instruction positions,188 equal after target
normalization,43 different positions,44 validated reference fields and zero
reported unresolved known-reference rows. This is NOT a function match percentage
or count of differing bytes. Remaining rows mostly show EAX/ECX/EDX temporary
register rotation, ACTCTXA store/LEA scheduling, the OLE CMP-vs-TEST form, and two
reordered loads before the Application Initialize call. Register-only textual
changes are not automatically certified semantically equivalent.

The new operand diagnostic is wired into future InitInstance runs. Existing
run016 reports were updated offline and all three diagnostic stages exercised;
controller syntax parsed. No new guest build was necessary: canonical code and
compiler settings did not change. New diagnostics do not alter previous helper
acceptance rules. No binary/source/Ghidra changes, patches, navigation or save.

Next evidence-based experiments should target register-allocation context and
failure-block ordering, including original translation-unit/LTCG boundaries. Do
not randomly reassign class ownership or infer eight matching fixes from length.

## Bounded module-instance-local trial (2026-11-03)

Status: live verified original/on-disk code and controlled rebuilt evidence.
The baseline41 ledger found no unchecked original snapshot instruction ranges;
therefore InitInstance was selected instead of reopening layout/IAT work.
Original hash remains61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735;
baseline41 hash6953DEA3584BECDFBE03A0D55511D8B4093A593F421603B97EE3938214E8580B.

One hypothesis, one actual XP worker trial: ordinary local
`HINSTANCE currentInstance = moduleState->m_hCurrentInstanceHandle;` before
the Initialize expression may recover the original module-handle/vtable load
schedule. Prediction recorded before dispatch:43->41 differing nonbranch pairs,
all other diagnostic rows/established gates unchanged. No volatile, assembly,
ABI change, binary patch or flag variation. Original source syntax remains unknown.

Actual job:vm-share/modules/fsx/runs/initinstance-module-local-20261103/candidates/
initinstance-module-local-20261103-001. Candidate SHA256:
71AC546145A95B8E79BF5E972729DFEFDB0D0928089DD5050BC0FCD4EA3EB5E8.
Build command byte-identical to41; only effective startup_context_pilot.cpp
source hash changes, with the exact predicted local/expression delta.

| Code | Original01001C5D / candidate01001C45 | Baseline41 at01001C45 |
|---|---|---|
|Five bytes|8B40088B16|8B168B4008|
|Instruction order|MOV EAX,[EAX+8]; MOV EDX,[ESI]|MOV EDX,[ESI]; MOV EAX,[EAX+8]|

The five-byte range is raw-exact to original; four candidate bytes changed
relative to41. All other section differences are confined to CodeView GUID and
debug timestamp; header differences only COFF timestamp. Section extents,
all other code, imports and writable data are byte-identical to41. Independently
rerun all80 thunk/CC checks and writable-data gates pass. Full controller gates
(application/helper/OLE/CRT/ATL/reference/data/header/order/alias/import) pass.
ShowErrorMessage retains its two differences. InitInstance still777 bytes,
246 instructions,21 paired blocks and0x554 frame;41 nonbranch differences remain.
All other operand-diagnostic rows are identical, including branch-size residuals.

Evidence:artifacts/fsx.exe/initinstance/module-instance-local-20261103/PLAN.md and staged
source; actual candidate request/receipt/build/source hashes/comparison/diagnostics;
artifacts/fsx.exe/initinstance-module-local-acceptance-20261103.json,
initinstance-module-local-thunks-20261103.json and
initinstance-module-local-data-20261103.json. Reproduce the independent
source/whole-section/five-byte audit with scripts/verify-initinstance-module-local.cjs.

Accepted the small source change into src/fsx/startup_context_pilot.cpp. Archive41
and its coverage ledger remain immutable; the new candidate is retained in its
run, with no golden/replay/default-inventory path promotion. The reconstructed
local name currentInstance is an actual source addition, not a Ghidra rename or
claim about the original variable name. No supported new database rename follows.

Stopping rule satisfied after the first successful trial. No further variable
permutations. Remaining code work: ACTCTXA/path-buffer register scheduling,
OLE CMP-versus-TEST and D3DX/manifest failure-block placement. A later question
needs its own concrete evidence/prediction; prior label/boolean experiments
must not be repeated blindly. Layout16/code-gap32/IAT103 remain deferred.
Ghidra fsx.exe still saved at1793671788023:335a223f:3; no edits/save/backup or
tab switch. No candidate execution or VM/toolchain changes. Worker ready.
