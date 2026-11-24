# GPS legacy data lifecycle

2026-11-15. Live verified in gps.dll against the pinned PE described
in [GPS provider registration](gps-provider-registration.md).

| Address | Evidence and behavior |
|---|---|
| 2086a380 | Slot 0x10 callback. Reads the first stack argument as a selector and exits with RET 8. For 0x36 it allocates 16-byte storage through api.dll ordinal 106 when the global at 2086e9f0 is null. New storage has count and extent zero, field +8 set to 5, flags set to 4 and a null entries pointer. It initializes the data record with the storage entries pointer and resets selected request fields. It calls the refresh callback at 2086a520 with two zeros, then publishes the data and request records through panels.dll ordinal 34 with selector 0x26. For 0x37 it removes the two names through panels.dll ordinal 38 in request-then-data order, calls 2086a4c0 for a nonnull storage pointer, and clears the global. |
| 2086aa10 | Receives the data record pointer in EAX and the entries pointer on the stack. It clears eight double fields and six 32-bit fields, stores size 0xb0 at +0 and the supplied pointer at +0xac, and uses RET 4. The call at 2086a41d is the only live direct reference. |
| 2086e940 | Request record. The lifecycle stores 2086e140 at +0xac. The refresh callback reads the size at +0 and the request selector at +4. |
| 2086e9f8 | Data record passed to the initializer and published as FS2002 GPS DATA. |

`GpsLegacyRecord` is a reconstructed minimum layout of 0xb0 bytes.
Padding covers fields outside these initialization paths. Storage
allocation uses an out pointer. The original still loads storage +0xc
after the null-return branch. The pilot preserves that sequence.
The source receives both callback stack arguments as 32-bit values.
The second argument's type and the external event names remain open.

The updated pilot registers and unregisters a local
`GpsLegacyDataLifecycle` and reaches the previously built ESI cleanup.
This supersedes the report that the lifecycle callback was an
unresolved address. The refresh callback and record storage remain
unresolved. The initializer uses a 7-byte adapter to move the EAX
argument to ECX and tail-jump to the generated body.

## Verification

Run `gps-legacy-lifecycle-20261115-003` passed without warnings on
VC8 x86 14.00.50727.762. The run retains the source snapshot,
build command, hashes, receipt and both listings. Object SHA256
`1B76573A4FAEF9DCC4DB5D8265CFB9DB77574F9102393687CD1DF36BF3F210E6`.

- Callback size is 315 original bytes and 313 rebuilt bytes. The
  comparator verifies all 35 relocations, including import identities,
  strings and the addends of all 16 request-record fields. The
  147-byte reset and publish block matches after reference validation.
  The event dispatch, storage initialization and register-adapter
  setup use different encodings.
- The complete 85-byte initializer body agrees after accounting
  for the EAX/ECX register exchange. The adapter's jump target and
  the body's RET 4 are verified separately. Both complete bodies
  appear in the dashboard as diagnostics.
- Prior GPS comparison gates passed, including normalized matches
  for ordinal 2 and provider entry release.

Trial 001 used the address-taken storage local for initialization,
producing a 336-byte callback with repeated loads. Trial 002 used
a separate allocation out pointer, yielding the 313-byte body.
Trial 003 verified that source with the completed comparator.
See the [COFF report](../../../vm-share/modules/gps/runs/gps-legacy-lifecycle-20261115-003/legacy-lifecycle-comparison.json).

Suggested Ghidra names, unapplied. At 2086aa10, `in_EAX` to
`dataRecord` follows the record field stores, and `param_1` to
`waypointEntries` follows the pointer stored at +0xac and later
used by the refresh path. Ghidra was read-only and the prior save
state remains unverified. DLL linking and runtime checks remain open.
The next candidate is the count and allocation path at 2086aac0
and 2086ab20 used by the refresh callback.
