# API panel update regions and transfer preparation

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Three original instruction listings totaling 1279 bytes were checked
against the PE. Live xrefs identify the only direct region-rebuild
caller and all accesses to the second shared region. The live program
reported `changed=false`, modification 3. Names and record slices
describe observed API roles.

## Region rebuild at 2011e6cb

The original receives its window in ESI. A null window, null link
at +0x98, or graphics index 0xffffffff exits. The code saves the link
before index lookup and uses the resulting index with the graphics
receiver's +0 slot. Window flag 0x20000000 selects a zero-initialized
RECT supplied by entry slot +0xc. Its width and height are differences
between right/left and bottom/top. Other windows use +0x38/+0x3c
dimensions and offsets +0x30/+0x34.

The first shared region at `2020de40` is lazily created before entry
slot +0x14 is tested. A true low-byte result sets the update region at
link +0x24 to the full local bounds. A false result follows this path:

1. Lazily create the second region at `2020de44`. Failure of that
   creation returns immediately. No equivalent branch tests the first
   region's creation result.
2. Obtain a graphics region through entry +0x5c. If it is nonnull,
   copy it into both shared regions. If it is null, keep the shared
   regions' current contents.
3. Without flag 0x20000000, offset both scratch regions by negative
   window origin. Union the first scratch region into the update region
   and intersect with local bounds. The path without 0x20000000 also
   intersects with the second scratch region. The flagged path uses
   the graphics RECT dimensions for its bounds.

Both paths then copy link +0x28 into the first scratch region, apply
the same conditional negative origin, and subtract the result from
link +0x24. The two shared handles are retained. Their release remains
the existing last-group cleanup at `2011dd5d`. The source now defines
both handles and connects both region producers.

## Panel update caller and stack failure

`2012d584` calls the rebuild at `2012d650` from the window at context
+8. A null cached graphics pointer at window +0xa0 exits. When dimensions
+0x38/+0x3c differ from +0x7c/+0x80, it uses cached receiver slot +0x70
to draw a fill and four border strips with the observed colors. This
branch returns without starting the ordinary update.

For equal dimensions the original calls IAT `200e1310` with window,
zero and an opaque 36-byte stack span. Nonzero return exits. If flag
0x40000000 is clear and the link is nonnull, it reads or reuses
`DISPLAY/UsePartialPanelUpdate`, with default -1 and uninitialized
sentinel -2 at `201f9958`. A nonzero option invokes the rebuild,
queries the region data size, probes stack storage, reads the data,
and checks the header for size 0x20 and type 1. Valid data supplies a
bounding RECT from GetRgnBox. Invalid data skips transfer preparation
and still reaches the end-update call. The full update path supplies
a null RECT.

The original EH4 scope record at `201ef898` targets filter `2012d6cb`
and handler `2012d6df`. It guards the dynamic stack probe at
`2012d674` only. The filter handles 0xc00000fd, and the handler calls
msvcr80!`_resetstkoflw` via `200e113c` and returns before the
end-update call. The source uses an allocation-only `__try` region
with that filter and return. The native COFF contains its EH4 scope
reference, `__alloca_probe_16` and `_resetstkoflw` relocations. Original
probe placement, complete emitted EH equivalence and runtime behavior
remain open.

`2012d7ac` prepares two rectangles for `2011f7d3`. Partial bounds use
nonnegative left/top and right/bottom limited to the window dimensions.
Full bounds use the two separate dimension pairs. The rectangle passed
in EAX is offset by the window origin when flag 0x40000000 is clear.
The rectangle in EDX stays local. The three stack inputs to the transfer
are the window, opaque state and window +0x78. The transfer remains an
explicit API source boundary.

The `api.dll` IAT identifies `g2d.dll` ordinals 4, 68 and 69. Those
remain external. The graphics callee bodies and graphics module ledger
were outside this package.

Suggested Ghidra names, unapplied:

- `2011e6cb` `unaff_ESI` → `window`, from the link, flag and coordinate
  reads. `local_18` can be the start of `graphicsBounds` once the four
  contiguous words are typed together.
- `2012d584` `local_60` → `updateState` (provisional), from the ordinal
  68 output and transfer preparation arguments.
- `2012d7ac` `in_EAX` → `partialBounds`, based on its optional RECT
  coordinate reads.

## Native result

`api-panel-update-regions-20261109-001` returned exit code zero and
passed 19 comparators. Object SHA256 is
`7E039ADA9F5AEBB0880F7D427B3CDD824C18FECE51E259F0B6D0B903607DCB0A`.
Sizes are native/original: region rebuild 520/561, panel caller 546/511
and transfer preparation 211/207. One trial used the retained profile.
No new bytes entered acceptance. Totals remain 5175 complete code bytes
and 288 metadata bytes, with 123 partial Initialize bytes separate.
No linked runtime test was performed. The next connected body is the
API-side rectangle transfer at `2011f7d3`.
See [the panel pixel transfer follow-up](api-panel-pixel-transfer.md) for
its source integration.
