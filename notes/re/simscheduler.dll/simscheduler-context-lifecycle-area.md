# simscheduler.dll context lifecycle

Live verified in simscheduler.dll. `FUN_20cc27a0` constructs into
provided storage. It installs table `0x20cc1200`, copies a name into
the 32-byte field at `+0x08`, and initializes the live array, key index
and pending queue with growth 5 and flag 4. The copy resolves to
`api.dll:0x20137e17`. It initializes the step bounds to 1.0,
cursor to -1, baseline from the scheduler time slot, and remainder
to zero. The original masks the low mode and update bits. The former
whole-word zero initialization in C++ is superseded by the
bit-field reconstruction below.

`FUN_20cc2940` receives the context in ECX. For the active context
it resets the fallback connection and clears `DAT_20cc8360`.
It then releases owned storage in reverse order, pending array,
index, and live-pointer array. Ownership requires a storage pointer,
flag 1, and nonzero capacity. Release uses `api.dll:0x20135f51`.
Original unwind handlers at `0x20cc70b4` and `0x20cc7284` refer
to member cleanup through `FUN_20cc28f0`.

The slot at `0x20cc1250` (`+0x50`) points to the deleting wrapper
`FUN_20cc28c0`. It calls the destructor, then releases the context
storage through `FUN_20cc20b0` when flag bit 0 is set. It returns the
original pointer with `ret 4`. `FUN_20cc15d0` initializes an
eight-byte time value in the destination supplied in EAX.
Names and layouts are reconstructed minimum models.

The live original time initializer at `0x20cc15d0` aligns ESP to
eight bytes and restores it from EBP before returning with RET 8.
VC8 SP1 LTCG reproduces all 19 bytes when the local time value
has explicit eight-byte alignment. The destination still uses
EAX and the double still arrives on the stack. The earlier
14-byte linked diagnostic is superseded for this scope.
The aggregate time-value experiment changed the caller ABI and
was rejected. The aligned local is a reconstructed source form.
Ghidra was read-only and its save state was not verified.

- [Live helper listing](../../../artifacts/simscheduler.dll/context-time-initializer-ghidra.json)
- [Exact linked helper comparison](../../../artifacts/simscheduler.dll/context-time-initializer-comparison.json)

Live verified at `0x20cc28f0`. After the ownership checks, the
original always writes the count, replacing a positive value with
zero and writing back a nonpositive value. The earlier conditional
store is superseded. VC8 LTCG emitted 77 bytes with a conditional
expression, 71 with a local count followed by an unconditional
store, and 73 when the saved-buffer read followed the count store.
The 71-byte Diagnostic verifies the pending queue cleanup body,
the ESI member input and the api.dll ordinal 108 reference.
The storage pointer uses ECX in the original and EAX in the rebuild,
which spills the pointer earlier. The historical linked funclet
results refer to their archived source. Ghidra was read-only and its
current save state was not verified.

- [Live cleanup listing](../../../artifacts/simscheduler.dll/context-container-cleanup-ghidra.json)
- [Linked cleanup comparisons](../../../artifacts/simscheduler.dll/context-container-cleanup-comparison.json)

Two follow-up VC8 SP1 LTCG runs retained the 71-byte extent and
the checked release import. Testing the saved pointer directly
spills it before the first branch in run `004`. Moving the count
read ahead of the ownership test in run `005` allocates the saved
pointer to ECX, as in the live original at `0x20cc28f6`, but moves
the count read before the first branch and still spills the pointer
before the store. Neither trial improves the full comparison.
The source form from run `002` was restored. Both trial snapshots
and comparisons are retained.

| Address | Original | C++ COFF |
| --- | ---: | ---: |
| `0x20cc27a0` | 274 | 290 placement factory, 260 member constructor |
| `0x20cc2940` | 300 | 288 |
| `0x20cc28c0` | 31 | 35 |
| `0x20cc15d0` | 19 | 17 |
| `0x20cc28f0` | 71 | 74 |

