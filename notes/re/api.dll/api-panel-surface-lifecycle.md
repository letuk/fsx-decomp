# API panel surfaces and shared GDI device context

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Six instruction listings totaling 1006 bytes were checked against the
original PE. The import slots, array initialization words, property name
and live references were checked separately. The live program reported
`changed=false`, modification 3.

## Device context and surface registry

DC means GDI device context. The HDC at `2020d134` is separate from the
palette cache at `2020e688`. `2012c92c` calls CreateCompatibleDC with null
when the global is zero and stores the result. This occurs before it
checks the window's +0x74 key. A null creation result is stored and the
next registration can retry. The pilot now defines the HDC used by the
palette, DC wrappers and bitmap drawing.

For a nonzero window +0x74 key, `2012c92c` looks up the shared surface
registry at `201f94e0`. The original array starts empty with growth 32
and stride 64. A new record stores the key at +0, copies eight DWORDs
from window +0x78 into +4, and initializes an embedded callback array
at +0x24 with growth 32 and stride 4. The window pointer is appended
only if absent. The pilot reuses the previously modeled callback-array
append and removal operations and implements the four-byte equality
lookups directly.

`2012c9f7` receives context in EAX. For a zero key it calls the optional
window +0x18 callback with (6, window, 0, 0), then deletes and clears the
bitmap at context link +0x18. For a nonzero key it removes the window
pointer from the shared group's member array. A zero member count frees
that array and removes the group. The source preserves the callback
array's deferred-removal behavior. These paths do not delete the
shared memory DC. No direct DeleteDC path was established for this
global by the reviewed references.

`2012d335` selects the context link's bitmap into that DC, stretches from
the window's +0x7c/+0x80 dimensions to the client rectangle, then restores
the previous object. `2012d3b0` uses inclusive edges, aligns the left
coordinate down to a four-pixel boundary, and uses the expanded rectangle
on both sides of the SRCCOPY. Their larger drawing caller `2012c719`
remains a source integration boundary.

## Alternate panel transfer

`2012d2cd` checks window flag 0x1000. If set, it forwards the context
from the window link at +0xc to `2012d3fb`. Its first stack argument
is unused. `2012d3fb` starts at GetWindow selector 1 from the linked
HWND and walks selector 3. It lazily adds the `SimAWind` global atom
at `2020e2e0`, retrieves each sibling's property and skips null or
same-window pointers and targets missing either bit 31 or 0x100.

The target dimensions at +0xa4/+0xa8 must each be within two pixels of
the owner's dimensions at +0x7c/+0x80. A mismatch posts message 0x479
and continues. A match calls the update-begin ordinal with the target,
zero and the observed 36-byte output span. A nonzero return stops the
entire walk. On success the source rectangle starts at zero and spans
the target dimensions at +0x38/+0x3c. The destination starts at the
target's +0x30/+0x34, adding the owner's origin when the owner's flags
0x60000000 are clear. The routine transfers from the target's +0x78
surface into the ordinal's output surface, then calls the end ordinal.
The original preserves the sibling walk after a successful update.

`api.dll` IAT slots `200e1310` and `200e130c` identify `g2d.dll`
ordinals 68 and 69. They remain external, with their caller-side argument
shape modeled. The graphics module has no reconstruction attributed to
this package.

Suggested Ghidra names, unapplied:

- `2012d3fb` `pHVar3` → `siblingWindow`, from the GetWindow walk and
  GetPropA call.
- `2012d3fb` `pvVar4` → `targetWindowRecord`, from the property lookup
  and the later target flag and surface accesses.
- `2012c92c` `puVar3` → `surfaceGroup`, from the key, copied surface
  span and embedded member array.

## Native result

`api-panel-surface-lifecycle-20261109-001` returned exit code zero and
passed 20 comparators. Object SHA256 is
`18D30E498DFA68CD0E053217F133BFDD3E977DBFE6BE7D3FD1CAA4F7C7F444CB`.
Native/original sizes are 198/203 for registration, 254/176 for
unregistration, 374/393 for the alternate update, 124/123 and 70/75
for the StretchBlt helpers. The 36-byte notification wrapper matches
with its REL32 target verified and was already counted in generated
coverage. No new accepted bytes. Totals stay at 5263 code and 288
metadata bytes, with 123 partial Initialize bytes separate. The
retained profile was used for one build. Linking and runtime remain
open. Next is the bitmap drawing caller at `2012c719` and its
region enumeration.
See [the bitmap drawing follow-up](api-panel-bitmap-draw.md) for the
draw body, its fallback and its callers.
