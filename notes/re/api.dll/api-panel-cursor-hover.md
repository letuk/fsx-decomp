# API panel cursor and hover

Live verified in `api.dll`. Five retained instruction listings
totaling 976 bytes agree with the original PE. The live jump table
at `20136e78` also agrees with 32 original bytes.

## Cursor selection

`20136dec` reads record +0x14. Kinds 1 and 4 choose resource 0x67,
2 and 3 choose 0x68, 5 chooses 0x69 during capture and 0x66 otherwise,
6 chooses 0x66, and 7 chooses 0x69. Other values return zero.
`2011fd46` normally calls LoadCursorA. With controller slots +4
and +0x20 true and a null module it lazily loads six cursors,
using the first cache entry as the initialization check. Five
system cursor IDs have dedicated entries and other IDs select
the first entry. The follow-up package connects conversion at
`2011fbc1`. See api-cursor-conversion.md.

## Hover notification

`20136532` clears the published window and hover record before
checking the link, then publishes the supplied pair. A pending
reset clears the local selection. The cursor-suppression branch
can clear the link's hover record without sending 0x41c.
Otherwise a changed record sends 0x41c synchronously to link +8,
then calls `20136bc9` to update link +0x14.

`20136bc9` sends the selected record's text or resource ID
and the surface dimensions through `20136c32`. Clearing the
record passes zero dimensions and resource zero. The synchronous
packet has size 0x30 and marker 0x0eb1baba. For 0x409 it first
sends 0x408, then fills the bounds and text. The helper also
supports 0x404 without the preceding 0x408. It uses the existing
resource loader for missing text. Packet fields that the original
leaves unwritten remain unspecified in the source model.

Suggested Ghidra names, unapplied. In `2011fd46`, `unaff_ESI` →
`resourceModule` and `unaff_EDI` → `cursorName` from LoadCursorA.
In `20136532`, `unaff_ESI` → `hitWindow` and `param_1` →
`hoverRecord` from publication and link comparison.

## Native result

`api-panel-cursor-hover-20261110-001` passed all 20 existing
comparators. COFF SHA256 is
`52C5EC4F8A8F9E5FF06293DA47804F563E27121CC0F83808D998B509C3A489BD`.
The input dispatcher now calls the implemented cursor and hover
paths. No additional byte acceptance, linked runtime test or
Ghidra edits. No other DLL body was analyzed or reconstructed.

See api-cursor-conversion.md for the tooltip receiver.
