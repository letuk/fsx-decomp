# simscheduler.dll event dispatcher at 0x20cc1400

## VC8 RTM normalized match

Live verified in `simscheduler.dll` at `0x20cc1400`. Two VC8 RTM
x86 LTCG builds of the reconstructed `SchedulerGroup10Callback`
match all 88 bytes after resolving the three direct calls and the
object address. The comparator checks both PEs for the sole HIGHLOW
fixup, at the immediate for the EAX load of `0x20cc8370`. The
RTM output includes `xor eax,eax` at original `0x20cc141e`.

The control uses identical source, compiler flags and link arguments
with the installed VC8 SP1 tools. It emits 86 bytes. The remaining
differences are the three short-branch distances and the three changed
relative call distances. The PE linker version and Rich build field
50727 are shared by both toolchain revisions. This comparison establishes
the RTM match for the dispatcher and leaves the original build revision
of the rest of the DLL open. The RTM tools were extracted from the
local VS2005 DVD and run from the XP job directory with the installed
headers and libraries.

The original listings show two callback pointer references and one
direct call to each handler. Modeling the two pointer references with
the actual API ordinals under SP1 left the 86-byte body unchanged.
Removing the extra time-state caller from that probe caused SP1 to
specialize away the EAX object argument. The normalized RTM profile
retains the extra caller in the test fragment. This result covers the
dispatcher body. The handler bodies, complete DLL link and runtime
were outside the comparison. The nine native scheduler export gates
and the GPS regression passed on the installed SP1 profile. Ghidra was
read-only, save state unverified.

- [RTM build and comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-rtm-20261117-002/comparison-20cc1400.json)
- [SP1 control and byte delta](../../../artifacts/simscheduler.dll/event-dispatch-rtm-control.json)
- [Live Ghidra listing and references](../../../artifacts/simscheduler.dll/event-dispatch-rtm-ghidra.json)
- [RTM tool identities and media members](../../../artifacts/shared/toolchain-media/vc8-rtm-x86-tool-identities.json)

## 2026-11-17 SP1 linked dispatcher with handler profiles

Live verified for `simscheduler.dll` at `0x20cc1400`. The VC8 SP1
linked probe combines the ordinal 13 and 14 Flight imports, the
propagation copy profile and the time-state copy profile with a
second caller to keep the time-state object argument live. The
reconstructed `SchedulerGroup10Callback` passes the load payload
and time-state flag in ESI, the time-state object in EAX, and the
save payload on the stack. The comparison validates the three
direct call targets and the EAX object address against the PE and
link map. The second caller is an ABI probe, so verification is
scoped to this linked fragment. Runtime was not tested.

The 86-byte switch supersedes the 81-byte linked diagnostic below.
At original `0x20cc141e` the rebuilt code reuses EAX, which is zero
after the final event subtraction, and omits `xor eax,eax`.
After accounting for that instruction and its effect on the three
conditional branch displacements, the remaining instructions match
with the four address operands resolved. The 88-byte if/else chain
changes the event tests, block order and register allocation.
An explicit if for the flag also emits 88 bytes but replaces
`setne` with a branch and changes the save payload's temporary to
EAX. Disabling global optimization drops stack realignment and
changes the dispatch shape. Computing a bool before the switch
repeats the 86-byte result.

- [Best linked comparison, source snapshot, command and receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-handlers-20261117-006/)
- [Live Ghidra listing](../../../artifacts/simscheduler.dll/event-dispatch-handlers-ghidra.json)

Nine export gates and the GPS regression passed. The standard native
COFF dispatcher diagnostic remains 78 bytes. Ghidra was read-only
and its save state is unverified.

### 2026-11-17 instruction selection follow-up

Live rechecked `simscheduler.dll` at `0x20cc141e`. Local VC8
optimization for speed reproduces the 86-byte dispatcher byte for
byte. Optimization for size emits 73 bytes and shares one epilogue,
uses DEC for the second event subtraction, and pushes the save
payload directly from memory. Both reuse EAX from the final event
subtraction. The probe source was restored after the trials.
Ghidra was read-only and its save state is unverified.

- [Speed trial](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-xor-t-20261117-001/)
- [Size trial](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-xor-s-20261117-001/)

## Earlier 81-byte C++ probe

