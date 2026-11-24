# Writable contributions and native-TU diagnostic

## 2026-11-19 application getter at 0x01004384

**Live verified, linked `fsx.exe`.** The November 18 packaged build placed
`pfnGetApplication` at `0x01004380`. A fresh XP RTM control reproduced
that placement. The new packaged link places the getter at `0x01004384`.
Ghidra reports one write and six reads. The following seven complete
instructions match the original PE at the same addresses, including
the four-byte absolute operands. Both PEs mark base relocations stripped
and have an empty base-relocation directory.
[Original cross-references](../../../artifacts/fsx.exe/getter-placement-20261119/ghidra-xrefs.json).
[Byte and symbol checks](../../../artifacts/fsx.exe/getter-placement-20261119/audit.json).

| Reconstructed function | Getter references in original and rebuilt | Complete linked body |
| --- | --- | --- |
| `FSXAppPilot::ExitInstance` at `01001720` | `0100173e` load | 58/58 exact |
| `FSXAppPilot::InitInstance` at `010019e0` | `01001c03` store, `01001c08` compare, `01001c43` and `01001c82` indirect calls | 769/769 exact |
| `FSXAppPilot::Run` at `01001d00` | `01001d05` indirect call | 35/35 exact |
| `ShowErrorMessage` at `01001d30` | `01001e13` load | 436/437 exact. Original `0F 8C` (JL), rebuilt `0F 88` (JS) at `01001e0d`. Both branch to `01001eca`. |

**Build evidence.** With the prior ATL-first link, import-selection response
file, PDB path, version resource and BinPlace recipe retained, VC8 RTM
compiled the installed MFC `nolib.cpp` separately with `/GL /MD /D_AFXDLL`.
Its local `stdafx.h` includes `windows.h`. LINK received `mfc_nolib.obj`
immediately after `application_interface_pilot.obj` and used the existing
RTM libraries under `/LTCG /OPT:REF /OPT:ICF`. The MAP places
`__afxForceSTDAFX` at `01004020`, the MFC application-state flag at
`0100437c`, `__afxForceEXCLUDE` at `01004380` and the getter at `01004384`.
The window and application remain at `01004388` and `010043e0`. All 30
reference-validated writable ranges now occupy their original addresses,
including the twelve CRT DWORDs at `01004024..01004053`.
[Guest command](../../../artifacts/fsx.exe/getter-placement-20261119/verified/build.cmd).
[Writable ranges and MAP owners](../../../artifacts/fsx.exe/getter-placement-20261119/writable-ledger.json).

**Controls.** Deferring the native `nolib.obj` archive member moved the getter
to `01004388`. Compiling MFC `nolib.cpp` under LTCG with the full MFC
source-tree `stdafx.h` produced the desired getter VA, but moved registry
token literals into the LTCG input and changed 400 or 401 of the 668
read-only bytes. The archived native RTM `nolib.obj` contributes four BSS
bytes and carries no read-only literal section. Both narrow header controls
(`windows.h` and `afxwin.h`) kept the tokens in `mfcs80:stdafx.obj` and
reproduced all 668 bytes. The recorded replay uses `windows.h`.
[Trial scores, command hashes and MAP owners](../../../artifacts/fsx.exe/getter-placement-20261119/trials.json).
[RTM archive member inventory](../../../artifacts/fsx.exe/rtm-mfc-data-members-20261103.json).

**Scope and provenance.** This result supersedes the earlier four-byte
getter displacement for the selected link. The 512 stored writable
bytes, 672 virtual zero-fill bytes, 135/137 aligned IAT identities,
all 80 thunk VAs and 78 exact thunk bodies were independently checked.
Of the 146 dashboard scopes (some overlap), 143 are Exact. Two MFC
thunks still normalize the swapped slots for ordinals 5230 and 1207.
The remaining Different scope is the callback branch above. The
shipping build's use of this MFC compilation recipe and the original
owner of the adjacent four bytes are hypotheses. This evidence covers
the static linked image. Native COFF equality and runtime behavior
were not tested. Ghidra reported saved revision
`1795013130661:dcf4182e:3` after the read-only export.
[Paired code listings and byte ranges](../../../artifacts/fsx.exe/getter-placement-20261119/dashboard-comparisons.json).

## 2026-11-18 worker-path layout control

