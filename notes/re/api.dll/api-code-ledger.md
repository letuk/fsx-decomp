# API code evidence ledger

Panel memory DC acquire at 2011d05c adds 88 complete code bytes.
The 56-byte release at 2011d0b4 is verified again and counted once
under generated coverage. Twenty gates pass at 5263 code and 288
metadata on api-panel-pixel-transfer-20261109-001.
See api-panel-pixel-transfer.md.

Update rectangle union at 2011d315 adds 106 complete code bytes with six
validated relocation fields. Nineteen gates pass at 5175 code and 288
metadata bytes on api-window-region-order-20261109-001. Ordering, raise
and teardown remain outside acceptance. See api-window-region-order.md.

Private-heap slot release at 20135f51 adds 43 complete code bytes with
two validated relocation fields. Eighteen gates pass at 5069 code and
288 metadata bytes on api-numbered-names-20261109-001. The numbered-name
bodies remain outside acceptance. See api-numbered-names.md.

Configuration WORD/BYTE writer wrappers at 20130cdf and 20130cfa add 54
complete code bytes with two validated call relocations. Seventeen gates
pass at 5026 code and 288 metadata bytes on
api-stream-parser-default-writes-20261109-001. The shared integer writer
is outside acceptance. See api-stream-parser-writes.md.

File-stream cache registration at 201ee73a (25 bytes) and lock cleanup at
201eef1c (12 bytes) add 37 complete code bytes with six validated relocation
fields. Sixteen gates pass at 4972 code and 288 metadata bytes on
api-file-stream-consumers-20261109-001. The shutdown continuation and
consumer bodies remain outside acceptance. See api-file-stream-lifecycle-consumers.md.

File stream selection at 20133137 builds in api-file-stream-open-20261109-001.
The native body differs and stays outside acceptance. Fifteen gates retain
4935 code and 288 metadata bytes. See api-file-stream-open.md.

CAB extraction and the Initialize FDI open callback build in
api-cab-extraction-20261109-001. Fifteen gates retain 4935 complete code bytes
and 288 metadata bytes. Equal FDI open callback size does not pass body
comparison. See api-cab-extraction.md.

CAB owner metadata and attribute fallback build in
api-cab-owner-metadata-20261109-001. Fifteen gates retain 4935 complete code
bytes and 288 metadata bytes. Lookup and fallback are outside body
acceptance. See api-cab-owner-metadata.md.

Cache registration at 201ee724 adds 17 complete code bytes with three
validated relocations. Fifteen gates pass at 4935 code and 288 metadata
bytes on api-cab-buffer-containers-20261109-003. The array helpers and
cleanup bodies are source models outside acceptance. See
api-cab-buffer-containers.md.

Stream tables add86 code bytes and112 metadata bytes. Fourteen gates pass
at4918/288 on api-cab-stream-tables-20261109-002, object
D03BF4BF68A5E4DAC65FD976F4434CF51EB5B6621E97B2440F75E6D7653FE2F8.
See api-cab-stream-tables.md. Shared AddRef is counted only in prior coverage.

Pool registration adds11 raw bodies97 bytes and three validated table spans
132 metadata bytes. Totals are4832 code and176 metadata. Thirteen gates pass
on api-cab-pool-registration-eh-20261109-001. Object SHA256 is
5963240F0CA49217C9A582FA7300575BF127C6A557DDFFD97935C0D60FF82A65.
See api-cab-pool-registration.md for exact method and table scope.

Pool acquisition regression passes twelve gates on api-cab-pool-acquire-20261109-001.
Object SHA256 is C6853690C32B58622F0C58DF0281E69402248C4CE92AA8BC5270B5C2F30EBD8F.
Coverage totals4735/44. Pool-specific acquisition source is outside acceptance.

Lock and owner lifecycle regression passes twelve gates in
api-cab-pool-lifecycle-20261109-001. Object SHA256 is
745B32639E00D5197CAB0A3FDA50665837E6130D135022805B553FC864BD8C68.
Coverage totals4735/44. Twelve source entries remain outside body acceptance.

Slot transfer run api-cab-slot-transfer-20261109-001 passes twelve gates.
Object SHA256 is 9737DF04E4216E68EFA2F3A20A11F9172A06C02BF74E11E4799B8C8F30B32423.
Coverage totals4735/44. Shared transfer source remains outside body acceptance.

