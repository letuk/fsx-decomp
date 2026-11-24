# simscheduler.dll entry-container insertion

Live verified in simscheduler.dll. The live-pointer array at context
`+0x60` uses four-byte elements. The sorted key index at `+0x70`
uses eight-byte records. Both have count, capacity, growth increment,
flags, and storage pointer at relative offsets `+0/+4/+8/+0xa/+0xc`.
The C++ names and layouts are reconstructed minima.

- `FUN_20cc3bd0` appends a live entry. `FUN_20cc3b00` inserts
  at a supplied position and shifts the tail by four bytes.
- `FUN_20cc3c20` places the key and entry in the sorted index. It
  uses `FUN_20cc42c0` for the empty index, or calls `FUN_20cc4050`
  for the lower bound and `FUN_20cc4320` to shift the eight-byte
  records. Duplicate keys return zero.
- `FUN_20cc3f50` and `FUN_20cc40d0` resize the two arrays. Growth
  defaults to five when nonpositive, doubles under flag `0x04`,
  or rounds up to an increment multiple. Their count limits are
  `0x1fffffff` and `0x0fffffff` respectively. The resize paths copy
  retained elements and release the old allocation.

| Address | Original | Native COFF | Linked LTCG |
| --- | ---: | ---: | ---: |
| `0x20cc3bd0` | 72 | 61 | 55 |
| `0x20cc3b00` | 106 | 86 | 77 |
| `0x20cc3c20` | 94 | 100 | 93 |
| `0x20cc3f50` | 239 | 240 | 233 |
| `0x20cc40d0` | 239 | 240 | 233 |
| `0x20cc42c0` | 86 | 61 | 50 |
| `0x20cc4320` | 88 | 86 | 72 |

Both VC8 profiles compile idiomatic C++. The linked probe uses the
original API allocation, release and overlapping move imports and
the MSVCR80 memcpy thunk. It restores the container input in EAX
and `ret 4` for the live-array insertions, and EDI/EAX for the
resize helpers. The seven bodies remain diagnostic. Stack slots,
instruction scheduling and some register assignments differ.

The C++ insertions return without writing when a checked resize
fails or the maximum count has been reached. The original insertion
paths continue after calling the resize helper and clamp the requested
count. This remains a behavioral difference outside successful growth.
The caller-level failure contract requires runtime verification.

- [Original Ghidra listings](../../../artifacts/simscheduler.dll/entry-insertion-ghidra.json)
- [Linked index insertion comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-insertion-ltcg-20261116-002/comparison-20cc3c20.json)
- [Linked live resize comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-insertion-ltcg-20261116-002/comparison-20cc3f50.json)
- [Native resize comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-entry-insertion-20261116-002/comparison-20cc3f50.json)

All seven paired listings and reports are in the dashboard. Existing
native COFF matches, linked Flight save, and GPS regression passed.
Full DLL linking and runtime verification are pending. Ghidra
read-only, save state unverified. The next area is context lifecycle
at `FUN_20cc27a0` and `FUN_20cc2940`.
