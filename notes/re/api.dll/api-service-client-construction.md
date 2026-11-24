# API service listener client construction

Scope: FSX Acceleration `api.dll`. Live verified 3,859 instruction
bytes against the local PE and separately read the table entries and
auxiliary-list initializer. Names describe the reconstruction.

| Address | Live-verified behavior |
|---|---|
| `2011a549` | Initializes the base queue, its heap-allocated lock, transport state, invalid socket and event fields, receive limit default 0x400, and reference count 1. It increments `2020de24`. The base table is `200f2838`. |
| `20114b77`, `2011558e–201155c7` | Pipe and inline socket construction install the derived table at `200f23d4` and initialize its tail fields within the 0x208-byte allocation. Pipe construction creates a manual-reset event at +0x1f4. |
| `20114bf8–20114c25`, `20116ee1`, `20116fc5` | A pipe client with a valid event adopts a custom name by pointer identity. Shared names are registered once in the auxiliary list at `201f981c`. Count and capacity are at `201f9810` and `201f9814`. The PE initializer selects a base capacity of five and doubling. Transport shutdown owns the name buffers. |
| `2011a66f` | Stores the listener, receive-size limit, address family and no-delay flag, then allocates a 0x800-byte receive buffer and a 0x0c-byte message record. It initializes overlapped records and the embedded lock, and assigns the client key at +4 from `2020de1c`. |
| `201155f8–201155fe`, `201153c1`, `20115c75`, `20114f6f` | Socket setup supplies the current `addrinfo::ai_family` in ECX. The local at EBP−0x3c tracks the current resolution result. Pipe setup supplies zero in ECX. The accept routine reads +0x174 as the socket family. |
| `20114c34`, `20114c61`, `2011a612` | Deletion releases the pipe wait/event and queue lock and decrements `2020de24`. This confirms that the transport shutdown wait counts live clients. The final-release caller already handles I/O storage cleanup. |
| `20114dbd` | Derived activation accepts state 1. Socket activation extracts two address records, copies 0x80 bytes each to client +0x68 and +0xe8, and registers completion before setting state 2. Pipe activation sets state 2 and continues reading. `GetAcceptExSockaddrs` is a `mswsock.dll` import. |

The five derived slots at `200f23d4` resolve to deleting destructor
`20114c34`, start `20114ee9`, stop `20114e71`, message delivery
`20114cdd`, and activation `20114dbd`. The source installs this
dispatch table using x86 receiver adapters. The base-table transition
during construction and destruction is represented by direct base
initialization and cleanup. The reconstructed table is not an original
symbol or RTTI layout.

Source allocation failure can stop group population and release
partially configured clients. Auxiliary growth preserves its old
storage on allocation failure. The source uses interlocked updates for
the live count shared with completion callbacks. Reset and close
operations at `2011a7ed` and `2011a8d4` remain existing source
dependencies. The `mswsock.dll` address-extraction call remains an
import boundary.
The reset and close dependencies were subsequently connected in
api-service-client-connection.md.

Suggested Ghidra names, unapplied. In `20115314`, `local_40` →
`currentAddressInfo` based on traversal through +0x1c and the family
load at `201155fb`. In `2011a66f`, `param_1` → `addressFamily`
based on the socket caller and the use in `20114ee9`.

VC8 x86 build `api-service-client-construction-20261111-002`
passed all 20 existing comparators. COFF SHA256 is
`E7DF4A3442B19762F1387A93F3F62EFB74E631A08D8FE26C0266BF52AD2391FF`.
New byte matches and runtime verification remain open. Ghidra was
unchanged. The next boundary is transport-tree insertion at
`201165f2` and its balancing helpers.
That boundary was subsequently connected in api-service-transport-tree.md.
