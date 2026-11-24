# FSX EXE matching roadmap

Updated 2026-11-03. Scope: fsx.exe only; DLL implementation work is separate.
Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

Current golden: **41-rtm-mfc-support**. The synthetic 32-byte gap probe remains
optional and disabled. This current plan supersedes older chronological baseline
recommendations; historical evidence remains in topic files and CHANGELOG.md.

## Established status

Status: original PE/Ghidra evidence and actual compiled XP outputs at the stated
comparison scopes, not runtime-tested application equivalence.

- 18 application/helper/wrapper bodies, 1114 bytes, pass raw or individually
  target-normalized checks. Linked CRT/ATL identifications are reported separately.
- CRT startup/security/SEH/stack/exit/image helpers and ATL codepage bodies have
  independent body, target and selected data checks.
- All 137 import identities/hints and descriptor/module-block shapes match;
  entry ordering remains different. Imported data explains the two entries beyond
  Ghidra's 135 external function declarations.
- Selected literals/globals, token pointers and load-config/SafeSEH data pass.
- Image base, stack, selected PE flags/version fields, basename PDB record shape
  and non-resource section names/order/RVAs/raw extents match.

These are explicit scopes, not a whole-EXE percentage. Source names/layouts are
reconstructed. A normalized body match does not establish original addresses.

## Current execution order

User-requested continuation policy,2026-11-03: first consolidate a single
same-hash range coverage ledger for41, then choose the largest useful unchecked
code range or evidence-led InitInstance work. Defer layout/IAT polishing until
new evidence. See [the handoff](../handoffs/HANDOFF-20261103-coverage-and-code.md) for concrete
deliverables, limitations and a maximum of3 targeted trials per hypothesis.
This supersedes earlier chronological next-layout/toolchain suggestions.

## Remaining work packages

| ID | Work package | Evidence and next action | Completion criterion |
|---:|---|---|---|
|1|Function placement/order|Golden41 retains88 ordered application/ATL/MFC symbols and36 mapped runtime/framework symbols at uniform-32. Static CRT names reject /ORDER; entry INCLUDE fixes startup-member placement. All80 FF25 thunk identities, positions and immediate CC runs verified in40; see import-selection.md. Remaining range reconciliation and gap origin stay open.|Retain selected placement/body/alias gates; resolve remaining entries without claiming whole order matched|
|2|Import entry order|103 identities occupy different within-DLL positions (previously128). Direct INCLUDE trials worsen placement/order and are not adopted; investigate targeted member/contribution evidence.|Exact ordered identities/hints/initial ILT/IAT mapping and placement, not equal sets alone|
|3|Writable data placement|.data virtual1200 versus1184 (+16). RTM support-library directive removes4-byte common allocation;30 spans/1153 bytes retained,31 original/47 rebuilt bytes unassigned. Paired PDB contributions identify two small gaps; native-TU control improves size but fails initializer mapping. Next isolated contribution/toolchain evidence; see data-contribution-alignment.md.|Included storage and initial values accounted for; no unsupported original class-size inference|
|4|Residual application bodies|InitInstance769/777,246 instructions,43 differing nonbranch pairs. ShowErrorMessage437 bytes with two residual bytes. Choose source/TU/compiler-context trials from actual disassembly evidence.|Full body/reference equality, or precise documented remaining differences; token/CFG alignment alone insufficient|
|5|Complete range reconciliation|Combine all checks into one same-hash code/data/metadata coverage ledger and inspect uncovered ranges.|Every included range verified or assigned a precise open difference; selected checks not presented as exhaustive|

The IDs above identify work packages, not the current execution order. Start
with package5, then code work under package4 or an unchecked range it reveals.
Packages1–3 remain recorded residuals and are deferred pending new evidence.
No fixed number of builds, completion date or reliable match percentage follows
from current evidence. Historical compiler/release behavior may leave some bytes
unresolved, but useful experiments are not presently exhausted.

## Zero-gap investigation stays open

Original CodeView ends0x01001538, then40 zero bytes precede SafeSEH0x01001560.
Standard build41 has8 zero bytes and SafeSEH0x01001540. Current model has no defined data,
instructions, symbols or incoming references in that gap.

A synthetic32-byte .rdata$r object restores SafeSEH/RTC placement and .text virtual
size9654 with prior gates retained. It also reorders four SDK leaf symbols; a
uniform+32 code-shift claim is false. This is a diagnostic model, not recovered
original source or a proven release process. Default remains0; ordering41 is selected independently.
Retain it as a controlled alternative while investigating provenance; unknown
gap origin need not block independent function/import/data-order work.
See [codeview-gap.md](codeview-gap.md).

