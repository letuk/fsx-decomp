# API clip-region rebuild and window groups

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Five instruction listings totaling 1767 bytes were checked against the PE.
The ordinal, IAT identities and live caller references were checked
separately. Ghidra reported `changed=false`, modification 3. Layout and
field names describe observed portions of API records.

## Clip rebuild at 2011d836

The sole live direct caller is the controller visibility method at
`201260e3`. The rebuild creates a scratch HRGN, obtains the graphics
entry count from slot +0xc of the receiver returned by API IAT `200e1338`,
and reacquires that receiver before each indexed lookup through slot +0.
It skips entries whose slot +0x40 returns zero in AL. For each remaining
entry it clears the HRGN returned by slot +0x5c, then gets the base RECT
through slot +0xc on the entry receiver and creates a temporary region.

Groups at `2020de34` and their members are traversed in reverse order.
The code reads window +0x98 before calling the graphics-index helper.
A window contributes if its index matches, the saved link and its
+0xc link are nonnull, and flags +8 have bit 31 set and bit 0x100 clear.
The geometry pointer is reached through the inner link's +8. Its
origin +0x30/+0x34 is used unless its flags +8 contain 0x60000000.
Width and height at +0x7c/+0x80 are added to that origin. Window flag
0x800 makes the scratch rectangle empty.

The code subtracts scratch from the temporary base region with RGN_DIFF
when window flags 0x40004000 are clear. It independently unions scratch
into the output with RGN_OR when window +0x74 is nonzero. GetRgnBox
then writes a RECT passed to graphics entry slot +0x64. Slot +0x68
receives one. The code creates a region from the same RECT after those
calls and unions it into the output. It deletes both per-entry temporaries
and the shared scratch region at the end. The graphics-owned output
region is retained. The original ignores GDI return statuses throughout.

## Graphics index cache at 2011ee18

Export 262 returns 0xffffffff for null window, a missing link on a cache
miss, a null graphics receiver on the initial check, or an unresolved
entry. On a cache miss it calls the graphics getter twice, checks the
first result, then calls slot +0x20 on the second receiver with the key
from the saved link. On success it calls the entry's +0x50 slot with the
key and caches the pointer at window +0xa0. Both hits and successful misses
read the index through slot +0x30.

The only graphics dependency modeled here is the `api.dll` call-site
interface for `g2d.dll` ordinal 4. Offsets, argument shapes and consumed
returns are verified on the API side. The callee VTables and target
functions in g2d.dll were outside this package.

## Group storage

The globals at `2020de34`, `2020de38` and `2020de3c` hold the group
array, capacity and count. Each observed 0x10-byte group holds its member
array, capacity, count and key at offsets 0/4/8/0xc. The group key comes
from window +0x70.

- `2011db97` keeps unsigned keys in descending order and grows the group
  array by eight with allocate/copy/free. An existing key uses its group.
- `2011da64` receives the group in ESI. It rejects a duplicate pointer
  within that group and prepends new windows. Full member arrays grow by
  15 using allocate/copy/free.
- `2011dd5d` removes the first matching window pointer, compacts the
  member array and removes an empty group. When the last group is gone,
  it frees the group array, zeroes capacity and deletes and clears both
  shared region handles at `2020de40` and `2020de44` when present.

The second source slice supplies group storage and its producers to the
rebuild model. The original registration/removal callers at
`2011ce6d` and `2011cef4`, and producers of the two shared GDI handles,
remain source integration boundaries.

Suggested Ghidra names, unapplied:

- `2011d836` `local_8` → `scratchRegion`, `local_10` → `remainingRegion`,
  and `local_14` → `graphicsIndex`, from the GDI operations and lookup.
- `2011ee18` `param_1` → `window`, from the reads at +0x98/+0xa0 and the
  cache store. The precise original type name remains unknown.
- `2011da64` `unaff_ESI` → `group`, from the member pointer, capacity and
  count accesses.

## Native results

| Run | Native object SHA256 |
|---|---|
| api-window-clip-regions-20261109-001 | A2EF105AA82107E912BE3E3C2ADDC6B6F3BC41FA9FED42CD090408649014A4B5 |
| api-window-clip-groups-20261109-001 | 5FF14A68F3F100A7C671B5649A347B854F6AABC4FC783E2258BB3278255B5D4E |

Both receipts have exit code zero and 18 passing comparators. COFF
relocations connect the controller refresh to the rebuilt region body
and that body to the index helper. Sizes are native/original: 533/558
for rebuild, 122/122 for index, 195/307 for member insertion, 361/454 for
registration, and 312/326 for removal. The equal-size index body differs
outside relocation operands. The retained compiler profile was used for
one trial per slice. No bytes were added to acceptance. Totals remain
5069 complete code bytes and 288 metadata bytes, with 123 partial
Initialize bytes separate. No linked runtime test was performed.
The next connected consumer is the related region path at `2011d580`.
See [the window-ordering follow-up](api-window-region-order.md) for its
source integration and update-region byte acceptance.
