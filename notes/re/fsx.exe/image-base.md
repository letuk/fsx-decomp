# Original image-base experiment

2026-11-02. Status: actual XP builds, source/command controls, PE/MAP/body/data/
import comparisons and wrong-base controls. Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Result

/BASE:0x01000000 gives the rebuilt PE the observed original image base. All
existing18/1114 application/helper/wrapper body checks and separate CRT/ATL/data/
RTTI/import-content gates retain their stated results. InitInstance remains769
original versus777 rebuilt bytes,246 instructions each and43 differing nonbranch
pairs. ShowErrorMessage still differs at offsets0xdb/0xdc. No source body changed.

Actual XP jobs image-base-original-20261102-001 and
image-base-controlx-20261102-001 both completed full regressions and InitInstance
diagnostics. Source-file hashes are identical; build.cmd differs only by
/BASE:0x01000000. Compiler/linker, libraries, padding, RTTI mode and other flags
remain unchanged. The original-base job was run first, then a fresh default-base
control after the base-check update. Both are actual compiled outputs, not replay.

All343 address-bearing MAP symbols agree in RVA between those two builds; none
are missing/extra. This includes aliases/import/global symbols, not343 recovered
functions. Their VAs differ by the chosen image base. Source-derived locations
still do not equal original locations just because their bases now agree:
original entry0x0100262e versus rebuilt0x0100209b; original IAT0x01001000 versus
rebuilt0x01003000. Import entry order and section layout remain open.

## PDB path-length control

The matched-length job names original/controlx deliberately produce equal92-byte
CodeView records. Their path text/GUID/timestamps still differ and are not a
raw-file match. Earlier baseline32's embedded job path produces an85-byte record;
therefore a direct layout comparison against32 also includes a debug-path-length
effect. In the fresh matched-length control both .rdata virtual sizes are3534,
and the343 MAP RVAs agree. This distinguishes a metadata layout effect from a
base-option effect. Do not assume every worker run has an identical PDB path
length or deterministic GUID/timestamp.

Report: artifacts/fsx.exe/image-base-experiment.json. It validates command/source
identity, bases and existing runtime/data/import scopes, and records the observed
RVA/path-length results. It does not assert full binary equality or execution.

## Comparator changes and controls

Four legacy scripts subtracted hardcoded0x00400000 from rebuilt VAs:
compare-copy-pilot.ps1, compare-formatter-pilot.ps1, compare-version-pilot.ps1 and
compare-show-error.ps1 (including its referenced fallback-string read). They now
read the actual PE ImageBase via scripts/lib/pe-image.ps1. Body lengths, original
addresses, required reference targets and normalized fields are unchanged.
The updated scripts still pass their established checks on old baseline32 and
the actual new original-base output. No new byte masking was added.

verify-startup-profile.ps1 additionally accepts ExpectedImageBase, records actual
and expected values, and rejects a mismatched build mode. The controller always
passes the requested expected base. Real baseline32 rejected when original base
was required; real new build rejected when default base was required. Results:
artifacts/fsx.exe/image-base-negative-controls.json. These checks do not change bytes.

## Adopted baseline and remaining work

Default host parameter is now -ImageBaseMode Original; -ImageBaseMode Default
retains the omitted-/BASE control. The flag is injected into staged linker
commands and the request records the mode. No worker restart or VM UI action.

Golden/replay/inventory baseline33-original-image-base preserves actual
image-base-original-20261102-001 source, request/receipt, build.cmd, log,
EXE/PDB/MAP, headers/disassembly, comparisons and InitInstance diagnostics.
EXE SHA256:
F6008B207A89E9EC20C95C4EC36FE237C044A3E812E1AB98A6FE7D8F3682AB61.
Control SHA256:
340204313CE6EBE74A2B5FD5D400F194718F758DECEA1872AB14A9893A5BC67E.
Baseline32 and both real runs remain retained. Default standalone paths in the
four touched legacy scripts also select33. Inventory/vtable checks pass on33.

No Ghidra database edits, save, program navigation or api.dll changes were needed
for this linker experiment; the original program's last saved state remains
unchanged. No candidate was executed and no new naming suggestion follows.

Next: a controlled section-merging experiment from observed original .text/
.data placement, with all current gates retained. Separate remaining tasks are
PE flag/version/stack fields, code/import order, debug identity/path and original
certificate/resource differences. The base experiment fixes the base and its
absolute-address contribution, not those other layout/content questions.
