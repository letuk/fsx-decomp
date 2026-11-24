# GetApplication factory and observed Application VTable

2026-11-03. Status: live verified original PE/model and actual native VC8 COFF
comparison. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.

## Original bridge and RTTI boundary

GetApplication is the original named export144, entry20120442,72 bytes. The PE
name/ordinal/function association is independently parsed and asserted by the
comparator. It returns cached global2020de64, lazily requests4 bytes through the
heap helpers, installs table200f2b24 and publishes the result or NULL. Global
2020d8ec supplies the allocator's zero-initialize argument and starts0. Original
2020de64 also starts0; candidate object globals remain unresolved definitions,
so no candidate initial-data equality is claimed in this object check.

The DWORD at table-4 (200f2b20) is0073255c: the tail of a preceding terminated
string, not an image VA to an RTTI Complete Object Locator. Four observed slots
map Initialize2012048a, Shutdown2012052a, RunMessageLoop20122817 and
GetWindowHandle20120535. This supports an RTTI-less source bridge fragment;
it does not establish whole-DLL /GR- settings, original C++ syntax/class name,
complete original object size or every possible interface relationship.

## Ordinary C++ source and bounded experiment

application_boundary.h now declares the four methods virtual in observed order,
with no invented virtual destructor. The source type is deliberately a minimum
bridge fragment. A compile check requires sizeof4 for the observed allocation.
factory_boundary.cpp calls the real heap dependencies, constructs into the
returned allocation using ordinary placement new, and publishes the pointer.
No manual VTable byte array, instruction patch or fake implementation is used.
Actual outer method definitions, including the still-unmatched Initialize, are
compiled in this object; unresolved callback/filter/heap/global dependencies
remain explicit.

First native profile: /O2 /Oy- /hotpatch /GS /MD /Zi /GL- /Gy /GR-, compiler
14.00.50727.762. /GR- is a fragment modeling choice supported by the absent COL
shape, not a recovered original full command. Existing window/lifecycle source
now has virtual method decorations; comparators accept the precisely named old
nonvirtual or new virtual symbol, never an arbitrary address/name substitute.

Run api-factory-object-20261103-001 produced75 bytes, not72. It reloaded EAX
from [EBP-4] at object offset34 after initializing the vptr, adding3 bytes.
The source used placement construction then reread its application local.
Rejected as a full getter match; factory-diagnostic.json preserves this result.

Trial2 assigned placement-new's returned pointer to that local:
`application = new (application) ApplicationBoundaryPilot;`.
Prediction: omit exactly that reload and recover the72-byte body/branch offsets.
Actual run:vm-share/modules/api/runs/api-factory-object-20261103-002.
api-factory.obj SHA256
4FB6329CFC42EEDFABFB16988C2BF4279B81F43C9ECA3FD65A35D3B1AEE18DF0.
Only factory_boundary.cpp changed; actual build commands are byte-identical.
Source/command controls in source-control.json; plans retained before dispatch.
The prediction succeeded; stopped after two trials, no third permutation.

## Certified object scope and limits

- GetApplication20120442..20120489: complete72-byte body equality after seven
  COFF symbol/addend validations (cached pointer/global uses, both heap calls,
  zero-initialize global and generated VTable). All remaining bytes, conditional
  branches, allocation size4, stack frame and returns are exact.
- Generated16-byte VTable COMDAT: four DIR32 references to actual defined method
  sections, in original order. No COL/header in that COMDAT. Pointer target
  association does not certify Initialize's still-different body or its callees.
- Window getter21 bytes now also verified in this object. The prior317 complete
  object-code bytes and28 EH4 metadata bytes retain their gates; Initialize
  retains its separate123-byte/21-field partial report and false whole-match flag.

Total unique original complete-body code in this object scope410 bytes:
previous317 + getter72 + window21. Metadata44 = EH4 table28 + VTable16. Window21
overlaps existing linked DLL evidence, so never add it twice to an address union.
Initialize's partial ranges are not a complete-function count or percentage.
Unused placement-new/constructor COMDATs are not counted as additional recovered
original functions. No resource/full-image/layout/runtime equality follows.

Negative controls reject allocation size4->8, wrong VTable pointer and
Shutdown-to-RunMessageLoop table substitution; all previous opcode/addend/target
controls still reject. scripts/compare-api-lifecycle-object.cjs checks the native
COFF relocation fields and entire stated ranges using private buffers only.
Both old177-byte and317-byte object reports were rerun successfully to verify
backward compatibility. Original files/object files are never modified.

The COFF name _GetApplication reflects the chosen zero-argument cdecl source
model. Bare RET/no arguments cannot uniquely distinguish original cdecl/stdcall.
Original export144 is verified; a rebuilt linked DLL export table/ordinal144 is
not yet produced or claimed. Missing global/implementation definitions prevent
complete linked bridge integration. New worker transport fsx.exe contains COFF,
with canonical api-factory.obj and compiler api-factory.pdb under API-owned runs.

Linked regression api-window-virtual-bridge-20261103-001 still passes the full
21-byte window gate and both negative controls. DLL SHA256
A02DCC69262868C8A2F1D816EEABF7606347FCE7DB99B5D6901A05F37935D0C1.
This verifies the virtual declaration change without extending that DLL's scope.

## Database evidence and continuation

Before the comment update, API current as selected by user, fsx.exe also open;
saved API revision1793722735273:636d9e2e:312. GZF backup:
artifacts/api.dll/ghidra-backups/api-before-factory-evidence-20261103.gzf.
Added an English evidence/algorithm/return plate comment at20120442. Preserved
the established GetApplication name/return type, zero-input signature and stored
unknown calling convention; no unproven original-convention fix. No body, memory,
table-type or EXE changes. Saved readback1793724055614:636d9e2e:314,
unsaved_changes=false. New code/table-prefix/initial-flag reads match the pinned
original file exactly. No agent tab switch or candidate execution.

Snapshots:api-factory-memory-20261103.json and api-factory-after-ghidra-20261103.json.
Unapplied naming suggestion: DAT_2020d8ec -> allocationZeroInitializeFlag
(provisional, verified role at this caller only; inspect other uses before giving
it an application-only global name). Existing g_application/newApplication names
are preserved.

API-02 factory source/object milestone is complete; final linked factory/export
gate remains open. Next inspect initialization callback entry20120869 as API-05:
map its actual reachable control transfers and phases before creating large
Ghidra functions or attempting broad startup source. Keep interface/core logic
work moving rather than reopen native heap/register permutations.