Live verified for `fsx.exe` at `0x01004000`. The RTM ATL-first worker-path
link reproduces the `0x01001f92` thunk pointer and all 512 stored `.data`
bytes, with the original section geometry and 672 bytes of virtual
zero-fill. The same thirteen mapped objects are still shifted. The longer
CodeView record changes the SafeSEH placement and gives the original
addresses to all 80 import thunks. The basename build below remains
the comparison with the shipping debug-record shape. The shipping
layout and debug record have yet to be reproduced in a single build.
[Layout controls](codeview-gap.md#2026-11-18-resource-and-pdb-path-controls).

## 2026-11-18 link-time address ordering trials

Live verified read-only for `fsx.exe`. The original and each XP trial mark
PE base relocations stripped and leave the Base Relocation directory empty.
The remaining comparison differences are in addresses resolved at link
time. In the ATL-first profile the cached pointer at `0x01004000` resolves
to rebuilt `0x01001f72`, 32 bytes before the original thunk
`0x01001f92`. The linked CodeView record still has the original basename
and size. The unresolved 32-byte layout difference before SafeSEH is
tracked in [codeview-gap.md](codeview-gap.md).

Moving the CRT archive in front of MFC substituted CRT operator new and
delete imports for the two original MFC imports. The import gate rejected
that proposal. Selecting individual unchanged archive members moved the
twelve CRT DWORDs at `0x01004024..0x01004053` to their original addresses.
The first verified split increased the virtual data extent to 1192.
Ordering the remaining MFC and GS report contributions restored the
1184-byte size and reduced shifted known ranges from 13 to 9. But the
file-backed data had 11 raw differences, versus one in the ATL-first
profile, and the 80-thunk placement check rejected the changed CRT thunk
layout. The final trial passed the controller's function and reference
gates, including InitInstance. The single ShowErrorMessage byte remained.

The optional explicit-member route checks both the actual MAP object
owner and, for extracted `crtexew.obj` and `appmodul.obj`, the archived
COFF member hash. A prior trial without the object evidence remains
rejected. The ordinary archive-member route stays supported. The
ATL-first profile remains the preferred writable-image result. The
experiments used identical source snapshots. The Ghidra database was
read-only and reported saved revision `1795013130661:2514814d:58`.
[Build receipts, address deltas and regression results](../../../artifacts/fsx.exe/linked-address-order-20261118/trials.json).

## 2026-11-18 explicit ATL object resolves section geometry

Live verified for `fsx.exe` at `0x01004000` and the linked writable
section. Extracting the existing `atltrace.obj` from the guest's
`atls.lib` and placing it before the archive inputs yields the original
RVA, raw offset, access flags and extents. Virtual size is 1184 and raw size
is 512. This supersedes the 16-byte discrepancy for the new opt-in
`AtlObjectFirst` profile. Earlier reports describe their own builds.

The 512 file-backed bytes have one raw difference at section offset zero.
The ATL comparator verifies the cached getter targets `0x01001f92`
and `0x01001f72`. Normalizing that four-byte field gives complete
initial section equality, including 672 bytes of virtual zero-fill.
The cookie and the observed window and application storage match
their original addresses. Twelve CRT DWORDs from argc through fmode
remain four bytes later, and the application getter is four bytes earlier.

Adding the extracted cookie object after ATL reproduced the data result.
The simpler ATL-only profile passed a fresh replay with a job-local copy
of the installed `atls.pdb`, resolving LNK4099. All controller gates passed.
InitInstance stayed normalized and the 80 thunk placements and trailing
CC counts retained their verified scope. USER32 import order now matches.
MFC, CRT and KERNEL32 import order and the single ShowErrorMessage byte
remain open. The writable-image comparator rejects the old geometry.

The two four-byte MFC force contributions precede argc. The original
has four unmapped bytes before argc and four after fmode. Mapping those
bytes to the MFC contributions is a hypothesis. Source, build defaults
and installed toolchain are unchanged. Ghidra access was read-only
and reported saved revision `1795013130661:2514814d:58`.
[Section comparison](../../../artifacts/fsx.exe/writable-object-order-20261118/match.json)
and [build ledger](../../../artifacts/fsx.exe/writable-object-order-20261118/trials.json).

## 2026-11-18 current RTM layout and isolated controls

Live verified in `fsx.exe`. The fresh RTM reference build retains 1184/1200
original/rebuilt virtual bytes and 512 raw bytes in both images. All
1153 mapped initial bytes pass the writable-data ledger. The saved Ghidra
revision is `1795013130661:2514814d:58`. Ghidra xrefs identify
the getter at `0x01004384`, the window at `0x01004388` and the application
at `0x010043e0`. Queries for the starts of the unmapped intervals found no
stored references. That leaves their original ownership unresolved.

The current PDB is paired with the linked EXE by RSDS GUID and age. It
places the ATL contribution at rebuilt `0x01004020` and the first MFC BSS
contribution at `0x01004030`. The CRT output span beginning with argc maps
from original `0x01004024` to rebuilt `0x01004038`, a 20-byte displacement.
By the exception record the displacement is 16 bytes. The original's
four-byte unmapped range at `0x01004054` has no corresponding gap before
the rebuilt exception record. From the MFC flag to the end of the section
the net displacement stays 16 bytes. This accounts for the net difference
by measured intervals. It does not identify every gap as linker padding.

All three controls used the same source hashes as the fresh reference.

| Change | .data virtual bytes | Verified result |
|---|---:|---|
| Getter TU without /GL | 1200 | Getter moves to `0x01004030`. Window and application placement hold. InitInstance and regression checks pass. |
| Explicit ATL library before pinned CRT | 1200 | ATL pointer reaches original `0x01004000`. Cookie moves to `0x01004010` versus original `0x01004004`. InitInstance and regression checks pass. |
| Startup TU without /GL, getter still /GL | 1188 | Window and app move to `0x01004028` and `0x01004080`, and the application initializer fails the constructor-target gate. Rejected. |

The last trial isolates a different result from the historical 1192-byte
trial, which compiled both TUs without /GL. The accepted build remains
at 1200 bytes. The evidence supports further contribution-order work
without adjusting the class layouts on the basis of section size.
[Trial receipts, commands, hashes and layouts](../../../artifacts/fsx.exe/writable-layout-20261118/comparison.json)
and [baseline PDB contribution evidence](../../../artifacts/fsx.exe/writable-layout-20261118/baseline-contributions.json).

2026-11-03. Status: live verified at the stated scopes. Original fsx.exe SHA256
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735;
selected41 SHA2566953DEA3584BECDFBE03A0D55511D8B4093A593F421603B97EE3938214E8580B.
Baseline .data remains1200 versus original1184 (+16). No new promotion.

## Read the actual rebuilt PDB rather than infer all zero gaps

Added scripts/inspect-pdb-contributions.ps1, a narrow read-only MSF7/DBI-v60
reader. It handles the observed single-page directory block map, validates
stream/record bounds, pairs PDB Info GUID/age with the EXE RSDS record and reads
module/section-contribution entries. Unsupported formats/maps fail explicitly.
It does not edit symbols, use symbol-server downloads or launch either EXE.

The implementation uses [LLVM MSF documentation](https://llvm.org/docs/PDB/MsfFile.html),
[LLVM DBI documentation](https://llvm.org/docs/PDB/DbiStream.html), and the
[LLVM record definitions](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/DebugInfo/PDB/Native/RawTypes.h).
These define the reader layout, not the cause of FSX differences.

Baseline41 PDB SHA256:
ECD32C7E270B43B05D6DA079E171710E6862D61B8DF4454826EB7076332BE708.
Its20 reported public writable-data owners match actual MAP owners (including
the six common allocations owned by the linker), confirming stored module-index
interpretation. The three pilot getter/window/app symbols have no contribution
entry in this PDB table. Their known allocations remain established by the
existing source/MAP/body ledger; they are not padding merely because this table
does not report them.

PDB-reported writable contributions cover913 bytes;287 are not reported:

| Baseline41 VA | Bytes | Evidence |
|---|---:|---|
|Rebuilt 0x01004018|8|Zero bytes between recorded CRT initialized data and ATL pointer|
|Rebuilt 0x01004024|12|Zero bytes between ATL pointer and recorded MFC BSS|
|Rebuilt 0x0100438D|267|Includes the three known pilot globals; cannot be called a padding span|

The two small gaps are verified zero in the actual PE. Input sections already
inspected in the previous milestone have4-byte alignment; their flags alone
do not explain this larger observed spacing. There is no original PDB/contribution
map, so these current gaps do not prove original padding locations or prove the
presence/absence of a particular original global.

2026-11-18 dashboard correction. The three cells above are rebuilt-image
addresses from the historical PDB, so they no longer appear as original
function entries. In the original image, live readback at `0x01004018`
shows the locale status and next math-error DWORD, with exact initial
values at rebuilt `0x01004010` and `0x01004014`. Original CRT output
DWORDs at `0x01004024`, `0x01004028` and `0x0100402c` map to rebuilt
`0x01004038`, `0x0100403c` and `0x01004040`, with equal zeros.
Original `0x0100438d` falls inside the observed window storage that begins
at `0x01004388`. All 88 observed initial bytes match at rebuilt
`0x01004398`. The Global data view records the verified original
addresses. The historical 267-byte span has no verified counterpart as a
single original object. The fresh rebuild's writable section still has
a 16-byte virtual-size delta.
[Writable data ledger](../../../artifacts/fsx.exe/crt-documented-remainder-20261118/writable-data.json).

Reports: artifacts/fsx.exe/pdb-contributions41-20261103.json and
data-contribution-analysis41-20261103.json. A negative control supplies the native
trial's PDB with the41 EXE via an override path; it is rejected specifically for
RSDS GUID/age mismatch before interpretation. No files modified. Evidence:
pdb-contribution-pair-negative-20261103.json. Both genuine EXE/PDB pairs parse
successfully and their reported public data owners pass independent MAP checks.

## Controlled native application-TU trial

Question: does changing the application's GL compilation context affect data
placement? Actual data-native-context-20261103-001 uses exactly the same source/
header/order hashes and libraries as41. Changes are only context /GL -> /GL-
and omission of getter /GL; the existing linker /LTCG flag is retained. This
does not claim that a linker flag alone was isolated or that all compilation
inputs of every library were rebuilt without GL.

Actual result:

- .data virtual1192 (original1184, difference+8), raw512 retained.
- Initialized MAP .data length36, BSS begins at40. Pilot globals move before
  library BSS; original counterpart ordering is not recovered.
- natstart contribution now starts01004010; native source allocations become
  visible in the PDB contribution table. The data layout changes materially.
- .text virtual9786/raw10240, versus41's9622/9728.
- All137 import contents still match; this is not full layout/body equality.
- Existing controller rejects: Global application initializer mapping differs:
  appCtor. No full comparison.json is produced and no guard is relaxed.

EXE SHA2561A1C1F8EADFCF62918F7C19E8F2F5823903314D16E5C1CD4926B45F5E4775F6A.
Evidence: actual request/log/source hashes/MAP/EXE/PDB and
artifacts/fsx.exe/data-native-context-control-20261103.json,
data-native-context-pe-20261103.json, data-native-context-pdb-20261103.json.
This is a rejected layout/code-context control, not a new matching baseline or
proof of a runtime semantic error. The smaller size cannot override a failed
existing reconstruction invariant.

## Decision and remaining uncertainty

Default ContextCompilation GL/GetterCompilation GL and golden/replay41 remain.
The optional ContextCompilation NonGL switch records the context choice in job
metadata. No canonical C++ edit, invented global, object patch or padding removal.
The remaining16 bytes are still unresolved. The experiment supports dependence
on compilation/contribution arrangement; it does not isolate one particular
linker alignment rule or exclude zero-initialized state in original gaps.

Next use contribution/section evidence for an isolated toolchain or section-order
comparison while retaining the established bodies. Do not accept the native-TU
trial, change class packing or assign original private names on size alone.
Known ShowErrorMessage/InitInstance differences, import103 and synthetic gap0
remain. No new variable rename is supported. No Ghidra DB read/edit/save or tab
navigation this milestone; save/backup state unchanged. No candidate execution,
VM UI/autostart or installed toolchain change. Worker ready.

Background checks consulted [Microsoft LTCG](https://learn.microsoft.com/en-us/cpp/build/reference/ltcg-link-time-code-generation)
and [OPT](https://learn.microsoft.com/en-us/cpp/build/reference/opt-optimizations)
documentation. Their general mechanisms do not establish a VC8-specific cause
of these16 bytes; conclusions above come from actual local PDB/PE/MAP/build reads.
