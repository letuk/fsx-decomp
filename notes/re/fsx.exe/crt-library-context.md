# CRT startup library context

2026-11-02. Status: live verified original fsx.exe table/import reads, inspected
installed CRT archive members, local installation-medium extraction, and actual
XP builds. Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Cause of the additional initializer

Original C initializer range [0x01001250,0x01001258) contains the null start and
pre_c_init 0x01002547, followed by the null end sentinel. Baseline29's SP1 CRT
also has ___CxxSetUnhandledExceptionFilter in .CRT$XIY.

Installed SP1 msvcrt.lib member build\intel\dll_obj\crtexew.obj owns the four-byte
.CRT$XIY section and its DIR32 relocation to ___CxxSetUnhandledExceptionFilter.
The routine itself comes from build\intel\dll_obj\unhandld.obj. This ownership
distinction matters: the application did not add an arbitrary callback, and
changing application exception flags does not directly remove a fixed section
of this already-compiled startup member.

Evidence: crt-table-probe-20261102-001/build-log.txt, lines318 onward for the
crtexew member, section/relocation at437/455, undefined symbol at1022. The member
inspection also catches wcrtexew.obj because the selection uses substring matching;
the actual linked narrow GUI startup is crtexew.obj, confirmed separately in MAP.
The .CRT$XIY initializer is not owned by cinitexe.obj or unhandld.obj.
Original SetUnhandledExceptionFilter/UnhandledExceptionFilter imports are still
present at IAT0x0100107c/0x01001078; missing this C-table entry does NOT imply
that the original executable never uses Windows exception filters.

Preserved RTM console baseline15 has XIA/XIAA/XIZ, without XIY; SP1 baseline18
has XIY and the filter initializer. Those historical builds have different caller
contexts and are corroboration, not a controlled original-compiler identification.
The new source-identical library experiment below supplies the stronger control.

## RTM library extracted without installation or VM rollback

Read existing C:/Users/robin/Downloads/en_vs_2005_pro_dvd.iso with installed 7-Zip.
Extracted only vs/_2025_RTL_x86_enu_CRT_Shared_Libs_msvcrt.cab and its release x86
FL_msvcrt_lib_____X86.3643236F_FC70_11D3_A536_0090278A1BB8 entry; renamed that
extracted library to msvcrt.lib. No installer, disk mount, snapshot revert, guest
UI automation, system library replacement or runtime DLL installation performed.

Preserved local artifacts:

- CAB: artifacts/shared/toolchain-media/_2025_RTL_x86_enu_CRT_Shared_Libs_msvcrt.cab,
  SHA256 6110D9D13E04B57CB50DFCE72F86062C0F2587BEA0DB03EC024B25B9ABC43C50.
- Library: vm-share/toolchains/vc80-rtm/msvcrt.lib, 939484 bytes,
  SHA256 B1456D9A7E17865E3DC35663E0377BE4C7B8BE97A372210CD064F9F5635DFFEA.

Compiler and linker remain installed SP1 14.00.50727.762 / 8.00.50727.762.
Installed headers and MFC libraries remain SP1. This is a controlled RTM CRT
static/import-library selection, not a switch of the entire toolchain or proof
of the version of an original runtime DLL. It does not establish that the original
compiler itself was RTM or that all original libraries are now identified.

## Library precedence and outcomes

First experiment crt-rtm-library-20261102-001 compiled successfully but regression
gates rejected it: putting explicit RTM msvcrt.lib first selected MSVCR80 operator
new/delete, instead of the original MFC80 allocation imports. Its baseline_failed
state is preserved; it is not an accepted match.

Corrected ordinary link order: mfc80.lib, mfcs80.lib, then the explicit RTM
msvcrt.lib, before the remaining inputs/default search. This preserves MFC new
and delete while selecting RTM CRT startup. The actual guest command includes
the shared-library UNC path and the request records the pinned library hash.
This explicit path assumes the established guest share name fsx-build; a different
VM share mapping must be configured before using this option.

crt-rtm-mfc-order-20261102-001 passed all18 existing matched bodies/1114 bytes,
OLE/EH/vtable/import and source-identity gates. C++ and C initializer tables now
have the same verified roles/order as the original; the comparator verifies
the real CRT calls use those table boundaries. AdditionalCInitializer=null.
No binary patch, initializer deletion, replacement stub or application C++ edit.
Application source hashes equal baseline29.

The InitInstance diagnostic remains777 vs769 bytes,246 instructions/frame0x554,
43 non-branch positional differences; ShowErrorMessage remains two bytes. Thus
this library correction resolves the CRT table discrepancy, not those near-matches.

Adopted CrtLibrary Auto: RTM for MfcWindows, InstalledSP1 for ConsolePilot.
Explicit -CrtLibrary InstalledSP1 retains the old control. The RTM file hash is
pinned; an RTM MFC build must also pass the C initializer table role/order gate.
The library does not need installing in XP: the linker reads the existing share.
Fresh default run crt-default-20261102-001 ends matched at the static-initializer
scope, with all18 body regressions retained. Control results are stored in
artifacts/fsx.exe/crt-context-controls.json (SP1 table=false; RTM table=true).

Golden baseline30-rtm-crt-context, EXE SHA256:
D8973AC14E07DA44C01F0FA4AE207023E290361C2B2125460BB57FF902E17A7B.
Source/build command/receipt/request/hash/log/EXE/PDB/MAP/comparisons retained.
Old29 remains the SP1-library control. Inventory/replay defaults now reference30.
Matched-body count remains18/1114; table-role equality is not a new function-body
match, complete pre_c_init/CRT/EH equality, original-source proof or runtime test.

## Diagnostics and Ghidra

ToolchainDiagnostics is an optional host-controller switch: link /VERBOSE:LIB,
object directives, and selected installed SP1 archive-member headers/symbols/
relocations. A separately compiled /GL- diagnostic object is never linked into
the candidate. Installed CRT source files were not found at the checked paths.
The first two diagnostic runs used a missing separator after VCINSTALLDIR, so
their absent member output did not establish that source/libraries were missing;
crt-member-path and crt-table-probe corrected the path. /GL anonymous objects
did not expose useful directives, so archive relocations supply the actual cause.
All diagnostic builds preserve normal comparisons; no candidate binary executed.

Read fsx.exe and api.dll open, fsx.exe user-selected; no tab switches. Original
pre_c_init bytes at0x01002547 have no instructions/stored function in the model;
no disassembly or function creation was done here. Updated only existing plate
comments at0x01001238/0x01001250 to supersede the now-resolved current-build
difference and explain its library provenance. Saved fsx.exe; no API edits or
additional renames/type changes. The pre-static-initializer GZF backup remains.
See ../STATUS.md for final saved revision. Documentation is not a database backup.

Next bounded work: compare actual remaining original CRT/runtime body regions,
starting with pre_c_init, against symbol-resolved library counterparts before
creating functions; or proceed to API implementation behavior. Original runtime
undecoded ranges remain unresolved. Avoid blind variants of the near-matches.
