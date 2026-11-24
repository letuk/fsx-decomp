# Application-prefix function ordering

2026-11-03. Status: actual XP /ORDER builds, MAP/body/data/import checks,
verified response input and negative controls. Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Measured result

/ORDER:@function-order-input.h is accepted by the installed SP1 linker with
/GL /LTCG, no ignored-symbol warning in the selected builds. The response is
plain symbol-order data, not included C++ code. Its .h extension lets the existing
worker return/hash the actual used file without changing the worker. It is
generated before source hashing; all original C++/header hashes stay unchanged.
Microsoft describes /ORDER for packaged COMDAT functions using decorated C++
names: [official reference](https://learn.microsoft.com/en-us/cpp/build/reference/order-put-functions-in-order).
Actual VC8 behavior, not modern documentation alone, establishes this result.

Small three-entry trial succeeds. The expanded12-entry prefix has the exact
original relative distances, uniformly32 bytes earlier with the standard gap:

| Original VA | Baseline38 VA | Reconstructed role |
|---|---|---|
|01001590|01001570|Formatter|
|01001600|010015e0|Copy worker|
|01001650|01001630|Message-map getter|
|01001660|01001640|Application constructor|
|010016d0|010016b0|Application deleting destructor|
|01001720|01001700|ExitInstance|
|01001760|01001740|Windows5.1/SP2 helper|
|01001800|010017e0|Windows5.2/SP1 helper|
|010018a0|01001880|Filter deleting destructor|
|010018d0|010018b0|Message-pending override|
|010018e0|010018c0|OLE/filter initialization|
|010019e0|010019c0|InitInstance|

Actual diagnostic with DebugGapProbeBytes32 also passes all prior checks and
places all12 entries at their exact original VAs. This remains synthetic/optional;
it does not recover the original reason for the zero extent. Default gap stays0.
These are entry placement results, not12 newly matched complete bodies: the
known InitInstance body mismatch still exists even at an exact entry address.

All existing18/1114 body gates and CRT/ATL/RTTI/data/header/debug checks retain
their scopes. ShowErrorMessage still two differing bytes; InitInstance769/777,
246 instructions and43 differing nonbranch pairs. Import contents pass but128
import entry slots remain reordered. /ORDER did not solve that separate issue.
Unlisted callback/Run/ShowErrorMessage, SDK and library function order is not
certified by the12-entry response.

## Initial failures and corrected verification

The first three-entry job stopped at the old lifecycle length parser. DUMPBIN
omitted a label for the static current-message callback after the70-byte deleting
destructor; the old parser wrongly included that later JMP when finding the end.
The parser now bounds its scan by the next MAP symbol. It still enforces70 bytes
and all existing body bytes/target fields; no additional masking or length
tolerance. Old37 and the ordered candidate both pass the corrected check.

The first expanded response accidentally used a no-argument decorated name for
OnMessagePending; actual source/MAP has the const MSG* parameter. LNK4037 reported
it as ignored. Corrected to ?OnMessagePending@FSXMessageFilterPilot@@UAEHPBUtagMSG@@@Z.
That initial run is retained as a rejected ordering diagnostic even though body
gates passed. New verify-function-order.ps1 rejects ignored/unresolved option
warnings and requires strictly increasing actual MAP addresses for all response
entries. Real unordered37 and the real bad-symbol-warning job are both rejected.

## Outputs and default

Actual runs: order-first-three, order-first-three-verified,
order-application-prefix (bad symbol), order-app-prefix-verified,
order-prefix-gap32-diagnostic, order-default, all suffixed20261103-001.
Reports:
- artifacts/fsx.exe/application-order-prefix-20261103.json
- artifacts/fsx.exe/application-order-gap32-diagnostic-20261103.json
- artifacts/fsx.exe/application-order-experiment-20261103.json
- artifacts/fsx.exe/application-order-default-20261103.json
- artifacts/fsx.exe/application-order-negative-controls-20261103.json

Default FunctionOrder Auto resolves to ApplicationPrefix for MfcWindows and
Default for ConsolePilot. Explicit Default omits /ORDER; FirstThree retains the
small control. No runtime candidate execution or VM UI/worker change.

Golden/replay/inventory38-application-order preserves actual order-default output
and source/request/receipt/order input/log/EXE/PDB/MAP/diagnostics/comparisons.
SHA256:14413012056017DD01059F7EA9C7D8A80B527243DF1100F9189F49731C5B5469.
37/all trials remain retained. Inventory/vtable checks pass38. Ghidra unchanged;
no save/navigation/type/signature/name edits or new rename suggestions needed.
Original response names are reconstructed source identifiers, not recovered
original decorated class names or proof the original build used /ORDER.

Next: extend placement analysis to unlisted functions and inspect import/data
ordering. Keep synthetic gap controls distinct from standard builds.
