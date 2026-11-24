# GPS provider lifetime and legacy callbacks

2026-11-15. Live verified in gps.dll against the pinned PE described
in [GPS provider registration](gps-provider-registration.md).

| Address | Evidence and behavior |
|---|---|
| 20855f20 | Ordinal 1. Writes the tables at 208519b0 and 208519e4 in succession, initializes the 24-byte provider, registers FS9GPS and calls its table slot at +4. Ghidra records a computed call to 20856050 from 20855f7f. |
| 208519b0 | All 13 entries of the initial table point to 2086b002. |
| 208519e4 | The second table has 13 observed entries. The pointer at 208519e8 resolves to 20856050. |
| 20856050 | Receives the object in ECX with no stack arguments. Decrements the 32-bit count at +4 and returns the remaining count in EAX. At zero it calls 208560e0 and passes an address containing the object to api.dll ordinal 108. The comparison uses an unsigned count to represent the original CMP/JNC. |
| 208560e0 | Called with one stack argument and RET 4. Restores the table at 208519e4, calls 208561b0 with the object in ECX, then conditionally disposes of the pointer at object +0x14. The callee remains unresolved in the pilot. |
| 2086a380 | Callback registered in slot 0x10. Compares the first stack argument to 0x36 and 0x37 and returns with RET 8. The 0x36 path prepares a 16-byte state object, clears the request fields and calls 2086a520 with two zero stack arguments. The 0x37 path removes both legacy data names and calls 2086a4c0 with the state pointer in ESI when present. |
| 2086a4c0 | Receives the state pointer in ESI. When pointer +0xc is nonnull, bit 0 at +0xa is set and +4 is nonzero, adjusts the signed value at +0 to at most zero and releases the nested pointer. Always passes the state pointer by address to api.dll ordinal 108. Returns the original ESI value in EAX. |
| 2086a520 | Callback registered in slot 2. Returns with RET 8 and has a stack cookie and exception frame. It refreshes module flight-plan state, copies waypoint records to 0x40-byte slots and processes requests. The direct call from 2086a380 supplies two zero stack arguments. |

The previous description of ordinal 1's call as an unidentified
dispatch is superseded by the verified Release target and body.
`GpsProvider` is a reconstructed minimum layout with a table pointer
at +0 and a reference count at +4. Fields at +8, +0xc, +0x10, +0x12
and +0x14 have known widths and initial values. Their broader roles
remain open. The pilot uses a volatile table member to preserve the
two observed writes while the original class hierarchy is unresolved.
The table type models the first two slots and types only the verified
Release slot. The source still references both tables, the allocator,
the destructor and the callbacks as unresolved symbols.

Suggested Ghidra names, unapplied. At 20856050, `param_1` to
`provider` follows the ECX object and `iVar1` to
`remainingReferences` follows the returned decremented value. At
2086a380, `param_1` has two disjoint live ranges. `eventCode` is a
candidate name for its incoming value, and `allocatedState` describes
the reused stack slot after allocation. Callback parameter types
and the event meanings remain open. Ghidra reads made no edits and
the prior save state remains unverified.

## Build evidence

Run `gps-provider-init-20261115-003` passed the VC8 x86
14.00.50727.762 build without warnings. The retained source snapshot,
build command, hashes and receipt are in the
[run directory](../../../vm-share/modules/gps/runs/gps-provider-init-20261115-003/).
Object SHA256 is
`E5607D08DE134497F57268DCC4731B6771A13D781E0DE27BB9D12F4700DBC7F3`.

- Ordinal 1 is 137 bytes on both sides. All 11 relocations were checked.
  The rebuilt code issues the provider and string argument pushes
  earlier in the same block. Reordering these two instructions in the
  diagnostic accounts for every remaining byte difference.
- Release is 49 original bytes and 46 rebuilt bytes. Both relocations
  were checked. The original compares the decremented count against 1
  before an unsigned branch. The rebuilt code branches on the zero
  flag from the decrement. That instruction selection accounts for
  the entire difference after relocation normalization.
- Ordinal 2 still matches the complete 42-byte body with all five
  target-validated relocations.

Trial 001 reused the address-taken allocation local, producing
repeated loads and a 165-byte ordinal 1. Trial 002 gave the provider
pointer its own local and reduced the body to 137 bytes. Trial 003
replayed the latter source through the completed diagnostic.
The [comparison report](../../../vm-share/modules/gps/runs/gps-provider-init-20261115-003/provider-comparison.json)
and dashboard retain the observed instruction differences. DLL
linking and runtime verification remain open.

The follow-up [cleanup analysis](gps-provider-cleanup.md) reconstructs
destructor 208560e0, its entry-release loop and the ESI boundary of
cleanup helper 2086a4c0.
