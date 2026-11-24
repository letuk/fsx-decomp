# API pointer owner updates

## VC8 COM smart-pointer provenance check

Live verified in `api.dll` on 2026-11-24. Fresh Ghidra listings and
raw bytes for the three bodies agree with the pinned original PE.
The XP worker compiled header candidates from the installed VC8 RTM
`atlbase.h` and `comip.h`, with `/Ob0` and `/Ob2` on the established
x86 COFF profile. The probe defines its own source-slot transfer
method and invokes each candidate's existing methods.

| Original VA | Scope | Native header comparison |
| --- | --- | --- |
| `20161815` | 31-byte source-slot transfer | Both probes reproduce the caller encoding after aligning its REL32 operand. Both targets resolve to candidate Attach bodies with differences in the complete call chain. |
| `201611be` | 23-byte Attach | With /Ob0, _com_ptr_t reproduces the caller encoding after aligning its REL32 operand. It calls the 13-byte private _Release, which leaves the slot intact. With /Ob2 it inlines that call. ATL directly calls through virtual slot +8 in both builds. |
| `20161203` | 22-byte Release | The original skips NULL and clears the slot after the virtual call. The ATL build clears it before the call. Public _com_ptr_t::Release signals E_POINTER on NULL. |

The tested stock VC8 RTM header methods therefore fail to reproduce
the complete original call chain. The same source-slot caller can wrap
either candidate, so that result establishes no class identity. The
origin of the original wrapper, a modified implementation and direct
versus static-library linkage remain open. The existing reconstructed
source and comparison classifications are retained. Read-only Ghidra
inspection made no edits. The current save state was not independently
queried. All 20 source-pass regressions passed on the retained baseline.

[Original listings](../../../artifacts/api.dll/com-owner-vc8-20261124/ghidra-original.json)
· [Original bytes](../../../artifacts/api.dll/com-owner-vc8-20261124/ghidra-original-bytes.json)
· [COFF comparison](../../../artifacts/api.dll/com-owner-vc8-20261124/comparison.json)
· [Probe source](../../../artifacts/api.dll/com-owner-vc8-20261124/com_owner_probe.cpp)
· [XP /Ob0 run](../../../vm-share/modules/api/runs/api-com-owner-vc8-20261124-ob0/)
· [XP /Ob2 run](../../../vm-share/modules/api/runs/api-com-owner-vc8-20261124-ob2/).

```powershell
./artifacts/api.dll/com-owner-vc8-20261124/dispatch.ps1 -Inlining Ob0
./artifacts/api.dll/com-owner-vc8-20261124/dispatch.ps1 -Inlining Ob2
node artifacts/api.dll/com-owner-vc8-20261124/compare.cjs
```

The two dispatch commands record how these runs were created. The
controller rejects existing IDs. The comparator can be rerun on both
retained runs.

## Initial pointer-owner update block

Live verified in `api.dll` on 2026-11-24. The contiguous block from
`0x20161815` through `0x20161878` contains four functions previously
in the dashboard's Inventoried category. All use an ECX receiver and
one stack argument, with `RET 4`. The class and member names below are
reconstructed, and the single-pointer layouts cover the observed
receiver state.

| Original VA | Operation | Direct target | Bytes |
| --- | --- | --- | ---: |
| `0x20161815` | Takes a pointer from a source slot, clears the slot, attaches the pointer and returns the receiver. | `0x201611be` | 31 |
| `0x20161834` | Calls the private-heap pointer replacement and returns the receiver. | `0x201611d5` | 23 |
| `0x2016184b` | Releases the previous prefixed buffer and stores the replacement. | `0x2015115f` | 23 |
| `0x20161862` | Calls the prefixed-buffer replacement and returns the receiver. | `0x2016184b` | 23 |

The attach path from `0x201611be` reaches `0x20161203`, which
calls table offset 8 on the existing nonnull pointer before clearing
the slot. This supports the provisional `IUnknown*` source model
without establishing the original interface identity. The private-heap
replacement calls `0x20161030`, which clears the slot after invoking
the private-heap free helper `0x200f40a9`. The prefixed-buffer release
reaches `0x20150c15`, which passes the pointer minus four to the
process-heap free helper `0x2014e75f`. These paths distinguish the
ownership semantics of the three source classes.
[Live listings and dependency context](../../../artifacts/api.dll/pointer-owner-updates-20261124/ghidra-pointer-owner-updates.json).

Suggested Ghidra names are `this` for the ECX receiver, `source` for
the source-slot pointer at `0x20161815`, and `replacement` for the
remaining stack arguments. The suggestions follow the loads, stores
and argument forwarding. Ghidra was read-only and reported saved
revision `1793741866691:0852fe08:3`.

