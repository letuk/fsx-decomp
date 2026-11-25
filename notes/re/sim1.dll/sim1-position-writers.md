# sim1.dll world velocity, longitude and altitude

The concrete position and attitude step at `0x20bdec10` calls the
world-velocity writer at `0x20bdec67`, the altitude writer at
`0x20bded3e` and the longitude writer at `0x20bded6e`. The original
listings show the flight object in ESI at all three helper entries.
The vector call supplies 24 stack bytes and returns with `ret 0x18`.
Each coordinate setter supplies an eight-byte stack argument and
returns with `ret 0x8`. The source names, C++ types and minimum layouts
are reconstructions.

| Original VA | Live verified behavior |
| --- | --- |
| `0x20bd1d90` | Copies the three transformed velocity components into `+0x140`, `+0x148` and `+0x150`, then stores the horizontal magnitude `sqrt(x*x + z*z)` at `+0x1ec`. The original direct call goes through `0x20c66fc2` to `MSVCR80.dll!_CIsqrt`. |
| `0x20bd1bd0` | Adjusts longitude by one full turn where required and sets radians, cosine and sine at `+0x120`, `+0x128` and `+0x130`. Converts to fixed point in `+0x20c`. Conditionally publishes its low and high words at shared-state offsets `+0x160` and `+0x164`. |
| `0x20bd1c90` | Limits altitude above to 100,000,000 feet and stores it at `+0x138`. Converts to a fixed-point metric value at `+0x214`. Conditionally publishes its two words at shared-state offsets `+0x168` and `+0x16c`. Subtracts a virtual reference-height result and stores the difference at `+0x1fc`. |

The original IAT cell at `0x20bb11fc` is `api.dll` ordinal 304.
Both coordinate writers use this imported address as the base of the
shared-data writes. The linked comparison resolves the same ordinal
in its own PE import directory.

The coordinate writers scale to a double and use `FISTP qword` with
the active x87 rounding direction. They then shift the resulting bits
left by 16. Longitude uses `2^48 / (2*pi)`, and altitude uses
`0.3048 * 65536`. Build 001 established that VC8 emitted a truncating
`__ftol2` call for the attempted 64-bit cast, including with
`/QIfist`. The selected reconstruction uses a separate rounding
helper that reads the floating-point control word and implements
the four rounding directions, including nearest with ties to even.
It preserves the resulting shifted integer for the masked-invalid
case. Matching the original FP exception behavior remains open.

The position-publishing check loads slot `+0x50` from the flight
object's table, passes the object in ECX and tests its EAX return.
The altitude writer loads another object from flight offset `+0x14`,
calls that object's slot `+0x08` with the object in ECX and uses the
x87 return. These slot positions and call-site ABIs are verified
independently of the dynamic target identities. A later constructor
trace resolves the altitude call to `0x20bceec0` through `0x20bbd940`
for the installed ground-reference object. The flight object's
position-publishing target identities remain open.
The source uses minimum table projections for these two calls so it
can preserve the slots without assigning signatures to unreviewed
virtual methods. The established C++ heading-transform interface
remains shared with the acceleration and velocity stages.

## Match results

| Scope | Original bytes | Rebuilt bytes | Result |
| --- | ---: | ---: | --- |
| `0x20bd1d90` world-velocity writer | 86 | 86 | Normalized linked match. The one REL32 field targets the independently verified CRT square-root import through its thunk. |
| `0x20bd1bd0` longitude writer | 186 | 176 | Different in the linked body. Conversion, IAT reload and surrounding register and stack layout differ. |
| `0x20bd1c12` longitude radians and trigonometric cache | 31 | 31 | Normalized in the linked DLL. Both relative calls are verified against the original CRT cosine and sine imports. |
| `0x20bd1c90` altitude writer | 159 | 147 | Different in the linked body. Conversion and publication differ, and the reference call uses a different call register. |
| `0x20bd1d0c` altitude reference query and subtraction | 22 | 22 | Exact in the native COFF build. The range ends before the result store. |

The selected linked run uses VC8 RTM `14.00.50727.42`, `/fp:precise`
and LTCG. The reconstruction-only caller supplies already calculated
geographic values and invokes the reviewed heading-transform slot and
the three position helpers. Under LTCG the compiler passes the
instance in ESI. In linked run 003 the velocity helper had the original
size and two residual stack-operand byte differences. Run 004 changed
the order of terms in the horizontal-speed source expression and the
emitted loads now agree with the original. The selected 86-byte result
is a whole-function comparison. The DLL has no entry point and the
linker reports a CRT initializer warning. It was used for comparison.

Native run 005 compiles the same canonical source. It independently
checks the original state-import and trig references and retains the
22-byte exact altitude scope. The native velocity writer receives
the instance in ECX and measures 90 bytes. The native longitude and
altitude bodies measure 186 and 157 bytes. The final linked callers
reuse the API IAT load for the two words of each coordinate. The
original reloads the IAT for the second word.

Integration regression run 006 rebuilt the earlier code using the
shared layout header and retained all three exact transform bodies
and the 144-byte exact acceleration-history copy. The known whole-body
acceleration and integration differences remain. Runtime behavior and
the complete position and attitude caller at `0x20bdec10` remain open.
The next small entries in that caller are the latitude writer and
the attitude-angle setters.

Suggested Ghidra names are `param_1` → `longitudeRadians` in
`FUN_20bd1bd0` and `param_1` → `altitudeFeet` in `FUN_20bd1c90`,
based on the trigonometric and metric conversions. In `FUN_20bd1d90`
the six decompiler words correspond to three stack doubles, and the
ESI value is the flight object. These are suggestions only.

The tags were applied in Ghidra and read back after saving.
Revision `1795528515497:962445d7:41` reported `unsaved_changes=false`.
The earlier complete function inventory is a snapshot before these
tag edits.

- [Position source](../../../src/sim1/position_attitude.cpp)
- [Original Ghidra call sites, state and tags](../../../artifacts/sim1.dll/position-writers-ghidra.json)
- [Selected linked comparison](../../../vm-share/modules/sim1/runs/sim1-position-20261124-004/position-comparison.json)
- [Selected native COFF comparison](../../../vm-share/modules/sim1/runs/sim1-position-20261124-005/position-comparison.json)
- [Integration regression](../../../vm-share/modules/sim1/runs/sim1-state-integration-20261124-006/state-integration-comparison.json)
