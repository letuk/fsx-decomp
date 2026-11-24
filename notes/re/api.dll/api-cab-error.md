# API cabinet FDI error record

## ErfSetCodes at 0x2014a51b

Live verified in `api.dll`. Ghidra identifies the 30-byte body as
`_ErfSetCodes@12`. The three stack arguments and `RET 0xc` support
`void __stdcall ErfSetCodes(PERF error, int operation, int type)` as a
reconstructed signature. The body copies the second and third arguments
to record offsets 0 and 4 and writes the value 1 at offset 8. The VC8
Platform SDK's `fdi.h` provides the corresponding `ERF` members
`erfOper`, `erfType` and `fError`. The live caller inventory includes
`_FDICreate` at `0x201493a1` and `_FDIReadCFFILEEntry@4` at
`0x20149b27`. [Live listing and callers](../../../artifacts/api.dll/cab-error-20261119/ghidra-2014a51b.json).

The historical generated candidate emitted a different body. The typed
C++ control under VC8 RTM `14.00.50727.42` and the factory's default
`/O2` differed at six byte offsets. It loaded the third argument early
using EDX. The XP receipt records success, while the host stopped artifact
collection at its SP1 compiler check. The native object remains in the
retained worker output. [Control comparison](../../../artifacts/api.dll/cab-error-20261119/default-o2-comparison.json).

Live verified exact in the second run. The function-local size optimization
reuses ECX for the third argument after storing the second. The VC8 RTM
native x86 COFF body equals all 30 original PE bytes from `0x2014a51b`
through `0x2014a538`. Both bodies have zero relocations in this range.
The original bytes also agree with the fresh Ghidra export. The host and
XP worker source hashes agree, and all 20 source-pass regression comparators
passed. Linked DLL and runtime verification remain open.
[Exact comparison](../../../artifacts/api.dll/cab-error-20261119/comparison.json).

Ghidra readback reported saved revision `1793741866691:51f17f6c:3`.
This was a read-only inspection. Suggested parameter names for a future
typing pass are `param_1` → `error` for the ERF record pointer,
`param_2` → `operation` for its offset-zero field and
`param_3` → `type` for its offset-four field. The stored Ghidra
prototype still lists zero parameters.

The accepted run is `vm-share/modules/api/runs/api-cab-error-20261119-002`.
It retains the effective guest command, source snapshot and hashes, worker
receipt and log, native object, listing and regression results.

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-cab-error-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-cab-error.cjs vm-share/modules/api/runs/api-cab-error-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-cab-error-UNIQUE-ID
```
