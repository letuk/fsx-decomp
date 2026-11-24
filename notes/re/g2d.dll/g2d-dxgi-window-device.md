# G2D DXGI window device and output setup

2026-11-12. Live verified against the pinned g2d.dll PE with SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.
Read-only Ghidra pseudo-disassembly matched 965 instruction bytes
across the three entry points and three shared helpers. Separate
import reads confirmed the two Interlocked functions.

| g2d.dll address | Verified behavior |
|---|---|
| 20690d40 | Clears two optional byte outputs. One arrives in EAX and reports a change to device flags at window +0x10c, sourced from 20704ca4. The other is a stack argument and reports selected descriptor changes. Takes a snapshot of the old 15-DWORD descriptor at +0x114, reads API controller slot +0x30 into +0xc4, and uses window byte +0xcd to choose between a mode lookup and desktop dimensions. Lookup failure returns -1. Desktop descriptor failure propagates. On success it publishes the new descriptor at +0x114 and returns zero. |
| 20690e90 | A difference in descriptor words at +8 and +0xc contributes only when both differ. Differences at +0x10, +0x14, +0x1c, +0x20, +0x28, +0x34 or +0x38 each contribute independently. |
| 20690fa6–20691022 | The first output has no peer at +0x108. Sets performance options and creates the device at +0x104 through 20704c78 with adapter +0xfc, two zero arguments, flags OR 1 and version 0x1d. The resolver at 2067f130 identifies the pointer as D3D10CreateDevice. E_FAIL maps to -1 and other negative HRESULTs map to 3. After device creation, calls 206d8d70, 206d74c0 and 2067e620 to populate +0x164, +0x168 and +0x160. The first nonzero helper status propagates. |
| 20690f00–20690fa5 | A secondary output shares the device of its first output. It retains the replacement COM interface before releasing the old one, and skips reference operations for identical device pointers. The three holders are copied in order +0x164, +0x168, +0x160. Any missing result returns -1 at that step. |
| 20692610, 20692660 | The holder assignments increment the source count and decrement the old count, then call the old object's slot zero with 1 when its count reaches zero. Reference count offsets are +4 for the first helper and +0x2c for the second. |
| 20691100 | ESI supplies an eight-word mode destination and the stack supplies the window. Queries output +0x100 through COM slot +0x1c. A negative HRESULT returns -1. Success writes desktop width and height from rectangle differences, sets format 0x1c, clears four words and leaves destination word zero intact. Returns zero. This supersedes the decompiler's void signature. |

The source bodies in src/g2d-pilot/dxgi_window_device.h are called by
the DXGI initializer. They use explicit source signatures for the
original register interfaces. Resource creation, mode lookup at
20697080 and final import binding remain boundaries.

VC8 x86 run g2d-dxgi-window-device-20261112-002 compiled the isolated
G2D unit with /EHsc and no compiler warnings. Source snapshot hashes
match. COFF SHA256 is
4984199A2F6EE5432EC7446DBA0D65D1DD8F83510F26A4A774685FE2CFABC2C7.
Byte matching of the generated functions, DLL linking and runtime
verification remain open. Ghidra was read-only and reported changed=false.

Suggested Ghidra names, unapplied. At 20690d40, param_1 to window and
param_2 to descriptorChanged follow the field accesses and byte store.
The optional EAX output can be named deviceFlagsChanged. At 20691100,
param_1 to window and unaff_ESI to outputMode follow the output COM call
and the descriptor stores.

Next package is device-resource construction at 206d8d70, 206d74c0
and 2067e620. Mode enumeration and scoring remain separate.

All three factory calls now connect to source models. The shader
catalogue and registry cleanup remain boundaries. See g2d-dxgi-window-resources.md.
