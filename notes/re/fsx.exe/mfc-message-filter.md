# COM/OLE and message filter

Sources S1 and L1: [../SOURCES.md](../SOURCES.md). Binary fsx.exe.

Live InitializeOleAndMessageFilter, 0x010018e0:
1. CoInitialize(NULL), CoInitializeSecurity; negative HRESULT takes failure path.
2. AfxOleInit.
3. AfxGetThread retrieves current MFC thread object.
4. Call existing filter through VTable +0x04 with this and flag 1, reconstructed as deletingDestructor.
5. Set m_pMessageFilter to NULL.
6. operator_new(0x44), COleMessageFilter base constructor, assign FSXMessageFilter_vftable, zero +0x28/+0x2c.
7. Store new filter and call COleMessageFilter::Register; return its result, otherwise 0.

S1 associates FSXMessageFilter_vftable with 0x01001458. Original derived-class name unknown.

## Current partial layouts

| Type path | Size | Known fields |
|---|---|---|
| /Demangler/CWinThread | 0x44 | +0x40 COleMessageFilter *m_pMessageFilter |
| /Demangler/COleMessageFilter | 0x30 | +0x00 COleMessageFilterVTable *vftable; +0x28/+0x2c undefined4 |
| /COleMessageFilterVTable | 0x08 | +0x04 deletingDestructor *deletingDestructor |

All report alignment 1. These are partial models, not full original layouts. The allocated derived object is 0x44 bytes; this does not establish the base-class size. Unknown fields appear as field37_0x28/field38_0x2c; meaning open.

S1 provisional definition: void * __thiscall deletingDestructor(COleMessageFilter *this, uint flags). Register/stack passing matches the caller; target implementation and exact return type remain to be checked.

## Retained corrections

- Repeated AfxGetThread calls reassign the local pointer, not replace the thread object.
- The OLE filter handles COM calls, not general keyboard/mouse messages.
- Register means COM registration, not Windows registry.
- pMessageFilter[10].vftable/[11].vftable were artifacts of a 4-byte partial type: actual accesses are +0x28/+0x2c.
- Extra uVar1 in the virtual call was a typing artifact; current call is deletingDestructor(pMessageFilter,1).
- previousExceptionRegistration, exceptionHandler, exceptionState describe SEH/cleanup scaffolding, not proven original try/catch syntax.
- Initialization alone does not establish a concrete ActiveX/OLE game feature.


## Installed SP1 MFC evidence and reconstruction (2026-11-01)
Guest experiment21 captured actual afxole.h/inl and afxwin.h/inl. afxole.h protected members m_bEnableBusy/m_bEnableNotResponding and afxole.inl35-38 EnableBusyDialog/EnableNotRespondingDialog directly assign BOOL flags. This supports interpreting original zero stores at0x28/0x2c, pending guest compile-time offset validation. Existing reconstructed minimum0x30 base layout is not complete; installed full COleMessageFilter/derived size0x44 is now tested via C_ASSERT, not assumed by manually padding the class.
Read-only original FSXMessageFilter_vftable0x01001458 inventory: slot04->0x010018a0, slots08/10->0x01001ef0, slot0c->0x01001f00, remaining listed slots include imported thunks. Exact derived overrides are not yet established; empty derived candidate with constructor flags may require further virtual override recovery. No original class-name claim. Ghidra fsx.exe active only, no edits/save.
Added ole_message_filter_pilot.cpp with observed CoInitialize/CoInitializeSecurity/AfxOleInit/delete+replace filter/Register flow, included in startup TU. New ordinary C++ allocation replaces failing opaque stub, no manual SEH code. /EHsc added to GL compilation for MFC C++ unwind semantics and ole32.lib linked. This is a deliberate configuration addition and must be considered if previous matches change; original /EHsc use not yet proven. Guest compile asserts derived size0x44 and flag offsets0x28/0x2c. No OLE helper match claim until build/assembly comparison. Expected original helper length237 bytes. Experiment22 staged, guest run pending.

2026-11-01 experiment22 successful: SP1 compile assertions verify derived/base candidate size0x44 and m_bEnableBusy/m_bEnableNotResponding offsets0x28/0x2c. Code calls actual MFC EnableBusyDialog(FALSE)/EnableNotRespondingDialog(FALSE), compiled to original-offset zero stores. These offsets now supported by installed matching-family headers plus compiler output; not original header revision proof. Original partial0x30 Ghidra model remains incomplete, unchanged; do not mistake it for original full base size.
Original helper0x010018e0..0x010019cc and pilot0x00401190..0x0040127c both237 bytes. Assembly reviewed live: corresponding COM/MFC/new/register paths and exception-registration structure. Excluding twelve address fields at offsets08,18,2d,46,53,5c,7c,92,98,b1,bb,c3 leaves ZERO differing bytes. This is instruction-skeleton evidence only: full EH handler metadata and derived VTable equivalence have NOT been verified, so do NOT report a certified fifth exact function match. ole-instruction-review.json states limitation explicitly. Original stored undefined return remains a BOOL-return cleanup suggestion; no edit applied.
All four earlier helper comparisons remain exact after /EHsc; ShowErrorMessage retains two-byte CMP/TEST difference. Pilot SHA25686AF887AD3F9298DEC469979AE165F1673AE1B6CE44E206060CA7EC4A9A8005A. Full artifacts/source/header/reports archived archive/22-ole-layout-verified. No new build staged. Ghidra read-only check only fsx.exe, saved revision1793559605675:8eb5e96a:275, no edits/navigation/save. Next validation: compare derived VTable slots and EH cleanup/handler data, then InitInstance source/layout differences.

