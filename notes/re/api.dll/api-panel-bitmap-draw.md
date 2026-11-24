# API panel bitmap drawing and graphics update requests

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Five instruction listings totaling 1082 bytes were checked against the
PE. The exception handler and filter, selected import identities and live
call references were checked separately. Ghidra reported `changed=false`,
modification 3. The source names and layouts are reconstructed.

## Bitmap drawing at 2012c719

The original context arrives in ECX. Window flags 0x380 cause an immediate
return. With flag 0x40000000 clear, the routine sets context byte +0x41
to one. With that flag set, it requests a graphics update and clears
+0x41 when the window has a shared surface key at +0x74. The same
update request and clear occur on the partial bitmap path when there
is no bitmap at link +0x18.

For an unshared surface, the routine reads the client rectangle and
requires its extents to match window +0x7c/+0x80. A mismatch returns
before the final update-region clear. With flag 0x10000000 clear, it
acquires the HWND's DC, calls the full StretchBlt helper `2012d335`,
and releases the DC. With that flag set and a bitmap present, it:

1. Selects the bitmap into the shared memory DC and saves the prior
   object. Queries GetRegionData for the required size.
2. For nonzero size, allocates that span on the stack. The readback is
   accepted only if nonzero and its header has size 0x20 and type 1.
3. On acceptance, gets the window DC, walks the header's rectangle
   count from offset 0x20 in 16-byte steps, and invokes `2012d3b0`
   on each RECT. Releases the destination DC even when the count is zero.
4. Restores the previous bitmap on the normal path, including zero
   size or an invalid header. At the common tail, if flag 0x10000000
   is set, clears the update HRGN at link +0x24.

The EH4 scope at `201ef350` names filter `2012c881` and handler
`2012c895`. It protects the dynamic stack probe at `2012c7f9`. The
filter handles stack overflow only. The handler restores the stack,
calls `_resetstkoflw` and branches to the epilogue, bypassing both the
bitmap restoration and the final region clear. The source intentionally
preserves those exits. The emitted object contains the EH4 reference,
stack probe and reset call. The compiler's probe helper and full EH
layout remain outside byte acceptance.

## Graphics fallback and message callers

`2011ee97` requires a window link and cached graphics entry. It obtains
two bounds through `2012d2f1`, whose original context is in ECX and
RECT output in EAX. That helper chooses either window +0x30/+0x34 or
zero as origin using flags 0x60000000, then adds +0x7c/+0x80. The update
request uses supplied local bounds or full extents, requires positive
dimensions, and maps the local rectangle to the root HWND if needed.
It then calls cached graphics slot +0x84 with four stack arguments.
The controller's +4 result selects between the two RECT pointers
and two null pointers. Both calls also pass the root HWND and zero.

The two verified message callers forward DefWindowProcA's result for
message 0x466 after drawing:

- `20140d7b` clears a pending byte at `2020de87` and calls the graphics
  receiver's +0x38 slot when that byte was set.
- `2013f3ec` uses a nonnull cached entry and its +0x44 low-byte result
  to decide whether to clear the byte and notify. After drawing, it
  frames the surface in black when the message HWND equals GetFocus.

The source connects both callers to the bitmap body. Their dispatch
registration remains separate. The graphics receiver is reached
through `api.dll` IAT `200e1338`, `g2d.dll` ordinal 4. Only API-side
argument and slot uses were modeled in this package.

Suggested Ghidra names, unapplied:

- `2012c719` `param_1` → `panelContext` and `local_2c` →
  `previousBitmap`, from the context reads and SelectObject restoration.
- `2011ee97` `param_2` → `localBounds`, from the optional RECT reads.
- `2013f3ec` `unaff_ESI` → `panelContext`, from the cached entry,
  HWND and bitmap drawing uses.

## Native result

`api-panel-bitmap-draw-20261109-001` returned exit code zero and passed
20 comparators. Object SHA256 is
`A3B0E0AADFF9C9E45B37353557452FF2EAD7E8B8898000F02A7DEF0CD95580C8`.
Native/original sizes are 526/493 for drawing, 278/278 for graphics
request, 73/68 for bounds and 74/70 and 177/173 for the two callers.
The equal-size request differs outside relocation operands. One source
build used the retained profile. No new bytes enter acceptance. Totals
stay at 5263 complete code and 288 metadata bytes, with 123 partial
Initialize bytes separate. No linked runtime test was performed.
Next is the other graphics-request caller, the paint/clip path at
`2012c588`.
