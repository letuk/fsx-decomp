# EXE literals, selected initial state and load configuration

## 2026-11-18 exact startup read-only block

Live verified for `fsx.exe` VAs `0x0100127c..0x01001517`. All 668
contiguous bytes in that range match the original after linking. The
previous admitted image differed at 306 offsets in that range. The
new range covers all 18 previously mapped literals, the MFC token
pointers, the message map and its 24-byte empty terminator, the
application and message-filter vtables, the empty error caption and
the complete 72-byte load configuration. Original raw bytes were read
from saved Ghidra and compared with the original PE file.

Moving the constructor definition below the message map places the
map before `FS98MAIN`. The OLE filter implementation follows the
application implementation in the startup translation unit, which
recovers the vtable order. The D3DX helper takes the export name from
its caller. This puts the literal in startup call order while retaining
the original short branch layout at `0x01001a44`. A direct in-method
guard put the literals in that order but expanded InitInstance from
769 to 778 bytes. Ordering literal COMDATs explicitly with `/ORDER`
placed the group before the original start. The trials preserve these
separate outcomes.

The shared empty caption points to the null byte at `0x010014c8`.
Defining an array in the startup translation unit placed the byte in
the gap before the vtables. Defining it in the adjacent application
support translation unit moved it after the vtables. The C++ name
`emptyErrorCaption` is reconstructed. Both historical pooled literals
and this explicitly named array are verified by the string comparators.

The packaged source replay and the ordinary XP RTM controller replay
reproduce the same 668-byte read-only range. The packaged comparison
preserves the previous 135 IAT slot identities, all 80 thunk addresses
and trailing CC counts, and the exact initial `.data` image. All twelve
InitInstance string operands now match. Of the method's 769 bytes,
four differ, all referencing the same application getter cell at
original `0x01004384` and rebuilt `0x01004380`. ShowErrorMessage still
has one non-address byte after normalization at `0x01001e0e`.
The dashboard now has 136 Exact, nine Normalized and one Different
across its 146 code scopes. CodeView metadata and the resource section
are outside the read-only block scope. Read-only Ghidra reported saved
revision `1795013130661:2514814d:58`.

The standard explicit RTM and pinned SP1 profiles also passed their
InitInstance comparisons. The installed-SP1 default stopped at the
compiler identity check because the guest reported RTM `14.00.50727.42`.
The recorded runs identify the toolchains and outcomes separately.

[Exact bytes and object records](../../../artifacts/fsx.exe/rdata-order-20261118/rdata-layout.json),
[XP variant ledger](../../../artifacts/fsx.exe/rdata-order-20261118/trials.json),
[InitInstance operand comparison](../../../artifacts/fsx.exe/rdata-order-20261118/initinstance-body.json).

## Historical observations

2026-11-02. Status: live verified original fsx.exe anchors and existing data
types; original PE versus baseline32 and fresh actual XP output, target-validated
comparisons, negative controls and saved Ghidra comments. Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Literal bytes and the observed token-pointer group

All18 selected terminated literals,183 bytes including their null terminators,
are byte-exact in baseline32. Each rebuilt location is independently identified
by a string-literal MAP symbol and exact content; uniqueness is required.

| Original VA | Content |
|---|---|
|0x0100127c|Delete|
|0x01001284|NoRemove|
|0x01001290|ForceRemove|
|0x0100129c|Val|
|0x010012a0/12a4/12a8/12ac|B / D / M / S|
|0x010012f8|FS98MAIN|
|0x01001304|language.dll|
|0x01001314|D3DX9_34.DLL|
|0x01001324|D3DXCheckVersion|
|0x01001338|msxml4.manifest|
|0x01001348|API.DLL|
|0x01001350|GetApplication|
|0x01001360|Unable to install message filter|
|0x01001384|Error ID = %d\r\n%s|
|0x010014c8|Empty string|

The eight observed pointers at0x010012b0..0x010012cc map uniquely to the32-byte
rebuilt group0x004032b0, in the same S/M/D/B/Val/ForceRemove/NoRemove/Delete order.
All target literals have independent exact-byte checks; no unvalidated addresses
were masked. Rebuilt literal provenance is mfcs80:stdafx.obj. This supersedes
the earlier open counterpart question in runtime-remainders.md. The original
table identifier, full extent and specific parser behavior remain unproven.

