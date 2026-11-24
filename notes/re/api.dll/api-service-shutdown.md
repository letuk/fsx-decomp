# API service shutdown and callback removal

Original `api.dll`. Five instruction listings totaling 939
bytes were verified against the PE. The startup registration
at `200f7bb6` confirms the same owner and callback pairs as the
shutdown at `200f7c66`.

| Owner | Callback removed |
|---|---|
| `2020bb40` | `200f7b9f` |
| `2020bb34` | `200f7b95` |
| `2020bb4c` | `200f7b7b` |
| `2020bb34` | `200f7ba9` |
| `2020bb4c` | `200f7ba9` |

The shutdown entry calls `200fdfd4` before removal and then calls
`200f757e` and `201179b0` before clearing byte `2020d90a`.
Those three routines remain source boundaries for object,
service-window and console cleanup.

`200f65b1` takes the owner in EAX and removes the first live
matching entry. The optional context check matches both words.
All five shutdown calls use callback-only matching. With zero
dispatch depth, the helper shifts the tail, zeroes the final
entry and decrements count. During dispatch it sets entry bit 0,
the list pending flag and global `2020d8fc`. A null list returns
zero. A populated header without a match returns -1.

The source reuses the existing 16-byte callback header and
entry layouts. Dispatch and deferred compaction at `200f67bd`
and `200f6688` are the next API package. Owner and callback
targets remain linkage boundaries. Suggested Ghidra names,
unapplied, are `in_EAX` → `listOwner` and `uVar4` →
`entryIndex` at `200f65b1`.

`api-service-shutdown-20261111-001` passed all 20 comparators.
COFF SHA256 is
`B93B388557390D62F4D13BD3034CB66481240DB1E65B8EF1A2CF92127A9AADC4`.
New byte matches and runtime verification remain open.
Ghidra was unchanged.

## Main-window service callback

2026-11-11. Seven original API listings totaling 805 instruction
bytes were verified against the PE. Callback `200f724b` uses
stdcall with two arguments. For event 0x2002a it reads the
event record at +0x0c. A zero value enters the queue drain
when enabled and when no drain is active. API ordinal 323
at `200f7809` sets the enable byte and posts the initial
WM_COMMAND. The registered callback reuses the existing
queue detach and dispatch operations.

For a nonzero value, successful module-name lookup selects
the module path. The API looks for `SimConnect.dll`, resolves
`SimConnect_CallDispatch` and skips duplicate exports. Failed
module-name lookup selects the observed 0x18-byte client
descriptor path. Its acceptance callback receives the
0x20-byte version-2 registration record. The source explicitly
terminates a full module-path buffer before the name search.

Suggested Ghidra names, unapplied. At `200f724b`, `param_2`
→ `eventRecord` from the +0x0c access and `local_11c` →
`dispatchExport` from GetProcAddress and the duplicate check.

The callback is connected to service-window registration and
removal. The upstream event-record producer, export storage,
and client registration helpers remain linkage boundaries.
SimConnect is observed only through the API-side export lookup.

`api-service-main-dispatch-20261111-001` passed all 20
comparators. COFF SHA256 is
`4E6DDD65BC90276C9D9C233092FE6B0806FB37C209ABCD7813BBF6F459A583EE`.
No new byte matches were accepted. Ghidra was read-only.
Runtime verification remains open. Next are the dispatch-export
list and client registration helpers at `200f7829` and `201164e2`.

## Dispatch exports and client registration

2026-11-11. Six original API listings totaling 601 instruction
bytes were verified against the PE. The export list header at
`201f96b8` starts empty, with growth 5 and flags 4. The append
at `200f7829` uses the growth branch of `200f787d`. Source
preserves geometric growth and keeps the existing list and
count on allocation failure or capacity overflow. Shrink paths
of the generic original capacity helper are outside this use.

`20117198` searches the sorted index at `201f982c` for the
lower bound. It publishes the candidate slot even on a miss.
`201164e2` receives the key in EAX and stores the two
registration words at client +0x200 and +0x204 on a hit with a
nonnull client. The registration record now also connects to
the private-heap callbacks at `200f7137` and `200f7158`.

Suggested Ghidra names, unapplied. At `20117198`, `uVar5`
→ `lowerBoundIndex` from the binary search and slot address.
At `200f787d`, `in_EAX` → `requestedCapacity` from its
comparison against the allocation limit.

