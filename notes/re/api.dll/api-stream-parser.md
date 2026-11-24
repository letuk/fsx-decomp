# API stream-backed configuration parser

The target is original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Thirteen exported instruction listings, totaling 2245 bytes, were checked
against that PE. Live `api.dll` references confirm `20130410` → `20143891`
and the latter's tokenizer, section and key calls. The live program reported
`changed=false` and modification 3. Source identifiers describe observed
roles. No other binary was analyzed or reconstructed in this package.

## Tokenization and index construction

Original-instruction verified:

- `20143891` receives the parser in EAX and uses the existing input loader
  `2014391b`. It walks from contents +4 to contents plus byte count +0xc,
  stopping early at NUL. The counter at +0x10 increments once per tokenizer
  call. Consecutive CR/LF bytes can be coalesced in one iteration.
- `20143aeb` splits the writable input in place. CR and LF end a line even
  inside quotes. Outside double quotes, a semicolon or two slashes start a
  comment. The first equals encountered after a key token splits the key;
  an equals encountered after the value token has started clears both outputs
  and skips the line, including inside quotes. A value starting with a double
  quote advances past it and strips the last quote if present. The NUL exit
  performs quote handling without the line-ending whitespace trim.
  `20143ebb` skips to the next line and `20143eff` trims from the delimiter
  using CharNextA/CharPrevA. The outputs borrow the original input allocation.
- `20143c7a` requires at least three bytes and a final closing bracket on
  the header token. A new section opens a key table with stride 8. A duplicate
  selects the existing section and returns zero, so subsequent keys are
  skipped until another section is accepted. A malformed header likewise
  suppresses subsequent keys. `20143d21` inserts a key only if it is absent,
  including when the prior entry's value pointer is null.
- `20143dcc` searches case-insensitively using `_mbsicmp`. A hit returns
  the sorted rank and a miss returns the complement of the insertion rank.
  `20143e41` grows both arrays by 50 entries when full, shifts only the
  sorted index, and appends a record in insertion order. Both reallocation
  statuses are ignored. Record lookup uses a DWORD stride product and
  clamps a negative signed product to byte offset zero, as in the listing.

The observed 0x2c-byte parser allocation at `201305c2` contains a mutable
buffer pointer +4, byte count +0xc, line counter +0x10, selected section
pointer +0x14 and the section index at +0x18. The index has count, capacity,
stride, sort-order pointer and record-array pointer at relative offsets
0/4/8/0xc/0x10. A 0x18-byte section record begins with the borrowed name and
embeds a key index at +4. An 8-byte key record contains two borrowed pointers.
Teardown at `201435c1` frees each section's two arrays, then the outer index
arrays, path and shared contents. This refines the prior allocation-only
field names in the pilot.

## Five-entry cache and query callers

`20130410` first tests the previous request path at `2020df00`, selector at
`201f755c` and cached parser pointer at `2020e004`. The request comparison
is case-sensitive and null denotes an empty request string for this check.
Null selects the default configuration path at `2020d248`. A nonnull path
is resolved twice with the same selector if the first call succeeds. The
first failure still falls through to the cache with a potentially unwritten
stack buffer. The pilot carries this failure output as unspecified.

The five parser slots at `2020deec` compare resolved paths with
`_mbsicmp`. A hit moves its entry to the front. A miss uses the first
empty slot or evicts slot four, shifts the prefix, calls
WritePrivateProfileStringA with three null arguments and the resolved
path, allocates 0x2c bytes, then duplicates the path and loads the parser.
The parser is published even when its input load returns zero. A null
allocation is published to slot zero without updating the last-request
triple. The original unwind funclet `201edcf1` calls `200f40a9` on raw
parser storage. Source placement-delete expresses that action. The fixed
profile's native object has no corresponding cleanup relocation, so emitted
unwind equivalence remains open.

`20143d6d` selects a section, reusing the current one on a name match.
A miss leaves the current selection stored. `2014369a` reads a value.
Missing section/key leaves the output untouched. A present null value
writes a leading NUL and returns zero. A present string calls `20137e17`
and returns the lesser of string length and supplied capacity.
Exports 61 and 180 (`2013086a`, `20130889`) acquire the default
configuration parser and check section or section/key existence.

Unapplied Ghidra naming suggestions:

- `20143891` `local_8` → `acceptSectionValues`, from the section-handler
  return and the guard on subsequent key insertion.
- `20143aeb` `param_1` → `keyOutput`, `param_2` → `valueOutput`, from
  the ordered output stores into the input buffer.
- `20143e41` `unaff_EDI` → `insertionRank`, from the index shift and
  insertion store. `unaff_ESI` → `nameIndex`, from the count, stride and
  two allocation fields accessed through it.

## Native builds and stop point

| Run | Native object SHA256 |
|---|---|
| api-stream-parser-20261109-001 | 3D4360320D0671EB70D3A438112257FFE30C627187810494D899861677137 |
| api-stream-parser-cache-20261109-001 | 2DA3BC8F89A60C58695D9712576E634B7DE43687232BC3C3E366CF64135AA02C |

Both builds have receipt exit code zero and 16 passing gates. COFF inspection
confirms the acquisition call chain into load and cleanup and confirms the
missing unwind relocation. Native/original sizes include load 182/138,
tokenizer 421/399, section 167/167, index search 132/117, insert 128/122,
acquire 577/734, value read 138/170 and key existence 66/66. Both equal-size
bodies differ outside relocation operands. No new bytes are accepted.
The ledger retains 4972 complete code bytes and 288 metadata bytes with
123 partial Initialize bytes separate. No linked runtime test was performed.
The next parser slice is the enumeration and typed-value callers around
`20131708`, `2013174d` and `201318cd`.
See [the configuration query follow-up](api-stream-parser-queries.md) for
their source integration.
