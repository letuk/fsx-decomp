# Stream pool lock acquisition

The follow-up in api-cab-pool-registration.md connects the owner registry and
pre-reset callback chain. Thirteen gates now pass at4832/176.

2026-11-09. Original api.dll SHA256 is
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
The original listings for 20146e07, 20145ca9 and 20145d2a were checked against
the PE in artifacts/api.dll/api-cab-pool-lifecycle-evidence-20261109.json. This pass
compared the source transitions with the instruction listings and checked the
live 3231-byte function model at saved revision1793741866691:2a9d629f:3.
Ghidra access was read-only.

## Scope and source entry

All five pool call sites now use AcquireCabPoolLockForStreamPoolsPilot with
mode 0, 1 or 3. Those original calls set infinite timeout, nonalertable waits,
zero policy flags and NULL callback. The source entry takes only state and
mode. Original 20146e07 has EDI receiver and six stack words. General modes,
callback dispatch, finite timeouts and alertable acquisition remain outside
this model. The source dispatcher rejects modes outside its three-mode scope.

| Mode | Queue state | Wait and retry | Nonzero wait result |
|---|---|---|---|
| 0 shared reader | 20146f5e adds 0x01000000 to the low word with carry into the high word | Reader semaphore, then test low reader-blocking bits before converting waiter to owner | 2014710c removes the split waiter count with borrow |
| 1 exclusive writer | 2014725f adds one to the low writer count | 20147342 waits on writer event. A confirmed blocked snapshot retries the wait and can re-signal an upgrader | 201474e3 removes one writer and wakes queued readers when the writer and exclusive bits permit |
| 3 shared-to-exclusive upgrade | 201477d4 adds a writer and sets upgrade bit while removing one shared claim | 201478ba waits on writer event. An available grant clears the upgrade bit | 20147a0d removes the writer, clears the upgrade bit and restores the shared claim before considering reader wakeup |

Mode 0 direct grants add 0x1000 to the high word. Queued grants subtract the
split waiter unit and add that shared-owner unit in one CAS64. The writer and
upgrade paths use separate blocker masks. In particular a queued upgrader
does not block on its own upgrade bit. The two writer wake paths first attempt
the original no-change comparison on a blocked snapshot. A failed comparison
falls through to a grant attempt on that same snapshot, as at 20147460 and
20147998. CAS failure then retries the snapshot. The source uses the shared
CAS64 primitive and selects LOCK for a positive signed mode byte.

Mode 3 requires a shared claim. Without one, 20147916 returns 0x57. An existing
upgrade reservation and a pending writer return 0xe0000002 and 0xe0000003.
The recursive-owner branch converts the shared claim into one recursion level.
The mode 1 recursion path at 201471ba masks the expected word for its special
spin-bit comparison while leaving the desired word intact. This operand
direction is preserved. Owner-tracking field meanings remain provisional
beyond the observed sign and zero tests.

The optional backoff emits RDTSC and PAUSE, starting from the original zero
mask and increasing it as in 20145ca9. The three queue paths spin for up to
1000 iterations when the mode is positive and bit400000 is set. For this
fixed wait profile the 20145d2a upgrade wait resolves to the same infinite,
nonalertable WaitForSingleObjectEx call used in the source.

## Build and verification

Run api-cab-pool-acquire-20261109-001 under vm-share/modules/api/runs has object
SHA256 C6853690C32B58622F0C58DF0281E69402248C4CE92AA8BC5270B5C2F30EBD8F.
cab-pool-acquire-diagnostic.json records emitted spin instructions, wait
imports and source sizes. It also verifies the old unresolved acquisition
symbol is gone from this object. The 147-byte dispatcher and its helpers
are a pool-specific decomposition. They remain outside body acceptance.
Twelve existing gates pass at 4735 code bytes and 44 metadata bytes.
This first source build uses the existing compiler profile. Matching trials
are deferred at the known scope and ABI differences. Runtime contention,
thread scheduling and linked DLL behavior remain unverified.

Suggested Ghidra names are 20146e07 param_1 to acquireMode and local_10 to
requestingThreadId, from mode dispatch and the GetCurrentThreadId store.
These are suggestions only. The next pool integration work is the owner
registration and pre-reset callback chain at 201128d7, including the actual
VTable targets and remaining lifetime boundaries.
