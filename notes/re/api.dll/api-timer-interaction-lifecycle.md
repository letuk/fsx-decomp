# API mouse capture and repeat lifecycle

Live verified in `api.dll`. Four instruction listings totaling
405 bytes agree with the original PE. API IAT entries `200e19f8`
and `200e19fc` resolve to USER32 SetCapture and ReleaseCapture.
Names and partial layouts are reconstructed.

## Publication

`201378f2` receives the interaction record in ECX, context in EDX,
and flags on the stack. With a record and current input window
(`2020e07c`), it updates capture if either differs from the published
pair. It writes the record at `2020e068` and flags at `2020e06c`,
copies seven DWORDs into `2020d89c`, and remembers the window at
`2020d8bc` and input word from `2020e088`. It sets the capture
countdown and reload to one. It clears `2020e070`, then uses
window +0x98 and that link's HWND at +0 for SetCapture when the
link exists. It ends repeat on every exit, including an unchanged
capture pair and missing record or window.

`20137988` receives record in EAX, flags in ECX and context on
the stack. If the record exists, it saves the flags and seven
context DWORDs, publishes `2020e08c`, copies the input word from
`2020e080`, and sets initial delay ten and reload one. The active
countdown is updated by the 0x465 worker.

## Release

`20137891` and `201379d1` act only on an active record. When flag
0x800 is set and the pointer at +0x28 exists, they dispatch 0x800
through `20136d48` before clearing the respective active global.
The 28-byte local context contains two zero words, owner +0x18,
and the record pointer. The original leaves the last three words
unwritten. Only capture teardown calls ReleaseCapture. The source
preserves the callback-before-clear order and keeps the already
established window-link fields.

Suggested Ghidra names, unapplied. In `201378f2`, `param_2` →
`interactionContext` from the seven-DWORD copy. In `20137988`,
`in_EAX` → `repeatRecord` from the null check and publication
to `2020e08c`.

## Native result

`api-timer-interaction-lifecycle-20261110-001` passed 20 existing
comparators. COFF SHA256 is
`A7007572BE8B231089569B37F60C999590309178F3CB520017C747E11A4BEEB2`.
The four former lifecycle boundaries have source bodies. The two
start functions keep explicit source arguments in place of the
original register ABI. No new accepted byte range or linked runtime
test. Ghidra was used read-only. USER32 remains an import boundary.

Next trace the input dispatcher at `2013754f`, which writes the
current input window and consumes the saved capture window.
