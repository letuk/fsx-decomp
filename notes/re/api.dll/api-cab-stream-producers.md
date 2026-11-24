# File and wrapping stream producers used by CAB loading

2026-11-09 field correction: read/seek usage establishes +0xc bufferStart,
+0x10 bufferValidBytes, +0x28 position. This supersedes the provisional position
at10/unknown28 naming below. Actual source renames/shared record header applied;
no Ghidra edits. Six methods and three full matches now bring total4735/44.
See api-cab-stream-methods.md for current evidence and build.

2026-11-08. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Evidence: artifacts/api.dll/api-cab-stream-producers-evidence-20261108.json verifies12
exported ASM listings/1624 bytes against original and retains selected VTables
and constructor-unwind bytes. Live state/signatures checked explicitly in api.dll;
revision1793741866691:2a9d629f:3 saved, unsaved_changes=false. No Ghidra edits,
renames or tab switches. All source names/layouts are reconstructed observations.

## File stream201246c4

Seven stack arguments/RET28, pointer in EAX. Calls CreateFileA with all seven
arguments; invalid handle returnsNULL. Access mode is2 if GENERIC_WRITE and
sign bit both set, otherwise boolean GENERIC_WRITE. Object pool20124903 supplies
record; failed allocation returnsNULL WITHOUT closing the already-open handle.
Preserve this observed leak path; do not silently add CloseHandle.

Record observed276: VTable200f2c70 at0, refs1 at4, handle8, access modec, path260
at10. Custom copy200f79d5 receives EAX source/EDX destination/stack capacity260;
NULL path substitutes empty string. Pool allocation/recycling remains unresolved
source, not a new heap substitute. Source assertions validate layout.

VTable slot4 is200f40da (InterlockedIncrement of refs at+4). Slot8 is2012431e:
InterlockedDecrement; when result<1, CloseHandle and return record via20124b11,
then return0. Pool record lifetime and file HANDLE lifetime are distinct.

## Wrapper factory20127440 / constructor20126be0

Factory takes six stack args/RET24; returns constructor EAX despite stored void
signature. Allocates pool record20127a04. Constructor receives ESI receiver,
EAX capacity, EDX underlying stream, four stack args length/flags/data/baseOffset;
returns receiver in EAX/RET16. Do not treat decompiler fastcall text as complete
original interface. Source uses explicit ordinary argument lists, not ABI shims.

Observed48 record: VTable200f2d50 at0, refs4, data8, unknown0c, position10,
capacity14, flags18, underlying1c, baseOffset20, length24, unknown28, buffer
origin indicator2c. Unknown semantic fields remain neutral. Constructor starts
position/capacity from EAX and zeros0c/28. With no data it allocates buffer via
20127642 (size in EAX), sets flag04000000, position0 and indicator1. Zero
capacity becomes length if no underlying stream, otherwise1000h. Existing data
sets indicator0; no invented data-copy step or new allocation-failure check.

NonNULL underlying receives slot4 AddRef. LengthFFFFFFFF triggers Seek(0,0,END,
out64 initialized0); return ignored, only low32 bits minus baseOffset become
length. High32 truncation and modulo32 subtraction are preserved. Flag1 is set.
Without underlying, sentinel length becomes capacity.

This proves ordinary ownership transfer: successful wrapper holds one extra
reference, so CAB caller may release its original reference. NULL pool allocation
does not AddRef and caller retains original. No runtime refcount trace is claimed.

## Destruction and exceptional allocation return

VTable release20126d5b decrements refs and invokes20126d82 on result<1.
Destructor20126d82 (ESI receiver, EAX receiver return) resets VTable, then if
flag04000000 and data nonNULL: indicator0 calls private-free helper20135f51,
indicator nonzero returns buffer through2012772b. It zeros data under that flag,
releases underlying if present, then returns wrapper record through20127c12.
Source implements these paths; it does not free caller data when flag is clear.

Original factory EH FuncInfo201efcac has one unwind entry201efca4 pointing to
201ed6eb. Action loads saved raw record and jumps to20126bd5, which passes EAX
record to20127c12. This is POOL return, not the heap free used by the CAB-owner
constructor. The placement tag/delete in source expresses that distinction.
Fixed pilot profile emits no wrapper unwind handler; source intent is not native
EH equivalence. Known blocker, no profile/flag matrix attempted.

## Owner reload failure remains unresolved

20133d17's inspected source/ASM still has failure exits for suffix selection,
relative combine, capacity and canonicalization. Capacity260 alone does not
prove success. Earlier cache normalization uses selector0; reload uses34h.
No supported invariant excludes reload failure20144a30 ->20144a87. Therefore
the uninitialized stream slot noted in api-cab-image.md remains an open original
edge, not a resolved unreachable path or a reason to insert NULL initialization.

## Build and next work

Source: src/api-pilot/cab_stream_producers.cpp; addReference slot typed in the
source-only observed VTable. Ghidra VTable types/signatures unchanged.
Run vm-share/modules/api/runs/api-cab-stream-producers-20261108-001.
Object SHA25686DAD616F17054FFD3B5A3F8F82CD327E890951BC9D90AB3454330C0875F23C6.

| Routine | Original/native bytes | Status |
|---|---:|---|
|201246c4 file producer|151/157|unmatched, custom copy and scheduling|
|20127440 wrapper producer|102/59|unmatched, native unwind absent|
|20126be0 initialize wrapper|227/215|unmatched, private register ABI|
|20126d82 destroy wrapper|77/94|unmatched, private receiver/free-helper inlining|

One initial source build per question; stop at documented ABI/EH blockers.
Eleven comparison gates pass, including full release/read wrappers. Coverage
stays4522 complete code,44 metadata, Initialize partial123 separate. No padding,
register forcing, goto, flag matrix, final linked DLL or candidate execution.

Suggestions only, no Ghidra renames:201246c4 pvVar1 -> fileHandle, puVar2 ->
fileStreamRecord (CreateFile/pool writes);20127440 iVar1 -> wrapperRecord;
20126be0 unaff_ESI -> wrapperRecord, in_EAX -> initialCapacity, param_2/EDX ->
underlyingStream. Keep other inferred parameter names provisional until custom
storage is represented.20126d82 unaff_ESI -> wrapperRecord.

Next functional source package: methods behind wrapper VTable200f2d50, especially
read/seek/stat and file-stream forwarding. Pool20124903/20127a04 and buffer
cache20127642 remain dependencies to reconstruct later. Keep native ABI/EH
matching work bounded and separate from extending source behavior coverage.
