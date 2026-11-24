# API checked arithmetic

## Unsigned addition at 0x2014e674

Live verified for `api.dll` on 2026-11-19. Ghidra names the entry
`FUN_2014e674`. The function adds two 32-bit arguments and uses an
unsigned comparison against the first to detect wraparound. On success
it writes the result and returns zero. On overflow it returns
`0x80070216` and leaves the output untouched. The three stack arguments
and both paths through `RET 0xc` support the reconstructed C++ signature
`HRESULT __stdcall AddUnsignedCheckedPilot(DWORD left, DWORD right, DWORD* result)`.
The SDK expression `HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW)`
compiled to the observed constant. [Live listing and caller inventory](../../../artifacts/api.dll/checked-add-20261119/ghidra-2014e674.json).

The generated 37-byte source-pass body used a different branch layout
and separate epilogues. Its comparison is superseded for this scope.
The typed implementation places the successful store first and uses
function-local size optimization. It reproduces the shared epilogue and
all 35 bytes through `0x2014e696` in VC8 RTM native x86 COFF. The COFF
body and original PE range both contain zero relocations. The XP receipt
succeeded, source hashes agree, and all 20 source-pass checks plus the
prior exact gates for ordinals 172 and `ErfSetCodes` passed. Linked DLL
and runtime verification remain open.
[Exact comparison](../../../artifacts/api.dll/checked-add-20261119/comparison.json).

Ghidra reported saved revision `1793741866691:51f17f6c:3` after read-only
inspection. Suggested decompiler names are `param_1` → `left`,
`param_2` → `right` and `param_3` → `result` based on the operands
and successful output store.

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-checked-add-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-coff-body.cjs checked-add vm-share/modules/api/runs/api-checked-add-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-checked-add-UNIQUE-ID
```

## Unsigned subtraction at 0x2015347a

Live verified for `api.dll` on 2026-11-19. Ghidra names the entry
`FUN_2015347a`. It subtracts the second 32-bit operand from the first
and uses an unsigned comparison of the difference against the first
operand to detect underflow. On success it stores the difference and
returns zero. On underflow it returns `0x80070216` and leaves the
output untouched. The stack accesses and `RET 0xc` support the
reconstructed `HRESULT __stdcall SubtractUnsignedCheckedPilot(DWORD left, DWORD right, DWORD* result)`.
Ghidra reports six callers, including `0x20151449` and `0x20151e3c`,
which also call the checked add.
[Live listing and caller inventory](../../../artifacts/api.dll/checked-subtract-20261119/ghidra-2015347a.json).

The 37-byte generated source-pass body used separate epilogues. Its byte
result is superseded for this scope. The typed implementation uses the
same function-local size optimization as the checked add and matches all
34 bytes from `0x2015347a` through `0x2015349b` with no relocations
in the native VC8 RTM x86 COFF body or original PE range. The XP receipt
succeeded, the worker source hash agrees with the submitted snapshot
and all 20 source-pass checks passed. The checked add, ordinal 172 and
`ErfSetCodes` exact gates passed again. The retained build includes
the effective command, source hashes, log, receipt and native object.
Linked DLL and runtime verification remain open.
[Exact comparison](../../../artifacts/api.dll/checked-subtract-20261119/comparison.json).

Read-only Ghidra reported saved revision `1793741866691:51f17f6c:3`.
Suggested decompiler parameter names are `param_1` → `left`,
`param_2` → `right` and `param_3` → `result`, based on the
subtraction operands and the conditional store.

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-checked-subtract-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-coff-body.cjs checked-subtract vm-share/modules/api/runs/api-checked-subtract-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-checked-subtract-UNIQUE-ID
```

## Unsigned 64-bit subtraction at 0x20171131

Live verified in `api.dll` on 2026-11-19. The original subtracts the
low words with SUB and propagates the borrow into the high words
with SBB. It compares the unsigned difference to the first operand,
using the low words only when the high words are equal. On success it
stores both result words and returns zero. On underflow it returns
`0x80070216` and leaves the output untouched.

The original five stack words, paired arithmetic and `RET 0x14`
support the reconstructed signature
`HRESULT __stdcall SubtractUnsigned64CheckedPilot(ULONGLONG left, ULONGLONG right, ULONGLONG* result)`.
Ghidra names the function `FUN_20171131` and identifies the forwarding
caller at `0x201711d2`. The caller still displays five separate words.
Suggested names are `param_1` → `leftLow`, `param_2` → `leftHigh`,
`param_3` → `rightLow`, `param_4` → `rightHigh` and `param_5` →
`result`, based on the SUB/SBB operands and two result stores.
Read-only Ghidra reported saved revision `1793741866691:51f17f6c:3`.
[Live listing and caller](../../../artifacts/api.dll/checked-subtract64-20261119/ghidra-20171131.json).

