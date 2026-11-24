# API controller mode and visibility callbacks

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
The four modeled bodies total 490 original bytes, checked against the
pinned PE using listing exports and bounded disassembly. The original
class string and API-side imports were verified from the PE. Live reads
of `api.dll` reported `changed=false`, modification 3. The field and
method names are reconstructions.

## Mode transitions at 20125b7f

The original receives the controller in ECX, an unsigned mode and a second
stack argument whose low byte is tested for nonzero. It returns with RET 8.
The relevant observed storage is mode at +0x44, signed count at +0x48 and
callback enable byte at +0x38.

| Request | Verified result |
|---|---|
| Mode 0 or unsigned mode above 2 | Exit immediately |
| Enable mode 1 | Increment the count with DWORD arithmetic. Promote stored mode 0 to 1. |
| Disable mode 1 | Decrement only a positive count. Clear stored mode 1 only when that decrement reaches zero. |
| Enable mode 2 | Promote a signed stored mode below 2 to 2. |
| Disable mode 2 | Set stored mode to 1 if the count is nonzero, otherwise 0. |

A count change can occur without a mode change. A changed mode is stored
before callbacks. With +0x38 nonzero, call the controller's +0x18 slot,
then obtain the external graphics receiver from API IAT `200e1338` and
call its slot +0x38. The original ignores the first callback return and
does not null-check the graphics receiver on this notification path.
The pilot retains this order and virtual dispatch.

## Controller slots and visibility

The VTable fields and target bodies were checked separately. Live fields
`200f2cf4`, `200f2cf8` and `200f2d0c` are untyped. Their pointers resolve
to `20125b54`, `20126096` and `20125b7f`, respectively. The first has no
stored function at its entry. The other two have undefined stored
prototypes. The original instructions support the following source
implementations:

- Slot +0x14, `20125b54`, returns true only when byte +0x39 is set, the
  graphics receiver is nonnull and either +0x3a is set or +0x38 is clear.
  A clear +0x39 skips the imported lookup. This call is a guard on the
  API clip-region rebuild at `2011d836`.
- Slot +0x18, `20126096`, obtains the controller HWND's DC with flags 2,
  queries its clip box, releases the DC, and evaluates slot +0x14 before
  deciding whether to enumerate thread windows. Clip-box return 1
  (NULLREGION) leads to enumeration. Every other value, including zero,
  sets visibility true. It stores the result at +0x3b and returns it in AL.
- The EnumThreadWindows callback at `201251ba` compares the returned class
  name case-insensitively with `FS98FLOAT` from `201f7148`. It checks the
  clip box only for a match. The first match with a non-NULLREGION clip
  result sets the caller's integer to 1 and stops enumeration. The original
  does not check the GetClassNameA result before comparing the buffer.

The graphics lookup is a source boundary for `g2d.dll` ordinal 4, verified
from the `api.dll` import table. The graphics target body and VTable were
not inspected in this package. The API clip-region rebuild at `2011d836`
also remains an explicit boundary. The three methods have native source
definitions on the controller interface. Construction, binding to the
original table and final linking remain separate.

Suggested Ghidra names, unapplied:

- `20125b7f` `param_2` → `requestedMode` and `param_3` → `enableMode`,
  from the range and byte tests.
- `20126096` `local_1c` → `clipResultOrFoundWindow` if kept as a single
  stack variable. The slot holds the clip return first and is later
  cleared and passed to the enumerator.
- `201251ba` `param_2` → `foundWindowOutput`, from the successful
  match's integer store.

## Native result

`api-window-controller-mode-20261109-001` returned exit code zero and
passed 18 comparators. Object SHA256 is
`B60007B7642B34251242C7058A921CA873CD097367104C53C9E1652176EEE99B`.
Native/original sizes are 133/130 for the mode setter, 126/147 for the
visibility refresh, 177/170 for the enumerator callback and 43/43 for
the clip guard. The guard differs outside relocation operands.
COFF disassembly confirms the virtual +0x18 and +0x38 calls in the mode
setter. The trial retained the existing profile. No new bytes are
accepted. Totals remain 5069 complete code bytes and 288 metadata bytes,
with 123 partial Initialize bytes tracked separately. No candidate was
linked or run. Next is the API-side clip-region rebuild at `2011d836`.
See [the clip-region follow-up](api-window-clip-regions.md) for its
source integration.
