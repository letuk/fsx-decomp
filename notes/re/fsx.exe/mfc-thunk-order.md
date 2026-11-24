# MFC import thunk placement

Follow-up: crt-selection-order.md adopts this response plus controlled CRT entry
INCLUDE as default40. Statements below about retaining default39 describe the
isolated initial MFC experiment, not the current selection.

Status: live verified, 2026-11-03. Binary: original/fsx.exe, SHA256
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

The original range 0x01001ffc..0x0100230d contains 66 six-byte FF25 MFC
import thunks, before pre_cpp_init at 0x01002313. Each was matched by its exact
IAT import identity to a unique rebuilt thunk and public MAP symbol. The complete
executable-section scan finds 80 such thunks in each binary. MAP provenance
identifies the 66 as MFC import-library members. This does not establish the
original object-selection sequence or original linker command.

## Controlled build

Optional FunctionOrder MfcThunks appends these 66 decorated symbols, in original
address order, to the reviewed 22-symbol ApplicationAll response. The checked-in
manifest is scripts/matching/mfc-thunk-order.json. CallbackLinkage Auto resolves
External for this mode, as for ApplicationAll; original callback linkage remains
unknown. No other C++ or header input changes versus baseline39, and build.cmd
is identical. The response remains hashed and checked against worker-returned
compiled inputs. Default Auto remains ApplicationAll; baseline39 is retained.

Actual XP worker job:
vm-share/modules/fsx/runs/order-mfc-thunks-20261103/candidates/order-mfc-thunks-20261103-001.
EXE SHA256: 6F2DE3881ECE606C1A352A061CBA322DC484AB14523B5D97FC06F38821CE2A70.

All 88 ordered symbols are accepted without ignored-order linker warnings and
have verified actual MAP ordering. All 66 MFC thunks now have a uniform -32-byte
offset from their original addresses; the existing 22-symbol prefix placements
are unchanged. compare-mfc-thunk-order.ps1 independently checks each original
and rebuilt FF25 opcode and its exact imported target, source hashes, command,
prefix and manifest. Six-byte thunk bodies are covered; surrounding padding is
not certified by that check. Evidence:

- artifacts/fsx.exe/runtime-order-inventory-20261103.json (baseline39 inventory)
- artifacts/fsx.exe/mfc-thunk-order-20261103.json (controlled placement/body report)
- job comparison.json, build-log.txt, source-hashes.json, MAP and worker receipt

All existing body/reference/header/data/import gates and InitInstance diagnostics
pass. Import contents still match, but 128 within-DLL slots still differ. Entry
remains 0x010026dd versus original 0x0100262e. Thus ordering executable import
thunks does not reorder the IAT in this experiment. ShowErrorMessage's two-byte
difference and InitInstance's 769/777-byte and 43 nonbranch-instruction diagnostic
differences remain. No counted application-body coverage increase is claimed for
these library thunks. Synthetic gap probe remains disabled.

## Next work and state

Map the remaining CRT public/static contributions and test their ordering
eligibility separately. In particular, do not infer that file-local CRT startup
functions are orderable merely because import thunks are accepted. Investigate
library-member selection and IAT entry order independently. The remaining CRT
placement and writable-data layout are still open.

No Ghidra edits, save, navigation, names/types or api.dll changes occurred.
No original variable-name inference follows from this linker experiment. The
worker compiles and inspects binaries only; no candidate EXE was executed and
no VM UI/autostart settings were changed. Database backup/save state is unchanged.