Slot storage regression passes twelve gates on api-cab-slot-storage-20261109-001.
Object SHA256 is 30E9ED6F804642CABF97DB3EE3614B6F23FC6E536A2A0FB255AAACBB23723899.
Coverage totals4735/44. Eight new source bodies are outside acceptance.

Shared pool regression on api-cab-pool-blocks-20261109-001 passes twelve gates.
Object SHA256 is 1A5F0D795C6909063A2AE22E4C550597C2FE65A9BC5648703147814B46109D73.
Coverage totals 4735 code bytes and 44 metadata bytes. Six source entry points
use shared helpers and remain outside body acceptance. See api-cab-pool-locks.md.

2026-11-09 buffer cache regression. Twelve gates pass on
api-cab-buffer-cache-20261109-001, object
5F7437CE23C42E31F7DF0B3F6358885C754075884627B6487F61AD6868F4EE93.
Complete code totals4735 bytes and metadata44 bytes. Three cache bodies compile
unmatched. See api-cab-buffer-pools.md.

2026-11-09 method extension: file read20124356/57, file seek201243c8/64 and
wrapper seek20126fc0/92 add213 disjoint complete bytes: total4735 code/44 metadata.
Comparator compare-api-cab-stream-methods.cjs; two validated import fields,
wrapper seek raw-equal, rejecting controls and generated overlap checks.
Twelve gates pass on api-cab-wrapper-seek-20261109-001, object
9CB0E58935446FFFFFAB04852ADA021233228460FD2281A1229F96FE28C4BDC1.
See api-cab-stream-methods.md. Other three new methods remain unmatched.

Stream-producer regression: eleven gates pass on api-cab-stream-producers-20261108-001,
object86DAD616F17054FFD3B5A3F8F82CD327E890951BC9D90AB3454330C0875F23C6.
Complete total4522 unchanged; four producer/lifetime source bodies unmatched.
See api-cab-stream-producers.md for original ABI and emitted-EH limitations.

2026-11-08 CAB stream extension: release201333ac33 plus read201333cd39 are full
raw-equal disjoint bodies, comparator compare-api-cab-stream-wrappers.cjs.
Current complete total4450+72=4522; metadata44 and partial Initialize123 separate.
Eleven gates pass on api-cab-stream-read-20261108-001, object
8B8242DC8FC42E93712BAB7273610AC5E035BF3517FB4B226E7D49E570AB02F5.
Image/seek/stat source remains unmatched; owner reload remains unresolved.
See api-cab-image.md. Older totals below are historical subsets/snapshots.

CAB cache follow-up: ten gates pass on api-cab-cache-20261108-001, object
7A26A93498D37DFA991F25318E7645FC0CCF89CBDB513B78C1B19C439A2EE424.
Total4450 unchanged; four additional source bodies unmatched, native resolver
unwind missing. See api-cab-owner-cache.md for the scoped source/EH distinction.

2026-11-08 regression: all ten gates pass on api-alternate-enumeration-20261108-001,
object C69A9A7863DB89A348ADAB3AA0F4930949A94293195A27133D2CCF560F73572E.
Total4450 unchanged; three new CAB backend source bodies remain unmatched.
See api-alternate-enumeration.md for sizes, contracts and stop decision.

2026-11-07 extension: latest run api-source-message-loop-20261107-001 retains
all1120 bytes in the historical table below and adds88 disjoint generated model
bodies/3116 bytes plus foreground-idle hook20122a0b/214 bytes. New combined
complete-body evidence is4450 bytes; metadata44 and Initialize partial123 remain
separate. Fixed generated address/size list:
scripts/matching/api-source-pass-accepted.json. Full reference maps:
source-pass-comparison.json in the run. Comparator for the hook:
scripts/compare-api-foreground-idle-hook.cjs. Ten gates pass. See api-source-pass.md
for object hash, raw-versus-normalized scope and incomplete module integration.
The historical1120 total below is the retained old subset, not the current total.

2026-11-03. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Addresses are original VAs. Names are reconstructed, not original symbol proof.
All rows below are complete object-body gates; references are individually mapped
where applicable. No full linked DLL/runtime claim or whole-image percentage.

