# API CAB owner metadata and entry attributes

Original `api.dll` SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Instruction evidence is in `artifacts/api.dll/api-source-pass-20261107/functions`.
Caller xrefs and VTable slot xrefs were checked live at saved Ghidra revision
`1793741866691:2a9d629f:3`. This package made no database edits.

## Stat and reload contract

`20135378` calls stream VTable slot +0x30 with flag 1. It returns -1 without
writing any caller outputs on a null stream or nonzero Stat result. On success
it optionally copies creation, access and modification from Stat offsets
+0x18, +0x20 and +0x10 in that order. The existing source already follows
this contract. Compile-time checks now pin those three offsets.

| Reload path after successful path resolution | Observed result |
|---|---|
| Open fails at `20144a49` | Null stream. The helper leaves the modification pair unwritten. |
| Open succeeds, wrapper creation returns null at `20144a66` | File Stat `20124408` via `200f2ca0`. It clears the Stat buffer first. A size error with nonzero last error or a GetFileTime failure yields E_FAIL. The helper leaves the caller pair unwritten on E_FAIL. |
| Open and wrapping succeed | Wrapper Stat `2012701c` via `200f2d80`. It forwards to file Stat, overwrites only the low size word, and returns zero regardless of the underlying result. The helper therefore copies the modification pair from the forwarded Stat buffer, including whatever the file call supplied on failure. |

Memory-only wrapper Stat clears all 72 bytes and returns success with zero
timestamps. It is a separate branch from the wrapper created for metadata in
`201449c4`, which receives the opened file stream. With flag 1 the file
method skips name allocation. A nonzero Stat result in the file method and
success in the wrapper describe different layers.

Reload still stores the helper's output unconditionally at owner offsets
+8/+0xc. It can also reach the unwritten stream slot after path-resolution
failure as recorded in `api-cab-buffer-containers.md`. Neither a zero
timestamp policy nor a previous timestamp policy follows from those stores.
The explicit source reload boundary remains. These are static control-flow
findings. The three Stat bodies were already built and measured
156/143, 202/198 and 96/98 original/native bytes. Their established matching
stop point remains in effect.

## Entry lookup and attribute fallback

Original `20144b9b` uses EDI for the owner, ECX for the relative path and one
stack argument for the attribute output. It records the input length before
the bounded copy, uppercases the copy and checks the owner +0x120 cached
entry first. The table walk advances by 0x11 plus the stored name length.
An exact match writes 0x81 and caches the entry. A longer matching path
with a backslash at the requested boundary writes 0x11 without replacing
the cached exact entry. A miss writes FFFFFFFF and returns null. Lookup
returns on the first qualifying record.

`2014507f` resolves the owner and returns 0x11 for the archive root even
when the entry count is zero. A non-root path requires a nonzero count and
returns the lookup's attribute output. Live xrefs show calls from
`2013341b` at `20133480` and `20133497`. The source now connects both
existing fallback calls. The actual source field rename from `unknown120`
to `lastExactEntry` preserves the observed offset.

Suggested Ghidra names are `20144b9b` `unaff_EDI` → `cabOwner` and
`param_2` → `attributesOutput` based on the owner reads and output stores.
At `20135378` the three output parameters `param_2` to `param_4` can be
named `createdTimeOutput`, `accessedTimeOutput`, `modifiedTimeOutput`.
These suggestions were left unapplied.

Run `api-cab-owner-metadata-20261109-001` in `vm-share/modules/api/runs`
retains native object SHA256
`115D72FF22B67C6BBCB5773784300606EDE40F39903B270A9490C2810352C37B`.
Lookup emits 519 versus 450 original bytes and the fallback 141 versus 145.
The lookup's original register contract and the fallback's frame and lookup
call differ. Stop after the initial source build. All 15 gates pass and
coverage remains 4935 complete code bytes and 288 metadata bytes. The next
bounded package is the entry-open and size consumers at `20144b0e` and
`20144f5f`, through shared FDI dispatch `20144924` and its cleanup.

Follow-up in `api-cab-extraction.md` supersedes the size-consumer label.
Both paths produce an output stream.
