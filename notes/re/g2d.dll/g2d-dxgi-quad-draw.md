# G2D DX10 quad drawing

2026-11-15. Live verified in g2d.dll. Three original listings covering
579 bytes match the pinned PE SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.
Public COM slots were checked against Windows SDK 10.0.26100.0 d3d10.h.

| Address | Evidence and behavior |
|---|---|
| 2067ea50 | Reads the D3D10 device through holder +8 and window +0x104. Propagates failure from state preparation at 2067eb50. Clears the input layout, sets point-list topology and calls slot +8 on the three catalogue shaders. Copies four DWORDs and a float into the inline constant payload. Propagates upload failure before drawing. On success dispatches the constant through the geometry shader's slot +0x14, +0x18 or +0x1c for selectors 0, 1 or 2. Calls Draw(1, 0), clears selected pipeline state and invokes shader slot +0x0c in vertex, geometry and pixel order. |
| 2068d500 | Compares the current and previous CPU payloads bytewise. Copies changed data to the previous payload before calling ID3D10Buffer::Map with WRITE_DISCARD and flags zero. On Map failure returns -1 without Unmap. On success copies the payload to mapped memory and calls Unmap. Identical data returns zero without Map. |
| 2067f500 | Bits 0 through 3 select null-state calls in order to RSSetState, OMSetDepthStencilState, OMSetBlendState and IASetInputLayout. The blend reset passes a null factor pointer and mask 0xffffffff. |
| 206db4b0 | Live cross-reference from 206db390 to 2067ea50. This higher-level caller still needs source integration. |

The pilot exposes a normal C++ draw interface in place of the original
mixed register/stack convention, uses library byte comparison and
copies, and groups the repeated shader slot calls into loops. It
preserves the original snapshot update before Map and the early error
returns. State preparation at 2067eb50 was subsequently connected
in the [quad render-state package](g2d-dxgi-quad-draw-state.md).

Suggested Ghidra names, unapplied. At 2067ea50, `in_EAX` to
`windowSharedState` follows the holder fields, `piVar1` to
`d3d10Device` follows the checked SDK slots, and `param_1` to
`stateEntryIndex` follows the eight-entry table access. Ghidra reads
made no database edits.

VC8 x86 run g2d-quad-draw-20261115-001 passed without warnings.
Compiled-source hashes match the workspace. The COFF listings contain
all three counterparts. The draw routine relocates to state preparation
and the local upload function.
Object SHA256 is DDB4C91E005F23BEC544232B184AFCEDCFA83EC81BF9D4675B551A14D3B85F29.
The dashboard retains both listings for each Compiled entry. Generated
byte comparison, DLL linking and runtime tests remain open.

Next package is state entry preparation and caching at 2067eb50.
The observed call sites lead to construction and initialization at
2067f1e0 and 2067f2d0 and state creation and application at 2067f360
and 2067f480.
