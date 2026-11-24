# Static initialization and teardown matching

Follow-up: [crt-library-context.md](crt-library-context.md) resolves the additional
XIY entry through the RTM CRT library. The SP1-table difference and baseline29
below describe the preserved control. Current default/baseline30 reproduces both
C and C++ table roles/order; full CRT body equality remains unproven.

2026-11-02. Status: live verified original Ghidra listing/table reads, real XP
VS2005 SP1 builds, and static PE/MAP/import comparisons. Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Actual initialization order

Original CRT startup at 0x01002363 calls _initterm with the range
[0x01001238,0x0100124c). The actual entries, including boundary sentinels, are:

| Slot | Target | Supported role |
|---|---|---|
|0x01001238|0|null start sentinel|
|0x0100123c|0x01002313|CRT pre_cpp_init|
|0x01001240|0x01002d16|MFC application-state initializer|
|0x01001244|0x01002ce0|construct global CWnd and register its destructor|
|0x01001248|0x01002d00|construct global application and register its destructor|
|0x0100124c|0|null end sentinel|

Baseline28 already generated matching wrapper bodies, but its XCU entries had
application before window. Canonical source now declares g_mainWindowPilot before
g_fsxAppPilot, producing the original order through ordinary C++ global objects.
Wrapper addresses alone did not establish this order. The application destructor
is registered after the window destructor; their relative atexit order is therefore
application before window. This is a static registration finding, not an executed
full shutdown trace or a claim about all intervening callbacks.

## Five wrapper bodies

Final real worker run: static-default-20261102-001.

| Original body | Rebuilt body | Bytes | Validated address fields |
|---|---|---:|---:|
|0x01002ce0 global window initializer|0x00402650|22|4|
|0x01002d00 global application initializer|0x00402670|17|3|
|0x01002d16 MFC application-state initializer|0x00402681|18|2|
|0x01002d40 global window destructor|0x004026a0|10|2|
|0x01002d50 global application destructor|0x004026b0|46|7|

All 113 bytes match after normalizing only 18 validated address fields. Existing
ordinary C++ source/SDK already produced these bodies; the source change corrects
their registration order rather than introducing five hand-written wrappers.
There are now 18 explicitly counted matched application/helper/wrapper bodies,
1114 bytes total, at their individual raw or normalized comparison scopes.
The SDK AfxInitialize body and CRT atexit adapter are checked as dependencies,
not additionally counted. This is not whole-file equality or original source proof.

compare-static-glue.ps1 checks every non-reference byte, mapped globals, event
offset +0xa4, derived vtable, application constructor, destructor callback pointers,
MFC constructor/destructor imports, CloseHandle, and atexit target. It checks
the actual rebuilt XCA/XCZ and XIA/XIZ MAP symbols and verifies that CRT startup
passes those bounds to import-identified _initterm/_initterm_e calls.

The 18-byte atexit adapter at original 0x010026f5 matches after checking its call
to original __onexit 0x01002651 and rebuilt MAP __onexit. Both rebuilt symbols
come from msvcrt:atonexit.obj. Original __onexit's existing Ghidra library match
and live decompilation show the MSVCR80 _onexit/__dllonexit registration paths.
The internal __onexit implementation and its EH metadata are not body-certified.
The 38-byte AfxInitialize dependency at 0x01002c0d is separately normalized-exact,
with module-state and setmbcp import identities checked.

## Explicit remaining CRT difference

Original _initterm_e range [0x01001250,0x01001258) contains a null sentinel and
pre_c_init 0x01002547, with null end sentinel at 0x01001258. The rebuilt table
also contains ___CxxSetUnhandledExceptionFilter in .CRT$XIY, from
msvcrt:unhandld.obj. That extra entry is absent from the original checked table.
The comparator reports FullCInitializationTableMatchesOriginal=false; it does
not silently call the whole CRT initialization matched. The cause and appropriate
compiler/library context remain open. Do not remove the initializer or patch the
binary merely to hide this difference.

