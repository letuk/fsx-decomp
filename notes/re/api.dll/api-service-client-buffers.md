# API service client buffering and I/O continuation

Original `api.dll`. Five retained listings totaling 1035
instruction bytes were verified against the PE. The IAT
identifies WSARecv, WSASend, ReadFile and WriteFile.

`2011aa7f` examines the leading length word, checks it
against client +0x1d0 and publishes a complete message to
the caller. Incomplete data remains buffered. A full
receive buffer is compacted or grown in 0x800-byte steps.
At the observed growth limit the routine can publish the
0xffffffff length sentinel. The source checks replacement
allocations before releasing existing storage and reports
result 4 if the receive buffer cannot grow.

`2011ad8b` prepares the remaining receive span and submits
the socket or pipe read with an operation reference.
`2011ac24` removes one node under the queue lock, transfers
its payload to the active write slot and submits the socket
or pipe write. Immediate I/O errors release the acquired
reference. Source checks the transport before taking the
reference and publishes the write flag before the release
call, which can delete the client.

Suggested Ghidra names, unapplied. At `2011aa7f`, `_Src`
→ `receiveCursor` from the cursor loads and compaction source.
At `2011ac24`, `lpMem` → `writeNode` from the queue unlink
and payload transfer.

`api-service-client-buffers-20261111-001` passed all 20
comparators. COFF SHA256 is
`974994E2EF381601A1BCC92E73AA382EC99B53E75F85F57E577E75C29BE47B64`.
No new byte matches were accepted. Ghidra was unchanged.
Runtime verification remains open. The next boundary is
message interpretation at the client table slot +0x0c.
