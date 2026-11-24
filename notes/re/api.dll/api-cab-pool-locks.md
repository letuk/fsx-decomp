# Shared CAB stream pool source

Slot creation and destruction source now build with the node storage routines.
See api-cab-slot-storage.md for their contracts and the remaining atomic head
dependencies.

2026-11-09. Original api.dll SHA256 is
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Seven listings covering 4902 bytes were verified against that PE. Their
addresses and hashes are in artifacts/api.dll/api-cab-pool-lock-evidence-20261109.json.
Live api.dll queries confirm saved revision1793741866691:2a9d629f:3.

20124903 and 20127a04 acquire with mode 0 through 20146e07. They reserve from
an existing block and decrement its available count with InterlockedDecrement.
On growth they initialize a new block, then request mode 3 while the first
acquisition is held. Successful mode 3 permits publication of the new block.
After a failed upgrade they release mode 0, destroy the speculative slot
state and free the block. The retry starts with mode 1. Publication updates
the current and peak block counts and decrements the new block's available
count only when reservation succeeds. Final release uses mode != 0.

The initializer's zero return takes the release path with the new block still
unlinked. This preserves the observed failure edge, including the absence of
a free on that edge. The low-level lock's mode 0 release reduces the shared
count. The nonzero release checks exclusive ownership and recursion before
clearing the exclusive bit and waking waiters as indicated by its state.
These caller contracts cover the pool paths. The full wait and wake protocol
of 20146e07/20147aa6 remains a separate source dependency.

20124b11 and 20127c12 take a record address on the stack and use RET4. After
mode 0 acquisition they find the block whose payload interval contains that
address. They call 201462fc with slot state in EDI and payload on the stack.
An accepted publication increments the block's available count. The address
test is an interval check. The source preserves its handling of interior
addresses. Slot publication at 201462fc removes a node from the head pair at
+0/+4, stores the payload in node+4 and pushes it onto the head at +8.
Reservation at 201464c3 reverses that transfer. Both use the original mode
byte to select atomic instruction forms and have distinct counter and
semaphore branches. Full atomic source remains open.

20124d34 and 20127eae initialize the slot structure through 20145fe3 or drain
an existing one with 201464c3. They publish each record at a fixed stride
using 201462fc, and reset the available count only after all publications
succeed. The file block uses 1024 records of 276 bytes. The wrapped block
uses 128 records of 48 bytes.

cab_object_pools.cpp shares the allocation, return and block initialization
logic across six source entry points. The shared design follows the verified
instruction and branch correspondence. This deliberately emits shared helper
bodies and short entry wrappers, so all six are outside accepted body coverage.
The shared allocation, return and initialization bodies are 597, 113 and 127
bytes respectively. Lock, slot state and low-level publication/reservation
definitions remain external and preserve explicit source arguments.

The first run api-cab-object-pools-20261109-001 builds allocation and return.
The follow-up api-cab-pool-blocks-20261109-001 adds the initializer source.
Its object SHA256 is
1A5F0D795C6909063A2AE22E4C550597C2FE65A9BC5648703147814B46109D73.
Twelve comparator gates pass. Complete body coverage remains 4735 code bytes
with 44 metadata bytes tracked separately. Each build checks a distinct
source addition. Matching at the unresolved lock/slot boundary is deferred.

Suggested Ghidra names are param_1 to returnedRecord for 20124b11/20127c12,
unaff_EDI to slotState and local_8 to publicationNode for 201462fc. The latter
is provisional based on the payload store and head insertion. Ghidra names
and types were left intact. Next recover slot state construction at 20145fe3,
destruction at 201460ff and node storage at 201461b4 before translating the
atomic transfer operations. Full lock wait/wake source is a separate package.
