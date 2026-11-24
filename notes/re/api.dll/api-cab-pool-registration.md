# Stream pool registration and owner tables

2026-11-09. Original api.dll SHA256 is
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Read-only Ghidra checked the callback, table and startup spans in api.dll and
reported saved state. Pinned PE anchors are in
artifacts/api.dll/api-cab-pool-registration-evidence-20261109.json. The original listing
inventory supplies the boundaries of the small routines that are outside
the stored function model. Names in source are reconstructed working names.

## Pre-reset callback and owner tables

201128d7 receives the owner in ECX. It calls slot+0x14 for capacity, reloads
the table and calls slot+0x10 for the available count. It returns their
unsigned difference. The reset routines ignore that return. The available
methods 20124ba0 and 20127ca1 acquire mode 0, sum the block available counts
and release mode 0. The source preserves the call order and shared-lock
traversal as part of the callback's behavior.

The concrete tables at200f2cb0 and200f2d90 each expose eleven checked slots.
They contain deletion, reset, trim, used count, available count, capacity,
block count, peak count, slots per block, block bytes and record bytes.
The deleting entry receives ECX owner and a stack flag word. Source uses
fastcall with an ignored EDX slot to express that machine interface. Bit0
requests a private-heap free after destruction. The observed base prefix at
200f0528 contains its deleting entry and ten calls through20148c31 to
msvcr80.dll!_purecall. The source table uses the CRT symbol for those slots.
The prefix boundary describes verified table bytes only.

## Registration, lifetime and storage

The pointer table at200e1eb0 contains the file thunk201ee703, intervening
thunk201ee70e and wrapper thunk201ee719. The pool thunks call their guarded
getters20124c11/20127d14 and publish returned pointers to2020e194/2020e1a0.
The getters set bit0 of guards2020eac4/2020eaa8, initialize owners at
2020eaac/2020ea90 and register atexit handlers201eeee2/201eeef7. Those
handlers destroy the respective static records. Pool pointers, records,
guards and registry head2020e2bc have observed zero initial storage. The
source now defines them. Atomic mode201f7090 has original initial byte2,
also defined in source. The register functions reproduce the two thunk
actions. Placement into the final CRT startup order remains a linker task.

The getter unwind actions201ed460/201ed414 clear guard bit0. Constructor
action201ed43d and destructor action201ed3c0 dispatch the registry unlink
path. Scoped C++ cleanup now describes those actions. The fixed pilot
compiler profile emits no corresponding getter or owner unwind handlers.
Original exception behavior therefore remains outside the verified build.

## Builds and accepted coverage

Initial source run is api-cab-pool-registration-20261109-001. Follow-up
api-cab-pool-registration-eh-20261109-001 adds scoped cleanup and the initial
atomic mode byte. Final object SHA256 is
5963240F0CA49217C9A582FA7300575BF127C6A557DDFFD97935C0D60FF82A65.
Both runs are under vm-share/modules/api/runs. The new comparator
compare-api-cab-pool-registration.cjs verifies eleven raw bodies totaling
97 bytes. Shared methods are counted once. The used-count callback adds31
of these bytes and ten statistic methods add66.

The comparator validates all 33 DIR32 fields in the three observed table
spans against the original PE targets and source symbols. This adds132
metadata bytes, tracked separately. The overlap check includes the earlier
16-byte application table and 28-byte scope table. Opcode, return and
wrong-slot controls reject their mutations. All thirteen gates pass at
4832 code bytes and176 metadata bytes. Registry, lifecycle, available-count
and registration routines remain outside code acceptance. Final table
placement, CRT startup, owner unwind and runtime validation remain open.

Suggested Ghidra names are 20124c75/20127da5 param_1 to poolOwner and param_2
to deletionFlags, based on destruction and the bit0 free test. The getter
local_8 can become unwindState, based on the EH state store. These suggestions
were not applied. Next connect the actual file and wrapper stream VTable
methods at200f2c70/200f2d50 and recover their remaining data/forwarding slots.
