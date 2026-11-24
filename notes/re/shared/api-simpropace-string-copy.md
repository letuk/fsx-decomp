# Bounded ANSI copy in api.dll and simpropace.dll

## 2026-11-15 cross-module findings

The [fingerprint scan](../../../artifacts/shared/fsx-api-copy-fingerprint-20261115.json)
covered executable sections and ordinary imports in 47 supplied x86 PEs.
It permitted only three validated IAT operands to move for the 169-byte
exact-body search. Only `api.dll:20137e17` matched. An ordered reference
search found another _getmbcp, lstrcpynA and _ismbblead sequence in
`simpropace.dll:00440740` (stored name `FUN_00440740`). Twenty-six
supplied images import api.dll ordinal 157, which is evidence of API
consumption rather than static copies. Other code shapes and dynamic
imports were outside the scan.

Live verified in Ghidra and checked against the original PEs. Both
functions use the same imported branches and place the destination in
EDI and source in ESI. The 27-byte trail-copy sequence at
`api.dll:20137e6a` and `simpropace.dll:0044078d` agrees outside two
one-byte branch displacements. Both the single-byte NUL case and the
trail-byte NUL case reach the same final store in both functions.
The API helper copies the first byte before branching on the lead-byte
test. The simpropace helper copies it separately in each branch.

The 167-byte `simpropace.dll:00440740` reads destination in ECX, source
in EAX and capacity on the stack, then returns with RET 4. Call sites
`0044081c` and `0045de6b` corroborate all three locations. That
supersedes the stored `__thiscall` signature as an explanation of the
full ABI. The stored signature has not been edited. The API entry is
a public three-stack-argument helper and returns with RET 0x0c.

Shared source or an earlier common helper is a hypothesis. The evidence
does not identify the source owner or a library archive. See the
[PE bytes, call sites and live Ghidra listings](../../../artifacts/shared/fsx-api-simpropace-copy-comparison-20261115.json).
Both Ghidra programs reported saved states and were read-only.
