# Read, seek and stat methods used by CAB streams

2026-11-09. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Selected original VTable targets: file200f2c70 -> read20124356, seek201243c8,
stat20124408; wrapper200f2d50 -> read20126dcf, seek20126fc0, stat2012701c.
Six exported listings861 bytes reverified against original in
artifacts/api.dll/api-cab-stream-methods-evidence-20261109.json, with VTable/IAT identities.
Live state/signatures checked in api.dll, revision1793741866691:2a9d629f:3,
unsaved_changes=false. No DB changes/tab switching. Observed source types/names
are reconstructed, not original classes or complete interfaces.

## Contracts and preserved behavior

File read20124356: four stack args/RET16. NULL read-count output is replaced
with local storage; ReadFile success maps0 and failure80004005. A successful
short read is not converted to failure. Complete57-byte native match, one IAT
field at offset35 independently validated as200e18d0/kernel32.dll!ReadFile.

File seek201243c8: five stack words/RET20. Ignores high offset, SetFilePointer
uses low offset and NULL high pointer. ReturnFFFFFFFF always maps80004005,
without consulting GetLastError. Optional position output gets low result/high0.
Complete64-byte native match, one IAT field at offset24 independently validated
as200e16c8/kernel32.dll!SetFilePointer.

File stat20124408: three args/RET12. Zero72-byte output, type2, GetFileSize
low/high with GetLastError disambiguation, GetFileTime created/accessed/modified,
then mode at28. Optional name allocated through CoTaskMemAlloc and converted
to wide text. Allocation/conversion failure does not itself change return status.
Observed stat fields now named in source; unknown bytes remain neutral.

Wrapper read20126dcf: four args/RET16. Accumulate available buffered bytes, then
for flag20000000 or request>2*capacity directly seek/read underlying; otherwise
refill aligned window. Return negative read status immediately, ignoring seeks;
partial/end read returns0. Arithmetic stays32-bit, including window-end/2*capacity
overflow and low32 stream positioning. No extra EOF, zero-progress, NULL-stream
or zero-capacity protection is invented. In particular flag20000000 still selects
direct underlying calls after remaining request is zeroed; do not hoist a new
early return over that branch. A successful zero-byte refill can repeat when
length/request still require progress; runtime behavior was not exercised.

Wrapper seek20126fc0: five words/RET20, high offset ignored. SET/CUR/END use
modulo32 addition then signed range tests. Negative result clamps0; positive
result beyond signed length fails80004005. Optional output high word0. Initial
source92 differed only comparison operand order/direction. One assembly-supported
source revision position>length produces complete raw-equal92 bytes. Stop on
success after2 trials; no further variations.

Wrapper stat2012701c: three args/RET12. Underlying stat return is ignored,
size LOW word overwritten by wrapper length while high word is left as returned,
and method returns0. Without underlying: zero72, type3, sizeLow valid-buffer
bytes, mode ~(flags*2)&2. Do not substitute wrapper length for this branch.

## Source field reconciliation

Read/seek now establish wrapper+0xc as bufferStart, +0x10 as bufferValidBytes,
and +0x28 as current position. These supersede the earlier provisional source
names unknown0c/position/unknown28, respectively. Source rename was actually
applied in cab_stream_records.h and producer initialization, preserving offsets
and machine behavior. The earlier topic's description of +0x10 as position is
superseded. Ghidra types/names were NOT changed. Minimum observed storage48 and
file storage276 are now shared in a header; stat buffer remains observed72.

## Builds, coverage and limits

New source cab_stream_methods.cpp includes six structured methods, no goto.
Initial run api-cab-stream-methods-20261109-001 passes eleven prior gates.
Final comparison-order revision: api-cab-wrapper-seek-20261109-001, object
9CB0E58935446FFFFFAB04852ADA021233228460FD2281A1229F96FE28C4BDC1.
All runs under vm-share/modules/api/runs; receipts/source/logs/diagnostics retained.

compare-api-cab-stream-methods.cjs checks full57+64+92 bodies, two specific
DIR32/IAT mappings, opcode/RET/IAT rejecting controls and generated-body overlap.
Twelve gates pass without uninitialized/missing-return warnings.213 new disjoint
complete bytes increase4522 to4735; metadata44 and partial Initialize123 separate.
Matched indirect method does not certify its interface, data or callees.

| Routine | Original/native bytes | Status |
|---|---:|---|
|File read20124356|57/57|complete target-validated match, first build|
|File seek201243c8|64/64|complete target-validated match, first build|
|File stat20124408|202/198|unmatched, frame/scheduling|
|Wrapper read20126dcf|350/325|unmatched, frame/local/control scheduling|
|Wrapper seek20126fc0|92/92|complete raw match, second trial|
|Wrapper stat2012701c|96/98|unmatched, arithmetic/code selection|

The three unmatched questions stop at initial source build; second run changed
only the independently evidenced seek comparison, not a matrix. No padding,
register forcing, native profile changes, linked DLL or runtime execution.
Original owner undefined failure edge and missing constructor EH remain open.

## Suggestions and next package

Suggestions only, no Ghidra edits:20126dcf param_1 -> wrappedStream, param_2 ->
destination, param_3 -> remainingBytes, param_4 -> totalRead; local_20 -> chunkBytes
(copy/direct-read count).20126fc0 param_2 -> lowOffset, param_3 -> ignoredHighOffset,
param_4 -> origin, param_5 -> resultingPosition. Source fields above were renamed
actually; these decompiler suggestions were not applied.

Next allocation dependencies: object pools20124903/20127a04 and shared reservation/
slot machinery201464c3, plus buffer cache20127642/2012772b. Recover shared
contracts before duplicating large pool code or further matching of stat/read.