The previous generated four-word arithmetic candidate emitted 72 bytes.
Its result is superseded for this scope. The typed implementation with
function-local size optimization matches all 50 bytes through
`0x20171162` in the VC8 RTM native x86 COFF build
`api-checked-subtract64-20261119-001`. Both ranges contain zero
relocations. The XP receipt succeeded, the compiled-source hash agrees,
and all 20 source-pass checks and four previous exact gates passed.
The run retains its source snapshot, effective command, receipt, log
and object. Linked DLL and runtime verification remain open.
[Exact comparison](../../../artifacts/api.dll/checked-subtract64-20261119/comparison.json).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-checked-subtract64-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-coff-body.cjs checked-subtract64 vm-share/modules/api/runs/api-checked-subtract64-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-checked-subtract64-UNIQUE-ID
```

## 64-bit subtraction forwarder at 0x201711d2

Live verified in `api.dll` on 2026-11-19. Ghidra names the entry
`FUN_201711d2` and reports no incoming references. The listing pushes
five stack words in order for the original direct call at `0x201711e6`
to `0x20171131`. It then restores the frame and returns with
`RET 0x14`, preserving the callee's EAX. The reconstructed
`ForwardUnsigned64SubtractionPilot` signature groups the four operand
words into two `ULONGLONG` arguments, takes an output pointer and
returns the callee's `HRESULT`. No external caller was available
to establish use of the returned value. Suggested decompiler names
for `param_1` through `param_5` are `leftLow`, `leftHigh`,
`rightLow`, `rightHigh` and `result`, based on the forwarding order
and the callee's arithmetic and output store.
[Live listing and references](../../../artifacts/api.dll/checked-subtract64-forwarder-20261119/ghidra-201711d2.json).

The 59-byte generated candidate inlined the subtraction. Its byte
comparison is superseded for this scope. The typed source marks the
callee `noinline` to retain the observed call. In the VC8 RTM native
x86 COFF build `api-checked-subtract64-forwarder-20261119-001`,
the comparator validates the sole REL32 at offset 21, its zero addend,
the rebuilt callee symbol and the original target VA. All 29 function
bytes agree after resolving that field. The same build keeps all 50
callee bytes exact. The XP receipt succeeded, source hashes agree and
all 20 source-pass checks and five earlier exact gates passed.
The run retains its source snapshot, effective command, receipt,
log and object. Linked DLL and runtime verification remain open.
Read-only Ghidra reported saved revision `1793741866691:51f17f6c:3`.
[Normalized comparison](../../../artifacts/api.dll/checked-subtract64-forwarder-20261119/comparison.json).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-checked-subtract64-forwarder-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-coff-body.cjs checked-subtract64-forwarder vm-share/modules/api/runs/api-checked-subtract64-forwarder-UNIQUE-ID
node scripts/compare-api-coff-body.cjs checked-subtract64 vm-share/modules/api/runs/api-checked-subtract64-forwarder-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-checked-subtract64-forwarder-UNIQUE-ID
```

## Unsigned 32-bit comparison at 0x20153784

Live verified for `api.dll`. The body reads two unsigned 32-bit values
through stack-passed pointers and returns -1, 0 or 1 for less than,
equal or greater than. Both call sites at `0x20153864` and `0x20153ca4`
test the sign. The two stack arguments and `RET 8` support the
reconstructed `int __stdcall CompareUnsigned32Pilot(const DWORD* left,
const DWORD* right)`. Suggested Ghidra names `param_1` → `left` and
`param_2` → `right` follow the order of the compared values.
The read-only follow-up on 2026-11-24 found the same bytes and saved
revision `1793741866691:0852fe08:3`.
[Live listing and references](../../../artifacts/api.dll/compare-unsigned32-20261119/ghidra-20153784.json).

The retained VC8 RTM COFF body has 33 bytes with zero relocations.
Ten byte offsets differ. The original evaluates the less-than result
first, then the greater-than result. The compiler evaluates the
greater-than result first and assigns different registers to the loaded
values and comparison results. The generated control, pinned SP1 run
009 and linked RTM LTCG run 010 reproduce the same differing bytes.
The size, alternative expression and early-return trials are retained
with their source snapshots, commands, hashes, receipts and logs.
The complete byte match remains open.
[Comparison and all ten builds](../../../artifacts/api.dll/compare-unsigned32-20261119/comparison.json).

The regular typed source matches the compiled-source hash for run
`api-compare-unsigned32-20261119-006`. All 20 source-pass checks, five
previous exact gates and the validated forwarder gate passed on that
run. Runtime verification remains open. To repeat the selected COFF
diagnostic, use the factory source-pass profile with `-Vc8Rtm` and
run `node scripts/compare-api-coff-body.cjs compare-unsigned32 <run>`.

## Signed-to-unsigned range check at 0x201513ba

Live verified in `api.dll` on 2026-11-24. This entry previously had
dashboard status Inventoried, with no manual source or comparison.
The automated source pass deferred it because the decompiler's single
parameter disagreed with `RET 8`. The live listing shows a signed
comparison of the first stack word against zero, with no read of the
second word. The sole caller at `0x201513cf` passes the same value in
both slots at `0x201513da` and copies it to the output only when the
returned status is nonnegative.

The reconstructed `HRESULT __stdcall CheckNonnegativeLongPilot(LONG value, LONG)`
preserves the two-word ABI. It returns `S_OK` for nonnegative values and
`HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW)` for negative values.
Suggested Ghidra name `param_1` → `value` reflects the signed check.
The second slot is unnamed in source because its role beyond stack
cleanup is unestablished. Read-only Ghidra reported saved revision
`1793741866691:0852fe08:3`.
[Live listing and caller](../../../artifacts/api.dll/check-nonnegative-20261124/ghidra-201513ba.json).

The function-local size-optimization profile emits all 21 original
bytes through `0x201513ce` in the VC8 RTM native x86 COFF run
`api-check-nonnegative-20261124-001`. The complete COFF body and PE
range contain zero relocations. The XP receipt succeeded, submitted
and compiled source hashes match, and all 20 source-pass checks and
six prior comparison gates passed. The run retains sources, command,
log, receipt and object. Linked DLL and runtime verification remain
open. [Exact comparison](../../../artifacts/api.dll/check-nonnegative-20261124/comparison.json).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-check-nonnegative-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-coff-body.cjs check-nonnegative vm-share/modules/api/runs/api-check-nonnegative-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-check-nonnegative-UNIQUE-ID
```
