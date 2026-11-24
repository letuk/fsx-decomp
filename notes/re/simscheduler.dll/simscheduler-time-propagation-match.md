# simscheduler.dll time propagation at 0x20cc1a40

Live verified in simscheduler.dll. `FUN_20cc1a40` receives the
scheduler object in EBX and an eight-byte time value on the stack.
It updates object offset `+0x04`, walks the source array at
`+0x3c` using a `0x24`-byte stride, and calls each source's
VTable slot `+0x4c` with an explicit stack object argument.
It propagates the value to each nested entry at `+0x04` and
the returned group at `+0x44`. The wrapper returns with `ret 8`.
The indirect call slot is verified. The set of dynamic targets and
their concrete signatures remain open. C++ names and layouts are
reconstructed minimum models.

| VC8 x86 LTCG approach | Linked body |
| --- | ---: |
| Double assignment and default frame in the retained dispatcher probe | 78 bytes |
| EBP frame and internal stdcall helper | 84 bytes |
| Same frame and call profile, byte-preserving memcpy stores | 116 bytes |

The 116-byte build matches the EBX object input, stack time argument,
callee cleanup and paired integer stores. It reuses two registers for
the time words, while the 138-byte original materializes intermediate
x87 copies in stack locals. Loop scheduling and the indirect call
register also differ. The linked PE comparator checks the complete
original and rebuilt byte ranges and the VTable slot in the listing.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/time-propagation-ghidra.json)
- [116-byte comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-propagate-time-20261116-002/comparison-20cc1a40.json)
- [84-byte comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-propagate-time-20261116-001/comparison-20cc1a40.json)
- [78-byte comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-ltcg-20261116-002/comparison-20cc1a40.json)

The nine SimScheduler export COFF gates and GPS regression passed.
Matching stopped after three approaches with the best linked diagnostic
retained. Runtime verification is pending. Ghidra read-only, save state
unverified. Next candidate `FUN_20cc48d0`.
