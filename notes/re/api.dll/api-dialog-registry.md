# API dialog registry and message dispatch

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Eleven instruction exports totaling 1939 bytes were verified against the
pinned PE. Evidence also includes export ordinals, original data and
import slots, and explicit live xrefs and VTable field/target reads.
Ghidra reported `changed=false`, modification 3. No other binary is
attributed to this package.

## Dialog operations

- Export 63 at `2013246b` looks up the first matching HWND. Show invokes
  controller slot +0x2c with (1, true), appends an HWND even if the lookup
  found a match, and calls ShowWindow(SW_SHOW) and UpdateWindow. Hide
  invokes ShowWindow(SW_HIDE) and UpdateWindow, calls the controller slot
  with (1, false), and removes the first matching entry if found.
  A missing entry still goes through the hide/update/controller calls.
- Export 64 at `20132546` passes MSG and callback `2013259f` to the
  forward iterator. Its result is the booleanized callback result.
  `2013259f` requests removal for an invalid HWND. For a valid HWND it
  sends synchronous message 0x464 with zero parameters, reloads the HWND
  from the entry and calls IsDialogMessageA. It returns the entry on a
  handled message, so later entries are skipped.
- The controller hide callback `2013256c` hides and removes valid
  entries. Invalid HWNDs are left in place on this path. It returns zero
  so the iterator continues. The verified caller is `20125ee5`, via
  its visitor invocation at `20125fba`.

The source routes the HWND lookup directly through an equality scan. The
original uses the direct four-byte specialization of `2013bfe4`.
Its other search modes remain separate. No call from the reconstructed
internal application message loop to export 64 is established by these
`api.dll` references. Export 64 is available as its own source entry.

## Storage and deferred removal

The registry at `201f7540` and pending-removal list at `201f7098` have
observed 0x1c-byte records with count, capacity, growth, stride, traversal
depth, pending flag and backing pointer at +0/+4/+8/+0xc/+0x10/+0x14/+0x18.
Both start with growth 32 and empty backing storage. The dialog record has
stride 4. The pending record has stride 8 and contains owner and index.

`2013bc26` increases capacity before its multiplication check, uses the
private allocator or reallocator, increments count and zeroes a new
record. `2013bd13` computes the physical index from the record pointer.
At nonzero traversal depth it sets the flag and appends a pending removal.
Otherwise it decrements count and shifts the tail. The original calls
the memcpy thunk at `201ec951` for the lower-address overlapping shift.
The pilot uses memmove to express the shift with defined overlap behavior.

`2013bdee` increments traversal depth, checks pending entries and stops on
a nonnull callback return. Its pending skip compares traversal ordinal,
even on the reverse path where the physical record is chosen separately.
After the outermost traversal, it processes pending entries for that
owner in reverse queue order. The original also tests whether the pending
array itself is under traversal when removing a pending entry. The same
index removal model covers that branch. This pilot connects the dialog
consumer. Other original callers of the shared container remain outside
this package. Shutdown writes and releases the registry storage in the
unreconstructed prefix of `20120ed5`.

## Ordinal 172 record lookup

Live verified on 2026-11-19 for `api.dll` at `0x2013bdcc`. The PE
export slot resolves ordinal 172 to the same address. Ghidra names the
function `FUN_2013bdcc` and shows only an external entry-point reference.
The two stack arguments, unsigned comparison and `RET 8` support the
reconstructed `__stdcall GetCallbackArrayRecordPilot(array, index)`.
The lookup returns NULL when the unsigned index is greater than or equal
to the count at offset 0. Otherwise it returns the backing pointer at
offset 0x18 plus the index multiplied by the stride at offset 0xc.
The existing reconstructed callback-array layout covers these fields.
[Live listing and references](../../../artifacts/api.dll/callback-array-record-20261119/ghidra-2013bdcc.json).

