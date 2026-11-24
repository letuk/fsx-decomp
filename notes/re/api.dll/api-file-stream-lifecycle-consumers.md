# API file-stream lifecycle and size consumers

The original `api.dll` SHA256 is
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Findings use original PE bytes, original-verified instruction exports and
explicit live `api.dll` cross-references. The live readback reported
`changed=false` and modification 3. The source names and observed parser
layouts are reconstruction names and minimum layouts.

## Cache registration and application shutdown

Original PE and live verified:

- CRT table slot `200e1ec4` points to `201ee73a`. The 25-byte body calls
  InitializeCriticalSectionAndSpinCount through `200e1760` on lock
  `2020e284` with spin count 100, then registers `201eef1c` with
  `20148c1f`. The 12-byte callback calls DeleteCriticalSection through
  `200e1898`. It only handles the lock lifetime.
- Both paths through the window guard in shutdown callback `20120ed5`
  converge at `20121021`. The unconditional cleanup sequence is
  `2012260f`, `201306ee`, `20143f3d`, `201444c7`, `2013556a` and
  `200f638d`. Thus the file-stream drain runs after parser cleanup and
  before CAB owner drain. The CAB owner drain destroys and frees each
  head through `201448a4`, then calls FDIDestroy `20149bc1` without
  clearing context `2020d8dc`. Dynamic suffix cleanup frees each string
  and the array, clears `2020e00c` and leaves count `2020e008` stored.
- At `2012103f`, shutdown frees the cached application if present,
  decrements the heap initialization count and clears the application
  pointer. On a resulting zero count it attempts HeapDestroy, clears the
  heap handle on success and clears the initialized flag regardless.
  This proves a decrement/zero-test in this shutdown path. A full reference
  ownership model for the count remains open.

The pilot defines zero-initialized stream-cache head and lock storage, an
explicit registration body and a lock callback. The existing Shutdown
wrapper now calls a source body that reproduces the common continuation and
heap tail. The earlier window-dependent prefix and three subsystem calls
(`2012260f`, `200f638d` and the prefix itself) remain source boundaries.
CRT initializer placement and a linked native shutdown remain open.

## Size helper callers and parser cleanup

Verified original instructions, with live xrefs to the listed call sites:

| Entry | Behavior reconstructed in source |
|---|---|
| `20132cbf`, export 73 | Resolves the destination, opens read/write with exclusive sharing and CREATE_ALWAYS, and stores the resulting stream. Resolution failure leaves the caller's output untouched. |
| `201349f0`, export 72 | Creates the destination first. For a source with disk attributes, opens via export 85 mode 2, calls `20133611` with an initially zero low word, ignores the status and passes low/zero-high to CopyTo. If attributes fail, it delegates to the CAB extraction path. Releases destination then source. |
| `2014501e` | Resolves archive owner and relative path, then compares the return from `20144b0e` to the destination in ESI. The result is pointer equality after successful owner resolution. |
| `2014391b` | Opens a read-only disk stream with optional buffering, or opens the CAB fallback directly. Ignores the size helper status and rejects a zero size. Allocates size + 1 on the private heap, reads with an actual-count output at parser +0xc and releases the stream. Terminates the content with a NUL on read success. The allocation remains stored at +4 on read failure. The original allocation retries publish the pointer on each attempt. The source uses the shared allocator and publishes its result when the helper returns. |
| `201435c1` | Releases the two owned words at +0x10/+0x14 in each 0x18-byte section record, then frees and clears parser-owned fields +0x24, +0x28, +0 and +4 in that order. |
| `201306ee` | Visits five parser slots at `2020deec`, releases their contents, frees the records and clears the slots. Resets `2020e004` to zero, `201f755c` to all ones and byte `2020df00` to zero. |

The CopyTo field and target were checked separately. Live original table
fields `200f2c8c` and `200f2d6c` are untyped and both point to
`2012451d`. Ghidra's target has an undefined prototype. Its listing loads
the two stream pointers, forwards size words and count pointers to
`20127084`, and ends in RET 0x18. The source now types slot +0x1c with
the existing six-argument CopyTo implementation, preserving table offsets
and targets. No Ghidra type was changed.