| Original address | Working name/scope | Complete bytes | Comparator |
|---|---|---:|---|
|20120442|GetApplication|72|compare-api-lifecycle-object.cjs|
|20120535|GetWindowHandle|21|compare-api-lifecycle-object.cjs; separate DLL gate also retained|
|2012052a|Shutdown outer method|11|compare-api-lifecycle-object.cjs|
|20122817|RunMessageLoop outer method|29|compare-api-lifecycle-object.cjs|
|2013e3c1 plus detached EH|SEH callback wrapper,90+47|137|compare-api-lifecycle-object.cjs|
|2014474c|FDI Read|33|compare-api-lifecycle-object.cjs|
|2014476d|FDI Write|33|compare-api-lifecycle-object.cjs|
|2014478e|FDI Close|23|compare-api-lifecycle-object.cjs|
|201447a5|FDI Seek|51|compare-api-lifecycle-object.cjs|
|20132c8e|Path existence wrapper|49|compare-api-path-wrapper.cjs|
|201230da|Initialization error wrapper|48|compare-api-initialization-error.cjs|
|2013266e|Lazy disk-space API dispatch|139|compare-api-disk-dispatch.cjs|
|2013b6a4|Splash stop/thread handle cleanup|98|compare-api-startup-stop.cjs|
|20137ec0|Bounded path append|62|compare-api-path-string-helpers.cjs|
|20137fb0|NULL-aware path length|34|compare-api-path-string-helpers.cjs|
|20122834|Main-window keyboard hook|188|compare-api-main-keyboard-hook.cjs|
|20134929|File enumeration close|92|compare-api-font-enumeration-close.cjs|
|Total disjoint complete code||1120||

Metadata44 bytes (EH28 plus VTable16) is separate. Initialize2012048a partial
123 bytes is separate and not added to complete-body total. Linked window21
overlaps the object row and is counted only once.

Latest shared regression snapshot:
vm-share/modules/api/runs/api-font-resources-20261103-001,
object SHA256CEB1F80139323AC6D7D2C2E2A14ECD70732F985C1FE8B89BA136E6DF379BCDB1.
All eight accepted-body comparison scripts passed there, including their negative
controls. The separate main-child-class comparator rejects its near match.
Detailed scope/hashes/source trials remain in linked topic and per-run JSON files.

Arithmetic correction: earlier progress reports claiming648 after adding139 to
507 were wrong; correct total646. Adding98 gives744, not746. The underlying
body gates and byte ranges are unchanged; this supersedes historical totals.

## Built but unmatched, not added

| Original address | Original bytes / latest native bytes | Main unresolved issue |
|---|---|---|
|2012048a|160/158|Custom owner/register scheduling;123 scoped bytes separate|
|2013341b|202/202|Frame/operand scheduling|
|20133d17|258 stored-body cardinality/238|Custom source boundaries; full original extent not inferred from count|
|200f79d5|86/82|Original register ABI versus source stack ABI|
|20134fc4|182/251|Custom ABI and copy inlining|
|201325f1|125/125|Multiply operand schedule; stopped on identical output|
|20133e19|318/363|Copy inlining/custom boundary and frame/scheduling|
|2013b5c1|227/225|Prologue/shrink-wrapped register save and frame/scheduling; structured source|
|2013b444|381/354|Window-thread source/initialization scheduling|
|2013b1c1|475/620|Structured procedure model; custom rectangle boundary/dispatch scheduling|
|2013afdd|445/488|Bitmap loader source, custom copy/trim interfaces and inlining|
|20121360|100/100|Only frame reservation byte differs28h/2ch; no full-match acceptance|
|201214ad|336/309|Monitor callback frame alignment/copy/scheduling|
|201215fd|409/407|Monitor selector frame/local/register scheduling|
|20121796|585/575|Custom ECX monitor ABI versus source stack boundary|
|201213c4|233/233|Custom EDX formatter boundary and scheduling|
|2013e64f|729 native flow+128 descriptors /357 source|Normal-flow source model only; integrity/formatter implementation open|
|20121083|733/891|Font lifecycle; custom trim/copy and enumeration dependencies; stopped at1|

These are selected recent slices, not an inventory of every original function or
all earlier unmatched heap helpers. See api-initialization-flow.md and roadmap.
