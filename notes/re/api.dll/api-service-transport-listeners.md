# API service transport listeners

Scope: FSX Acceleration `api.dll`. Live verified 4,376 instruction
bytes from the original PE across the four listener entries and
seven adjacent helpers. All type and function labels are reconstructed.

| Address | Live-verified behavior |
|---|---|
| `20115689`, `2011b0ca` | Startup requests Winsock version 2 and resets the listener count on success. It probes both loopback address families with numeric-host TCP resolution. `Auto` prefers IPv6 and falls back to IPv4 at family selection. There is no fallback after the selected family's bind fails. Explicit `IPv4` and `IPv6` require the corresponding probe. |
| `201157a0–20115a57` | The configuration has VC8 string fields at +0x00 (transport), +0x1c (scope), +0x38 (address), and +0x58 (service or pipe suffix), with the maximum client count at +0x54. The fields +0x74 and +0x78 are forwarded to client setup. Socket scope chooses loopback, link-local resolution through `..localmachine`, or configured/default passive binding. A nonzero numeric `%index` selects an IPv6 link-local interface. Explicit addresses override the default loopback and link-local names. IPv6 passes protection values 30, 20, and 10 for the observed scope branches. |
| `20115314` | Resolves passive TCP endpoints, filters IPv4 169.254/16 and IPv6 fe80::/10 in link-local mode, and records each created socket in the 64-entry shutdown array. It applies exclusive address use, binds, retrieves the assigned port, listens and registers completion entry `2011ae51`. Socket storage remains registered even when a later step fails. A successful endpoint starts its own client group. |
| `20115ad8`, `201166f2`, `20116580` | Caps pipe instances at 255. Endpoint `0` defers to the default pipe name. Other endpoints use a bounded local pipe name. The group is appended before client startup. Client allocations are 0x208 bytes. A case-insensitive `global` scope sets client +0x1e1. Both listener families call client vtable +4 and then publish the client by its key at +4 through `201165f2`. |
| `2011523c` | The local socket path publishes the assigned port, or zero when no port was returned, under `SimConnect_Port_IPv4` or `SimConnect_Port_IPv6`. The writer uses HKCU and the Flight Simulator key, stores decimal text as REG_SZ with the observed string length, and sets the caller's publication flag on success. The shutdown package uses the same writer. |

The source connects listener selection, endpoint setup, group
allocation, client activation and registry publication. Construction
and pipe-name adoption (`20114b77`, `2011a549`), per-client setup
(`2011a66f`), and balanced tree insertion (`201165f2`) remain
external dependencies at this build. Client construction, pipe-name
adoption and setup were subsequently connected in
api-service-client-construction.md. The configuration model ends after +0x78.
Its complete original type and size remain open. The source uses the
SDK's WSABUF in the existing receive and send functions.

The source returns or stops filling a group on allocation failure,
tracks the constructed prefix for shutdown, and frees a custom pipe
name when no client adopts it. It replaces the original checked-vector
failure paths with bounded allocation checks. The original registry
formatter is represented with decimal formatting. The direct diagnostic
formatting uses the reconstructed source call boundary.

Suggested Ghidra names, unapplied. In `20115689`, `unaff_EDI`
→ `configuration` based on the caller and field accesses, and
`local_1ec` → `interfaceIndex` based on the percent prefix parse
and the scope comparison at `201153f4`. In `20115314`,
`local_35` → `noListeningEndpoint` based on its initial value and
the clear at `20115647`.

VC8 x86 build `api-service-listeners-20261111-002` passed all 20
existing comparators. COFF SHA256 is
`671F96B08A9131B606AD1A9DDB3DBE1EC4D75FE0AFD52DDE25E54E537AF7D685`.
New byte matches and runtime verification remain open. Ghidra was
unchanged. The next package is the per-client construction and
configuration called during listener startup, including pipe-name
ownership. Balanced tree insertion remains a separate follow-up.
