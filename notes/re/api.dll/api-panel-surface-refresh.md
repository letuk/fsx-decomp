# API panel surface refresh after geometry changes

Live verified in original `api.dll`. Eleven retained function
listings totaling 5957 instruction bytes were checked against
the original PE. Names and fields are reconstructed.

## Child window refresh

FS98CHILD message 0x467 reaches `201401d3`. The source models the
controller gate, visibility callbacks 3 and 4, graphics-frame
selection, movement between that frame and the auxiliary window,
and the geometry refresh counter at context +0x38. It reuses the
related-panel geometry paths. `2012befe` converts the saved
reference-grid values to frame coordinates, `2012c2bb` applies
the anchor flags and `2012c22f` reparents eligible related panels.

The resize and refresh paths meet at `2012b9af`. The 0x467
handler can then emit WM_MOVE and WM_SIZE, notify a changed
parent, reorder clip windows and flush the pending graphics
notification. The two WM_SIZE frame-style reads and menu query
at `20140e58` through `20140e75` use link +4, the auxiliary HWND.
The previous source use of link +0 is superseded.

## Surface backing

`2012b9af` guards reentry with the DWORD at `2020e434` and
limits the requested client dimensions before splitting by the
shared-surface key.

- Private surfaces compare the prior width, height and format,
  plus the graphics index at link +0x1c when a graphics entry
  exists. The changed path releases the old bitmap and selects
  graphics backing or a top-down DIB. The DIB descriptor supports
  8-bit RGB332, 16-bit RGB555 or RGB565, 24-bit and 32-bit formats.
  The path invokes callback 6 and callback-list event 0x70.
- Shared surfaces use the existing registry and callback-array
  visitor. Callback `2012b7bd` finds maximum member dimensions,
  and the refresh routine also applies the graphics clip dimensions.
  With a registered panel callback, a change in pitch, height or
  format clears context +0x41 and writes the observed `0xdeadbeef`
  sentinel into the pixel field. Callback `2012b7fc` distributes
  format, pitch, byte count and the pixel field. Width and height
  remain per panel.

The API-side IAT confirms `g2d.dll` ordinals 4 and 66 at
`200e1338` and `200e1318`, `window.dll` ordinals 15, 16 and 23
at `200e1518`, `200e151c` and `200e1504`, and
`gdi32.dll!CreateDIBSection` at `200e1bfc`. External module
bodies are outside this package. The API graphics-release helper
crossing `20138f47` and slot +0x70 was subsequently identified
as menu-interface handle release and connected in
[the menu-host package](api-panel-menu-host.md). The callback-list implementation
and graphics-table targets remain integration boundaries. The
DIB path follows the observed unchecked allocation behavior.

Suggested Ghidra names, unapplied. In `201401d3`, `in_EAX` →
`panelContext` from the HWND, window and counter accesses. In
`2012b9af`, `pHVar12` → `surfacePixels` from the format, width,
pitch and pixel-field accesses. The decompiler's HDC inference
for that variable is superseded by the instruction listing.

## Native result

`api-panel-surface-refresh-20261110-003` passed all 20 existing
regression comparators. COFF SHA256 is
`13B19F0D5324BECA57EFFC8E980217D61A6CDF48E3ACFB69821542CCA81699D0`.
The generated `2012b7fc` leaf still matches all 62 bytes and was
already counted. The typed callbacks in the pilot and the larger
handlers have no newly accepted byte matches. Accepted coverage
remains 5263 code and 288 metadata bytes. Linking and runtime
verification remain open. Ghidra was used read-only.

The follow-up identifies `20138f47` and +0x70 as menu-interface
lookup and handle release. The earlier graphics-context wording
is superseded. Graphics receivers and surface backing in
`g2d.dll` belong to a separate module package.
