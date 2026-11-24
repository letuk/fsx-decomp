# API constructor record initialization

## Constructor block at 0x2016198a

Live verified in `api.dll` on 2026-11-24. The contiguous range
`0x2016198a`–`0x20161a29` contains eight 20-byte bodies. Each preserves
the ECX receiver in ESI, calls a distinct default-copy routine with
that receiver, sets the first receiver DWORD to 1, and returns the
receiver in EAX. The prefix and teardown are the same in all eight.
Their immediate callers overwrite that DWORD with a table address
after the call. Constructor staging is the source reconstruction.
Original class names and complete object layouts remain open. The
one-word C++ receiver is a minimum layout for this scope.

| Constructor VA | Copy target VA | Default data VA | DWORDs |
| --- | --- | --- | ---: |
| `0x2016198a` | `0x20161267` | `0x201fb138` | 7 |
| `0x2016199e` | `0x20161285` | `0x201fb154` | 5 |
| `0x201619b2` | `0x201612a3` | `0x201fb168` | 5 |
| `0x201619c6` | `0x201612c1` | `0x201fb17c` | 6 |
| `0x201619da` | `0x201612df` | `0x201fb194` | 8 |
| `0x201619ee` | `0x201612fd` | `0x201fb1b4` | 5 |
| `0x20161a02` | `0x2016131b` | `0x201fb1c8` | 8 |
| `0x20161a16` | `0x20161339` | `0x201fb1e8` | 5 |

The eight original copy routines use `REP MOVSD` and begin copying
at ECX minus four for a nonnull receiver. Their source immediates
have PE base relocations and identify 196 contiguous zero bytes in
the initial on-disk image. The retained export verifies the data,
both sets of instruction listings, the eight target addresses and
the declared word counts. It makes no claim about runtime mutations
of those source records. The copy routines are explicit unresolved
boundaries in the rebuilt object and their bodies are outside the
reconstruction scope.
[Live Ghidra and data evidence](../../../artifacts/api.dll/construction-records-20261124/ghidra-construction-records.json).

Suggested Ghidra parameter name for each constructor's `param_1` is
`receiver`, based on the ECX input, first-word store and EAX return.
The caller's later store leaves the semantic name of that DWORD open.
Read-only Ghidra reported saved revision `1793741866691:0852fe08:3`.

## COFF comparison

The eight C++ constructor instantiations share a source definition
and bind one original dependency each. All 160 bytes match in native
VC8 RTM x86 COFF after resolving one validated REL32 per body. Each
field starts six bytes into its constructor. The comparator checks
the original direct call, distinct rebuilt symbol, zero addend and
remaining bytes. The XP receipt succeeded, source hashes agree and
all 20 source-pass checks and seven previously accepted comparison
gates passed. Linked DLL and runtime verification remain open.
[Comparison report](../../../artifacts/api.dll/construction-records-20261124/comparison.json).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-construction-records-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-construction-records.cjs vm-share/modules/api/runs/api-construction-records-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-construction-records-UNIQUE-ID
```

## Default-copy block at 0x20161267

Live verified in `api.dll` on 2026-11-24. The eight bodies from
`0x20161267` through `0x20161356` each read 5 to 8 DWORDs from their
own default record. For a nonnull ECX receiver the destination is four
bytes before it. A null receiver resolves to a null destination before
the copy. The source preserves the observed receiver value in EAX.
The inspected constructors discard that result. The reconstructed
return type is `void*`, based on the preserved EAX value rather than
on external caller use. Suggested Ghidra name `param_1` → `receiver`
applies to all eight copy bodies.
[Live listings and initial data](../../../artifacts/api.dll/construction-default-copies-20261124/ghidra-construction-default-copies.json).

The factory previously left the copy symbols unresolved. The automated
source pass retained eight 47-byte candidates with status Different.
Those results are superseded here by the manual reconstruction.
The first shared-translation-unit control used a void return and
emitted 28-byte copy bodies. Returning the receiver produced the
original 30-byte bodies after relocation, but the same translation
unit allowed VC8 to preserve the constructor receiver in EDX,
shortening the constructors to 16 bytes. Separate native translation
units retain the original instruction sequence on both sides.
[Unity controls](../../../artifacts/api.dll/construction-default-copies-20261124/unity-controls.json).

The standalone VC8 RTM build matches all 240 copy bytes after validating
one DIR32 per routine. Each field begins at offset 21, has addend zero,
and references the separately verified data record listed above.
The eight named COFF data symbols occupy disjoint zero-filled `.bss`
ranges. The corresponding 196 initial bytes in the original PE are
zero. Original data types, linked placement and runtime mutations
remain open. This supersedes the earlier open copy-body status while
preserving that run's original constructor scope and evidence.
[Copy and data comparison](../../../artifacts/api.dll/construction-default-copies-20261124/comparison.json).

The independent factory replay matches all eight constructor bodies
after their REL32 fields are resolved. All 20 source-pass checks and
seven earlier comparison gates passed. Both XP receipts succeeded.
Read-only Ghidra reported saved revision `1793741866691:0852fe08:3`.
Linked DLL and runtime verification remain open.
[Constructor regression](../../../artifacts/api.dll/construction-default-copies-20261124/constructor-regression.json).

```powershell
./scripts/matching/start-api-window-pilot.ps1 -RunId api-construction-default-copies-UNIQUE-ID -FactoryObject -ConstructionDefaultsObject -Vc8Rtm
node scripts/compare-api-construction-default-copies.cjs vm-share/modules/api/runs/api-construction-default-copies-UNIQUE-ID
./scripts/matching/start-api-window-pilot.ps1 -RunId api-construction-default-copies-regression-UNIQUE-ID -FactoryObject -SourcePass -Vc8Rtm
node scripts/compare-api-construction-records.cjs vm-share/modules/api/runs/api-construction-default-copies-regression-UNIQUE-ID
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory vm-share/modules/api/runs/api-construction-default-copies-regression-UNIQUE-ID
```
