# Writable-data layout ledger and controlled probes

Follow-up: mfc-support-library.md supersedes the selected40 state below with
default41. The isolated RTM support replacement reduces the size delta20->16;
the same1153 mapped bytes leave31 original/47 rebuilt bytes unassigned. The
following experiment outcomes remain historical controls, not current defaults.

2026-11-03. Status: live verified at the stated scopes. Original fsx.exe SHA256
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735;
selected baseline40 SHA256323E3F7F56CD0A4423C08605DECCDFF5B8B3CF610EC9FA90670A198B3625F393.

## Reference-validated ledger

Original PE .data virtual size1184, rebuilt1204, both raw512 bytes at RVA4000.
inventory-writable-data.ps1 retains the complete existing DataConstants/body
gate chain and combines its established reference mappings with CRT argument/
startup outputs, failure records and the one-byte MFC initialization flag. It
validates30 disjoint spans,1153 bytes in each image. These are initial storage
and reference identities, not full original source declarations or class sizes.

Selected mappings:

| Role | Original VA | Baseline40 VA | Delta |
|---|---|---|---:|
| ATL cached getter pointer |01004000|01004020|+32|
| Security cookie/complement |01004004/08|01004000/04|-4|
| Global locale status/default math error |01004018/1C|01004010/14|-8|
| CRT argc/env/argv outputs |01004024/28/2C|01004038/3C/40|+20|
| CRT managed/status/newmode/result/has-cctors fields |01004030..40|01004044..54|+20|
| CRT wildcard/newmode/commode/fmode |01004044..50|01004058..64|+20|
| Failure exception record,80 bytes |01004058|01004068|+16|
| Failure debugger flag |010040A8|010040B8|+16|
| Failure context record,716 bytes |010040B0|010040C0|+16|
| MFC state flag,one byte |0100437C|0100438C|+16|
| Application getter |01004384|01004390|+12|
| Observed window/app storage |01004388/43E0|01004398/43F0|+16|
| adjust_fdiv and remaining startup/common states |01004488..9C|0100449C..B0|+20|

This disproves a uniform+20 shift for all writable globals. Every unchanged raw
initial value is checked including virtual zero fill; the ATL pointer instead
retains the existing independently checked code-target normalization.

The ledger leaves31 original bytes and51 rebuilt bytes unassigned. Original
regions are0100400c/12 bytes,01004020/4,01004054/4,010040ac/4 and0100437d/7.
Eight original unassigned bytes are FF at4010..4017; the others are zero. The
rebuilt unassigned FF pair at4008..400f belongs to public natstart symbols, but
an original identity is not established merely from matching constants. Do not
label all unassigned bytes as padding. Zero gaps may include unreferenced globals.
In particular, neither the zero DWORD400c nor the seven bytes after the original
one-byte MFC flag is assigned to a manifest/force symbol without evidence.

Rebuilt MAP provenance includes force flags from mfcs80:nolib.obj/stdafx.obj,
the ATL pointer from atls:atltrace.obj, and __forceMFCManifest at01004498 as
<common>. The original counterpart of that common symbol remains unknown; it
is not proved absent or an extra source variable. The PE alone does not reveal
the original COFF initialized-data/BSS split or section ownership.

Evidence: artifacts/fsx.exe/writable-data-layout40-20261103.json contains all30 mappings,
actual public owners, raw prefix DWORDs and both unassigned-region lists.

## Actual object/header diagnostics

Job writable-data-diagnostics-20261103-001 retains baseline inputs/settings and
passes existing comparisons. The additional non-GL diagnostic object is never
linked. The log records installed MFC header force-manifest directives and
HEADERS/SYMBOLS for pinned RTM gs_cookie/gs_report/natstart and installed ATL
atltrace members. Actual cookie and ATL pointer sections are4-byte aligned;
natstart has8 initialized bytes aligned4; gs_report BSS is804 bytes aligned8.
Thus the observed output alignment cannot simply be attributed to an assumed
16-byte alignment flag on the ATL pointer or cookie input section.

