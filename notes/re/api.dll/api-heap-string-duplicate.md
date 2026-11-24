# API private-heap string duplicate

## Live verification (2026-11-15)

`api.dll:20137efe` is export 158. Its original body is 178 bytes.
The function returns NULL for NULL input. Otherwise it allocates
strlen(input) plus one bytes on the private heap with flags zero.
It retries once after failure and calls kernel32!lstrcpyA at
`20137fa1`. EAX receives the allocation pointer from ESI after the
copy, independently of the copy API return. The caller owns the
allocation. `DuplicateHeapStringPilot` is the reconstructed source
name. Suggested Ghidra name `DuplicateHeapString` for the existing
`FUN_20137efe` follows from the allocation and copy. No rename
or signature edit was applied.

The 67-byte failure path at `20137f59` is raw equal to the path at
`2013238d` in `api.dll:201322e2` (`AllocateNumberedNamePilot`).
Both are consistent with inlining the known allocator
`api.dll:20135dd9` with retry enabled. The exact original source
expression and failure mode constant are hypotheses. The duplicate
uses flag zero. The numbered-name helper uses HEAP_ZERO_MEMORY.

## SP1 source and comparison

The older duplicate source in `suffix_registry.cpp` spelled out the
allocation. That state is retained as an expanded control. The regular
implementation now lives in `duplicate_heap_string.cpp` and calls
`AllocatePrivateHeapMemoryPilot`. VC8 SP1 /GL and /LTCG inlines that
call. The expanded and shared linked probes each emit 177 bytes.
The five differing raw bytes belong to the HeapAlloc, GetModuleHandleA,
GetCurrentProcess, TerminateProcess and lstrcpyA IAT operands. Both
probe listings and their PE import tables identify the same targets.

The fresh integrated run `api-heap-string-integrated-sp1-20261115-001`
preserves that result. The 29-byte entry at `20137efe` is raw exact.
Both 67-byte allocation failure spans match after mapping the five
imports and the three operands referring to verified globals. These
are partial linked comparisons. The complete duplicate body differs
in register saves, heap initialization, handle loads and the final
lstrcpyA argument load. Both rebuilt string helpers test the heap
flag with a memory CMP. Both originals load the flag and TEST the
register. This gives a shared point of comparison for a future
allocator-source experiment.

The worker receipts, staged source and commands are retained in the
run directories. The integrated DLL SHA256 is
`e1f9757663e6e38a031021f7d82216a84b8f441d24cd9020c7ba50f84172041d`.
See the [integrated report](../../../artifacts/api.dll/api-duplicate-heap-string-integrated-comparison-20261115.json)
and [expanded control](../../../artifacts/api.dll/api-duplicate-heap-string-control-comparison-20261115.json).
The three previously established linked checks and all twenty COFF
regressions pass. The fresh COFF run is
`api-heap-string-integrated-regression-20261115-001`.
Both dashboard listings are present. Ghidra reported saved,
modification 3, and remained read-only. Runtime remains open.

Replay with `-NumberedNameSdkProbe -AllocatedNameSdkProbe -WholeProgram
-DuplicateHeapStringProbe`. Add `-DuplicateExpandedHeapProbe` on
the second run for the expanded control. Compare each run with
`scripts/compare-api-numbered-name-sdk.cjs` and the retained no-LTCG
control `api-allocated-name-control-20261115-001`.

## Complete linked matches from the shared allocator (2026-11-15)

Live verified in `api.dll:20135dd9` and both callers. The allocator
reads the initialization flag at `2020e054`. The two original
callers use a load and a register TEST. The previous regular source
produced a memory CMP in both. The SP1 volatile-read experiment
produces the original load, register and scheduling in each caller.
A separate compiler-barrier experiment produces a register test but
does not match either complete body. The local volatile reference is
a verified reconstruction technique. The original C++ qualifier
remains a hypothesis.

The regular allocator now reads the flag through a local
`const volatile int&`. A fresh linked build
`api-heap-flag-integrated-sp1-20261115-001` verifies both complete
bodies against the pinned original PE. This supersedes the old
incomplete linked results above.

| Original VA | Reconstructed source name | Linked VA | Complete bytes | Verified address fields |
|---|---|---|---:|---:|
| `20137efe` | `DuplicateHeapStringPilot` | `200e1200` | 178 | 14 |
| `201322e2` | `AllocateNumberedNamePilot` | `200e10c0` | 313 | 20 |

The linked DLL SHA256 is
`8b718d62a85fc7cbaf620487e3d708724563fab037f0ce6b9d2b2d791d1d6e8b`.
The [comparison](../../../artifacts/api.dll/api-heap-flag-integrated-comparison-20261115.json)
retains both byte ranges and all target mappings. The worker receipts,
source snapshots and build commands are retained. Three prior linked
gates and all twenty fresh COFF checks pass. The baseline for
`--require-heap-matches` is the integrated linked profile; the barrier
control rejects. The separate COFF coverage and runtime verification
are unaffected by these linked results. Both dashboard listings are
present. Ghidra was read-only and reported saved, modification 3.

Replay the integrated profile with `-NumberedNameSdkProbe
-AllocatedNameSdkProbe -WholeProgram -DuplicateHeapStringProbe`.
Pass `--require-heap-matches` after the retained no-LTCG control path
when using `scripts/compare-api-numbered-name-sdk.cjs`. The alternate
compile context can be replayed with `-HeapFlagProbe ReadBarrier`.

## Standalone linked allocator (2026-11-15)

Live verified at `api.dll:20135dd9`. The same retained SP1 build
links the standalone `AllocatePrivateHeapMemoryPilot` at `200e1360`.
Its complete 174-byte body matches after thirteen verified address
fields. The allocator, numbered-name builder and string duplicate
also share the same 67-byte original failure block. This supersedes
the prior outstanding standalone match. No source change was needed.

See the [comparison](../../../artifacts/api.dll/api-standalone-allocator-sp1-comparison-20261115.json).
Add `--require-allocator-match` to the existing comparison command.
The earlier memory-CMP control rejects. Both caller matches and
the prior three linked checks pass. Runtime and the body of the
initializer at `2013623e` remain separate. Ghidra was read-only and
reported saved, modification 3.

## Linked initializer (2026-11-15)

Live verified in `api.dll:2013623e`. A local volatile flag read supplies
the tested zero reused for both HeapCreate arguments. The regular source
now uses this form. The original C++ qualifier remains a hypothesis.
The fresh SP1 integrated build matches the complete 144-byte initializer
at `200e12c0` after nine verified address mappings. This supersedes the
earlier unmatched initializer. The old memory-CMP build rejects the
`--require-initializer-match` gate. All six prior linked matches survive.
The fresh factory source-pass build passes twenty COFF checks.
The original and rebuilt listings are retained in the dashboard.
Ghidra was read-only and reported saved, modification 3. Runtime is open.

Run `api-heap-initializer-integrated-sp1-20261115-001` has DLL SHA256
`5a9288f50ccc64be0a6adacfdb831cfe229d54594ea11bd2bf6c6806dccae7c4`.
The [comparison](../../../artifacts/api.dll/api-heap-initializer-integrated-comparison-20261115.json)
records the verified import and global mappings.
