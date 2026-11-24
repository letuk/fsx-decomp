# Baseline 41 consolidated range ledger

2026-11-03. Status: live verified on-disk identities; same-hash archived comparison evidence.

Original SHA256: `61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735`.
Rebuilt SHA256: `6953DEA3584BECDFBE03A0D55511D8B4093A593F421603B97EE3938214E8580B`.

Machine-readable evidence: [coverage-baseline41-20261103.json](../../../artifacts/fsx.exe/coverage-baseline41-20261103.json). Reproduce with `node scripts/coverage-baseline41.cjs`. Each paired claim records both addresses/extents, hashes, evidence and normalization. Independent interval partitions retain every overlapping evidence ID and count each byte once. Source/MAP names are reconstructed.

## Unique range accounting

| Region/status | Original bytes | Rebuilt bytes |
|---|---:|---:|
| imports / target-validated normalized | 1096 | 1096 |
| imports / exact raw | 1293 | 1293 |
| executable-section data / target-validated normalized | 580 | 580 |
| padding/unknown zeros / unchecked | 681 | 673 |
| PE/debug metadata / known difference | 44 | 44 |
| executable-section data / exact raw | 223 | 223 |
| PE/debug metadata / exact raw | 16 | 16 |
| executable code / target-validated normalized | 3649 | 3649 |
| executable code / exact raw | 313 | 313 |
| executable code / known difference | 1206 | 1214 |
| padding/unknown zeros / exact raw | 495 | 495 |
| unclassified executable-section data / unchecked | 32 | 32 |
| imports / diagnostic-only | 100 | 100 |
| writable state / target-validated normalized | 4 | 4 |
| writable state / exact raw | 1149 | 1149 |
| writable state / unchecked | 31 | 47 |

These are scoped byte unions, not a whole-EXE match percentage. A whole-body known-difference row marks its entire extent; it does not mean every byte differs. Exact raw initial writable bytes include virtual zero-fill, not runtime state. Target-normalized equality does not establish original addresses or source/compiler identity.

## Remaining work and selection

No unchecked interval overlaps an instruction in the retained Ghidra snapshot. This is a snapshot-based triage result, not proof that the original executable contains no hidden/unmodeled code. All 80 import thunks and their immediate CC runs are included; aliases and prerequisite comparator chains are not added repeatedly. The 18/1114 application count is retained as historical scope, not used as the total.

InitInstance (original 0x010019E0, 769 bytes; rebuilt 0x010019C0, 777 bytes) is selected for bounded code work. Its 21 paired CFG blocks and 246 instructions are diagnostic only; 43 nonbranch positions differ. ShowErrorMessage (0x01001D30, 437 bytes) retains two normalized byte differences at 0xDB/0xDC. No substantial unchecked application code range was exposed.

Unassigned nonzero executable-section interval: 0x01002DC4..0x01002DE3 (32 bytes, metadata-area data, no snapshot instructions); mapping remains unchecked. Other unassigned .text bytes are 00/CC, with ownership unresolved. Writable unassigned bytes remain 31 original / 47 rebuilt. Do not reinterpret zero runs as absent variables or recovered padding.

All 137 import identities/hints are checked. The 100 non-null descriptor bytes are diagnostic-only in this ledger; pointer parsing and import-set equality do not certify physical descriptor bytes. Import names/terminators have explicit raw checks. 103 within-DLL positions still differ. Layout/IAT refinement remains deferred.

## Comparison policy and limitations

The VA partitions cover all 9728 raw .text bytes in each image and .data virtual extents 1184/1200. File-scope rows separately locate headers, raw sections, resources and certificate ranges. Resources are excluded from the current reconstruction task. Header effects are not excluded: selected PE settings pass, but whole headers, checksum, timestamps, original CodeView GUID, certificate/signature and final metadata policy remain unresolved. CodeView signature, age and path bytes are separately checked; the GUID and debug directory remain different. No whole-file identity is claimed.

Archived evidence is accepted only for the exact selected EXE hashes. Raw claims and explicit vtable targets are rechecked by this script. Normalized body claims retain the exact scopes of their existing comparators rather than introduce a new masking scheme. No builds are needed for this reconciliation. Current canonical source differs from effective archived source by the documented callback linkage staging; archive/source and source-hashes.json describe compiled input.

Read-only Ghidra check: fsx.exe current, api.dll also open; InitInstance revision 1793671788023:335a223f:3, unsaved_changes=false. No tabs, database names/types, source defaults, save or backup changed. No new variable rename is supported by this accounting. Static checks do not establish runtime equivalence; no candidate EXE was executed.

## Subsequent bounded code result

After this baseline41 ledger was complete, one InitInstance source trial recovered the original five-byte load sequence at 0x01001C5D. The candidate has a different EXE hash and is not silently substituted into this ledger. See [initinstance-matching.md](initinstance-matching.md#bounded-module-instance-local-trial-2026-11-03). Canonical source now contains the accepted local; archive41 remains the immutable coverage baseline.
