# API cursor conversion and tooltip receiver

Live verified in `api.dll`. The listings for `2011fbc1`,
`2013699b` and `20136645` total 876 bytes and agree with the
original PE. The cursor-related IAT identities and class string
at `200f3170` were checked from the PE.

## Cursor conversion

`2011fbc1` receives the cursor name in EAX. It loads and copies
an IMAGE_CURSOR. If GetIconInfo returns both bitmaps and the
mask BITMAP is square, it tries two compatible DCs and a bitmap
twice the mask height. The two SRCCOPY blits stack the original
mask above the color image. It restores selections, replaces
the two bitmaps in ICONINFO with the single mask, destroys the
copied icon, and returns CreateIconIndirect's result. Earlier
failure exits return the copied cursor. The source preserves
the original cleanup sequence, including the paths where the
GetIconInfo bitmaps or the replacement mask are left allocated.
The lazy cache at `2011fd46` now calls this source body.

## Hover receiver

`2013699b` clears link +0x14, creates a topmost window of class
`tooltips_class32` with style 1 and stores its HWND at link +8.
It initializes the receiver with 0x404 and the same packet
layout used by `20136c32`. Thus API sends 0x408, 0x409 and 0x41c
to the tooltip control HWND. The receiving implementation is
outside the API-side reconstruction. The PE also imports
`comctl32.dll` ordinal 17. No body in that module was analyzed.

Live xrefs show `20136645` calls `2013699b` at `2013666f`
when link +8 is null, then continues into hit-group lookup and
registration. The follow-up source connects this caller.
See api-panel-hit-registration.md.

Suggested Ghidra names, unapplied. In `2011fbc1`, `in_EAX` →
`cursorName` and `local_14` → `stackedMask` from LoadImageA
and the two blits. In `2013699b`, `unaff_EDI` → `panelWindow`
from the link lookup and tooltip HWND store.

## Native result

`api-cursor-conversion-20261110-001` passed 20 existing
comparators. COFF SHA256 is
`B6D946542D09D6D5CA4180D97E69635EEA1167D96723E8656EE1CF9BCD2FA283`.
The cache references the emitted conversion body. Tooltip creation
is emitted as a source target. No byte acceptance, linked runtime
test or Ghidra edits.
