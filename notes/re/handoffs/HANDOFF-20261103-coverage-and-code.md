# Handoff: consolidate baseline41 coverage, then resume code reconstruction

Prepared 2026-11-03 for a new chat. Project root:
G:\Projects\Reversing\fsx-decomp

## User direction and next assignment

The user wants fsx.exe reconstructed as closely as possible, ideally matching.
DLL implementation and resource reconstruction are outside the current focus.
After several small layout experiments, the user explicitly wants to avoid an
endless loop. The next phase is:

1. Consolidate a single, same-binary code/data/metadata coverage ledger for the
   selected baseline. Determine what is actually checked, different or unchecked.
2. Use that ledger to choose the largest useful unchecked code range, or return
   to InitInstance if no larger useful range is found. Make a bounded,
   evidence-led code reconstruction improvement.

Do not begin by chasing the16 data bytes,32-byte code offset or103 IAT positions
again. These are recorded residuals, deferred until genuinely new evidence.
Read project AGENTS.md, notes/re/STATUS.md, SOURCES.md and relevant topics first.
Historical chat answers and early paragraphs of chronological notes/README files
can be stale; the current baseline and policy in this handoff supersede old next
steps. Project documentation must be English; speak German with the user.

## Selected baseline and immutable inputs

Original: original/fsx.exe, x86, image base0x01000000, SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

Selected rebuild: vm-share/modules/fsx/archive/41-rtm-mfc-support/fsx.exe, SHA256:
6953DEA3584BECDFBE03A0D55511D8B4093A593F421603B97EE3938214E8580B.
Actual originating job:
vm-share/modules/fsx/runs/mfc-support-default-20261103/candidates/mfc-support-default-20261103-001.
The archive includes effective compiled source, response input, source hashes,
request/receipt, build command/log, EXE/PDB/MAP/manifest, disassembly/headers and
comparison.json. Earlier baselines and rejected trials are retained.

Canonical src is not identical to effective staged source: the controller removes
only static from GetCurrentMessagePilot for the current ordering mode. Use the
archive's source/ and source-hashes.json to describe what was actually compiled.
Original callback linkage is unknown; the external linkage is reconstructed.

Compiler/linker remain XP-installed VS2005 SP1 .762; installed headers, MFC import
and ATL libraries remain. Selected msvcrt.lib and mfcs80.lib are pinned RTM media
copies, with the matching mfcs80.pdb. There was no RTM compiler installation or
runtime DLL replacement in these recent milestones. The actual request/build.cmd
is authoritative for flags and library hashes.

## Established results and remaining differences

- 18 selected application/helper/wrapper bodies,1114 bytes, pass exact raw or
  individually target-validated normalized checks. CRT/ATL bodies have additional
  separate checks. Do not turn these counts into whole-EXE coverage percentages.
- 88 selected ordered symbols and36 additional mapped runtime/framework symbols
  have original relative placement at uniform-32. Counts contain aliases and
  differ from a distinct-function or byte-coverage count.
- All80 executable FF25 import thunks have correct unique import targets and
  original-32 placement; their immediate trailing CC runs match.
- All137 import identities/hints match;103 within-DLL entry positions differ.
- .data virtual1200 versus1184 (+16), raw512 in both. The current ledger maps
  30 disjoint spans/1153 bytes in each;31 original/47 rebuilt bytes are unassigned.
  Unassigned zero bytes are not automatically padding or absent variables.
- The standard CodeView-to-SafeSEH area has32 fewer zeros than the original.
  An optional synthetic gap probe exists but remains disabled. It is not recovered
  original source or an established release-tool explanation.
- ShowErrorMessage original0x01001d30:437 bytes, two remaining byte differences
  at offsets0xdb/0xdc (TEST EAX,EAX versus CMP EAX,EBX with EBX zero).
- InitInstance original0x010019e0:769 original/777 rebuilt bytes.246 instructions
  in each,21 CFG-paired blocks,43 differing nonbranch instruction pairs in the
  diagnostic comparison. That is not an exact-match certification.
- Original resource24/1/1033 requests MFC/CRT8.0.50608.0. This supports dependency
  context, not identification of the original compiler or exact library files.
  Generated sidecar still contains additional dependencies; resources/manifests
  are not claimed matched. Metadata timestamp/GUID/certificate policy remains open.
- No candidate has been executed as a replacement FSX. Passing static checks
  does not establish runtime equivalence or a complete working executable.

## Useful local evidence and tools

Start with:

- notes/re/fsx.exe/exe-matching-roadmap.md
- notes/re/fsx.exe/mfc-support-library.md (adoption of41 and actual controls)
- notes/re/fsx.exe/writable-data-layout.md (original/rebuilt writable mappings)
- notes/re/fsx.exe/data-contribution-alignment.md (latest rejected native-TU trial)
- notes/re/fsx.exe/import-selection.md (closed thunk placement and rejected IAT controls)
- notes/re/fsx.exe/initinstance-matching.md and show-error topic files as needed

