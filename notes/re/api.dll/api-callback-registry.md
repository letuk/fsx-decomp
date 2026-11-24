# Callback registry registration and private-heap reallocation

2026-11-15: Live verified `api.dll:20135e98`. A local volatile flag
read in the regular C++ source reproduces the original MOV and TEST.
VC8 SP1 links the complete 185-byte body at `200e14a0`, matching
after fourteen verified address mappings. This supersedes the earlier
unmatched COFF diagnostic for the linked scope. The nullable delegation
to `20135dd9`, mode-4 restoration, NULL retry, and claimed failure
path are retained. Ghidra was read-only and reported saved, mod 3.
Runtime verification remains open.

Run `api-reallocator-sp1-20261115-001`, DLL SHA256
`c82f0998da7c0b0a63f22c1e5f1480123a798b04f8425036976a682725dd4269`.
The reallocator TU uses /GL- to retain the direct allocation call.
See the [comparison](../../../artifacts/api.dll/api-heap-reallocator-sp1-comparison-20261115.json).
Replay with `-NumberedNameSdkProbe -AllocatedNameSdkProbe -WholeProgram
-DuplicateHeapStringProbe -HeapReallocatorProbe` and comparator option
`--require-reallocator-match`. Prior linked bodies retained.

### Linked registration probe (2026-11-15)

Live verified at `api.dll:200f6451`, the registry registration
helper receives its owner in ESI and returns with RET 0x10.
The explicit-owner C++ model remains an ABI diagnostic with five
stack arguments and RET 0x14. The SP1 LTCG DLL
`api-callback-registry-linked-sp1-20261115-001` emits 326 bytes
versus 318 original, superseding the old uninlined 194-byte COFF
body as the linked comparison reference. The staged export is a
probe anchor. The regular function remains internal.

VC8 inlines allocation into registration and both HeapAlloc
attempts appear locally. The complete 67-byte failure block at
`200f64ab` is raw equal to the original allocator's block.
In the rebuilt registry at `200e131f` it matches after eight
target-validated address mappings. The direct growth call at
`200f6526` resolves to `20135e98`; the rebuilt direct call
also resolves to the matched reallocator. The full registry body
remains open, including the owner ABI and record-write scheduling.

The worker receipt passed. The linked DLL SHA256 is
`1af663f7e81a6db6b7704e017833cd6a3da69306980753a750f18ad811c76fbd`.
Replay the linked heap profile with `-HeapRegistryLinkedProbe`.
See the [report](../../../artifacts/api.dll/api-callback-registry-linked-sp1-comparison-20261115.json).
All eight prior linked matches and twenty COFF checks pass.
Both dashboard listings are present. Ghidra was read-only and saved,
modification 3. Runtime remains open.

### Internal registration ABI replay (2026-11-15)

Live verified `api.dll:200f63df` and `200f6403` compute an owner from
the 43-slot table at `2020bb30`, put its address in ESI and pass four
stack arguments to `200f6451`. The internal C++ probe retains these two
caller shapes and uses noinline solely to keep the helper visible.
The probe table is exported solely for address verification. These
choices are not evidence of the original declarations.

With /GL and /LTCG, VC8 chooses ESI and RET 0x10 for the internal helper
without assembly. This supersedes the stack ABI diagnostic above.
The helper is 319/318 bytes and remains diagnostic. The existing
67-byte failure comparison and matched reallocator target survive.
Both complete callers match after validating the owner-table address
and direct call. Their original bodies are 36 and 40 bytes respectively.

The receipt for `api-callback-registry-internal-sp1-20261115-002` passed.
DLL SHA256 is `d23d3442439ba36c213cd561238b6ab1f90faf3bc80a5f353f3b67bb1373b748`.
The source snapshot, build command and hashes are retained. Replay with
`-HeapRegistryInternalProbe` in addition to the linked registry profile.
All prior linked and COFF checks pass. No regular ABI or Ghidra edits.
Ghidra reported saved, modification 3. Runtime remains open.
[Linked comparison](../../../artifacts/api.dll/api-callback-registry-internal-sp1-comparison-20261115.json).

