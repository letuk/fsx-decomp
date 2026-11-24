# EXE RTTI presence audit and disabled-RTTI build

2026-11-02. Status: live verified fsx.exe listing/bytes, PE/MAP/import scans,
source-controlled actual XP builds and saved Ghidra comments. Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Original evidence and corrected reconstruction assumption

The original known primary vtables do not have a local x86 RTTI complete-object
locator immediately before their first slot:

| Anchor | Observed preceding word | Independent interpretation |
|---|---|---|
|Application0x01001398|0x00000073 at0x01001394|End of existing `Error ID = %d\r\n%s` string, not an image pointer|
|Filter0x01001458|0x010022a8 at0x01001454|Last observed application slot; destination is JMP [0x010010bc], not a metadata record|

Native inspection preserves established FSXApp_vftable and
FSXMessageFilter_vftable user names. The original non-resource sections have no
scanned null-terminated decorated class/struct type names beginning .?AV/.?AU.
This does not assert absence of every possible metadata form or external DLL
RTTI. MFC GetRuntimeClass/runtime-class facilities are separate from these C++
RTTI records and are not disabled or reinterpreted by this audit.

Baseline31 instead has local COL records at both reconstructed vtables, eight
type names and linked RTTI symbols: type_info, CWinThread, CWinApp, FSXAppPilot,
CObject, CCmdTarget, COleMessageFilter and FSXMessageFilterPilot. Pilot identifiers
remain reconstructed names; no original class-name inference is made. Renaming
them cannot remove the extra metadata. Disabling C++ RTTI is the evidence-based
experiment; exact original flag/TU configuration remains unproven.

## Controlled builds and result

Actual XP runs rtti-disabled-20261102-001 and rtti-default-20261102-001 both
complete all existing regressions and InitInstance diagnostics. The latter also
passes the newly integrated RttiMetadata gate with the new default.
Source-file hashes against baseline31 are identical; build.cmd differs solely
by /GR- on both compiler command lines. Linker options, /hotpatch, /GL, /GS,
RTM CRT library, MFC precedence and source remain unchanged.

The eight type names, linked RTTI symbols and both local COL associations
disappear. Two extra imports also disappear, agreeing with original absence:
MSVCR80.DLL!?terminate@@YAXXZ and
MSVCR80.DLL!?_type_info_dtor_internal_method@type_info@@QAEXXZ.
These dependencies were observed in baseline31; this is not a claim that
TerminateProcess, which remains imported, should disappear.

Rebuilt file size drops from12800 to11264 bytes (1536 bytes). The rebuilt file
does not yet reproduce original resources/layout; this size reduction is not a
whole-file match percentage. Both known vtables still differ in placement/order:
baseline32 filter0x004033a0 and application0x00403410, versus original application
before filter. Their predecessor raw words are not expected to be equal; the
gate checks the stated RTTI shape, not identical surrounding layout.

All18/1114 previously counted application/helper/wrapper checks, separate CRT/
ATL scopes and76-slot address-sharing pattern remain valid. ShowErrorMessage
still differs at two bytes. InitInstance still769 original/777 rebuilt,246
instructions each and43 different nonbranch pairs; no new function-body match.

## Gate, negative control and baseline

scripts/compare-rtti.ps1 checks the two known vtable anchors, recognizes valid
local zero-offset x86 COL/header/type-name relationships, scans non-resource
sections for decorated class/struct names, checks rebuilt RTTI MAP symbols and
the two selected imports. -RequireOriginalShape enforces the observed absence.
It is not a full RTTI parser for arbitrary inheritance or a whole-data verifier.
The real RTTI-enabled baseline31 is rejected under that switch; no artificial
binary mutation is needed for this counterfactual control.

Default host option is -Rtti Disabled; -Rtti Enabled retains the prior control,
which reports the mismatch rather than enforcing absence. comparison.json now
contains RttiMetadata; inventory reports RttiObservedShapeMatches. Reports:
artifacts/fsx.exe/rtti-disabled-comparison.json, rtti-enabled-control.json,
rtti-negative-control.json and rtti-controlled-inputs.json. The initial disabled
report belongs to the initial disabled EXE, not the later default EXE; each
report contains its own actual hash.

New golden/replay/inventory baseline: vm-share/modules/fsx/archive/32-rtti-disabled, copied
from rtti-default-20261102-001 with source/request/receipt, build.cmd, log,
EXE/PDB/MAP, disassembly, headers, diagnostics and comparisons. SHA256:
E0E82FCFA5885B26692B9DA7C43E9D3BBB5883158605A715E65368BCA455650F.
Baseline31 and both actual disabled runs remain retained. No guest worker change
or candidate execution occurred. The /GR- configuration is now experimentally
supported for reconstruction, not uniquely identified as the original command.

## Ghidra persistence and next work

Pre-edit backup artifacts/fsx.exe/ghidra-backups/fsx-before-rtti-audit-20261102.gzf,
2237777 bytes. DocumentExeRttiAbsence.java checks original anchor values and
established names before appending evidence comments at0x01001398/0x01001458.
No code, type, name, prototype, function bound or program selection changed.
fsx.exe explicitly saved, native readback unsaved_changes=false; api.dll untouched.
No new local-variable rename suggestion follows from this metadata audit.

Next: remaining constants/strings, MFC runtime-class/message-map data, global
initial values and other metadata contents, then section/import/image-base/code
order/header layout. C++ RTTI is no longer an unexplained extra in the default
rebuild; that does not close these separate data and layout questions.