Parser state above the input and teardown paths, including `20143891`
and its line/section handlers, was open at this checkpoint. See
[the API stream parser follow-up](api-stream-parser.md) for reconstruction
of the parser, its cache and query callers. The parser record starts with
the path and content pointers. The highest mapped field begins at +0x28.

Suggested Ghidra names, unapplied:

- `2014391b` `unaff_ESI` → `parser`, based on its path, contents and
  read-count fields and caller `201438a5`.
- `2014391b` `local_11c` → `sizeLow` before increment, since the helper
  writes the end position's low DWORD. The original reuses the stack slot
  for the allocation size after adding one.
- `201435c1` `unaff_EDI` → `parser`, based on the same owned path and
  contents fields and the cleanup caller's EDI load.

## Ordinal 195 transform follow-up

Verified original `util.dll` PE disassembly and C++ EH records. Original
SHA256 is
`A668292DD99BCBEAD89489A0BD4B75B98A376577558C7A1172FB4294CF47576B`.
The core `20f4cf50` is a bit-coded `JM` decoder. Helpers `20f4cc10` and
`20f4cc80` read one and several bits respectively, least significant bit
first. Their four observed DWORDs at +0/+4/+8/+0xc are the cached bits,
cached bit count, next input pointer and remaining input bytes.

After the four-byte prefix the token loop takes either a seven-bit literal,
a seven-bit literal with bit 7 set, or a back-reference. Distance branches
use 6 bits, 8 bits plus 0x40, or 12 bits plus 0x140. Distance 0x113f skips
the copy and continues the loop. For a length prefix of k zero bits
followed by one, k=0 produces 3 bytes. For 1 through 15, the length is
2^k plus k extra bits plus 2. Greater than 15 returns zero. Helper
`20f4cd40` copies forward from output minus distance, allowing overlap.
The output limit is tested between tokens, with no separate span check
inside that helper.

Input exhaustion in both bit readers throws integer 1 through the
`20f6cbee` thunk to msvcr80!_CxxThrowException. The handler at
`20f4d0d5` catches the int into frame offset -0x14 according to the
descriptor at `20f6ddb4`. Its nonzero continuation at `20f4d103` returns
-1. Bad prefix and short input also reach that return. The zero catch
continuation at `20f4d0e7` returns the current output count. These are
static findings. The API retains the original ordinal as a source boundary
pending independent transform verification. No replacement was linked or run.

## Builds and acceptance

| Run | Native object SHA256 |
|---|---|
| api-file-stream-lifecycle-20261109-001 | 193C0AA3E63A413145E7B8240BF3E3865FA7A6C21119C912A792639833538194 |
| api-file-stream-consumers-20261109-001 | C9CE5A2C09B06E8353464C4424A2D0C3418117614D4E44ED1108739DF37B7F46 |

Both runs passed 16 comparators. The new gate checks the two complete cache
registration/cleanup bodies against original bytes, validates their six
relocation fields, resolves both IAT identities and checks the original
initializer pointer. Changed return opcodes and incorrect import targets
reject for both functions. Two initial gate assertions were corrected
against the first retained object: the lock lives inside a shared BSS
section, and the atexit displacement starts at +19. No source rebuild
was needed for those corrections.

Acceptance grows by 37 complete code bytes to 4972, with 288 metadata
bytes and a separate 123 partial Initialize bytes. Shutdown continuation,
CAB drain, suffix teardown and the consumer/parser bodies are outside
acceptance. Native/original byte counts are 118/430 for the partial
shutdown model versus the full original callback, 65/72 CAB drain,
106/105 suffix teardown, 230/209 create, 102/97 CAB path extraction,
570/267 copy, 420/464 parser load, 208/217 parser release and 87/90
parser-slot clear. The native copy body inlines export 85. Each source
slice used one build with the retained compiler profile.
