# simscheduler.dll DLL startup and CRT dispatch

Live verified in simscheduler.dll. PE entry `0x20cc6988` initializes
the security cookie on process attach and forwards the notification
to the CRT dispatcher at `0x20cc686d`. On attach the dispatcher calls
the raw module hook, CRT initialization and the module's DllMain in
that order. Detach starts at DllMain, then calls CRT initialization
with the detach reason and invokes the raw hook on success. A failed
process attach from DllMain takes the corresponding cleanup path.
The dispatcher clears the notification reason through `0x20cc6978`.

The raw callback slot at `0x20cc1178` points to `FUN_20cc4980`. Its
process attach and detach calls use API targets `api.dll:0x2013623e`
and `api.dll:0x201362ce`, checked through the original PE import and
export tables. DllMain at `FUN_20cc49c0` records the module handle at
`0x20cc835c` and calls DisableThreadLibraryCalls on process attach.
The C++ source implements that behavior and always returns success.
The original has a second syntactic copy of the call in a branch
that has already excluded the process-attach reason. The C++ probe
emits a single call and a 35-byte body against 60 original bytes.

CRT initialization at `0x20cc66c9` manages the process attach count,
serialized startup state and C and C++ initializer tables. On
detach it traverses registered exit handlers in reverse order.
`0x20cc6677` prepares the exit-handler table, initializes the RTC
support and registers RTC and type-information cleanup.
`0x20cc7021` records SSE2 availability through the CRT CPU probe.
The standalone link probe has its own CRT tables. The comparison
checks table-boundary roles, the two C initializers and the raw
loader hook pointer. The scheduler's previously reconstructed C++
initializer entries remain outside the linked probe.

| Entry | Scope | Original / linked bytes | Status |
| --- | --- | --- | --- |
| `0x20cc4980` | Raw module hook | 48 / 48 | Normalized |
| `0x20cc49c0` | DllMain | 60 / 35 | Diagnostic |
| `0x20cc6677` | CRT pre-C initializer | 77 / 77 | Normalized |
| `0x20cc66c9` | CRT attach and detach initialization | 415 / 415 | Normalized |
| `0x20cc686d` | CRT notification dispatcher, including embedded handlers | 267 / 267 | Normalized |
| `0x20cc6978` | Notification reason reset | 11 / 11 | Normalized |
| `0x20cc6988` | PE entry | 33 / 33 | Normalized |
| `0x20cc6c08` | EH4 prologue | 69 / 69 | Normalized |
| `0x20cc6c4d` | EH4 epilogue | 20 / 20 | Exact |
| `0x20cc6c8e` | Security cookie initialization | 148 / 148 | Normalized |
| `0x20cc7021` | SSE2 availability initializer | 13 / 13 | Normalized |

All builds use the VC8 SP1 compiler. The installed SP1 CRT emits
451 bytes for `0x20cc66c9`. The pinned RTM CRT emits the matching
415-byte body. The retained third build also links the RTM SSE2
member through `___sse2_available`. It verifies 79 fields across
the nine normalized bodies and independently validates the three
pointer fields in the 40-byte EH4 scope table at `0x20cc7340`.
The CRT library selection provenance is in the
[existing CRT library analysis](../fsx.exe/crt-library-context.md).

- [Ghidra listings, scope table and CRT slots](../../../artifacts/simscheduler.dll/dll-startup-ghidra.json)
- [Retained linked worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-dll-startup-20261116-003/receipt.json)
- [CRT dispatcher comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-dll-startup-20261116-003/comparison-20cc686d.json)
- [CRT initialization comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-dll-startup-20261116-003/comparison-20cc66c9.json)

The 49 preceding native gates and retained Flight save and GPS
comparisons pass. The new results apply to the linked startup probe.
Runtime and complete DLL linkage remain unverified. Ghidra reports
no unsaved changes following inspection and dry-run disassembly.
The CRT SSE2 detection and exception path follow in the
[SSE2 area](simscheduler-crt-sse2-area.md).
