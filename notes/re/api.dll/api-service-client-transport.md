# API service client transport lifecycle

Original `api.dll`. Seven retained listings totaling 1517
instruction bytes were checked against the PE. The IAT confirms
the Winsock, AcceptEx, Named Pipe and wait calls. Source names
and offset roles are reconstructed.

`20114ee9` removes the previous client key under the index lock
at `201f9830`, resets the connection, then inserts the updated
key under the lock. It clears client +0x1e0 before checking the
accept gate at `2020d93b`. Source also stops if insertion fails.
Transport value 0 starts an overlapped socket accept. Value 1
registers a wait and starts an overlapped pipe connection.
Both paths acquire an operation reference before initiating I/O.

`201151b0` handles the pipe wait callback. It releases the
operation reference, checks the pipe and its overlapped result,
binds the completion callback at `2011ae51`, and invokes
client slot +0x10 on successful binding. `20114e71` disconnects
and closes an existing pipe, removes the index entry under
the same lock and forwards the close arguments to `2011a8d4`.

For synchronous failure cleanup, source clears the stored
handle and closes it before releasing the operation reference.
The original releases the reference first. The pipe branch
preserves the observed two GetLastError reads on the pending
path. The window SDK import declarations for the wait APIs
are supplied explicitly in the source.

Suggested Ghidra names, unapplied. At `20114ee9`, `param_1`
→ `client` from the key, handle and I/O fields. At `20114e71`,
`hNamedPipe` → `connectedPipe` from DisconnectNamedPipe
and CloseHandle.

`api-service-client-transport-20261111-002` passed all 20
comparators. COFF SHA256 is
`4EC53B903DDD41B73274056E380213B33AA53D2C93A620A4F3184736CE59FDD0`.
New byte matches and runtime verification remain open.
Ghidra was unchanged. The next API package is I/O completion
and client reference lifetime, beginning at `2011ae51` and
`20114b46`. Winsock and SimConnect are separate module work.
