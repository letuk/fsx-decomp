# API service application registration

Scope: FSX Acceleration `api.dll`. Live verified 2,203 instruction
bytes against the local PE. Names describe the reconstruction.

| Address | Live-verified behavior |
|---|---|
| `2010c714`, `2010c798`, `200fe54f` | The tree at `2020d920` uses unsigned client keys. Open obtains a value slot and packet dispatch queries the same tree. The node's protocol client at +0x10 has its own allocation and lifetime. |
| `200fe4cc`, `2010afc1` | Finds the exact registration, invokes protocol-client teardown at `200ff859`, frees the client, then erases the node. A missing key leaves the tree untouched. The transport client is looked up independently in the signed-key tree at `2020db68`. |
| `200fe28e` | The registration portion of application shutdown walks from the lowest key, captures the transport key from protocol client +0x2e4, destroys and erases the registration, then sends the 12-byte message `{12, 4, 3}`. It also removes an object callback and cleans up additional objects. |
| `200f7bb6`, `200f7c66` | First startup sets the active flag, calls log and console setup, installs packet dispatch and registers five callbacks across owners `2020bb40`, `2020bb34` and `2020bb4c`. It initializes application service objects and calls the service-window entry on every invocation. Shutdown removes the matching callback pairs. |
| `200f7ba9`, `200f77d0` | With main-window dispatch enabled, invokes cached `SimConnect_CallDispatch` entries with `(0, -2, -3)`. This is a call boundary into SimConnect.dll. |

The source uses a VC8 standard map for registration nodes and keeps
protocol client payloads on the existing private heap. It models the
observable lookup and erase behavior without reproducing original
node allocation or checked iterator paths. An empty registration map
exists from source initialization. Shutdown routes registration teardown
through the shared map before delegating remaining service-object
cleanup. For an empty payload slot, it falls back to the entry key.

Protocol client construction, configuration and teardown
(`200ff057`, `201003e2`, `200ff859`), log/console setup, and
remaining service-object initialization and cleanup stay external.
The source entry for `200f7bb6` is implemented. Its original callers
are verified at `200fb3c3`, `200fb91b` and `200fb9fd`, in the
configuration paths beginning at `200fb303` and `200fb4d7`.
Those callers remain outside the source integration.
The listener configuration package subsequently connects both callers.
See api-service-listener-configuration.md.

Suggested Ghidra names, unapplied. In `200fe4cc`, `local_c` →
`registrationNode` based on the exact lookup, +0x10 payload access
and erase call. In `200fe28e`, `local_c` → `transportKey` based
on protocol client +0x2e4 and its use in the outbound send.

VC8 x86 build `api-service-app-registration-20261112-001` passed
all 20 existing comparators. COFF SHA256 is
`148BB090EF4CC6783AE29D23C407F21CDF37749A7A155B0FB515E2426ADAE1EF`.
New byte matches and runtime verification remain open. Ghidra was
unchanged. The next package is listener configuration and the callers
of `200f7bb6`.
