# G2D DX10 depth-resource allocation

2026-11-15. Live verified in g2d.dll. The complete 409-byte original
body at 206d9270 matches the pinned PE SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.

| Address | Evidence and behavior |
|---|---|
| 206d9270, 206d92c9 | Clears the output and calls 206d87a0 with the description key, device at window +0x104, and values from +0x1a8/+0x1ac. The factory returns a counted resource. |
| 206d9310–206d9364 | Publishes the resource and sets reservation byte +0x14. Inserts the description key and a counted resource reference into the first pool index at holder +0x0c via 206d9ad0. |
| 206d93b5–206d93f3 | Inserts a second reference under resource ID +8 in holder index +0x18 through 206dbe30, then releases the factory reference. |

The pilot connects the allocation path to the existing pool lookup.
It publishes only after both insertions succeed. Its source interfaces
require each successful insertion to retain one reference and the
first insertion to return its node so the pilot can roll it back
through 206d95f0 if the second fails. This rollback is a source
decision. Texture creation and tree operations remain explicit boundaries.

Suggested Ghidra names, unapplied. At 206d9270, `param_1` to
`resourceHolder` follows the window and index fields. `param_2`
to `resourceOut` follows the initial clear and final pointer store.
Ghidra remained read-only.

VC8 x86 run g2d-depth-allocate-20261115-001 passed without warnings.
Compiled-source hashes match the workspace. Object SHA256 is
07460B181FBE20ED7F09C6F4AC42EE54DE021EA1E1557B258FF6F374486D517C.
The [comparison](../../../artifacts/g2d.dll/g2d-depth-allocate-20261115-001/comparison.json)
compares the complete 409-byte original and 345-byte COFF body.
It validates three factory/index call mappings, the incoming pool
call and the existing 16-byte exact getter. The original iterator,
reference and exception handling differs at instruction level. The
dashboard status is Diagnostic. DLL linking and runtime remain open.

Next is the depth texture factory at 206d87a0, starting with
construction 206d8830 and texture initialization 206d89f0.
