# CAB path discovery, owner lifetime and cache

Follow-up: api-cab-image.md records prefix-reader/stream source and two accepted
wrapper matches (current4522 code). Owner reload remains unresolved because its
path-failure branch consumes an uninitialized local stream slot; no NULL fix.

2026-11-08. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Evidence: artifacts/api.dll/api-cab-owner-evidence-20261108.json, original-byte checked
listings for20145169/2014483b/201448a4/201452e1/200f40a9, raw exception metadata,
and read-only live unwind instructions. No Ghidra edits; final live revision
1793741866691:f31fc448:3, unsaved_changes=false. Names below are reconstructed.

## Observed behavior

20145169 receives input in EDI and archive/relative output on stack, RET8.
It scans while tracking last dot/separator. Encountering case-insensitive .CAB
constructs the archive candidate, rejects invalid attributes, and accepts it if
not a directory. After that spelling it skips one character before copying the
relative path, without checking that this character is a separator. A bare CAB
produces empty relative output. If the last dot follows the last separator,
it replaces that separator with .CAB, requiring separator index>=1. Otherwise
it appends .CAB. These latter two branches only test invalid attributes, not
FILE_ATTRIBUTE_DIRECTORY. Source preserves these asymmetric checks, look-ahead
and output mutation. It does not introduce new bounds or path validation.

2014483b receives owner in EAX, path on stack and returns owner in EAX (RET4),
despite void decompilation. Allocation is0x124 bytes. Source now records observed
next pointer0, last-use value4, file-time pair8/c, path260 at10, image114,
entry count118, first entry11c, unknown field120. Last-use helper2012de9b has a
nontrivial application time calculation; do not rename it GetTickCount or give
its unit an unsupported name. The constructor clears fields, inserts itself
under critical section2020e29c into list2020d8e8, then invokes201449c4 ignoring
its load result. An owner return does not prove successful CAB loading.

201448a4 receives owner in ESI. It frees image114 and unlinks the owner under
the same lock; it does NOT free owner storage. Caller lifetime still matters.

201452e1 has two stack arguments, RET8. It discovers archive/relative path,
normalizes archive via20133d17 ignoring return status, then holds2020e29c while
searching case-insensitively. Hits update value4 and move to list head. A miss
allocates0x124 via20135dd9 with mode global2020d8ec and constructs the owner.
The constructor's acquisition of that same critical section is intentional in
the observed code. NULL allocation returns NULL after ordinary lock release.

## Verified unwind and source limitation

Handler201ee2e1 points at FuncInfo201f0768 (magic19930522), one unwind-map entry
at201f0760: state predecessor -1, action201ee2d3. The action loads saved raw
allocation [EBP-124h], calls200f40a9, then returns.200f40a9 conditionally frees
through private heap2020e050. No outer cache-lock release or owner unlink occurs
in that action; do not invent a lock guard. Partial construction may already
have inserted the owner; no exception-safety repair is inferred or introduced.

cab_owner.cpp implements path discovery, initialization, image/list destruction
and normal resolver flow with structured C++. A placement-storage tag and
matching placement delete express the observed constructor-failure free intent.
This is a source model, not proof of original source syntax. The fixed native
pilot profile emits NO resolver unwind handler/free action, so exception
behavior is incomplete even though source compiles. Do not call this an EH
match or a runnable replacement. No /EH flag matrix was attempted.

## Builds and stop decision

First path/owner build: api-cab-owner-20261108-001,
object FA26F9EBCF9FCCE25C82F1709A03CB6FB5DA72C8FCE80CF7C7224800624F7684.
Resolver addition: api-cab-cache-20261108-001,
object 7A26A93498D37DFA991F25318E7645FC0CCF89CBDB513B78C1B19C439A2EE424.
Both under vm-share/modules/api/runs. Retained source/receipt/log/COFF diagnostics.

| Routine | Original/native bytes | Status |
|---|---:|---|
|20145169 path discovery|376/443|unmatched, hidden EDI input/copy ABI|
|2014483b initialize owner|105/106|unmatched, original EAX input|
|201448a4 release image/unlink|128/144|unmatched, original ESI input|
|201452e1 resolver|311/259|normal-flow source; emitted unwind missing|

Ten accepted-body gates pass on final run; no uninitialized/missing-return
warnings. Complete code4450, metadata44 and partial Initialize123 unchanged.
Each new source question stops after its initial build at a concrete ABI/EH
blocker. These are distinct source additions, not repeated matching trials.
No goto, register forcing, padding, flag matrix, linked DLL or candidate execution.

## Naming suggestions and next package

Suggestions only, not Ghidra renames:20145169 unaff_EDI -> inputCabPath,
param_1 -> archivePathOutput, param_2 -> relativePathOutput (scan and writes).
201452e1 param_1/param_2 -> inputPath/relativePathOutput; puVar4 -> cabOwner,
puVar1 -> previousOwner (case-insensitive list lookup and relink).201448a4
unaff_ESI -> cabOwner. lastUseValue is deliberately neutral about time units.

Next source package:201449c4 owner loading and2014420b CAB image loading, with
stream interfaces and ownership on errors. Native calling-convention/EH profile
work remains a separate bounded question before full matching. No further
font/body tuning based only on size proximity.

2026-11-09 follow-up: see api-cab-buffer-containers.md for the live caller
xref, selector-0x34 path flow and the independently unwritten metadata pair
on Stat failure. The reload boundary remains explicit in source.
