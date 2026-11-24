# simscheduler.dll function coverage

Ghidra's live Function Manager counts 229 function objects.
Its internal iterator returns 167 functions, including 12 local
thunks. Its external iterator returns 62 imported function objects.
The dashboard covers all 167 internal entries. The live inventory
retains the external entries separately. Decoded ranges without a
Ghidra function object are outside the count.

Live verified in simscheduler.dll. `FUN_20cc15f0` is a ten-byte wrapper
used by the unwind action at `0x20cc71e9`. The action loads the
context storage pointer into EBX. The wrapper calls
`FUN_20cc20b0` to return that storage to the pool while preserving ECX.
The reconstructed factory already expresses that cleanup with
`ReleaseSchedulerContextStorage(storage)` in its exception path.

Live verified in simscheduler.dll. `FUN_20cc1ef0` is a 15-byte
cleanup helper shared by eight unwind actions. EAX points to a
reference slot. If the slot is populated the helper invokes the
object's reference release through VTable slot `+0x04`. The
verified SchedulerContext table at `0x20cc1200` maps that slot
to `FUN_20cc2740`. The reconstructed source expresses the release
through `SchedulerContextReference` and `ReleaseNamedContext`.
Both helpers have original listings and PE byte verification.
The VC8 SP1 native COFF run `simscheduler-unwind-helpers-20261116-001`
uses an opt-in build of the existing scheduler translation unit.
The worker returned exit code zero and the compiled-source hash
matches the staged source.

| Original entry | Original / rebuilt bytes | Status | Difference |
| --- | --- | --- | --- |
| `0x20cc15f0` | 10 / 11 | Diagnostic | EBX input and an ECX-preserving call become a stack argument and a tail jump. The call and jump target the corresponding storage release. |
| `0x20cc1ef0` | 15 / 15 | Diagnostic | The first instruction reads the reference slot through ECX instead of EAX. The following 13 bytes are identical. |

The standalone C++ probes use supported calling conventions.
The original register inputs remain open. The production
reconstruction uses typed arguments and compiler-generated unwind
actions. The retained EH comparison now includes rebuilt action
listings and COFF relocations. Nineteen original cleanup funclets
still lack individual byte comparisons.

### RTM comparison for the storage wrapper

Live verified in `simscheduler.dll` at `0x20cc15f0`. The action at
`0x20cc71e9` loads EBX from `[EBP+0x10]` and jumps to the wrapper.
The wrapper preserves ECX across its call to `0x20cc20b0`. Native
VC8 RTM emits the same 11 wrapper bytes as SP1, including the REL32
placeholder for the verified release target. The original body is
10 bytes. The rebuilt factory state-1 funclet instead uses
`lea ecx,[ebp-38h]` and jumps to the reconstructed
`UnconstructedSchedulerContextStorage` destructor. This points to
the EH and internal call context as the next matching boundary.
The result covers native COFF. Linked and runtime verification are
open. Ghidra read-only, save state unverified.

- [RTM comparison and source snapshot](../../../vm-share/modules/simscheduler/runs/simscheduler-unwind-helpers-rtm-20261117-001/comparison-20cc15f0.json)
- [Live wrapper, caller and release target](../../../artifacts/simscheduler.dll/unwind-storage-rtm-ghidra.json)

### Linked class allocation and unwind

Live verified in `simscheduler.dll` that the factory funclet at
`0x20cc71e9` loads `[EBP+0x10]` into EBX and jumps to `0x20cc15f0`.
The deleting wrapper at `0x20cc28c0` independently puts the same
storage argument in EBX and calls `0x20cc20b0` directly after
destruction. These are distinct paths through the same pool release.

The C++ LTCG probe models the construction path with class-specific
`operator new` and `operator delete`. It reuses the constructor and
pool-return code from the scheduler source. Allocation, destruction
and the lock/free-list operations use projection link boundaries.
The mapping of `0x20cc15f0` to the class deallocation operator
is a reconstruction hypothesis.

VC8 RTM emits an eight-byte factory funclet with a single frame load
and direct jump to the deallocation wrapper. It reads `[EBP-0x10]`
into EDI. The wrapper is a five-byte tail jump to the linked pool
release. The original is ten bytes and saves ECX around a CALL
followed by RET. The comparator validates both release paths and the
direct continuation from the action. The SP1 member variant has the
same action and wrapper instruction bytes. Native COFF remains
Diagnostic and the linked result is also Diagnostic.

