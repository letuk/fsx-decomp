# API panel hit-group registration

Live verified in `api.dll`. Six retained listings totaling 782
instruction bytes agree with the original PE. The source names
and field views are reconstructed.

`20136645` receives the panel window in EAX and five stack
arguments. It rejects a missing window or link. If link +8
is empty it attempts tooltip creation at `2013699b` and continues
to lookup regardless of that result. It searches the array at
link +0x2c by the record-list pointer at entry +0x10.
Registration rejects duplicates. Update requires an existing
group and a nonnull owner on its first record, then reuses that
owner's saved origin. The group bounds start at surface width
and height for left and top, and zero for right and bottom.

`2013670f` walks 0x2c-byte records. Kind four contributes no
bounds. Kind one recursively consumes its children, using the
parent's calculated left and top as the child origin. Kind zero
or three ends a level. The return count includes that terminator.
The builder allocates a 0x20-byte owner when adding or when an
update has no owner, and reuses it otherwise. Right and bottom
include the original extra pixel. It expands the group rectangle
and refreshes an active hover record through `20136bc9` when
required. The private heap allocation retries once before the
original resource-8 error path.

Exports 146 (`201362f6`) and 186 (`20136339`) add a group with
default or explicit context respectively. Exports 147 (`20136317`)
and 187 (`2013635c`) update it. Export 147 reads the first owner's
context word before forwarding. All four return the status from
EAX. The source connects them to registration, tooltip creation
and recursive bounds construction. Their final export table
placement remains separate.

Suggested Ghidra names, unapplied. In `20136645`, `in_EAX` →
`panelWindow` and `param_5` → `updateExisting` from the link
lookup and branch. In `2013670f`, `local_c` → `recordIndex`
from the 0x2c stride and recursive increment.

## Native result

Both builds passed 20 existing comparators. Final run
`api-panel-hit-registration-20261110-002` emits all six entry
bodies. COFF SHA256 is
`7B9F6D55685EE70C6234D7F6C7A98B371325EC82031803B5A0BA3CEEEDB6A392`.
No additional byte acceptance, linked runtime test or Ghidra edits.
No other DLL body was analyzed or reconstructed.

The follow-up package implements removal and cleanup at `201368ce`
and its caller. See api-panel-hit-removal.md.
