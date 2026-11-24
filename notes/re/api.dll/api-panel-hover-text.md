# API panel hover text

Live verified in `api.dll`. The listings for export 214
(`20136409`) and the refresh helper (`20136ae6`) total
356 bytes and agree with the original PE. IAT `200e1140`
resolves to `msvcr80.dll!strncmp`.

The setter returns -1 for a missing window, record, window link
or bounds owner. The pointer at owner +0x1c holds optional heap
text. A null input frees existing text and refreshes the tooltip.
If the pointer was already null, the setter returns zero directly.
For nonnull text it skips updates when the first 0x50 bytes
compare equal with strncmp. Otherwise it reuses the existing buffer
or allocates 0x51 bytes, then calls the existing `20137e17` copy
boundary. Allocation retains the original retry and resource-8
error path. The setter returns zero independently of the refresh
result.

`20136ae6` receives the window in ESI and the record in EDI.
It refreshes the global active hover target when the record
matches, then separately checks and refreshes the supplied
window's link selection. Both notifications can occur. The
source uses the same helper for the equivalent hover checks
in the hit-bounds builder. Removal uses the existing private
heap slot release helper for the text.

Suggested Ghidra names, unapplied. In `20136409`, `param_3` →
`hoverText` from the compare and copy. In `20136ae6`,
`unaff_EDI` → `hoverRecord` and `unaff_ESI` → `panelWindow`
from the selection checks.

## Native result

`api-panel-hover-text-20261110-001` passed all 20 existing
comparators. COFF SHA256 is
`C9976D1BEB6D92C0A4B139110C10F52039F51025D155DDF34F56F82260B6569C`.
The setter and refresh are emitted in the API source object.
Final export placement, linked runtime testing and new byte
acceptance remain open. Ghidra was used read-only. The CRT
routine remains an import boundary.

The follow-up package connects the cursor consumer and fallback
to WM_SETCURSOR. See api-cursor-events.md.