VC8 emits a member constructor with `this` in ECX. The placement
factory folds the C++ constructor into an entry with the original
two stack inputs and `ret 8`. It adds an unwind state and still
differs from the original. The native comparator checks the original
body spans, HIGHLOW entries, the two imports, and the rebuilt EH
handler and member unwind references. It does not claim equivalence
of the compiler-generated unwind tables or exceptional paths.
The helper register ABIs remain different.

## Destructor unwind

Live verified for simscheduler.dll at `0x20cc2940`. The original
and retained native COFF destructor use state 2 during fallback
reset. Unwind runs the pending queue cleanup at `0x20cc7276`,
then the key index at `0x20cc726b`, then the live-pointer array
at `0x20cc7260`. Both decrement the state before each normal
container teardown. A failure in a teardown therefore runs only
the remaining cleanups. The three member offsets are +0x80,
+0x70 and +0x60, respectively.

The three comparisons cover 11, 11 and 14 bytes and are diagnostic.
Both builds load the context from EBP-0x14 and add the same
member offset. The original dispatches through ESI to the shared
helper at `0x20cc28f0`. The COFF actions pass ECX to three typed
destructors which emit the same cleanup sequence and storage
release relocation. The C++ was retained. Static verification
only. Ghidra read-only, no unsaved changes.

- [Unwind maps and action comparisons](../../../artifacts/simscheduler.dll/eh-funclets-comparison.json)

## Constructor unwind

Live verified at `0x20cc27a0`. Before name copy and baseline time
lookup the original and the retained VC8 member constructor have
all three container cleanup actions armed. They unwind in reverse
member order: pending queue (+0x80, `0x20cc70a6`), key index (+0x70,
`0x20cc709b`), live-pointer array (+0x60, `0x20cc7090`).
The inlined placement factory has the same three action bodies and
an additional placement-delete action after those cleanups. Its
placement-delete implementation in this COFF object is a single
RET. It does not return raw storage to the pool. That responsibility
belongs to the calling factory's storage guard.

All three instruction comparisons are diagnostic. The original
loads the context from EBP+8 and passes the member in ESI to
`0x20cc28f0`. VC8 uses EBP-0x10 and passes it in ECX to a typed
destructor. This result verifies the static unwind actions. The
mode and flag initialization differences were addressed in the
subsequent reconstruction below. Ghidra read-only, saved.

## Constructor flag and time initialization

Live verified in simscheduler.dll. At `0x20cc2847` and
`0x20cc284e`, `FUN_20cc27a0` clears three low bits at context +0x38
and two at +0x3c while retaining the remaining bits. It clears the
first four name bytes before the import call and copies the baseline
and remainder through integer loads and stores. The C++ uses
VC8 bit-field views for initialization and word views of the same
storage for the existing accessors. The union views use the pinned
x86 VC8 object representation. The time copies use `memcpy` to
preserve their object representation. Placement construction uses
the non-null storage precondition: the original dereferences storage
unconditionally and the only observed caller checks for null at
`0x20cc173e` before the call at `0x20cc1745`.

| Retained native VC8 COFF run | Original / rebuilt bytes | Result |
| --- | --- | --- |
| `simscheduler-context-flag-fields-20261116-001` | 274 / 294 | Diagnostic. Direct bit-fields emit the correct masks but lose the exact matches at `0x20cc2bc0`, `0x20cc2c80` and `0x20cc2cb0`. |
| `simscheduler-context-flag-fields-20261116-002` | 274 / 277 | Diagnostic. The two mask instructions match byte-for-byte and all three accessor regressions are resolved. The wrapper has the original stack inputs and `ret 8`. |

The retained run uses the existing placement factory profile. It
verifies the original and rebuilt reference targets. Frame layout,
store order, the compiler-generated placement-delete state and
the time helper ABI remain different. The placement-delete body in
this object is a single RET. Startup wrapper, Flight save, GPS and
archived unwind comparisons were replayed. Full DLL linkage and
runtime behavior were outside this pass. Ghidra reported no unsaved
changes.

