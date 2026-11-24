# fe.dll and sim1.dll magnetic-variation boundary

Live verified in the FSX 10.0.61637.0 binaries. `sim1.dll`'s PE import
table maps IAT `0x20bb12cc` to `fe.dll` ordinal 4. Ghidra shows a read
of that slot at `sim1.dll:0x20bcf294` in `FUN_20bcf230`. The function
converts its position source at `+0x108` and `+0x120` from radians to
degrees, calls the FE lookup and stores a float result in radians at
its cache object's `+8`. It refreshes after its timer expires or on a
zero-step call and reloads the timer to 30.0. The original FE export
table resolves ordinal 4 to `fe.dll:0x203efc50`, which forwards to the
grid sampler at `0x203ec9a0`.

The numeric operations on this boundary sample an angle for the current
position. The flight-step call chain in `sim1.dll` supplies the separate
acceleration, velocity and position/attitude stages.

- [Cross-module import, export and Ghidra evidence](../../../artifacts/shared/fe-sim1-magnetic-variation.json)
- [FE grid sampling](../fe.dll/fe-magnetic-variation.md)
- [Sim1 state integration](../sim1.dll/sim1-state-integration.md)
