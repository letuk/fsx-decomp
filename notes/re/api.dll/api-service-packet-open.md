# API service client open and exception responses

Original `api.dll`. Six listings totaling 1002 instruction
bytes were checked against the PE. The product string was
verified at `200f06bc`.

`200f7d7a` receives the 0x128-byte request in ECX and the
client ID on the stack. It checks for a terminated name
within the 0x100-byte field and a zero word at +0x110.
A failed field check sends exception 2 with index 0x10.
The accepted path constructs a client from an observed
0x3d0-byte allocation, passes the five words at +0x114
through +0x124 to the configuration boundary, and inserts
the configured client. Configuration failure tears down
and frees the client.

The open response is 0x134 bytes with type 2, product name,
the observed product and protocol version words and the
transport generation at +0x12c. The transport is marked
ready after attempting the response allocation and send.
`200f7d05` builds the 0x18-byte type-1 exception response.
Its sequence and index originate in EDI and ESI. Both
responses carry version word 4 and transfer the buffer
to the outbound boundary at `2011628d`.

Suggested Ghidra names, unapplied. At `200f7d7a`, `_Dst`
→ `openResponse` from the allocation and product fields,
and `iVar6` → `client` from construction and insertion.

`api-service-packet-open-20261111-001` passed all 20
comparators. COFF SHA256 is
`D31C6082DD75308C69412B5406F17C511DD53AFDED5CA732E117886D1219884F`.
New byte matches and runtime verification remain open.
Ghidra was unchanged. Client construction and configuration,
registry insertion and transport send remain source
boundaries. The next area begins at `2011628d`, where
outbound ownership splits between the registered callback
and the transport write queue.
