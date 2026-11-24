# API receive message handoff

Original `api.dll`. The base table at `200f2838` maps slot +0x0c
to `2011ae4b`. The derived table at `200f23d4` maps the same
slot to `20114cdd`. Constructor writes and the completion call
confirm the receiver and argument shape. Four retained listings
totaling 830 instruction bytes, the table slots and the five-byte
base stub were checked against the PE.

The base target returns zero. The derived target moves the payload
from the three-word receive descriptor into a service queue node
and clears the descriptor. The node receives the descriptor's
second word, the allocation capacity, as its byte count. Its
actual message length remains in the payload header. The derived
path reuses the same queue builder as client delivery at `20114d58`.
Source preserves ownership in the descriptor if enqueue fails
and clears an empty descriptor without sending a null payload
to the router. The original clears the descriptor before routing.

Source supplies ECX receiver adapters with unused EDX and one
stack argument. Table placement and client constructor integration
remain open. The slot is a handoff to the service queue. The next
area is the packet dispatch behind `200fa39d`, which startup
`200f7bb6` assigns to `2020d904`.

Suggested Ghidra names, unapplied. At `20114cdd`, `param_2`
→ `receivedMessage` from its payload read and three clearing stores.

`api-service-message-handoff-20261111-001` passed all 20 existing
comparators. COFF SHA256 is
`1AE73F6EA47F8AADB37F61E6384D4A71D8C0DA0B1ACDAF8A764940733F61BCCC`.
Direct object inspection found the base stub raw-equal to the
original at five bytes with no relocations. It has no dedicated
regression gate and is separate from accepted coverage. The
derived target and runtime verification remain open for matching
and execution respectively. Ghidra was unchanged.
