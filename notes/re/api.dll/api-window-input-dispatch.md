# API window input dispatcher

Live verified in `api.dll`. Three listings totaling 721 bytes agree
with the original PE. Live xrefs identify eleven direct calls to
`2013754f`. The observed stack arguments are the source window,
input message and optional interaction. EAX supplies the hit window.
Names and layouts are reconstructed.

The dispatcher rejects missing source, message or window link. It
resets the cursor and hover outputs, publishes source and hit windows
at `2020e07c` and `2020e080`, and selects the captured record when
the source equals the saved capture window. An explicit or captured
record routes through `20136e98` and marks the event handled.
Otherwise it visits the hit window's entries at link +0x2c through
`2013bf93` with visitor `20137248`. WM_NCHITTEST skips repeat and
cursor/hover postprocessing.

`20137a2c` updates the first two words of the repeat context with
coordinates relative to the active record's rectangle when the
hit window matches the saved repeat window. Leaving that rectangle
ends repeat. After a handled hit, `20137a83` can also end repeat
when the hit window differs and the postprocessing gate is clear.
With no hit window the dispatcher checks the saved -1 sentinel
before ending repeat. Both temporary window globals are cleared
before click fallback.

An unhandled left or right button-down can obtain an alternate
target via API IAT `200e1504`, `window.dll` ordinal 23, when source
flag 0x100 is set. A right click calls the context-click helper and
sets the handled result. A left click with WPARAM 1 or 2 activates
the auxiliary HWND or sends application event 0x20007 and keeps the
unhandled result. Finally, if allowed, a changed cursor updates
link +0x20 and posts WM_SETCURSOR. Hover updates run only without an
explicit or selected interaction.

Suggested Ghidra names, unapplied. In `2013754f`, `in_EAX` →
`hitWindow` and `param_1` → `sourceWindow` based on the separate
window-link reads and publication. `param_2` → `inputMessage`
from the message, WPARAM and point fields.

## Native result

`api-window-input-dispatch-20261110-001` passed 20 existing
comparators. COFF SHA256 is
`F0E3C9B293B4A8DC4866D1373733A3CAC6DF9355E587AF5709E287CE2F5F5AA7`.
The dispatcher connects to the reconstructed repeat teardown.
Its original caller bodies and hit, cursor and hover helpers remain
source boundaries. The alternate-target import remains external.
No new byte acceptance, linked runtime test or Ghidra edits.

The follow-up package connects hit visitor `20137248` and
captured-input helper `20136e98`. See api-panel-input-hits.md.
