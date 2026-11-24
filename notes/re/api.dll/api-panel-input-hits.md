# API panel hit selection and captured input

Live verified in `api.dll`. Listings for `20137248`, `20136e98`
and `2013bf93` total 1699 bytes and agree with the original PE.
The source names and field views are reconstructed.

## Hit selection

The 29-byte wrapper `2013bf93` visits the callback array at link
+0x2c in reverse order. The source reuses its shared iterator.
Visitor `20137248` first tests the group's rectangle. It walks
0x2c-byte records until kind zero. Kinds one and three delimit
nesting. A missed kind-one rectangle skips that subtree.
The last eligible hit wins within a group. The hover record
and last nonzero input group are updated independently.

For wheel messages, flag 0x8000 advances the index by one.
The visitor can select a neighboring kind-two record based
on the signed delta, direction flag 0x10000, cursor kinds one
and two, and parity of the preceding alternating run. It
keeps the original hit for cursor selection even when wheel
dispatch moves to another record or declines the event.
WM_NCHITTEST skips cursor selection.

## Capture and action dispatch

`20136e98` receives the record in ECX, message in EAX and
input group on the stack. It builds a seven-DWORD context
with local coordinates relative to owner bounds, owner word,
record, original coordinates and a pointer to the local
message header. Capture moves call the record callback and
refresh the countdown and saved context. The byte copy preserves
the original stack-local header pointer. Its later lifetime
has not been established.

Flag 0x400 selects raw mouse callback mode and suppresses
cursor/hover postprocessing. Other messages map to action flags
for movement, button transitions, double-clicks, wheel and
0x468. The wheel invokes one action for each started 120-unit
interval of the absolute delta. The right-button fallback
requires a nonzero group and forwards group and mode one to
`2012d08c`. The outer dispatcher's unhandled right-button
fallback passes zeros for those register arguments. The
alternate-window lookup remains an API-side boundary to
`window.dll` ordinal 23.

Suggested Ghidra names, unapplied. In `20136e98`, `in_EAX` →
`inputMessage` and `local_34` → `interactionContext`. In
`20137248`, `local_1c` → `selectedIndex` based on its use
for the record offset and wheel adjustment.

## Native result

`api-panel-input-hits-20261110-002` passed all 20 existing
comparators. The first build found missing wheel constants in
the retained SDK. The adopted build uses the observed values.
COFF SHA256 is
`F32372D2E391D61FE463C8D591D815D9E6A9D693EE8FB824274F1A9BF1A35F0B`.
Both paths are connected inside the dispatcher. No additional
byte acceptance, runtime execution or Ghidra edits.

The follow-up package connects cursor and hover helpers `20136dec`
and `20136532`. See api-panel-cursor-hover.md.
