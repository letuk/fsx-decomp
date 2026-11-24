# API CAB buffer cache containers and owner reload

Evidence is from the pinned original `api.dll` (SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`),
the original-verified listings in `artifacts/api.dll/api-source-pass-20261107/functions`,
and read-only live Ghidra checks. Live revision
`1793741866691:2a9d629f:3` was saved with no pending changes. This package
made no Ghidra edits.

| Original address | Verified behavior and source model |
|---|---|
| `2020e1a8` | Observed cache layout spans 0x40 bytes. Lock at +4, live and cached arrays at +0x1c and +0x2c, retention limit at +0x3c. |
| `2020e1c4`, `2020e1d4` | Both containers have signed count and capacity, signed growth step, flags and data pointer at offsets 0/4/8/0xa/0xc. Live entries are eight bytes, cached entries four. |
| `2011468d`, `2011369a` | Unsigned address lower bound with optional insertion slot. Tracking rejects duplicates. Cached search at `20127dd3` instead dereferences each allocation's size header. |
| `20114798`, `20127fa8`, `2011cbcc`, `20127f43` | Append and insertion clamp the resize request. Insertion shifts the suffix. Original callers proceed after resize failure. The source retains that behavior. |
| `20118423`, `2011448e` | Typed resize models the 8-byte and 4-byte variants. A positive request uses step rounding or flag-4 doubling. Allocation failure leaves existing storage in place. |
| `201274a6`, `20127513` | Initialize spin count 100, growth step 5, flags 4 and limit INT_MAX. Teardown drains both collections under the cache lock, then performs the owned-storage fallback and deletes the lock. |
| `200f2d4c` → `20127d78` | Original table points to a deleting destructor with ECX self and a stack flag. The pilot table represents the same operation with an explicit stack self parameter. |
| `201ee724` → `201eeeed` | Startup body initializes the cache and registers its cleanup with `_atexit` at `20148c1f`. The original cleanup loads ECX with the singleton and jumps to `20127513`. |

Allocation and return at `20127642` and `2012772b` now use these typed
containers directly. The compiled sizes are 219 and 291 bytes against
233 and 305 original bytes. Cache size search is 75 versus 74 bytes.
The cache startup entry is explicit in source. Final CRT placement, native
helper ABI equivalence, the destructor slot, and runtime remain open.

## Owner reload boundary

Live xrefs for `201449c4` show one direct call, at `20144896` in
`2014483b`. The constructor ignores its return. The resolver at `201452e1`
checks archive discovery at `20145331` but does not test its selector-0
normalization result from `2014534c` before it can construct an owner at
`201453bb`. Reload then independently resolves selector 0x34, whose inline
suffix at `200eff38` is `.CAB`. An already matching suffix bypasses append,
while the copy and canonicalization can still report failure.

The branch at `20144a30` reads the unwritten frame slot `[EBP-0x114]` at
`20144a87`. Both possible stores to that slot follow successful resolution.
Separately, `20135378` leaves the time pair unwritten for a null stream or
failed Stat. The reload caller at `20144a99` copies the pair into owner
offsets +8/+0xc regardless of return. For example, a failed open at
`20144a49` passes a null stream to Stat. This is a static control-flow
result. Actual execution of the resolver-failure branch through a discovered
archive remains unverified. The source keeps the explicit reload boundary
pending evidence for those outputs.

Suggested Ghidra names are `2011468d` `param_1` → `keyAddressInput` and
`param_2` → `insertionEntryOutput`, based on the indirect key read and
optional slot store. At `201449c4`, `local_118` → `metadataStreamSlot`
remains provisional due to the unwritten path. These are suggestions.

## Native result and stop point

Run `api-cab-buffer-containers-20261109-003` retains the native object
SHA256 `4E0186F3B211249EA9B94B062E1A243228CE46B5058AA415A9D6DF332C6F4683`.
The first run failed because the pilot SDK headers hid the spin-count
declaration. The second built with the original import declaration, and the
third corrected negative resize input handling after checking EBX in the
original listing. No build varied compiler flags or layout.

The registration comparison accepts the full 17-byte body at `201ee724`
with three verified target fields. Opcode, return, and wrong callback
mutations reject. Its callees and callback are outside that acceptance.
Fifteen gates pass at 4935 complete code bytes and 288 metadata bytes.
Initialize's partial 123 bytes stay separate. Stop cache shape trials here.
The next bounded CAB question is the owner metadata read and its failure
outputs, starting at `20135378` and the two Stat targets.

Follow-up in `api-cab-owner-metadata.md` distinguishes raw file Stat failure
from wrapper success and connects the independent entry attribute fallback.
