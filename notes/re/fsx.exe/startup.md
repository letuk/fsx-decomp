# EXE startup and FSXApp

Sources S1, S2, L1: [../SOURCES.md](../SOURCES.md). All addresses refer to fsx.exe, image base 0x01000000.

## Entry and flow

Live: entry at 0x0100262e calls ___security_init_cookie and ___tmainCRTStartup (0x01002363). CRT startup runs initterm_e/initterm and calls external AfxWinMain.

Historical identification of FSX_InitInstance (0x010019e0) as MFC InitInstance override: VTable slot +0x58, entry 0x010013f0 in table 0x01001398. The internal MFC call was not retraced in L1.

Current FSXApp::FSX_InitInstance decompilation:
1. Save security cookie and this as pFsxApp.
2. AfxEnableControlContainer(NULL), load language.dll.
3. CheckWindows51SP2OrLater or CheckWindows52SP1OrLater.
4. Load D3DX9_34.DLL, call D3DXCheckVersion(0x20,0x22), unload on success.
5. Determine executable directory; check msxml4.manifest using CreateActCtxA and release context.
6. Load API.DLL from executable directory (historical call 0x01001bf1); resolve GetApplication.
7. InitializeOleAndMessageFilter (0x010018e0).
8. Obtain Application; call Initialize through VTable +0x00.
9. On success assign g_mainWindow to g_fsxApp.m_pMainWnd; GetWindowHandle(0) through +0x0c; CWnd::Attach.
10. Return startupResult; common error path returns 0.

## Global objects

Live: InitializeGlobalFSXApp at 0x01001660 calls CWinApp::CWinApp on g_fsxApp, sets FSXApp_vftable and creates event "FS98MAIN". GetLastError()==0xb7 causes its handle to be closed and set to NULL.

Historical address mapping: g_fsxApp 0x010043e0; FSXApp_vftable 0x01001398; g_mainWindow 0x01004388. Counterpart 0x010016d0 cleans up the event/CWinApp; initialization reference at 0x01002d00. The latter two locations were not reread in L1.

Live /FSXApp: size 0xa8, alignment 1:

| Offset | Type | Field |
|---|---|---|
| 0x00 | void * | vftable |
| 0x20 | CWnd * | m_pMainWnd |
| 0x4c | char * | m_lpCmdLine |
| 0xa4 | HANDLE | singleInstanceEvent |

This is a minimum layout, not the proven original class size. Alignment is the Ghidra model value, not original packing evidence. FSXApp is a reconstructed name; the base-constructor call supports CWinApp inheritance.

0x01004400 = object +0x20; 0x01004484 = object +0xa4. The global is an FSXApp object, not FSXApp *. Distinguish CWnd object, CWnd pointer, and HWND.

## Errors

| errorId | Cause visible in current decompilation |
|---|---|
| 0x17 | Windows checks |
| 0x10 | DirectX check path |
| 0x19 | Manifest CreateActCtxA |
| 0x18 | API.DLL/GetApplication unavailable |
| 0x1c | OLE/message-filter initialization |

LAB_01001cc5 leads to ShowErrorMessage (historical function address 0x01001d30). The goto preserves specific errors instead of overwriting them with 0x10; it does not establish an original-source goto.

## Compiler artifacts

Live: securityCookie = __security_cookie ^ (uint)&hLanguageModule.
S2 historically checked MOV EAX,[0x01004004]; XOR EAX,ESP; MOV [ESP+0x550],EAX.
The local's address represents the stack pointer, not an uninitialized handle value.

Live: 0x0100263d is void __fastcall __security_check_cookie(uint cookie), marked inline; replaced by security_check_cookie injection in the caller. FSX_InitInstance now returns startupResult or 0. Earlier extraout_EAX and a supposed cookie-check return value were resolved artifacts.

AfxEnableControlContainer(NULL) enables control-container support; it does not establish specific ActiveX usage. The C++ default-argument explanation is background, not recovered original source spelling.


## Windows version helpers (2026-10-31, live verified)

fsx.exe 0x01001760 (CheckWindows51SP2OrLater) and 0x01001800 (CheckWindows52SP1OrLater) build OSVERSIONINFOEXA requests for 5.1 SP2 and 5.2 SP1 respectively. Both select major/minor/service-pack-major (type mask 0x23), each with VER_GREATER_EQUAL (3). Version comparisons are hierarchical, not independent field comparisons. Microsoft reference: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-verifyversioninfoa

Both return the BOOL from VerifyVersionInfoA in EAX. Calls at 0x010017db / 0x0100187b are followed only by cookie checks and epilogues. The live checker at 0x0100263d compares ECX and returns without changing EAX on success. Stored helper return types are undefined; the displayed void/return; is a decompiler artifact. Suggested correction: BOOL return type for both helpers. No Ghidra changes applied.

Variable suggestions in both helpers: local_ac -> versionInfo (OSVERSIONINFOEXA request); local_8 -> securityCookie (compiler stack protection); uVar1 -> conditionMaskIntermediate (64-bit mask accumulated by VerSetConditionMask). Existing dwlConditionMask is already descriptive. VerSetConditionMask decompilation shows a suspicious four-argument first call; inspect its stored imported signature before editing (expected DWORDLONG mask, DWORD type, BYTE condition). The assembly passes the initial 64-bit zero as two stack words.

Source: current S2 chat; live get_functions disassembly/decompilation. No binary hash verified. Revision 1793460953482:5073cd9e:13 reports unsaved_changes=false.

