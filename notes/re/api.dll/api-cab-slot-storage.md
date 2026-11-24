# CAB slot state and node storage

The follow-up in api-cab-slot-transfer.md implements the atomic head operations
and connects publication/reservation to the pool source. Pool lock wait/wake
remains external.

2026-11-09. Original api.dll SHA256 is
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Nine original-byte verified listings cover 1544 bytes. Evidence and hashes
are in artifacts/api.dll/api-cab-slot-storage-evidence-20261109.json. Live read-only
api.dll queries report saved revision1793741866691:2a9d629f:3.

20145fe3 creates a zeroed 0x48-byte slot state through the aligned allocator.
The original raw allocation is stored at+0x3c and byte accounting at+0x34.
Signature4c665374 is at+0x38. The requested growth chunk is clamped to 16–256
nodes, then reduced to a smaller positive limit. Limits below 1 become -1
and limits at or above0x0fffffff become0x0ffffffe. Lists and semaphore handles
start empty. The mode byte at201f7090 is initialized from GetSystemInfo when
it is 2. A single processor with at most one active-mask bit selects -1.
Other configurations select 1. The source retains synchronization boundaries
and initializes scratch locals whose original values are discarded.

201460ff takes the state in EAX and returns 1. InterlockedExchange clears
the signature. The valid-signature path closes both present semaphores,
frees the linked chunks and then releases the raw state allocation. Raw
process-heap and allocator-callback paths are separate. The source keeps
that ordering and the unconditional result.

201461b4 reads the growth chunk size at+0x28 and remaining budget at+0x2c.
Zero budget returns NULL. A nonnegative budget is reserved using compare
exchange, with the reduced count retained across retries. A negative budget
permits growth without reserving from it. Each node is eight bytes. The first
is returned and the remaining nodes are published in reverse address order.
The allocated-node counter at+0x30 is increased by the full count. Failed
chunk allocation sets the budget to zero. With a free-list semaphore present,
the waiter count and number of surplus nodes determine the release count.

## Storage and accounting

20145d4c takes its optional allocator in EAX. The default path uses the process
heap and accounts the positive HeapSize result. A custom allocator contributes
the requested byte count even if its callback returns NULL. 20145e2f receives
allocator in EAX, size in EDX and the raw allocation output in EDI. It derives
the alignment from size and allocator guarantee, allocates the extra space
and returns the aligned address. The raw pointer is retained separately.

20145ec2 reserves eight extra bytes after the aligned node array. This trailing
node links into the chunk head at state+0x20 and stores the raw pointer at+4.
20145f32 removes these trailer nodes using the tagged-head pop and releases
their raw payloads. 20145ddc receives raw allocation in ESI. Its default
accounting subtracts a positive HeapSize before freeing the allocation.
The observed allocator descriptor has alignment at0, context at4 and
allocate/release calls at8/0xc. The source uses that observed prefix.

## Build and next package

cab_slot_state.cpp adds eight compiled source bodies for creation, destruction,
growth and backing storage. State layout asserts cover the observed72 bytes.
The initial build api-cab-slot-state-20261109-001 covers lifecycle and growth.
The follow-up api-cab-slot-storage-20261109-001 adds the five storage helpers.
The final object SHA256 is
30E9ED6F804642CABF97DB3EE3614B6F23FC6E536A2A0FB255AAACBB23723899.
The run's cab-slot-storage-diagnostic.json records all sizes and references.

| Entry | Native/original bytes |
|---|---:|
| 20145fe3 | 205/284 |
| 201460ff | 174/181 |
| 201461b4 | 286/328 |
| 20145d4c | 144/144 |
| 20145e2f | 147/147 |
| 20145ec2 | 99/112 |
| 20145ddc | 82/83 |
| 20145f32 | 162/177 |

The equal sizes still have different argument storage. All eight remain outside
body acceptance. Twelve gates pass at 4735 code bytes and 44 metadata bytes.
The builds cover distinct source additions. Further shape trials are deferred
at the known register and atomic primitive boundaries.

Suggested Ghidra renames include 20145fe3 param_1 to requestedChunkNodes and
param_2 to nodeLimit, 201460ff in_EAX to slotState, and 201461b4 iVar6 to
reservedNodeCount. The size clamps, signature clear and budget operations
support those roles. Ghidra edits were not applied. Next recover the exact
tagged pop and head push contracts, including each LOCK threshold, and use
them to complete 201462fc and 201464c3. The full lock wait/wake remains separate.
