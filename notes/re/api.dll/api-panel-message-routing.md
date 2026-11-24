# API panel message routing and context lifetime

Live verified in original `api.dll`. Fourteen retained listings and
129 bytes of the main-window registration inventory were checked
against the PE (7962 instruction bytes total). The class name
`FS98DLG3D` was checked at `201f7154`. Names and layouts are reconstructed.

## Window procedures

| Class and procedure | Context lifecycle |
|---|---|
| FS98CHILD `20142031` | WM_NCCREATE takes the window from the creation parameter, writes the HWND to its link and sets the SimAWind property. Kind 3 increments the count at `2020dee4` and registers the surface callback on the first transition. It reuses the context or allocates 0x48 bytes and writes link +0x10 to -1 on that allocation path. Ordinary messages look up the property and dispatch only while the quit flag is clear. WM_NCDESTROY removes the callback at the last transition, unregisters the surface, invokes table slot zero with flag 1, clears the link context and removes the property. |
| FS98DLG3D `2013f2e5` | Message 0x467 takes the window in LPARAM, allocates 0x48 bytes and publishes the context and property. It updates the window flags and surface fields, registers the surface and calls the refresh boundary. Other messages use the property and dispatch when a context exists. |

Both constructor entries `2012b5e7` and `2013f246` install
table `200f30b0` and reach the same observed final field values.
The source shares the allocation and initialization. The original
leaves byte +0x43 unwritten. Slot zero points to `2013f296`.
Its source target clears +0x44, forwards release of a nonzero
graphics handle through the graphics boundary, and frees the context
when bit zero is set. The child procedure retains its virtual call.
The reconstructed FS98DLG3D class registration publishes the atom
at `2020de84`. The encompassing initialization routine and table
placement are separate integration work.

## Dispatch

Both `20142211` and `2013f6c3` now route WM_PAINT and WM_SETCURSOR
to the previously reconstructed handlers. Both route 0x466 to their
respective bitmap handler and 0x478 to the owned-input-packet handler,
which dispatches the packet and releases it from the private heap.
The surface refresh route for 0x46c forwards 0x46c in FS98CHILD and
0x466 in FS98DLG3D. The child 0x47c case sets the skip-next-paint byte
and then delegates. Its ordinary WM_KEYDOWN is forwarded to FS98MAIN.
The dialog uses its own keyboard path and consumes WM_CHAR.

The dispatchers connect their inline pointer paths to the existing
input dispatcher and the 0x407 notification. In the controls branch,
the argument setup for ordinal 46 uses the current pointer transformed
into main-window client coordinates, with X then Y normalized by
the controller bounds. The instruction listing supersedes the
decompiler's mixed-coordinate expressions. The ordinals 45–48
remain an API import boundary to `controls.dll`. No bodies in that
DLL were inspected or reconstructed.

## Boundaries and native result

The dispatcher sources retain named calls for subsystem bodies.
The geometry and drag paths are now connected in the
[panel geometry package](api-panel-geometry-shadow.md). General
pointer translation crosses `20141a0b`, `20141d13`, `2012cb9d`
and `2012d08c`. Surface recreation remains at `2012b9af`.
Keyboard, menu and command bodies are also explicit boundaries.
The source models their call interfaces using explicit arguments
in place of the original register inputs.

Suggested Ghidra names, unapplied. In `20142031`, `local_8` →
`linkOrContextStorage` from the link save and allocation output.
In `20142211` and `2013f6c3`, `in_EAX` → `message` and
`param_2` → `panelContext` from the case tables and member reads.

`api-panel-message-routing-20261110-003` passed 20 existing
comparators. COFF SHA256 is
`FA011959DDAD3FF48F149B9D738F0668E3D9412CBD9242C94A7EE3AF55538ED8`.
Both window procedures, dispatchers, the deletion target and class
registration are emitted. No new byte acceptance, linked runtime
test or Ghidra edits.

The geometry and drag follow-up reaches surface recreation.
General pointer translation remains a separate API package.