Read-only fsx.exe Ghidra metadata confirms the original path/base. Xref queries
find no references to400c,4010,4014 or4380;4484 is the already documented
application event field. Absence of references does not prove padding. No Ghidra
edit, rename, type application, save or navigation occurred. Existing provisional
DAT_0100437c -> g_mfcApplicationStateInitialized suggestion from
static-initialization-matching.md remains a suggestion, not an applied rename.

## Three isolated layout probes

All compiled input hashes, including the88-symbol order response, remain equal
to40. compare-writable-data-selection.ps1 checks only the stated command changes,
reruns existing data/body checks and all80 thunk placement/CC checks. All three
jobs pass those normalized-body gates; this does not certify original PE layout.

| Actual run (all suffixed20261103-001) | Only command change | Outcome |
|---|---|---|
|writable-data-atl-first-|INCLUDE for the existing ATL cached-pointer symbol|Data addresses/sizes unchanged; IAT differences103|
|writable-data-atl-library-first-|Explicit atls.lib after mfcs80, before pinned CRT|ATL pointer4000 matches, cookie4010 does not match original4004; .data still1204/raw512; IAT differences102|
|writable-data-bss-merge-|/MERGE:.bss=.data|BSS precedes initialized data; virtual1216 (+32), raw1536 instead of512; LNK4254 reports differing input attributes; IAT differences103|

The first option does not move the ATL contribution. The explicit library order
does move it, but does not reproduce original placement of the other globals or
reduce the virtual-size discrepancy. It is retained as an isolated control even
though one more IAT slot happens to align. The explicit BSS merge worsens both
virtual and raw extents and is not adopted. No attribute/body/order gate is
weakened to accept a raw-layout regression.

EXE SHA256 values in table order:

- 2CA9AF324D815455B21A7CACF13AC01B07A53C25AD2D0914AC04401EDF90E776
- 479FFECFD6255DC46EC354E1DD27C0FD8B29D4CDC19BDE72B3DFADD8CB5BC7C8
- 850C11E6EE8D572A5B5552E37382F05F5C1EBD5E0180546AB5741BFC30FAC699

The job files/reports are authoritative for hashes. Reports are
artifacts/<run-id>-control.json, with separate layout/thunk reports for the two
selection trials. All outputs, source hashes, requests/receipts and diagnostics
remain in their runs. Worker compiles/dumps only; no candidate is executed.

## Installation-medium inspection, not a library switch

Read the existing VS2005 RTM ISO without installing/mounting it. Preserved two
CABs under artifacts/shared/toolchain-media:

- _137_RTL_x86_enu_MFC_Ansi_Shared_Libraries.cab,
  SHA25623B3B65FF6E8C7A124811485B1D838B23627D44BC751097DD9CF651A5475DC5C.
- _153_RTL_x86_enu_MFC_Ansi_Static_Libraries.cab,
  SHA25679ED7F402C37E06B1313AED11F819215EAEB15DED1C1DB1E16D0E339F488EE58
  (listing only; no contained static library was linked).

The shared CAB's legacy installer entry
FL_mfcs71_lib_____X86.3643236F_FC70_11D3_A536_0090278A1BB8 is793834 bytes,
SHA2568B0BE145F8256766954D36DBC43486F549FFCFE8F5C7F58BB2D7B0929458708A.
Read-only COFF inspection finds INTEL\$DLL.W\stdafx.obj non-COMDAT BSS4,
nolib.obj BSS4 and appmodul.obj BSS1. This does not establish an original FSX
library version, original ownership of the unidentified zero bytes, or suitability
of the entire library. No RTM MFC/ATL/header substitution was built or installed.
See artifacts/fsx.exe/rtm-mfc-data-members-20261103.json. The generalized archive inspector
now accepts a member regex while retaining its CRT default.

## Decision and next evidence

Baseline40/default/replay remains selected, WritableDataSelection Default,
MergeBssIntoData false, diagnostics off. No new archive promotion or source edit.
The20-byte size difference is narrowed to an explicit ledger, not solved. Next
compare the relevant installed/media MFC force/data contributions, and identify
the remaining original gaps before choosing a library or packing experiment.
Do not infer a full original class size, remove a manifest symbol or fill gaps
with invented variables. Known application near-matches, import103 and optional
synthetic code-gap0 remain unchanged. Ghidra database save/backup state unchanged;
VM UI/autostart and installed toolchains untouched; worker ready.