### Registration body trials (2026-11-15)

Live verified at `api.dll:200f6451`. The original loads the freshly
allocated owner pointer once and uses it both for the NULL test and
the count store. The staged Snapshot variant expresses that reuse.
The shared-failure label alone emits 316 bytes, and the combination
emits 314 bytes. Snapshot is the closest verified result at 318 bytes.
After fourteen validated address mappings it differs only at offsets
203, 204, 208 and 210. At original `200f651c` the growth size uses
LEA EAX and the subsequent shift and push use EAX. The rebuilt region
at `200e138b` instead adjusts and passes ECX. The full body is still
diagnostic. Regular source remains unchanged.

The [Snapshot comparison](../../../artifacts/api.dll/api-registry-body-snapshot-sp1-comparison-20261115.json)
supersedes the 319-byte comparison. DLL SHA256 is
`523bf6a39338cf0cd0a6437cf0e0fcee43bd877bad3db2d0ee6d473aa176c845`.
Replay the internal registry profile with `-RegistryBodyProbe Snapshot`.
The other staged profiles are SharedFailure and Both. All linked
caller and heap matches and twenty COFF checks pass. Ghidra was
read-only and saved, modification 3. Runtime remains open.

2026-11-03. Status: live verified original instructions/dataflow and actual native
VC8 source compilation; new helper bodies are not certified matches.
Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.

## Role and bounded scope

These routines are supporting API.DLL infrastructure, with direct application
callers: Initialize registers a callback, shutdown removes it, and the message
loop dispatches another list. Understanding their ABI/storage/failure behavior
is necessary for faithful caller reconstruction. Byte-matching every helper is
not a prerequisite for understanding or moving on to application logic. This
slice establishes real source dependencies and stops without another flag matrix.

## Reallocator: behavior that must be preserved

Original20135e98..20135f50,185 bytes, export ordinal107. Three explicit stack
words, RET0xc, status0/-1 in EAX. If uninitialized, calls InitializeHeapManager.
If *inOutMemory is NULL, delegates to AllocatePrivateHeapMemory with zero-fill0.
Otherwise performs HeapReAlloc(privateHeap,0,oldPointer,size) and immediately
writes the result through the output slot. On failure:

- Mode4 restores the previous pointer and returns-1.
- Other modes retry HeapReAlloc with NULL lpMem, observed PUSH0 at20135efe.
  This is not a retry with the old pointer. Error reporting calls LoadStringA
  with stored instance2020e028 directly, unlike allocation's GetModuleHandleA
  fallback. It displays resource8 and calls TerminateProcess before the final
  output store/zero status path if control returns.

No claim about the intended Windows API behavior or whether these error paths
occur in FSX. Source reallocate_boundary.cpp preserves the observed NULL retry
and fallback difference rather than silently fixing them. Negative instruction
controls reject changing the retry pointer to1 or replacing restoration of the
old pointer with the failed result. Original live/PE bytes corroborate the paths.

## Callback-list source model

Source callback_registry_boundary.h models only the observed16-byte header:
dispatch depth, count, capacity, pending compaction; records are separate16-byte
callback/flags/context1/context2 entries following it in raw heap storage. Static
x86 compile checks enforce both widths. This is not a recovered original class
name, complete class size or array-capacity declaration.

callback_registry.cpp accepts an explicit pointer owner (including NULL), lazily
allocates0xb0 zero-filled bytes, initializes capacity10/count0/pending0, grows via
reallocator to(capacity+11)*16, adds10 capacity and appends the callback record.
Flag2 is conditional on withContext. DWORD arithmetic preserves the observed
unsigned width; no invented bounds/overflow handling. Initialization's existing
minimum owner method forwards to this real source helper. It is an integration
adapter, not another identified original function. A separate pointer-owner
function keeps the original NULL-owner branch without relying on calling a C++
member on a NULL object. Removal/dispatch helpers remain separate dependencies;
their complete source implementations are not claimed here.

