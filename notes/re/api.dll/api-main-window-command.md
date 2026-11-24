# API main-window WM_COMMAND

Live verified in `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Six retained listings totaling 1201 bytes agree with the original PE.
The stored listing for `201205a6` covers only part of its control flow.

## Dispatch and preprocessing

The FS98MAIN dispatcher calls `2012e96f` at `2012fb29` with packed
WPARAM in ECX, controller in EDX and LPARAM on the stack. The high
byte is passed separately to callback `2020e030`. The remaining
24 bits go to the callback and default procedure. When bits 16–23
are zero or one, the low word goes to `2012fded` in ECX with the
controller retained in ESI. This prefilter consumes a command on a
nonzero result. Otherwise the application callback can consume it
with a nonzero result. Either consumed path returns zero. The last
path returns DefWindowProcA's result. The source omits the private
scratch initialization because the caller never reads that buffer
and the prefilter only writes its first byte.

| Command | Verified prefilter behavior |
|---|---|
| 0x1103 | If Shift is down, calls export 11 at `2012e130`. Continues ordinary command dispatch either way. Export 11 requests a 1024×720 client rectangle when controller slot +4 is false and always posts WM_SIZE. |
| 0x110a | If installed, calls `2020e030` with event 0x100c4, original LPARAM and zero. Consumes the command even when the callback is absent or returns zero. The prefilter itself performs no quit-flag write. |
| 0x1601, 0x1603, 0x1604 | Loads resources 0xf60c, 0xf60b and 0xf60d respectively into shared scratch `2020d770`, dispatches list `2020bb70` with event 400 and that pointer, and consumes the command. |
| 0x166a | Loads resource 0xf609 into a 0x824-byte buffer and clears shared scratch. Calls `201205a6` with (0, 0x102, scratch, 0x104). If the result is nonnegative and scratch is nonempty, appends it using the first terminator within the buffer and the remaining capacity. Calls controller slot +0x64 and consumes the command. |

For 0x166a, live table field `200f2d44` resolves to `2012629e`.
The caller passes six stack arguments, and that target ends in RET
0x18. The target may send WM_COMMAND 0x1501 with LPARAM 0x50524f47,
checks the supplied HWND, substitutes its own if invalid, and calls
ShellExecuteA. The source keeps the virtual call, resource loader,
callback list and product-text query as explicit boundaries.

The 0x110a producers at `2012dfbd` and `20122ae1` can set the quit
flag before posting. This is separate from the prefilter's event
translation. See api-main-window-close.md.

Suggested Ghidra names, unapplied. In `2012e96f`, `param_1` →
`packedCommand` and `param_3` → `commandData` from the masks and
callback arguments. In `2012fded`, `local_834` → `launchTarget`
from the resource load, append and slot call.

## Native result

`api-main-window-command-20261109-001` passed 20 existing comparators.
Object SHA256 is
`EF70845B15140922BF41749F2141EC74C0F4A36459218A1AAD8E0199288818B7`.
The handler, prefilter and size request are emitted. The window
procedure's WM_COMMAND branch calls the source handler. No added
byte acceptance, linked runtime test or Ghidra edits.

The follow-up package implements the product formatter and the
+0x64 shell-launch target. See api-main-window-product-text.md.
