# API FS98FLOAT registration and window procedure

Original `api.dll`. The live procedure at `2013e9a8`, five
retained listings totaling 2096 instruction bytes, and 74
registration bytes were checked against the PE. The class
string was verified at `201f7148`.

The registration block at `201221f6` assigns `2013e9a8` as
the procedure with class style 0x4003, the arrow cursor and
stock brush 4. WM_CREATE publishes the creation record as
the SimAWind property. WM_DESTROY removes it.

The move and size routes at `2013f199` and `2013f0d4` update
the floating and child geometry. They also run for the menu
host record obtained through its +0x40 slot. During interactive
sizing, globals `2020e444` and `2020e448` defer both routes.
WM_EXITSIZEMOVE replays size before move and forwards the
notification to the child and the menu host HWND.

The source also handles focus and close forwarding, background
paint, adjusted tracking limits and caption hit testing. The
0x467 refresh selects thick frame or client edge using the
resizability check at `2012b579` and updates visibility when
the controller mode allows it. Messages 0x475 and 0x47a are
forwarded to the child.

Suggested Ghidra names, unapplied. At `2013e9a8`, `local_78`
→ `windowRecord` from the SimAWind lookup, and `local_74` →
`windowLink` for its initial role from record +0x98. The
decompiler later reuses those stack slots for window styles.

`api-float-window-20261111-001` passed all 20 existing
comparators. COFF SHA256 is
`C79F0C10DB7F088E0ED29B73A3CFD8AB917CA54A8AB825D147936CD03D282864`.
No new byte matches were accepted. Ghidra was read-only.
The startup caller at `2012205d`, menu-host table integration
and runtime verification remain open. The surrounding class
registration and startup failure routes are the next API boundary.
