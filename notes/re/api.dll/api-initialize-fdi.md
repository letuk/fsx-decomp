# Initialize boundary, callback registry ABI and FDI stream adapters

2026-11-03. Status: live verified original bytes/types/dataflow and actual native
VC8 source builds. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.

## ESI receiver is a real input

Initialize2012048a passes2020bb48 in ESI to200f6451 with four explicit stack
words. The318-byte helper immediately TESTs ESI and dereferences [ESI]. It
allocates0xb0 zero-filled bytes when the owner's pointer is null, initializes
count+4=0, capacity+8=10 and pending-state+c=0, and appends16-byte callback
records after a16-byte header. Records contain callback, flags, context1/context2.
Full allocation/retry/error details are in the live snapshot. Capacity growth
uses20135e98 with size(capacity+11)*16 and then adds10 to capacity. Return is
0/-1 in EAX, RET10; caller preserves its prior ESI around this particular call.

Cross-checks with200f65b1/200f67bd show removal/dispatch behavior: flag1 means
pending removal while dispatching, flag2 selects whether two extra contexts are
passed. This supports a callback-list registration interpretation, not the earlier
vague event-registration-helper label. Concrete original class/name/threading
and complete allocation-owner layout remain hypotheses. The source's minimum
owner class contains only its observed data pointer; it is not a full class size.

The native C++ member boundary uses conventional ECX thiscall. Original uses
ESI, consistent with optimized internal conventions but not proof of original
LTCG settings or method-versus-free-function syntax. No inline assembly, volatile
or forced-register source is introduced to conceal that mismatch.

## Four complete FDI adapter matches

Source:src/api-pilot/fdi_stream_callbacks.cpp, compiled through lifecycle.cpp.
Installed VC8 SDK objidl.h provides IStream method declarations. The observed
VTable slots+c/+10/+8/+14 match Read/Write/Release/Seek ABI. This is an
IStream-ABI-compatible handle model; concrete class and COM IID are not proven.

| Original VA | Reconstructed source operation | Complete raw bytes |
|---|---|---:|
|2014474c|Read(buffer,count,&count), return output count|33|
|2014476d|Write(buffer,count,&count), return output count|33|
|2014478e|Release(), return1|23|
|201447a5|Seek(low distance/high0,origin,&same local), return low result|51|

These140 bytes are raw-equal native COMDAT bodies, with zero relocations; no
address/instruction masking. HRESULTs are ignored in read/write/seek. Seek's
high DWORD is explicitly zero, even for a negative low-word bit pattern; a
sign-extension rewrite would contradict original instructions. Read/write use
the existing count argument slot for output. UINT and ULONG are distinct C++
types in the SDK but both32-bit here; explicit output-pointer casts preserve
the observed slot, without claiming original source syntax.

First actual job api-fdi-stream-object-20261103-001 failed compilation because
UINT* could not bind to ULONG*. Receipt/log/source retained; no output accepted.
The explicit32-bit output-pointer cast was the only source correction in002.
Run:vm-share/modules/api/runs/api-fdi-stream-object-20261103-002.
Object SHA2565154CE6930B83A0E09536F38B9A8153AFF69DDDCBB812CF4015E08B554301586.
All177 prior lifecycle code bytes and28 EH4 metadata bytes retain their gates.
Thus complete-body object scope is317 code bytes, not317 linked DLL bytes.
New negative control changes Read's VTable slot to Write's and is rejected.
Original opcode/addend/wrong-callback controls continue to reject.
Stopped this question after the corrected successful second attempt.

## Initialize source and precise residual

Source:src/api-pilot/initialize_boundary.cpp. Uses actual installed fdi.h macros
for allocator/free/open callback declarations and FDICreate's9-argument contract.
Six startup-state stores, FDI context/error globals, seven callback identities,
SEH invocation and conditional registry call are represented with real unresolved
dependencies. The owner/member declaration is a provisional source model.

Native trial001 failed because a guessed CPU_TYPE cast was not an SDK type;
002 uses the observed literal1 accepted by the actual header. This is a source
preparation correction, not evidence about the original symbolic constant name.
No enum/CPU interpretation was inferred. Run:
vm-share/modules/api/runs/api-initialize-object-20261103-002.
Object SHA25668765BFA5F061A073AA93AD39140DEAF4F519550EDE1DF1B32C104D6F723BC7F.
All317 previous code/28 metadata gates pass in this object.

