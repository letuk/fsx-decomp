# API panel hit-group removal

Live verified in `api.dll`. Four retained listings totaling
542 instruction bytes agree with the original PE. API IAT
`200e1b90` resolves to USER32 DestroyWindow.

Export 148 at `2013637b` requires an attached link on a nonnull
window. It sets the pending hover reset at `2020e05c` before
looking up the group by record-list pointer. It returns -1
for a missing group. On a hit it removes the array entry first
through `2013bd13` and then releases the hit bounds. An empty
array afterward invokes `20136ab8`, which destroys the tooltip
HWND and clears link +8 and +0x14.

`201368ce` compares the list base at each recursion entry
with the published capture and repeat records. A capture match
uses `20137891`, including its release callback. A repeat match
clears `2020e08c` directly. It walks records of stride 0x2c,
skips kind four, recurses for kind one, and stops at kind zero
or three. For each visited record it clears hover through
`20136b21`, frees the optional text at owner +0x1c, then frees
and clears the owner at record +0x28. The count includes the
terminating record. The caller-owned record list is retained.

`20136b21` clears both the published global hover and the
supplied window's link selection when they match the removed
record. It sends the existing 0x409 clear packet when a target
link is available. The source retains the extra link clear
after that synchronous notification.

Suggested Ghidra names, unapplied. In `201368ce`, `param_2` →
`records` from the stride and recursive walk. In `2013637b`,
`param_2` → `groupRecords` from the array key and cleanup call.

## Native result

`api-panel-hit-removal-20261110-001` passed all 20 existing
comparators. COFF SHA256 is
`1EE0A043915A278B8B077FDC8A3F944AB26F27BBCFDF269F68CBE22329220B8B`.
Registration and removal share the source lookup. No additional
accepted byte range, linked runtime test or Ghidra edits.
USER32 remains an import boundary.

The follow-up package implements the text setter and refresh.
See api-panel-hover-text.md.
