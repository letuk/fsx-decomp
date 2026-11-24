# simscheduler.dll internal time update at 0x20cc1b90

Live verified in simscheduler.dll. `FUN_20cc1b90` takes the
scheduler object and a double on the stack and returns with
`ret 0xc`. It multiplies the input by the signed value from
VTable slot `+0x1c` (`FUN_20cc1a20`) and the double `1/256`
at `0x20cc12d8`. It adds the result to the stored SimTime
and calls `FUN_20cc30d0` for each context obtained through a
source object's slot `+0x4c`.

The original per-context call passes the context in EAX and
two 32-bit flags and a double on the stack. The callee returns
with `ret 0x10`. The C++ diagnostic instead gives its rebuilt
callee an explicit context argument on the stack. It requires
a rebuilt callee and is ABI-incompatible with the original
binary entry. The complete source and context VTable layouts
and dynamic target sets remain open.

If the context call succeeds for `DAT_20cc8360`, the handler
checks the context's slot `+0x28` and may call slot `+0x00`
on the non-null object in `DAT_20cc8364` with a zero double.
This supersedes the earlier interpretation of `DAT_20cc8364`
as an integer setting. `FUN_20cc1500` still matches after typing
its source input as an opaque observer pointer. Its exact type and
the observer's contract remain open.

The imported API state at `api.dll:0x201f7180` supplies a flag at
`+0x110`. When clear, the handler commits the new SimTime and scaled
step. When set, the original keeps SimTime and clears the scaled step.
It stores the raw input step and dispatches event `0x22` through
`api.dll:0x200f679c`.

The VC8 x86 C++ COFF probe emits 241 bytes against 284 original.
The comparator validates the scale literal, both API imports,
three data operands and the direct context call reference. Stack
layout and x87 scheduling differ as well. Source identifiers and
layouts are reconstructed minimum models. The nine export matches,
group 0x20 callback, linked Flight save and GPS regression passed.

- [Original listing and register ABI](../../../artifacts/simscheduler.dll/advance-time-ghidra.json)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-advance-time-20261116-001/comparison-20cc1b90.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-advance-time-20261116-001/worker-output/disassembly.txt)

The stack-only probe is superseded in the dashboard by the linked
diagnostic. The reconstructed context and substep handlers are in
the [internal time-update area](simscheduler-context-time-update-area.md).
Runtime and full-module link verification are pending.
Ghidra read-only, save state unverified.
