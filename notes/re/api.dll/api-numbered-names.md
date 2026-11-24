# API numbered names and private-heap release

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Six instruction listings (694 bytes) were checked against the pinned PE.
Evidence includes its export table, format literals and import identities,
plus read-only live references. Ghidra reported `changed=false` and
modification 3. The source names describe reconstructed roles.

| Entry | Original-instruction verified contract |
|---|---|
| `201322e2` / 206 | Returns null for a null prefix. Formats a signed number with `.%d` from `200f30fc` into a 100-byte temporary. Requests prefix length plus suffix length plus one byte from the private heap with HEAP_ZERO_MEMORY. Copies the prefix with `20137e17` and appends the suffix using the same copy routine and remaining capacity. Returns the allocation to the caller. |
| `2013241b` / 207 | Accepts prefix, signed number, output and capacity. Formats `%s.%d` from `200f3100` and forwards the formatter's EAX status. RET 0x10. |
| `20132441` / 208 | Accepts prefix, two signed numbers, output and capacity. Formats `%s.%d.%d` from `200f3108` and forwards the formatter's EAX status. RET 0x14. |
| `20135f51` / 108 | Takes a pointer slot. If its value is nonnull, calls HeapFree on the private heap and clears the slot independently of the HeapFree result. Returns zero. The slot pointer itself is dereferenced unconditionally. |

The two caller-buffer C exports have void decompiler signatures. The
listings leave EAX intact after the call to `200fb1ab` and therefore
supersede those signatures for the reconstructed return contract. Original
formatter arguments are EDI output, EAX capacity and stack varargs. The
pilot uses an ordinary variadic source interface. Statuses are zero,
0x80070057 for invalid capacity, and 0x8007007a for truncation. The prefix
and formatting argument handling continues through the original CRT API.

The allocated variant uses the existing private-heap allocation policy with
zero initialization enabled and the existing multibyte-aware copy and
append source helpers. Ordinal 108 supplies the verified private-heap
release interface. Live xrefs show no direct call from the allocation
export to the release export. The caller owns the lifetime. The release
helper is wired into the two previously reconstructed paths that actually
call `20135f51`: decoder success at `201330a8` and CAB image read failure
at `20144448`. Numbered-section counting now uses the same source formatter
as the name exports, consistent with its original `200fb1ab` call.

Suggested Ghidra names, unapplied:

- `201322e2` `param_1` → `prefix`, from the length calculation and first
  copy, and `local_6c` → `formattedSuffix`, from its format and append uses.
- `20135f51` `param_1` → `allocationSlot`, from the dereference and
  explicit null store.

## Build and acceptance

`api-numbered-names-20261109-001` returned exit code zero. Object SHA256 is
`04D4B538CBE594C0E231DD1CF85981D3F237808D456462C69D9EB2023AE05530`.
The source build passed all 17 existing gates. The added release comparator
validates ordinal 108, all 43 instruction bytes, the heap global at
`2020e050` and kernel32!HeapFree at IAT `200e19a0`. A modified slot-clear
instruction and an incorrect import each reject. The retained object
passes all 18 gates.

The ledger grows to 5069 complete code bytes and 288 metadata bytes, with
123 partial Initialize bytes tracked separately. The allocated helper emits
211/313 native/original bytes. The two buffer wrappers emit 38/38 and
42/42 but differ outside relocation fields. The original wrappers' custom
formatter argument storage is still distinct from the pilot source
interface. No additional size trials, linking or runtime test were done.
The next nearby independent package is the dialog registry and message
dispatch around `2013246b` and `20132546`.

## Ordinal 207 register-ABI probe (2026-11-15)

Live verified in `api.dll` at `2013241b`: the buffer is passed in EDI
and the capacity in EAX to `200fb1ab`. The wrapper pushes the number,
prefix and `%s.%d` pointer, removes 12 bytes after the call and returns
with RET 0x10. In the previous COFF build the C formatter receives
both additional arguments on the stack and the wrapper removes 20 bytes.
The documented mismatch remains valid for the regular C++ source.

