# Handoff for API startup and add-on loading

Prepared 2026-11-12. Workspace: `G:\Projects\Reversing\fsx-decomp`.
Read AGENTS.md, notes/re/STATUS.md and notes/re/SOURCES.md. Write project
notes in English and communicate with the user in German. Work through
complete topics to the next functional boundary and report the next package.

## Position in simulator startup

- `fsx.exe` calls `GetApplication` and enters the API application through
  vtable +8 in its Run override at `01001d00`. See exe-vtable-inventory.md.
- Live verified in the local `main.dll` PE: the instruction at `20949c24`
  is `FF 15 F4 13 93 20`, an indirect call through IAT `209313f4`.
  That import is `api.dll!#322`. The containing MAIN routine and its
  position relative to API window creation and readiness remain open.
  This is module-boundary evidence only. There is no MAIN reconstruction
  in this handoff.
- `api.dll` ordinal 322 at `200fb207` first invokes `20124051` with
  mode 0, or with modes 1 then 2 when `201f9130` is zero. If
  `201f7d80` is zero, it then calls `200fb4d7` for SimConnect listener
  configuration, `200fbb94` for the EXE.xml path, and `200fc2b7` for
  the DLL.xml path. The file names were read from the original PE.
  Retained decompilation shows CreateProcessA in the first add-on path
  and LoadLibraryExA/LoadLibraryA in the second. The specific gating and
  load policies still need instruction-level review.

## Resume here

1. Reconstruct `api.dll:200fb207` and verify the mode selection, startup
   gates, argument widths and meaning of the two flags forwarded to the
   EXE.xml and DLL.xml routines. Connect the existing
   `ConfigureServiceListenersSourcePilot` at the verified call site.
2. Follow the immediate helpers through the add-on startup topic.
   Trace each XML reader, activation gates, and process/module lifetime.
   Record any further subsystem calls at the relevant boundary. Use the
   retained ASM and C under artifacts/api.dll/api-source-pass-20261107/functions.
3. Keep evidence and source attribution by binary. `main.dll` is the
   caller, `api.dll` owns the package, and `simprop.dll` supplies the
   property-set interfaces. Read api.dll/api-service-listener-configuration.md,
   api.dll/api-service-application-registration.md, and api-initialization-flow.md.

## Verification baseline

The most recent build is `api-service-listener-config-20261112-001`.
The VC8 x86 factory object passed all 20 existing comparators.
The accepted ledger records 5,263 code bytes and 288 metadata bytes.
The recent service work adds source reconstruction. New body matches,
complete DLL linking and runtime verification remain open. Ghidra was
unchanged. Verify the target before edits and let the user control tab switches.

Build with `scripts/matching/start-api-window-pilot.ps1 -FactoryObject -SourcePass -RunId <unique-id>`.
Run `scripts/matching/test-api-source-pass.ps1 -BuildDirectory <run-directory>`.
Record new evidence in the topic notes, significant changes in CHANGELOG.md,
and the current boundary in STATUS.md.