Heap allocation remains a real unresolved source dependency in the native object;
no fake helper body or forced ESI source convention is used. Later optimization
may inline it, but the original ESI/four-stack-word shape is not recovered yet.
The free registry helper is kept internal (no artificial dllexport), so future
real caller/callee optimization is not constrained by an invented public ABI.
The original public realloc ABI remains ordinary stdcall/three stack words.

## Actual trials and outcome

One native source build, api-registry-realloc-object-20261103-001, produced
api-registry.obj SHA256
2EBCC222B70EC5381D8EE2A07772515AB052843C4E32E3131CEE1822E4B8F3F3.
The visibility cleanup is retained as002, SHA256
FD3F86C49ADF85511721925C332FD405D3C3F3B38063C5C085416675EB4FD6C2.
All four generated function COMDAT byte ranges are identical between these
source snapshots; only removing the registry's artificial export directive was
intended. No third build, flag permutation or original flag claim.

Native realloc185/original185 retains14 identity-mapped references to globals,
heap APIs and helpers. Critical mode4 restoration and NULL-retry instruction
shapes are checked. Original MOV/TEST versus memory CMP and temporary-register
differences remain. Equal length/reference inventory is not a whole-body match.

Original registration318 versus native explicit-owner body194 plus separate
method adapter31. Original uses ESI/four stack words and inlines the allocator's
error path; native uses an explicit stack owner/five words and an allocator call.
The source-only Entries helper4 is not counted as an identified original helper.
No subtraction of these sizes is a byte-difference score or coverage measure.

Inspection: scripts/inspect-api-registry-object.cjs; registry-diagnostic.json in
each run. It is deliberately diagnostic-only; selected shape checks and reference
inventories do not add certified function/byte coverage. Shared strict native
COFF reading moved to scripts/lib/coff.cjs. Existing lifecycle comparator gates
remain unchanged in scope and pass after this extraction.

Regression build api-registry-dependencies-regression-20261103-001, object SHA256
8DB95188E023625BD040982DFA23BB77FFCCD953C9EC713541C4A87C7C2C6B52,
retains all317 complete object-code bytes,28 EH4 metadata bytes and Initialize's
123 scoped bytes/21 mapped fields. These gates do not certify the new helper
bodies. Existing linked21-byte window evidence stays separate. Plan:
artifacts/api.dll/api-registry-realloc-plan-20261103.md. Native source work stops here.

## Actual database change

Before editing, api.dll current as selected by user; fsx.exe also open. Saved
revision1793721567359:636d9e2e:281. Backup:
artifacts/api.dll/ghidra-backups/api-before-realloc-types-20261103.gzf.
TypeApiHeapReallocator.java validates current generic name,185-byte extent and
RET0xc, then renames FUN_20135e98 -> ReallocatePrivateHeapMemory and types it
int stdcall(int failureMode,uint sizeBytes,void** inOutMemory). Added evidence/
failure-path comment. uint is a neutral x86 size word; original typedef name
is not recovered. Existing registry custom ESI storage remains intact.

Readback/saved revision1793722735273:636d9e2e:312, unsaved_changes=false.
New live code reads match pinned original byte-for-byte. No instruction/body
patch, tab switch or candidate execution. Names/layouts remain reconstructed.
Unapplied local naming suggestions: iVar1 -> allocationStatus (NULL-input
delegation result); pvVar2 -> resizedMemory (first HeapReAlloc result). pvVar3
combines old and retry pointer lifetimes, so do not misleadingly rename it only
oldMemory without separating those roles first.

Evidence: api-registry-realloc-live-20261103.json,
api-registry-realloc-after-ghidra-20261103.json,
api-registry-realloc-memory-20261103.json and retained module-owned worker runs.

## Next area

Return to API-02's GetApplication/export144 and the observed four-slot VTable.
Use the now available outer-method and dependency source, and inspect original
RTTI/VTable prefix evidence before selecting that compilation model. Keep missing
initialization/filter/loop implementations explicit. Do not spend another native
heap/register permutation series trying to force ESI; revisit that residual only
with genuine linked caller/callee context. Main initialization20120869 remains a
later application-logic package. No baseline promotion or full-module claim.