Run from the project root:

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-numbered-name-abi-20261115-001 -FactoryObject -SourcePass -NumberedNameAbiProbe
node scripts/compare-api-numbered-name-abi.cjs vm-share/modules/api/runs/api-numbered-name-abi-20261115-001
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-numbered-name-abi-20261115-001
```

The retained run contains the source snapshot, source hashes, VC8 build
command, build log and worker receipt (exit 0). Native x86 COFF SHA256
is `f030b269bdc9128fa4049e4d98d70686f15c88a861fd3c15937eb54c93d4c3a4`.
The comparator verifies ordinal 207, the full 38 bytes after two
relocation mappings, and the format bytes at `200f3100`. It rejects
changed stack cleanup and a relocation to the C formatter. All twenty
existing source-pass gates pass.

The reconstructed probe symbol `FormatNumberedNameAbiProbe` is opt-in.
The relative call maps to an unresolved symbol for the register ABI at
`200fb1ab`. Keep the callable C++ implementation in the regular build
until a compatible formatter entry is implemented and verified. The
probe is diagnostic coverage. Linked and runtime behavior remain open.
The dashboard retains both listings. Ghidra was read-only and its save
state was unverified.

## Formatter source provenance (2026-11-15)

Live verified at `api.dll:200fb1ab`: the 92-byte formatter checks the
capacity against zero and 0x7fffffff, calls `msvcr80.dll!_vsnprintf` via
`200e1160`, and implements the StrSafe termination and HRESULT branches.
The pinned original PE body agrees with `fsx.exe:01001590` except for
the four-byte operand at offset 0x34. Both PE import tables identify that
operand as `msvcr80.dll!_vsnprintf`.

The archived EXE experiment in `vm-share/modules/fsx/archive/08-four-matches` was
rechecked with `scripts/compare-formatter-pilot.ps1`. It reproduces the
same 92-byte body with one validated IAT mapping from the VC8 Platform SDK
`StringCchPrintfA`, `_STRSAFE_USE_SECURE_CRT=0` and /GL plus /LTCG.
This also reproduces the EDI/EAX calling convention from ordinary C++
source. See [EXE startup notes](../fsx.exe/startup.md#formatter-matching-milestone-2026-11-01-experiment-8).

Hypothesis for the original API source: a call to the SDK StrSafe ANSI
formatter, emitted with compiler-selected parameter registers. The
original header revision and exact Cch versus Cb spelling remain open.
Test the SDK call in a linked VC8 API probe for ordinals 207 and 208,
using the existing /GL- build as a control. The assembly probe answers
only the wrapper encoding question. Ghidra was read-only, with save state
unverified.

## Linked SDK source match (2026-11-15)

The linked C++ result supersedes the open formatter boundary in the
assembly probe and the original C++ wrapper mismatch as the best
available match. Both regular wrappers now call the VC8 Platform SDK
`StringCchPrintfA` in `numbered_name_exports.h`. The same source is used
in the DLL experiment, with _STRSAFE_USE_SECURE_CRT=0 and /GL plus /LTCG.

Run `api-numbered-name-sdk-ltcg-20261115-002` passed the worker receipt
and compiled source hash checks. Linked PE SHA256 is
`1693a26a984dee0e2b68819f4231ae4a2e2651f034393d93c68a6cab0eb9a581`.
Replay:

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-numbered-name-sdk-ltcg-20261115-003 -NumberedNameSdkProbe -WholeProgram
node scripts/compare-api-numbered-name-sdk.cjs vm-share/modules/api/runs/api-numbered-name-sdk-ltcg-20261115-003
```

| Original entry | Rebuilt entry | Linked bytes | Verified address targets |
|---|---|---:|---|
| `api.dll:2013241b` (ordinal 207) | `200e1060` | 38 | Format literal and shared formatter |
| `api.dll:20132441` (ordinal 208) | `200e1090` | 42 | Format literal and shared formatter |
| `api.dll:200fb1ab` | `200e1000` | 92 | `msvcr80.dll!_vsnprintf` IAT |

Verified against the original PE and live Ghidra listings. All 172
linked code bytes agree after the five target-validated address mappings.
The /GL- linked control `api-numbered-name-sdk-control-20261115-001`
pushes the buffer and capacity on the stack and uses 0x14 and 0x18
stack cleanup. The ordinary COFF source-pass run
`api-numbered-name-sdk-regression-20261115-001` passes all twenty gates.
The SDK emits C4996 deprecation warnings for the requested legacy CRT path.
The first LTCG run (suffix 001) linked successfully but the host could
not verify the .def snapshot because the worker collects only .cpp and
.h compiled inputs. The accepted run uses C++ linker export directives.

The dashboard contains both listings for all three entries. This result
adds scoped linked image evidence. The COFF ledger remains separate.
Whole DLL and runtime verification remain open. Ghidra was read-only
and its save state was unverified. The exact original StrSafe spelling
and header revision remain hypotheses.

## Ordinal 206 automatic inlining (2026-11-15)

Live verified at `api.dll:201322e2`: the original calls the same StrSafe
formatter with `".%d"`, then scans the prefix and suffix lengths. The
allocation code includes the initialization check, two HeapAlloc attempts
and the retry failure path. Both subsequent copies call `20137e17`.
The copy helper's PE imports are _getmbcp, _ismbblead and lstrcpynA.
Its C++ probe implementation models these branches and remains provisional.

VC8 /GL and /LTCG automatically inline both
`AllocatePrivateHeapMemoryPilot` and `AppendResolvedPathPilot`. The
/GL- control retains both calls and emits 173 bytes. The first LTCG
run, `api-allocated-name-ltcg-20261115-001`, emits 315 bytes and also
changes the CopyResolvedPathPilot parameter convention despite noinline.
Run `api-allocated-name-ltcg-20261115-002` compiles the copy boundary
with /GL- and restores the stack argument calls. The resulting wrapper
is 314 bytes versus 313 in the original. The 60-byte range
`20132312..2013234d` matches after validating references to the suffix
literal and StrSafe body. The first difference after this range is the
heap initialization check. Instruction selection and register allocation
also differ around the copy calls. The helper inlining is verified for
the probe. The original source-level boundaries remain a hypothesis.

