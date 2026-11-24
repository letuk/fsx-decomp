# G2D Direct3D 10 shader creation

2026-11-14. Live verified in g2d.dll. All three 60-byte bodies read from
Ghidra match the pinned PE SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.
Separate reads verified shader table slot +4. The device offsets were
checked against the x86 slot order in Windows SDK 10.0.26100.0 d3d10.h.

| Shader table | Slot +4 target | ID3D10Device slot |
|---|---|---|
| 2066f790 | 206800b0 | +0x13c CreateVertexShader |
| 2066f7c0 | 20680100 | +0x140 CreateGeometryShader |
| 2066f7f0 | 20680150 | +0x148 CreatePixelShader |

The target takes the shader receiver in ECX and reads the bytecode blob
at +0x44 and device at +0x48. It calls the blob's size and pointer slots
in that order. Each device call receives the bytecode pointer, its size
and the address of shader field +0x70 as the output. The wrappers return
-1 for a negative HRESULT and zero otherwise. The SDK's slot +0x144 is
CreateGeometryShaderWithStreamOutput and is skipped. The [ID3D10Device reference](https://learn.microsoft.com/en-us/windows/win32/api/d3d10/nn-d3d10-id3d10device)
documents the public methods.

The PostProcess catalogue uses one source helper and its existing
validated stage index to choose the device slot. The original uses
three separate shader virtual methods. The source keeps the existing
publication and failure cleanup order. VC8 x86 run
g2d-shader-creation-20261114-001 passed without warnings. Source hashes
matched the workspace. The COFF listing shows the catalogue call,
the two blob calls and the indirect device call with the output at +0x70.
Object SHA256 is 9986FB285AD06822381505B788D954FDB818AD84B2B61FAEDE674CE50CD3A383.
Generated code differs in dispatch and frame layout. DLL linking and
runtime verification remain open.

Suggested Ghidra names, unapplied. At 206800b0, `piVar1` to
`bytecodeBlob` follows the size and pointer slots, and `piVar2`
to `device` follows the verified device vtable. Ghidra made no
database edits.

Next package is shader object lifetime and owned storage, starting
at constructor 2067fa30, base destructor 2067f710 and define copying
20680210. The vertex deleting destructor at 206d6230 provides the
handoff by releasing the interface at +0x70 and then calling 2067f710.
The reflected-binding collection insertion at 20680710 remains open.
