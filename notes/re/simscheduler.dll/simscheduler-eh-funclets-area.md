# simscheduler.dll C++ exception handlers and cleanup funclets

Live verified in simscheduler.dll from `0x20cc702e` up to the CRT
initializer at `0x20cc72b0`. The original PE matches the complete
642-byte Ghidra export, including padding. Ten 27-byte handlers
recover and check the stack cookie, load their FuncInfo address in
EAX, and tail-jump to the CRT import thunk at `0x20cc6f50`.
The ten FuncInfo records reference 19 original cleanup funclets.
Handler owners were verified through relocated prologue operands.

| Handler | Parent addresses | Native COFF handler |
| --- | --- | --- |
| `0x20cc7061` | `0x20cc17d0` | Diagnostic |
| `0x20cc70b4` | `0x20cc27a0` | Diagnostic |
| `0x20cc70e8` | `0x20cc18c0` | Diagnostic |
| `0x20cc7118` | `0x20cc3eb0`, `0x20cc46f0` | Normalized for both |
| `0x20cc7148` | `0x20cc3810`, `0x20cc4380` | Normalized for both |
| `0x20cc717e` | `0x20cc4650` | Diagnostic |
| `0x20cc71ae` | `0x20cc3cf0` | Diagnostic |
| `0x20cc71f9` | `0x20cc1670` | Diagnostic |
| `0x20cc7238` | `0x20cc1cc0` | Diagnostic |
| `0x20cc7284` | `0x20cc2940` | Normalized |

The three normalized matches cover only the handler instructions.
The comparator validates the cookie check call, FuncInfo relocation
for the corresponding owner, and CRT handler jump. FuncInfo and
unwind-map contents are outside the match. For example, the original
pool constructor handler at `0x20cc7118` has one cleanup state and
each retained COFF counterpart has two. The context destructor
handler at `0x20cc7284` matches even though the original cleanup
actions pass their member in ESI and the rebuild uses ECX.
The seven archived COFF diagnostics retain the cookie/frame operand and
instruction-size differences in the best available archived objects.
Full cleanup-funclet matching and runtime exception execution
remain open and depend on the owning function's frame and ABI.

The retained startup conversion and Flight save comparisons were
replayed. Ghidra was inspected read-only and reported no unsaved
changes. The neighboring startup entries beginning at `0x20cc72b0`
already have [scheduler object](simscheduler-object-lifecycle-area.md)
and [pool lifecycle](simscheduler-pool-lifecycle-area.md) coverage.

- [Original Ghidra block](../../../artifacts/simscheduler.dll/eh-funclets-ghidra.json)
- [Handler comparisons and unwind states](../../../artifacts/simscheduler.dll/eh-funclets-comparison.json)

## Linked constructor exception handler at 0x20cc70b4

Live verified in simscheduler.dll. The full 27-byte handler is
Normalized in the VC8 SP1 LTCG internal constructor projection.
The comparator checks the REL32 call to the stack cookie helper,
the HIGHLOW reference to the constructor's FuncInfo, and the
REL32 jump through a thunk that imports
`msvcr80.dll!__CxxFrameHandler3` in both images. The stack cookie
helpers compare ECX against their checked cookie slots and
branch to their respective GS failure targets.

The older COFF handler comparison remains a Diagnostic for that
artifact and is superseded by the linked result for the handler
body. FuncInfo records and cleanup action bodies are outside this
byte match. Runtime exception execution remains unverified.
Ghidra exported the handler with a dry-run disassembly and the
current save state was not verified.

- [Live handler listing](../../../artifacts/simscheduler.dll/context-constructor-handler-ghidra.json)
- [Full linked handler comparison](../../../artifacts/simscheduler.dll/context-constructor-handler-comparison.json)

The linked constructor actions at `0x20cc7090`, `0x20cc709b`,
and `0x20cc70a6` are Normalized over 11, 11 and 14 bytes.
Each loads the context from EBP+8 and passes its member in ESI.
The original jumps to the shared cleanup at `0x20cc28f0`.
The linked build jumps to three typed destructors. Both destinations
check the same buffer, ownership, capacity and count fields and release
storage through api.dll ordinal 108. The destination bodies differ
in code generation. This supersedes the native COFF action diagnostics
for the linked build. Runtime execution remains unverified.

- [Live dry-run listings](../../../artifacts/simscheduler.dll/context-constructor-linked-funclets-ghidra.json)
- [Linked action comparisons](../../../artifacts/simscheduler.dll/context-constructor-linked-funclets-comparison.json)

