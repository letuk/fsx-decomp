# API panel geometry and shadow drag

Live verified in original `api.dll`. Nineteen retained function
listings and the class registration block account for 5539
instruction bytes checked against the original PE. The class string
`FS98SHADOW` was verified at `201f713c`. Source names and field
interpretations are reconstructed.

| API entry | Observed behavior connected in source |
|---|---|
| `20140f65`, `2013f5ed` | Child WM_MOVE updates panel offsets in the graphics frame and refreshes order and visibility. Dialog WM_MOVE conditionally notifies the graphics controller. |
| `20140dc1`, `2012c129` | WM_SIZE updates panel dimensions, stores scaled geometry in an 8192 by 6144 grid, reaches surface refresh for nonzero dimensions, and invokes the resize callback. |
| `20141131`, `2012b83a` | WM_GETMINMAXINFO supplies a 39 by 39 minimum on the flagged route. The other route obtains its limits from panel fields and a graphics query. |
| `201410e0`, `2012c502` | WM_EXITSIZEMOVE sends WM_MOVE then WM_SIZE to the panel and to eligible linked windows on the enumerated route. |
| `20141fd6`, `2012c3d1` | Message 0x479 aligns or scales the related panel when dimensions are unset or differ by more than two pixels. Context byte +0x40 guards reentry while setting geometry. |
| `20122240`, `2013d488`, `2013d683`, `2013d703` | Registration and creation of the shadow window, capture, initial hit test and dispatch. The observed allocation is 0x64 bytes, with the reference count at +0x28. |
| `2013d7f5`, `2013cfbb`, `2013cefd` | Updates the drag preview with sizing limits and a 15-pixel snap threshold against eligible siblings and the parent frame. |
| `2013daee`, `2013d796`, `2013dbc3` | On release, conditionally reparents the panel, applies bounds and sends 0x467. Nested destroy releases capture and its reference. Paint only validates the shadow update region. |

The package ends at surface refresh. The release notification goes
to FS98CHILD handler `201401d3`, which reaches `2012b9af` at
`20140c55`. WM_SIZE also calls `2012b9af`. This
handler, the surface recreation routine and callback `2012b7bd`
are covered by the separate [surface refresh package](api-panel-surface-refresh.md).

The original IAT identifies `g2d.dll` ordinal 66 at `200e1318`
and `window.dll` ordinals 15, 16 and 23 at `200e1518`,
`200e151c` and `200e1504`. These imports and graphics-table calls
are interface boundaries. Analysis and source changes in this
package are confined to `api.dll`. The API sibling-container
append at `201265b6` and startup registration caller remain
integration boundaries. The snap source models valid iteration.
Original checked-iterator failure paths still need coverage.

Suggested Ghidra names, unapplied. At `2013d683`, `lpMem` →
`dragState` from dispatch and the +0x28 reference count. At
`2013cfbb`, `in_EAX` → `requestedBounds` from the four
rectangle edge reads.

`api-panel-geometry-shadow-20261110-002` produced COFF SHA256
`455B03CBAF672EC4432504E0FC710332EE3B3D7B1DEEEA0DEB5C4504294DE78B`.
All 20 existing regression comparators passed. Accepted coverage
remains 5263 code and 288 metadata bytes. The package has no linked
runtime result or Ghidra edits.