The generated candidate's byte result is superseded for this scope by
the typed C++ definition in `src/api-pilot/dialog_registry.cpp`. The
generated control remains in the same factory object and differs at
offsets 0x0b and 0x0d. It reverses the CMP operands and uses JA where the
original uses JB. The typed definition uses the unsigned index as the
left operand and reuses the existing record address calculation. The
helper now takes an unsigned index, matching its existing unsigned
multiplication by the stride.

Live verified in VC8 RTM build `api-callback-array-record-20261119-001`.
The complete native x86 COFF body matches all 34 original PE bytes at
`0x2013bdcc`–`0x2013bded`. There are no relocations in either range.
The XP receipt succeeded, all 20 source-pass comparators passed and
the prior 30-byte `ErfSetCodes` exact gate passed again. The retained
run includes the source snapshot, command, source hashes, build log
and object. Linked DLL and runtime verification remain open.
[COFF and ordinal comparison](../../../artifacts/api.dll/callback-array-record-20261119/comparison.json).

Suggested Ghidra parameter names are `param_1` → `array` for the
record owner and `param_2` → `index` for the bounded record index.
These are suggestions for the decompiler variables. Read-only Ghidra
reported saved revision `1793741866691:51f17f6c:3`.

Reproduce with the factory source-pass profile:

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-callback-array-record-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-callback-array-record.cjs vm-share/modules/api/runs/api-callback-array-record-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-callback-array-record-UNIQUE-ID
```

## Controller field and target evidence

Construction at `20125380` installs table `200f2ce0`. The live fields
below are untyped. Target evidence is separate from the field read.

| Field | Target and observed use |
|---|---|
| `200f2ce0` (+0) | `20124dcc` returns whether the signed field at +0x34 is positive. |
| `200f2ce4` (+4) | `20124dd5` returns byte +0x38 in AL. |
| `200f2cf0` (+0x10) | `20126129` returns true for an iconic window or a rectangle whose left or top is at least 20000. |
| `200f2cfc` (+0x1c) | `20124ddd` returns byte +0x3b in AL, already used by the pilot idle path. |
| `200f2d04` (+0x24) | `20125ee5` handles controller hide and invokes the registered-dialog hide visitor. |
| `200f2d0c` (+0x2c) | `20125b7f` reads two stack words and ends with RET 8. Its second argument is tested as a byte. It changes fields +0x44 and +0x48 and can call additional slots. |

`20125ee5` exits if slot +0x10 returns true. If slot +0 returns false,
it writes `MAIN/Maximized` using mode 1 for slot +4 true, otherwise 2
for IsZoomed and 0 for false. It can set object byte +0x3f around a
WM_COMMAND. It then moves the window to (20000, 20000) with flags 0x15
and visits the registered dialogs. The pilot reconstructs this body and
shares the extended controller interface with the existing idle path.
The controller object, its original VTable placement and the +0x2c method
implementation remain external. No Ghidra field or signature was edited.

Suggested Ghidra names, unapplied:

- `2013246b` `param_1` → `dialogWindow` and `param_2` → `show`, from
  the HWND lookup and visibility branches.
- `2013259f` `param_1` → `dialogEntry` and `param_2` → `message`, from
  the HWND dereference and the IsDialogMessageA arguments.
- `2013bdee` `param_3` → `reverseTraversal`, from the physical-index
  calculation. The pending skip still uses the traversal ordinal.

## Native result

`api-dialog-registry-20261109-001` has object SHA256
`BC9964AC09E12AA08BCA7FDCA1F00F27360132107D95CAA4B5204A28A753BF6D`,
receipt exit code zero and 18 passing gates. Native/original sizes include
visibility 296/219, dispatch export 36/38, message visitor 101/82,
controller hide 225/223, and iterator 401/392. The COFF dispatch export
references the visitor, registry and iterator. One source trial used the
retained compiler profile. New bodies remain outside acceptance. The
ledger stays at 5069 complete code bytes and 288 metadata bytes, with
123 partial Initialize bytes separate. No linked runtime test was performed.
The next connected slice is the controller mode update at `20125b7f`
and its state-change callbacks.
See [the controller mode follow-up](api-window-controller-mode.md) for
their source definitions and remaining boundaries.
