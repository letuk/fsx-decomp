# Interface between fsx.exe and api.dll

2026-11-03 factory source/object milestone:
[../api.dll/api-factory-matching.md](../api.dll/api-factory-matching.md). Complete72-byte GetApplication
and generated observed four-slot table16 match after validated COFF references;
source virtual-method model and RTTI-prefix evidence are scoped, not original
class-size/name/whole-DLL flag proof. Linked factory/export gate remains open.

Current API reconstruction plan (2026-11-03): [../api.dll/api-matching-roadmap.md](../api.dll/api-matching-roadmap.md).
Fresh initial boundary evidence and unresolved wrapper-return/lifetime questions:
[../api.dll/api-application-boundary.md](../api.dll/api-application-boundary.md). This supplements and
qualifies historical singleton/void-wrapper interpretations below.

Sources S1, S3, L1: [../SOURCES.md](../SOURCES.md).

## Live EXE model

/Application: size 4, alignment 1; offset 0 ApplicationVTable *vftable.
/ApplicationVTable: size 0x10, alignment 1:

| Offset | Field type | Field |
|---|---|---|
| 0x00 | Initialize * | Initialize |
| 0x04 | undefined | open |
| 0x08 | undefined | open |
| 0x0c | GetWindowHandle * | GetWindowHandle |

At fsx.exe 0x010019e0, Initialize receives this plus five explicit arguments: instance handle from pModuleState, the word at pFsxApp+0x4c, 1, ShowErrorMessage, and singleInstanceEvent. The former command-line interpretation is superseded; see the 2026-11-02 verification below.

S1/S3 report five PUSH instructions from 0x01001c64, ECX=object, CALL EDX at 0x01001c70. This supports __thiscall, not the decompiler argument count alone. startupMode is a provisional name for argument 1.

GetWindowHandle(this,0) returns a value used as HWND by CWnd::Attach. /Demangler/AFX_MODULE_STATE currently has size 0x0c and HINSTANCE m_hCurrentInstanceHandle at +0x08. Verify the original MFC80 layout/name against version-matched evidence.

pfnGetApplication is a function pointer, not an object pointer. It remains represented through FARPROC with the returned value cast to Application *.

## DLL findings: historical, not live verified

Addresses belong to the api.dll build investigated in S1/S3. Its hash and current image base were not established during consolidation.

| Address | Reconstructed meaning/name |
|---|---|
| 0x20120442 | GetApplication: global object, created on demand |
| 0x2020de64 | g_application |
| 0x200f2b24 | application_vftable |
| 0x2012048a | Application::Initialize, slot +0x00 |
| 0x2012052a | Slot +0x04, meaning open |
| 0x20122817 | Slot +0x08, meaning open |
| 0x20120535 | GetWindowHandle; intended class-qualified name Application::GetWindowHandle, slot +0x0c |
| 0x2020e010 | g_mainWindowHandle |

S1: GetApplication requests four bytes through AllocatePrivateHeapMemory, writes the VTable pointer, and stores the same pointer in g_application. The EXE type size alone would not prove object size; the reported allocation is additional evidence. FSXApp and Application are different objects. Application has not been established as COM.

Historically reconstructed Initialize types were (HINSTANCE, char *, int, error callback, HANDLE), this in ECX, RET 0x14. The second argument's char* interpretation is not independently established and is superseded as a definite type claim. It saves startup data, calls FDICreate, and executes 0x20120869 through wrapper 0x2013e3c1.

GetWindowHandle returns the value at 0x2020e010 for windowId==0, otherwise NULL. Its HWND interpretation is supported by CWnd::Attach; writes to the global remain an independent check.

S3: wrapper 0x2013e3c1 calls the supplied function pointer immediately (CALL [EBP+8], RET 4). Surrounding SEH is probably compiler scaffolding. The earlier callback-registration hypothesis is superseded.
- Slot +0x00 -> target 0x20120869.
- Slot +0x04 -> target 0x20120ed5.
- Slot +0x08 stores its argument at 0x2020de7c, calls 0x20122ae1 via the wrapper, returns 0.
Original try/catch syntax and exact wrapper return semantics remain unverified.

## Heap management: historical

- 0x2013623e: InitializeHeapManager, reported export ordinal 103.
- HeapCreate(0,0x800000,0): private heap with 8 MiB initial size.
- Attempts LFH mode 2 for private, process, and CRT heaps; these need not be three distinct heaps.
- 0x2020e054: g_heapManagerInitialized.
- 0x2020e058: g_heapManagerInitCallCount; increment observed, reference-count semantics unproven.
- g_privateHeap reportedly renamed; address not established here.
- 0x20135dd9: AllocatePrivateHeapMemory, reported export ordinal 106.
- Intended signature: int (int failureMode, SIZE_T sizeBytes, void **outMemory, int zeroInitialize).
- HeapAlloc with optional HEAP_ZERO_MEMORY; output through outMemory. Mode 4 reportedly allows error return; other paths retry/terminate.
- Historical GetApplication call: (1,4,&newApplication,DAT_2020d8ec).
- Specific internal-library origin and original use of operator new remain hypotheses.

