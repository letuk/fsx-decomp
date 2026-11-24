# API service listener configuration

Scope: FSX Acceleration `api.dll`. Live verified 2,059 instruction
bytes against the local PE. All eleven source GUID constants match
the corresponding 16-byte PE records.

| Address | Live-verified behavior |
|---|---|
| `200fb303`, `200fb3f6`, `200fb464` | Default selector 0 starts IPv4, 1 starts IPv6, and 2 starts Pipe. Defaults are local scope, an empty address, endpoint text `0`, 64 clients, receive limit 0xa080, and no-delay off. The configuration contains four VC8 strings. |
| `200fb4d7`, `20133f57`, `2013341b` | Resolves `SimConnect.xml` with selector 0x1a, checks its attributes and opens the property set. An absent, unreadable or document-disabled configuration leaves the built-in listeners eligible. |
| `200fb5d4–200fb938` | Enumerates child property sets through slots +0x38 and +0x3c. Entry defaults use transport `Auto`. Nonempty text overrides transport, scope, address and endpoint. The client count is capped at 1,000. The receive limit is raised to the configured value if larger, capped at 0xffff. The no-delay value is passed through the field at +0x78. |
| `200fb875–200fba34` | An entry with an explicit Pipe, IPv6 or IPv4 transport, local scope and endpoint `0` suppresses that transport's built-in listener, including when the entry itself is disabled. Enabled entries start in document order. Remaining defaults start Pipe, IPv6, then IPv4. `Auto` leaves all three default flags intact. |
| `200fb207`, `200fb243` | API ordinal 322 calls listener configuration after an initialization branch and before two further startup routines. The call is gated by `201f7d80 == 0`. |

The source retains the original GUID-based schema boundary.
The file reader and property accessors are imports from `simprop.dll`.
The child enumeration and release calls use the imported objects'
observed stack-call slots. The schema reader implementation and the
path builder at `20133f57` remain external. The API source uses scoped
release for property sets, including non-null pointers returned with
failure, and guards a successful read with a null document.

Suggested Ghidra names, unapplied. In `200fb4d7`, `local_218`,
`local_217` and `local_216` → `startDefaultIpv4`,
`startDefaultIpv6` and `startDefaultPipe`, respectively, based on
the comparisons that clear them and the fallback calls that read them.
`local_21c` → `documentProperties` based on the file-read output.

VC8 x86 build `api-service-listener-config-20261112-001` passed
all 20 existing comparators. COFF SHA256 is
`7F33AD5037DDD9CD2C6DFF61EC9A1B4E47F988DDC86F80D4D74830603669A66E`.
New byte matches and runtime verification remain open. Ghidra was unchanged.
The next boundary is API ordinal 322 at `200fb207`, including its
startup gates and the routines following listener configuration.

The ordinal 322 caller is connected in the add-on startup source
package. See api-addon-startup.md for the verified gates and ordering.