The VC8 RTM native x86 COFF build matches all 100 bytes after
individually validating the four REL32 fields. The wrapper's call at
`0x2016186d` resolves to the replacement matched within this object.
The three other targets remain external declarations in this build.
The XP receipt succeeded, source hashes agree, and all 20 source-pass
checks, the eight construction-record matches and seven earlier
comparison gates passed. Linked DLL and runtime verification remain open.
[Comparison report](../../../artifacts/api.dll/pointer-owner-updates-20261124/comparison.json).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-pointer-owner-updates-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-updates-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-pointer-owner-updates-UNIQUE-ID
```

## Release and replacement follow-up

Live verified in `api.dll` on 2026-11-24. The scope includes the
91-byte contiguous group at `0x201611be`–`0x20161218` and its
related owner routines at `0x20161030` and `0x2015115f`. The
replacement methods release the current value before storing the
new one. The cleanup methods check for null, release or unmap the
resource and then clear the receiver's pointer.

| Original VA | Operation | Comparison |
| --- | --- | --- |
| `0x201611be` | Interface attachment calls the release body at `0x20161203` and stores the replacement. | Normalized, 23 bytes |
| `0x201611d5` | Private-heap replacement calls `0x20161030` and stores the replacement. | Normalized, 23 bytes |
| `0x201611ec` | Mapped-view cleanup calls `kernel32.dll!UnmapViewOfFile` through IAT slot `0x200e1834` and clears the pointer. | Normalized, 23 bytes |
| `0x20161203` | Interface cleanup calls the observed table slot at offset 8 with the pointer as the stack receiver and clears the pointer. | Exact, 22 bytes |
| `0x20161030` | Private-heap cleanup calls `0x200f40a9` and clears the pointer. The caller pops the helper argument. | Normalized, 23 bytes |
| `0x2015115f` | Prefixed-buffer cleanup calls `0x20150c15` and clears the pointer. The helper pops its argument. | Normalized, 22 bytes |

The `IUnknown::Release` model reproduces the observed virtual-call
encoding in the exact match. The concrete runtime table and method
implementations remain open. The mapped-view owner class has a minimum
one-pointer layout. Suggested Ghidra names are `this` for the ECX
receiver and `replacement` for the first stack word of the two
replacement methods. No Ghidra type or VTable was changed. Read-only
Ghidra reported saved revision `1793741866691:0852fe08:3`.
[Live listing and dependency targets](../../../artifacts/api.dll/pointer-owner-release-20261124/ghidra-pointer-owner-release.json).

All 136 bytes match in the native VC8 RTM x86 COFF scope, with one
raw-equal body and five bodies verified through four REL32 fields
and one DIR32 import field. Three entries were Inventoried. The
other three had unmatched generated bodies, which are superseded
by this comparison. The two heap free endpoints remain external.
The same run verifies the four earlier pointer-owner updates with
their previously external helper symbols now defined. The prior
constructor and arithmetic gates and all 20 source-pass checks passed.
The XP receipt and source hashes were checked. Linked DLL and runtime
verification remain open.
[Release comparison](../../../artifacts/api.dll/pointer-owner-release-20261124/comparison.json).
[Update regression](../../../artifacts/api.dll/pointer-owner-release-20261124/update-regression.json).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-pointer-owner-release-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-release-UNIQUE-ID release
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-release-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-pointer-owner-release-UNIQUE-ID
```

## Heap bridge endpoints

Live verified for `api.dll` on 2026-11-24. The two previously external
free endpoints and the adjacent process-heap allocation and free
wrappers now have native COFF matches.

| Original VA | Observed operation | Body bytes |
| --- | --- | ---: |
| `0x200f40a9` | Frees a nonnull pointer using the private heap handle at `0x2020e050` and flags zero. Plain RET. | 29 |
| `0x20150c15` | Frees a nonnull pointer minus four through `0x2014e75f`, then returns zero. RET 4. | 27 |
| `0x2014e744` | Calls `GetProcessHeap` and `HeapAlloc` with flags zero. Its caller at `0x20151e86` preserves EAX and tests the returned allocation. RET 4. | 27 |
| `0x2014e75f` | Calls `GetProcessHeap` and `HeapFree` with flags zero and preserves the HeapFree EAX result. RET 4. | 27 |

