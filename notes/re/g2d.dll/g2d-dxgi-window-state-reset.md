# G2D DX10 window shared-state reset

2026-11-15. Live verified in g2d.dll. Three Ghidra listings covering
358 bytes match the pinned original PE SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.

| Address | Evidence and behavior |
|---|---|
| 2067e9c0 | Walks eight pointer slots starting at holder +0x84. Clears each populated slot before resetting the entry. Clears entry +8 and byte +5, calls the entry destructor and frees through api.dll ordinal 108. Detaches and releases holder +0x58, the inline constant's GPU pointer. Clears holder +0x48, the three borrowed shaders at +0x10/+0x14/+0x18, the window back-pointer at +8 and initialized byte +0x0c. |
| 2067f3f0 | ESI supplies the entry. If byte +4 is set, mask bits 0 through 3 at +0 select owned interface slots +0x288 through +0x294. Clears each selected slot before COM Release, then clears byte +4. |
| 2067f260 | EAX supplies the entry. If byte +5 is set, clears it and entry +8. Releases remaining nonnull COM interfaces in descending slot order from +0x294 through +0x288. |
| 2067ea50 | A separate consumer reads the holder's indexed entry at 2067eb11 and calls 2067f500 at 2067eb18. This is the next scope for tracing how the holder applies the quad state. |

The pilot wires the three reset helpers into the previous holder
destructor. The source uses loops for the original unrolled COM
release sequences and keeps the release order and pointer clearing.
The later inline-constant destructor sees a null GPU pointer after
the initialized-holder reset.

Suggested Ghidra names, unapplied. At 2067e9c0, `param_1` to
`windowSharedState` follows the holder fields, `iVar1` to
`resourceEntry` follows the entry reset and free, and `local_8` to
`remainingEntries` follows the fixed eight-slot loop. Ghidra reads
made no database edits.

VC8 x86 run g2d-window-state-reset-20261115-001 passed without warnings.
Compiled-source hashes match the workspace. The listing contains all
three counterparts and the holder destructor's reset call relocation.
Object SHA256 is EBFC341B9D827019984BB6EBDACED48E64F2932E46CC4C15AD16D829D013810C.
The dashboard retains both listings for each Compiled entry. Byte
comparison of generated code, DLL linking and runtime tests remain open.