Tagged placement allocation added another argument and a call in
the action. The guard variant passed a guard address to its
destructor. Disabling global optimization for the deletion operator
retained a CALL/RET but required a stack parameter, EBP frame and EDI
save, producing 17 bytes. Separating the pool lookup, changing the
release declaration's calling convention, allowing inlining and
giving the deletion operator an empty exception specification all
retained the linked tail jump. Those trials remain in the worker runs.

The normal factory still uses its storage guard. The linked probe
covers construction failure and the direct pool-call topology. Full
DLL linkage and runtime remain open. The native export and GPS
regression gates passed. Ghidra read-only, save state unverified.

- [Linked wrapper, factory funclet and pool release listing](../../../vm-share/modules/simscheduler/runs/simscheduler-storage-cleanup-20261117-001/comparison-20cc15f0.json)
- [Live Ghidra listings](../../../artifacts/simscheduler.dll/context-storage-cleanup-ltcg-ghidra.json)
- [SP1 linked member control](../../../vm-share/modules/simscheduler/runs/simscheduler-storage-member-sp1-20261117-001/comparison-20cc15f0.json)
- [CALL/RET control](../../../vm-share/modules/simscheduler/runs/simscheduler-storage-new-noglobal-20261117-001/comparison-20cc15f0.json)

### Linked pool implementation

Live verified in `simscheduler.dll` at `0x20cc15f0`. Linking just
the reconstructed lock and free-list operations kept the five-byte
tail jump. Adding the existing context-pool acquisition code changed
the wrapper to a normal call in both VC8 RTM and SP1. The retained
projection also links the existing context destructor and resolves
the original API pool imports and Kernel32 count and break imports.

The wrapper is ten bytes. After validating and normalizing the
relative call to the counterpart of `0x20cc20b0`, the sole differing
byte is at offset 2, PUSH EAX instead of PUSH ECX. The original
keeps the storage in EBX. The linked funclet loads it into EAX from
`[EBP-0x10]` and the wrapper forwards it on the stack. POP ECX in
the candidate discards the argument. Native COFF and the linked
wrapper remain Diagnostic.

Separate LTCG translation units for the factory and pool changed
the call displacement and retained the same ABI and instruction
difference. Pool lookup separated from release and explicit EBP
frames also retained the difference. Testing stdcall and fastcall
on the release helper moved stack cleanup to the callee and removed
the wrapper's POP ECX. The 10-byte single-unit version supersedes the
five-byte projection as the dashboard diagnostic.

The native scheduler export and GPS regression gates passed. Ghidra
was read-only, save state unverified. The full DLL and runtime remain
unverified.

- [10-byte linked comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-storage-linked-imports-20261117-001/comparison-20cc15f0.json)
- [Separate translation units](../../../vm-share/modules/simscheduler/runs/simscheduler-storage-linked-split-20261117-001/comparison-20cc15f0.json)
- [SP1 comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-storage-linked-sp1-20261117-001/comparison-20cc15f0.json)
- [Live Ghidra listings](../../../artifacts/simscheduler.dll/context-storage-linked-pool-ghidra.json)

All 62 external Ghidra function entries were reconciled with unique
IAT slots in the original PE. Three additional IAT slots at
`0x20cc10a0`, `0x20cc10c4`, and `0x20cc10ec` have no entry in
Ghidra's external function iterator. External implementation bytes
are outside the module comparison.

Live verified in simscheduler.dll. The six-byte thunks below match
the retained RTM conversion linkbuild after validating HIGHLOW
offset 2 and the MSVCR80 import identity.

| Address | Import |
| --- | --- |
| `0x20cc6e30` | `_unlock` |
| `0x20cc6e3c` | `__dllonexit` |
| `0x20cc6e48` | `_lock` |
| `0x20cc6e54` | `__clean_type_info_names_internal` |
| `0x20cc6e60` | `_except_handler4_common` |

Ghidra was inspected read-only and reported no unsaved changes.
Matching gaps and runtime verification remain open.

- [Live function inventory](../../../artifacts/simscheduler.dll/live-function-inventory.json)
- [Original listings](../../../artifacts/simscheduler.dll/remaining-functions-ghidra.json)
- [Storage wrapper comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-unwind-helpers-20261116-001/comparison-20cc15f0.json)
- [Reference release comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-unwind-helpers-20261116-001/comparison-20cc1ef0.json)
- [Reconciled original IAT inventory](../../../artifacts/simscheduler.dll/import-inventory-crosscheck.json)
- [Retained linked run](../../../vm-share/modules/simscheduler/runs/simscheduler-crt-conversion-20261116-001)
