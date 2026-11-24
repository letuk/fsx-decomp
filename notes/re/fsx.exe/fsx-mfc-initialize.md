# fsx.exe MFC module initialization

Live verified at `fsx.exe 0x01002c0d..0x01002c32`. The previous
`FUN_01002c0d` Ghidra name and undefined prototype are superseded by
`BOOL __stdcall AfxInitialize(BOOL isDll, DWORD mfcVersion)`.
The identity comes from the previously matched `mfcs80:appmodul.obj`
symbol `?AfxInitialize@@YGHHK@Z`. The original reads two stack arguments,
pops eight argument bytes at return, and returns TRUE.

The first parameter is stored as a byte in the current
`AFX_MODULE_STATE::m_bDLL` at offset `0x14`. The full 32-bit argument
controls the call to `_setmbcp(_MB_CP_ANSI)` when zero. The second
parameter is stored in `AFX_MODULE_STATE::m_dwVersion` at offset `0x44`.
Both offsets were checked at compile time using the VC8 MFC headers.
The caller at `0x01002d16` supplies zero and `0x800`.

`InitializeMfcModuleState` is the reconstructed source name. The ordinary
startup link continues to use the SDK entry. The optional probe compiles
the reconstruction with VC8 RTM `/O1 /Oy /GL-` and verifies its linked body.
With `/OPT:NOICF`, the source entry at `0x01002303` and the SDK entry at
`0x01002c2d` have distinct addresses. All 38 source-body bytes match
after validating the relative call through `mfc80.dll!#1084` and the IAT
operand for `msvcr80.dll!_setmbcp`. With ICF enabled the linker folds the
two bodies. Both runs pass the controller regression gates and the complete
`InitInstance` comparison. This is a linked-body result. COFF and runtime
verification remain open.
[Source](../../../src/fsx/afx_initialize.cpp),
[comparison](../../../artifacts/fsx.exe/afx-initialize-20261118/match.json),
and [artifact hashes](../../../artifacts/fsx.exe/afx-initialize-20261118/evidence-hashes.json).

Ghidra readback verified the name, prototype and plate comment. The
database is saved at revision `1795011990050:2514814d:49`. Suggested
parameter names `param_1` to `isDll` and `param_2` to `mfcVersion`
were applied with the prototype. The stored module-state datatype was
left untouched.