`api-service-client-registry-20261111-001` passed all 20
comparators. COFF SHA256 is
`98A257342EFF54B0F713C213D1C4290A46AC6390B63CBD3A6C94E3C531D78EDB`.
No new byte matches were accepted. Runtime verification remains
open. Ghidra was unchanged. The next API boundary is delivery
through `200f7177` and `20114d58` and the client index owner.
SimConnect bodies are separate module work.

## Dispatch and deferred removal

2026-11-11. Original `api.dll` listings for `200f67bd` and
`200f6688` account for 420 instruction bytes verified against
the PE. The dispatcher skips entries with bit 0 set. Bit 1
adds the two context words to the event and argument. The
assembly shows callee stack cleanup for both call shapes.
The existing cdecl registration type serves as a stored pointer
type in this source. The dispatcher uses stdcall at invocation.
Individual callback targets still need ABI verification.

The dispatcher reloads the owner after each invocation because
registration can reallocate the list. Deferred compaction walks
43 owners in the range `2020bb30` up to `2020bbdc` exclusive.
It preserves order, clears the vacated records and leaves busy
lists pending. Source maps six named owners to their existing
globals and uses an external slot array for the remaining
owners. That array still needs linkage to the application.

Suggested Ghidra names, unapplied. In `200f67bd`, `param_1`
→ `listOwner` from the pointer reload and `uVar5` →
`entryIndex` from the count comparison.

`api-callback-dispatch-20261111-001` passed all 20 existing
comparators. COFF SHA256 is
`0CC046EF351CB3542413BAF865D863D28EC06E5B1674B7C90FE79ECD47A7CEBE`.
No new byte matches were accepted. Ghidra was unchanged.
Runtime verification remains open. Next is the service-window
and queue cleanup at `200f757e`.

## Service window and queue teardown

2026-11-11. Four original API listings, totaling 805 instruction
bytes, were verified against the PE. At `200f757e`, client cleanup
`20115e3a` runs first, followed by removal of callback `200f724b`
from owner `2020bb50`. The procedure destroys the message window
and unregisters its atom. The callback owner also participates
in the existing deferred removal sweep.

The two queue headers use a 0x14-byte span with lock pointer,
head, tail, entry count and byte count. The enqueue path
`200f76d7` confirms both counters. Shutdown detaches each
chain under its critical section and frees nodes and payloads
after unlocking. The heap-owned header at `2020d900` also
releases its lock and header. The shared dispatch header at
`2020e150` retains its lock. The source reuses this detach
operation for `200f7793`, whose caller owns the returned chain.

Client shutdown, the callback body and shared queue storage
remain linkage boundaries. Next is the message-window
initialization at `200f74ab` and the queue routing around its
procedure at `200f7426`. Suggested Ghidra names, unapplied,
are `puVar2` → `windowQueue` and `puVar1` → `detachedNode`
at `200f757e`, based on the header reads and chain walk.

`api-service-window-shutdown-20261111-001` passed all 20
comparators. COFF SHA256 is
`8D8DDC483F7BE0177CD79EA4DE177454F32471AD56F6C3BAD65C6CED813A5863`.
No new byte matches were accepted. Ghidra was unchanged.
Runtime verification remains open.

## Service window initialization and queue routing

2026-11-11. Seven retained `api.dll` listings totaling 970
instruction bytes were verified against the original PE.
`200f74ab` initializes the local queue and its critical
section once, registers callback `200f724b` at owner
`2020bb50`, and creates a message-only window using the
registered class atom. The client initializer `20115d07`
runs on each call. The source preserves the observed unchecked
class and window creation results.

`200f76d7` sends ordinary entries through the main window
WM_COMMAND route. The observed special packet branch posts
WM_USER to the service window and uses the shared queue.
`200f7426` suppresses nested WM_USER dispatch, detaches the
shared queue under its lock and dispatches it after unlocking.
`200f71c2` publishes each node's timestamp, checks its
recorded generation against the client state, invokes the
callback when eligible and frees the node and payload.

The source covers both enqueue routes and the service-window
consumer. The caller at `200f7bb6`, the main-window consumer
`200f724b` and client initialization at `20115d07` remain
integration boundaries. Shared queue storage and callback
target linkage also remain open.

Suggested Ghidra names, unapplied. At `200f71c2`, `in_EAX`
→ `queueHead` from the next-pointer walk, and `iVar2` →
`clientGeneration` is provisional based on the +0x1cc
load and comparison against the queued value.

`api-service-window-init-20261111-001` passed all 20
comparators. COFF SHA256 is
`A88A2FDA455A085CB8819F88F016D29DC3B1D7355593B6B48E1EFA4B73A60D0E`.
New byte matches and runtime verification remain open.
Ghidra was unchanged.
