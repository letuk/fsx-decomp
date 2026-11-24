# API.DLL broad source pass — 2026-11-07

Status (2026-11-25): **broad source generation retired**. The generated
snapshot and its classification ledgers are in
[`archived-source/`](../../../artifacts/api.dll/api-source-pass-20261107/archived-source/).
The former generation scripts are preserved in
[`archived-generators/`](../../../artifacts/api.dll/api-source-pass-20261107/archived-generators/)
with their original paths and assumptions in the script contents.
The opt-in `-SourcePass` build profile uses the frozen snapshot for
existing COFF regression checks. The whole-DLL implementation remains unfinished.

The original broad pass used VS2005 SP1. No extra agents were used.

Target: `original/api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
All addresses below are original VAs. Generated and pilot names are reconstructed.

## Live evidence and whole-model inventory

Live verified: api.dll was already selected; 16 programs were open. Every MCP
analysis call specified api.dll. No tab switch or database mutation was made.
Final readback: `unsaved_changes=false`, modification number 3; original hash
matches the pinned file. This work needed no database save or backup because it
changed only source, scripts, documentation and evidence files.

`ExportApiSourcePass.java` exported all 5,077 internal model functions to per-VA
`.c`, `.asm` and `.json` evidence; all decompiler requests succeeded. Another 789
functions are external declarations, giving the current model total 5,866.
This supersedes the older 5,865 model count, not any coverage claim.
`ExportApiGlobalTypes.java` separately retained inferred global symbol types.

`ExportExeInventory.java` was reused read-only with api.dll explicitly selected.
The listing contains 903,143 instruction bytes, 80,622 defined-data bytes and
148,779 undefined bytes in the executable block. Of those instructions, 7,406
instructions / 23,619 bytes are outside model functions. The source-pass verifier
checks all 321,805 exported function instructions / 879,524 bytes against the
original PE. Successful decompilation does not establish complete real control
flow: the existing initialization/main-window/language inventories remain
authoritative for their documented success-flow scopes. In particular, shutdown
20120ed5 is not an exported standalone function in this model.

Evidence root: `artifacts/api.dll/api-source-pass-20261107/`. The model export is a broad
automated collection, not a claim of manual semantic review of every function.

## Source and native compilation

`archived-source/leaf_candidates.cpp` contains **567** compiled source
candidates. Its historical filename also covers functions with ordinary calls
and SDK imports. Scalar widths, direct RET stack cleanup, compatible inferred
callee prototypes, global identities and explicit pointer conversions drive
generation. An unused EDX parameter models eligible ECX member receivers.
Only full native comparison can accept such a candidate; inferred declarations
alone are not ABI evidence. Primitive-pointer casts retain x86 address values,
not reconstructed ownership or complete layouts.

All 5,077 internal entries have an automated-route disposition in
`archived-source/source-pass.json`. Examples of deferred categories:
1,447 storage/control-flow cases; 1,016 caller/callee argument-count conflicts;
193 entries with retained native compiler contract diagnostics. Categories are
ordered by the generator, not an exhaustive independent defect classification.
Pending automated rows can already have hand-maintained pilot source.

Build profiles remain `/O2 /Oy- /hotpatch /GS /MD /Zi /GL- /Gy /GR-`, compiler
14.00.50727.762. These settings are a tested candidate, not proven original flags.
Each run retains effective source, command, hashes, log and receipt. Initial
compiler failures are kept. Repeated preparation builds filtered diagnosed
contradictions and repaired pointer syntax; they were not flag/layout searches
or repeated byte-tuning trials. Uninitialized-return warnings were deferred,
never silenced by supplying an invented initial value.

Latest run: `vm-share/modules/api/runs/api-source-message-loop-20261107-001`.
Object SHA256:
`141E6DC83FC6DF94F394F333CE96E889607A7CEED903DD331383597822C72623`.
It is a native **COFF object with unresolved dependencies**, not a linked DLL.
The original worker, installation and toolchain were unchanged. Worker is ready.

## Hand-reconstructed additions

| Address | Source / behavior | Original model bytes | Native bytes | Status |
|---|---|---:|---:|---|
|20134495|file_enumeration_open_next.cpp / open|503|512|compiled, unmatched|
|2013468c|file_enumeration_open_next.cpp / next|669|714|compiled, unmatched|
|201350be|suffix_registry.cpp / copy selected suffix|86|85|compiled, unmatched|
|201336ab|suffix_registry.cpp / uppercase and lookup suffix|205|207|compiled, unmatched|
|20133778|suffix_registry.cpp / register dynamic suffix|255|250|compiled, unmatched|
|20137efe|suffix_registry.cpp / duplicate private-heap string|178|177|compiled, unmatched|
|2012294b|message_loop.cpp / idle scheduling|192|205|compiled, unmatched|
|20122a0b|message_loop.cpp / foreground-idle hook|214|214|full target-validated object match|
|20122ae1|message_loop.cpp / message pump|361|370|compiled, unmatched|

Live-verified contracts: enumeration advances before returning a record; skips
`.` and `..`; directory records do not write writable; timestamps use the
observed six-WORD ordering. The original empty-directory preceding-byte read and
upper flag-bit preservation were retained, not silently repaired. Suffix lookup
uppercases input in place; copy201350be always returns zero. Allocation retries,
error UI and termination paths remain source-visible.

20145110 receives path on stack and WIN32_FIND_DATAA output in ESI (caller
201345de/201345fd). 20144df0 receives owner on stack, cursor in EDI and output in
ESI (201348af..201348c0). These source interfaces explicitly name all arguments
but **do not implement the original register ABI**. Their implementation and
alternate-backend record layout remain open.

20122ae1 retains callback-selected message storage, WM_QUIT cleanup, three-stage
filtering, quit posting, Sleep20/Sleep0 decisions and the unsigned >30 idle
counter. 2012294b preserves reentrant -1, tick throttling and cursor suppression.
Observed mode VTable slots 0x18,0x20,0x30 and controller slot0x1c are represented
in source only. Unknown slots are explicit placeholders, not recovered types or
class sizes. Stored Ghidra VTable types/signatures were not edited.

The decompiler reports void for20122a0b, but assembly20122aca..20122ade preserves
CallNextHookEx's EAX through the established cookie check and RET12. Source returns
LRESULT. The full214-byte match,13 separately verified fields, and rejecting
opcode/addend/callee controls support this correction. This supersedes treating
the raw decompiler void result as the hook's actual machine return contract.

## Matching result and gates

All eight earlier accepted-body gates pass unchanged: 1,120 complete code bytes
and44 metadata bytes, with Initialize's123 partial bytes still separate.

The generated pass accepts **88 additional disjoint model bodies / 3,116 bytes**.
46 bodies /1,935 bytes are raw-equal; the others use individually validated global,
callee or IAT reference fields. Their addresses and fixed regression requirements
are in `scripts/matching/api-source-pass-accepted.json`; per-field identities and
all unmatched results are in `source-pass-comparison.json` in the run.
The manual foreground-idle hook adds another214 bytes, disjoint from that list.
Combined complete-body evidence: **4,450 unique code bytes**, metadata44 separate.
No whole-image percentage, data implementation, final export table or runtime
equivalence is inferred. A matched caller does not certify its dependencies.

`scripts/matching/test-api-source-pass.ps1` runs ten comparators, includes fixed
new-body regressions and rejects uninitialized/missing-return compiler warnings.
`inspect-api-source-pass.cjs` retains manual sizes/references without treating
equal sizes as matching evidence. No mismatch was hidden by padding or masking.

## Provisional naming suggestions, not Ghidra renames

- 201336ab: `param_1` -> `mutableSuffix`, `param_2` -> `outputSelector`: input is
  uppercased, then its table index is written through the second argument.
- 20133778: same suggestions; it inserts an unknown uppercase suffix and writes
  its dynamic selector. Source uses `suffix`/`selector`; Ghidra names unchanged.
- 20145110: `unaff_ESI` -> `findDataOutput`; caller passes state+4, independently
  verified as WIN32_FIND_DATAA. This is a hidden register parameter, not a local.
- 20144df0: `unaff_EDI` -> `enumerationCursor`, `unaff_ESI` -> `findDataOutput`;
  cursor interpretation is provisional, output width/uses are assembly-supported.
- 20126622: `param_1` -> `rangeOwner` (provisional): fields+4/+8 delimit a range
  whose byte difference is divided by28. This does not identify its full class.

## Remaining work

The user's whole-api.dll request is **not complete**. Remaining work includes
thousands of unresolved bodies/contracts, native flow outside current function
bounds (especially initialization/shutdown), indirect calls and structures,
alternate enumeration ABI/backend, globals and initializers, DLL entry/CRT,
288-export reconciliation, resources/imports/relocations and final linking.
The first broad pass and its build/gate pipeline are ready to continue; there is
no runnable replacement or blanket claim that all decompilations compile.
