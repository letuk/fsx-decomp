# CAB prefix image loading and stream wrappers

Follow-up source: api-cab-stream-producers.md covers producer/constructor/
destructor implementations and verified AddRef ownership. Their bodies remain
unmatched; pool/buffer dependencies and native EH are open. Current4522 unchanged.

2026-11-08. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Eight exported ASM listings (1568 bytes) reverify against the pinned original:
artifacts/api.dll/api-cab-image-evidence-20261108.json. Live signature/state checks
explicitly targeted api.dll; revision1793741866691:2a9d629f:3 is saved with
unsaved_changes=false. No DB edits or tab changes. Reconstructed names/types
below are not original symbol or complete interface declarations.

## Image reader2014420b

Verified stdcall one path argument, RET4,700 original bytes. Resolve20133d17
with selector0x34, open201246c4 with read/1/NULL/3/0x80/NULL, then optionally
replace stream with20127440(-1,0,0,0x1000,stream,0), releasing original on success.
The returned wrapper is consumed from EAX despite its stored void declaration.

Direct virtual read at slot0xc loads36 header bytes. M/S/C/F required. Flags
at0x1e must be0 or4; flag4 reads four reserve bytes, requires both extra bytes
zero, then skips the WORD byte count relative to current stream position. Entry
count at0x1c must be nonzero; another8-byte record supplies allocation/read length
from its first DWORD. The reader allocates that length, retries once then uses
the established fatal error path. It seeks to0 ignoring seek status and reads
that same length, freeing/resetting the image on read failure. This is a CAB
PREFIX image, not proof of reading/decompressing the entire archive. This
corrects an earlier conversational whole-file description.

Entry pointer is image+DWORD(image+0x10); every name at entry+0x10 is uppercased,
its length truncated to WORD at entry+0xe, then stride0x11+WORD length. No extra
image/entry bounds validation is present in this observed routine or introduced
in source. Last stream reference is released on ordinary exit. No exception
cleanup is inferred from these normal paths. Open/wrapper implementations remain
external source dependencies.

## Observed stream slots and helpers

cab_stream.h describes only observed slots: release8, read0xc, seek0x14,
stat0x30, explicit stack receiver and callee stack cleanup. Stat buffer72 uses
modified/created/accessed pairs at0x10/0x18/0x20. No full interface recovery or
original type identity is claimed.

- 201333ac release33: NULL returns-1; otherwise ignores virtual release return
  and returns0. Full raw-equal object match on first build.
- 201333cd read39: default-1, optional virtual read with receiver/output/length/
  read-count. First early-return source43 differs; one assembly-supported source
  revision initializes default result before NULL test, yielding full39 match.
  Stop at success after2 source trials; no further read variants.
- 20133592 seek57: zero high offset and NULL new-position output; source43
  differs, original alignment/register choices retained as unresolved.
- 20135378 stat/time156: slot0x30 with flag1; copy optional outputs only on
  success, preserve outputs on failure; source143 differs.

## Owner reload201449c4: explicitly deferred

330-byte routine takes owner in ESI and path in ECX, no stack args. It frees
old image, copies path, calls image reader, resolves again, opens/wraps a metadata
stream, obtains time pair via20135378, releases stream and publishes entry count/
pointer when image exists and count nonzero. It does not clear prior count/
pointer on image-load failure.

Critical unresolved failure path:20144a30 branches to20144a87 on resolution
failure.20144a87 reads [EBP-114h], but the only local assignment to that slot
occurs after successful open at20144a50 (or wrapper replacement20144a7f).
There is no preceding initialization in the function. Stat failure also leaves
the local timestamp pair unwritten before it is copied to owner. Do not replace
these with invented NULL/zero values or certify an uninitialized C++ build.
LoadCabOwnerSourceBoundaryPilot therefore stays unresolved; no full owner reload
source/match is claimed. Investigate failure reachability/context before source
translation of that branch. Existing source constructor still ignores load status.

## Build, matches and boundaries

Initial run api-cab-image-20261108-001 (object
8FFDA8654CA4BFC09CB619FE3831A5B9555AFC250CE1D8BE1FAFACDE7F8F0D04):
image669/700, read43/39; release33 exact.
Final run api-cab-stream-read-20261108-001 (object
8B8242DC8FC42E93712BAB7273610AC5E035BF3517FB4B226E7D49E570AB02F5):
read39 exact, image671/700 after changed helper inlining. All runs under
vm-share/modules/api/runs. Image shape question stopped at1; second build changes
only the separately evidenced read-helper question, not image tuning.

compare-api-cab-stream-wrappers.cjs compares full33+39 bytes without relocations
or masking, asserts no overlap with accepted generated bodies, and rejects NULL
branch, VTable-slot and RET-size mutations for both bodies. Eleven regression
comparators pass, no uninitialized/missing-return warnings. Disjoint complete
code total increases4450+72=4522; metadata44 and Initialize partial123 separate.
No whole-DLL, indirect callee implementation or runtime claim. New source is
structured/goto-free. No register forcing, padding or flag matrix.

## Suggestions and next package

Suggestions only, no Ghidra renames:2014420b param_1 -> cabPath; local_14c ->
activeStream; pvVar4 -> cabPrefixImage (allocation/read/entry-walk evidence).
201449c4 local_118 -> metadataStreamSlot, with explicit potentially-uninitialized
failure-path warning; local_124/local_120 -> modifiedTimeLow/High from stat offsets.

Next inspect the stream producers201246c4/20127440 and constructor20126be0:
receiver/return conventions, ownership when wrapping succeeds/fails, and whether
owner reload's path-failure branch has a supported reachability constraint.
Keep the known owner undefined-state branch and cache EH blocker separate from
accepted wrapper bodies; no blind font or image byte tuning.
