# simscheduler.dll first pass

Original PE SHA-256 `16059B50E722BFA10357582DEF8D422A8222F620189E40C8878928A7DDAA9DD4`.
Image base `0x20cc0000`. Nine ordinal-only PE exports. Ghidra reports 229 functions including runtime code.

## Live verified

- `0x20cc1460` (ordinal 1) passes callbacks `0x20cc13c0` and
  `0x20cc1400` to API ordinal 231 with masks `0x20` and `0x10`.
  Ordinal 2 at `0x20cc1490` passes the same pairs in reverse order
  to API ordinal 232, then calls slot `+0x04` on each linked node.
- `0x20cc1400` dispatches messages `0x25`, `0x26`, and `0x171`.
  Its first two handlers `0x20cc4790` and `0x20cc48d0` load and
  save `SimTime` and `SimSpeed` under `SimScheduler` through
  flight.dll ordinals 13 and 14 respectively.
- `0x20cc14d0` (ordinal 3) returns the object address `0x20cc8370`.
  Exports 6–9 use the table pointer in its first word at byte offsets
  `+0x20`, `+0x1c`, `+0x14`, and `+0x18`, respectively.
  The save handler ties `+0x20` to SimTime and `+0x18` to SimSpeed.
- `0x20cc14e0` (ordinal 4) stores its 32-bit input at `0x20cc8368`
  and returns with `ret 4`. `0x20cc2ce0` compares its second input with
  that value. Its semantic role remains open.
- `0x20cc1500` (ordinal 5) calls `0x20cc1cc0` and stores the input
  at `0x20cc8364`. Its callee uses the literal `SimConnectFallback`
  for object lookup and lifecycle calls.
- `0x20cc30d0` compares time values, advances a modulo index and invokes
  callbacks from indexed records. The calling convention and record layout
  require listing verification.

## Matching order

Start at ordinal 3. Compare the complete six-byte body and resolve
the operand relocation to `0x20cc8370`. The original PE has a HIGHLOW
fixup at `0x20cc14d1`. Follow with ordinal 4 to test the one-argument
ABI and hotpatch prologue. Its HIGHLOW fixup is at `0x20cc14e9`.
Then try ordinals 7 and 9 and trace the scheduler object's initialization
and each table target and signature.

[PE and Ghidra first-pass evidence](../../../artifacts/simscheduler.dll/first-pass.json).
The build-profile and COFF status from this first pass was superseded by the
[getter match](simscheduler-object-getter-match.md).
Linked image and runtime verification are pending. Ghidra read-only,
save state unverified.
