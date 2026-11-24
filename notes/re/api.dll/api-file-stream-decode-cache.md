# API file stream decoder and cache

## Evidence and scope

The target is original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Instruction-level findings use the original-verified ASM exports under
`artifacts/api.dll/api-source-pass-20261107/functions`. All 1623 listed bytes across
the eight reconstructed bodies were checked against the original PE. Live Ghidra xrefs confirm the
export-85 decoder caller and the cache critical section references. Live
readback for `api.dll` reported `changed=false`, modification 3. Ghidra reads
were read-only. Source identifiers and the cache record layout are reconstructions.

## Decoder at 20132e20

Verified in original instructions. Stream slot +0xc must return zero and
report at least 0xbc bytes. Offsets below are relative to that probe.

| Check or value | Observed rule |
|---|---|
| DWORD +0 | Reject 0x19920201 |
| DWORD +0x7a | Require bit 31 |
| DWORD +0x76 | 0x87654321 selects prefix size 0xa8, otherwise 0x80 |
| DWORD at prefix size | Require 0xabcdef01 |
| Next DWORD | Expected transform output size |
| Encoded payload | Starts at prefix size +0x14, or 0xbc / 0x94 |

Helper `20133611` rejects null input or null output with -1 and performs
no output store in that case. With both arguments it seeks current, end
and back to the saved 64-bit position through slot +0x14. The first two
statuses are ignored. The restore status is returned, and the end low DWORD
is stored even if restore fails. A seek that leaves its position unwritten
leaves the consumed stack value unspecified. The original instructions
supersede the inconsistent operand recovery in the exported C for this helper.

After helper success and rejection of the low-DWORD sentinel 0xffffffff,
the decoder allocates the input buffer from the private heap, seeks to zero
and rereads through `201333cd` with a null byte-count output. It ignores the
seek status. It allocates prefix plus expected output bytes, copies the
prefix from the reread buffer, and invokes the transform with source
pointer, stream size minus payload offset, destination pointer and expected
output size. Arithmetic uses DWORD wraparound. The instructions do not
show extra size or short-read checks at this stage.

On a transform result equal to the expected size, the original frees the
encoded buffer through `20135f51` and returns the result of
`20127440(-1, 0x04000000, decoded, totalBytes, NULL, 0)` directly. A
constructed wrapper owns the supplied private-heap buffer. A null wrapper
result also returns directly, leaving the decoded allocation without cleanup
on that path. The normal failure tail rewinds a nonnull input and frees
decoded then encoded buffers if present. The source reuses the existing
private-heap retry/fatal path with zero-initialize disabled for both allocations.

## Transform boundary

Verified in original PE disassembly. The `api.dll` IAT slot `200e148c`
imports `util.dll` ordinal 195. The original `util.dll` SHA256 is
`A668292DD99BCBEAD89489A0BD4B75B98A376577558C7A1172FB4294CF47576B`.
The export at `20f4d270` uses RET 0x10. It forwards the four arguments
through `20f4d1b0` to `20f4cf50`, where the encoded input is checked for
length greater than four and initial bytes 0x4a 0x4d. The bit reader starts
after the first four bytes. The output count is measured by pointer
subtraction. The core also has explicit zero and -1 exits. The destination
limit is compared at the loop boundary. This establishes the API-facing
argument order and return interpretation. Complete bit-reader, back-reference
and exception semantics remain open. The pilot retains the ordinal transform
as a source boundary.

## Independent file-stream cache

Verified in original `api.dll` instructions. The recursive critical section
at `2020e284` protects the list at `2020d8cc`. The observed 0x114-byte
allocation contains next +0, hit count +4, last-use DWORD +8, stream +0xc
and path storage +0x10. The use value comes from `2012de9b`. Its exact
clock domain remains open.

- `20144003` calls resolver `20133d17` with selector zero and compares
  paths with `lstrcmpiA`. The resolver status is ignored. A hit increments
  the count, refreshes last use and moves the entry to the head. If an
  output slot is supplied, Clone at +0x34 receives it directly. The cache
  returns one independently of Clone's status. A miss returns zero.
- `20143f76` rejects a null stream or null/empty path. Under the lock it
  rechecks with a null Clone output. A duplicate returns zero after the
  normal hit updates. A miss prunes, allocates through `200f4068` using
  global `2020d8ec`, and links if allocation succeeds. The miss returns
  one even if allocation returns null.
- `201440cc` saves each next pointer before any stream calls. Candidates
  have unsigned elapsed use value greater than 300000 or one-based rank
  greater than five. It probes the reference count with AddRef then Release.
  Only a Release result of one triggers detachment and private-heap free.
- `20144152` resolves the stored key, ignores resolver status, retains the
  stream, initializes hit count and last use, and links at the head under
  the lock. The original returns the entry pointer.
- `201441a4` releases and clears the stream before entering the recursive
  critical section and unlinking. `20143f3d` drains the cache through this
  same path. Both are represented in source.

Original PE disassembly and live lock xrefs identify registration at
`201ee73a` with spin count 100 and the `201eef1c` critical-section deletion
callback. The source keeps head and lock storage external. Startup registration
and the teardown caller at `2012102b` were integration boundaries at this
checkpoint. See [the lifecycle and consumer follow-up](api-file-stream-lifecycle-consumers.md)
for their source wiring and the transform core findings. The
cache functions' source signatures use stack arguments where the originals
use registers. No native linking or runtime test was performed.

## Builds

Each source slice used one targeted trial. Both receipts have exit code 0
and all 15 comparators passed.

| Run | Native object SHA256 |
|---|---|
| api-file-stream-decode-20261109-001 | 55A5DB89BAF9A71741040B02D4CD8BA2754BFFFCCC98E0F7B672D685153BC345 |
| api-file-stream-cache-20261109-001 | 707A9C57C2A94CF73E6C19611FBAC7E173C480428FD4B17BF84A123A7828CF2E |

COFF section and relocation inspection confirms the source call targets,
including export-85's decoder and cache boundaries in the same translation
unit and the unresolved four-argument transform from the decoder. Sizes
are native/original bytes: decoder 528/791, size helper 104/114, lookup
204/201, remember 132/141, prune 135/134, link 103/82, detach 89/103 and
drain 50/57. These bodies remain outside acceptance. The ledger stays at
4935 complete code bytes and 288 metadata bytes, with 123 partial Initialize
bytes tracked separately.

Suggested Ghidra names, unapplied:

- `20132e20` `local_114` → `probeBytesRead`, initialized to zero and filled
  by the probe Read before comparison with 0xbc.
- `20133611` `param_2` → `sizeLowOutput`, destination of the single
  low-DWORD store after restore.
- `20144003` `puVar2` → `cacheEntry`, the current record supplying
  the comparison key and cached stream.