Naming suggestion only: currently unlabeled group0x010012b0 ->
g_mfcTokenStringPointers, a descriptive reconstructed name supported by the
matched literal provenance/contents. No label, pointer array or type was changed.
The stronger registry-parser interpretation remains provisional.

## Load configuration and SafeSEH

Original0x010014d0 versus rebuilt0x004034d0: all72 bytes match after validating
and normalizing only two pointers:
- SecurityCookie at+60: original0x01004004 -> rebuilt ___security_cookie.
- SEHandlerTable at+64: original0x01001560 -> rebuilt linker-defined table.

Internal structure Size is72, but PE data-directory Size is64 in both binaries.
This observed distinction is intentional in the comparison; neither field was
silently corrected or treated as identical to the other. PE directory addresses
are checked against the actual72-byte structures. Existing Ghidra
IMAGE_LOAD_CONFIG_DIRECTORY32 type remains preserved.

The complete SafeSEH table has two RVA entries,8 bytes total:
original0x2768 identifies the verified EH4 forwarding helper;
original0x2ca8 identifies the verified OLE C++ EH handler. Both rebuilt RVAs are
validated against their independently mapped functions, with count2. Existing
ImageBaseOffset32 types at0x01001560/1564 are preserved; these entries are RVAs,
not absolute pointers. This does not certify all PE header/layout fields.

## Global initial values and MFC runtime-class dispatch

Source-symbol-mapped zero-initialized spans agree in both images, including
virtual zero-filled BSS: window0x01004388/88 bytes, application0x010043e0/168
bytes including its event field, getter0x01004384/4 bytes. These260 bytes describe
observed pre-initializer storage, not post-constructor objects or proof of full
original class sizes. Reconstructed source symbols are not original-name proof.

Twelve named CRT DWORDs also agree: globallocalestatus0x01004018=FFFFFFFE,
defaultmatherr0x0100401c=1; dowildcard4044, newmode4048, commode404c, fmode4050,
adjust_fdiv4488, native_startup_state448c, native_startup_lock4490,
onexitend4494, onexitbegin4498 and dyn_tls_init_callback449c are all0. These
state comparisons overlap earlier CRT gates; they are not new function matches.
Cookie/exception buffers and other state already checked by previous gates remain
covered there; this is not a declaration that every writable byte is classified.

The application and filter first vtable slots retain imported runtime-class
getters: MFC80.DLL!#3333 maps to CWinApp::GetRuntimeClass; #3169 maps to
CCmdTarget::GetRuntimeClass. Original and rebuilt import identities agree.
Thus these observed dispatch paths use inherited MFC runtime-class facilities;
no custom metadata structure is inferred from reconstructed pilot names. This
is separate from C++ RTTI disabled in the preceding experiment and does not
inventory runtime-class records residing inside MFC80.DLL.

## Regression, controls and persistence

New mandatory MfcWindows DataConstants gate in comparison.json uses
scripts/compare-data-constants.ps1 and retains prior runtime gates. Fresh actual
XP job data-constants-20261102-001 passes all existing application/CRT/ATL/RTTI
gates and this data scope. Fresh EXE SHA256:
127D6E48578A2A152D5943B572AADF61844F4F778C685FACDA83D9DD967DDC65.
No source, build flag or library change. Baseline32 remains selected and gains
the new report against its own original EXE hash; fresh output remains in its
run directory rather than replacing the golden binary.

Three negative controls independently alter language.dll, a token-group pointer
and a SafeSEH RVA. All rejected; results in
artifacts/fsx.exe/data-constants-negative-controls.json, modified EXEs deleted and never
executed. artifacts/fsx.exe/data-constants-comparison.json is the baseline32 report.
Inventory now reports the18/183 literal scope, not a whole-data match percentage.

Pre-edit GZF artifacts/fsx.exe/ghidra-backups/fsx-before-data-constants-20261102.gzf
(2237514 bytes). DocumentExeDataConstants.java checks live anchors, existing
load-config type and zero spans before appending evidence comments at token
group, load config, SafeSEH table and three globals. Established names/types,
signatures, code and function boundaries are untouched. fsx.exe explicitly saved;
readback unsaved_changes=false. No tab navigation, api.dll edits or candidate
runtime execution. Existing18/1114 body count and known application nonmatches
are unchanged.

Next: inventory remaining unclassified data and PE directories/import contents,
then isolate image-base/section-placement/order changes. Identical literal and
initial-state contents do not establish identical layout, debug data or imports.
