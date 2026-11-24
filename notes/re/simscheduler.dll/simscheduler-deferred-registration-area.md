# simscheduler.dll deferred registration area

## Live verified

`FUN_20cc3490` takes the context in EDX and the entry identity in
EBX. The three stack arguments are classification, flags, and add/remove
operation in that order. Its three exits use `ret 0xc`. It scans the
pending array for the identity. A duplicate operation returns zero.
An opposite operation cancels the old record by shifting subsequent
records over it. A new identity appends a 16-byte record.

The queue starts at context offset `+0x80`. Its minimum layout is:

| Queue offset | Observed field |
| --- | --- |
| `+0x00` | 32-bit count |
| `+0x04` | 32-bit capacity |
| `+0x08` | Signed 16-bit growth increment |
| `+0x0a` | 16-bit storage flags |
| `+0x0c` | Pointer to pending records |

Record offsets `+0x00/+0x04/+0x08/+0x0c` hold the operation,
entry pointer, flags and classification. `FUN_20cc41d0` resizes
the queue with the queue address in EDI and requested count in EAX.
The original uses growth 5 when the increment is nonpositive. Flag
`0x04` selects doubling. Otherwise capacity rounds up to a growth
multiple. The helper allocates a replacement, copies live records,
releases the old buffer and sets flag `0x01`.

The original imports resolve to `api.dll:0x20135dd9` for allocation,
`api.dll:0x20135f51` for release and `api.dll:0x20135f9c` for the
overlapping move. `0x20cc6e86` is the `msvcr80.dll` memcpy thunk.
Allocator input is loaded from `DAT_20cc836c`. C++ names and layouts
are minimum reconstructed models.

## Matching

| Function | Original | Native COFF | Default LTCG | EBP LTCG |
| --- | ---: | ---: | ---: | ---: |
| `0x20cc3490` | 221 | 182 | 172 | 174 |
| `0x20cc41d0` | 235 | 211 | 211 | 225 |

The linked EBP probe keeps the entry in EBX, restores `ret 0xc`,
and keeps the resize inputs in EDI/EAX. It puts the enqueue context
in EAX rather than EDX. Code scheduling and stack layout differ.
The comparison checks the full bodies, validated API import identities,
the linked resize call and the original and rebuilt memcpy thunks.
Builds from before a fixed preprocessor isolation error are retained
as failed receipts. They used the same two LTCG approaches.

The C++ queue deliberately returns zero on growth failure or an
out-of-range count. The original appending path continues after a
resize call without testing its result and clamps the count. This
behavioral difference prevents a match across those paths. The
three approaches retain diagnostics and both original listings.

- [Original listings and API evidence](../../../artifacts/simscheduler.dll/pending-registration-ghidra.json)
- [Enqueue comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-pending-ltcg-20261116-004/comparison-20cc3490.json)
- [Resize comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-pending-ltcg-20261116-004/comparison-20cc41d0.json)
- [Default LTCG control](../../../vm-share/modules/simscheduler/runs/simscheduler-pending-ltcg-20261116-003/comparison-20cc3490.json)
- [Native COFF control](../../../vm-share/modules/simscheduler/runs/simscheduler-pending-coff-20261116-002/comparison-20cc3490.json)

## Area boundary

`FUN_20cc2ce0` queues an add during an update. Its classification
comes from comparison against `DAT_20cc8368`. `FUN_20cc2e80`
queues a removal for a found entry and marks the live record with
flag `0x02`. It discards the queue helper's result. The time update
at `FUN_20cc30d0` drains the pending records through slots
`+0x38` and `+0x3c` after the update guard is cleared. The immediate
registration paths and entry lookup, pool and container management
are covered in the [immediate registration pass](simscheduler-immediate-registration-area.md).
The full bodies of `FUN_20cc2ce0` and `FUN_20cc2e80` now have
native COFF diagnostics.

Existing scoped matches and GPS regression passed. Full-module link
and runtime verification remain open. Ghidra read-only, save states
unverified.
