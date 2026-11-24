# API service transport lifecycle

Scope: FSX Acceleration `api.dll`. Live verified against the local
PE, with 3,598 instruction bytes checked across the lifecycle entries,
tree and vector helpers, window caller, listener entries, and registry
writer. Source names and observed prefixes are reconstruction labels.

| Address | Live-verified behavior |
|---|---|
| `200f74ab`, `200f756f` | The window initializer forwards its stack argument in ECX. Its original entry returns with `RET 4`. This supersedes the earlier zero-argument source declaration. |
| `20115d07`, `20110614` | The once flag at `2020da5d` guards allocation of the client-group vector header at `2020da64` and the transport tree at `2020db68`. The allocations are 0x10 and 0x0c bytes. The 0x18-byte sentinel has self-links at +0/+4/+8 and color and sentinel bytes set at +0x14/+0x15. Tree count at +8 starts at zero. The private heap allocation path is `20135dd9`. |
| `20115de8`, `20115df8` | Reads `SimConnect/InprocDisable` once into the byte at `2020da5c`. Each call compares the configuration's first string with `Pipe` via the imported VC8 string comparison. The pipe entry at `20115ad8` takes the configuration on the stack. The socket entry at `20115689` takes it in EDI. Both read further fields beyond the string. |
| `20115e42–20115e99`, `200fe46f`, `2010ecac`, `201112ae` | Shutdown publishes port zero under `SimConnect_Port_IPv4` and `SimConnect_Port_IPv6` when their flags are set. It erases the tree before touching the client groups. The subtree helper recursively frees right branches, iterates left and stops at the sentinel. It releases node storage only. |
| `20115ea5–20116032`, `2011656d` | With a group header present, clears the client accept byte at `2020d93b`. The outer vector holds group headers and each inner vector holds client pointers. The inner loop calls client vtable slot +8 with two zero arguments and releases the reference at client +0x1d4. Final release shares the cleanup sequence observed at `20114b46`. Inner storage and headers are then freed. |
| `20116038–2011611f` | Closes valid listener sockets, waits at most twenty 250 ms intervals on worker count `2020de24`, conditionally cleans up Winsock, and frees the outer vector. Frees and clears auxiliary entries using `201f9810` and `201f981c`, retaining the backing list and count. The once flag remains set. |

The source uses a typed pointer to the observed VC8 string prefix and
normal stack-call adapters. The listener producers, registry writer,
worker state and auxiliary-list producers remain external boundaries.
The later client construction package identifies the wait value as a
live client count and connects both it and the auxiliary name list.
See api-service-client-construction.md.
The source handles sentinel allocation failure by freeing the tree
header and leaving the tree pointer null. Vector walks assume valid
ranges and omit the original CRT checked-access branches.

Suggested Ghidra names, unapplied. In `20115d07`, `local_14` could
be split into `clientGroupStorage` and `transportTreeStorage` for the
0x10 and 0x0c allocation outputs. The register loaded from `2020da64`
at `20115e91` represents the client-group vector header.

VC8 x86 build `api-service-transport-owner-20261111-002` passed all 20
existing comparators. The COFF SHA256 is
`6825604181065FAFEB679E0CB8274C11833D761C676D52F8DE6F09ED90A73F27`.
New byte matches and runtime verification remain open. Ghidra was unchanged.

The next API package is listener startup and publication at
`20115689`, `20115ad8` and `2011523c`.
