# API FS98MAIN controller lifetime

Target is `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
The constructor, teardown and procedure listings were previously checked
against the PE in api-main-window-sizing.md. This pass also checked the
93-byte registration sequence at `201220fc..20122159` (exclusive) against
the PE and read the constructor strings and unwind records live.

## Construction and release

`20125350` accepts heap storage and HWND as stack arguments. It installs
table `200f2ce0`, sets HWND +4 and initializes the observed fields in two
phases. Between phases it reads `MAIN/Maximized` into `201f94fc` and
calls `2013174d` for `MAIN/ALLOW_REMOTE_SESSION` at +0x40. The signed
Maximized test is `>= 0 || <= 2`, which admits all values. The source
preserves that result. The constructor leaves +0x5c untouched. The
0x6c source storage follows the observed allocation size and offsets.

`2012e506` reinstalls the table, deletes a nonnull snapshot at +0x28,
frees a nonnull owned block at +0x60, clears +0x60/+0x64/+0x68, and
frees the controller from the private heap. Its ESI receiver is copied
back to EAX after free. The table itself remains external.

The constructor's C++ unwind map `201f0614` points to `201ee1c1`,
which passes controller +0x5c in ESI to `201100d4`. That target frees
and clears its +4 member and clears +8/+0xc. The procedure's map
`201f0850` points to `201ee39a`, which frees the pending constructor
allocation through `200f40a9`. Normal construction publishes both
global aliases only after the constructor returns.

## Procedure and registration

`2012e413` first handles WM_NCDESTROY by freeing the old controller and
clearing `2020de9c` and `2020de98`. If the resulting controller is null
and the DWORD at `2020e044` is clear, it can allocate again even on
WM_NCDESTROY. The original allocator receives mode 1, size 0x6c and
the zero-initialize flag from `2020d8ec`. With a controller, the
procedure returns the dispatcher result. With a null controller, it
returns DefWindowProcA for the incoming HWND. This return is evidenced
by the calls and epilogues, superseding the void decompiler signature.

The reconstructed FS98MAIN registration uses the procedure, style 3,
icon resource 0x65, system arrow and stock brush 4. The larger
initialization routine and its error reporting remain external.
The procedure directly routes WM_PAINT, sizing and WM_CLOSE cases, and
forwards the remaining messages to an
explicit `2012f93d` source boundary. No other DLL body was analyzed
or reconstructed for this package.

Suggested Ghidra names, unapplied. In `20125350`, `param_1` →
`controllerStorage` and `param_2` → `window` from the table and
HWND stores. In `2012e506`, `unaff_ESI` → `controller` from
the member cleanup and private-heap free. In `2012e413`,
`param_2` → `message` from the WM_NCDESTROY comparison and dispatch.

## Native result

Both retained-profile builds passed 20 existing comparators. The
first build's ordinary C++ cleanup guards emitted no unwind records.
The adopted `api-main-window-controller-20261109-002` uses explicit
SEH finally blocks. Both scope references and HeapFree cleanup calls
are present. SEH is a source model of the observed unwind actions.
The original C++ exception metadata and runtime unwind behavior
remain outside acceptance.

Object SHA256 is
`AA58ED02DAF9955368E1F814B5F050F6726C6D51FBD84EAB746FF3EF89E57644`.
Coverage remains 5263 complete code and 288 metadata bytes. No linked
runtime test or Ghidra edits. The subsequent package connects WM_CLOSE
at `2012edbb` and the `2020e044` quit flag. See api-main-window-close.md.
