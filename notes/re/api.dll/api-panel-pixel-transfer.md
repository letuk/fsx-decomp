# API panel pixel transfer

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Sixteen instruction listings totaling 3229 bytes were checked against the
PE. Live references confirm both transfer callers and the palette, DC and
format table dependencies. Ghidra reported `changed=false`, modification 3.
All source identifiers are reconstructed and only API-side evidence is
attributed here.

## Dispatch and clipping at 2011f7d3

The original reads the destination rectangle in EAX and the source
rectangle in EDX. The three stack inputs are the window, destination
surface and source surface. The latter is window +0x78 in the two
verified callers. This supersedes ambiguous operand roles in the
exported C. The observed surface fields are signed 16-bit format +0,
dimensions +4/+8, byte pitch +0xc and pixel pointer +0x18. The
seven observed format-width entries at `201f9910` are 1, 1, 2, 2, 2, 3
and 4 bytes. The original performs the table lookup before dispatch and
does not range-check its signed format index.

The two rectangles must have equal extents. The original clips the
destination first and then the source, advancing the opposite origin
when a negative coordinate is clipped. Nonpositive final extents or
null pixel pointers skip the transfer. There is no scaling.
Window flag 0x800 enables masked writes unless 0x40000000 is set.

| Source format | Destination format | Observed operations |
|---|---|---|
| 1 | 2 or 3 | Expand byte indices through the palette and store the low WORD. Masking skips the index sampled from the source allocation's first byte, before clipping. The original selects a grouped loop at width four and above. |
| 1 | 6 | Expand indices to DWORD palette values, with the same optional index skip. |
| 2 | 2 | Copy two bytes per pixel. The masked path writes only source words with bit 15 set. |
| 2 | 3 | Preserve the low five bits and shift the next ten left by one. Masking gates on bit 15. |
| 2 | 6 | Expand three five-bit channels into the high five bits of each output byte. Masking gates on bit 15. |

Unsupported pairs have no pixel-copy branch. The format-1 path calls
the palette helper before validating the destination dispatch. The
source uses scalar loops for the output operations that were grouped
in the original, preserving row strides and masked destination bytes.
It explicitly retains the unmasked 2-to-3 helper's alignment-dependent
width adjustment and row advance at `2011f532`. A separate arithmetic
check compared the scalar and paired conversion expressions across
all 65536 input words. This does not establish identical memory access
widths or runtime behavior.

## Palette and DC boundaries

`2011fe4c` caches 256 DWORD palette entries at `2020e688` by destination
format at `2020ea88`. It updates the cache key before obtaining the
color table and does not check the GetDIBColorTable count. Its observed
packing is five bits per channel for format 2, five/six/five for format
3 and 0x00RRGGBB for format 6. The source preserves the format-only
cache and unchecked color-table output.

Export 182 at `2011d05c` leaves its output untouched on a null window.
With nonzero window +0x74, it clears the output and returns -1.
Otherwise it saves the shared DC at `2020d134`, selects the bitmap from
link +0x18, writes the DC and returns zero. Export 183 at `2011d0b4`
restores that shared DC to -1 only for a nonnull window and DC, zero
window +0x74 and a matching handle. The valid alternate-handle case
returns zero without restoring. The shared DC producer at `2012c92c`
remains outside this source slice.

The pilot connects `2012d7ac` to the transfer. The other original
caller at `2012d545` in `2012d3fb` remains a follow-up. No graphics
callee implementation in g2d.dll was reconstructed.

Suggested Ghidra names, unapplied:

- `2011f7d3` `in_EAX` → `destinationRect` and `param_2` →
  `sourceRect`, from the matched surface extents and pixel-kernel operands.
- `2011f7d3` `local_c` → `clippedWidth`, from the clipping updates and
  row-kernel arguments.
- `2011fe4c` `in_EAX` → `destinationSurface`, from its signed format
  load and cache-key comparison.

## Native result and acceptance

`api-panel-pixel-transfer-20261109-001` returned exit code zero. Object
SHA256 is
`513D2FB7FCC76E017740215EAB5ED72ECC8EE081ED14E8F59631CCAAE392AB76`.
The initial build passed 19 existing gates. The new comparator verifies
the full 88-byte acquire and 56-byte release bodies, their two export
ordinals and seven relocation operands. Changed entry opcodes and
incorrect GDI import relocations reject. The ledger overlap check
identified the previously accepted generated release body. The gate
now requires that exact existing range and counts the 88-byte acquire
body only. The retained object passes 20 gates.

Totals are 5263 complete code bytes and 288 metadata bytes, with
123 partial Initialize bytes tracked separately. The main transfer
emits 719/820 bytes and the palette helper 757/756 native/original.
Both remain outside byte acceptance. One build used the retained
compiler profile. No linked candidate or runtime test was performed.
Next is the alternate transfer caller `2012d3fb` and the shared DC
producer.
See [the surface lifecycle follow-up](api-panel-surface-lifecycle.md)
for their source integration.
