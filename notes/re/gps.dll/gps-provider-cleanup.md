# GPS provider and legacy storage cleanup

2026-11-15. Live verified in gps.dll against the pinned PE from
[GPS provider registration](gps-provider-registration.md).

| Address | Evidence and behavior |
|---|---|
| 208560e0 | Provider destructor. Restores the table at 208519e4, calls 208561b0 with the provider in ECX, and releases the buffer using the embedded storage at provider +8. The function takes one stack argument and uses RET 4. |
| 208561b0 | The provider table pointer at 20851a04 targets this method. The loop reads the signed count at provider +8 on each iteration, reads a pointer from the array at +0x14 and gives a local copy by address to api.dll ordinal 108. The method clears the count on both exit branches. |
| 20856500 | Buffer cleanup with the storage address in ESI. Requires a nonnull pointer at storage +0xc, bit 0 at +0xa and nonzero storage +4. Clamps the signed count to at most zero, passes a local copy of the buffer pointer to api.dll ordinal 108, sets flag bit 0 and clears storage +0xc and +4. |
| 2086a4c0 | Legacy cleanup uses the same 16-byte field arrangement and buffer cleanup. It always frees the outer storage object and returns its original pointer value in EAX. The lifecycle callback loads the object into ESI before calling this function. |
| 2086b820 | The unwind action loads the provider from the destructor's argument slot, adds eight and jumps to 20856500. |
| 2086b82b | The destructor establishes this exception handler. It checks the frame cookie and references the function info at 2086be00, whose one-state unwind map at 2086bdf8 points to 2086b820. |

`GpsOwnedStorage` is a reconstructed minimum layout with a signed
count, a 32-bit allocation field at +4, two 16-bit fields and an array
pointer. The source name `capacity` for +4 is provisional. Cleanup
only proves its zero test and reset. This supersedes the earlier
description of +8 and +0x14 in the provider as fields with only
known widths and initial values.

The pilot connects `Release` to the reconstructed destructor and
entry loop. A local RAII guard runs the shared buffer cleanup on
the normal path and during C++ unwinding. The reconstructed
`ReleaseGpsLegacyStorage` entry uses a 12-byte ABI adapter to push
ESI for the C++ body and copy the original pointer value back to
EAX. This run left the lifecycle callback unresolved. The later
[lifecycle reconstruction](gps-legacy-lifecycle.md) connects it.

## Build evidence

Run `gps-provider-cleanup-20261115-004` completed without warnings
with VC8 x86 14.00.50727.762. The source snapshot, hashes, build
command, receipt and listings are retained under the run directory.
Object SHA256 is
`435A962049969567C7F249DCC29CD3342E3C7430F5A46B4A24A0537B33888817`.

- Entry release matches all 78 bytes after validating the single
  relocation to api.dll ordinal 108.
- The destructor is 148 original bytes and 157 rebuilt bytes.
  Five body relocations were checked. The rebuilt guard changes
  frame layout and the base register for storage. VC8 emits a
  branchless count clamp. The diagnostic follows both one-state
  unwind maps through their respective buffer release paths.
- Legacy cleanup compares the original 86-byte body with the
  95-byte C++ body. The checker verifies the import relocation
  and the separate 12-byte ESI adapter's bytes and call target.
- The ordinal 2 normalized match and the ordinal 1 and Release
  diagnostics passed again.

Trial 001 used /EHsc and emitted no unwind action for the guard.
Trial 002 used /EHs and emitted the unwind action with an
out-of-line guard destructor. Trial 003 used a pointer guard
and an explicitly inlined destructor, removed the guard warnings,
and inlined buffer cleanup on the normal path. Trial 004 replayed
that source with the complete comparator.
See the [comparison report](../../../vm-share/modules/gps/runs/gps-provider-cleanup-20261115-004/cleanup-comparison.json).

Suggested Ghidra names, unapplied. At 208560e0, `param_1` to
`provider` follows the inbound stack argument before its stack
slot is reused for the buffer pointer. At 2086a4c0, `unaff_ESI` to
`storage` follows the field accesses and final deallocation.
Ghidra was read-only and the prior save state remains unverified.
DLL linking and runtime verification remain open.
