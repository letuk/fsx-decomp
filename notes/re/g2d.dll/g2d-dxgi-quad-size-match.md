# G2D DX10 quad constant byte-width match

2026-11-15. Live verified in g2d.dll. Table 2066f51c slot zero
targets 2067e570. A read-only Ghidra pseudo-disassembly covers the
full 16-byte leaf through RET and matches the pinned original PE.
The leaf reads the payload size from ECX +0x2c and rounds it up
to a multiple of 16. It returns the width in EAX.

The first VC8 x86 run, g2d-quad-size-20261115-001, emits 20 bytes
from C++ with frame-pointer omission. Its conditional and adjustment
sequence differ. Run g2d-quad-size-20261115-002 uses VC8 inline x86
assembly and matches all 16 original bytes. The unique COFF symbol
starts at offset zero of an executable COMDAT with zero relocations.
The match uses no normalization, byte masks or link-time fixups.
Both runs passed without compiler warnings and retain source hashes.

The source buffer initializer uses this getter for the width.
The COFF relocation confirms that call. Original VTable binding,
DLL linking and runtime verification remain open. Ghidra remained
read-only. The pseudo-disassembly has no named locals to rename.

See [comparison](../../../artifacts/g2d.dll/g2d-quad-size-20261115-002/comparison.json)
for both complete bodies and object hashes. The dashboard lists
the accepted body as Exact and retains both instruction listings.
