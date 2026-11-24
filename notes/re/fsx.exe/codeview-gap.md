# CodeView-to-SafeSEH zero gap and optional layout probe

## 2026-11-18 BinPlace reproduction and repeat

Live verified for `fsx.exe` at `0x01001518..0x01001567`. VC8 RTM links
the ATL-first build with enough space for the actual intermediate PDB
path. Microsoft BinPlace then places the EXE and separates the public
and private PDB outputs with `-a -x -s ... -n ...`. It reduces the
CodeView size to 32, writes `fsx.pdb`, and clears the unused path
storage. The resulting 56 bytes at `0x01001530` reproduce the original
basename, terminator, 40-byte zero gap and both SafeSEH RVAs. SafeSEH
starts at `0x01001560`. BinPlace also updates the PE checksum. The
before/after image diff is confined to the path storage, debug size
field and checksum. The linked RSDS GUID, age and debug timestamp
survive packaging. The GUID, timestamp and checksum still differ from
the original image.

The first successful XP control used the actual absolute worker PDB
path and a 70-byte reservation. Two independently named XP jobs used
the fixed intermediate path `.\build\release\symbols\intermediate\fsx.pdb`
for both `/PDB` and `/PDBALTPATH`. The 69-byte intermediate records
produced the same addresses and packaged record shape. The packaged
EXEs differ only in known per-build metadata. `.text` and `.data`
section headers match the original, and the EXE entry at `0x0100262e`,
ATL thunk at `0x01001f92`, and all 80 import thunk addresses match.
The entire initial `.data` image is byte-exact. Thirteen individual
data ranges still have shifted addresses. InitInstance passed its
normalized body comparison. ShowErrorMessage still has one residual
byte after address normalization, at `0x01001e0e`.

The tested x86 BinPlace is version `6.1.6920.0`, extracted from the
official Windows Driver Kit 7.1 distribution. Its resource-free XP
control rejected the EXE because it lacked a version resource. An
ordinary diagnostic `VERSIONINFO` satisfied that prerequisite. It adds a
576-byte `.rsrc` at RVA `0x5000`; the shipping resources are outside this
control. The linker macro `%_PDB%` and VC8 `/PDBPATH:NONE` both produce
a 32-byte basename record during linking, with SafeSEH still at
`0x01001540`. LTCG ignored the incremental-link option in its trial.
These controls supersede the earlier open question of whether the
basename and layout can be reproduced together. The original release
tool and command remain a hypothesis. The reproducible trial is retained
separately from the standard profile, which does not have the shipping
resources. Ghidra was read-only and reported saved revision
`1795013130661:2514814d:58`.

[XP jobs, command and tool hashes, and byte-diff ledger](../../../artifacts/fsx.exe/codeview-packaging-20261118/trials.json),
[section and ATL pointer comparison](../../../artifacts/fsx.exe/codeview-packaging-20261118/writable-exact.json),
[import thunk placement](../../../artifacts/fsx.exe/codeview-packaging-20261118/thunk-placement.json),
[InitInstance body](../../../artifacts/fsx.exe/codeview-packaging-20261118/initinstance-body.json).

## 2026-11-18 resource and PDB path controls

Live verified for `fsx.exe`. The original `.rsrc` starts at RVA `0x5000`,
after `.text` and `.data`. An XP build added a single VC8 `RCDATA` resource
to the ATL-first basename replay. The new `.rsrc` also starts at `0x5000`.
Both existing section headers and all 512 file-backed `.data` bytes agree
with the replay. The 18 `.text` byte differences are confined to the
debug-directory timestamp and RSDS GUID. SafeSEH stayed at `0x01001540`,
and the ATL thunk and cached value stayed at `0x01001f72`. The control
also introduces Rich tuple `(124, 50727, 1)`, present in the original and
absent from the resource-free replay. This measures a header effect and
rules out the absence of a separate resource section as the cause of the
observed displacement. The original resource payload was not extracted
or linked in this trial.

The same RTM ATL-first inputs built in `-PdbPathMode Worker` with run ID
`cvpath` produce a 70-byte RSDS record at `0x01001518`. VC8 embeds the
actual worker path `C:\fsx-build\matching-jobs\cvpath-001\fsx.pdb`. The
record ends at `0x0100155e` and two zero bytes precede SafeSEH. The table
address and both handler RVAs match the original. The ATL thunk and the
cached pointer at `0x01004000` agree with the original target `0x01001f92`.
The `.text` and `.data` section headers agree with the original. The
file-backed `.data` bytes and 672 bytes of implicit zero-fill compare
exactly. Thirteen individually mapped data ranges still have shifted
addresses. The image entry `0x0100262e` and all 80 import thunk entry
addresses and trailing `CC` counts agree. The thunk IAT operands were
validated by imported identity. InitInstance passed its normalized body
comparison and the controller passed its regression gates. The
ShowErrorMessage entry now agrees at `0x01001d30`, with the same residual
byte at `0x01001e0e` after address normalization.

The original stores `fsx.pdb` in a 32-byte RSDS record, followed by
40 zero bytes. `/PDBSTRIPPED` generated a separate public PDB and
`/RELEASE` populated the PE checksum. Both basename controls kept the
32-byte record and early SafeSEH placement, as did the pinned SP1
compiler build. The worker-path trial demonstrates that native linker
allocation for the debug record can recover the downstream addresses.
How the shipping build obtained the same addresses with the short
record remains an open hypothesis. The 2026-11-03 synthetic reservation
below documents the earlier experiment. The current trial required no
synthetic reservation or edit to source or build defaults. Original
memory at `0x01001518` agrees with the file, and read-only Ghidra reported
saved revision `1795013130661:2514814d:58`.

