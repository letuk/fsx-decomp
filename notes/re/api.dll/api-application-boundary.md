# API Application boundary: initial work-package evidence

Latest correction2026-11-03: Shutdown/RunMessageLoop now have defined target
signatures and separate VTable function-pointer fields; Initialize target and
table argument2 both use uint startupArgument. The earlier retained-char* state
below is superseded. See [api-lifecycle-matching.md](api-lifecycle-matching.md)
for exact source/object evidence, actual edits, backup/readback/save and limits.

## Follow-up: EH, lifetime and saved database corrections (2026-11-03)

This follow-up supersedes the unresolved handled-exception return and unapplied
wrapper/parameter-renaming statements in the initial record below.

EH4 scope table201f0808 contains its header and one record at+16:
enclosing=-2, filter2013e41b, handler2013e42a. Filter extracts exception pointers
and code and calls2013e310; it can select continued execution/search or handling.
Handler2013e42a restores saved ESP, sets try level-2, XOR EAX,EAX and restores
SEH/registers before RET4 at2013e447. Hence handled exceptions return0, while
normal return forwards callback EAX. Not every exception is swallowed. This is
now sufficient for an int-return callback-wrapper ABI model; original typedef,
callback zero-argument calling convention and source syntax remain reconstructed.
Stored parent body remains90 bytes; detached EH code is not silently counted as
a complete stored function. Filter internals and their downstream callbacks
remain separately unverified, and linear decoding must not interpret switch
tables as instructions.

Shutdown callback20120ed5..20121082 is a430-byte sequential decoded span ending
RET. ESI is zeroed at entry and remains the zero carrier. At20121051 EDI invokes
IAT200e19a0=kernel32!HeapFree with (privateHeap,0,application). The tail decrements
2020e058, clears2020de64, and if the count reaches zero calls HeapDestroy via
200e19ac; it clears the heap handle on successful destroy and initialization flag
on this last-owner path. This disproves an immortal-object interpretation;
the global pointer can be reset and later lazily recreated. Full subsystem
cleanup ordering/concurrency correctness is not yet certified.

Actual Ghidra changes, explicitly to api.dll after GZF backup:

- FUN_2013e3c1 renamed InvokeApplicationCallbackWithSEH.
- Wrapper prototype now int __stdcall with /FSXReconstruction/ApplicationCallback*
  parameter; definition int __cdecl ApplicationCallback(void). cdecl for the
  zero-argument callback is an explicit reconstruction choice. Normal EAX dataflow
  now survives decompilation. Function body/memory unchanged.
- Initialize parameter commandLine renamed startupArgument (provisional); the
  legacy char* type remains, with a warning comment pending coordinated VTable
  definition/target-signature review. No silent type propagation claim.
- Added evidence/limit plate comments. Existing VTable fields preserved.

The direct nested callback-prototype tool rejected its parser syntax; a bounded
native Ghidra transaction created the explicit function definition and updated
only this wrapper. Successful readback retained in
artifacts/api.dll/api-boundary-after-edits-20261103.json. Saved revision
1793718640047:636d9e2e:31, unsaved_changes=false. Backup:
artifacts/api.dll/ghidra-backups/api-before-boundary-20261103-1601.gzf. No tab switch.
Evidence:ApiBoundaryEvidence.java, api-boundary-eh-lifetime-20261103.json and
api-boundary-eh-memory-20261103.json; all new raw memory ranges match original
on-disk bytes. See api-first-pilot.md for first actual DLL matching work.

2026-11-03; status: live verified listing/bytes and pinned original PE.
Binary/hash and work-package order: [api-matching-roadmap.md](api-matching-roadmap.md).
Existing EXE call evidence: [../shared/fsx-api-interface.md](../shared/fsx-api-interface.md). Names below are
reconstructed except the actual export name GetApplication.

| Entry / slot | Current evidence | Open matching/ABI question |
|---|---|---|
| GetApplication20120442 / export144 |72-byte body, no stack args; returns2020de64 or allocates4 bytes via20135dd9 after2013623e; installs200f2b24 | Zero-arg cdecl/stdcall indistinguishable here; full lifecycle/thread-safety and allocator failures not established |
| VTable+0 ->2012048a |160-byte Initialize; five stack words; RET14; argument2 stored as DWORD2020de6c | Stored char* commandLine signature is not semantic evidence; EXE supplies m_nCmdShow word |
| VTable+4 ->2012052a |11-byte listing PUSH20120ed5/CALL2013e3c1/RET; EXE ExitInstance uses no explicit args | Target is not a defined function; exact shutdown return/type/lifetime open |
| VTable+8 ->20122817 | Existing verified EXE callback contract and previous DLL listing support one callback arg, RET4, loop20122ae1 | Retain reconstructed RunMessageLoop contract; reverify full loop before body rebuild |
| VTable+c ->20120535 |21 bytes; index0 returns2020e010, other indices zero; RET4; EXE uses HWND in Attach | Trace window-global writers and lifetime; no complete class-size inference |

Stored /ApplicationVTable is16 bytes but only fields+0 Initialize* and+c
GetWindowHandle* are defined; +4/+8 remain untyped. This is separate from the
four verified target addresses. No type edits performed. Allocation request4
supports this factory object's observed storage; it does not prove a recovered
original C++ declaration or every related application's class size.

## Wrapper return contradiction

2013e3c1 is currently `undefined FUN_2013e3c1(void)`; the decompiler renders a
void callback wrapper. The90-byte normal path actually calls [EBP+8] at2013e3fd,
then restores SEH/register/stack state without writing EAX, and RET4 at2013e418.
Initialize20120501 calls it, MOV EDI,EAX at20120506 and TEST EDI,EDI at20120508.
Therefore a definite void-return model loses caller-observed normal-path dataflow.
The normal path forwards callback EAX. Exception-path return behavior is still
unverified: scope table201f0808 and handler20148e1d need inspection. Do not apply
a final int-return prototype or claim original try/catch syntax from this alone.

## Lifetime and naming follow-ups

Fresh xrefs add an important follow-up to the getter:2020de64 is read at2012103f
and written at2012105a outside GetApplication, in currently unassigned listing
code. Inspect these before labeling it an immortal singleton. A global cached
pointer is established; concurrency and teardown semantics are open.

Suggestions only, none applied:

- Initialize `commandLine` -> `startupArgument` (provisional): EXE+4c and DLL
  DWORD-only store contradict a definite char* interpretation. Retain word ABI
  until additional uses are established.
- FUN_2013e3c1 -> InvokeApplicationCallbackWithSEH (provisional): all three
  known calls are Initialize, exit and loop wrappers, with immediate indirect
  invocation and visible SEH setup. Return/exception semantics remain open.
- DAT_2020de68 -> g_applicationInstance (provisional): Initialize stores its
  HINSTANCE argument there. Review other uses before applying a global name/type.

## Completed actions and limits

Created reproducible original PE inventory using the existing PE helper; saved
live function, type, memory and xref snapshots. All three live memory ranges
were compared byte-for-byte against original/api.dll. No DLL compilation or
matched rebuilt body yet. No database edits, name/type changes, save or backup.
Stored revision1793405534817:636d9e2e:3 is saved. Documentation is not a DB backup.

Additional supported naming suggestion after the wrapper prototype correction:
wrapper local iVar1 -> callbackResult, because it receives callback EAX and is
returned directly on the normal path. Suggestion only, not applied. Existing
Initialize local iVar1 could likewise be named initializationResult, based on
the wrapper call and subsequent nonzero test; this name remains provisional
until the initialization callback20120869 is reconstructed.
