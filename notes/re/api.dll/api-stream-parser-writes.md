# API configuration writes and parser invalidation

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Nineteen instruction exports totaling 5193 bytes were compared against the
pinned PE. The export ordinals, selected import targets, format strings and
live caller references were verified separately. Ghidra reported
`changed=false`, modification 3. This package concerns api.dll.

## Cache invalidation and profile writes

- `20130748` normalizes the path with selector zero twice if the first
  call succeeds, or copies the default configuration path when passed null.
  A failed first resolution can leave the compared key unwritten. It stops
  at the first empty cache slot, compares paths with `_mbsicmp`, frees the
  matching parser and compacts the five-entry array. It resets the
  last-request triple only when its pointer matches the removed parser.
- `20131545` receives section in ECX and key in EDX. Value, path and
  selector are on the stack. Null path selects the default configuration.
  For an explicit path it returns -1 on first resolution failure, otherwise
  repeats resolution with the same selector and ignores the second status.
  It invalidates the parser before calling WritePrivateProfileStringA.
- A failed write checks the read-only bit of the original attribute word.
  If set, the sequence is profile flush, clear the bit, flush, retry once,
  flush, restore the original attribute word with its read-only bit, and
  flush again. The return is zero or -1 based on the write result. The
  original bit test also admits the all-ones GetFileAttributesA failure
  value. Flush and attribute-set results do not override the write result.
- `2013222d` (185) resolves the file, invalidates the parser and calls
  WritePrivateProfileStringA with three null arguments. It maps that
  result to zero or -1. The first resolver status is tested for exactly
  -1. There is no attribute retry on this export.

## Export families

| Operation | Selected path | Default path |
|---|---|---|
| Delete section | `20131d24` (60) | `20130a27` (59) |
| Write string | `201321fb` (58) | `201313e6` (48) |
| Write integer | `20131d4d` (50) | `20130b6b` (40) |
| Write double | `20131fa8` (54) | `20130fd9` (44) |
| Write integer list | `20131dd9` (52) | `20130d15` (42) |
| Write double list | `20132039` (56) | `20131152` (46) |
| Forward unsigned narrow value | — | `20130cdf` (201), `20130cfa` (203) |

Selected writers reject selector 0x27. Numeric variants format before the
selector check, and the list variants allocate, assemble and free their
private-heap buffer even on that rejection. Export 58 uses EAX status.
Its C export's 64-bit return conflates the live EDX key with an additional
return word. The original's common callee returns a single status in EAX.

The original integer and double formats are `%d` at `200f30e4` and
`%f` at `200f30ec`. `200fb1ab` uses EDI output and EAX capacity with
stack varargs. It passes capacity minus one to msvcr80!`_vsnprintf`,
terminates a full or truncated result and distinguishes invalid capacity
from truncation. The selected and default list writers use a 1024-byte
heap buffer and comma separators. The bounded append at `201203f4` and
the element append both receive a remaining-capacity value but start the
terminator scan at the buffer base. The pilot retains that limit, including
its early truncation effect. An empty list still supplies an empty string.

The source uses one write/retry implementation for both path families and
one typed list assembly routine. It keeps the original status, output and
selector behavior. The default WORD and BYTE wrappers zero-extend the
incoming stack value and forward the integer writer's status.

## Native result and acceptance

| Run | Native object SHA256 | Passing gates |
|---|---|---:|
| api-stream-parser-writes-20261109-001 | AAD58BDD78420EA89D4F9F7BA6DA4D74554C87A83A44F69DA799FDFA89D002FF | 16 |
| api-stream-parser-default-writes-20261109-001 | 019E49C7244875DFA6E576A9FEA99A1F41506C744BBC2364C471F786494C1972 | 17 |

Both receipts have exit code zero. The final gate compares the complete
27-byte WORD and BYTE bodies and validates their two REL32 operands against
the original integer writer at `20130b6b` and its defined COFF counterpart.
Changed zero-extension opcodes and incorrect call targets reject. The first
comparator invocation exposed a CommonJS identifier collision, corrected
without changing the retained build.

Acceptance increases by 54 complete code bytes to 5026, with 288 metadata
bytes and 123 partial Initialize bytes tracked separately. The shared
writer, formatter and other wrappers remain outside acceptance. In
particular, flush emits 181/181 bytes but differs outside relocation
operands. Native linking and runtime behavior remain unverified.

Suggested Ghidra names, unapplied:

- `20130748` `local_10c` → `resolvedPath`, from both path preparation branches
  and the cache comparison.
- `20131545` `param_4` → `profilePath` and `param_5` → `suffixSelector`,
  from the resolver arguments and default-path branch.
- `20131dd9` `destination` → `formattedList`, from the append, profile write
  and final heap free.

The next nearby package is the numbered-name helpers and allocated
string variants around `201322e2`, `2013241b` and `20132441`.