- [Live original listing](../../../artifacts/simscheduler.dll/context-constructor-flag-fields-ghidra.json)
- [Mask and accessor verification](../../../artifacts/simscheduler.dll/context-constructor-flag-fields-comparison.json)
- [Retained build, source and receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-context-flag-fields-20261116-002)

## Complete constructor matching follow-up

### Class layout review

Live verified in simscheduler.dll. The constructor installs the
table at `0x20cc1200`, then clears four bytes at object +0x08 before
the container writes at `0x20cc27ef`, `0x20cc2803`, and
`0x20cc2814`. The bounded name copy follows at `0x20cc283f`. The
name accessor at `0x20cc2a80` returns object +0x08. A 32-byte name
member with an initializer that clears its first word is a
source-layout hypothesis. The instructions alone also permit
compiler scheduling of independent stores. No source change or
new matching attempt was made in this review.

The original PE has 21 relocated pointers from `0x20cc1200`
through `0x20cc1250`. The word immediately preceding the table
is zero, so it supplies no RTTI locator evidence. Sampled targets at
slots +0x00 and +0x08 take the instance on the stack and return
with RET 4. The deleting wrapper at +0x50 takes the instance in ECX,
calls the destructor, and returns with RET 4 after processing the
deletion flag. A native C++ virtual layout would have to preserve
both calling conventions and the destructor slot.

[Live class-layout evidence](../../../artifacts/simscheduler.dll/context-class-layout-ghidra.json).

### Native virtual class ABI probe

Live verified against simscheduler.dll and native VC8 SP1 COFF
in run `simscheduler-context-virtual-abi-20261116-001`. The probe
declares three virtual `__stdcall` methods, then a virtual
destructor using the default member ABI. VC8 writes the vptr at
object +0x00, puts the first three methods at +0x00/+0x04/+0x08,
and puts the deleting entry at +0x0c. The entry's weak vector
deleting destructor symbol resolves to the emitted scalar deleting
destructor. It receives this in ECX, reads flag bit 0 from the stack,
and returns with RET 4.

The release dispatch in the original at `0x20cc275e` and the
probe has identical instructions except for the VTable slot
displacement, +0x50 and +0x0c respectively. This supports a native
VC8 class as the original representation. The four-slot probe
does not verify the complete 21-slot declaration or the original
pool deallocator. It was compiled only. Reconstruction source and
the existing byte comparisons were retained.

- [Probe source](../../../artifacts/simscheduler.dll/context-virtual-abi-probe.cpp)
- [Verified COFF relocations and dispatch bytes](../../../artifacts/simscheduler.dll/context-virtual-abi-comparison.json)

The native COFF runs above provide two source approaches. The third
approach tests VC8 `/GL` and `/LTCG` using the retained union-view C++
and a link-only projection of the constructor, time helper, and
container cleanup dependencies. The projection has the same canonical
source hash as the retained native run. The probe exports the
constructor wrapper to retain its two-argument stack ABI and provides
typed scheduler data anchors for address comparison. The context table
anchor has no verified runtime contents.

| Linked configuration | Constructor original / linked bytes | Time helper original / linked bytes |
| --- | ---: | ---: |
| `/O2 /Oy` | 274 / 280 | 19 / 9 |
| `/O2 /Oy-` | 274 / 276 | 19 / 14 |

Live verified from the pinned original, fresh Ghidra listings, and
the retained VC8 SP1 linked images. LTCG reproduces the pointer in
EAX and `ret 8` for the helper at `0x20cc15d0`. The `/Oy-` run
also reproduces the constructor's EBP frame, cookie sequence and
epilogue. The original helper still aligns ESP to eight bytes.
The complete constructor comparison is **Diagnostic**. With the
seven address operands independently verified, the first 44 bytes
and the final 75 bytes agree modulo their address fields. The middle
still differs in the scheduling of the scalar and container stores,
the write of the first name word, and the placement unwind-state
writes. The placement-delete target in the probe is a single RET.
The scope is the linked constructor and helper; native COFF remains
a separate diagnostic at 277 bytes.

