# API service client connection reset and close

Scope: FSX Acceleration `api.dll`. Live verified 1,362 instruction bytes
from six listings and separately read the 46 bytes of the three
application callback adapters. Names describe the reconstruction.

| Address | Live-verified behavior |
|---|---|
| `2011a7ed`, `2011a4eb` | Drains the embedded write queue, frees the active write payload, and clears write state. Assigns a connection generation from `2020de20`, skipping zero, and restores receive cursors. Pipe clients create two manual-reset I/O events. The generation at +0x1cc is distinct from the transport key at +4. |
| `2011a8d4` | Clears connection state and generation under the optional client lock. Connected sockets call shutdown and CancelIo before closesocket. Pipe teardown closes its handles. When requested, records the transport-specific last error and caller's reason. Writes the report to client +0x1d8 and sets the global pending flag at `2020de28`. |
| `20116120` | Consumes the global pending flag and scans client groups when pending or forced. For socket clients still in state 1, probes socket option 0x700c and calls vtable +8 with status 7 and reason 0xc000013f when the finite connection time exceeds three seconds. Returns one client key, status and reason, consuming its status with InterlockedExchange. |
| `200f7a62` | Logs the report, calls `200fe4cc` to remove the connection's application-side registration, and calls the transport client's vtable +4 to restart acceptance if the key resolves. The scan repeats with the caller's force flag. |
| `200f7b7b`, `200f7b95`, `200f7b9f` | Application callbacks force a scan every fourth invocation (starting at zero), scan when pending, and always force a scan, respectively. The original wrappers use RET 8. The source adapters follow the existing callback declaration used by the shutdown package. |

The close path hands reports to application callbacks through the
pending flag and client fields. Message-window packet delivery has
its own queue. The source checks for an absent group header and
skips CloseHandle on a null pipe event. It retains the observed
state-gated handle cleanup and uses the numeric socket option
because the VC8 SDK header does not expose its name.

Suggested Ghidra names, unapplied. In `2011a8d4`, `iVar1` →
`previousConnectionState` based on the transition to zero. In
`20116120`, `local_8` → `connectTimeSeconds` based on the socket
option and threshold comparison.

VC8 x86 build `api-service-client-connection-20261112-002` passed
all 20 existing comparators. COFF SHA256 is
`09640F92C0EB87B1CD5F8C36445C08E4E6BBFD63C2D18BF8FC5CCE8E363EA30C`.
New byte matches and runtime verification remain open. Ghidra was
unchanged. The application-side registration tree at `2020d920`,
its removal at `200fe4cc`, and callback registration in the
startup path at `200f7bb6` remain source integration boundaries.
These two entry points were subsequently connected in
api-service-application-registration.md.
