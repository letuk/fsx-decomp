# simscheduler.dll named context registry

Live verified in simscheduler.dll from the original listings. The registry
index is at owner offset `+0x30`. Its count, capacity, growth step, flags and
record pointer are at `+0x00/+0x04/+0x08/+0x0a/+0x0c`. Records are
`0x24` bytes, with a 32-byte name and a retained context pointer at
`+0x20`. The C++ names and layouts are reconstructed minima.

| Original entry | Reconstructed operation | Original bytes | VC8 COFF bytes |
| --- | --- | ---: | ---: |
| `0x20cc17d0` | Find by name and return a retained reference through the output pointer | 220 | 212 |
| `0x20cc18c0` | Remove by the name read through context slot `+0x08` and release the incoming reference | 205 | 198 |
| `0x20cc2150` | Look up a fixed-width key and optionally assign the retained context | 86 | 145 |
| `0x20cc22e0` | Insert a key and retain its context, rejecting duplicate keys | 170 | 156 |
| `0x20cc2390` | Lower-bound search with an optional insertion-position output | 165 | 168 |
| `0x20cc2440` | Resize backing storage, transfer retained records and release truncated entries | 307 | 292 |
| `0x20cc2580` | Open a slot by moving the suffix upward | 110 | 100 |
| `0x20cc2600` | Release the removed entry and move the suffix downward | 66 | 77 |
| `0x20cc2650` | Set logical count and initialize new slots or release truncated entries | 191 | 185 |

The `0x20cc2650` comparison uses the contiguous range through the final
return and includes the three-byte gap at `0x20cc26dd`–`0x20cc26df`.

`0x20cc17d0` rejects lengths of 32 or greater. Its listing reads the local
reference after `0x20cc2150` returns and assigns it to the result. The Ghidra
decompiler dropped those blocks because it did not model the EDI output argument
of the helper. The lookup passes the 32-byte key on the stack and the index in
EAX. The insert wrapper receives the key in EAX, the index in ECX and a single
stack argument. The slot eraser receives the index in ESI and the position in
EBX. The C++ uses explicit parameters for the internal register conventions.

The three retained approaches use explicit reference cleanup, scoped C++
reference ownership, then frame-pointer omission for the two slot helpers.
All nine comparisons are diagnostic. The best retained run uses the third
approach with VC8 SP1, native x86 COFF, and the existing module profile.

## Registry unwind actions

Live verified in simscheduler.dll. Lookup at `0x20cc17d0` keeps the
temporary result under state 1 (`0x20cc7059`) through name copying,
lookup and output assignment. Once the output has been published,
state 0 (`0x20cc7040`) releases it if the final local release throws.
The older C++ modeled the temporary state only. The lookup now
reuses the published-reference guard from the factory.

Removal at `0x20cc18c0` protects the consumed incoming reference
through name lookup and removal. Its sole action at `0x20cc70e0`
releases that reference. Both original and rebuilt code disarm the
state before the normal final release. The removal source is unchanged.

The retained VC8 SP1 run `simscheduler-registry-unwind-20261116-001`
verifies both unwind maps, the state writes and the actual guard
destructor bodies. Lookup is 220 original versus 228 rebuilt bytes
and removal is 205 versus 198. All three funclet comparisons are
diagnostic due to stack layout and register ABI. Ghidra read-only and
saved. Runtime exception execution remains unverified.

- [Unwind action comparisons](../../../artifacts/simscheduler.dll/eh-funclets-comparison.json)
The two wrappers recover EH references with scoped cleanup, but VC8 emits
different cookie handling. The lookup also emits a cookie and copies its
stack arguments differently. The insertion code checks allocation failure
before using a newly requested slot. The original listings proceed after
the resize calls without checking their return values. Runtime behavior,
including exceptional cleanup and allocation failure, remains unverified.

The comparator reads hash-identified PE bytes and checks DIR32 fields against
the PE relocation directory and REL32 calls against the original listing.
It verifies API import slots against the original import table and records
unpaired fields and changed operand offsets in each report.

- [Original Ghidra listings](../../../artifacts/simscheduler.dll/name-registry-ghidra.json)
- [Best worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-name-registry-20261116-003/receipt.json)
- [Lookup comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-name-registry-20261116-003/comparison-20cc17d0.json)
- [Insert comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-name-registry-20261116-003/comparison-20cc22e0.json)
- [Remove comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-name-registry-20261116-003/comparison-20cc18c0.json)

The twelve existing native COFF gates pass. The retained Flight save linked
comparison and GPS registration COFF comparison also pass. Live Ghidra MCP
reported `unsaved_changes: false` and the inspection was read-only.
`FUN_20cc1cc0` is the next boundary. It uses the registry to create, find and
remove the `SimConnectFallback` context and invokes its time-state callbacks.