The reconstructed prototypes use cdecl for the private-heap release
and stdcall for the three RET 4 functions. The process-heap free's
`BOOL` return models the observed passthrough of EAX. The caller
at `0x20150c15` overwrites it with zero. Suggested Ghidra names for
the argument are `bytes` at `0x2014e744` and `pointer` at the
three release entries. The shared private-heap cell retains its
established name and four-byte extent. Its initial zero agrees in
Ghidra and the pinned PE, and its linked placement remains open.
[Live listings and caller evidence](../../../artifacts/api.dll/pointer-owner-heap-20261124/ghidra-pointer-owner-heap.json).

The size-optimized private-heap control emitted a 30-byte body with a
memory null check and memory push. Under default `/O2`, VC8 RTM
reproduces the original 29-byte body, which loads both the argument and
the private heap handle into EAX. The other three helpers retain
function-local size optimization. The prior unmatched generated
comparisons for the three process-heap entries are superseded here.
[Size-optimization control](../../../artifacts/api.dll/pointer-owner-heap-20261124/size-control.json).

The accepted build matches all 110 code bytes after resolving seven
verified fields. Five DIR32 fields identify named KERNEL32 imports,
one DIR32 identifies the private-heap cell, and one REL32 identifies
the rebuilt process-heap free wrapper. The two owner-release calls
now reference defined symbols in the same native COFF object. All
ten earlier owner methods retain their matches, as do the constructor
block, seven earlier comparison gates and all 20 source-pass checks.
The XP receipt and matching source hashes are retained. Read-only
Ghidra reported saved revision `1793741866691:0852fe08:3`. Linked
DLL and runtime verification remain open.
[Heap comparison](../../../artifacts/api.dll/pointer-owner-heap-20261124/comparison.json).
[Release regression](../../../artifacts/api.dll/pointer-owner-heap-20261124/release-regression.json).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-pointer-owner-heap-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-heap-UNIQUE-ID heap
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-heap-UNIQUE-ID release
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-heap-UNIQUE-ID updates
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-pointer-owner-heap-UNIQUE-ID
```

## Adjacent pointer-owner adapters

Live verified for `api.dll` on 2026-11-24. This selection includes
the contiguous 78-byte group at `0x20161219`–`0x20161266` and four
related routines. Six entries previously had dashboard status
Inventoried. The `0x20161047` generated result was Different and is
superseded by the typed C++ comparison.

| Original VA | Operation and direct target | Bytes |
| --- | --- | ---: |
| `0x20161219` | Unmaps the old view through `0x201611ec`, then stores the new view. | 23 |
| `0x20161230` | Clears the source view slot, transfers its pointer through `0x20161219`, and returns the receiver. | 31 |
| `0x2016124f` | Stores the counter pointer in the output slot, then calls `kernel32.dll!InterlockedIncrement` through `0x200e19c4`. | 24 |
| `0x20161047` | Frees a nonnull process-heap allocation through `0x2014e75f` and clears its slot. | 22 |
| `0x2016105d` | Calls `kernel32.dll!LeaveCriticalSection` through `0x200e1758` with the stored record plus eight, then clears the record pointer. | 26 |
| `0x20161937` | Sets ECX from the first stack argument, forwards the second to `0x20161815`, and returns zero. | 22 |
| `0x2016194d` | Releases the old process-heap pointer through `0x20161047` before storing its replacement. | 23 |

The `LONG` return of the counter helper preserves the imported
function's observed EAX value. Caller use of that result remains
unverified. The critical-section record is a minimum reconstructed
layout with two unknown DWORDs followed by the SDK critical section
at offset eight. Suggested names are `this` for ECX member receivers,
`replacement` at `0x20161219` and `0x2016194d`, `source` at
`0x20161230`, `counter` and `slot` at `0x2016124f`, and `owner`
and `source` at `0x20161937`. These suggestions follow the live
load, store and call order. No Ghidra edits were applied.
[Live listings and target evidence](../../../artifacts/api.dll/pointer-owner-adapters-20261124/ghidra-pointer-owner-adapters.json).

All seven bodies match in VC8 RTM native x86 COFF after resolving
five direct call fields and two named IAT references. The 171-byte
scope preserves every other byte. The XP receipt succeeded and
source hashes match. The previous pointer-owner groups, eight
constructor matches, earlier comparison gates and all 20 source-pass
checks passed. Read-only Ghidra reported saved revision
`1793741866691:0852fe08:3`. Linked DLL and runtime verification remain
open. [Comparison report](../../../artifacts/api.dll/pointer-owner-adapters-20261124/comparison.json).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-pointer-owner-adapters-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-adapters-UNIQUE-ID adapters
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-pointer-owner-adapters-UNIQUE-ID
```

## Mapped view and handle transfer

