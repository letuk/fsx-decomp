# API service client I/O completion

Original `api.dll`. Four retained listings totaling 632
instruction bytes were verified against the PE. The selector
table at `2011afdc` and targets at `2011afd0` were checked
separately. Names and field roles are reconstructed.

The completion callback at `2011ae51` reads the client and
operation kind at +0x14 and +0x18 of the extended OVERLAPPED
record. The status table maps `0xc0000120` to reason 8,
three statuses to reason 5, and other failures to reason 9.
Pipe status `0x80000005` takes the success route. A successful
completion with zero bytes reports reason 4.

The read route updates byte counts and calls the receive
buffer inspector and client message slot +0x0c. Inspector
results 2, 3 and 4 report failures. Other terminal results
request another read. The write route continues pending
output in connected state, with the optional client lock.
Both routes release the operation reference after processing.

`20114b46` decrements client +0x1d4 atomically and returns
a boolean in AL. The final reference drains the embedded
queue, frees the receive and message buffers, clears the
buffer cursors and deletes the optional client critical
section before invoking the deletion slot. This connects the
pipe wait and immediate I/O failure paths to the same release.

Suggested Ghidra names, unapplied. In `2011ae51`, `param_3`
→ `operation` and `piVar1` → `client` from the extended
OVERLAPPED access and reference decrement.

`api-service-client-completion-20261111-001` passed all 20
comparators. COFF SHA256 is
`0128AF976F24C8872F451B9F344C2A2B09B138556443ADA386823C9C488D358B`.
No new byte matches were accepted. Ghidra was unchanged.
Runtime verification remains open. The next API boundary
is receive buffering and I/O continuation at `2011aa7f`,
`2011ad8b` and `2011ac24`. External modules remain separate.