## Pool constructor cleanup at 0x20cc7110

Live verified for both constructors at `0x20cc3eb0` and
`0x20cc46f0`. The original publishes its registration node and
sets state 0 before the lock creation call. State 0 loads the node
from EBP-0x10 and tail-jumps to `0x20cc3580`, which restores the
base table and unlinks the node. The retained COFF build sets state 0
before publishing the node and state 1 afterwards. An exception
during lock creation unwinds state 1 first, restoring the base table
and unlinking the node, then state 0. State 0 calls the two-argument
placement delete, whose COFF body is a single RET with no relocations.
The extra state therefore adds a call without changing the pool or
freeing storage on that path.

The eight bytes of `0x20cc7110` match both generated registration
actions after validating their REL32 tail jumps. The rebuilt
registration destructor has the same bytes and relocations as the
standalone rebuilt unlink helper. The latter has a diagnostic
comparison against `0x20cc3580`, so the eight-byte result covers
the funclet only. Static behavior at the lock creation call is
verified. Runtime exception execution remains unverified. The
existing source preserves the observed cleanup and was retained.
Ghidra reports no unsaved changes.

## Pool destructor cleanup at 0x20cc7140

Live verified for `0x20cc3810` and `0x20cc4380`. The original and
retained COFF build each have one unwind state, active during block
drain and lock destruction. The original action loads the pool
pointer from EBP+8 and tail-jumps to `0x20cc3580`. The rebuilt action
passes the address of EBP+8 to the guard destructor. The latter
dereferences the slot and operates on the same registration node.
Both restore the base table and unlink the node. The rebuilt guard
references the corresponding base table and list head.

The eight-byte comparison is diagnostic in both owners. Following
verification of the REL32 jump targets, the first opcode differs
(`MOV` versus `LEA`). The existing C++ was retained because the
action and protected calls agree. This verification is static.
Ghidra reports no unsaved changes. The next related actions clear
the one-time guard bits at `0x20cc7170` and `0x20cc71a0`.

## Pool initialization guards

Live verified in simscheduler.dll. The entry pool accessor at
`0x20cc4650` uses the action at `0x20cc7170` to clear bit 0 of
`0x20cc83ec`. The context accessor at `0x20cc3cf0` uses
`0x20cc71a0` to clear bit 0 of `0x20cc83f0`. Both original
unwind maps have one state spanning pool construction and exit
callback registration.

The older C++ probe used catch-all followed by rethrow. The
scope guard now clears the bit during unwind, like the original.
The VC8 SP1 native run `simscheduler-pool-guard-unwind-20261116-001`
returned exit code zero. Both rebuilt accessors have one cleanup
state and no catch table. The comparator validates both protected
calls, the false completion flag in the stack slot on both calls,
the destructor relocation to the corresponding guard, and the
bit mask. The rebuild returns normally with the bit set.

Both 14-byte original actions have diagnostic comparisons. VC8
emits an eight-byte funclet that calls the 13-byte scope-guard
destructor via a tail jump. The comparison covers the static
exception path. Runtime exception execution remains unverified.
Ghidra reports no unsaved changes.

## Context factory cleanup

Live verified in simscheduler.dll. The factory at `0x20cc1670`
sets state 1 while constructing into raw storage and state 2
while registering and publishing the constructed reference.
Both unwind to state 0. State 0 releases the output reference
only if the factory has published it. The publication flag is
armed immediately before the final local-reference release and
cleared before the output cleanup calls the release slot.

| Original action | Verified cleanup |
| --- | --- |
| `0x20cc71d0` | Release the published output reference through `0x20cc1ef0` when armed |
| `0x20cc71e9` | Return unconstructed storage through `0x20cc15f0` |
| `0x20cc71f1` | Release the constructed local reference through `0x20cc1ef0` |

The previous reconstruction used two catch-and-rethrow paths and
omitted the armed output-reference cleanup. The current C++ uses
scope guards and reuses the existing reference guard. The retained
build `simscheduler-context-factory-unwind-20261116-001` emits three
unwind states, transitions `[-1, 0, 0]`, and no catch table. The
comparison verifies the ownership transitions, action entry bodies,
destructor targets and their actual bytes. The factory comparison
is diagnostic at 341 original versus 318 native COFF bytes. Its
three eight-byte COFF actions remain diagnostic against the original
25-, eight-, and eight-byte actions. The heap/storage release helper
still uses its reconstructed ABI. Static verification only. Ghidra
was read-only and reported no unsaved changes.
