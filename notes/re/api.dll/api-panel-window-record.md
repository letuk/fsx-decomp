# API panel window record creation

Original `api.dll`. Three instruction listings totaling 677
bytes were checked against the PE. The class strings at
`201f7130` and `201f7148` were verified separately.

`2011cc66` takes the record in EDI and the candidate parent on
the stack. It publishes the observed 0x48-byte link at record
+0x98, creates two empty regions, and initializes the hit array.
Mode bit 2 selects the supplied parent for FS98CHILD. Mode bit 1
uses the supplied HWND as the auxiliary window. With neither
bit set, the routine creates an FS98FLOAT auxiliary window.
After creation it registers the window with the existing clip
group routine `2011db97` and sends 0x467 to the child.

The reconstructed caller now uses the signed status from
`2011cc66`. The original returns 0 or -1. The failure routes
report 0xf004 through the callback shared with `201230da` and
clear the link. Source preserves the original cleanup scope.
The regions and a child created before an FS98FLOAT failure
are outside that local cleanup sequence.

`2011d262`, API ordinal 25, checks the record, quit state and
graphics controller before sending 0x467 and setting the title
from record +0. Both routines are connected to the menu host
initialization source. The graphics controller lookup remains
at the `g2d.dll` ordinal 4 interface.

Suggested Ghidra names, unapplied. In `2011cc66`, `unaff_EDI`
→ `windowRecord` from the geometry reads and +0x98 link write,
and `puVar3` → `windowLink` from the allocation and HWND stores.

`api-panel-window-record-20261111-001` passed all 20 existing
comparators. COFF SHA256 is
`532BAE18533DFCFC0127CF927E4715FAFA0E48BB960696F21D7D1882615572A3`.
No new byte matches were accepted. Linking and runtime verification
remain open. Next is FS98FLOAT registration and its window procedure.