[Trial ledger](../../../artifacts/fsx.exe/codeview-layout-20261118/trials.json),
[basename control section comparisons](../../../artifacts/fsx.exe/codeview-layout-20261118/basename-control-sections.json),
[writable image comparison](../../../artifacts/fsx.exe/codeview-layout-20261118/cvpath-writable-exact.json),
[import thunk placement](../../../artifacts/fsx.exe/codeview-layout-20261118/cvpath-thunk-placement.json),
[Rich header for the resource control](../../../artifacts/fsx.exe/codeview-layout-20261118/rich-resource-probe.txt).

## 2026-11-03 investigation

2026-11-03. Status: live verified original fsx.exe model/memory, PE/MAP reads,
actual XP probe/control builds, negative control and saved comment. Original
SHA256:61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Original range, not a recovered source object

Original CodeView record VA0x01001518 has declared size32 and ends exclusively
at0x01001538. All40 bytes0x01001538..0x0100155f are zero. Native Ghidra inspection
finds no defined data, instructions, symbols or incoming model references in
this gap. SafeSEH begins0x01001560. Existing DotNetPdbInfo31 excludes the path
terminator at1537; that separate31/32 distinction does not explain this gap.

Baseline37 has the same32-byte CodeView shape but only8 zero bytes before its
SafeSEH table0x01001540. Its MAP identifies .rdata$debug/32 at1518,
.rdata$r/0 at1538, then .rdata$sxdata/8 at1540. Thus an extra32-byte allocation
in the intervening contribution is a concrete layout model to test. This does
not establish that the original contains a C++ object named by our probe.

## Deliberately synthetic experiment

Actual XP job debug-gap-probe32-20261103-001 adds only this diagnostic source
object to otherwise unchanged original C++ inputs:

```cpp
#pragma section(".rdata$r", read)
extern "C" {
    __declspec(allocate(".rdata$r"))
    extern const unsigned char fsx_debug_gap_probe[32] = {0};
}
```

The staged linker explicitly includes the object/symbol. The data-only object
uses /Zl to avoid adding default CRT-library directives. It is generated before
source hashing, and its compiled source copy is verified by the worker/controller
just like the original source files. Canonical src files are untouched.

Measured results:
- .rdata$r now32 bytes at0x01001538; SafeSEH table0x01001560, exactly original.
- RTC table group positions agree with original; .text contribution begins1580.
- .text virtual size9654 now equals original; raw size9728 remains equal.
- .data virtual1204 still differs from original1184; no whole-data/layout match.
- Entry0x0100261b versus original0x0100262e (-19), not a uniform original offset.
- Existing application/CRT/ATL/RTTI/data/import/header/debug/stack/base gates pass.
- ShowErrorMessage retains two differing bytes; InitInstance769/777,246
  instructions and43 nonbranch differences remain. No new function-body match.

The extra object also affects small SDK leaf placement: of343 common image MAP
symbols,147 shift exactly+32 and192 remain unchanged, but four do not obey that
simple rule. CObject::Serialize and Dump (shared body), AssertValid and operator
delete reorder within the initial leaf group. These are four symbols for three
bodies; previous body/vtable/alias checks still pass. This disproves a blanket
claim that the probe merely shifts every code symbol+32. No discrepancy was
hidden by changing the expected map-shift model.

Only one new image symbol appears: _fsx_debug_gap_probe. It is synthetic, not
an original recovered variable. Comparison/source/command evidence and actual
shift exceptions: artifacts/fsx.exe/debug-gap-probe-20261103.json and
scripts/compare-debug-gap-probe.ps1. Probe EXE SHA256:
A3A116882DB81944178B17C095F4A865CB3469094796F595490B9384A711BE5E.

## Default retained, regression and limits

Controller option -DebugGapProbeBytes 32 enables this experiment only for merged
MfcWindows. Default remains0. Fresh actual default job debug-gap-control0-
20261103-001 also passes all regressions/InitInstance diagnostics and has no added
probe source. Golden/replay/inventory remains37-original-header; no baseline38
was selected. Probe and fresh default outputs remain under their run directories.
Worker ready/no active job, no VM UI actions or candidate execution.

A copy with the first gap byte changed to1 is rejected as nonzero gap, while
the earlier routine/data gate scopes remain intact. Result in
artifacts/fsx.exe/debug-gap-negative-control-20261103.json; modified copies deleted.
This validates the diagnostic's actual zero-content claim, not an original
reason for those zero bytes.

Status of origin: unresolved hypothesis. Linker allocation, explicit contribution
or release/debug postprocessing may account for it. Microsoft's
[BinPlace documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/devtest/binplace-command-line-syntax)
documents symbol-stripping capabilities, but does NOT establish that FSX used
that tool or that it produced this particular gap. No such release tool was run,
and no original postprocess or full PDB path was recovered. The experiment proves
the effect of an explicit reservation, not its historical cause. For that reason
the diagnostic is not silently enabled as reconstructed production source.

## Ghidra persistence and next work

Pre-edit packed database artifacts/fsx.exe/ghidra-backups/fsx-before-codeview-gap-20261103.gzf
(2238507 bytes). InspectCodeViewGap.java is read-only. DocumentCodeViewGap.java
checks original anchors/zeros/no model references before appending evidence at
0x01001538. No type, symbol, code or function body changed; no original char
array or invented variable name was applied. fsx.exe explicitly saved, native
readback unsaved_changes=false. api.dll untouched; no program navigation.
No variable-renaming suggestion is justified for an unidentified zero extent.

Next: treat gap origin as open, retain this optional model for layout controls,
and examine actual function order from known original entries versus MAP. The
prototype's SDK leaf-order changes must be separated from any explicit ordering
experiment. A measured original contribution/release pipeline would be needed
before claiming the32-byte reservation as original source/build behavior.
