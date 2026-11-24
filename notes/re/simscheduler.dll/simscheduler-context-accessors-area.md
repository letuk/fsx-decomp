# simscheduler.dll context accessors and mode setters

Live verified in simscheduler.dll. The context VTable at `0x20cc1200`
supplies the following slots. The target bodies, stack cleanup and source
signatures were checked separately. Source names are reconstructed.

| Entry | Slot | Operation | Original / rebuilt bytes | COFF |
| --- | --- | --- | --- | --- |
| `0x20cc2780` | `+0x4c` | Return the context pointer itself | 12 / 12 | Exact |
| `0x20cc2a80` | `+0x08` | Return the name at context `+0x08` | 15 / 15 | Exact |
| `0x20cc2aa0` | `+0x0c` | Set both step bounds | 48 / 48 | Exact |
| `0x20cc2ae0` | `+0x10` | Set minimum step | 45 / 45 | Exact |
| `0x20cc2b20` | `+0x14` | Set maximum step | 45 / 45 | Exact |
| `0x20cc2b60` | `+0x18` | Read minimum step | 15 / 15 | Exact |
| `0x20cc2b80` | `+0x1c` | Read maximum step | 15 / 15 | Exact |
| `0x20cc2ba0` | `+0x20` | Read primary mode as 0 or -1 | 21 / 21 | Exact |
| `0x20cc2bc0` | `+0x24` | Set primary mode and reset substep state | 83 / 83 | Exact |
| `0x20cc2c20` | `+0x28` | Read update bit as 0 or -1 | 21 / 21 | Exact |
| `0x20cc2c40` | `+0x2c` | Set update bit and propagate baseline when set | 50 / 46 | Diagnostic |
| `0x20cc2c80` | `+0x30` | Set second mode bit subject to primary mode | 36 / 36 | Exact |
| `0x20cc2cb0` | `+0x34` | Set third mode bit with its existing-bit guard | 40 / 40 | Exact |
| `0x20cc2fa0` | — | Propagate time to the live entries and context baseline | 72 / 50 | Diagnostic |
| `0x20cc2ff0` | `+0x44` | Write context time and return the output pointer | 20 / 20 | Exact |
| `0x20cc3010` | `+0x48` | Read optional substep outputs when primary mode is set | 53 / 53 | Exact |

The step setters reject an ordered minimum greater than the maximum.
The primary mode setter checks mode bits 1 and 2 and scans the live
entry flags from the end when its argument is zero. A blocked clear
leaves the substep fields untouched. A successful write resets the
fields at `+0x54/+0x5c`. The second mode setter permits a zero argument
or an active primary mode. The third tests the previous third bit and
the primary bit. These are distinct gates.

The update setter passes the context in EDX and a stack double to
`0x20cc2fa0`. ECX is unused on entry to the helper. The rebuilt
fastcall preserves the EDX and stack inputs and initializes the unused
ECX. Its direct call reference was verified. Its stack alignment and
x87 argument-store sequence differ. The helper passes the time to
live entry offset `+0x04` and saves the raw value to the context baseline
at `+0x44`. The original rounds through a temporary double before
the per-entry integer stores. The retained VC8 build stores the value
directly from x87 in the loop and preserves the baseline bits with
`memcpy`. The third build with floating-point exceptions enabled on
the helper instead emitted raw integer copies for both locations.
The earlier void getter at `0x20cc2ff0` used ECX for the output
pointer. The return-pointer reconstruction below supersedes that result.

The default native VC8 SP1 run produced seven exact bodies. The
retained run `002` produced thirteen after scoping fast floating-point
code generation to the two step setters and adjusting the signed
flag outputs and control flow. Run `003` retained those matches and
changed the helper to 52 bytes, with the raw-copy difference above.
Run `002` preserves the observed x87 per-entry conversion and matches
the workspace source hash. Linked image and runtime verification
remain open. The previous twelve native matches and the retained GPS
and Flight save comparisons pass. Ghidra reported no unsaved changes
and was inspected read-only.

- [Original Ghidra listings and VTable](../../../artifacts/simscheduler.dll/context-accessors-ghidra.json)
- [Retained worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-context-accessors-20261116-002/receipt.json)
- [Update setter comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-accessors-20261116-002/comparison-20cc2c40.json)
- [Time propagation comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-accessors-20261116-002/comparison-20cc2fa0.json)
- [Superseded void time getter comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-accessors-20261116-002/comparison-20cc2ff0.json)

Next area is the scheduler object's own accessors and state entries
at `FUN_20cc19a0`, `FUN_20cc19c0`, `FUN_20cc1af0` and
`FUN_20cc1b10`.

Context table slot `+0x40` points to `FUN_20cc2e20`. Its entry cleanup
and reset path is covered in the [context reset area](simscheduler-context-reset-area.md).