The worker receipts, compiled source hashes and build logs are retained.
The final linked PE SHA256 is
`195714a3f216e342abbe94494397d8749722e9d0fb493c7c01f4c32331d8d757`.
The control PE SHA256 is
`fc290d875f36ceed06b599891c9ecf45e2881f3fad27e2b4f710e34a4cd73777`.
Ordinals 207 and 208 and the StrSafe body retain their full linked gates.
The probe leaves the regular 206 source in place. The dashboard retains
both listings and marks 206 diagnostic. No runtime verification.
Ghidra was read-only with save state unverified.

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-allocated-name-ltcg-20261115-003 -NumberedNameSdkProbe -AllocatedNameSdkProbe -WholeProgram
node scripts/compare-api-numbered-name-sdk.cjs vm-share/modules/api/runs/api-allocated-name-ltcg-20261115-003 vm-share/modules/api/runs/api-allocated-name-control-20261115-001
```

## Ordinal 206 regular source integration (2026-11-15)

Live verified in `api.dll:20137e17`: the zero code-page path delegates to
`lstrcpynA`. In the multibyte path capacity zero leaves the destination
untouched, capacity one writes only NUL, and each iteration reserves a
terminator. A lead byte at the truncation boundary is cleared. If the
lead byte is followed by NUL and room remains, the routine stores the
trail NUL, clears the lead byte and writes a further terminator. The
initial probe omitted the further store in this case and is superseded.
The regular `resolved_path_copy.cpp` implements these paths. The live
original and rebuilt listings were checked statically.

Live verified at `api.dll:201322e2`, `20135dd9` and `2013623e`:
the integrated C++ source preserves the observed heap initialization
check, two HeapAlloc attempts with flag 8, and the order of the failure
calls. The repeated failure resolves the resource instance when needed,
loads resource 8, displays the message and calls TerminateProcess with
exit code 1. In the rebuilt wrapper, a memory CMP replaces the original
load and register TEST in the initialization check. Heap handle loads
and the copy-tail scheduling still differ.

`api-allocated-name-integrated-20261115-001` builds from the regular
`numbered_names.cpp`, which now uses StrSafe for the suffix, and links
the regular copy routine with its stack ABI via /GL-. The resulting
DLL SHA256 is
`dd53cc1fc0b2d7c97074ffa13a91022af33b598ee7674ae8a4a0f716070a4232`.
The worker receipt, compiled source hashes and build log are retained.
The 206 body remains 314/313 bytes and retains the 60-byte match. Both
copy calls resolve to the same linked function. The copy body is
157/167 bytes and its three imports were checked in both PEs.
The three existing linked gates and all 20 COFF gates pass.
The COFF regression run is
`api-allocated-name-integrated-regression-20261115-001`.
Both dashboard entries are diagnostic. Runtime testing remains open.
Ghidra was read-only and save state unverified.

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-allocated-name-integrated-20261115-002 -NumberedNameSdkProbe -AllocatedNameSdkProbe -WholeProgram
node scripts/compare-api-numbered-name-sdk.cjs vm-share/modules/api/runs/api-allocated-name-integrated-20261115-002 vm-share/modules/api/runs/api-allocated-name-control-20261115-001
```

## Ordinal 206 call provenance (2026-11-15)

Live verified in `api.dll:201322e2`. Addresses below are original VAs.
The [listing and import snapshot](../../../artifacts/api.dll/api-ordinal-206-call-provenance-20261115.json)
retains the call sites, helper bodies and PE import identities.

| Call sites | Target and provenance |
|---|---|
| `20132307`, `20132410` | `20148b71`, established `__security_check_cookie`. VC8 /GS emits the call on both exits. The retained linked helper at `200e13ed` has the same 15-byte instruction shape. The only raw differences lie in the cookie address and failure jump displacement. The failure bodies were not compared. |
| `20132324` | `200fb1ab`, StrSafe ANSI formatter. The 92-byte linked SDK body and register calling convention were reproduced. Exact original Cch/Cb spelling remains a hypothesis. |
| `2013235e` | `2013623e`, reconstructed private heap initializer. Creates an 8 MiB heap, requests LFH for private, process and CRT heaps through HeapSetInformation, and updates module state. |
| `20132374`, `20132385` | EBX loaded from `200e19a8`, kernel32!HeapAlloc. Both sites pass flag 8. |
| `20132397`, `201323aa`, `201323bb`, `201323c3`, `201323ca` | PE imports GetModuleHandleA, LoadStringA, MessageBoxA, GetCurrentProcess and TerminateProcess in the failure path. |
| `201323d6`, `20132402` | `20137e17`, reconstructed bounded multibyte copy. CRT _getmbcp selects the path. The zero-code-page branch forwards to kernel32!lstrcpynA. The other branch uses CRT _ismbblead and explicitly copies and terminates bytes. |

The copy helper's authorship remains open. Its pointer/three-stack-argument
contract and code-page branches distinguish it from a StrSafe
StringCchCopyA call. There is no _mbsnbcpy import in this PE. A static
library origin is possible, but no candidate body has been identified.
The inline scans at `20132332`, `20132345` and `201323e9` are reproduced
by the current C++ strlen expressions with VC8.

The strongest inlining hypothesis is the private heap allocation at
`20135dd9`, given its matching retry policy, shared globals and error
imports. Automatic inlining was verified for the reconstruction. The
original source-level call remains a hypothesis. No new build or Ghidra
edit. Ghidra save state unverified.

## Static VC8 CRT copy candidates (2026-11-15)

Live verified at `api.dll:20137e17`: the original body extends through
`20137ebf`, including all three bytes of the final RET 0x0c at
`20137ebd`. The previous 167-byte result is superseded. It omitted the
final two bytes. The corrected size is 169 bytes. The integrated
rebuild is still 157 bytes and remains diagnostic.

