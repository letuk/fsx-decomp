# API cursor events and pointer queries

Live verified in `api.dll`. Seven retained instruction listings totaling
1792 bytes agree with the original PE. The hit-test mapping was read from
the PE at `2011fb90` and `2011fbb0`. Names and field views are reconstructed.

## Message routes and cursor precedence

The main dispatcher `2012f93d` sends WM_SETCURSOR to `2012f031`
at `2012fa75`. Panel dispatchers `20142211` and `2013f6c3`
both reach `20141f43` at `20142eb2` and `2013f80c`.
The panel handler uses `2012caa7` and delegates to DefWindowProcA
if that helper declines. Both paths record the screen position at
`2020e668` and `2020e66c`, refreshing the inactivity counter to 100
on movement. The timer and idle paths already share that counter.

- The panel handler checks cursor suppression first, then API IAT
  `200e1224` (`controls.dll` ordinal 45). A nonzero result selects
  system cursor 0x7f03. Otherwise an eligible cursor at link +0x20
  takes precedence over the controller's hit-test fallback.
- The main handler selects 0x7f02 when the override gate is set.
  Otherwise it uses the controller's mode at +0x44 and state byte
  +0x38 to decide whether to hide the cursor, return handled without
  setting one, or use the hit-test fallback and default procedure.
- The shared fallback `2011fb07` (export 230) requires controller
  slots +4 and +0x20. The low word of LPARAM selects the resource.
  Hits 10/11 map to 0x7f84, 12/15 to 0x7f85, 13/17 to 0x7f82 and
  14/16 to 0x7f83. Other hits use 0x7f00. It calls the existing
  cursor loader and cache, which rechecks its own mode conditions.

## Override and polling

`2012e1dd` (export 12) increments or decrements the signed depth
at `2020dee8`. A positive depth sets the shared gate `2020e03c`
and shows the override cursor. Otherwise it clears the gate,
calls ShowCursor(FALSE), then sets the normal cursor or null based
on suppression. Each call reaches ShowCursor and leaves the
SetCursor result in EAX. The timer reads the same gate.

`20137c26` can report the HWND under the pointer, screen and
client coordinates, and key state. It writes the HWND and screen
position before attempting client conversion. The key sample reuses
the same seven entries as the timer input reconciler.
`20137cf8` (export 253) translates those key bits, optionally
converts to the supplied panel's client coordinates and returns
0x30001 for a different HWND under the pointer or 0x30002 for
suppression on the matching window.

Suggested Ghidra names, unapplied. In `20141f43`, `in_EAX` →
`panelContext` and `unaff_EDI` → `cursorMessageData` from
the forwarding calls. At `2020dee8`, `cursorOverrideDepth`
describes the add/subtract and positive-depth test.

## Integration and package boundary

The source FS98MAIN procedure calls the reconstructed handler.
The shared panel handler is implemented and both original routes
are verified. Their larger dispatchers still have no source callers
for that handler. The message routes consume the cursor choice
produced by the existing hit and hover code. USER32 calls and the
controls ordinal remain external. No other DLL body was analyzed.

`api-cursor-events-20261110-001` passed all 20 existing comparators.
COFF SHA256 is
`5F662E08B3740C7817D4D8A5897E4078D4DA21DC186617B98AE6A4F1A4C0E99C`.
The original register-passed receivers use explicit source arguments.
No additional byte acceptance, linked runtime test or Ghidra edits.

The follow-up panel routing package connects `20142211` and
`2013f6c3` to the cursor handler. See api-panel-message-routing.md.
If following the external cursor override, track
`controls.dll` ordinal 45 and its writers in a separate module package.