The live update setter at `0x20cc2c40` aligns ESP to eight bytes
and passes the context in EDX. The `-ContextUpdateLtcg` profile
produces a 49-byte VC8 linked body against 50 original bytes.
It reproduces the aligned frame and RET 8. It passes the context
in ECX and writes the outgoing double directly through ESP.
The original addresses that outgoing slot through EAX. The call
to `0x20cc2fa0` is mapped to the rebuilt helper. The linked body
remains Diagnostic. Aligned COFF trials emitted 70 bytes with a
larger frame. The trial pragma that regressed two mode setters
was removed. Runtime behavior is unverified. Ghidra was read-only
and its save state was not verified.

- [Linked update setter comparison](../../../artifacts/simscheduler.dll/context-update-setter-comparison.json)

Further VC8 SP1 LTCG trials preserve the live original call at
`0x20cc2c67`, whose sole reported target reference is to
`0x20cc2fa0`. Removing the unused source parameter still emits
the earlier 49-byte setter. A reconstructed by-value time type
with a copy constructor emits the original MOV EAX, ESP and
FSTP [EAX] sequence. Both the two-argument trial and the retained
fastcall form emit 50 bytes. The checked REL32 operand is aligned
at +0x28. Five other bytes differ at +0x09, +0x0c, +0x15, +0x18
and +0x1e, each encoding ECX in place of EDX. This supersedes the
49-byte linked comparison and remains Diagnostic. The layout of
the original source type and runtime behavior remain hypotheses.
Ghidra was read-only and its save state was not verified.

## Linked update setter register follow-up

Live verified in simscheduler.dll. The original helper at
`0x20cc2fa0` keeps the context in EDX. When entries exist it performs
one x87 load and store before traversing the array, then writes the
two resulting words to each entry. The context baseline receives the
original two stack words. Entry ECX is unused. During the loop ECX
holds the array and entry pointers. The single reported direct caller
is the update setter at `0x20cc2c67`.

The earlier reproduction performed the floating-point store in the
loop. VC8 SP1 LTCG gave that helper the context in ECX and used EDX
for the current entry. Moving the same helper into a second LTCG
translation unit produced the same result. Exporting the helper or
compiling it as an ordinary COFF object forced the declared fastcall
registers. Both variants produced a 52-byte setter, including a
two-byte XOR ECX, ECX to supply the unused argument. Their branch
and call displacements account for the remaining layout shifts.

The retained source copies the time value once before a guarded do
loop and copies that representation to each entry. It separately
copies the original stack value to the baseline. LTCG now places
the internal context argument in EDX without initializing ECX.
The full linked setter is Normalized at 50 bytes with one verified
REL32 target. The previous five-byte register diagnostic is
superseded for the linked setter. The paired helper remains
Diagnostic at 72 original / 72 linked bytes. Its count test,
instruction scheduling and loop padding still differ. The compiler's
exact allocation heuristic and the original C++ type remain
unproven. Ghidra was read-only and its save state was not verified.
Runtime execution remains unverified.

- [Original setter and helper evidence](../../../artifacts/simscheduler.dll/context-update-register-abi-ghidra.json)
- [Setter match and boundary experiments](../../../artifacts/simscheduler.dll/context-update-setter-comparison.json)
- [Complete linked helper diagnostic](../../../artifacts/simscheduler.dll/context-propagation-linked-comparison.json)

The linked helper at `0x20cc2fa0` now matches all 72 original
bytes, including the skipped loop alignment instruction. The
reconstruction uses the initial index for the count check and
advances it as it reads the current entry, before the two word
stores. The new result supersedes the previous linked diagnostic.
The caller at `0x20cc2c40` still matches after validating its
REL32 call. Ghidra dry-run listing verified. Save state and
runtime execution were not verified.

## Context time getter return value

Live verified in simscheduler.dll. The relocated VTable slot
`0x20cc1244` (+0x44) points to `0x20cc2ff0`. The body loads
the context from EBP+8, reads a double at +0x44, then loads the
output pointer from EBP+0xc into EAX. It stores through EAX
and leaves EAX as the output pointer at RET 8. The two scheduler
object getters at `0x20cc1ad0` and `0x20cc1af0` use the same
sequence with their own field offsets.

The reconstructed `double*` return type is consistent with the
body and the observed machine ABI. Live Ghidra still decompiles
the function as void and shows a data reference from the VTable
slot. An indirect caller consuming EAX was not verified. Suggested
Ghidra names are `param_1` to `context` for the +0x44 read and
`param_2` to `result` for the output store and EAX value. These
are unapplied suggestions.

VC8 SP1 reproduces all 20 bytes in both the free
`ReadSchedulerContextTime` function and the virtual
`SchedulerContext::ReadTime` method. The bodies have no
relocations. The native class comparison independently verifies
that the generated VTable slot points to the exact method body.
All 25 established accessor-profile matches passed in the new
run and the other 20 native virtual body comparisons are unchanged.
Linked DLL and runtime behavior are unverified. Ghidra was
read-only and its current save state was not verified.

- [Fresh Ghidra listing and slot evidence](../../../artifacts/simscheduler.dll/context-time-getter-ghidra.json)
- [Exact free and virtual body comparisons](../../../artifacts/simscheduler.dll/context-time-getter-comparison.json)
