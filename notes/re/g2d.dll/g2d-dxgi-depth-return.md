# G2D DX10 depth resource return

2026-11-15. Live verified in g2d.dll. The 121-byte body at 206d9190
and 119-byte body at 206db310 match the pinned PE SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.

| Address | Evidence and behavior |
|---|---|
| 206d9190 | Looks up the resource ID in the secondary tree at holder +0x18 via 206d9950. On a miss returns -1. On success clears reservation byte +0x14, calls main.dll ordinal 61 through IAT 2066158c and saves its result at resource +0x28. The exact meaning of that value remains open. |
| 206db310 | Rejects an inactive target. When the compositor flags are set, restores the device views and calls 2067a020, propagating failure. Otherwise or after success, returns the depth resource by its ID, clears target +0x18 and resets bytes +0 and +1. The original ignores the pool return status. |

The pilot shares direct tree lookup between the target registry and the
return path. The original uses checked iterators. The finish boundary
and original window callers still need source integration.

Suggested Ghidra names, unapplied. At 206d9190, `in_EAX` to
`resourceHolder` follows the secondary-tree access, and `local_c`
to `resourceNode` follows the sentinel checks and resource load.
Ghidra remained read-only.

VC8 x86 run g2d-depth-return-20261115-001 passed without warnings and
compiled-source hashes match. Object SHA256 is
0A970D6405114C5492BD9CE68DF4262B07250CBC69710607D3E07232078CD1A4.
The [comparison](../../../artifacts/g2d.dll/g2d-depth-return-20261115-001/comparison.json) maps four
call targets to validated COFF relocations. It also records both full
bodies. Sizes are 121/40 and 119/126 bytes. Frame layout, iterator
checks and call encodings differ beyond address operands, so both
dashboard entries are Diagnostic. The previous 16-byte width getter
still matches in the same object. DLL linking and runtime remain open.

Next package is depth resource allocation at 206d9270 and insertion
into the pool's two indexes.