The third C++ approach linked the dispatcher and the modeled
handlers `FUN_20cc4790`, `FUN_20cc48d0` and `FUN_20cc1b30`
under VC8 x86 SP1 `/GL /LTCG`. The separate no-entry DLL
fragment uses exported storage for external Flight transfer
dependencies. The fourth internal definition models
`FUN_20cc1a40`. Their layouts and names are reconstructed.
Handler body matches and runtime equivalence remain open.

The 81-byte dispatcher recovers stack alignment and internal
register arguments. The linked output passes the load payload
and time-state flag in EDI instead of ESI, specializes the
time-state handler to the global object instead of passing EAX,
and reuses the event subtraction's zero result. The save call
retains its stack shape. The comparison independently resolves
all three direct calls from the original PE and the linked
fragment and verifies the rebuilt targets against its map.

The guest's VC8 environment supplies SDK libraries. Adding
`kernel32.lib` resolved the first link's /GS support dependencies.
The SDK affects that link boundary. The dispatcher has no SDK
headers or SDK-owned layout dependencies. The probe retains
`/O2 /hotpatch /GS /MD /GR-` and uses local EBP preservation.
The listing gives no volatile access requirement for the event
or object input. Adding volatile solely to force loads would
change the source's access semantics and was not used.

The C++ matching attempts were the default COFF frame profile,
local EBP preservation, and the linked LTCG profile. The link
dependency fix retried the same LTCG approach. The linked diagnostics
above supersede this 81-byte probe. The 78-byte source
profile remains available for the native COFF export regressions.
All nine SimScheduler export gates and the GPS regression pass.
Ghidra read-only, save state unverified.

- [Linked comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-ltcg-20261116-002/comparison-20cc1400.json)
- [Rechecked original listings](../../../artifacts/simscheduler.dll/event-dispatch-ltcg-ghidra.json)

### Earlier C++ COFF controls

The maintained implementation uses a C++ switch. Two VC8 x86 XP
builds emitted 72 bytes with the default frame-pointer setting and
78 bytes with local EBP-frame preservation, against 88 original.
The 78-byte build restores the hotpatch and EBP prologue and is
retained as the current diagnostic.

Both C++ builds use stack arguments for the load and time-state
handlers and omit the original stack realignment. The declarations
describe rebuilt handlers and are incompatible with the original
binary entries at `0x20cc4790` and `0x20cc1b30`. All nine export
comparisons and the GPS regression passed. Ghidra read-only, save
state unverified.

- [Current 78-byte diagnostic](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-cpp-20261116-002/comparison-20cc1400.json)
- [72-byte control](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-cpp-20261116-001/comparison-20cc1400.json)

## Original behavior and superseded assembly implementation

Live verified in simscheduler.dll. The group `0x10` callback
`FUN_20cc1400` takes an event and a 32-bit payload on the stack
and returns with `ret 8`.

- Event `0x25` passes the payload in ESI to the Flight load handler
  `FUN_20cc4790`, which returns with plain RET.
- Event `0x26` passes the payload on the stack to the Flight save
  handler `FUN_20cc48d0`, which returns with `ret 4`.
- Event `0x171` passes `DAT_20cc8370` in EAX and the payload
  converted to a 0/1 flag in ESI to `FUN_20cc1b30`. That handler
  captures the current time when the flag becomes active and feeds
  the stored time to `FUN_20cc1a40` when it becomes inactive.

This supersedes the first-pass decompiler rendering that omitted the
register arguments. The source names are reconstructed. The first
matching approach uses a naked wrapper with x86 inline assembly to
reproduce the mixed handler ABIs and stack alignment.

The complete 88-byte VC8 x86 COFF body matches after verifying one
DIR32 object reference and three REL32 call targets. The two wrappers
that reference this callback also match with the now-defined COFF
symbol. All nine prior SimScheduler checks and the GPS regression pass.

- [Original Ghidra evidence](../../../artifacts/simscheduler.dll/event-dispatch-ghidra.json)
- [Comparison report](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-20261116-001/comparison-20cc1400.json)
- [Worker listing](../../../vm-share/modules/simscheduler/runs/simscheduler-dispatch-20261116-001/worker-output/disassembly.txt)

The three handler bodies remain outside the match scope. Link and
runtime verification are pending. Ghidra read-only, save state
unverified. Next candidate `FUN_20cc1b30`.
