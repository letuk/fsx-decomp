# CAB buffer cache and stream object pools

The follow-up in api-cab-pool-locks.md supersedes the earlier pool source deferral.
The shared outer pool and block initialization source now build. The primitive
lock and atomic slot operations remain external.

2026-11-09. The original api.dll SHA256 is
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Eight exported listings covering 2357 bytes match the original PE. Listing hashes
and addresses are in artifacts/api.dll/api-cab-buffer-pool-evidence-20261109.json.
The live api.dll state was saved at revision1793741866691:2a9d629f:3. This pass
used read-only Ghidra queries. All working names below are reconstructed.

## Cache source

20127dd3 selects the first cached block whose unsigned size meets the request.
It returns -1 when no block qualifies. The size occupies the DWORD immediately
before the returned payload. The array at2020e1e0 is ordered by that size.

20127642 takes size in EAX. Zero returns NULL. A hit removes the selected array
entry and registers the allocation under critical section2020e1ac. A miss
leaves the lock before allocating size+4 from the private heap, writes the size
header and reacquires the lock to register the block. The caller gets header+4.

2012772b takes the payload on the stack and returns with RET4. It finds the
allocation by its header pointer in the live list and removes that entry. At
the cache limit it removes and frees one entry chosen by rand modulo the limit.
It then inserts the returned block in size order. A live-list miss exits after
releasing the lock. The source retains the original container-helper calls as
explicit boundaries because their register arguments and growth behavior need
separate reconstruction. The size and cache-limit arithmetic follow the original.

Source in cab_buffer_cache.cpp supplies the buffer allocation and return calls
used by the wrapper. The independent search trial reused the input variable as
the result to test the original argument-slot reuse. VC8 still chose different
register scheduling and produced 75 bytes versus 74 original. The clearer
77-byte first implementation is retained and its source hash matches the first
build receipt. The search question stops at two trials. Allocation243/233 and
return306/305 native/original stay unmatched after one source build each.

Run api-cab-buffer-cache-20261109-001 has object SHA256
5F7437CE23C42E31F7DF0B3F6358885C754075884627B6487F61AD6868F4EE93.
The separate search trial is api-cab-buffer-search-20261109-001. Twelve gates
pass on the retained first run. The ledger records 4735 complete code bytes and
44 metadata bytes. The three new source bodies contribute no accepted bytes.

## Verified pool correspondence

20124903 and 20127a04 each have 167 instructions and 23 internal branches to
corresponding instruction positions. Their 15 operand differences are retained
in artifacts/api.dll/api-cab-pool-correspondence-20261109.json. The common control flow
supports sharing the eventual source after the lock and slot contracts are recovered.

| Pool | Owner pointer | Slots per block | Slot bytes | Block bytes | Initializer |
|---|---|---:|---:|---:|---|
| File stream20124903 | 2020e194 | 1024 | 276 | 0x4500c | 20124d34 |
| Wrapped stream20127a04 | 2020e1a0 | 128 | 48 | 0x180c | 20127eae |

Each block ends with three DWORDs for the available count, slot structure
pointer and next block. The owner tracks block head at+8, lock object at+0xc,
block count at+0x10 and peak count at+0x14. Both pools acquire through20146e07
with the lock object in EDI and release via20147aa6. Reservation201464c3 takes
the slot structure in EDI and checks magic4c665374 at+0x38. It removes a node
with a pointer and sequence pair at+8/+0xc, publishes it on the other list and
returns its payload from node+4. The implementation selects LOCK-prefixed or
unprefixed CMPXCHG8B/CMPXCHG/XADD paths using mode byte201f7090. Counter and
semaphore branches are part of the contract.

Pool source remains deferred pending the lock upgrade and slot return pairing.
The next analysis follows 20146e07/20147aa6 and 201462fc with the reservation
helper, then checks both release callers against the shared block model.

Suggested Ghidra names only. In 20127dd3, param_1 can become minimumSize and
iVar5 can become firstCandidate based on the search bounds. In 20127642,
in_EAX can become requestedSize and local_8 can become allocationHeader.
In 201464c3, unaff_EDI can provisionally become slotState and local_8 slotNode.
No renames were applied.
