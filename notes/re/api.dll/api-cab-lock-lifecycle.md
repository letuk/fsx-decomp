# Stream pool lock and owner lifecycle

Follow-up api-cab-pool-acquire.md supersedes the pool acquisition deferral
below. Modes0/1/3 for fixed pool arguments now build and connect to both pools.
General acquisition and runtime contention validation remain open.

2026-11-09. Original api.dll SHA256 is
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Fifteen exported listings covering 5606 bytes match the original PE. The
addresses and hashes are in artifacts/api.dll/api-cab-pool-lifecycle-evidence-20261109.json.
Live read-only api.dll queries report saved revision1793741866691:2a9d629f:3.
Working names and layouts are reconstructed from the observed accesses.

## Lock release and lifecycle

20147aa6 takes two stack arguments and returns with RET8. NULL or invalid
signature returns 0x57. Mode 0 atomically subtracts 0x1000 from the shared
count in the high DWORD. Missing shared ownership returns 0xe0000001. When
the resulting shared count and exclusive bit are clear and a writer waits,
it signals the writer event. Nonzero release checks thread ownership when
state+8 is nonnegative, handles recursion at+0x1c and clears the exclusive
bit0x100000. Pending writers take priority when no readers hold the lock.
When no writer is pending, it releases the reader semaphore by the split
waiter count from low bits24–31 and high bits0–11. Both CAS64 paths use LOCK
when signed mode201f7090 is positive. The shared primitive now serves both
slot pop and lock state transitions. Slot pop still chooses its own threshold.

20147dff constructs 40 bytes of aligned lock state, a reader semaphore with
initial count0, and an initially unsignaled auto-reset event. The valid
signature4c664c6b is installed after both handles exist. Failure releases
the raw allocation and closes the semaphore if the event creation failed.
The source retains the original process-heap path and allocator release hook.

20147c77 receives state in EAX. It initializes the processor mode before
validating the pointer, clears the signature atomically, then checks owners
and waiters. If busy, it confirms the snapshot with a no-change CAS and
returns EAX0 with the signature already cleared. An idle state sets bit800000
through CAS, closes event then semaphore and frees the raw allocation.
The EAX result combines both CloseHandle results. Pool destructors ignore it.
An EDX return contract has not been established from the decompiler's wide
return expression. The source models the observed EAX result.

## Owner lifecycle and block maintenance

20124ca3 and 20127e1d insert the two observed static owner records at2020eaac
and2020ea90 into registry2020e2bc, establish the appropriate VTable, construct
the lock and clear block statistics. The source models normal construction.
Original constructor/destructor unwind metadata remains outside this slice.

201247c3 and 201278c4 call slot+0xc before acquiring mode1. The two original
VTables resolve that slot to201128d7. The source keeps it as an indirect ECX
receiver call because its downstream callback flow needs separate analysis.
After acquisition the routines detach the head, destroy every block and
clear the current count. 20124845 and 20127946 instead remove blocks whose
available count equals their slot capacity. Both retain the peak count.

2012475b and 2012785c restore their concrete VTable, reset the pool, destroy
its lock if present and clear the lock field. Base helper200f43d6 changes the
VTable to200f0528 and unlinks the registry node, including the head case.
The source shares reset, trim, initialization and destruction logic across
both pool types. Original VTable/global definitions and startup registration
remain separate integration work.

## Acquisition evidence and remaining source boundary

20146e07 receives the lock in EDI plus six stack arguments and returns with
RET24. The pool callers use modes0,1,3 with NULL callback, zero policy flags,
infinite timeout and alertable FALSE. Mode0 takes or queues a shared reader.
Mode1 takes or queues an exclusive writer, including owner recursion. Mode3
converts one existing shared claim. The raw branches at20147923 and20147930
return 0xe0000002 for an existing upgrade reservation and 0xe0000003 when
mode3 encounters pending writers. An upgrade which has queued a waiter has
a distinct failure rollback that restores a shared count, drops the writer
count and clears the upgrade bit. The exclusive queue and shared queue use
different counters and handles. Source for the complete acquisition routine
remains external pending verification of the contention state transitions.

The optional spin uses20145ca9 with an evolving mask in ESI, RDTSC and PAUSE.
The default upgrade wait at201478cb calls20145d2a with timeout in EAX,
alertable in ECX and event on the stack. That helper returns WAIT_TIMEOUT
for timeout0 and otherwise calls WaitForSingleObjectEx. This resolves the
missing timeout and alertable inputs in the decompiler call. These findings
bound the next source package to queue insertion, wake retry and rollback
for the three pool modes. Other acquisition modes and callbacks need their
own contract checks.

## Builds and coverage

api-cab-lock-lifetime-20261109-001 builds release and lock lifetime. The
retained api-cab-pool-lifecycle-20261109-001 adds both pool lifecycles, reset,
trim and registry unlink. Its object SHA256 is
745B32639E00D5197CAB0A3FDA50665837E6130D135022805B553FC864BD8C68.
The run's cab-pool-lifecycle-diagnostic.json records twelve source entry
points and their relocations. Twelve existing comparison gates pass at 4735
code bytes and 44 metadata bytes. Lock release485/465, create188/242 and
destroy326/392 native/original remain outside body acceptance. The shared
pool helpers also remain outside acceptance.

Registry unlink trial one was63/63 with different instruction ordering.
Trial two moved the empty-list check to mirror the original traversal and
produced53/63. The clearer first source was restored and its hash matches
the retained run. Stop at two trials. Runtime concurrency and linked DLL
validation remain open.

Suggested Ghidra names are 20147aa6 param_1 to lockState and param_2 to
exclusiveRelease, 20146e07 unaff_EDI to lockState, param_5 to timeout and
param_6 to alertable. 20147c77 in_EAX can become lockState. These follow the
checked signatures and wait calls. No Ghidra renames or type edits were applied.
