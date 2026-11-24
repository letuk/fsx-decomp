# API window ordering and shared update regions

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Four instruction listings totaling 1150 bytes were checked against the PE,
along with their export ordinals and imported call identities. Live xrefs
confirm the ordering callers and shared-region accesses. Ghidra reported
`changed=false`, modification 3. All source names describe observed roles.

## Window ordering and overlap at 2011d580

Verified original-instruction flow:

1. Return if API quit flag `2020e044` is set, the first graphics lookup
   returns null, no window groups exist, or guard `2020e2d8` is set.
   Set the guard before a fresh graphics lookup and count read.
2. For each enabled graphics entry (slot +0x40 returns nonzero in AL),
   get a parent HWND through entry slot +0x10 and begin a deferred
   position batch with capacity one. The controller's slot +4 selects
   the parent HWND or null as the initial insertion anchor.
3. Create or clear shared region `2020de40`. Visit each group in forward
   order, processing its members first with window flag 0x100 clear,
   then with it set. Only windows with bit 31 set and matching GetParent
   enter the batch. Each DeferWindowPos call receives the previous
   window as insertion anchor and flags 0x13. Its return replaces the
   batch handle, and the current child becomes the next anchor.
4. If window +0x74 is zero and flags 0x4100 are clear, copy accumulated
   coverage into its link region at +0x28. Get the child rectangle, map
   two points from screen to parent coordinates, and use an empty
   rectangle for flag 0x800. Set the shared region to the current
   rectangle and union the saved coverage back into it.
5. End the batch for each eligible graphics entry, then clear the guard.
   Original call results are not separately checked. The shared region
   remains allocated for reuse or final group cleanup.

The API IAT identifies the graphics getter as `g2d.dll` ordinal 4.
Graphics receiver arguments, offsets and consumed return values are
described from the `api.dll` side. The graphics module was outside the
reconstruction scope.

## Connected API paths

| Entry | Observed behavior modeled in source |
|---|---|
| `2011d315` / 28 | Requires nonnull window, flags bit 31 set and a nonnull link. Lazily creates `2020de40`, sets the caller's rectangle and unions into link +0x24. |
| `2011d37f` / 29 | Rejects null or flag 0x1000. Finds the window pointer under its group key, moves it to index zero and invokes `2011d580`, even when already first. Returns zero on a hit, -1 otherwise. |
| `2011cec9` / 20 | Clears `2020e060` on pointer equality, unregisters the window, then destroys both HWNDs and both link regions, frees link +0x44 if present, clears +0x2c/+0x30 and frees and clears window +0x98. If quit is clear and the graphics getter returns nonnull, runs `2011d580`, reacquires the graphics receiver and calls +0x38. |

The same link +0 word is the native HWND for this package and the word
passed to the graphics lookup by `2011ee18`. The source types it as HWND
and explicitly passes its DWORD bits to the existing index helper. The
two shared-region producers reconstructed here share the storage freed by
the previous last-group path. The producer of `2020de44` at `2011e6cb`
and the graphics/window construction paths remain boundaries.

Suggested Ghidra names, unapplied:

- `2011d580` `pvStack_3c` → `deferredPositions` and `pHStack_38` →
  `insertAfter`, based on DeferWindowPos arguments and returned handle.
- `2011d580` `uStack_40` → `flagPass`, from the two passes testing bit
  0x100.
- `2011d315` `param_2` → `updateRect`, from its four coordinates passed
  to SetRectRgn.

## Native result

`api-window-region-order-20261109-001` has receipt exit code zero and object
SHA256 `E4A009ED934095C79DF2F08A5A6C206A3555B509AB2A682B4DA1972099ABB423`.
The initial build passed all 18 prior gates. The final run of 19 gates
accepts all 106 bytes of export 28, including six verified relocation
fields for the shared region and three GDI imports. Changed flag-mask and
incorrect region-import controls reject.

The code ledger now contains 5175 complete code bytes and 288 metadata
bytes, with 123 partial Initialize bytes tracked separately. Ordering,
raise and teardown remain outside acceptance at 640/694, 153/144 and
203/206 native/original bytes. One build used the retained compiler
profile. Native linking and runtime verification remain open. Next is
the related update/overlap consumer at `2011e6cb`.
See [the panel update follow-up](api-panel-update-regions.md) for its
source integration and its direct caller.
