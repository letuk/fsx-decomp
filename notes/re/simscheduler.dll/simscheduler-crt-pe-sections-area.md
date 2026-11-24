# simscheduler.dll CRT PE section checks

Live verified in simscheduler.dll. `__ValidateImageBase` at
`0x20cc6af0` checks the DOS signature, NT signature and PE32 optional
header magic. `__FindPESection` at `0x20cc6b20` searches the virtual
address ranges in the section table. `__IsNonwritableInCurrentImage`
at `0x20cc6b67` applies both checks to the current image and tests
the section's write flag. Its EH4 filter handles access violations.

The two header helpers match byte-for-byte over 41 and 66 bytes in all
three retained linked probes. The complete 108-byte section check,
including the embedded filter and handler, matches after normalizing
six individually checked address fields in both pinned RTM builds.
The installed SP1 CRT uses an inline SEH implementation and produces
a 187-byte diagnostic body. Both HIGHLOW pointers in the original
28-byte EH4 scope table at `0x20cc7388` were validated separately
against the filter and handler in each image.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/crt-pe-sections-ghidra.json)
- [Section check comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-dll-startup-20261116-003/comparison-20cc6b67.json)

Existing linked probes were replayed. Runtime behavior remains
unverified. Ghidra was inspected read-only.

Live verified in simscheduler.dll. CRT import thunks `0x20cc6bd8`
(`_initterm`), `0x20cc6be4` (`_initterm_e`), `0x20cc6bf0`
(`_amsg_exit`) and `0x20cc6bfc` (`__CppXcptFilter`) each contain
one six-byte indirect jump. All four match in the retained SP1 and
RTM builds after checking the HIGHLOW operand and exact MSVCR80 IAT
identity. The original inter-thunk padding is captured in the evidence.
The comparison covers the instructions. Imported runtime bodies and
runtime behavior remain unverified.
[Original listings](../../../artifacts/simscheduler.dll/crt-import-thunks-ghidra.json).

Live verified in simscheduler.dll. The EH4 handler at `0x20cc6c66`
forwards the four exception-dispatch arguments together with the
security-cookie address and the cookie-check function pointer to the
CRT common-handler thunk at `0x20cc6e60`. Its full 35-byte body matches
in the retained SP1 and RTM linked images with three validated address
fields. The common-handler IAT identity and cookie-check prefix were
verified separately. The imported implementation and runtime behavior
remain outside the comparison. Ghidra reports no unsaved changes.
[Original listing](../../../artifacts/simscheduler.dll/crt-eh4-handler-ghidra.json).

Live verified in simscheduler.dll. `__security_check_cookie` at
`0x20cc69ae` compares ECX with the cookie at `0x20cc8008` and
tail-jumps on mismatch to `___report_gsfailure` at `0x20cc6d27`.
The reporter captures the x86 context, constructs an exception record
with status `0xc0000409`, invokes the unhandled exception filter, and
calls `TerminateProcess`. Its debugger hook goes through the MSVCR80
import thunk at `0x20cc6e6c`.

The complete 15-, 260-, and 6-byte linked ranges match after address
normalization in the retained SP1, RTM, and RTM plus SSE2 builds
`simscheduler-dll-startup-20261116-001` through `003`. The comparison
checks the cookie and failure jump, exception-record and context
fields anchored by the two relocated EXCEPTION_POINTERS members,
debugger flag, and imported function identities. Ghidra was read-only
and reports no unsaved changes. Runtime execution remains unverified.
[Cookie check](../../../artifacts/simscheduler.dll/crt-cookie-check-ghidra.json),
[failure reporter](../../../artifacts/simscheduler.dll/crt-gs-failure-ghidra.json),
[debugger thunk](../../../artifacts/simscheduler.dll/crt-debugger-hook-ghidra.json).
