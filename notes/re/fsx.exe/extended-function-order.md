# Callback, folded SDK leaves and ATL ordering

2026-11-03. Status: original body/vtable evidence, actual XP builds, MAP/linker
warnings, source controls and regression/negative checks. Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Result

The reviewed response now covers22 symbols /21 distinct function entries from
formatter through application code, SDK leaves and the three ATL codepage
routines. Every entry has the same original relative placement, uniformly32
bytes earlier with standard gap0. With optional synthetic gap32, every one of
the22 symbol addresses equals its original address. This is entry placement,
not22 newly matched bodies or proof of original linker command/source names.

Appended original addresses after the prior12-entry prefix:

| Role | Original VA | Standard39 VA |
|---|---|---|
|Current-message callback|01001cf0|01001cd0|
|Run|01001d00|01001ce0|
|ShowErrorMessage|01001d30|01001d10|
|CObject Serialize / Dump (one body)|01001ef0|01001ed0|
|CObject AssertValid|01001f00|01001ee0|
|CObject operator delete wrapper|01001f10|01001ef0|
|ATL fallback getter|01001f1f|01001eff|
|ATL real getter|01001f89|01001f69|
|ATL selecting thunk|01001f92|01001f72|

Original SDK-leaf identities follow actual vtable slots13a0/13a4/13a8 and the
existing OLE delete-cleanup body/import checks; original names are not guessed
from positions alone. The preexisting Serialize/Dump folded pair is retained.

All previous application/CRT/ATL/data/RTTI/header/debug/base/stack/section gates
pass. Known ShowErrorMessage two-byte and InitInstance769/777,246 instructions,
43 nonbranch differences remain. Count18/1114 unchanged.137 import contents pass,
128 import slots still differ. .text virtual9622 versus9654 and .data1204 versus
1184 remain. Entry is now010026dd versus original0100262e (+175): selected prefix
improvement does not certify placement of remaining runtime/library groups.

## Two specific obstacles and isolated controls

1. Actual ApplicationAll/Static trial receives LNK4037 for the file-local
   GetCurrentMessagePilot. The mandatory warning gate rejects that trial.
2. A separately controlled ApplicationPrefix/External trial removes only static
   from the reconstructed callback definition. Its five-byte tail-jump body,
   stdcall signature/MSG* result and MFC import target retain the existing exact
   checks. The original linkage is unknown. This is a reconstruction visibility
   choice to make the callback orderable, not recovery of original externalness.
   No PE export is added: both original and new export directories remain absent.
3. The initial external ApplicationAll trial names Serialize alone. With ICF,
   Serialize/Dump stays at the unordered representative location, breaking the
   requested ordering even without a linker warning. That real trial is rejected.
4. Including both names adjacently restores placement. The verifier allows equal
   addresses only for the specifically reviewed Serialize-then-Dump pair; arbitrary
   duplicates/decreases are still rejected. Existing full76-slot alias checks
   remain active. No ICF disabling or body-mask change.

The corrected all-order build changes only the response data and the single
callback static modifier versus38. Other C++/header hashes and build.cmd are
identical. Compare-tail-order.ps1 checks this explicitly; the order file already
uses /ORDER in both builds. Callback linkage control was built independently
before combining it with the extended response. Canonical src is not edited;
actual compiled staged source is preserved under each candidate/source directory.

## Actual runs and reports

Actual jobs (each suffixed20261103-001): order-all-static-callback (rejected),
callback-external-prefix, order-all-external-callback (folded pair misplaced,
rejected), order-all-folded-pair, order-all-default and
order-all-gap32-diagnostic. All selected positive builds complete the existing
regressions and InitInstance diagnostics; no candidate is executed.

Reports:
- artifacts/fsx.exe/application-order-all-20261103.json
- artifacts/fsx.exe/extended-function-order-20261103.json
- artifacts/fsx.exe/extended-order-negative-controls-20261103.json
- artifacts/fsx.exe/extended-order-gap32-diagnostic-20261103.json

FunctionOrder Auto now resolves ApplicationAll for MfcWindows, Default for console.
CallbackLinkage Auto resolves External for ApplicationAll and Static otherwise.
Explicit ApplicationPrefix/Static restores38 controls. DebugGapProbeBytes remains0.
The unknown original zero-gap cause is not silently converted into production data.

Golden/replay/inventory39-extended-function-order preserves actual order-all-default
source, response/hash/request/receipt, command/log, EXE/PDB/MAP, diagnostics and
comparisons. SHA256:
F649657A9C559DC2B9CF95DC81A8E56F28972968078A34F3A49E8FC8D60FB29B.
38 and all diagnostic/control runs retained; inventory/vtable checks pass39.
Worker ready, no worker/VM UI change. No Ghidra database edit/save/navigation,
api.dll change or new variable-renaming suggestion is justified by this linker
and reconstructed-visibility experiment.

Next: map remaining runtime/library contribution order and relate it to import
member selection. Selected application/ATL placement, import entry order, global
storage layout and unresolved body/codegen differences remain distinct scopes.