## Builds, regressions and preservation

- static-order-20261102-001: real source-order experiment, all previous 13 bodies
  retained; five wrappers and C++ table order pass. InitInstance remains 777 vs
  769 bytes, 246 instructions, frame 0x554, 43 non-branch positional differences;
  ShowErrorMessage retains its two-byte difference. Its diagnostic reports belong
  to that run, not to the final binary below.
- static-default-20261102-001: fresh real build through integrated
  StaticInitialization target; status matched at the stated wrapper/order scope.
  All previous regression gates and ShowErrorMessage two-byte baseline pass.
- Negative control: baseline28 still passes the five bodies but is rejected with
  -RequireOriginalOrder. Output retained in artifacts/fsx.exe/static-glue-order-negative.txt.

The new gate runs on every MfcWindows build. Explicit target example:
`./scripts/matching/start-search.ps1 -RunId <unique-id> -Target StaticInitialization -MaxCandidates 1`.
ConsolePilot remains a separate control profile and cannot claim this target.

Golden baseline: vm-share/modules/fsx/archive/29-static-initialization-matches.
Final EXE SHA256:
65E417DEE0AAE8FB0471023078EED88067E4357CBBED08FE3F35A0CCD48E1F71.
Archive includes actual compiled source snapshots, command, request, source
hashes, receipt, log, EXE/PDB/MAP, disassembly and comparison reports. Replay
fixture and current inventory defaults now reference29; earlier baselines retained.
No candidate EXE or DLL stub was executed. Worker is ready; no active job remains.

## Ghidra changes and save state

Before editing, fsx.exe was user-selected and api.dll also open. Live fsx.exe
revision 1793559605675:8bd7bef0:3 was saved. Exported a pre-edit GZF backup:
artifacts/fsx.exe/ghidra-backups/fsx-before-static-glue-20261102.gzf (2191266 bytes).

Applied five new, explicitly bounded void __cdecl functions on existing listing
instructions, with reconstructed descriptive names:

- LAB_01002ce0 -> InitializeGlobalMainWindow
- LAB_01002d00 -> InitializeGlobalFSXApp
- LAB_01002d16 -> InitializeMfcApplicationState
- LAB_01002d40 -> DestroyGlobalMainWindow
- LAB_01002d50 -> DestroyGlobalFSXApp

These names describe verified behavior; they are not recovered original names.
Added English plate comments on those bodies and the two CRT table starts.
Preserved established g_mainWindow/g_fsxApp names and types. No API edits or tab
switching. Script scripts/ghidra/AnnotateStaticInitialization.java preflights
program/hash/instruction boundaries/function conflicts and applies in a transaction.
Readback verified names, signatures, bodies and comments; saved fsx.exe. Later
readback revision 1793623738938:8bd7bef0:69 reports unsaved_changes=false.

Refreshed inventory observes two additional stored import thunks beyond the five
explicitly created wrappers: CWnd destructor 0x01002014 and __CxxFrameHandler3
0x010027c2. These are observed follow-on model changes, not two more source/body
matches. Current model: 195 total functions = 63 internal stored functions plus
132 external declarations; 145 unassigned instructions in 63 ranges. Inventory
still has 130 mixed-origin distinct-address rows; undefined nonzero ranges remain
15. No claim of 195 original local functions or newly decoded runtime coverage.

Suggestion only, not applied: DAT_0100437c -> g_mfcApplicationStateInitialized
(provisional descriptive name). Evidence: stores AL from AfxInitialize(0,0x800),
and the SDK counterpart is char _afxInitAppState. Preserve the one-byte storage;
this is not evidence for a four-byte BOOL global or an original FSX variable name.

## Next bounded investigation

Investigate why the current linked CRT contributes the additional XIY exception
filter initializer, using original import/body evidence and actual library/member
provenance before any compiler-flag experiment. Then review the remaining undefined
runtime ranges or proceed to API behavior. Do not resume blind source variations
for the two application near-matches without new evidence.
