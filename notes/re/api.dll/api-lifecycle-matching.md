# API lifecycle outer bodies and SEH wrapper

2026-11-03. Status: live verified original model/bytes and real native VC8 COFF
comparison. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
API-04 slice; this does not complete Initialize, exception filter internals,
shutdown/message-loop implementation or the entire package.

## Why object comparison is useful here

The outer methods depend on large callbacks not yet reconstructed. Compiling
ordinary C++ into native COFF preserves unresolved references to these actual
dependencies rather than supplying dummy bodies. /Gy emits independent COMDAT
sections, enabling exact extent/relocation checks. This evidence level remains
separate from the already linked21-byte GetWindowHandle match. No synthetic
dependency call is represented as a certified implementation.

Source:src/api-pilot/lifecycle.cpp and application_boundary.h. One hypothesis,
one actual build; no source/flag permutation was necessary. /O2 /Oy- /hotpatch
/GS /MD /Zi /GL- /Gy, compiler14.00.50727.762. Natural __try/__except source:
return callback() on normal completion; exception filter receives code/pointers;
handled exception returns0. Original syntax/typedef names remain unknown.

Actual run:vm-share/modules/api/runs/api-lifecycle-object-20261103-001.
Canonical artifact api-lifecycle.obj, SHA256
22F13BD7733A8C25276C6502A845C7C059F8545E853083DDD6CAC1B693431072.
api-lifecycle.pdb is the compiler PDB, not a linked-image PDB. No DLL is linked
or executed in this run. The fixed worker filename fsx.exe transports the COFF
object; its actual format/header and module.json identify this explicitly.

The initial request.json inherited generic window/DLL wording from the pilot
controller. Preserve that historical file; PLAN.md, experiment.json, actual
build.cmd/object header and lifecycle-comparison.json identify the real test.
Controller metadata has now been corrected for future object jobs. All artifacts
and the completed job snapshot are under the API module-owned directories.

## Complete verified ranges

| Original range | Native COMDAT extent | Result |
|---|---:|---|
|2012052a..20120534 Shutdown|11 bytes|Relocation-validated object equality|
|20122817..20122833 RunMessageLoop|29 bytes|Relocation-validated object equality|
|2013e3c1..2013e449 wrapper including detached EH|137 bytes|Relocation-validated object equality|
|201f0808..201f0823 EH4 scope table|28 bytes|Relocation-validated metadata equality|

Unique code union177 bytes, metadata28. Wrapper includes normal90 bytes plus
47 filter/handler bytes; the stored Ghidra function body remains90 and is not
silently counted twice. These object ranges are disjoint from the21-byte linked
window getter; do not combine the evidence levels into a whole-module percentage.

scripts/compare-api-lifecycle-object.cjs parses x86 COFF sections, symbol table,
labels, DIR32/REL32 relocations and addends. Only explicit dependency identities
or paired section-local labels are accepted. A private comparison buffer resolves
the relocation fields to validated original targets; no file bytes are patched.
Every non-relocation byte and the complete section extent must match. The EH4
bridge's original instruction shape/cookie association and actual import thunk
to msvcr80!_except_handler4_common are checked separately. Its downstream cookie
helper/filter/library implementation is not certified by this comparison.

The scope table's two relocations must point to wrapper offsets5a and69;
all other header/enclosing bytes remain exact. All four original VTable targets
are cross-checked; callback global2020de7c starts zero. Negative controls reject
a changed opcode, a changed relocation addend and a wrong shutdown callback
mapping. Detailed fields/targets and binary hashes are in lifecycle-comparison.json.

Header regression build:api-window-lifecycle-header-20261103-001, DLL SHA256
61E7EB5D6B7FB0FF70F9B30A502D9E266F9E074F8F990A27BBBF03091A0A976C.
The full21-byte window comparator and both negative controls pass with the new
class method declarations. Heap controls were not restarted.

## Ghidra corrections: actual edits

Both programs were checked open; fsx.exe remained current, api.dll explicitly
targeted. Before edits, saved API revision1793718640047:636d9e2e:31 was backed
up as artifacts/api.dll/ghidra-backups/api-before-lifecycle-slots-20261103-1636.gzf.
scripts/ghidra/CompleteApiLifecycleSlots.java validates original table targets,
slot04 bytes, existing function state/layouts and type shapes before editing.

- Created the exact11-byte function Application::Shutdown at2012052a, modeled
  as int __thiscall (known EXE caller ignores its forwarded EAX result).
- Renamed FUN_20122817 to Application::RunMessageLoop; typed int __thiscall with
  WINAPI no-argument MSG* callback. This is consistent with EXE Run and the DLL
  loop's callback call/use. Added evidence/limit comments to both functions.
- Filled /ApplicationVTable+4/+8 using pointers to distinct function definitions
  /FSXReconstruction/ApplicationShutdown and ApplicationRunMessageLoop. Target
  signatures were separately read back; table pointer slots and code unchanged.
- Added /FSXReconstruction/CurrentMessageCallback, returning /winuser.h/tagMSG*,
  __stdcall with no arguments. Preserved existing callback wrapper types.
- Coordinated target Initialize argument2 and VTable /Initialize definition:
  char* commandLine/startupArgument -> neutral uint startupArgument. The EXE
  supplies a32-bit m_nCmdShow word; no DLL-side pointer use is established.
  Signedness/original semantic type remains open. Corrected both existing
  VTable definitions' this parameter from undefined to Application*.
- Preserved errorCallback void* pending its own coordinated ABI review; retained
  observed Application4 / ApplicationVTable16 layouts without enlarging them.

The first transaction hit an array-covariance exception while replacing managed
ParameterDefinition entries and rolled back. The script now copies those arrays
to their interface type first; the subsequent bounded transaction succeeded.
Readback:artifacts/api.dll/api-lifecycle-after-ghidra-20261103.json and independent table
definition/target signature reads. Saved revision1793720305185:636d9e2e:135,
unsaved_changes=false. No tab changes, instruction patching or original-file edits.
Backup/save are actual database operations, separate from these notes.

Useful suggestions only: DAT_2020de7c -> g_currentMessageCallback, supported by
typed slot08 store and message-loop indirect call/MSG* use. Shutdown local iVar1
-> shutdownResult; wrapper local iVar1 -> callbackResult. These variable/global
renames are not applied; function/parameter/type edits above are applied.

## Next question

Preserve this source/object evidence and the existing linked getter baseline.
Next reconstruct the160-byte Initialize outer body with honest unresolved FDI,
callback and event-registration dependencies. Inspect the nonstandard ESI input
to helper200f6451 before choosing its C++ model; a generic stdcall guess would
erase known register dataflow. Then expand exception-filter/shutdown/message-loop
dependencies and integrate a genuinely linked Application VTable/GetApplication.
Object equality does not close the linked-body/export144 or runtime gates.
