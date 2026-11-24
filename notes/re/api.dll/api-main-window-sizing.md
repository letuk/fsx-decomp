# API main-window sizing and paint suppression

Live verified in `api.dll`. Nine retained instruction listings
(2428 bytes) were checked against the original PE SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Live cross-references and decompilation identify the producer and
message paths. Names and field views below are reconstructed.

## Controller lifetime

`FS98MAIN` uses window procedure `2012e413`. The live xrefs to
`2020de9c` show two direct writes, both in that procedure. When it
needs a controller and `2020e044` is zero, it requests 0x6c bytes
through `20135dd9` and calls constructor `20125350` on allocation
success. The constructor installs table `200f2ce0`, stores HWND at +4
and initializes byte +0x3d to zero at `2012539e`. The procedure publishes
the same EAX to `2020de9c` and `2020de98` at `2012e4b6` and
`2012e4bb`, including the null allocation result. 0x6c is the
observed allocation size.

On WM_NCDESTROY it calls `2012e506` on the old nonnull pointer, which
deletes the snapshot and frees the owned block at +0x60 and the
controller. It clears both global pointers and enters the same
allocation check, so destruction can fall through to allocation when
the gate allows it. The dispatcher `2012f93d` is reached when the
controller is available. The follow-up source package implements the
procedure, constructor and teardown. See api-main-window-controller.md.

## Sizing and painting

- WM_ENTERSIZEMOVE reaches `2012f1aa` through `2012fba4`. It replaces
  the snapshot at +0x28 with a copy of the client DC at client extents
  plus one. Fields +0x2c/+0x30 store those dimensions. A failed capture
  leaves the snapshot null. After optional callback `2020e030` with
  event 0x20017 it sets +0x3d at `2012f2b0` regardless of capture
  success and returns the default window procedure's result.
- WM_EXITSIZEMOVE reaches `2012f2c2` through `2012fbd7`. It calls
  event 0x20018 before clearing +0x3d at `2012f2e5` and dropping the
  snapshot. Depending on slot zero and +0x38 it checks the iconic
  state and can return zero. Otherwise a changed client RECT or zero
  LPARAM updates the saved bounds and can refresh the graphics entry
  and send 0x467 to children through `20125264`. Normal completion
  sends WM_EXITSIZEMOVE to children via `20125285`, refreshes visibility,
  notifies graphics and forwards to the default procedure.
- Parent WM_PAINT reaches `2012f709` through `2012fac3`. With +0x3d
  clear and slot zero false it refreshes visibility and delegates.
  Otherwise it fills black if the snapshot is null or stretches the
  snapshot with COLORONCOLOR when SetStretchBltMode succeeds.
- Child WM_PAINT at `20140d00` uses the same controller byte to
  suppress drawing. Context +0x42 consumes the first subsequent
  eligible paint without drawing. See api-panel-paint.md.

The source shares the controller type for both pointer aliases and
keeps graphics as an API-side call boundary to `g2d.dll` ordinal 4.
The event callback and remaining dispatcher are explicit integration
boundaries. The follow-up procedure routes these three handlers.

Suggested Ghidra names, unapplied. In `2012f1aa`, `unaff_ESI` →
`mainWindowController` from the dispatch receiver and member accesses,
and `hdc_00` → `snapshotDc` from CreateCompatibleDC and BitBlt. In
`2012f709`, `mode` → `previousStretchMode` from the save/restore pair.

## Native result

`api-main-window-sizing-20261109-001` built the three handlers and
two child visitors. Twenty existing comparators passed. COFF SHA256 is
`B1117EC7C87FBD03C720AF09395883EF5AD2D9748AE140D8EDE2881AB2A75308`.
The original register-passed receiver is modeled as an explicit source
argument. No new body is accepted into byte coverage. Ghidra was used
read-only and no linked replacement was executed.