Follow-up 2026-10-31 (live verified): UI cursor is in fsx.exe at 0x01001760. The imported function at EXTERNAL:00000068 currently has signature `undefined VerSetConditionMask(void)`, confirming missing imported prototype information. Microsoft documents a 64-bit unsigned return and three parameters: ULONGLONG ConditionMask, DWORD TypeMask, BYTE Condition (https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-versetconditionmask). Correcting the imported prototype is recommended before interpreting the apparent four-argument call or finalizing intermediate variable types. This is a suggestion only; no Ghidra edits or navigation performed. Both helper names and existing dwlConditionMask should be preserved. Saved revision remains unchanged.
`n2026-10-31 live verification, fsx.exe 0x01001760: SUB ESP,0xa8 at 0x01001765 reserves stack space. LEA EAX,[EBP-0xa8] at 0x0100177b supplies versionInfo to memset at 0x01001784; arguments are destination, zero, 0x9c bytes. This clears the stack-resident OSVERSIONINFOEXA, not the condition mask and not a heap allocation. Current decompilation shows versionInfo and securityCookie already renamed (observed existing state, not edits by this chat). Revision 1793460953482:5073cd9e:43 is unsaved.

2026-10-31 live verified, fsx.exe 0x01001760: user changes now appear as ULONGLONG conditionMaskMajor and ULONGLONG conditionMaskMajorMinor, distinguishing the first and second mask results. These descriptive names match the data flow and supersede the generic conditionMaskIntermediate suggestion for this helper. dwlConditionMask remains DWORDLONG. Import EXTERNAL:00000068 still has undefined VerSetConditionMask(void); helper return remains undefined. No edits applied by this check. Revision 1793460953482:5073cd9e:100 is unsaved.

2026-10-31 completed edit, fsx.exe EXTERNAL:00000068: changed VerSetConditionMask Condition parameter from UINT to BYTE via set_function_prototype, preserving ULONGLONG return/first parameter, DWORD TypeMask and __stdcall. Readback confirms BYTE at Stack[0x10]:1; this describes the meaningful byte, not the full ABI stack slot. Helper 0x01001760 still shows 0.0 and broken return data flow; user-reported typedef double ULONGLONG remains the suspected cause, not repaired in this edit. Revision 1793460953482:5073cd9e:188 is unsaved.

2026-10-31 live verified, fsx.exe EXTERNAL:00000068: user changed parameters to ulonglong, dword, byte; return still ULONGLONG. Helper 0x01001760 now shows integer zero in first call, but return assignments remain missing and CONCAT44/extraout artifacts persist. Existing mask locals remain ULONGLONG. Recommendation: repair underlying ULONGLONG/DWORDLONG typedefs (double base reported by user), or use ulonglong consistently for return and mask locals as a temporary workaround. No edits applied; revision 1793460953482:5073cd9e:310 unsaved.

2026-10-31 live verified, fsx.exe 0x01001760: user-applied local signature override on first VerSetConditionMask call restores conditionMaskMajor = VerSetConditionMask(0,2,3). Second call now consumes conditionMaskMajor correctly, but still lacks return assignment; third call retains CONCAT44. Thus the override fixes one call site; why imported prototype alone does not propagate remains unresolved. Recommended same local signature for remaining two calls. No edits by this review. Revision 1793466450390:5073cd9e:357 unsaved.

2026-10-31 rundown, live verified in fsx.exe: 0x01001760 now returns BOOL (__stdcall stored; zero-argument machine code does not uniquely distinguish cdecl/stdcall). All three VerSetConditionMask results are recovered cleanly as ulonglong conditionMaskMajor, conditionMaskMajorMinor, dwlConditionMask; VerifyVersionInfoA result is returned. User-applied local overrides resolve this helper's display; exact override records not directly inspected. Result variable bIsWinows51SP2OrLater has a spelling error; suggested bIsWindows51SP2OrLater, not applied. 0x01001800 remains undefined return with CONCAT44/extraout artifacts and generic locals; same cleanup remains suggested. Both imported signatures now use integer ulonglong masks, including VerifyVersionInfoA. ULONGLONG/DWORDLONG typedef bases remain user-reported double and are not proven repaired. FSX_InitInstance 0x010019e0 accepts either helper success; both failing selects error 0x17. Both fsx.exe and api.dll are open, fsx.exe active; no navigation or Ghidra edits in this review. Revision 1793467972012:5073cd9e:418 is saved, superseding earlier unsaved reports.

2026-10-31 completed edit: added an English learning-oriented plate comment to fsx.exe 0x01001760, covering stack allocation/memset, version fields, condition-mask stages, stdcall argument slots and EDX:EAX return, IAT/CALL ESI, hierarchical verification, cookie artifacts, and local overrides. Readback verified. Existing result name is now bIsWindows51SP2OrLater (user correction observed). get_function_documentation reports program hash daffc3740373c76a73d312bb78d592e0945c3debfac07e8f4d881803db288d59. No type or code edits; database not saved by this operation.

2026-10-31 live verified, fsx.exe 0x01001800: second helper cleanup completed by user. BOOL return, versionInfo values 5.2 SP1, uint securityCookie, ulonglong conditionMask accumulated over all three calls, and BOOL bIsWindows52SP1OrLater returned correctly. No CONCAT44/extraout artifacts remain. Single reused conditionMask name is appropriate; separate stage names are unnecessary. Revision 1793468800776:5073cd9e:490 saved. Read-only review.

## D3DX startup check (2026-10-31)

Live verified in fsx.exe, FSXApp::FSX_InitInstance 0x010019e0: LoadLibraryA("D3DX9_34.DLL") is followed by GetProcAddress(hModule, "D3DXCheckVersion") and an indirect call with 0x20 (32), 0x22 (34). Loading alone does not execute this explicit version-check call. Missing module, missing export, or zero result selects errorId 0x10 and the common failure path; nonzero result calls FreeLibrary before continuing to executable-path/manifest handling. Microsoft documents BOOL D3DXCheckVersion(UINT D3DSDKVersion, UINT D3DXSDKVersion), checking build/runtime D3DX version agreement: https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxcheckversion . DLL implementation was not inspected.

Suggestions only: pFVar1 -> pfnD3DXCheckVersion, supported by the resolved export name. iVar2 is reused for several later results; a D3DX-specific result name should only be applied to a split variable. Suggested function-pointer prototype: BOOL (__stdcall *)(UINT, UINT). No Ghidra edits or navigation. Revision 1793468935670:5073cd9e:498 is saved.

2026-10-31 live naming review, fsx.exe FSX_InitInstance 0x010019e0: user-applied pfnD3DXCheckVersion and BOOL bIsD3dMatching observed; exeDirectoryStart/exePathStart now char[260]. exePathBuffer is separated from errorDetailText in decompilation and used for GetModuleFileNameA destination. dwExePathSize still represents both input capacity and returned length. Suggestions only: exeDirectoryStart -> exeDirectory; exePathStart -> exePath; bIsD3dMatching -> bIsD3DXVersionMatching; acStack_558 -> exeDrive and acStack_110 -> exeDirectoryPart based on _splitpath_s output positions and _makepath_s inputs. Preserve current types pending separate review; split capacity/result before giving distinct semantic names. No Ghidra edits. Revision 1793481262086:5073cd9e:575 saved.

2026-10-31 live assembly check, fsx.exe 0x010019e0: drive-buffer address computed at 0x01001af3 by LEA EDX,[ESP+0x2c]. Let B be ESP after local allocation and saved EBX/ESI/EDI, with no pending arguments. Seven preceding argument PUSH instructions lower ESP by 0x1c, so the drive buffer is B+0x10. The later LEA EDX,[ESP+0x1c] at 0x01001b13 follows three PUSH instructions and computes the same B+0x10. Prologue aligns ESP to 8 bytes then reserves 0x554 bytes; B = aligned ESP - 0x560. Buffer therefore starts at aligned ESP - 0x550. Ghidra char[4] extent does not prove original source array size: _splitpath_s receives drive capacity 3 (PUSH 3 at 0x01001af1), sufficient for drive letter, colon, NUL. Fourth byte may be padding; original size unproven. No Ghidra edits. Revision 1793481455350:5073cd9e:608 reports unsaved_changes=true.

2026-10-31 live readback, fsx.exe FSX_InitInstance 0x010019e0: user renames exeDrive[4], exeDirectoryPart[260], exePath[260], exeDirectory[260] verified and semantically consistent with split/make path calls. Capacity/result now separated: dwExePathBufferSize=0x103; dwExePathSize=GetModuleFileNameA(...,dwExePathBufferSize). Suggestions only: dwExePathSize -> dwExePathLength; bIsD3DVersionMatching -> bIsD3DXVersionMatching for precision. Earlier combined-capacity/result report is superseded. No Ghidra edits. Revision 1793483097771:5073cd9e:637 saved.

2026-10-31 live assembly verification, fsx.exe manifest initialization: using B as ESP after saved EBX/ESI/EDI with no pending arguments, 0x01001b27 PUSH 0x103 sets ESP=B-4; 0x01001b2c LEA ECX,[ESP+0x355] yields B+0x351 (manifest buffer +1). After PUSH EBX and PUSH ECX, ESP=B-0xc; MOV byte ptr [ESP+0x35c],BL at 0x01001b35 zeroes B+0x350 (buffer start). memset at 0x01001b3c writes 0x103 zero bytes from B+0x351 through B+0x453. Together this initializes 260 bytes at B+0x350. Pending argument slots contain destination pointer, zero, count; LEA does not read an existing pointer from the buffer. No Ghidra edits; revision 1793483217796:5073cd9e:652 saved.

2026-10-31 live verification, fsx.exe 0x01001b50-0x01001b5d: LEA EAX,[ESP+0x360] occurs after four argument pushes (ESP=B-0x10), hence EAX=B+0x350=&manifestPath[0]. PUSH 0x104 then PUSH EAX supplies capacity and destination to manifest _makepath_s. Current decompiler correctly displays manifestPath. User reports Listing shows an earlier EXE-path/length variable name on PUSH; symbolic register-markup explanation is provisional because the rendered Listing was not inspected. Raw instruction data proves EAX is overwritten with manifest buffer address before PUSH, irrespective of displayed alias. Revision 1793484006691:5073cd9e:665 saved; no Ghidra edits.

2026-10-31 live review, fsx.exe FSX_InitInstance 0x010019e0: AStack_550 is currently ACTCTXA, a stack-resident input descriptor for CreateActCtxA. cbSize=0x20, lpSource=manifestPath, dwFlags=0, all optional fields zero. Creation failure selects error 0x19; successful handle is immediately released, with no ActivateActCtx call in this sequence. This checks context creation from msxml4.manifest, not proven persistent activation. Suggested rename AStack_550 -> actCtx (descriptor), preserving hActCtx (returned handle); not applied. Microsoft structure reference: https://learn.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-actctxa . Revision 1793484006691:5073cd9e:675 saved; no Ghidra edits.

## Full InitInstance review (2026-10-31)

Live verified fsx.exe FSX_InitInstance 0x010019e0, revision 1793485977652:5073cd9e:707 saved. Reviewed current decompilation, locals, and all 246 instructions through RET at 0x01001ce0. No Ghidra edits or navigation.

- User-applied actCtx, oleInitResult/startupResult split, dwExePathLength, and bIsD3DXVersionMatching verified. Existing path names/data flow consistent.
- Remaining fragmented buffer: apiDllPathStart char at Stack[-0x320] plus auStack_31f. Suggested char apiDllPath[260] starting at the former byte, not 264 bytes: clear first byte +0x103 bytes at 0x01001bae-0x01001bc3; _makepath_s capacity 0x104 and LoadLibraryA use the same start (baseline B+0x248). Do not absorb the extra padding into the inferred buffer.
- pfnD3DXCheckVersion remains FARPROC, pfnGetApplication remains a generic function pointer with casts at use sites. Suggested precise pointer types BOOL (__stdcall *)(UINT,UINT) and Application *(*)(void), with GetApplication calling convention requiring separate target verification before applying.
- Stored helper signature at 0x010018e0 remains undefined InitializeOleAndMessageFilter(void), despite inferred int return. ShowErrorMessage 0x01001d30 remains undefined ShowErrorMessage(void), despite inferred six parameters and DWORD return; its decompiler warns type propagation not settling. Full prototypes require separate verification; do not copy inferred signatures blindly, especially callback type/ABI.
- ApplicationVTable fields remain Initialize* at +0 and GetWindowHandle* at +0xc. Caller assembly independently confirms five stack arguments and ECX=this at 0x01001c64-0x01001c70, and zero window ID with ECX=this at 0x01001c88-0x01001c90. No api.dll target signatures reverified; do not claim complete cross-program type agreement.
- Optional names: bIsVersionGreaterOrEqual -> bIsSupportedWindowsVersion (result of either accepted OS helper); hModule is reused for language/D3DX/EXE/API and final error resource module, so split uses before specific renames. errorId remains undefined4; integer type should follow confirmed ShowErrorMessage prototype. Existing function comment's blanket incomplete-recovery warning is outdated; refine when editing.
- Original-code behavior, not decompiler repair targets: GetModuleFileNameA receives 259 despite 260-byte allocation; path helper return codes ignored; activation context immediately released; D3DX FreeLibrary only on successful check; API module remains loaded on success. No code changes recommended from this review.

2026-11-01 live readback, fsx.exe FSX_InitInstance 0x010019e0: pfnD3DXCheckVersion now D3DXCheckVersionFunc*, cast from GetProcAddress, two-argument call and BOOL result preserved. /D3DXCheckVersionFunc exists; get_type_size reports size -1 (function definition has no object size). These endpoints do not expose its internal parameter/return/convention definition, so full signature not independently verified. apiDllPath now char[260], correct initialization/path use observed. User changes, no agent Ghidra edits. Revision 1793486487350:5073cd9e:729 saved.

## ShowErrorMessage review (2026-11-01)

Live verified in fsx.exe 0x01001d30 through 0x01001ee2, with helpers 0x01001590/0x01001600 and caller FSX_InitInstance. MCP decompilation and full instruction listings inspected. Stored signature is still undefined ShowErrorMessage(void); inferred six stack arguments are confirmed by EBP-relative reads and RET 0x18 (stdcall-compatible callback). No Ghidra edits/navigation. Revision 1793490968639:5073cd9e:747 saved.

- Arguments by use: UINT errorId; second argument is a formatting argument or direct text pointer depending on branch; non-null caption string pointer; DWORD style; WORD icon resource ID; HINSTANCE string-resource module. Proposed descriptive prototype is provisional until callback call sites in API.DLL are checked.
- If resource module is NULL, helper 0x01001590 formats "Error ID = %d\r\n%s" using errorId and argument 2. If module is non-null and ID nonzero, LoadStringA loads up to 512 bytes and the same helper formats that resource using argument 2. If module is non-null and ID zero, helper 0x01001600 copies argument 2 directly. Destination capacity is 1024 bytes. Formatting helper receives destination in EDI, capacity in EAX, format/varargs on stack; missing register parameters explain the misleading decompilation. It uses _vsnprintf, returns HRESULT-like 0/0x80070057/0x8007007a; negative formatting status aborts display. Direct-copy status is ignored.
- local_640[1] through local_618 form a 40-byte MSGBOXPARAMSA, not independent logical locals. Field order, cbSize=0x28 and pointer passed to MessageBoxIndirectA establish the identification. local_640[0] is separate thread/dialog result. local_410 is the start of the 1024-byte text buffer; displayed 1028-byte extent is not proven original size. local_610 + local_60f represent a 512-byte resource buffer.
- Caption uses argument 3 unless empty, then NULL. It is dereferenced without NULL check. Caller supplies an empty string at 0x010014c8 (raw bytes verified), so default message-box caption applies.
- Attempts to resolve owner via GetApplication/GetWindowHandle(0)/GetLastActivePopup, but overwrites hwndOwner with NULL at 0x01001e9a. No null guard for returned Application pointer in that branch.
- Instance comes from AFX_MODULE_STATE +0x08, distinct from the input resource module. Style becomes (inputStyle | 0x10080) & ~0x80 | 0x40030: clears MB_USERICON, adds MB_SETFOREGROUND, MB_TOPMOST and warning-icon bits. Input icon resource is stored but ignored because MB_USERICON is cleared. With caller style=0, final style is 0x50030 (OK/warning/foreground/topmost).
- CreateThread uses the imported MessageBoxIndirectA address directly as thread start and &MSGBOXPARAMSA as its sole argument. WaitForSingleObject(INFINITE), GetExitCodeThread and CloseHandle follow. Thus caller still blocks; result is the dialog button ID via thread exit code, initially zero on early failure. Wait/query return statuses are unchecked.
- Suggestions only: param_1 -> errorId; param_2 -> messageArgument (provisional dual role); param_3 -> caption; param_4 -> messageBoxStyle; param_5 -> iconResourceId; param_6 -> hStringResourceModule; local_c -> securityCookie; iVar1 -> formatStatus; hHandle -> hMessageBoxThread. Reconstruct MSGBOXPARAMSA before naming its fragments. Helper descriptive names FormatMessageBuffer/CopyMessageBuffer are suggestions, not established original library names.

API references: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-msgboxparamsa and https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox . Resource strings and API.DLL callback callers remain uninspected.

2026-11-01 live assembly clarification, fsx.exe FSX_InitInstance: 0x01001a19 calls LoadLibraryA("language.dll"); 0x01001a1b copies returned EAX to ESI; 0x01001a1d stores ESI in baseline [ESP+0x0c]. On API-loading/export failure, 0x01001c10 reloads that saved language-module handle into ECX, and 0x01001c14 PUSH ECX supplies ShowErrorMessage argument 6 (right-to-left stack argument order). It is not the API.DLL handle. The decompiler merges branch values into hModule; hLanguageModule is the precise source on this branch. No Ghidra edits; saved revision unchanged.

2026-11-01 live user-edit readback, fsx.exe FSX_InitInstance 0x010019e0: HMODULE hErrorRessourceModule now separated from other hModule uses; language handle selected for OS/D3DX/manifest/API failures, NULL for OLE failure. Data flow is consistent, including initial LoadLibraryA result feeding hLanguageModule. Suggested spelling correction only: hErrorRessourceModule -> hErrorResourceModule (English resource). ShowErrorMessage stored prototype remains undefined; parameter-name suggestions not applied. No agent Ghidra edits; revision 1793492540064:5073cd9e:757 saved.

2026-11-01 provenance hypothesis, fsx.exe FUN_01001590: strong match to an optimized StringCchPrintfA-style StrSafe implementation (strsafe.h): validates capacity 1..0x7fffffff, _vsnprintf with capacity-1, explicit final NUL, returns S_OK / 0x80070057 / 0x8007007a. Microsoft StringCchPrintfA documentation describes matching capacity/varargs/HRESULT semantics: https://learn.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcchprintfa . Exact source/library version and StringCch versus StringCb identity remain unproven (ANSI byte/character sizes coincide). EDI/EAX parameter placement is consistent with compiler optimization, not the public API ABI. Suggested name StringCchPrintfA_optimized is provisional; no rename applied.

2026-11-01 completed MCP edit, fsx.exe 0x01001590: renamed FUN_01001590 to StringCchPrintfA_optimized at user request; added English plate comment identifying provenance as provisional and documenting EDI/EAX/stack inputs and HRESULT behavior. Name/comment readback verified. Stored prototype unchanged/incomplete. Revision 1793493280309:5073cd9e:828 has unsaved_changes=true; no database save or navigation performed.

2026-11-01 live readback, fsx.exe 0x01001590: user-defined HRESULT __cdecl StringCchPrintfA_optimized(char *pszDest, size_t cchDest, char *pszFormat, ...) has correct custom parameter storage EDI:4, EAX:4, Stack[0x4]:4. Callee decompilation now uses parameters correctly. However ShowErrorMessage call sites still render obsolete stack-only calls (three/two arguments; resource call casts param_2 to size_t), so caller propagation remains unresolved; do not interpret the displayed arguments as actual register inputs. Existing call-site overrides versus varargs/custom-storage handling remain hypotheses pending inspection. Prior plate sentence claiming stored prototype incomplete is now outdated. ShowErrorMessage resourceText[512] verified; argument 6 currently named hMessageBoxThread incorrectly (resource module, not thread); suggest hStringResourceModule, while local hHandle is actual thread handle. No agent Ghidra edits; revision 1793493918376:e38ea5f1:31 unsaved.

2026-11-01 attempted read-only override inspection: MCP has no discovered dedicated call-signature-override reader. run_script_inline with a read-only symbol/storage inspection was rejected because server script execution is disabled (GHIDRA_MCP_ALLOW_SCRIPTS). No script ran and no Ghidra data changed. Cannot establish whether overrides exist at 0x01001dd3 / 0x01001e03 from available endpoints; do not claim override presence/absence. Next route is UI inspection or user-enabled script capability. Varargs/custom-storage issue remains unresolved.

2026-11-01 live MCP script inspection now succeeds after user environment setup. Read-only InspectFormatterOverrides confirms fsx.exe 0x01001590 custom storage=true, varargs=true, parameters EDI:4/EAX:4/Stack[0x4]:4. Symbol-table inspection found zero matching override symbols in ShowErrorMessage / at calls 0x01001dd3 and 0x01001e03. This rules out the suspected ordinary stored call-signature overrides in the inspected symbol scope; custom-storage/varargs caller handling remains to investigate. No Ghidra database edits by script (script source generated by MCP outside database).

## Optimized formatter ABI repair (2026-11-01, live verified)

Binary: fsx.exe, formatter 0x01001590; calls in ShowErrorMessage at 0x01001dd3 and 0x01001e03. Ghidra 12.1.4. No ordinary local signature override symbols found; custom-storage plus varargs was the actual failure mode. Installed local decompiler source fspec.cc, FuncProto::characterizeAsInputParam around line 4292 (and related routines), explicitly bypasses locked parameter storage for varargs and delegates to the convention model. Ordinary __cdecl therefore classified stack inputs while ignoring EDI/EAX as inputs. Related upstream issue: https://github.com/NationalSecurityAgency/ghidra/issues/4334 . Earlier advice that __cdecl plus custom storage suffices is superseded for this variadic function.

Completed repair: added program-local prototype model __fsx_edi_eax_varargs using SpecExtension.addReplaceCompilerSpecExtension; input allocation order EDI, EAX, Stack[0x4] onward, EAX return, caller stack cleanup (extrapop=4, stackshift=4). Applied dynamic parameter storage using the model, preserving HRESULT, parameter names/types and varargs. Model name is reconstructed analysis metadata, not an original compiler ABI name. XML is retained in scripts/ghidra/fsx-edi-eax-varargs.xml for reproduction via Program Options / Specification Extensions (or SpecExtension API). No installation files or binary instructions changed; no program navigation.

Validation: XML validation passed; fresh DecompInterface plus MCP readback both show destination local_410 and capacity 0x400 at BOTH calls. Resource call has (local_410,0x400,resourceText,param_2); fallback has (local_410,0x400,"Error ID = %d\r\n%s",errorId,param_2). Callee decompilation preserves capacity checks and _vsnprintf data flow, parameter storage readback EDI:4/EAX:4/Stack[0x4]:4. Plate comment updated to explain repair and provisional provenance.

Remaining: ShowErrorMessage type-propagation warnings still present (not caused/fixed by this formatter ABI repair); FUN_01001600 prototype and MSGBOXPARAMSA reconstruction remain separate cleanup. Suggested local names: _Count -> maxFormattedChars, uVar1 -> formattedLength (signed int merits checking against CRT prototype), HVar2 -> formatStatus. Suggestions not applied. Database revision 1793536182953:c57cedf3:57 is unsaved. Save/backup still required; documentation/XML is not a database backup.

2026-11-01 completed MCP edit, fsx.exe ShowErrorMessage 0x01001d30: local_410 char[1028] retyped to char[1024] and renamed messageText, readback verified at Stack[-0x410]:1024. Capacity 0x400 in both formatter calls and copy helper supports 1024-byte modeled text buffer; former extra four bytes are not proven part of original array. resourceText remains separate 512-byte resource-format buffer. No instruction edits or save; revision 1793546575673:c57cedf3:77 unsaved.

## Copy helper recompilation pilot (2026-11-01)
Live verified fsx.exe FUN_01001600 at 0x01001600..0x01001647, 72 bytes: destination ECX, source EAX, capacity constant 0x400 in EDI; copies until NUL or capacity exhausted, truncation backs destination up one and terminates with HRESULT 0x8007007a; success terminates and returns zero. The TEST EDI,EDI branch on encountering source NUL remains even though loop exit may render its zero case unreachable; preserve this compiler/header artifact in comparisons. Stored single-parameter fastcall prototype omits source EAX; do not treat it as a complete ABI. Suggestion only: param_1 -> destination, in_EAX -> source, iVar2 -> remainingCapacity; accurate source parameter recovery should precede renaming. No renames or type edits applied.
Hypothesis: optimized StrSafe StringCchCopyA-style implementation with constant capacity, exact Cch/Cb identity/version unproven. Experiment 5 stages installed SDK strsafe.h StringCchCopyA through a pilot-only noinline wrapper and dynamic source from GetCommandLineA, without changing established version-helper source/options. noinline keeps a comparison boundary and is not evidence of original wrapper syntax. Guest /showIncludes and header copy will capture provenance. Comparison and regressions pending guest build. fsx.exe only open/current; revision 1793546795596:8eb5e96a:3 saved. No Ghidra edits/navigation/save.

2026-11-01 experiment 5 guest build verified: actual included header is VS8 VC/PlatformSDK/include/strsafe.h, captured as sdk-strsafe.h. DUMPBIN identifies emitted StringCopyWorkerA at 0x00401160..0x004011a9 (74 bytes) and pilot wrapper as a tail jump at 0x004011b0. Worker source is EAX; destination specialized to global g_pilotMessage via MOV ECX,immediate. Remaining loop, HRESULT and termination paths closely track original FUN_01001600, but not an exact match. SDK origin is now supported by a concrete compilation, though original header revision and ANSI Cch/Cb entry identity remain unproven. Global specialization is verified from compiled code, not evidence of original API ABI.
Both version helpers remain exact after independently verified five-field address mapping (scripts/compare-version-pilot.ps1). Build SHA256 D85B7266A9B58A99BFCF87A109A9623F452E3E4A07EBC83CA439241094A685CF. Experiment 5 artifacts/header/source/batch archived in vm-share/pilot-copy-05/. Experiment 6 changes only harness destination from global array to local char[1024], consistent with original ShowErrorMessage caller context; SDK wrapper and flags unchanged. Staged separate input/work directories avoid outstanding shared-folder file locks. Guest run pending. No Ghidra changes or fresh state query.

## Copy worker exact match (2026-11-01, experiment 6)
Live verified on-disk byte comparison: original fsx.exe FUN_01001600 at 0x01001600..0x01001647 and rebuilt SDK StringCopyWorkerA at 0x00401180..0x004011c7 are RAW EXACT across all 72 bytes. No reference normalization, instruction patch, padding insertion or byte masking. Pilot SHA256 D27138C6AF00FC1944C6B2F40754D52CDA6FCB6822479E21EBD52FCC24688AA6; original SHA256 61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735. Reproducer scripts/compare-copy-pilot.ps1 identifies the worker through DUMPBIN symbol and reads actual PE bytes. Reports/artifacts/header/source/batch archived in vm-share/pilot-copy-06-match/.
Period SDK StringCchCopyA(destination,1024,source), through unchanged pilot-only noinline wrapper and stack-local destination in harness, produces the exact worker, including EAX source, ECX destination, fixed capacity and alignment LEA ECX,[ECX]. This strongly supports identifying original machine code as an optimized StrSafe StringCopyWorkerA specialization. It does not prove original exported/source name, precise original header revision, or Cch vs ANSI Cb entry; wrapper is pilot scaffolding. Established names preserved in Ghidra; no type/name edits applied. Optional supported rename suggestion: FUN_01001600 -> StringCopyWorkerA_1024_optimized (reconstructed specialization label).
Both prior version-helper bodies remain address-normalized exact in the expanded build. Three matching bodies total, not a matching EXE or independent runtime proof. Ghidra was not queried/edited/navigated/saved this turn; prior saved-state readback remains last known state.

2026-11-01 experiment 7 staged: formatter at fsx.exe 0x01001590..0x010015eb (92 bytes) reverified live, EDI destination/EAX capacity/stack format+varargs and _vsnprintf IAT 0x01001208 retained. New src/fsx/string_format_pilot.cpp calls installed SDK StringCchPrintfA twice with local char[1024], one runtime-selected safe format and original fallback format. Caller is pilot-only scaffolding, not reconstructed ShowErrorMessage. No compiler convention written into source; SDK/LTCG must generate observed ABI. Both version helpers and copy worker retained as regression targets. Build pending; no new match claimed. No Ghidra edits/navigation/save; revision 1793554472087:8eb5e96a:5 reports saved.

2026-11-01 experiment 7 verified: original formatter 0x01001590 (92 bytes) compared with SDK StringCchPrintfA at rebuilt 0x00401210. SDK already generates EDI destination/EAX capacity and stack format/varargs, but calls _vsnprintf_s with five arguments, unlike original _vsnprintf with four. Captured sdk-strsafe.h lines 63-69 select _STRSAFE_USE_SECURE_CRT by __GOT_SECURE_LIB__; lines 6702-6706 explicitly select _vsnprintf_s versus _vsnprintf. This is an identified header-configuration difference, not evidence that original source used this exact header revision. All three previous matches remain verified by comparison scripts. Pilot SHA256 74AFAAA81EAC81847158F78CE63A539FFD1CFCC718AF2B559B240BB88F80CD0B. Artifacts archived archive/07-formatter-secure-crt. Experiment 8 stages _STRSAFE_USE_SECURE_CRT=0 only in formatter TU, unchanged SDK implementation/compiler flags/caller context. Build pending. No Ghidra changes or fresh saved-state check.

## Formatter matching milestone (2026-11-01, experiment 8)
Live verified PE-byte comparison: original fsx.exe formatter 0x01001590..0x010015eb and rebuilt SDK StringCchPrintfA 0x00401210..0x0040126b match over 92 bytes after mapping exactly one _vsnprintf IAT operand at function offset 0x34 (4 bytes). Original operand 0x01001208; rebuilt MAP resolves __imp___vsnprintf at 0x004020a8. Four raw bytes differ, zero differences after verified reference mapping. No other bytes/constants/branches masked. Script: scripts/compare-formatter-pilot.ps1. Pilot SHA256 0A0B0516495EC7DCE1235A4D2058BBF4F7693D792074E929B1A693C243AD5964. Installed SDK strsafe.h with _STRSAFE_USE_SECURE_CRT=0 and established GL/LTCG flags generated the EDI/EAX/stack ABI without custom source assembly. Strong support for StrSafe formatter provenance; original header revision, exact source spelling and ANSI Cch/Cb entry identity remain unproven. This does not uniquely prove original macro use: an older header could produce the same legacy CRT branch. Existing Ghidra name/model preserved.
Regression checks: two version helpers retain normalized exact matches; copy worker remains raw exact over 72 bytes. Four bodies total 452 bytes (144+144+72+92), not whole-file equality or independent runtime proof. Archived source/batch/EXE/PDB/MAP/header/reports: vm-share/modules/fsx/archive/08-four-matches. No Ghidra edits/navigation/save or fresh database-state query this turn. Next substantial candidate: ShowErrorMessage, requiring typed MSGBOXPARAMSA and callback ABI/caller review.

## ShowErrorMessage reconstruction, experiment 9 (2026-11-01)
Live verified fsx.exe 0x01001d30..0x01001ee4: 437 bytes including full RET 0x18 encoding (earlier ending at 0x01001ee2 named only the RET start). Current stored prototype has six parameters, __stdcall, undefined return; this supersedes earlier zero-parameter stored-prototype reports. Actual EAX result loads the DWORD thread result. Current copy helper name observed as StringCopyWorkerA_1024_optimized (existing user state, not agent rename). MSGBOXPARAMSA remains fragmented in decompilation with propagation warnings. Owner overwrite and unchecked wait/query statuses still verified. Revision 1793554472087:8eb5e96a:5 saved; only fsx.exe open/current; no Ghidra edits/navigation/save.
Added src/fsx/show_error_message.cpp using real MFC AfxGetModuleState, MSGBOXPARAMSA, legacy-CRT SDK StrSafe calls and observed branch/thread flow. Argument 2 modeled ULONG_PTR as provisional generic machine word, caption nonnull by observed dereference, icon WORD, callback DWORD __stdcall return. API.DLL callback callers still not inspected, so do not apply this entire prototype blindly in Ghidra. Suggested names for existing params: param_2 -> messageArgument (provisional dual role), param_3 -> caption, param_4 -> messageBoxStyle, param_5 -> iconResourceId, hStringRessourceModule -> hStringResourceModule (spelling). Suggestions only.
Pilot minimum ApplicationPilot vtable models slot +0x0c with virtual GetWindowHandle; other slots explicitly unknown placeholders, not original class declarations. Separate non-GL application_interface_pilot.cpp prevents constant folding of the NULL-initialized getter. Comparison-only /EXPORT alias keeps ShowErrorMessage externally callable with six stack args under LTCG; not an original EXE export claim. Main does not execute ShowErrorMessage; this build is for static comparison. MFC headers supply actual module-state layout instead of embedding a guessed +8 struct. Established flags retained with _AFXDLL for MFC. Four prior matches are regression targets; shared SDK/LTCG context may change placement or code and must be rechecked.
Staged vm-share/input/09-show-error and BUILD.bat, outputs expected results/09-show-error. Guest compile/link and function comparison pending; no new match claimed. Four-function baseline preserved in archive/08-four-matches.

2026-11-01 experiment 9 failed at link: export alias expected _ShowErrorMessage@24, but reconstructed definition had C++ linkage. LNK2001/LNK1120; no new EXE/PDB or regression comparison available. This is pilot export setup, not original ABI evidence. Archived failure/source/batch in archive/09-show-error-export-failure. Experiment 10 adds extern C to definition solely to agree with comparison export alias, preserving DWORD __stdcall and six arguments. C linkage is pilot scaffolding, not a claim about original source. /EHsc warning in included MFC inline code noted; no exception-dependent behavior validated. Guest rebuild pending; no Ghidra edits/state check.

2026-11-01 experiment 10 builds successfully; ShowErrorMessage at 0x004012d0..0x0040148b is 444 bytes vs original 437. Original and rebuilt prologue/0x63c frame, message/resource buffer positions, MFC module-state +8 access, vtable window call and thread wait/query/close flow agree in reviewed assembly. Reconstructed resource branch duplicates HRESULT failure check, while original joins formatting paths before one check; this accounts for extra control-flow instructions and shifts later code. Final hwndOwner/style store order also differs. No exact ShowErrorMessage match claimed. All four previous matches retained by scripts, reports archived with build/source in archive/10-show-error-first-build. Pilot SHA256 A62AA8384AC29D677C3EAA46B09438E43B7A108C9DE78A8D89EA74A33CF4FBFD.
Experiment 11 stages shared HRESULT formatStatus check and direct-copy branch jumping past it. This source goto reproduces observed flow but is not evidence that original source contained goto. Final owner/style source order unchanged to isolate control-flow adjustment. Guest build pending. No Ghidra edits or fresh save-state query.

2026-11-01 experiment 11 verified: ShowErrorMessage at 0x004012d0..0x00401484 is now 437 bytes, equal to original size. Common HRESULT check recovered original branch arrangement. Remaining reviewed non-address differences: CMP EAX,EBX at rebuilt offset 0xdb vs original TEST EAX,EAX (EBX is known zero), and final hwndOwner/dwStyle stores reversed. These are not masked or declared matching. Four earlier helpers retain exact comparisons (version/formatter address-normalized, copy raw). Pilot SHA256 3D6A38B444ACEDBC43E80A7FD9DB16027DCBC8189C731E35C3701D1F9C576B70. Archived experiment 11. Experiment 12 changes only source order: hwndOwner=NULL before final dwStyle expression. Guest build pending; no Ghidra changes or save-state query.

2026-11-01 experiment 12 verified: ShowErrorMessage remains 437 bytes; final hwndOwner then dwStyle store order now agrees with original. Reviewed remaining instruction difference: CMP EAX,EBX vs original TEST EAX,EAX at offset 0xdb, with EBX zero. No exact normalized ShowErrorMessage claim until full mapped-reference comparison. All four prior helper comparisons remain exact. Pilot SHA256 267A0085830EF556BE249C4AA3B2FD9C6159868C665C15E0A4795D03B42BF9F2; archived archive/12-show-error-store-order. Experiment 13 replaces FAILED(formatStatus) with explicit unsigned HRESULT high-bit test, preserving semantics and avoiding assembly patches. This is a source formulation experiment, not recovered original syntax. Guest run pending; no Ghidra edits/state query.

2026-11-01 experiment 13 verified by new scripts/compare-show-error.ps1: full 437-byte body with 16 address fields independently mapped (MAP symbols, internal copy-worker symbol, fallback string content). Exactly one non-address byte differs at offset 0xde: original 0x8c JL versus rebuilt 0x88 JS after matching TEST EAX,EAX. The sign-bit source expression recovered TEST but changed branch choice; no exact match claimed. All four helper comparisons remain exact in their stated scopes. Archived archive/13-show-error-one-byte-difference with reports/source/build. Pilot SHA256 09B10780C8FC230104879F6BB5982F7C0470501FA1FF54768EF60A27663F69B4. Experiment 14 uses int formatStatus and signed <0, rather than HRESULT(long) FAILED macro or unsigned bit test; original source type not proven. Build pending, no Ghidra edits/state query.

2026-11-01 experiment 14: full 437-byte mapped comparison has differences only at offsets0xdb/0xdc (CMP EAX,EBX vs original TEST EAX,EAX). Signed-int source recovered JL but not TEST; experiment13 remains one-byte-nearest. Four helper matches retained. Pilot SHA256 60FDA64B80ECD810A30019DA7401FA49F33F56DDC941B9DB634D7B0042453E6E; reports/source/build archived archive/14-show-error-signed-int. Experiment15 changes early-return structure into formatStatus>=0 display block with shared return; copy branch still enters display without checking HRESULT. Source structure is experimental, not recovered original syntax. Build pending; no Ghidra changes/state query.

2026-11-01 experiment15 verified: successful-format block gives the same compared instructions as experiment14. Full 437-byte ShowErrorMessage comparison with 16 validated reference mappings leaves only offsets0xdb/0xdc, CMP EAX,EBX instead of original TEST EAX,EAX. EBX is initialized zero and remains preserved along both formatter paths; comparison and branch have the same relevant signed-less-than result here, but this is not an exact instruction match or independent runtime equivalence proof. Original TEST clears CF/OF, so do not claim complete CPU-flag equality. Both versions use JL; experiment13 used TEST/JS and had a one-byte encoding difference. Four helper bodies remain matching in established raw/normalized scopes. No further expression-only candidate is currently evidence-backed; investigate actual translation-unit/compiler context before expanding speculative variants. Pilot SHA256 4C2B371AA97E59A12A10AD0531AD0ABA298E2A5807E8CCB28213729A48ECFBBF. Reports/source/commands archived archive/15-show-error-near-match; no additional guest build requested. No Ghidra edits or fresh save-state check.

## Completed Ghidra transfer (2026-11-01)
Program fsx.exe only; no program navigation. Live pre-edit state revision1793554472087:8eb5e96a:27 was already unsaved. Applied and fresh-decompiler readback verified:
- ShowErrorMessage 0x01001d30 now stored DWORD __stdcall return, UINT errorId, uint messageArgument, char* caption, DWORD messageBoxStyle, preserved WORD iconResourceId, HINSTANCE hStringResourceModule. Argument2 remains deliberately generic/provisional pending API.DLL callback callers. const-char parsing was rejected by MCP; char* is the stored model, not evidence that original code modifies caption.
- Replaced ten raw stack fields at [-0x63c,-0x614) with existing /winuser.h/MSGBOXPARAMSA (40 bytes); separate DWORD dialogResult at -0x640. Preserved messageText[1024]/resourceText[512]. Cookie and saved caption locals named securityCookie/captionText. Register locals now formatStatus, applicationObject, applicationWindow, moduleState, hMessageBoxThread.
- StringCopyWorkerA_1024_optimized 0x01001600 now HRESULT return explicitly in EAX; custom-storage destination ECX/source EAX, no varargs/no capacity parameter. Convention label __cdecl is not standard stack cdecl ABI. Fresh caller now displays both arguments. Locals writePosition/remainingCapacity applied.
- Formatter 0x01001590 retains existing __fsx_edi_eax_varargs and storage/varargs. Applied maxFormattedChars, formattedLength and formatStatus names without speculative signedness retype.
- Added English recompilation evidence/limitations plate comments at 0x01001760, 0x01001800, 0x01001590, 0x01001600, 0x01001d30, preserving existing plate text. Four exact-scope matches and ShowErrorMessage near-match explicitly distinguished. Existing function names/types/vtable definitions otherwise preserved.
MCP individual rename endpoint partially applied changes despite returning failure and initially associated application name with module-state register reuse. Corrected via fresh high-symbol type-aware script; final readback verifies applicationObject versus moduleState. Copy return needed explicit EAX storage under custom model; final decompilation now renders HRESULT. Remaining ShowErrorMessage warning: Exceeded maximum restarts with more pending (and expected cookie injection notice). Prior type-propagation-not-settling warning is absent in final readback, but do not claim all decompiler issues solved. No instruction patches. Database saved through save_program after verification; no independent backup created.

2026-11-01 experiment16 staged at user request: remove source goto/label from ShowErrorMessage; direct-copy branch ignores helper return and sets local formatStatus=0, then all paths use common success block. This is semantically equivalent for observed return/formatting/display flow; source variable assignment is compiler input, not a recovered original statement. Compiler/linker flags and callback export scaffolding unchanged to isolate source control flow. Four-match and near-match archives retained; outputs results/16-show-error-no-goto pending guest build. No Ghidra edits or state query. If this leaves encoding unchanged, next evidence-based axis is real callback caller context rather than further expression-only variants.

2026-11-01 experiment16 verified: no-source-goto variant still differs only at offsets0xdb/0xdc after16 validated mappings; four helper matches retained. Pilot SHA256 ADA22BB66333D454322E5FACEDE1F6F9CDEFC34CED98DF33216E346B701A140B. Archived archive/16-show-error-no-goto; source goto is not sufficient cause of observed encoding difference in tested setup.
Live original caller recheck: fsx.exe FSX_InitInstance at0x01001c65 PUSH 0x01001d30 after obtaining Application and vtable slot0, confirming callback address escapes to API.DLL boundary. Only fsx.exe open/current, revision1793559605675:8eb5e96a:275 saved. Experiment17 removes pilot /EXPORT and passes ShowErrorMessage pointer from main to non-GL RetainErrorCallbackPilot, which stores it in a volatile function pointer. This models opaque escape only, NOT original full Initialize call/registration behavior. No callback execution or claim about original source syntax. Source callback prototype keeps provisional argument2 machine word. Compiler settings and no-goto function body unchanged. Guest build pending; no Ghidra changes/save.

2026-11-01 experiment17 verified: opaque non-GL callback retention without pilot export preserves ShowErrorMessage at437 bytes, rebuilt0x004012e0, still only offsets0xdb/0xdc differ after16 target mappings (CMP EAX,EBX vs TEST EAX,EAX). Four helper matches retained at moved addresses (copy0x004011c0, formatter0x00401220, version helpers unchanged). Pilot SHA256 69CD14BA043680D83AC819C6216B815AFCD0783AE94DCF04914E5546984F010F. Full reports/source/batch/header archived archive/17-callback-escape-near-match. Experiment16/17 do not establish source goto or artificial export as causes in tested configurations. They do not rule out broader original caller/TU/LTCG context differences. No new exact-match claim, runtime equivalence test, Ghidra edits/save or database-state query. Next evidence-based axes: separate VC8 compiler revision candidate with pinned current baseline, or actual FSX_InitInstance reconstruction; do not silently overwrite RTM toolchain with a service pack.

2026-11-01 experiment18 staged after user-reported SP1 installation/restart: exact source snapshot from archive/17-callback-escape-near-match copied to input/18-sp1-same-source and hash equality checked per source/header file. Same compiler/linker switches; fresh guest work directory prevents RTM object reuse. Installed compiler revision must still be established by guest build banner, and SDK/MFC libraries may also have been updated by SP1. This compares complete installed SP1 toolchain, not compiler-only isolation. Outputs results/18-sp1-same-source pending guest run. RTM baseline retained; no Ghidra changes/state query.

2026-11-01 experiment18 SP1 comparison verified: cl14.00.50727.762 banner and successful fresh link. Source hash-identical to experiment17 and flags unchanged. Both144-byte version helpers normalized exact,72-byte copy worker raw exact,92-byte formatter normalized exact. ShowErrorMessage437 bytes at0x004012e0 retains differences only0xdb/0xdc after16 independently checked mappings, CMP EAX,EBX versus original TEST EAX,EAX. SP1 does not resolve remaining encoding in tested context; neither RTM nor SP1 is uniquely established as original compiler by these results. SDK/MFC patch changes are included in toolchain comparison; no compiler-only isolation claim. Pilot SHA256 2FF44DDC9B874F50885CB954A3319E78E8C8125AB0ECC903F4F20A8463EAB43A. Complete reports/source/header/commands archived archive/18-sp1-four-matches-near-show-error. RTM baseline remains archive/17-callback-escape-near-match. No Ghidra edits/save or fresh saved-state check. No additional build staged; broader actual caller/TU context remains next evidence-based investigation.

## Experiment19: original startup caller/TU context (2026-11-01)
Read-only live fsx.exe FSX_InitInstance0x010019e0 decompilation (769-byte body) and InitializeOleAndMessageFilter0x010018e0 rechecked. fsx.exe only open/current, revision1793559605675:8eb5e96a:275 saved. Added startup_context_pilot.cpp with actual CWinApp-derived InitInstance override, language/OS/D3DX/path/manifest/API-loading branches, Initialize virtual call slot0 with instance/commandline/1/ShowErrorMessage/event, success Attach path and direct error callback. Minimum ApplicationPilot slot0 updated from unknown placeholder to reviewed Initialize signature; slots4/8 remain unknown. No api.dll program opened and target signatures not independently reverified; argument2 callback remains generic machine word.
Caller and ShowErrorMessage plus version helpers compiled in same TU via source includes, replacing their separate objects. This is experimental packaging, not recovered original file boundaries. CWinApp/CWnd globals keep override reachable through vtable and form real MFC compiler context; no startup function export/opaque retention call is used. Existing isolated formatter/copy pilots remain for regressions. Compiler SP1 and flags retained. Original source array declarations/initialization timing are not proven; this is a flow reconstruction, NOT an InitInstance match claim.
Explicit limitation: original237-byte COM/OLE/filter helper not reconstructed. Non-GL InitializeOleAndMessageFilterPilot returns FALSE as test boundary; callback code cannot assume result at LTCG. Constructor only initializes event handle NULL and does not recreate original CreateEvent logic. Binary remains comparison-only; user is asked to build, not run it as FSX. Real MFC headers determine CWinApp layout; reconstructed minimum Ghidra layout is not imposed as original complete size. Shared TU may inline/move helpers, so previous fixed-size comparison assumptions must be verified against MAP/disassembly before interpreting reports. Staged input/19-initinstance-shared-tu, results pending. No Ghidra edits/navigation/save.

2026-11-01 experiment19 failed compilation: C3861 AfxEnableControlContainer identifier not found in startup_context_pilot.cpp; missing afxdisp.h, not original-code evidence. Only error reported; no fresh linked output or matching claim. Failure/source/batch archived archive/19-startup-missing-header. Experiment20 adds afxdisp.h per MFC declaration requirement (Microsoft OLE Initialization reference); no logic/flags changes. Fresh input/work/output staged, guest run required. No Ghidra changes/state query.

2026-11-01 experiment20 verified: successful SP1 build with actual CWinApp-derived startup caller/shared TU. ShowErrorMessage remains437 bytes at0x00401530; full16-reference-mapped comparison differs only0xdb/0xdc (CMP EAX,EBX vs original TEST EAX,EAX). All four helpers retained: version0x00401030/0x004010c0 normalized exact144 bytes each; formatter0x00401480 normalized exact92; copy0x004014e0 raw exact72. No target helper was inlined away in this build. Pilot SHA256147BF73D1056BFFE9776AB0055A6839240B8580C6ABFF2AB7D5E889E7F51DB1B. Complete reports/source/batch/header archived archive/20-startup-context-near-match.
InitInstance pilot0x00401180..0x00401475 has758 bytes excluding alignment padding, original0x010019e0 has769 bytes. Pilot frame reserves0x564 vs original documented0x554. No matching claim for InitInstance. Original OLE/filter implementation and event constructor are still test placeholders; source declaration/init order and original translation-unit contents are unknown. Therefore tested caller/TU context did not resolve encoding but full original context is not excluded. No new build staged. No Ghidra edits/save or fresh saved-state query in this review.

2026-11-01 experiment21 staged: live original event initializer0x01001660 (95 bytes) and scalar deleting destructor0x010016d0 (70 bytes) assembly/decompilation reviewed in fsx.exe. CreateEventA(NULL,FALSE,FALSE,FS98MAIN), GetLastError()==ERROR_ALREADY_EXISTS closes handle and sets NULL; destructor closes nonnull event before base destruction, optional operator delete is compiler deleting-destructor scaffolding. Added corresponding ordinary C++ constructor/destructor to FSXAppPilot; not a matched constructor/destructor claim. No source-level manual vtable or deleting flag logic introduced. Original0xa4 handle offset remains separate layout evidence; actual installed CWinApp determines pilot offset.
Experiment21 also captures installed afxole.h/afxole.inl/afxwin.h/afxwin2.inl after successful guest build to resolve COleMessageFilter layout/member access before replacing opaque OLE/filter stub. No guessed filter field names or derived virtual overrides applied. Stub still returns failure. Only fsx.exe open/current; live revision1793559605675:8eb5e96a:275 saved; no Ghidra changes/navigation/save. Inputs21/startup work/result paths fresh; comparisons pending guest run.

## InitInstance matching follow-up (2026-11-02)
See initinstance-matching.md. Earlier command-line argument interpretation is superseded at original0x01001c5a: reads this+0x4c, whereas installed MFC/compiler emits m_lpCmdLine at+0x48 and m_nCmdShow at+0x4c. Canonical reconstruction uses generic word startupArgument with m_nCmdShow; actual API.DLL target signature remains unverified. No Ghidra edits applied. Final tested InitInstance frame0x554 matches original; body777 bytes still differs from original769. Buffer capacity declarations are reconstructed hypotheses supported by3/256 call capacities and generated frame, not original source proof.


2026-11-02 fresh class/override audit: original ctor-like0x01001660 calls CWinApp constructor via0x0100202c/IAT0x0100117c ordinal566, installs vtable0x01001398 on global0x010043e0. Slot+0x58 at0x010013f0 contains0x010019e0. Real SDK-based rebuilt vtable+0x58 also points to rebuilt InitInstance. Strong evidence for current CWinApp-derived override model, not original FSXApp name/complete size. No Ghidra edits. Full encoded block analysis in initinstance-matching.md explains all8 additional bytes through branch layout; remaining non-branch content differs despite equal lengths.

## Application lifecycle milestone (2026-11-02)
See app-lifecycle-matching.md. Constructor0x01001660 (95), deleting destructor0x010016d0 (70), and newly reconstructed ExitInstance0x01001720 (58) are normalized-exact against fresh SP1 run lifecycle-20261102-003 with22 validated address fields in total. ExitInstance at application vtable+0x70 detaches/clears main-window pointer, calls Application slot+4 if getter exists, then tail-calls CWinApp::ExitInstance. Original1720 has instructions but no Ghidra function; creating/naming it is suggested only. Existing application/global names remain reconstructed. Full vtable/RTTI/external bodies excluded; golden archive24. No Ghidra edits.

## EXE virtual-function inventory (2026-11-02)
See exe-vtable-inventory.md and artifacts/fsx.exe/vtables/application-vtable.html. Observed48 application slots:40 MFC imports,3 SDK leaf entries,5 own entries. Current source still lacks GetMessageMap(+0x30,01001650) and Run(+0x5c,01001d00); actual data/body evidence recorded, not yet reconstructed. Next work remains in EXE. Original Serialize/Dump target-sharing suggests a controlled folding experiment later; original linker flag unproven. No Ghidra edits.

2026-11-02 missing-override follow-up: GetMessageMap01001650 (6), Run01001d00 (35), and callback01001cf0 (5) now normalized-exact via actual SDK macros/ordinary C++. Empty message map and MFC imports validated, baseline25. Prior missing-override inventory superseded for current source; only InitInstance remains nonmatching among the five own application vtable entries. Source API slot+8 contract corrected from independent DLL evidence. No Ghidra changes; see app-dispatch-matching.md.

2026-11-02 static initialization follow-up: see static-initialization-matching.md.
Original XCU table proves CWnd construction before application construction;
canonical declarations now produce that order. Five complete wrapper bodies/113
bytes match with18 validated references, all13 earlier body gates retained
(total18/1114 at their documented scopes). Golden29-static-initialization-matches;
new mandatory MfcWindows gate and StaticInitialization target. Full C initializer
table still differs: rebuilt XIY registers ___CxxSetUnhandledExceptionFilter,
absent from original checked table. InitInstance/ShowErrorMessage remain open.
Ghidra: five bounded functions and CRT table comments applied, pre-edit GZF backup
exported, fsx.exe saved (1793623738938:8bd7bef0:69, unsaved_changes=false).
Current inventory195 model functions=63 internal+132 external,145 unassigned
instructions/63 ranges,130 mixed rows,15 nonzero undefined ranges. Two additional
stored import thunks observed after changes; not new matched source bodies.
No API edits/tab navigation/candidate execution. Worker ready, no active run.
Next: investigate extra XIY CRT initializer from library/exception context before
blind flag changes; then remaining runtime ranges or API behavior.

2026-11-02 CRT context resolved: see crt-library-context.md. Installed SP1
msvcrt.lib crtexew.obj owns .CRT$XIY's DIR32 reference to
___CxxSetUnhandledExceptionFilter; unhandld.obj supplies the routine. Extracted
release x86 RTM msvcrt.lib from the existing VS2005 DVD CAB without installation
or VM/snapshot changes. SP1 compiler/linker and installed MFC/headers retained.
Explicit MFC libraries must precede the RTM CRT; first incorrect-order experiment
was rejected by allocation-import gates and remains baseline_failed. Corrected
source-identical build passes all18 existing body matches/1114 bytes and both C
and C++ initializer table roles/order. This supersedes the open XIY discrepancy
for the selected default, not full CRT-body/original-compiler identification.
CrtLibrary Auto now selects RTM for MfcWindows, InstalledSP1 for ConsolePilot;
InstalledSP1 explicit control retained. RTM library SHA256 pinned B1456D9A7E17865E3DC35663E0377BE4C7B8BE97A372210CD064F9F5635DFFEA.
Fresh default crt-default-20261102-001 passes; golden30-rtm-crt-context SHA256
D8973AC14E07DA44C01F0FA4AE207023E290361C2B2125460BB57FF902E17A7B.
InitInstance777/769,246/frame0x554/43 positional differences and ShowErrorMessage
2 bytes unchanged. Updated two existing Ghidra table comments and saved fsx.exe,
readback1793624696372:8bd7bef0:72 unsaved_changes=false. No API edits/navigation,
function creation/type changes, or candidate execution. Inventory counts unchanged;
default library/inventory/replay references30. Worker ready; no active run.
Next: inspect actual pre_c_init/runtime bodies against identified counterparts
before defining undecoded functions, or continue API behavior; no blind variants.

2026-11-02 pre_c_init completed: see pre-c-init.md. Live original0x01002547..01002628
(226 bytes/56 instructions) normalized-exact with27 validated fields against
baseline30 _pre_c_init from msvcrt:crtexew.obj. Three raw PE dependencies checked:
RTC walker36, FP precision41, shared setargv/matherr zero stub3. Same body also
matches SP1 control29, so it does not distinguish original compiler revision.
New mandatory MfcWindows PreCInit gate; real worker pre-c-init-20261102-001 passes
all18 existing application/helper/wrapper body gates. Those counts remain18/1114;
CRT library identification is reported separately. No C++ source/flags changed.
Pre-edit GZF exported, bounded disassembly/function int __cdecl pre_c_init(void)
and comment applied; saved readback1793625481881:8bd7bef0:291 unsaved_changes=false.
No API edits/navigation/candidate execution. Follow-on model contains three
one-byte FUN_* dependency placeholders, not complete stored dependency bodies;
full raw PE matches are separate evidence. Current model202=67 internal+135
external;135 mixed inventory rows;4857 code/3150 data/1721 undefined bytes,
14 nonzero undefined ranges. Baseline30 retained and its matching report extended.
Negative PUSH2->3 control rejected; mutated copy deleted. Worker ready/no active job.
Suggestions only: DAT_01004030 -> g_crtManagedImageFlag; puVar1 -> pCrtModeValue.
Next: complete three bounded dependency definitions/alias documentation, then
pre_cpp_init or API behavior. No blind near-match variants.

2026-11-02 pre_cpp_init/helper completion: see pre-cpp-init-and-exe-gaps.md and
exe-matching-roadmap.md. Original0x01002313 complete75 bytes normalized-exact
with12 validated reference fields; RTC termination0x01002802 complete36 verified.
Mandatory PreCppInit MfcWindows gate added; real pre-cpp-init-20261102-001 passes
all earlier18 body gates plus CRT gates. Negative _amsg_exit8->9 control rejected.
No canonical C++/flags/library changes; baseline30 retained with extended report.
Ghidra pre-edit GZF exported; bounded full definitions/comments/signatures applied
for pre_cpp_init, _RTC_Initialize, _setdefaultprecision, CrtReturnZeroFallback,
_RTC_Terminate. Three prior one-byte helper placeholders superseded. Approved
DAT_01004030 -> g_crtManagedImageFlag applied exactly, existing undefined4 type
preserved. Readback saved1793626307309:8bd7bef0:445, unsaved_changes=false.
No API edits/navigation/candidate execution. New follow-on one-byte placeholders
at known import thunks2be0/2bec remain incomplete; raw identities already verified.
Current model206=71 internal+135 external;110 unassigned instructions/61 ranges,
134 mixed inventory rows;4934 code/3158 data/1636 undefined bytes,12 nonzero
undefined ranges. These are model counts, not matching percentages. Worker ready.
Remaining scope includes full CRT/security/SEH/stack/ATL bodies, data/RTTI/EH,
import/PE/linker layout, and explicit InitInstance/ShowErrorMessage nonmatches.
Next bounded block: full CRT entry/main startup and undecoded gaps near24dd/2529.

2026-11-02 full CRT startup region milestone: native Ghidra access recovered after
user restart. Explicit fsx.exe calls with api.dll user-selected; no tab switching.
Complete region01002363..01002541 (479 bytes/157 instructions) normalized-exact
with39 validated address fields, including54 previously undecoded EH4 bytes.
40-byte scope table/four target pointers, PE entry10, cookie init148, SEH prologue69
and raw epilogue20 plus MFC entry wrapper checked. Region is NOT a single stored
479-byte function:425-byte normal-flow parent ownership deliberately preserved.
New mandatory CrtStartup MfcWindows gate; actual crt-full-startup-20261102-001
passes all earlier18 application/helper/wrapper and CRT gates. SP1 control also
passes this region; no unique compiler identification. Canonical source/baseline30
unchanged; known InitInstance/ShowErrorMessage nonmatches remain open.
Ghidra pre-edit GZF exported;54 listing bytes filled, four embedded-EH labels and
scope refs/comments added; parent typed int __cdecl. Saved readback
1793646807668:5e7b737f:88 unsaved_changes=false. No API edits or candidate execution.
New _XcptFilter import-thunk one-byte placeholder observed, full stored definition
still pending. Inventory207=72 internal+135 external;136 mixed rows;131 unassigned
instructions/63 ranges;4988 code/3158 data/1582 undefined bytes,10 nonzero ranges.
Negative body-literal and EH-pointer controls rejected; temporary copies deleted.
Unneeded denied socket-fallback helper/request/schema scratch removed; native tools
used for actual edits. Worker ready/no job active. See full-crt-startup.md.
Suggestions only: Exchange->startupLockOwner; pvVar2->previousStartupLockOwner;
local_24->commandLineTail. Next: exception-dispatch wrapper and PE image helpers,
then remaining security/stack/ATL and full data/linker layout.

2026-11-02 CRT image/dispatch milestone: see crt-image-helpers.md. PE validation41
and section lookup66 raw-exact; nonwritable full region108 normalized-exact with6
fields, embedded EH23 and scope28/two pointers checked; dispatch35 with3 fields
and cookie checker15 with2 fields normalized-exact. Imported MSVCR80 dispatcher
and failure-reporter body remain outside this report. New mandatory CrtImageHelpers
gate; actual crt-image-helpers-20261102-001 retains all prior18/CRT gates. No C++,
flags/library/baseline30 change; known application nonmatches remain open.
Ghidra pre-edit GZF exported; defined dispatch35 with four-pointer minimum ABI,
filled23 EH listing bytes, preserved85-byte parent/prototypes, added two labels/
scope refs and evidence comments. Corrected false uninitialized pImageBase by
analysis-only inline on the verified41-byte leaf: actual EDX is preserved. This
is NOT original compiler/source inlining evidence or any byte/prototype change.
Saved readback1793647354097:5e7b737f:171 unsaved_changes=false. api.dll stays selected,
no tab navigation/API edits/candidate execution. Inventory208=73 internal+135
external;137 mixed rows,5011 code/3158 data/1559 undefined bytes,9 nonzero ranges.
Negative MZ-literal/EH-pointer controls rejected; altered copies deleted. Worker
ready/no active job. Suggestion only p_Var1->sectionHeader (PE Characteristics use).
Next: report_gsfailure and stack probes, then onexit/ATL and data/linker layout.

2026-11-02 GS failure/stack milestone: see crt-failure-stack.md. Reporter0x01002a6d
260 bytes normalized-exact with33 validated references, standard x86 CONTEXT716/
EXCEPTION_RECORD80 initial data and EXCEPTION_POINTERS links checked. Stack probes
22/22 normalized-exact plus43 raw-exact. New mandatory CrtFailureStack gate; actual
crt-failure-stack-20261102-001 passes all prior18/CRT gates. No source/flag/library
or baseline30 change; application nonmatches unchanged. Negative page-step/data-
pointer controls rejected and copies deleted. Ghidra backup exported; applied
existing Windows structures80/716/8, added data-role/reporter/special-ABI comments.
No user-defined symbols overwritten; automatic labels now reflect data types.
Existing function prototypes/injections/code bytes preserved; no guessed C ABI
for EAX/ESP stack helpers. Saved readback1793647925456:5e7b737f:196 unsaved=false.
api.dll still selected; no navigation/API edits/candidate execution. Inventory208
=73 internal+135 external;137 mixed rows;5011 code/3162 data/1555 undefined bytes,
8 nonzero undefined ranges. Worker ready/no active job. Data-name suggestions only
listed in topic file. Next: __onexit registration/locking/EH, then ATL and layout.

2026-11-02 exit-registration milestone: see crt-onexit.md. Complete __onexit150,
lock cleanup9 and atexit adapter18 normalized-exact with14/1/1 validated fields;
finally table28 with same cleanup target and lock8/import/global relationships
checked. New CrtOnexit gate; actual crt-onexit-20261102-001 retains all prior18/CRT
gates. No source/flags/library/baseline30 change; known nonmatches unchanged.
Negative lock-number/finally-pointer controls rejected; modified copies deleted.
Ghidra pre-edit GZF exported; existing __onexit name preserved and standard
/stdlib.h/_onexit_t reused for callback argument/return. FUN26e7 renamed
ReleaseCrtExitLock, FUN26f5 renamed atexit; separate void callback definition
/FSX/CRT/CrtAtexitCallback, comments and finally-table reference applied.
Text prototype parser failed on callbacks; datatype-backed Java script succeeded.
Readback saved1793648448204:5e7b737f:247 unsaved=false. Encoded global types/code
bytes/bounds preserved. api.dll still selected, no navigation/API edits/execution.
Inventory208=73 internal+135 external,137 mixed rows,8 nonzero undefined ranges;
worker ready/no active job. Suggestions only: p_Var1->registrationResult and encoded
begin/end global descriptions in topic file. Next: ATL thread-codepage helpers,
remaining runtime fragments, then data/RTTI/linker layout.

2026-11-02 ATL milestone: see atl-codepage.md. Fallback101 and selecting thunk100
normalized-exact with5/8 fields; real getter01001f89 raw-exact4 bytes returns3.
Actual selector target plus atls:atltrace.obj symbol resolve the prior fragment
hypothesis; now stored as ATL::_AtlGetThreadACPReal, uint stdcall(void). Cached
getter bootstrap/alias uses/imports checked. New AtlCodepage gate; real
atl-codepage-20261102-001 retains all prior18/CRT gates. No source/flags/library
or baseline30 change; application nonmatches unchanged. Two negative controls
rejected, copies deleted. Original5-byte CC gaps after fallback/real differ from
rebuilt0/0: concrete layout difference. /FUNCTIONPADMIN:5 is a hypothesis supported
as a valid option by local VC8 help; not yet enabled/tested or claimed original.
Ghidra pre-edit GZF exported; real getter defined in existing ATL namespace,
generic pointer refined via /FSX/ATL/ThreadCodePageGetter and default label
replaced with ATL::g_pfnGetThreadACP. Other names/signatures/OSVERSIONINFOA local
and code preserved. Readback saved1793649165631:5e7b737f:275 unsaved=false. api.dll
still selected, no navigation/API edits/execution. Inventory209=74 internal+135
external;137 mixed rows,128 unassigned instructions/62 ranges;8 nonzero undefined
ranges. Worker ready/no active job. Suggestions only Value->selectedGetter and
local_9c->versionInfo. Next classify remaining runtime/data ranges, then isolated
padding test and data/RTTI/section/linker reconstruction.

## RTM callback comparison, 2026-11-18

Live verified for fsx.exe ShowErrorMessage at 0x01001d30 through 0x01001ee4.
The linked VC8 RTM callback at 0x01001d10 contains 437 bytes and 135 instructions.
The 16 mapped address fields leave one unequal byte at offset 0xde. Original
0x01001e0d uses JL (0F 8C), while the rebuild uses JS (0F 88). The preceding
TEST EAX,EAX clears OF, so both conditions select the same error path for every
EAX value. The compiler revision used by the original remains open.
The reconstructed ErrorFormatString owns the localized format buffer. The callback
retains the six-argument stdcall ABI and bypasses the formatting check after
direct copying. This replaces the previous canonical two-byte residual and
reproduces the one-byte historical experiment using structured C++.
The RTM integration build also passed the 769-byte InitInstance comparison.
Runtime execution remains untested. Read-only Ghidra readback reported saved
revision 1793671788023:3045b49a:3.
[Comparison](../../../artifacts/fsx.exe/show-error-20261117/comparison.json)
and [trial ledger](../../../artifacts/fsx.exe/show-error-20261117/trials.json).

### Follow-up on the branch opcode

Live verified in `fsx.exe` at `0x01001e0b` and `0x01001e0d` within
`ShowErrorMessage` at `0x01001d30`. The new Ghidra MCP instruction listing
agrees with the retained 437-byte original listing. Ghidra reported saved
revision `1793671788023:2514814d:3` on readback.

The signed HRESULT success check produces the original `JL` in VC8 RTM,
but emits `CMP EAX,EBX` in place of `TEST EAX,EAX` at callback offset
`0xdb`, leaving two different bytes. Equivalent early-return, operand-order
and widened signed checks reproduced that result. The tested optimization
pragmas and other predicates failed the function boundary or established
regression checks. Both requests for the installed SP1 toolset were
rejected by the compiler identity check because the guest reported
`14.00.50727.42` (RTM).

The canonical C++ source retains the best result. Its fresh RTM replay
validated all 16 address fields and left offset `0xde` as the sole
difference. `InitInstance` also passed the complete linked-body comparison.
The original compiler setup and a source form that emits both original
instructions remain open. Runtime execution remains untested.
[Replay and trial ledger](../../../artifacts/fsx.exe/show-error-jl-20261118/trials.json).

### Pinned SP1 and control-flow trials

Live verified in `fsx.exe` at `0x01001d30` through `0x01001ee4`. The
fresh Ghidra MCP listing contains 135 instructions and agrees with the
retained inventory, whose 437 bytes were checked against the pinned original
PE. Saved revision `1793671788023:2514814d:3` was verified read-only.

The job-local `PinnedSP1` profile uses VC8 x86 `14.00.50727.762` with
the guest's installed headers and pinned RTM libraries. This supersedes
the toolchain limitation above for the pinned profile. The installed-SP1
profile in the current guest continues to identify as RTM. The VM
installation and snapshots were left intact.
[Tool provenance](../../../vm-share/toolchains/vc80-sp1/README.txt).

SP1 and RTM agree on the residual after all 16 address fields are validated.
The canonical linked callback at `0x01001d10` emits `TEST EAX,EAX; JS`,
with only offset `0xde` differing from the original. The signed HRESULT
variant gives `CMP EAX,EBX; JL`, with differences at `0xdb` and `0xdc`.
Separate format-call guards emit the original `TEST EAX,EAX; JL` after the
fallback call, but also emit a separate test and branch after the localized
call. The original shares one test block. The other tested ownership,
initialization and optimization variations yielded a larger difference or
failed a comparison gate. The canonical source retains the best result.
The pinned SP1 integration run passed the controller regression gates and
the 769-byte `InitInstance` comparison. Four raw bytes differ between the
RTM and pinned SP1 callback builds at the starts of validated address
fields. The two builds have identical normalized callback bodies.
[Pinned SP1 comparison](../../../artifacts/fsx.exe/show-error-jl-context-20261118/pinned-sp1-comparison.json)
and [trial ledger](../../../artifacts/fsx.exe/show-error-jl-context-20261118/trials.json).
