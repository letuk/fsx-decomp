# API service client delivery and sorted index

Original `api.dll`. Nine retained instruction listings totaling
1662 bytes were checked against the PE. Live cross-references
identify the index mutation callers. Names and field roles are
reconstructed.

`200f7177` uses its first argument as the index key and forwards
the third and fourth arguments as byte count and payload when
the entry has a client. `20114d58` builds the observed 0x24-byte
queue node, snapshots client +0x1cc and +4, and hands it to
`200f76d7`. This connects the registration callback to the
existing enqueue and dispatch paths. The source initializes
both timestamp words to zero and stops before enqueue on missing
queue or allocation failure. The original calls the router
with a null node after allocation failure.

The sorted index header at `201f9820` begins empty, with growth
5 and flags 4. The source supplies the storage and supports
lower-bound lookup, unique insertion and removal. Insertion
uses the growth path of `20117204`, then shifts the tail at the
saved lower-bound position. It preserves the previous index
on allocation failure. Removal shifts the remaining entries
without releasing the client itself. The observed generic
capacity helper has additional shrink behavior beyond insertion.

The index mutation callers at `20114ee9` and `20114e71` take
the critical section at `201f9830`. They cross into client
connection setup and teardown using sockets or named pipes.
Those transport routines own the lock and client lifetime and
are the next API package. SimConnect module bodies are separate.

Suggested Ghidra names, unapplied. At `20114d58`, `unaff_ESI`
→ `client` from the record fields and node back-pointer, and
`local_8` → `queueNode` from the allocation and router call.

`api-service-client-delivery-20261111-001` passed all 20
comparators. COFF SHA256 is
`3798A77CC8E085E68BA9373E306397BD6CF57A1C2F3FCECD9DF552CFBE007679`.
No new byte matches were accepted. Runtime verification remains
open. Ghidra was read-only.
