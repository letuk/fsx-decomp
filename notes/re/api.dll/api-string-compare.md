# API ordinal 160 string comparison

Live verified for `api.dll` at `0x20137fd2`. The original PE export
slot identifies ordinal 160 and Ghidra names the entry `FUN_20137fd2`.
The 54-byte function compares two null-terminated strings as unsigned
bytes, checks for termination before loading the second byte of each
pair and returns -1, 0 or 1. The two stack arguments and `RET 8`
support the reconstructed `__stdcall CompareStringsPilot` signature.
Ghidra records five internal call sites in `FUN_200fc2b7` in addition to
the export. Read-only state was saved at `1793741866691:51f17f6c:3`.
Suggested Ghidra names `param_1` → `left` and `param_2` → `right`
follow the two compared string operands.
[Live listing and references](../../../artifacts/api.dll/string-compare-20261119/ghidra-20137fd2.json).

The experimental C++ source calls `strcmp`. The VC8 RTM compiler
expanded it inline under the factory `/O2` profile. Run
`api-string-compare-20261119-001` emitted all the original instructions
plus a five-byte jump and LEA at offset 0x0b before the loop.
Run 005 emitted the same 59 bytes in a separately linked LTCG DLL
fragment. Size optimization in run 002 and disabling time optimization
in run 004 produced the same 50-byte body with different registers,
increments and epilogue. Disabling global optimization in run 003
produced a 104-byte body. The alignment bytes in the closest body are
actual code bytes. The complete match remains open. The five run
snapshots retain source, effective commands, receipts and build logs.
[Native and linked comparison](../../../artifacts/api.dll/string-compare-20261119/comparison.json).
