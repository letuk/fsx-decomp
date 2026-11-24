# G2D DX10 clear-target back buffer and depth view

2026-11-15. Live verified in g2d.dll. Three listings cover 735 bytes
checked against the pinned PE SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.

| Address | Evidence and behavior |
|---|---|
| 206db540 | Calls swap-chain slot +0x24 to acquire buffer zero with the ID3D10Texture2D IID at 2066a364. On success calls texture slot +0x30 for its description. Releases the temporary interface and converts a failed buffer acquisition to -1. |
| 206d9060 | Uses the tree at holder +0x0c to look up a 64-bit key. Walks matching entries in order until it finds a resource with byte +0x14 clear. Sets that byte and writes the resource output. Calls allocator 206d9270 when no resource is available. Lower-bound helpers 206d98e0/206da0a0 and iterator advance 206da310 establish key order and traversal. |
| 206d8c40 | Reuses a nonnull tag-2 view when the 20-byte description at resource +0x40 matches. Otherwise copies the descriptor, releases the old view, and calls device slot +0x134 with resource +0x1c as the texture and +0x24 as the output. Success sets tag 2. Failure sets tag zero, clears the descriptor and returns -1. |

Windows SDK d3d10.h confirms ID3D10Texture2D::GetDesc and
ID3D10Device::CreateDepthStencilView at the observed slots.
The source connects all three counterparts to the target bind.
The pool walks pointer links directly in place of checked iterators.
The view comparison uses memcmp and retains the original release
and failure order. Pool allocation remains an explicit source boundary.

Suggested Ghidra names, unapplied. At 206db540, `local_14` to
`backBufferTexture` follows the IID, description call and Release.
At 206d9060, `param_2` to `resourceKey` and `param_3` to
`depthResourceOut` follow the lookup and output store. Ghidra
reads made no database edits.

VC8 x86 run g2d-clear-target-depth-20261115-001 passed without warnings.
Source hashes match the workspace and the object identifies as x86
COFF. Relocations identify all three local calls and the remaining
pool allocation boundary. Object SHA256
F3EAD0A7E2762DA3C2767A3C8D6840855A1E3A5C8EA2B483B24FE87A405D948C.
Original exports, compiled listings and hashes are in artifacts/g2d.dll/g2d-clear-target-depth-20261115-001.
The dashboard retains both listings for each Compiled entry. Generated
byte comparison, DLL linking and runtime verification remain open.

Next package is depth-resource allocation at 206d9270 and the matching
return-to-pool path at 206d9190.