Live verified for `api.dll` on 2026-11-24. Seven related bodies
previously had dashboard status Inventoried. The composite receiver
holds a view at offset 0 and a handle at offset 4. These offsets
define a minimum reconstructed layout. The transfer at `0x20161964`
calls the known view transfer first and the handle transfer second.
The view transfer clears the source pointer to NULL. The handle
transfer sets its source DWORD to `0xffffffff` before replacement.

| Original VA | Operation | Bytes |
| --- | --- | ---: |
| `0x20160f45` | Closes the old handle, calls the predicate on the replacement and stores the replacement or `INVALID_HANDLE_VALUE` according to AL. | 40 |
| `0x20160f6d` | Calls the predicate on the current handle. If AL is zero, calls `CloseHandle` and stores `INVALID_HANDLE_VALUE`. | 31 |
| `0x20161077` | Calls handle replacement at `0x20160f45` and returns the receiver. | 23 |
| `0x20161669` | Forwards to `0x20161077` and returns the receiver. | 23 |
| `0x20161680` | Sets the source handle to `INVALID_HANDLE_VALUE` and forwards its previous value to `0x20160f45`. | 31 |
| `0x20161964` | Transfers the view through `0x20161230` and handle through `0x20161680`, then returns the receiver. | 38 |
| `0x20161e65` | Calls the paired transfer and returns zero. | 22 |

The original predicate entry at `0x20173c46` receives a stack word,
returns through stack-based control flow, and is followed by an AL
test at both in-scope call sites. `HandleValueRejectedPilot` is a
provisional name for the observed caller contract. Its actual
acceptance conditions remain open. The IAT at `0x200e176c` resolves
to `kernel32.dll!CloseHandle`. Suggested decompiler names are `this`
for ECX member receivers, `replacement` for a new handle, `source`
for a transfer source and `owner` for the first argument at
`0x20161e65`. The suggestions follow the original call and store order.
[Live listings and target evidence](../../../artifacts/api.dll/pointer-owner-mapping-20261124/ghidra-pointer-owner-mapping.json).

All 208 bytes match in native VC8 RTM x86 COFF after validation of
nine REL32 fields and one named IAT reference. The same factory run
retains the previously verified pointer-owner, heap and constructor
bodies, and all 20 source-pass checks pass. The worker receipt succeeded
and source hashes agree. Read-only Ghidra reported saved revision
`1793741866691:0852fe08:3`. Linked DLL and runtime verification remain
open. [Comparison report](../../../artifacts/api.dll/pointer-owner-mapping-20261124/comparison.json).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-pointer-owner-mapping-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-mapping-UNIQUE-ID mapping
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-pointer-owner-mapping-UNIQUE-ID
```

## Activity and critical-section leases

Live verified in `api.dll` on 2026-11-24. The six entries were
previously Inventoried. The reconstructed minimum activity record
has a signed activity count at +0, an event handle at +4 and
critical sections at +8 and +0x20. This follows observed accesses.
No Ghidra type was created. The source uses a volatile count for
the explicit read of a counter also updated by Win32 interlocked
operations. The first section can remain held while the wait
runs, and the decrement path uses the second section.

| Original VA | Operation | Result |
| --- | --- | --- |
| `0x20161785` | Enters the two sections in order, increments the count, resets the event when the result is one and leaves in reverse order. | Normalized, 72 bytes |
| `0x201617cd` | Enters the second section, decrements the count, sets the event when the result is zero and leaves the section. | Normalized, 46 bytes |
| `0x20161c75` | Releases the old section lease, stores the replacement and invokes `0x2014eb01` if nonnull. | Normalized, 32 bytes |
| `0x20161c95` | Calls the decrement routine for a nonnull lease and clears the pointer. | Normalized, 21 bytes |
| `0x20161caa` | Releases the previous activity, stores the replacement and invokes the increment routine if nonnull. | Normalized, 32 bytes |
| `0x2014eb01` | Enters the first critical section and waits indefinitely on the event when the signed count is positive. | Different, 35-byte scope |

The suggested Ghidra name for the ECX parameter is `this` for all
six bodies. The stack argument at `0x20161c75` and `0x20161caa`
is provisionally `replacement`, based on the conditional acquire
after the owner pointer store. Read-only Ghidra reported saved
revision `1793741866691:0852fe08:3`.
[Live listings](../../../artifacts/api.dll/pointer-owner-leases-20261124/ghidra-pointer-owner-leases.json).

The VC8 RTM native COFF build compares 238 bytes and 15 fields.
Five complete bodies match after resolving their 13 individually
validated fields. The wait body's two import fields also resolve,
but four bytes remain different at offsets 0x10, 0x12, 0x16 and
0x1a because the compiler uses different registers for the count
and the event. The two length variants and the 35-byte control
remain available with their source snapshots and logs. The current
35-byte build and the previous 35-byte control emit identical wait
bytes. Linked DLL and runtime verification remain open.
[Comparison](../../../artifacts/api.dll/pointer-owner-leases-20261124/comparison.json).
[Wait controls](../../../artifacts/api.dll/pointer-owner-leases-20261124/wait-controls.json).

The XP receipt succeeded, the current source matches the retained
compiled-source hash, and all 20 source-pass checks and the previous
pointer-owner, heap and constructor comparisons passed.

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-pointer-owner-leases-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-leases-UNIQUE-ID leases
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-pointer-owner-leases-UNIQUE-ID
```

