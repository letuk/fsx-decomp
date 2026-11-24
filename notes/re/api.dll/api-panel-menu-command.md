# API panel menu commands and context clicks

Original `api.dll`. Live decompilation and six instruction listings
totaling 1296 bytes were checked against the PE. The command jump
table at `201419b0` and selector bytes at `201419b8` were checked
separately. Source names are reconstructed.

`20141853` receives the full WPARAM in EAX, context in EDX and
LPARAM in ESI. It routes 0x201 and 0x202 to main window events
with the panel pointer, toggles the option backing 0x300 according
to controller mode, and invokes callback-list event 400 for 0x500.
The 0x7801–0x7eff range passes the command to the supplied callback
when a menu handle exists. The selector table confirms the eight
mode commands that share the event path with 0x400. Other commands
fall through to DefWindowProcA with the original arguments.

`2012d08c` is connected to the existing input dispatcher and
owned packet route. It checks the context click gate, converts
signed mouse coordinates for messages carrying screen positions,
records a nonzero group with those coordinates and sends the
main window events around menu construction and display.
The call at `2012d278` reaches `2012b637`. The display wrapper
at `2013ad98` dispatches through menu interface slot +0x8c with
the panel, menu handle, coordinates and panel HWND. Host position
uses the HWND returned by slot +0x44.

The 0x2000 alternate route at `2012cfc5` and the host creation
paths at `2013a534` and `2013a5be` are explicit source boundaries.
Both creation entries take the HWND in EDI and cross into
SimXUIWindow_Create. The API IAT at `200e1520` identifies
`window.dll` ordinal 4. External module bodies were outside
this package. Next is menu-host creation and its callbacks.

Suggested Ghidra names, unapplied. At `20141853`, `in_EAX`
→ `command` from the range check and jump table. At `2012d08c`,
`local_c` → `menuPoint` from ScreenToClient and the display call.

`api-panel-menu-command-20261110-002` passed all 20 existing
comparators. COFF SHA256 is
`CB1490880CB1DE8DDF800FB5C08296CF2835CDA38E30A63BB9915F77BBDD1A28`.
Accepted coverage remains 5263 code and 288 metadata bytes.
Linking and runtime verification remain open. Ghidra was read-only.