The `/Oy` comparison is run `003` and the actual `/Oy-`
comparison is run `005`. Runs `001` and `002` retain the
failed projection setup attempts. Run `004` is superseded as a
frame experiment: the staged command accidentally retained `/Oy`
and both functions' emitted bytes match run `003`. The controller
now verifies the frame flag in the staged command.

- Optimizer and ABI. Microsoft documents that x86 LTCG can choose an
  internal calling convention and affect stack alignment. A period
  Microsoft example shows an EAX argument. The probe confirms that
  mechanism on VC8 for this helper. The exact original options remain
  unproven. [LTCG reference](https://learn.microsoft.com/en-us/cpp/build/reference/ltcg-link-time-code-generation?view=msvc-170),
  [period Microsoft example](https://learn.microsoft.com/en-us/archive/msdn-magazine/2001/august/bugslayer-optimize-and-trim-your-code-with-new-switches-in-visual-c-net)
- FP, exception handling and access ordering. Both images use x87 and
  emit the bit-exact time copies. `/EHs` retains container
  cleanup around the calls. The placement-delete action comes from
  C++ placement construction. The observed fields are ordinary
  pool-backed storage with no evidence of volatile access. The
  documented volatile semantics would change the access contract.
  Size optimization, a different FP model, and PGO remain
  untested. [Optimization switches](https://learn.microsoft.com/en-us/cpp/build/reference/o-options-optimize-code?view=msvc-170),
  [exception handling](https://learn.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=msvc-170),
  [volatile semantics](https://learn.microsoft.com/en-us/cpp/cpp/volatile-cpp?view=msvc-170)
- Compiler and SDK. The original Rich records contain tool build
  50727 and the probe compiler reports 14.00.50727.762. This does
  not establish the original service pack. The probe uses the
  installed VC8 headers, the same CRT and Windows SDK import library
  settings as the baseline. The placement-delete target is
  compiled from the header into the probe object. The pinned RTM
  CRT library changes linked CRT members; an RTM compiler/header
  retest is unverified. The name-copy IAT matches independently of
  the SDK selection.

The three distinct approaches are retained with their commands,
receipts and hashes. Whole-body matching stops at the best diagnostic.
Canonical C++ was retained. The exact mode setter regressions,
SimConnect wrapper, retained unwind checks, linked Flight save,
and GPS comparison replayed successfully. The original Ghidra
program remains saved. Runtime execution is unverified.

Suggested reconstructed parameter names in Ghidra, unapplied:
`param_1` to `storage` (first stack argument is used as the
destination object and returned) and `param_2` to `contextName`
(second stack argument supplies the bounded copy).

- [Original constructor and helper listings](../../../artifacts/simscheduler.dll/context-constructor-ltcg-ghidra.json)
- [Best linked constructor comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-constructor-ltcg-20261116-005/comparison-20cc27a0.json)
- [Linked time helper comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-constructor-ltcg-20261116-005/comparison-20cc15d0.json)
- [Default-frame control](../../../vm-share/modules/simscheduler/runs/simscheduler-context-constructor-ltcg-20261116-003/comparison-20cc27a0.json)

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/context-lifecycle-ghidra.json)
- [Placement factory comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-lifecycle-20261116-003/comparison-20cc27a0.json)
- [Member constructor control](../../../vm-share/modules/simscheduler/runs/simscheduler-context-lifecycle-20261116-001/comparison-20cc27a0.json)
- [Destructor comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-lifecycle-20261116-003/comparison-20cc2940.json)
- [Deleting wrapper comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-context-lifecycle-20261116-003/comparison-20cc28c0.json)

All eleven previous matches and the GPS regression passed.
Full DLL linking and runtime verification remain open. Ghidra
read-only, save state unverified. Next area is context storage and
factory registration at `FUN_20cc1670`, `FUN_20cc1f10` and
`FUN_20cc20b0`.

## Full native virtual class reconstruction

Live verified for simscheduler.dll at `0x20cc1200`: VC8 SP1 x86
COFF emits the reconstructed `SchedulerContext` class with the same
21-slot order. The comparator checks each original PE base relocation
against the slot's Ghidra target and against the corresponding COFF
relocation. The first 20 methods use stack-this `__stdcall`; the final
slot is the compiler's weak vector deleting entry resolving to the
ECX-this scalar deleting destructor. The pool size check still enforces
0x90 bytes. The names and source signatures are reconstructed.

The native profile uses virtual calls at context dispatch sites, including
`delete context` on final release. The class-specific delete returns
the storage to the context pool. The original deleting body at
`0x20cc28c0` passes the pool storage through EBX; the native COFF
body saves this in ESI and calls the release routine with a stack
argument (31 original / 35 rebuilt bytes). This verifies the slot and
deletion flag behavior at instruction level, with different code
generation for the deallocation call. No runtime result is claimed.

| Verified scope | VC8 SP1 COFF result |
| --- | --- |
| VTable `0x20cc1200` | 21 target mappings, including deleting slot +0x50 |
| Virtual bodies | 13 Exact, including all three mode setters; two Normalized for `0x20cc2720` and `0x20cc2740`, each with its Interlocked import checked |
| Remaining virtual bodies | Diagnostic: `0x20cc2c40`, `0x20cc2ce0`, `0x20cc2e80`, `0x20cc2e20`, `0x20cc2ff0`, `0x20cc28c0` |
| Placement wrapper `0x20cc27a0` | Diagnostic: 274 / 277 bytes; COFF relocation targets the compiler-generated table |

Runs `001` and `002` retain compile setup failures. Run `003` was
built successfully and compared after the comparator was added. In that
run four virtual methods were forwarding jumps. Run `004` moved their
bodies to the virtual entry points, including the exact 83-byte primary
mode setter at `0x20cc2bc0`. Run `005` omits the manual deleting wrapper
from the native build and reproduces those results. It supersedes the
four-slot probe as evidence of table order and slot coverage.

The previous best constructor result remains the 276-byte linked
LTCG diagnostic. This COFF measurement records the effect of the class
conversion; it does not restart the constructor matching attempts.
The standard build still uses the earlier explicit table representation
for established matching profiles. Its accessor and wrapper regression
checks passed in `simscheduler-context-legacy-regression-20261117-001`.
The native build is available with `-NativeContext`. Full DLL linkage
and runtime verification remain open. Ghidra was read-only in this pass;
its current save state was not independently verified.

- [Original Ghidra bodies](../../../artifacts/simscheduler.dll/context-native-class-ghidra.json)
- [Final COFF body and slot comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-native-context-20261117-005/native-context-comparison.json)
- [VC8 build inputs and worker outputs](../../../vm-share/modules/simscheduler/runs/simscheduler-native-context-20261117-005)

## Constructor ownership and initialization follow-up

Live verified in simscheduler.dll. The factory at `0x20cc1670`
arms state 1 before calling the constructor at `0x20cc1745`.
The factory action at `0x20cc71e9` passes raw storage in EBX to
`0x20cc15f0`. In contrast, the constructor's three actions at
`0x20cc7090`, `0x20cc709b`, and `0x20cc70a6` pass the
container members at +0x60, +0x70, and +0x80 in ESI to the shared
cleanup at `0x20cc28f0`. These are separate ownership scopes.

The linked VC8 SP1 /Oy- experiments keep the factory's storage
ownership model and use a tagged placement allocation overload for
borrowed storage. The tag has no matching placement delete. This
reproduces the three-state unwind map and state 2 at name copy.
The three action prefixes and jump targets were checked in the PE.
The rebuilt jumps lead to three typed destructors. The original jumps
lead to the shared cleanup, so the action bodies are not whole-body
matches. The tagged overload and name member are reconstructed forms.

| Variant at `0x20cc27a0` | Original / linked bytes | Checked change |
| --- | ---: | --- |
| Previous frame control | 274 / 276 | Four states and late first-name-word write |
| Borrowed storage | 274 / 276 | Three states and state 2 before the import |
| Early name member | 274 / 277 | First name word cleared before the containers |
| Borrowed storage and early name | 274 / 277 | Both changes reproduced together |
| Both plus pointer-first container initialization | 274 / 277 | 104 contiguous bytes agree after checking the relocated import operand |

The combined container run is the strongest instruction comparison
and supersedes the previous frame control as the displayed diagnostic.
The 104-byte block starts at `0x20cc27ef` and ends before
`0x20cc2857`. It covers all three container initializations, the
state writes and name copy, both mode masks, and the step bound writes.
The three pointer-first constructor bodies also reproduce the use of
EDX and ECX for growth and flags throughout this block.

The whole body remains Diagnostic. Outside the matched block, the
rebuilt code zeroes EDI before storing the vptr, clears the name before the
two initial time stores, and writes the saved context to EBP+8 later
in the sequence. Near the end, the location of the stack-buffer LEA
and the order of the cursor and substep writes still differ. The
46-byte prefix and 75-byte suffix agree after address checks.
The time helper stays at 19 original / 14 linked bytes with its
verified EAX pointer argument and RET 8.

These are opt-in source experiments against the existing linked
projection. The canonical default and native class configurations
are separate. The default accessor regression build passed in
`simscheduler-constructor-control-20261117-001`. The diagnostic does not establish the original source
shape, full DLL linkage or runtime behavior. Ghidra was read-only and
its current save state was not verified.

Reproduce the combined build from the project root with a fresh run ID:

```powershell
./scripts/matching/start-simscheduler-pilot.ps1 -RunId simscheduler-constructor-<unique-id> -ContextConstructorLtcg Frame -BorrowedContextStorage -EarlyContextName -ContextContainerInit
```

- [Live Ghidra constructor, factory and cleanup listings](../../../artifacts/simscheduler.dll/context-constructor-ownership-ghidra.json)
- [Comparison of the four experimental runs and the previous control](../../../artifacts/simscheduler.dll/context-constructor-ownership-comparison.json)
- [Combined build and worker evidence](../../../vm-share/modules/simscheduler/runs/simscheduler-constructor-container-20261117-001)

## Remaining instruction scheduling

Live verified in simscheduler.dll. At `0x20cc2857` the original
computes the baseline buffer address before storing the substep and
cursor. Swapping the source order of those two member assignments in
the linked projection produced the same 277-byte body, seven reference
mappings and three-state unwind map as the container run. The source
variant was removed after preserving the worker snapshot.

Adding `/Os` to that run's otherwise unchanged build command produced
a 224-byte wrapper with verified direct calls to `__EH_prolog3` and
`__EH_epilog3`. The original has inline frame setup and teardown.
The full comparator rejected the changed call layout. The recorded
size experiment verifies the entry, return and direct call targets
only. This build option was removed after preserving its build inputs.

| Original range (end exclusive) | Remaining difference outside the three verified spans |
| --- | --- |
| `0x20cc27ce` to `0x20cc27ef` | 33 original / 36 rebuilt bytes. Vptr and zero-store order differ, and the rebuild stores the saved context in the caller argument slot. |
| `0x20cc2857` to `0x20cc2867` | 16 bytes in each build. The same four operations are scheduled differently. |

The three disjoint verified spans cover 225 of 274 original bytes
after their mapped address fields are checked. The previous combined
container build remains the displayed Diagnostic. Ghidra was read-only
and its current save state was not verified.

- [Scheduling experiment evidence](../../../artifacts/simscheduler.dll/context-constructor-scheduling-comparison.json)

## Context argument slot and source mapping

Live verified in simscheduler.dll. The original at `0x20cc27c9`
loads the storage pointer from EBP+8. The three original cleanup
actions use that same argument slot. The source entry is
`ConstructSchedulerContext` and the member initialization comes from
`SchedulerContext::SchedulerContext` in `scheduler_object.cpp`.
Both names are reconstructed. The dashboard keeps the Ghidra label and
resolves the two source definitions by name.

Passing the storage pointer directly with a separate placement tag in
`simscheduler-constructor-storage-tag-20261117-001` reproduced all
277 body bytes and the unwind map, including the store to EBP+8.
Using ordinary `inline` in `simscheduler-constructor-inline-20261117-001`
produced an 11-byte wrapper that jumps to a separate 274-byte member
body. The linked member body uses a stack argument for this. It omits
the EBP+8 store at rebuilt offset +0x4f. Its cleanup actions still
read EBP+8 and target the same three typed destructors. After the seven
address operands are mapped, removing those three store bytes from the
277-byte control makes the two rebuilt bodies agree. This is evidence
that the store is tied to inlining in the tested build. The exact
original source shape remains a hypothesis.

The separate body differs from the original at the other scheduling
sites, and the original entry contains its own frame setup rather than
a forwarding jump. The inlined build remains the entry comparison
in the dashboard. Both experimental source changes were reverted
after retaining the builds. Ghidra was read-only and the current save
state was not verified.

- [Argument slot and separate member comparison](../../../artifacts/simscheduler.dll/context-constructor-argument-slot-comparison.json)

## Direct call to the internal constructor

Live verified in simscheduler.dll at `0x20cc1743`–`0x20cc1745`.
The original factory pushes the name pointer and then the storage
pointer and calls `0x20cc27a0` directly. The constructor reads storage
from EBP+8 and name from EBP+0xc and returns with RET 8.

In `simscheduler-constructor-internal-20261117-001`, the linked
projection exports a separate caller that initializes borrowed
storage and writes the result. The member constructor stays internal.
VC8 LTCG emits a direct call to that member with the same stack
arguments and stack cleanup as the original. The 274-byte member body
has no extra write to EBP+8. After verifying the seven address operands,
it agrees with the previous separate-member experiment. The exported
caller is a link probe for the factory's construction branch.
It does not cover the full factory or replace the pool storage guard.

| Experiment at `0x20cc27a0` | Result |
| --- | --- |
| Linker export with the expected stdcall symbol | Unresolved symbol in run `direct-20261117-001` |
| Member declared stdcall and directly exported | Run `direct-20261117-002` emits 276 bytes with the thiscall symbol, ECX this, and RET 4. The cleanup actions read the context from EBP-0x10. |
| Separate initializer for scalar state before the containers | Runs `state-20261117-001` and `002` have the same 274 bytes after target mapping. The name is cleared after the initial time writes, but the first time write precedes the vptr store. Both variants still use the forwarding wrapper. No source layout conclusion follows from this experiment. |
| Internal member with a direct call | 274 original / 274 linked bytes, matching argument placement and RET 8. Three matching spans cover 225 bytes. |
| Internal member with `/fp:fast` | 266 linked bytes. VC8 specializes the helper to zero, so the helper drops the double argument and returns with plain RET. The original helper consumes the double and returns with RET 8. |

The internal caller run supersedes the inlined entry as the displayed
diagnostic. Instructions still differ in the ranges beginning at
`0x20cc27ce` and `0x20cc2857` (33 and 16 bytes, respectively).
The reconstructed `SchedulerContext::SchedulerContext` is the linked
comparison entry. The standard pilot still exposes its existing
`ConstructSchedulerContext` wrapper. The direct export, scalar state,
and fast FP variants are retained as build snapshots. The comparison
scope is linked code and verified targets, with no runtime execution.
Ghidra was read-only and the current save state was not verified.

Reproduce with a fresh run ID:

```powershell
./scripts/matching/start-simscheduler-pilot.ps1 -RunId simscheduler-constructor-<unique-id> -ContextConstructorLtcg Frame -BorrowedContextStorage -EarlyContextName -ContextContainerInit -InternalContextConstructor
```

- [Live Ghidra constructor and factory](../../../artifacts/simscheduler.dll/context-internal-constructor-ghidra.json)
- [Experiment summary](../../../artifacts/simscheduler.dll/context-internal-constructor-comparison.json)
- [Direct caller and constructor comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-constructor-internal-20261117-001/comparison-20cc27a0.json)
