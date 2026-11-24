# MFC support library: directive difference and four-byte improvement

2026-11-03. Status: live verified from original fsx.exe, actual archive objects,
and XP builds. Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Installed versus installation-medium object evidence

WritableDataDiagnostics copies the installed mfc80.lib/mfcs80.lib/atls.lib to
vm-share/toolchains/vc80-sp1 only if absent and verifies each copy with guest
fc /b. It does not replace installed files. Actual capture job
mfc-installed-data-capture-20261103-001 passes existing comparisons; diagnostic
probe objects are never linked. Local captured hashes:

| Library | Bytes | SHA256 |
|---|---:|---|
|mfc80.lib|2623050|43D69CA7F505B50F7670B68F6BF0080A316AC688659FD599A759404FF8497979|
|mfcs80.lib|797886|599FF5BC0C8D16023BAF5F8AC07092839463C3EE425C4BF53073486B213F62B7|
|atls.lib|992826|D37F2CDEA299FF2A140BCF0DDA092C0518848227BEAE9F5200DFA627EB3B6AE4|

The generalized read-only COFF inspector now records section-owned symbols,
common symbols and directives, skipping auxiliary symbol records correctly.
Installed versus RTM-media INTEL\$DLL.W\stdafx.obj, nolib.obj and appmodul.obj
have identical .data/.bss section shapes, flags, symbol names and offsets.
Their non-COMDAT BSS contributions remain4,4 and1 bytes respectively. Therefore
simply assuming RTM omits the STDAFX force DWORD is contradicted by actual objects.
Object hashes differ; matching data shapes does not imply identical entire objects.

The significant directive difference is appmodul.obj:

- Installed support object: /include:__forceMFCManifest.
- RTM-media support object: direct manifest dependency on Microsoft.VC80.MFC
  version8.0.50608.0, with no such generic force-symbol INCLUDE.

Actual baseline40 has __forceMFCManifest at01004498 as a four-byte common
allocation. Reports: artifacts/fsx.exe/sp1-mfc-data-symbols-20261103.json,
rtm-mfc-data-symbols-20261103.json and mfc-data-member-comparison-20261103.json.
The installed-context label alone is not proof every installed library changed
in SP1; these are measured bytes and member differences for this installation.

## Controlled RTM support-library builds

Use the793834-byte media entry described in writable-data-layout.md, SHA256
8B0BE145F8256766954D36DBC43486F549FFCFE8F5C7F58BB2D7B0929458708A,
as shared toolchains/vc80-rtm/mfcs80.lib. The legacy installer entry contains
MFC80-specific directives; its mfcs71 label is not used to infer binary version.
Only replace the explicit mfcs80.lib linker input path. Retain SP1 compiler,
linker, headers, installed mfc80.lib and atls.lib, and pinned RTM msvcrt.lib.
No installation, VM rollback, runtime DLL replacement or canonical C++ edit.

Actual mfc-support-rtm-20261103-001 passes all existing comparisons, removes the
common symbol and reduces .data virtual1204->1200 (original1184). It initially
reports three LNK4099 warnings because the matching library PDB is absent. This
trial is retained explicitly; warnings were not suppressed.
EXE SHA256F166C4E9398DE970BF7A07332C5BCCA4868D36AE5BD33977728D44D86CA3B8F5.

Extracted the matching release support PDB from the existing ISO's
_17751_RTL_x86_enu_MFC_Ansi_Shared_Lib_pdb.cab (CAB SHA256
584BC7383CA94300404492D6291B9385512C5529836006DEF838887F7379709A).
Entry FL_mfcs71_pdb_____X86.3643236F_FC70_11D3_A536_0090278A1BB8,
2322432 bytes, shared as mfcs80.pdb; SHA256
0AEE35F17868DCFD122D4FDF96F3821EDC46E4688875324F93C39C3D5696A0D1.
Actual mfc-support-rtm-pdb-20261103-001 passes with no linker warnings, retains
the same measured improvement and all80 import-thunk placements/CC runs.
EXE SHA25651914ADA80F1FD3A717E4B30CB85269D0BBE62F2AB9E1B518933CAC2146C3BE6.
The pinned library and PDB identities are now both mandatory and recorded in
new RTM-support job requests.

compare-mfc-support.ps1 verifies identical compiled source/order-response hashes,
only the support-library path replacement, unchanged88-symbol prefix placements,
byte-for-byte identical reported IAT entries, all80 thunk checks, absence of the
common symbol, and the existing full data/body gate chain. IAT differences remain
103. The30-span ledger still maps1153 bytes; remaining47 rebuilt versus31 original
bytes leave16 unresolved, not20. Removed allocation in the reconstruction does
not prove that no corresponding original state exists in its unassigned bytes.

## Original embedded manifest, read-only supporting evidence

inspect-pe-manifest.ps1 traverses PE resource type24 and parses the actual XML.
Original resource24/1/1033 at RVA0029d8d8,833 bytes, requests:

- Microsoft.VC80.MFC8.0.50608.0, x86.
- Microsoft.VC80.CRT8.0.50608.0, x86.

Both public-key tokens are1fc8b3b9a1e18e3b. Resource24/101/1033 at RVA0029dc1c,
772 bytes, separately requests Common-Controls6.0.0.0. Evidence:
artifacts/fsx.exe/original-manifest-20261103.json, including original hash/XML.
This corroborates the RTM-era dependency context, not a compiler/build-host
identification, exact original library-file proof or actual runtime resolution.

Generated sidecar manifest now includes the added direct RTM MFC dependency.
It still contains additional SP1 assembly dependencies from the retained build
context; it is not identical to the original manifest. No resource reconstruction
or dependency rewriting is performed in this milestone; candidates are not run.

## Adopted actual default41

MfcSupportLibrary Auto resolves RTM for MfcWindows with RTM CRT, InstalledSP1
otherwise. Explicit -MfcSupportLibrary InstalledSP1 restores40 support-library
settings. Actual default job mfc-support-default-20261103-001 passes all existing
gates and the dedicated controlled comparison with no linker warnings.
Golden/replay/inventory: vm-share/modules/fsx/archive/41-rtm-mfc-support. EXE SHA256:
6953DEA3584BECDFBE03A0D55511D8B4093A593F421603B97EE3938214E8580B.
Archive contains actual sources/response/hash/request/receipt, build command/log,
EXE/PDB/MAP/manifest, headers/disassembly and comparisons.40/trials retained.
Inventory and vtable checks pass41; worker ready.

Reports include mfc-support-default-control-20261103.json,
writable-data-layout41-20261103.json and all-import-thunk-placement41-20261103.json.
Known ShowErrorMessage/InitInstance differences, application18/1114 count,
gap probe0 and code relative placement remain unchanged. No Ghidra edit/save,
tab navigation, names/types/API changes or new original-variable naming inference.
Database backup/save state is unchanged. No VM UI/autostart change.

Next examine the remaining16-byte writable placement discrepancy with the new
ledger and specific object contributions. Do not remove force symbols or invent
padding variables to make an unproven original layout fit.
