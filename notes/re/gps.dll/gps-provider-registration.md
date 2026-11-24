# GPS provider registration

2026-11-15. Live verified in gps.dll. The pinned PE32 image has base
`20850000` and SHA256
`E36BE481BF8A6A8A0A56EA72D57961B169050F70DE401766C17FB374EE74E609`.
The PE export address table has two entries with ordinal base 1.
The original Ghidra listings and raw bytes are retained in the
[partial inventory](../../../artifacts/gps.dll/gps-inventory/manifest.json).

| Address | Evidence and behavior |
|---|---|
| 20855f20 | Ordinal 1. Calls api.dll ordinal 106 with arguments 1, 0x18, an out pointer and the value at 2086eab0. If the output is nonnull, initializes the object, calls panels.dll ordinal 72 with FS9GPS and the object, and invokes the object table slot at +4 with the object in ECX. Calls api.dll ordinal 231 with (0x10, 2086a380) then (2, 2086a520). Saves the result of multiplayer.dll ordinal 4 to 2086eaa8. |
| 20855fb0 | Ordinal 2. Calls panels.dll ordinal 72 with FS9GPS and a null pointer, then calls api.dll ordinal 232 with (2, 2086a520) and (0x10, 2086a380). The hotpatch prologue, balanced stack and plain RET support the reconstructed no-argument cdecl signature. Both imported calls consume two stack arguments without caller adjustment. |

The callback addresses recur in the registration and unregistration
exports. At `2086a380`, the branches for values 0x36 and 0x37 register
and remove the strings `FS2002 GPS DATA` and `FS2002 GPS DATA REQUEST`
with imported functions. At `2086a520`, the callback reads flight plan
state and copies selected fields and waypoint records to module storage.
These are live observations of the branch and data flow. Callback
signatures and event meanings remain open. `GpsLegacyDataLifecycle`,
`GpsLegacyDataUpdate` and `UnregisterGpsProvider` are reconstructed
names. The two callbacks are modeled as opaque address symbols in COFF.

Suggested Ghidra name, unapplied. At `20855f20`, `local_8` could become
`gpsProvider` because the output object is passed to panels.dll for
`FS9GPS` and supplies the subsequent virtual dispatch. The later
[lifetime analysis](gps-provider-lifetime.md) verifies the minimum
layout and the table target at +4. Ordinal 2 has no
decompiled local variables to rename. Ghidra was read-only and the
prior database save state is unverified.

## Verification

The GPS controller built one translation unit with VC8 x86
14.00.50727.762 through the shared XP worker. Run
`gps-registration-20261115-002` has a successful receipt, a warning-free
log, the source snapshot and build command with hashes, and both COFF
artifacts. Object SHA256
`B075A06D618D063DE91D1F6A86D74FD19570035D701C70030652E757029DA2E0`.
The complete 42-byte body matches after validating five DIR32 fields
against the original string, two PE imports and both callback
addresses. The comparator also verifies those callback addresses in
ordinal 1. See the [comparison report](../../../vm-share/modules/gps/runs/gps-registration-20261115-002/comparison.json).

Run `gps-registration-20261115-001` emitted the COFF before the
comparator was installed. Comparing that preserved object afterward
also passed. Run 002 verified the complete controller path.
DLL linking and runtime verification remain open. The next source
boundary identified in this run was ordinal 1, now covered in
[GPS provider lifetime](gps-provider-lifetime.md).
