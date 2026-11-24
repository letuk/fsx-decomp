# simscheduler.dll immediate registration

## Live verified

The context table at `0x20cc1200` is installed by the instructions
at `0x20cc27ce` and `0x20cc296f`. Its slots `+0x38/+0x3c`
resolve to `FUN_20cc2ce0` and `FUN_20cc2e80`. Both receive an
explicit context on the stack and return an integer success value
with `ret 0xc` and `ret 8`, respectively. The drain in
`FUN_20cc30d0` ignores these results.

- `FUN_20cc2ce0` checks pending mode and duplicates, acquires a
  timed record, initializes its timestamp, callback and flag bits,
  updates the active context, then inserts into the live array and
  key index. The diagnostic retains the observed null-acquisition
  path and the address-plus-four duplicate test.
- `FUN_20cc2e80` cancels a pending add if the key has no live
  index record. It marks live records when removal is queued.
  Immediate removal deletes matching live-array references,
  adjusts the cursor, erases the key record and hands the entry
  back to the pool helper.
- `FUN_20cc4050` is an unsigned lower-bound lookup on 8-byte
  records of key and entry pointer. It stores the insertion
  position through an optional output pointer and returns
  equality in AL. The original index argument is in EAX.
- `FUN_20cc3b70` scans the live pointer array backward and removes
  all copies of the entry. `FUN_20cc3c90` removes one key-index
  record after lookup. Both use the overlapping move import at
  `0x20cc10e8`, which resolves to `api.dll:0x20135f9c`.

The context index begins at `+0x70` with count at `+0x00` and
records pointer at `+0x0c`. Its source type and slot names are
reconstructed minimum models.

## Matching

| Address | Original | Native COFF | Helper LTCG |
| --- | ---: | ---: | ---: |
| `0x20cc2ce0` | 312 | 279 | Pending |
| `0x20cc2e80` | 283 | 267 | Pending |
| `0x20cc4050` | 111 | 87 | 88 |
| `0x20cc3b70` | 78 | 76 | 75 |
| `0x20cc3c90` | 84 | 82 | 87 |

The COFF comparator validates all 20 reference operands across the
five complete functions and checks both table pointers and slots
against the original HIGHLOW fixups. The linked helper probe resolves
the same move import and the internal lookup call. Its lookup uses
EBX for the index rather than EAX and a plain RET with caller cleanup.
The two main functions still model unreconstructed helpers using
explicit arguments incompatible with the original internal register
ABIs. Complete matches remain open. No inline assembly was used.

- [Original Ghidra listings and table evidence](../../../artifacts/simscheduler.dll/immediate-registration-ghidra.json)
- [Registration comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-immediate-registration-20261116-003/comparison-20cc2ce0.json)
- [Unregistration comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-immediate-registration-20261116-003/comparison-20cc2e80.json)
- [Linked lookup comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-entry-helpers-20261116-001/comparison-20cc4050.json)
- [Linked array removal](../../../vm-share/modules/simscheduler/runs/simscheduler-entry-helpers-20261116-001/comparison-20cc3b70.json)
- [Linked index removal](../../../vm-share/modules/simscheduler/runs/simscheduler-entry-helpers-20261116-001/comparison-20cc3c90.json)

Live verified at `0x20cc3b70`. ESI points to the live-list member
and EAX points to the entry pointer. The reconstruction passes the
list directly and snapshots the target once before the guarded
reverse scan. The second linked trial reproduces the single return
and matches all 78 bytes after mapping the IAT operand at +0x37
to api.dll ordinal 110. This supersedes the 75-byte linked
diagnostic. Native COFF and runtime verification remain separate.
Ghidra was read-only and its save state was not verified.

- [Linked removal match](../../../artifacts/simscheduler.dll/live-entry-removal-comparison.json)

The immediate operation pass stops at the entry pool and insertion
container boundary. Next are `FUN_20cc35d0` and `FUN_20cc3770` for
the pool, then `FUN_20cc3bd0`, `FUN_20cc3b00`, and `FUN_20cc3c20`
for insertion. The nine exported COFF matches, group 0x20 callback,
linked Flight save and GPS regression passed. Full DLL linking and
runtime verification remain open. Ghidra read-only, save state unverified.