## Remaining body questions

InitInstance0x010019e0: register/scheduling and branch-encoding differences remain.
Its21-block CFG pairing/246-instruction count are not complete match evidence.
See [initinstance-matching.md](initinstance-matching.md).

ShowErrorMessage0x01001d30: offsets0xdb/0xdc retain original TEST EAX,EAX versus
rebuilt CMP EAX,EBX, with EBX zero. Prior trials have not resolved this. Revisit
with new source/TU/compiler evidence; avoid repeating blind permutations.
Consult [../SOURCES.md](../SOURCES.md) for provenance and historical limitations.

## Eventual completion target

Keep two goals distinct: independently verified body/data equality, and raw
image/file equality over explicitly defined included ranges. Resources are
outside the user's current reconstruction target. This does not automatically
exclude timestamps, PDB GUID, checksum/entry/size fields, certificate data or all
resource-related header effects. Define that final comparison policy explicitly
when consolidating the range ledger. No original signature/debug identity claim.

Inventory:212 model functions =77 internal+135 external;140 mixed-origin rows,
129 unassigned instructions/63 ranges. Executable section5168 instruction/3290
defined-data/1270 undefined bytes. Undefined bytes are00/CC, which can be zero
data. All80 raw FF25 thunks listed:29 complete stored functions,51 unassigned
listing instructions. These are classification counts, not matching percentages.

## Evidence map

Application/data: [app-lifecycle-matching.md](app-lifecycle-matching.md),
[app-dispatch-matching.md](app-dispatch-matching.md),
[exe-data-constants.md](exe-data-constants.md).
CRT/library: [crt-library-context.md](crt-library-context.md),
[full-crt-startup.md](full-crt-startup.md),
[runtime-remainders.md](runtime-remainders.md) and linked topic reports.
Layout/header: [pe-imports-and-directories.md](pe-imports-and-directories.md),
[image-base.md](image-base.md), [section-merge.md](section-merge.md),
[stack-header.md](stack-header.md), [pdb-path.md](pdb-path.md),
[pe-header-settings.md](pe-header-settings.md).

The roadmap itself changes no Ghidra database; build-default changes are recorded in the linked experiment topics and CHANGELOG.md.
Last recorded original save remains the gap-comment milestone.

2026-11-03 follow-up: [application-function-order.md](application-function-order.md)
records the measured ordering/default38. [build-environment-factors.md](build-environment-factors.md)
separates host OS from SDK/target/library/optimization and release factors.

2026-11-03: [extended-function-order.md](extended-function-order.md) extends that
result through callback, folded SDK leaves and ATL. It documents isolated linkage
and ICF controls;39 is selected,38 retained. Remaining runtime/library/import
selection order is now the next placement target.

2026-11-03: crt-selection-order.md supersedes39 with actual default40. Remaining CRT thunks/import order and data layout are open.

2026-11-03: import-selection.md closes all80 thunk placement/CC checks, corrects the stale remaining-thunk claim and records two unsuccessful IAT selection controls. Default40 retained. Next independent action: writable-data placement (+20); no blind INCLUDE permutations.

2026-11-03: mfc-support-library.md records actual default41, reducing data delta20->16 while preserving code/IAT comparisons. Original manifest dependencies were inspected for provenance; generated sidecar/resources remain unmatched.
# Current result: coverage and bounded code work (2026-11-03)

The baseline41 consolidation requested in the handoff is complete; see
[coverage-baseline41.md](coverage-baseline41.md). The explicit byte unions
supersede the earlier pending-reconciliation action below. No unchecked interval
overlaps an instruction in the retained model snapshot, so InitInstance was chosen.
One controlled source trial restored the original two-load order at01001c5d,
reducing differing nonbranch pairs43->41 with all regressions retained. The small
source change is accepted; the new EXE remains in its uniquely named run, while
archive41 and the coverage ledger remain immutable and hash-specific. See
[initinstance-matching.md](initinstance-matching.md#bounded-module-instance-local-trial-2026-11-03).
The successful one-trial question is closed. Remaining InitInstance scheduling
and branch layout require a new concrete hypothesis; layout/IAT polishing stays
deferred. No complete body/EXE/runtime equivalence claim.

