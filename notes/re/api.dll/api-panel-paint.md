# API panel WM_PAINT and clipping

Live verified in `api.dll`. The original PE SHA256 is
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
The complete live ranges at `2012c588` (401 bytes) and `20140d00`
(123 bytes) agree with the raw PE bytes.

## Paint dispatch

The WM_PAINT cases of `20142211` and `2013f6c3` call `20140d00`
at `2014226a` and `2013f709`. The handler receives the panel context
in ESI. It calls controller slot +0x18, then BeginPaint. When the object
at `2020de9c` has a nonzero byte at +0x3d, it sets context +0x42
to one. Otherwise it consumes a set +0x42 without drawing, or calls
`2012c588` with the context on the stack and PAINTSTRUCT.hdc in EDI.
EndPaint runs on all normal branches. The default result is one, and the
draw branch forwards the helper's result. The `20142211` case 0x47c
also sets +0x42 before forwarding the message to DefWindowProcA.

## Clip drawing

`2012c588` ignores GetClipBox's status and returns one if either axis
has equal edges. Flag 0x200 or controller slot +0x30 returning two
forces a black PatBlt (ROP 0x42). Otherwise, with flag 0x40000000 set,
an unshared surface with a bitmap uses `2012d335` and returns one.
Without that flag, the helper returns zero when controller slot +4
is true and a fresh +0x30 call is nonzero, or when slot zero is true.
The latter call is short-circuited. With neither veto and context +0x41
set, it clamps left/top to at least zero and right/bottom to at most
the signed surface extents. It then calls `2011ee97`, which handles
nonpositive extents. All other paths black-fill the original clip and
return one. The graphics update still uses the existing API-side
boundary to `g2d.dll` ordinal 4.

Source uses explicit context and DC arguments and models only the
observed fields. The controller alias and the meaning of +0x3d were
resolved by the sizing analysis in api-main-window-sizing.md.
Suggested Ghidra names (unapplied) are `2012c588` `param_1` →
`panelContext`, `unaff_EDI` → `paintDc`, and `local_18` →
`clipBounds`, based on the caller and GetClipBox. At `20140d00`,
`local_4c` → `paint`, based on BeginPaint and EndPaint.

## Native result

`api-panel-paint-20261109-001` built with the retained profile and
passed 20 existing comparators. Object SHA256 is
`FB14FEDD50B20562C328E6A0C0587272881705BC6887A0675BD2726F44D3EAF0`.
Native/original sizes are 356/401 and 131/123. The object uses different
argument placement and shows the expected controller calls and GDI/User32
references. The original dispatchers were traced as evidence and are
outside the source implementation. No added byte acceptance or linked
runtime test. Ghidra was used read-only.

The follow-up sizing package traces publication at `2012e413` and
both sizing writes to +0x3d.