Run `api-multibyte-crt-20261115-002` links an isolated x86 DLL with
VC8 14.00.50727.762, /MT and libcmt.lib. The worker receipt passed and
the map identifies each actual library member. The PE has no entry
point. The first run failed to link because the selected crt0.obj
required main. A harness-only main resolved the link dependency.
The source snapshot, build command, logs and receipts are retained
under the respective VM run directories.

| Candidate | Linked entry | Linked worker | Finding |
|---|---|---|---|
| `_mbsnbcpy` from mbsnbcpy.obj | `200e117e`, 23 bytes | `_mbsnbcpy_l` at `200e109d`, 225 bytes | cdecl entry forwards to a locale worker. |
| `_mbsncpy` from mbsncpy.obj | `200e1269`, 23 bytes | `_mbsncpy_l` at `200e1195`, 212 bytes | cdecl entry forwards to a locale worker. |

Both workers use the locale lead-byte table, retain invalid-parameter
handling, call strncpy in the single-byte path and use memset for
padding. The original is a three-stack-argument callee-cleanup helper
with _getmbcp and _ismbblead imports, a direct lstrcpynA path and bounded
terminator stores. The measured VC8 library members are ruled out as
direct body matches. Other library versions or a separate wrapper
remain hypotheses. The regular copy implementation stays in place.

