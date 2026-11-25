# Turbine shaft torque

## Original behavior

The `sim1.dll` routine `FUN_20c2c850` publishes shaft torque for engine types
3 and 5. The original caller at `0x20c2b874` supplies the engine record in
EBX and pushes the flight-dynamics pointer, turbine record and step interval.
The original pops 16 bytes on return. All names below are reconstructions.
The preceding [spool update](sim1-turbine-spool-speed.md) supplies the
corrected speed used to query the drive torque curve.

| Original address | Finding | Status |
| --- | --- | --- |
| `0x20c2b827`–`0x20c2b874`, `0x20c2c825` | The previous update writes turbine state `+0x10`. The caller sets turbine state `+0x08` to the engine speed fraction `+0x5c` times 100 before invoking the torque update. The precise spool label of `+0x10` remains open. | live verified |
| `0x20c2c861`–`0x20c2c8a9` | When engine `+0x08` is nonzero, evaluate the AIR `0x1508` curve at turbine `+0x10`. Multiply by maximum torque and by `1 - 0.08 * engine.float[+0x28]`. Store the resulting drive torque as binary64 and select response rate 12. The provenance of the reduction input remains open. | live verified |
| `0x20c2c8ab`–`0x20c2c8c8` | With engine `+0x08` zero, set drive torque to zero and select `4 * previous normalized torque + 0.1` as the response rate. The rate is stored as binary64 at the branch merge. | live verified |
| `0x20c2c8cb`–`0x20c2c93a` | With history `+0x488` clear, multiply drive torque by the AIR `0x1548` curve evaluated at environment air density `+0x134`. Round the product to binary64. Subtract maximum torque times the AIR `0x509` curve evaluated at turbine `+0x08`. With history `+0x488` set, the target is zero. If engine `+0x04` is present, dispatch to its VTable slot `+0x48` before publishing the result. | live verified |
| `0x20c21e28`–`0x20c21e88`, `0x20c2c8e0`–`0x20c2c8f2` | The initializer stores the result of a factory call for resource `0xd04c` in engine `+0x04`. The torque caller passes the receiver on the stack, a binary32 zero and a zero dword. It observes callee cleanup and ignores the return value. The concrete target, return type and third argument type remain unresolved. The source models the observed call via a minimum dispatch table. | live verified |
| `0x20c2c93d`–`0x20c2c972` | For ordered shaft speed strictly above 5, update as `old + (target - old) * responseRate * step`. Otherwise store the target directly. Write engine `+0x44`, reload the rounded binary64 value, divide by the configured maximum and publish turbine state `+0x28`. | live verified |
| `0x20c26112`, `0x20c26168`–`0x20c261c8`, `0x20c26a3b`–`0x20c26a4b`, `0x20c26b54`–`0x20c26b5f` | The configuration readers bind `maximum_torque` to `+0x2d8` and apply `power_scalar` to it. AIR `0x509` fills the inline 0x44-byte curve at `+0x620`. The external allocation at `+0x19e4` holds AIR `0x1508` at `+0x1a44` and AIR `0x1548` at `+0x1a78`, with room for three and ten pairs respectively. | live verified |
| `0x20bcbf7f`–`0x20bcbf9b` | In the shared curve evaluator an unordered query takes the same first-sample branch as a query below the first breakpoint. The source previously evaluated unordered input in the search. That behavior is superseded for the reconstructed lookup. | live verified |

The current Ghidra prototype has only the three stack arguments. Suggested
argument names are `param_1` → `airplane` based on the history and environment
loads, `param_2` → `turbine` based on the loop stride and normalized torque
store, and `param_3` → `step` based on the caller interval and response
calculation. The decompiler's `unaff_EBX` is the implicit `engine` input.
Ghidra names and signatures were left as read back.

## Reconstruction and verification

`EngineForceRecord::UpdateTurbineShaftTorque` lives in
[`src/sim1/turbine.cpp`](../../../src/sim1/turbine.cpp).
The minimum turbine layouts are in [`turbine.h`](../../../src/sim1/turbine.h).
The propeller step and feathering logic continue to consume the shared
engine and turbine records. The turbine code calls the existing curve
evaluator. The source member uses the x86 C++ receiver convention.

| Original range, inclusive | Native VC8 precise comparison |
| --- | --- |
| `0x20c2c894`–`0x20c2c8a8` | 21 normalized bytes for the drive reduction, binary64 store and response-rate load. The two binary64 literal targets were verified individually. |
| `0x20c2c8b6`–`0x20c2c8c1` | 12 normalized bytes for the nonrunning response rate, with both literal targets verified. |
| `0x20c2c949`–`0x20c2c94f` | 7 exact bytes for the ordered comparison status and branch. |
| `0x20c2c953`–`0x20c2c958` | 6 exact bytes for the response rate and time step multiplications. |

The complete bodies differ at 291 original and 288 rebuilt bytes. The
comparison verifies all three direct calls against the original
`0x20bcbf60` target and the rebuilt curve symbol. Receiver and curve
arguments use different registers. The prologue, curve dispatch setup,
auxiliary dispatch scheduling and final x87 instruction layout differ.
The first precise build left the unrounded torque on the x87 stack for
normalization. A volatile read of the stored value restored the original
binary64 rounding point in the retained precise build.

Eight host x86 cases passed, covering interpolated input, the speed boundary,
unordered input, the nonrunning response, auxiliary dispatch and the zero
divisor. The original airplane configuration offsets `+0x19c4` and
`+0x19fc` are preserved in the expanded source layout. The final native
VC8 propeller and aerodynamics regression suites and the linked propulsion
comparison passed. The dashboard shows the original and rebuilt listings and
the four scoped comparisons. Simulator runtime and a complete linked
turbine-loop comparison remain open. Ghidra was read back clean at revision
`1795634906543:62a95bf0:520`.

[Live Ghidra evidence](../../../artifacts/sim1.dll/turbine-shaft-torque-ghidra.json) ·
[Retained VC8 comparison](../../../artifacts/sim1.dll/turbine-shaft-torque-vc8/turbine-shaft-torque-comparison.json) ·
[Fast-profile control](../../../artifacts/sim1.dll/turbine-shaft-torque-vc8-fast/turbine-shaft-torque-comparison.json) ·
[Host x86 results](../../../artifacts/sim1.dll/turbine-shaft-torque-host-check/results.txt)

[Native propeller regression](../../../artifacts/sim1.dll/sim1-turbine-shaft-torque-propeller-regression-20261125-002.log) ·
[Native aerodynamics regression](../../../artifacts/sim1.dll/sim1-turbine-shaft-torque-aero-regression-20261125-002.log) ·
[Linked propulsion regression](../../../artifacts/sim1.dll/sim1-turbine-shaft-torque-linked-regression-20261125-001.log)