## Buffer owner forwarding and reset

Live verified in `api.dll` on 2026-11-24. Four previously
Inventoried bodies use an ECX receiver. The three forwarding methods
pass one pointer argument to an existing replacement routine and
return the receiver. The reset passes the receiver unchanged to the
process-heap release routine, then clears the DWORDs at +8 and +4
in that order. The reconstructed buffer record holds the heap pointer
owner at +0. The roles of the two cleared fields remain open.

| Original VA | Reconstructed operation | Direct target | Bytes |
| --- | --- | --- | ---: |
| `20161bdd` | Forward private-heap replacement and return the receiver. | `0x20161834` | 23 |
| `20161bf4` | Release the process-heap pointer and clear both state fields. | `0x20161047` | 20 |
| `20161c5e` | Forward prefixed-buffer replacement and return the receiver. | `0x20161862` | 23 |
| `20161d26` | Call process-heap replacement and return the receiver. | `0x2016194d` | 23 |

Suggested Ghidra names are `this` for the ECX receiver and
`replacement` for the pointer forwarded from the stack. No Ghidra
renames or type edits were applied. The current database save state
was not independently verified in this pass.

The XP VC8 RTM native COFF build matches all 89 bytes after validating
the four REL32 operands and their defined target symbols. The
updates and adapters comparators independently verify the target
bodies in the same object. All six previous pointer-owner profiles
and all 20 source-pass checks reproduce their established results.
Linked DLL and runtime verification remain open.
[Comparison](../../../artifacts/api.dll/pointer-owner-buffer-20261124/comparison.json)
· [Live listings](../../../artifacts/api.dll/pointer-owner-buffer-20261124/ghidra-pointer-owner-buffer.json)
· [XP run](../../../vm-share/modules/api/runs/api-pointer-owner-buffer-20261124-001/).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-pointer-owner-buffer-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-buffer-UNIQUE-ID buffer
```

## Mapped file and interface cleanup adapters

Live verified in `api.dll` on 2026-11-24. Three previously
Inventoried bodies reuse the known owner release paths. The mapped
file method passes receiver +4 to the handle close, then passes
the unadjusted receiver to the view release by tail jump. These
offsets agree with the previously reconstructed composite layout
at `0x20161964`. The two stack-entry adapters test their argument
for NULL before calling the corresponding member method and pop
one argument on return. The static member form models that observed
ABI in the reconstruction. The return values are unverified.

| Original VA | Reconstructed operation | Direct target | Bytes |
| --- | --- | --- | ---: |
| `20162571` | Close the mapping handle, then release the view. | `0x20160f6d`, then `0x201611ec` | 21 |
| `20162fd5` | Release the held interface reference for a nonnull owner. | `0x20161203` | 21 |
| `20163585` | Close the mapping and release the view for a nonnull owner. | `0x20162571` | 21 |

Suggested Ghidra names are `this` for the ECX receiver at
`0x20162571` and `owner` for each adapter's stack argument,
based on the observed member calls. Ghidra was read-only and
the current database save state was not independently verified.

The XP VC8 RTM native x86 COFF build matches all 63 bytes after
validating three direct calls and one tail jump. The existing mapping
and release profiles verify the three dependency bodies in the same
object. The handle predicate at `0x20173c46` remains external.
All seven prior pointer-owner profiles, eight constructor matches
and 20 source-pass checks reproduced their established results.
Linked DLL and runtime verification remain open.
[Comparison](../../../artifacts/api.dll/pointer-owner-cleanup-20261124/comparison.json)
· [Live listings](../../../artifacts/api.dll/pointer-owner-cleanup-20261124/ghidra-pointer-owner-cleanup.json)
· [XP run](../../../vm-share/modules/api/runs/api-pointer-owner-cleanup-20261124-001/).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-pointer-owner-cleanup-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-pointer-owner-updates.cjs vm-share/modules/api/runs/api-pointer-owner-cleanup-UNIQUE-ID cleanup
```