## Typing pitfalls

VTable fields must be pointers to function definitions. Copying a VTable does not rename/type target functions. Class namespace plus __thiscall determines automatic this; do not add another this parameter.

Copying historically produced Application.conflict; its resolution in api.dll is unverified. Early pure-stdcall-table and three-argument interpretations were corrected by caller assembly. errorCallback was provisionally void *; inspect the stored type and establish the precise function-pointer type.


## GetApplication live verification (2026-11-01)

api.dll active, GetApplication 0x20120442, image base 0x200e0000. Current signature Application * GetApplication(void); decompiler warns unknown calling convention with locked parameter storage. Assembly: MOV EAX,[0x2020de64] at 0x20120448 returns existing g_application via JNZ 0x20120486; on zero calls heap initializer, passes address EBP-4 to allocator with size 4, reads resulting pointer into EAX at 0x20120469. Nonzero pointer receives vtable 0x200f2b24 at [EAX] and is stored globally before RET. Failure zeros EAX/global. PUSH ECX at entry reserves the four-byte local slot; no input ECX use establishing thiscall. All return paths yield object pointer or NULL in EAX. No stack parameters observed; bare RET cannot distinguish zero-argument cdecl/stdcall. Thus Application* return/void arguments supported, exact original calling convention unproven. Earlier historical mapping now live verified for this function only; vtable target signatures not checked. No Ghidra edits/navigation. api.dll revision 1793405534817:4a42ead2:3 saved.

2026-11-01 live readback plus user screenshot, fsx.exe FSX_InitInstance 0x010019e0: pfnGetApplication now GetApplicationFunc*, GetProcAddress cast and NULL comparison use this type, both calls return directly into Application* locals without casts. Screenshot agrees with MCP decompilation. Previous user type-editor screenshot showed Application* return, no arguments, __cdecl (chosen model, original convention unresolved). bIsSupportedWindowsVersion rename observed. No agent Ghidra edits/navigation. Revision 1793490968639:5073cd9e:747 saved.

## Live API boundary verification (2026-11-02)

api.dll SHA256 9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003,
image base0x200e0000. User opened the DLL; fsx.exe remains open, DLL current.
Read-only inspection, saved revision1793405534817:bb3b8861:3, no database edits.

- Vtable0x200f2b24 contains targets2012048a/2012052a/20122817/20120535;
  confirms the Initialize/GetWindowHandle target association independently of
  their stored function signatures. No vtable function types were edited.
- Initialize0x2012048a..20120529 has44 instructions/160 bytes, ends RET0x14.
  Five explicit stack words at EBP+8/+c/+10/+14/+18 feed globals2020de68,
  2020de6c,201f9500,2020de78,2020de80 respectively. Caller supplies ECX=this;
  this particular implementation does not read input ECX before overwriting it.
- At20120492/201204c1, argument2 is only loaded/stored as a DWORD. Current
  commandLine/char* signature is inherited analysis state, not semantic proof.
  Xrefs to2020de6c return only this write in the current analysis. This does
  not prove the value unused: callback20120869 is not defined as a function
  and portions of its control flow are incompletely represented. A read-only
  listing walk found the entry JMP20120873 and unusual control-transfer
  sequences; no function creation/disassembly repair was attempted.
- GetWindowHandle20120535 returns g_mainWindowHandle for windowId0, elseNULL.
- Third argument stored at201f9500 is tested for zero in20122ae1; zero triggers
  PostMessageA(g_mainWindowHandle,0x111,0x110a,1). Original mode name/intent open.

Cross-binary conclusion: original caller's+0x4c word remains SDK-backed
m_nCmdShow / startup-show-mode evidence, not a confirmed DLL-side original type.
Suggested pending rename: api.dll Initialize parameter commandLine ->
startupArgument (provisional), because the implementation only stores the word
and caller evidence contradicts the definite pointer interpretation. No rename
or type edit applied. Keep the reconstructed source boundary ULONG_PTR until
use sites establish a narrower semantic type. This supersedes definite char*
claims in the historical section, but not the verified five-word ABI.

Evidence snapshots: artifacts/api.dll/initinstance/api-initialize.json and api-check-*.json.

2026-11-02 live exit-path check: fsx.exe0x01001720 calls Application vtable+4 with no explicit stack args; api.dll target0x2012052a has PUSH0x20120ed5/CALL0x2013e3c1/RET in the existing listing, but no defined function. Source keeps UnknownSlot04; provisional Shutdown rename suggestion only, not applied. Caller body now normalized-exact58 bytes; full shutdown target semantics remain open. See app-lifecycle-matching.md.

2026-11-02 API slot+8 now independently checked:0x20122817 stores one stack argument at2020de7c, wraps20122ae1, returns0/RET4; loop calls callback and uses returned MSG*. Source UnknownSlot08 changed to reconstructed RunMessageLoop(CurrentMessageCallbackPilot), WINAPI MSG* no-arg callback. EXE Run and wrapper now normalized-exact; no API-body rebuild or Ghidra signature edits. Prior no-argument source model superseded. See app-dispatch-matching.md.
