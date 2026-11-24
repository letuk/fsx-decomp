# API window-thread timer work

Live verified in `api.dll`. Three listings totaling 560 instruction
bytes and the 84-byte input table at `200f3188` agree with the original
PE. Ghidra decompilation was checked against the argument pushes
and low-byte return instructions.

## Input reconciliation

The FS98MAIN 0x465 route now calls reconstructed `20137b6b`.
It starts with `20137a9a(0)`. That helper exits when both
`2020e068` and `2020e08c` are null. Otherwise it samples seven
virtual keys through GetAsyncKeyState and applies the table's
alternative mask for swapped left/right buttons. It ends capture
through `20137891` if GetCapture differs from `2020e070`.
For nonzero input it can also end capture when no primary mouse
button is down. For an active second record, flags 0x20000000,
0x40000000 and 0x80000000 require logical left, middle and right
respectively. Missing required input calls `201379d1`.

## Timer actions

The second record waits while `2020d464` is nonzero. Each such
tick decrements the delay and reloads `201f995c`. Afterward,
the tick decrements that countdown and dispatches when its
previous value was zero. It calls `20136d48` with the second
record, flags `2020d890`, context `2020d874` and event source two,
then reloads the countdown.

For the first record, flag 0x1000 at +0x1c enables the countdown at
`201f7120`. A previous value of zero invokes the optional callback
at +0x24 of the record selected by `2020d8a8` and reloads the
countdown from `2020d898`. The callback receives exactly two stack
arguments, context `2020d89c` and value `2020e06c`. The third
argument shown in the decompiler is superseded by the call-site
listing. The tick ends by setting the cursor counter at
`201f7160` to 100.

`20136d48` checks the record's flag intersections before capture
and action dispatch. It can call the record's +0x24 callback
or translate a qualifying +0x20 event code through the existing
application event callback. Its optional repeat-start branch and
the reconciliation code use explicit capture/repeat lifecycle
boundaries. The observed record fields describe a partial layout.
This is API input and callback work scheduled on the window thread.

Suggested Ghidra names, unapplied. In `20137a9a`, `param_1` →
`releaseWithoutButtons` from the gated no-button capture check.
In `20136d48`, `param_3` → `captureFlags` and `param_6` →
`eventSource` from the two callback argument paths.

## Native result

The first build exposed missing XBUTTON constants in the retained
SDK headers. The adopted build uses the verified table literals
0x05 and 0x06. `api-timer-window-work-20261109-002` passed all
20 existing comparators. Object SHA256 is
`72033EEBAC3E11F47E9E84EB75D8208D9885E7731D72B78FAEA86772696A93AC`.
The FS98MAIN route references the emitted worker. No additional
byte acceptance, linked runtime test or Ghidra edits.

The four capture and repeat boundaries now have source bodies.
See api-timer-interaction-lifecycle.md.
