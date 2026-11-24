# API outbound service packet delivery

Original `api.dll`. Four retained listings totaling 685
instruction bytes were verified against the PE.

`20116676` finds a transport client by signed key in the tree
at `2020db68`. The node at +4 of the tree header is the
sentinel. The source uses the observed exact-key result and
shares it with the transport generation lookup at `2011639f`.
This tree is separate from the sorted client index at `201f9820`.

`2011628d` transfers the buffer to client callback +0x204 if
the callback exists and client state is nonzero. The four
stdcall arguments are generation, registration context,
byte count and buffer. Otherwise it creates a 0x14-byte
write node. `2011a9d8` uses the optional client lock and
the queue lock, enqueues in state 2 and calls the existing
write continuation. Disconnected clients discard the node
and payload.

Source releases the buffer when no transport tree exists or
when node allocation fails. The original leaves the buffer
on the no-tree path and forwards a null node after allocation
failure. Source shares the queue append and cleanup operations
and uses only the first 0x14 bytes of the common node layout.

Suggested Ghidra names, unapplied. In `2011628d`, `iVar1`
→ `transportClient` after the +0x10 node load and `local_10`
→ `writeNode` when reused as the allocation output.

`api-service-packet-send-20261111-001` passed all 20 existing
comparators. COFF SHA256 is
`5B3BC29A7BD8B8A04D293F128F11AF706DEC900640F57DDAE96965046AEBD6C9`.
New byte matches and runtime verification remain open.
Ghidra was unchanged. The next API boundary is ownership
of the transport tree at `20115d07` and `20115e3a`.
