# CAB pool slot transfer

Follow-up api-cab-lock-lifecycle.md adds release, lock and owner lifecycle.
Only the acquisition side of the pool lock pair remains an external source
dependency. Slot and lock paths share the CAS64 instruction helper.

2026-11-09. Original api.dll SHA256 is
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Six listings covering 1552 bytes match the original PE. Addresses and hashes
are in artifacts/api.dll/api-cab-slot-transfer-evidence-20261109.json. Live queries
report saved revision1793741866691:2a9d629f:3. Ghidra access was read-only.

## Observed transfer contracts

201462fc receives the slot state in EDI and payload on the stack and uses
RET4. It checks both pointers and signature4c665374. It takes a node from the
free head at+0, growing the node storage if needed and permitted by budget.
It stores the payload at node+4, pushes the node onto the published head at+8
and returns1. Failure returns0. 201464c3 receives the state in EDI and takes
a node from the published head. It saves its payload, puts the node back on
the free head and returns the saved payload. An empty or invalid state returns
NULL. The source uses explicit stack signatures for both original register
interfaces and connects these implementations to the shared pool routines.

When the source list has a semaphore, the transfer first decrements its count.
The signed value after the decrement controls whether to attempt a pop.
A failed take rolls back the count when a semaphore was present. Publication
can grow before that rollback. On the destination list, the transfer increments
the count when a semaphore is present and releases one waiter if the signed
value after increment is nonpositive. These operations use DWORD arithmetic
before interpreting the result as signed, preserving x86 wraparound.

## Atomic instruction forms

The pop uses CMPXCHG8B on the pointer/sequence pair. It reads the candidate
node's next pointer and proposes sequence+1. A failed comparison supplies the
current pair to the next iteration. The push changes only the pointer using
CMPXCHG, rewriting node->next on each retry. The sequence changes on pop.

| Operation | LOCK predicate on signed mode byte201f7090 | Original examples |
|---|---|---|
| Tagged head pop | mode >= 0 | 20146519 / 2014652a |
| Pointer head push | mode >= 0 | 201465f5 / 201465ff |
| Wait count decrement | mode >= 0 | 20146563 / 20146569 |
| Wait rollback or increment | mode > 0 | 201465c7 / 201465cd |
| Growth budget compare exchange | mode > 0 | 201461fb / 20146201 |

cab_slot_state.cpp implements three small x86 assembly primitives and keeps
the transfer loops and state transitions in structured C++. This design
keeps each unprefixed operation a single instruction. The source abstraction
separates the mode predicate from the primitive and introduces no C++ goto.
Mode-dependent native forms are verified in the compiled object. Runtime
thread scheduling, wait/wake behavior and final linking remain open.

## Build and scope

Run api-cab-slot-transfer-20261109-001 under vm-share/modules/api/runs has object
SHA256 9737DF04E4216E68EFA2F3A20A11F9172A06C02BF74E11E4799B8C8F30B32423.
cab-slot-transfer-diagnostic.json records the two emitted forms of CMPXCHG,
XADD and CMPXCHG8B and the body sizes. Native publication is123/455 bytes and
reservation121/392 versus original because both share helpers and model EDI
as a stack argument. Their first source build is outside body acceptance.
Twelve existing comparison gates pass at 4735 code bytes and 44 metadata bytes.
No additional match-shape trials were made at these boundaries.

Suggested Ghidra names are unaff_EDI to slotState in both routines and
local_8 to transferNode based on the head pop and push. In 201464c3 iVar6 can
provisionally become returnedPayload because it comes from node+4. No names
were applied. The next remaining pool dependency is the lock wait/wake
pair 20146e07/20147aa6, starting with release states and the modes used by
the file and wrapper pools.
