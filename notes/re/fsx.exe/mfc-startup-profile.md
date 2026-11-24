# Windows/MFC startup profile

2026-11-02; status: actual XP/SP1 builds, static PE/MAP/import checks and all
established body comparisons. No candidate binary was executed.

Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.
Console/ICF control is preserved baseline26. New profile uses the same source
file contents (all staged hashes equal baseline26), compiler14.00.50727.762,
compile flags and /OPT:ICF. It changes selected translation units and subsystem,
not merely one linker bit: this is a startup/caller-context experiment.

## Profiles

- MfcWindows (new default): guest-build-mfc.cmd compiles
  startup_context_pilot.cpp with /GL and application_interface_pilot.cpp without
  /GL; links /SUBSYSTEM:WINDOWS. MFC supplies its actual WinMain startup object.
- ConsolePilot (retained control): guest-build.cmd also compiles the old console
  main and synthetic string-copy/format caller translation units, and links
  /SUBSYSTEM:CONSOLE. Select explicitly with -BuildProfile ConsolePilot.

The unselected pilot files remain in source/input snapshots for reproducibility;
their presence does not mean they were compiled. The exact selected compilation
commands and object list are printed from the actual guest-local build.cmd and
retained per job. BuildProfile is recorded beside folding mode and command hash.
The worker itself is unchanged and requires no restart.

## Verified startup

Added verify-startup-profile.ps1 and made it a build-acceptance gate:

- Original and new MFC build have PE subsystem2 (Windows GUI).
- New entry0x00401edd equals MAP _WinMainCRTStartup; original entry0x0100262e.
- MAP identifies _WinMain@16 at0x004025a7 from mfcs80:appmodul.obj.
- Actual wrapper bytes are E9 to the AfxWinMain thunk; that thunk is FF25 to
  MFC80 ordinal1207, independently present in the original import table.
- No _main, CopyErrorText_Pilot, RunFormatterPilot or g_pilot symbols in the
  linked MFC MAP. Build command excludes those translation units altogether.

This verifies the selected static startup structure, not CRT-body equality or
runtime behavior. The console control is accepted only as ConsolePilot and is
correctly rejected when asserted to be MfcWindows (negative check recorded in
artifacts/fsx.exe/vtables/startup-profile-negative.txt). Original Ghidra import naming
was read explicitly from fsx.exe while api.dll remained current; no navigation.

## Outcomes

First real profile build mfc-startup-20261102-001 passed all13 existing body gates,
all helper/OLE/EH/48+28-table sharing checks, input hashes, compiler and receipt.
Removing synthetic helper callers did not lose the actual SDK helper bodies:
the reconstructed application caller context retains their established matches.

Adopted MfcWindows as the controller default and retained ConsolePilot explicitly.
Fresh default build mfc-default-20261102-001 exercised the new startup gate plus
all regressions and InitInstance diagnostics. It ends diagnostic_complete,
because InitInstance remains nonmatching; this is not a build failure or a new
exact InitInstance claim. All13 previously certified function bodies remain
matched, totaling1001 bytes at their documented scopes. No new body is counted.

InitInstance is still777 vs769 bytes,246 instructions, frame0x554,231 non-branch
positions with188 normalized-equal and43 different positions. ShowErrorMessage
still differs at0xdb/0xdc after16 target fields. Vtable sharing differences0.
Thus the tested startup/caller-context change did not fix the open encodings;
it does not rule out other translation-unit/LTCG or source contexts.

Golden baseline27-mfc-windows-startup, EXE SHA256:
E69591F5EFA88B113D8160BEDADB2A261A46AF2598A5E27E3C76E7529BB6B650.
All source, actual build command, log, receipt, EXE/PDB/MAP and reports preserved.
Replay fixture, default inventory and shared-folder baseline now point to27.
Previous console controls26/25 remain intact. Canonical C++ files unchanged.

## Limits and remaining questions

This is a closer application build context, not a runnable FSX replacement.
Resources, full original CRT/global-registration metadata, external DLL bodies
and original source names/translation-unit packaging remain outside certification.
The application-getter definition still lives in the deliberate non-GL boundary
file; its former unused callback-retention helper is not a synthetic caller in
this linked build. That boundary can be tested separately if needed.

No Ghidra edits, save or backup. Only open-program/import inventory read; database
save flags were not refreshed. XP worker ready, no run active. Next investigate
remaining source/translation-unit optimizer context or continue identifying
non-framework EXE bodies; do not claim the two near-matches solved by startup.