Evidence: [static CRT comparison](../../../artifacts/api.dll/api-copy-static-crt-comparison-20261115.json)
and [corrected integrated comparison](../../../artifacts/api.dll/api-numbered-name-sdk-integrated-comparison-20261115.json).
The three existing linked matches and twenty COFF regressions passed.
Ghidra was read-only, with save state unverified. The probe was not run.

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-multibyte-crt-20261115-003 -MultibyteCrtProbe
node scripts/compare-api-multibyte-crt.cjs vm-share/modules/api/runs/api-multibyte-crt-20261115-003 artifacts/api.dll/api-copy-static-crt-comparison-replay.json
```

The original api.dll and fsx.exe Rich records contain build 50727.
The SP1 control also records 50727, so the Rich records alone do not
distinguish VC8 RTM .42 from SP1 .762. See the
[Rich record snapshot](../../../artifacts/shared/fsx-api-rich-records-20261115.json).
The same static CRT experiment can be dispatched from the VC8 RTM guest
with `-MultibyteCrtProbe -Vc8Rtm`. The controller checks the guest
compiler and DUMPBIN revisions.

### RTM replay (2026-11-15)

Run `api-multibyte-crt-rtm-20261115-001` completed with cl
14.00.50727.42 and DUMPBIN 8.00.50727.42. The worker receipt reports
exit 0. The linked no-entry x86 DLL SHA256 is
`5eaeb254e04a0f7c0a28702a9fede8324f3c15fda34611fbd1819228a4851d3c`.
The retained MAP identifies the two `libcmt` members.

The 23-byte entry bodies are raw equal to SP1. The 225-byte and
212-byte locale workers each differ at seven raw byte offsets, all
within the relative CALL operands for __errno, __invalid_parameter,
_strncpy and _memset. The corresponding instructions resolve to
the same symbol names in both listings. These two RTM candidates
retain the structural differences from `api.dll:20137e17`.
Their provenance as the original copy helper remains unsupported.
The three linked matches and twenty regular COFF regressions pass
on replay. No runtime test or Ghidra edit. Save state unverified.

[RTM comparison and SP1 byte differences](../../../artifacts/api.dll/api-copy-static-crt-rtm-comparison-20261115.json).

### Lead-byte result type experiment (2026-11-15)

Live verified at `api.dll:20137e59..20137e68`: the original calls
_ismbblead, restores ESP, tests EAX, copies the first byte and branches
on the test flags. The original helper spans 169 bytes.

The RTM control request `api-copy-lead-rtm-control-20261115-001`
was rejected by the host version gate because the worker reported cl
14.00.50727.762. The retained output is SP1 evidence. An RTM result
for this source experiment remains open.

The SP1 variant `api-copy-lead-sp1-int-20261115-001` changes only the
staged _ismbblead result from bool to int with a direct nonzero test.
Relative to the 157-byte SP1 control, it removes SETNE and tests EAX
directly. The linked helper at `200e1350` is 152 bytes. It copies the
byte and updates the pointers and count before TEST, unlike the
original ordering. The single-byte path still tail-jumps to lstrcpynA.
The exact-match hypothesis for this isolated change is superseded on
SP1. The regular copy source retains the bool form for now.

The linked DLL SHA256 is
`055e6b29abfd770241282e1800bdc8cdea8762a1967216d7f89672cfc062e92`.
The three linked matches remain exact after their address mappings.
The regular COFF replay passes all twenty gates. Runtime is open.
Ghidra was read-only, with save state unverified.
[SP1 variant comparison](../../../artifacts/api.dll/api-copy-lead-int-sp1-comparison-20261115.json).

To repeat on RTM, run the same build once with and once without
`-CopyLeadIntProbe`, using separate run IDs and
`-Vc8Rtm -NumberedNameSdkProbe -AllocatedNameSdkProbe -WholeProgram`.

### Shared return experiment on SP1 (2026-11-15)

Live verified at `api.dll:20137e36`: the single-byte branch calls
kernel32!lstrcpynA through the IAT and returns with RET 0x0c at
`20137e3e`. The source hypothesis was that assigning the import result
to the destination variable and returning through a common source exit
would prevent the tail jump.

Run `api-copy-shared-return-sp1-20261115-002` kept the direct integer
test and changed only the source-level return structure. VC8 SP1 emits
the same 152 bytes as the prior integer variant, including the tail
jump at `200e1360`. The shared return hypothesis is superseded for this
source and compiler profile. The integer variant remains a useful
local result because it removes SETNE. The regular copy source still
uses the existing baseline. The source experiment is scoped to SP1.

The retained source and build command passed the worker checks.
The DLL SHA256 is
`1d0e2b3375e29b7353c9d092a2d2985a06c490e879dbb482699680eb7d16608e`.
The three linked matches and twenty COFF regressions pass.
No Ghidra edits, with save state unverified. Runtime remains open.
[Shared return comparison](../../../artifacts/api.dll/api-copy-shared-return-sp1-comparison-20261115.json).

Replay by adding `-CopySharedReturnProbe` to the SP1 allocated-name
build with `-CopyLeadIntProbe` and a distinct run ID.

### Split pointer advance experiment on SP1 (2026-11-15)

Live verified at `api.dll:20137e59..20137e6d`: the original tests EAX,
copies the first byte, branches on the lead-byte test and advances the
pointers in the respective blocks. In the staged C++ variant, the
first store has no post-increment and each branch advances the two
pointers separately. The count and terminator behavior is retained.

Run `api-copy-split-advance-sp1-20261115-001` completed with SP1 and
linked DLL SHA256
`ab4d01e03be8efb33b6f9a6b0b48a94c06a38c5c31958aedd1898a1ac8db4640`.
VC8 hoists the common increments back before the branch. The result is
152 bytes, with only two bytes differing from the prior integer
variant. The two ModR/M bytes reflect moving the EBX count decrement
before the EDI/ESI increments. The instruction-order hypothesis is
superseded for this source variant and the regular copy stays as built
previously. The three linked and twenty COFF gates pass on replay.
Runtime was not tested. Ghidra was read-only, with save state unverified.

[Split advance comparison](../../../artifacts/api.dll/api-copy-split-advance-sp1-comparison-20261115.json).
Replay by adding `-CopySplitAdvanceProbe` to the SP1 allocated-name
build with `-CopyLeadIntProbe` and a distinct run ID.

### Export context for the copy helper (2026-11-15)

Live verified with Ghidra and the original API PE export table:
`20137e17` is the unnamed ordinal 157, with 66 direct call sites
reported by Ghidra. The adjacent string exports form a functional group:

| Ordinal | Original VA | Observed behavior |
|---|---|---|
| 157 | `20137e17` | Bounded copy with a code-page check and multibyte loop |
| 218 | `20137ec0` | Append through ordinal 157 at call site `20137ef2` |
| 158 | `20137efe` | Allocate a copy on the private heap and call kernel32!lstrcpyA at `20137fa1` |
| 159 | `20137fb0` | Return zero for a null pointer, otherwise scan string length |

The exports support the hypothesis of a string utility group exposed by
the module. They do not establish original authorship of the copy
algorithm. The observed append and length bodies already have complete
COFF matches with ordinary C++ and the established SP1 profile. See
[initialization flow](api-initialization-flow.md). The tested static CRT
or StrSafe candidates remain unsupported as direct matches.

The recent linked probes use `/GL-` for the copy translation unit to
preserve its stack ABI. They do not export the copy itself as ordinal
157. The next bounded control is to export the copy and append routines
as 157 and 218 and compare the public 157 entry under /GL- and /GL
plus /LTCG with identical C++ bodies. Check the stack ABI at the public
entry separately from direct internal call sites. This tests compiler
context before more source rewrites. Exact original build flags are
still a hypothesis. No build or Ghidra edit in this review. Save state
unverified.

[Export table, xrefs and nearby listings](../../../artifacts/api.dll/api-string-export-context-20261115.json).

### Public export context control on SP1 (2026-11-15)

Built regular `resolved_path_copy.cpp` and `path_append.cpp` as separate
translation units and linked both as unnamed public exports. The
controller verified the SP1 compiler, worker receipts and compiled
source hashes. The source inputs are identical across the two runs.

| Run | Profile | DLL SHA256 |
|---|---|---|
| `api-string-exports-sp1-control-20261115-001` | /GL- | `44ac5b3b05899edeae6abd3b80ff8891ae7ae1c9b2a126b00d7f321ffa4251f0` |
| `api-string-exports-sp1-ltcg-20261115-001` | /GL plus /LTCG | `1f3dd4858b23f37db7fdf62bc301431ed89e4122b77e16c6ddf3397a441a1f4e` |

Live verified original entries `api.dll:20137e17` and `20137ec0`
remain the comparison anchors. Both linked variants resolve ordinal
157 to `200e1000` and ordinal 218 to `200e10a0`. Their complete
157-byte copy bodies are raw equal. The public entry retains the
three-argument stack ABI and the single-byte path retains the import
tail jump. Both 62-byte append bodies match the original after mapping
the REL32 field at offset 51. Their internal call resolves to the
public ordinal 157 entry. This supersedes the hypothesis that simply
exporting the current bodies and enabling LTCG would change the copy
instructions in this profile.

[Linked export comparison](../../../artifacts/api.dll/api-string-exports-sp1-comparison-20261115.json).
The current C++ bodies were used without edits. Existing three linked
matches and twenty COFF gates pass on replay. The dashboard has both
listing pairs. Ghidra was read-only, save state unverified. Runtime
verification remains open.

Replay with `-StringExportsProbe` and unique run IDs. Add
`-WholeProgram` for the second run. Use
`scripts/compare-api-string-exports.ps1` with both run directories
and an evidence path under `artifacts/api.dll/`.

### SP1 copy source and optimization sweep (2026-11-15)

Live verified in `api.dll:20137e17`: the original decrements the
remaining byte count before _ismbblead. It tests the result and
branches after the first store but before the pointer updates. The
single-byte path calls lstrcpynA and returns with RET 0x0c. The live
original is 169 bytes. The public SP1 /O2 control is 157 bytes.

| Run suffix under `api-copy-sweep-*` | Scope | Result |
|---|---|---|
| `size-20261115-001` | Regular copy source with /O1 | 115 bytes. Tail jump and SETNE remain. |
| `noglobal-20261115-001` | Regular copy source with /O2 /Og- | 262 bytes. Normal import CALL, but the pointer and loop state move to the stack and a shared epilogue replaces the original return layout. VC8 warns D9035 for /Og-. |
| `earlyexit-20261115-001` | /O2 with count charged before the CRT call and explicit exits for DBCS termination | 157 bytes. The count decrement moves before _ismbblead, and TEST, first store and the branch precede pointer updates. Source/destination registers are reversed relative to the original. The lead path uses offset loads and increments by two. The import tail jump remains. |

The early-exit source structure improves the measured local ordering
that the split-increment probe failed to reproduce. The full 169-byte
body and exact source shape remain unresolved. Regular source remains
at the baseline. All variants retain the complete 62-byte ordinal 218
match with a validated direct call to public ordinal 157. Worker
receipts, actual compiler flags, snapshots and hashes are retained.
The three previous linked gates and twenty COFF gates pass on replay.
Ghidra was read-only with save state unverified. Runtime remains open.

See the [sweep report](../../../artifacts/api.dll/api-copy-sp1-sweep-20261115.json)
for build hashes and all rebuilt listings and the
[live original listing](../../../artifacts/api.dll/api-copy-sweep-original-listing-20261115.json)
for `20137e17`. Replay the variants with `-StringExportsProbe` and
`-StringCopyVariant Size`, `NoGlobal` or `EarlyExit`, each with a
distinct run ID. Pass the three directories as `-VariantRuns` to
`scripts/compare-api-string-exports.ps1` with the existing two controls.

### Trail-byte pointer advance on SP1 (2026-11-15)

Live verified in `api.dll:20137e68..20137e83`: the lead-byte branch
advances the pointers, checks remaining capacity, copies the trail
byte at the current addresses, decrements the count and advances the
pointers again. The `TrailAdvance` probe expresses that sequence
explicitly, retaining the previous termination branches.

Run `api-copy-trail-advance-sp1-20261115-001` completed with VC8 SP1
and linked DLL SHA256
`8510a5c69649a7212d456a6b981e63775e974ad3d03a9b596a5bbe31eb43815a`.
The 152-byte result replaces the early-exit probe's offset access
and adds of two with direct trail-byte loads/stores and increments.
However, VC8 hoists the first pair of increments ahead of TEST and
the lead-byte branch. The prior improvement in that ordering is lost.
The count decrement before _ismbblead survives. The source and
destination registers remain exchanged, and the lstrcpynA tail jump
remains. This is a local tradeoff, with the 157-byte early-exit probe
retained as the better branch-order reference.

Ordinal 218 still matches its complete 62-byte original body after
validating the direct call to public ordinal 157. The three prior
linked matches and twenty COFF gates pass on replay. The regular copy
source remains unchanged. Ghidra was read-only, with save state
unverified. Runtime verification remains open.

[Trail-advance and early-exit comparison](../../../artifacts/api.dll/api-copy-trail-advance-sp1-comparison-20261115.json).
The original listing is retained in the sweep listing export. Replay
with `-StringExportsProbe -StringCopyVariant TrailAdvance`.

### Separate exits and return barrier on SP1 (2026-11-15)

Live verified in `api.dll:20137e17`. The new probes separate the NUL
exit of the single-byte loop path before its pointer increments and
test a compiler-only barrier after the lstrcpynA import call.

| Run suffix under `api-copy-shape-*` | Linked bytes | Result |
|---|---:|---|
| `separatesingleexit-20261115-001` | 172 | Lead-byte branch precedes the pointer updates, and the trail byte is copied through the advanced pointers. lstrcpynA still tail-jumps. |
| `returnbarrier-20261115-001` | 173 | _ReadWriteBarrier after lstrcpynA produces an ordinary IAT CALL and RET 0x0c with /O2. |
| `combinedshape-20261115-001` | 188 | Both results coexist. The public copy entry is `200e1000`. |

In the combined build, the eleven instructions at
`200e1058..200e1072` have the same encodings as the original
`20137e6a..20137e84` after exchanging EDI and ESI in the pointer
operands, including identical relative branch displacements. This is
a 27-byte local structural correspondence, not raw equality or an
address-normalized match. The prologue, earlier byte load, register
assignment and NUL exit still differ. The barrier is a diagnostic
control and does not establish the original source construct.

The combined DLL SHA256 is
`2b63614a0d9e31eb7f2459d8d410b0c4b13773d20c25fb94acf7e85b388242e0`.
All three worker receipts pass. The build commands, hashes, source
snapshots and listings are retained. Each variant keeps the complete
62-byte linked 218 match with a call to public 157. The three existing
linked matches and all twenty COFF gates pass on replay. The regular
source remains at the earlier implementation. The dashboard shows
the combined probe as a diagnostic with the live original listing.
Ghidra was read-only, save state unverified. Runtime remains open.

[Three-way comparison](../../../artifacts/api.dll/api-copy-shape-sp1-comparison-20261115.json).
Replay using `-StringExportsProbe` and `-StringCopyVariant` with
`SeparateSingleExit`, `ReturnBarrier` or `CombinedShape`.

### NUL exit sharing on SP1 (2026-11-15)

Live verified in `api.dll:20137e17` (`CopyResolvedPathPilot` is
the reconstructed name). NUL in the trail-byte path flows from
`20137e85` to the final zero store at `20137e89`. NUL in the
single-byte loop path branches from `20137e9d` to the same exit.
The two trials start from the 188-byte combined probe and preserve
its compiler barrier and capacity-edge stores.

| Run | Copy bytes | Result |
|---|---:|---|
| `api-copy-nul-sharednul-20261115-001` | 189 | Keeps the first pointer updates after the lead-byte branch. SP1 duplicates the NUL exit despite the common C++ continuation. |
| `api-copy-nul-staggerednul-20261115-001` | 171 | The single-byte NUL case at rebuilt `200e1085` branches to the same final zero store as the trail-byte NUL case. SP1 moves the destination increment ahead of the lead-byte test. |

Both preserve the ordinary lstrcpynA CALL. The 171-byte result
improves the shared exit and loses part of the verified lead-branch
ordering. Neither is a complete 169-byte original-body match.
Keep the combined probe as the lead-branch comparison reference and
retain both variants for the NUL exit. The regular source is unchanged.

Both worker receipts pass and retained source hashes and DLL hashes
are in the [comparison](../../../artifacts/api.dll/api-copy-nul-exits-sp1-comparison-20261115.json).
Both link the 62-byte append body at original `20137ec0` after one
validated call mapping. The other three linked gates and twenty COFF
gates pass on replay. Runtime remains open. Ghidra was read-only, with
save state unverified. Replay with `-StringExportsProbe` and
`-StringCopyVariant SharedNul` or `StaggeredNul`.

### Destination register before the code-page check (2026-11-15)

The scan found a close second implementation at
`simpropace.dll:00440740`. See the
[cross-module evidence](../shared/api-simpropace-string-copy.md)
for exact scope and the different register calling convention.

| SP1 variant under `api-copy-prologue-*` | Bytes | Verified result |
|---|---:|---|
| `sharedcursor-20261115-001` | 175 | Shared cursor holds the destination before _getmbcp, but VC8 uses two nonvolatile registers. |
| `parametercursor-20261115-001` | 174 | Walking cursor is the incoming parameter. VC8 keeps the original destination in the incoming stack slot and emits one nonvolatile register save before _getmbcp. |
| `branchstores-20261115-001` | 193 | The simpropace-inspired branch-local first stores stay separate. The API original stores once before the branch, so this is a control only. |
| `parametercombined-20261115-001` | 191 | Using the parameter as cursor with the previous combined loop preserves the closer lead branch and improves the public entry path. |

Live verified original `api.dll:20137e17..20137e40` and rebuilt
`200e1000..200e1029` agree across the 42-byte prologue and single-byte
path after exchanging ESI/EDI for the destination operand and
mapping the two validated import operands. That is a local structural
comparison, not normalized exact coverage. The previous 27-byte
lead branch correspondence survives. The full function differs in
register assignment, capacity check and NUL paths.

All four worker receipts pass and the [probe report](../../../artifacts/api.dll/api-copy-prologue-sp1-comparison-20261115.json)
contains the hashes and rebuilt listings. The selected linked DLL
SHA256 is
`21ac6d33f256b23dfde31a7717284da71c57c989ebf7b248c874ee5a4dda75f9`.
All four retain the linked append body at original `20137ec0`.
The existing linked and COFF comparisons pass on replay. The regular
C++ source stays unchanged. API and simpropace Ghidra were read-only
and reported saved states. Runtime remains open. Replay with
`-StringExportsProbe` and `-StringCopyVariant SharedCursor`,
`ParameterCursor`, `BranchStores` or `ParameterCombined`.

### Shared zero-capacity return on SP1 (2026-11-15)

Live verified at `api.dll:20137e47`: the zero-capacity path jumps
over the NUL store at `20137e89` to the common return at
`20137e8c`. The `ParameterCombined` control had its own return
block for zero capacity at rebuilt `200e1032`.

The `SharedZeroReturn` variant guards the copy and final NUL store
with a nonzero capacity test and shares the pointer return.
`api-copy-shared-zero-sp1-20261115-001` completed with VC8 SP1.
The 188-byte copy now branches at `200e1031` to `200e1079`,
skipping the store at `200e1076` and sharing the return at
`200e1080`. The function entry through the first RET is raw equal
to the previous 42-byte rebuilt path. The 27-byte lead-byte block
is also raw equal to the previous rebuilt block at its new address.
Both retain their prior structural comparisons to the original.

The linked DLL SHA256 is
`fda34e25615a706669e02c605137970ee28bdf95fa5a6e807447f9e4de454f5a`.
The complete original body is 169 bytes and remains different.
Register assignment and the single-byte NUL path within the
multibyte loop are still open. This result is an experiment.
The 62-byte append helper at original `20137ec0` and the prior
three linked and twenty COFF checks pass. The worker receipt,
source, compiled-source hashes and compiler log are retained.
Both dashboard listings are present. Ghidra reported saved,
modification 3. No Ghidra edits or runtime tests.

[SP1 comparison](../../../artifacts/api.dll/api-copy-shared-zero-sp1-comparison-20261115.json).
Replay with `-StringExportsProbe -StringCopyVariant SharedZeroReturn`.

### Combined capacity and NUL exits on SP1 (2026-11-15)

Live verified in `api.dll:20137e17` (reconstructed
`CopyResolvedPathPilot`). All three probes below retain the shared
zero-capacity return and put the destination in EDI and source in ESI
as in the original. The complete 42-byte entry and single-byte path
through RET 0x0c now match after mapping the verified _getmbcp and
lstrcpynA import operands at offsets 14 and 33. The register exchange
required by the previous 188-byte comparison is superseded for this
range. No complete-body match or additional ledger coverage.

| SP1 run suffix under `api-copy-exits-*` | Bytes | Result |
|---|---:|---|
| `sharedzeronul-20261115-001` | 189 | Preserves the lead branch order and direct trail copy, but the compiler still duplicates the NUL exit. |
| `staggeredzeronul-20261115-001` | 171 | Shares the final NUL store, but moves the destination increment ahead of the lead-byte test. |
| `delayedleadadvance-20261115-001` | 175 | Shares both exits and retains the lead-byte branch before the pointer updates. The trail path tests remaining capacity before the updates and reads through `[ESI+1]`, unlike the original. |

The 175-byte variant is the preferred structural comparison.
At rebuilt `200e1031` the zero-capacity branch targets `200e107a`,
past the NUL store at `200e1077`. The single-byte NUL branch at
`200e108c` returns to that same store used by the trail-byte NUL branch.
The original spans 169 bytes. Capacity checks, trail loads and some
epilogue instructions still differ. The regular source remains unchanged.

All three worker receipts pass with VC8 SP1. The source snapshots,
build flags and hashes are retained. The selected DLL SHA256 is
`5c0e4edfedca99d337c0718db4525fb412a8195ad282b38585a4410f7b85bae9`.
The full 62-byte append helper at original `20137ec0` passes in each
variant with the validated call to `20137e17`. The three other linked
and twenty COFF gates pass on replay. Both dashboard listings are
present. Ghidra reported saved, modification 3. No edits or runtime tests.

[SP1 comparison](../../../artifacts/api.dll/api-copy-exits-sp1-comparison-20261115.json).
Replay with `-StringExportsProbe` and `-StringCopyVariant`
`SharedZeroNul`, `StaggeredZeroNul` or `DelayedLeadAdvance`.

### Trail pointer timing and capacity parameter on SP1 (2026-11-15)

Live verified in `api.dll:20137e17` (reconstructed
`CopyResolvedPathPilot`), the original starts the multibyte count
with SUB EBX,1 at `20137e49`. The 175-byte delayed-advance control
uses ADD EBX,-1 at rebuilt `200e1033`.

| Run suffix under `api-copy-trail-*` | Bytes | Result |
|---|---:|---|
| `sourcebeforecapacity-20261115-001` | 171 | Direct trail load through ESI, but the first source advance moves before the lead-byte test. |
| `destinationbeforecapacity-20261115-001` | 172 | The first destination advance moves before the lead-byte test; the trail load still uses ESI+1. |
| `capacityparameter-20261115-001` | 175 | Consumes the capacity parameter directly. Emits SUB EBX,1 with original encoding 83 eb 01 at `200e1033`. |

Only offsets 52 and 53 differ between the capacity-parameter variant
and the delayed-advance control. The change restores the original
instruction encoding while retaining the 42-byte entry match and
both shared exits. Pointer timing and the capacity-branch layout
still differ from the original. The regular copy source is unchanged.

All three worker receipts pass in VC8 SP1. Snapshots, build flags and
hashes are retained. The selected DLL SHA256 is
`e3820500f812ee94e7364259912e51bea5609f2988dcbe5f99d1e7e11d907c08`.
The complete linked 62-byte append body at `20137ec0` and the
three earlier linked and twenty COFF gates pass. Ghidra reported
saved, modification 3. No Ghidra edits or runtime tests.

[SP1 comparison](../../../artifacts/api.dll/api-copy-trail-capacity-sp1-comparison-20261115.json).
Replay with `-StringExportsProbe -StringCopyVariant` and one of
`SourceBeforeCapacity`, `DestinationBeforeCapacity`, `CapacityParameter`.

### Complete linked match for the allocated numbered name (2026-11-15)

The previous 314/313 diagnostic for `api.dll:201322e2` is superseded
by a complete 313-byte linked match after twenty verified address
mappings. The single flag-read change in the shared allocator
also matches the 178-byte string duplicate at `20137efe`. See
[heap string duplicate](api-heap-string-duplicate.md) for the run,
control and scope. The full copy body at `20137e17` stays outside
this acceptance. Its direct call targets were verified separately.
