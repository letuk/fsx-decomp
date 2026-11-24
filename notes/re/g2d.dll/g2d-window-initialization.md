# G2D per-window initialization

2026-11-12. Live verified against g2d.dll SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.
Read-only Ghidra pseudo-disassembly matched 2,814 PE bytes across
the two initialization bodies and the +0x9c deletion wrappers.
The two window table cells were verified in the preceding package.

| g2d.dll address | Verified behavior |
|---|---|
| 20685a60–20685bd5 | D3D9 calls window slot +0x8c with zero, reads the D3D interface and adapter display mode, installs mode text 1024x720x32 and gets the adapter monitor. The two adapter queries populate +0x290 and +0x294. A zero result at +0x294 triggers up to 25 retries, each repeating desktop and legacy preparation under API ordinal 216. |
| 20685bdb–20685dc9 | Reads adapter identifier and capabilities into +0x29c and +0x160. It can divide the adapter-memory value by an additional capabilities field. Driver version is queried only when both cached version DWORDs are zero. After mode enumeration it requires at least one 0x20-byte mode record. |
| 20685dce–20685f7b | Tests capabilities and the adapter kind. Selects shader level 0x27, 0x1d or 0x13. Unsupported shader versions store 9 before failing. The adapter rejection path can report an unsupported adapter through 20699890. On success a memory override can replace +0x294, ordinal 228 is called for adapter zero below 0xf00000, and the byte at +0xc9 requires the capability bit, 24 MiB, and both limits at least 2048. Success returns zero. Common failure calls +0x8c with 1, clears +0x98 and returns -1. |
| 2068fc80–2068febf | DXGI calls +0x8c with zero and runs three prerequisites at 20690d40, 20690f00 and 20691100. Their nonzero results propagate in EAX. A failed adapter/output descriptor query returns -1. A successful output query stores its monitor at +0x98. The adapter label is written before mode enumeration at 206908d0, whose nonzero result also propagates. A successful enumeration with no modes resets through +0x8c with 1, clears the monitor and returns -1. |
| 2068fec4–20690212 | With modes available DXGI sets +0xc9 and scores candidates. It applies the selected values through 206978f0 with bit zero of the flags cleared, calls 20690220, 20691030 and 20691d30, then returns zero. |
| 20685920, 2068f8c0 | Both +0x9c targets invoke their backend destructor and use API ordinal 108 to free the storage if argument bit zero is set. The D3D9 destructor is 20685950 and the DXGI destructor is 2068fa90. |

The retained Ghidra decompilation reports void for both initialization
targets. The original EAX epilogues and the controller's result checks
supersede that interpretation.

src/g2d-pilot/window_initialization.h connects both backend loops to
the reconstructed status, capability and initialization flow. The
source uses explicit interfaces for device and swap-chain prerequisites,
driver queries, mode enumeration and DXGI scoring. The D3D9 source
omits the temporary error-text copies on the identifier-failure path
because that path neither publishes nor displays the text before
returning. The DXGI destructor and the D3D9 destructor remain boundaries.

VC8 x86 run g2d-window-initialization-20261112-001 compiled the G2D
unit with /EHsc and no compiler warnings. Worker source hashes matched
the workspace and COFF relocations connect both controller callers.
COFF SHA256 is
9BAF338B8AB6B293CB9DAAB598E1527174156C5179B303B66CB4189E0327413E.
Byte matching, final DLL linking and runtime verification remain open.
Ghidra stayed read-only and reported changed=false.

Suggested Ghidra names, unapplied. At 20685a60, param_1 to window
follows the ECX receiver and its window table. The stack counter at
20685b74 can be named retriesRemaining. In 2068fc80, param_1 to
window follows the adapter and output field accesses. The status
temporary at 2068fe5c can be named modeEnumerationStatus once its
live range is separated from the reused ESI values.

Next package is the DXGI device and output prerequisites at 20690d40,
20690f00 and 20691100. Mode scoring and destructor internals remain
distinct follow-on boundaries.

The three DXGI prerequisites are now connected to their source
bodies. See g2d-dxgi-window-device.md for the resource boundaries.