Original Initialize160 bytes, reconstructed158. All21 relocation fields have
validated original targets/addends, including all FDI callbacks, startup/global
slots and the actual registry helper. Two precise disjoint ranges also match:

- 2012048a..20120497:14 raw bytes, object offset0.
- 20120499..20120505:109 target-validated object bytes, object offset15.

These123 bytes are partial-range evidence, not a complete function certification
or matching percentage. Original offset0e PUSH EDI versus native PUSH ESI remains
different. Original carries result in EDI, locally PUSH/POPs ESI and passes owner
via ESI; native carries result in ESI and uses ECX for the thiscall owner, omitting
the extra preservation pair. The remaining37 original/35 rebuilt bytes are not
certified; this does not mean every byte in them differs. Branch/layout and the
registry-call ABI remain visible and are not normalized away. The entire function
must not be certified from these spans or equal reference counts.
Additional prefix-opcode and reference-addend mutations are rejected by the
Initialize partial-range checker; outside-range/tail bytes remain uncertified.

Native source question stops here: initial prediction was compiled contract plus
register residual, which is now explicit. No third source form or repeated GL
matrix. Actual registry/helper/reallocation source and caller context must be
available before seeking optimized register agreement in a linked build.

Header regression run api-window-initialize-header-20261103-001 retains the full
linked21-byte window getter plus both negative controls. DLL SHA256
BCB8D00F2592BC2EB6A7C4C44F7DD5E9596CC67E3BC860249AF39710EE384E50.

## Database corrections and save

Both programs open, api.dll now current as set by the user; no agent tab switch.
Pre-edit revision1793720305185:636d9e2e:135 saved. Actual backup:
artifacts/api.dll/ghidra-backups/api-before-fdi-register-20261103-1650.gzf.
TypeApiFdiAndRegistry.java checks the five current function bodies/names and ESI
entry before its bounded transaction. Applied:

- FUN_2014474c -> ReadFdiStream; FUN_2014476d -> WriteFdiStream; signatures
  uint cdecl(int streamHandle,void* buffer,uint count).
- FUN_2014478e -> CloseFdiStream, int cdecl(int streamHandle).
- FUN_201447a5 -> SeekFdiStream, long cdecl(int streamHandle,long distance,int origin).
- FUN_200f6451 -> RegisterCallbackInList; int EAX return, owner void** in ESI,
  four stack words at Stack4/8/c/10, purge16. __stdcall is a Ghidra cleanup model
  combined with explicit custom storage, not a conventional five-stack-word ABI.
- Added scope/evidence comments. No function-byte/body expansion, table/layout
  invention, source-register forcing or unverified original-name claims.

Readback shows Initialize now calls RegisterCallbackInList(&DAT_2020bb48,
callback,0,0,0); the formerly hidden ESI owner is explicit. Saved revision
1793721567359:636d9e2e:281, unsaved_changes=false. All new live code ranges were
compared against pinned original bytes after edits and remain identical.

Useful suggestions only: DAT_2020bb48 -> g_initializeCallbackRegistryOwner;
DAT_2020d8dc -> g_fdiContext; DAT_2020d8d0 -> g_fdiErrorState, supported by the
observed helper/FDICreate argument and result roles. Globals were not renamed or
given complete class layouts in this step.

## Evidence and next slice

Live/decompiler snapshots:api-initialize-dependencies-live-20261103.json,
api-initialize-fdi-after-ghidra-20261103.json and
api-initialize-fdi-memory-20261103.json. Native build/receipt/source/log and
lifecycle-comparison.json under each API run. Plans:api-fdi-stream-plan and
api-initialize-native-plan-20261103.md. Dispatcher now snapshots failed jobs and
records build_failed/module ownership before raising; both initial compiler
failures have been retroactively recorded and hash-verified in API jobs storage.

Next: reconstruct callback registry registration with its allocator/reallocator
dependency20135e98, then investigate real caller/callee optimization context.
Allocator/free/open FDI implementations, initialization callback20120869 and
linked four-slot Application/GetApplication integration remain open. No candidate
executed or API whole-module matching baseline promoted.