## VTable and EH validation finding (2026-11-01)
Live read-only original FSXMessageFilter_vftable0x01001458 inspected all28 slots through+0x6c. Original slot+0x60 ->0x010018d0 bytes B8 01 00 00 00 C2 04 00 (MOV EAX,1; RET4), corresponding installed MFC slot OnMessagePending(const MSG*). Pilot22 slot+0x60 instead points to imported COleMessageFilter::OnMessagePending. Thus empty-derived candidate was semantically incomplete; instruction skeleton equality did not suffice. Added virtual BOOL OnMessagePending(const MSG*) returning TRUE in experiment23. Original function at0x010018d0 is not currently defined in Ghidra; existing listing instructions and on-disk bytes support identification. Suggested future function creation/name FSXMessageFilter::OnMessagePending (reconstructed namespace), not applied.
Other28-slot review: imported CCmdTarget/MFC method sequence agrees by names at corresponding slots, original+08/+10 RET4 and+0c RET agree with inherited Serialize/Dump/AssertValid empty bodies. Original destructor target0x010018a0 calls MFC80 ordinal700 base destructor and conditionally deletes via ordinal764; pilot generated deleting destructor needs final byte/reference comparison. Imported alloc original0x0100208c->MFC80 ordinal762; delete0x01001ffc->ordinal764; pilot MAP identifies MFC80 operator new/delete. Exact export ordinal mapping to decorated names still not independently checked against matching runtime DLL export table.
EH original handler0x01002ca8 and pilot22 handler0x004025f8 have corresponding27-byte guard-cookie/FuncInfo/CxxFrameHandler3 sequence. Original FuncInfo0x01002dec and pilot0x0040383c both magic0x19930522,maxState1,no catch/try tables and EHflags1. Unwind maps: original0x01002de4 {toState:-1, action:0x01002ca0}; pilot0x00403834 {-1,0x004025f0}. Actions both load allocation from EBP-0x10 and jump to CObject delete wrapper; original0x01001f10/pilot0x00401010 wrappers MOV EDI,EDI/PUSH EAX/CALL MFC delete/POP ECX/RET agree apart from call displacement. No evidence for explicit source try/catch; ordinary new constructor cleanup accounts for observed metadata. EH structure is supported; do not certify full helper exact until corrected VTable and remaining mappings verified. No Ghidra edits/save; live fsx.exe only, revision1793559605675:8eb5e96a:275 saved. Experiment23 build pending.

## Validated filter matches (2026-11-01, experiment23)
Reproducer scripts/compare-ole-filter.ps1 now parses actual original/pilot PE import tables, compares DLL+ordinal/name identities, checks all28 VTable slots and normalizes only validated target fields. Results: InitializeOleAndMessageFilter original0x010018e0 vs pilot0x004011a0 address-normalized EXACT237 bytes (12 fields). OnMessagePending original0x010018d0 vs pilot0x00401160 RAW EXACT8 bytes. Deleting destructor original0x010018a0 vs pilot0x00401170 address-normalized EXACT35 bytes, both imported calls checked. No instruction patches or unverified padding exclusion.
All28 VTable slot targets now verified: imported MFC80 DLL ordinals equal, empty inherited leaves byte-equal, destructor/always-TRUE override validated. This supersedes prior unresolved override/ordinal-mapping limitations for this candidate. Import checking directly establishes alloc/free/base-destructor ordinal identity without needing external DLL files; external implementation bytes/runtime DLL version are not independently compared.
EH handler27 bytes normalized with cookie target, CxxFrameHandler3 import and FuncInfo reference checked. FuncInfo36 bytes equal except verified unwind-map pointer; maxState1, unwind toState-1. Unwind action8 bytes equal after delete-wrapper mapping; wrapper10 bytes equal after actual MFC delete-import mapping. These checks close the previous excluded EH-metadata uncertainty for the observed initializer cleanup. RTTI and full module layout remain out of scope; reconstructed class names/source syntax and precise original header/compiler flags unproven.
Pilot SHA2562D14D02358089F1D8019B7C1E688EAA6BD1C35086BF7E2520AD6A21792C54FB4. Complete archive23-ole-filter-verified-matches contains EXE/PDB/MAP/source/header/build/scripts and machine-readable ole-filter-comparison.json. Four prior helpers retain matching scopes; ShowErrorMessage unchanged two-byte near-match. No Ghidra query/edits/save this turn; previous saved state remains last known state. No new guest build staged.
