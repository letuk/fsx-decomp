# fe.dll magnetic variation lookup

The `Facilities` string at `0x20341868` leads to index I/O in
`FUN_203e0290`, `FUN_203e1380` and `FUN_203e1f80`. The
`SceneryIndexes` and `SceneryCache` strings have code references in this
module. The relevant numeric path uses the latitude and longitude of the
simulated aircraft to sample a magnetic-variation grid.

| Original VA | Finding and evidence status |
| --- | --- |
| `0x203efc30` | Live verified ordinal 3 and fixed-coordinate adapter. It calls `0x203ec5d0` with the position pointer in EAX and returns with `ret 4`. |
| `0x203efc50` | Live verified ordinal 4 and degree-coordinate adapter. It calls `0x203ec9a0` with the position pointer in EDX and returns with `ret 4`. The numeric return is in x87 ST0. |
| `0x203ec5d0` | Live verified fixed-coordinate conversion. Reads the signed latitude high word at `+4` and the unsigned longitude high word at `+0xc`, converts to degrees and returns the low 32 bits of the truncated scaled angle. |
| `0x203ec9a0` | Live verified grid sample. Returns zero for an absent grid, adjusts latitude and longitude, selects two adjacent columns and uses three calls to the angle interpolation helper. The return is rounded to float. |
| `0x203ecb90` | Live verified wrapped linear angle interpolation. Receives endpoints through ECX and EDX and the fraction pointer through ESI. |
| `0x203ec650` | Live verified grid loader and writer for the latitude count at `0x20449a1c`, longitude count at `0x20449a20` and sample pointer at `0x20449a28`. Its 16-bit source samples are converted with the scale `360/65536`. |

The C++ names, fixed-position prefix and grid field names are
reconstructions. The build defines loader-owned storage in a separate
translation unit. Loading the original scenery grid and running the
DLL in FSX are outside the verification performed here.

| Scope | Original bytes | Selected rebuilt bytes | Result |
| --- | ---: | ---: | --- |
| `0x203efc30` fixed-coordinate adapter | 17 | 17 | Normalized, with the direct call target individually verified. |
| `0x203efc50` degree-coordinate adapter | 17 | 17 | Different. The pointer is forwarded in ECX in the rebuilt code, versus EDX in the original. |
| `0x203ec5d0` fixed conversion | 108 | 82 | Different. Call and data placement, x87 scheduling and conversion differ. |
| `0x203ec9a0` grid sampling | 485 | 437 | Different. Register allocation, division and x87 stack scheduling differ. |
| `0x203ecb90` angle interpolation | 154 | 158 | Different. The register-passed pointers agree in the fast build. Comparison and x87 stack ordering differ. |

Run 001 retained a link failure from missing `kernel32.lib`. The retry
used VC8 RTM `14.00.50727.42` and produced a no-entry PE32 DLL, with both
original ordinals verified from the original export table. Run 003 with
precise floating point preserved the normalized adapter and produced a
439-byte grid sampler. Run 002 is the selected fast build. The original
import caller in `sim1.dll` is recorded in the cross-module topic.

The live database already had unsaved changes before tagging. The tags
were read back in the live database and retained in a post-tag GZF.
The live database remains unsaved at `1795525213681:25d6d7eb:135994`.
Suggested local names in `FUN_203ec9a0` are `param_2` → `positionDegrees`
and `local_10` → `longitudeFraction` after its in-place subtraction.
They are provisional suggestions and were not applied in Ghidra.

- [Original Ghidra inventory](../../../artifacts/fe.dll/fe-inventory/manifest.json)
- [Loader references and tag readback](../../../artifacts/fe.dll/magnetic-variation-ghidra.json)
- [Linked comparison](../../../vm-share/modules/fe/runs/fe-magnetic-variation-20261124-002/magnetic-variation-comparison.json)
- [Precise floating-point control](../../../vm-share/modules/fe/runs/fe-magnetic-variation-20261124-003/magnetic-variation-comparison.json)
- [sim1.dll to fe.dll call](../shared/fe-sim1-magnetic-variation.md)