Existing inventory:

- artifacts/fsx.exe/exe-inventory/summary.json, inventory.csv, inventory.html
- artifacts/fsx.exe/exe-inventory/ghidra-export.json (snapshot, not necessarily live state)
- artifacts/fsx.exe/exe-inventory/unassigned-ranges.json, undefined-ranges.csv
- artifacts/fsx.exe/writable-data-layout41-20261103.json
- artifacts/fsx.exe/all-import-thunk-placement41-20261103.json
- artifacts/fsx.exe/pdb-contributions41-20261103.json
- artifacts/fsx.exe/data-contribution-analysis41-20261103.json
- selected archive/comparison.json and InitInstance diagnostic reports

The Ghidra snapshot has212 model functions (77 internal,135 external declarations),
140 mixed-origin inventory rows,129 unassigned instructions in63 ranges. These
are database/inventory classification counts, not a count of recovered original
functions or a direct list of missing source functions. Undefined executable
bytes include zeros/CC; raw section size includes data and alignment too.

scripts/inventory-exe.ps1 and inventory-app-vtable.ps1 default to41.
scripts/compare-data-constants.ps1 enforces a chain of CRT/ATL/data body gates.
Other comparison/verification scripts cover application bodies, imports, header,
debug record and order. Inspect their exact scopes before combining results.
Some experiment comparators intentionally default to older controls; do not
globally replace every historical baseline path with41.

scripts/inspect-pdb-contributions.ps1 reads rebuilt MSF7/DBI-v60 records, verifies
EXE/PDB GUID+age and cross-checks20 public data owners. Three known pilot globals
lack contribution entries in this table; their regions are not padding. The
reader deliberately rejects unsupported large directory maps/formats and a
wrong-PDB control was tested. No original PDB is available.

## Phase 1: concrete deliverable and completion criteria

Create a machine-readable range ledger and a concise readable report for41.
Reuse existing checks rather than create another unrelated summary counter.

For each claimed range record original/rebuilt address and extent, evidence
file/check, binary hash, status and normalization rules. Distinguish exact raw,
target-validated normalized, known difference, diagnostic-only, and unchecked.
Do not certify a whole function from a checked prefix or CFG alignment. Resolve
overlaps by interval union, retain the origins of overlapping evidence, and do
not count aliases or prerequisite gate chains repeatedly.

Separate executable code, executable-section data, writable state, imports,
padding/unknown zeros, and PE/debug metadata. State the comparison scope and open
resource/certificate/metadata policy explicitly. Source presence is not byte
coverage. Original raw binary scans and Ghidra snapshots answer different questions.

Keep all evidence paired to the same EXE hash. A report from another trial may
explain provenance but must not silently certify41. Unknown mappings remain
unknown. For unsaved/live Ghidra state, check explicitly rather than infer from
documentation.

Phase 1 is done when a reader can see exactly which included ranges are verified,
known different or unchecked, and identify the highest-value remaining code work
without relying on a misleading match percentage. Report material limitations.
Do not turn this into a large generic reporting framework or spend the phase
re-running unchanged passing builds.

## Phase 2: selection and stopping rules

Choose one concrete code target after phase1, explaining why it is useful:
prefer a substantial unchecked internal code range with clear callers/outputs;
otherwise use InitInstance's concrete instruction/reference differences. Database
gaps that are merely import thunks or padding are not new application logic.

Use one explicit hypothesis and at most three targeted trials per question.
Each trial needs a predicted observable outcome and existing regression gates.
After the bound, accept an improvement or record the negative result and change
topic. Do not automatically start another flag/source permutation series.
No adoption for a smaller file if established bodies/references/layout invariants
regress. Deferred layout/IAT questions require new evidence to reopen.

For this next phase, a successful bounded outcome is either a new verified code
range/improved body match or a precise residual with a defensible reason to stop.
No guarantee of eventual byte-identical reconstruction is established.

## Build and Ghidra operation

XP worker was ready/no job at handoff. Recheck vm-share/worker-state.json; do not
assume the VM is still running in a later chat. The worker only compiles/dumps
submitted jobs and does not generate variants autonomously. No VM UI/autostart
changes are needed. Do not run candidate EXEs.

If phase2 actually needs a fresh reviewed source build:

    ./scripts/matching/start-search.ps1 -RunId <unique-id> -MaxCandidates 1 -Target InitInstance

Read scripts/matching/README.md and controller code before staging changes.
Keep unique run IDs; no overwrites or baseline promotion without verification.

Ghidra may have fsx.exe and api.dll open. Check target/state, explicitly specify
program fsx.exe for this work, preserve established types/names, and let the user
control tab changes. No Ghidra edits occurred in the recent linker/data milestones;
do not claim a new DB save/backup. New edits require appropriate backup/save and
documentation. Supported naming suggestions must identify current/proposed names,
evidence and provisional status, separate from actual renames.

Update STATUS/topic/CHANGELOG after significant work. Do not restart completed
layout experiments merely because the new chat lacks conversational context.
