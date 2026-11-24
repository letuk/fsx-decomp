# API add-on startup and lifetime

2026-11-12, FSX Acceleration. Live verified against the local original
PE files. The retained listings under artifacts/api.dll/api-source-pass-20261107/functions
were checked byte for byte against api.dll at their listed virtual
addresses. The checked startup, teardown and helper listings total
9,415 instruction bytes. Source names describe observed roles.

| Binary and address | Verified behavior |
|---|---|
| main.dll:20949c24, 20949c52 | Indirect calls through IAT 209313f4 and 209313f0 import api.dll ordinals 322 and 323. The API export table maps them to 200fb207 and 200f7809. The enclosing MAIN routine and its place relative to window creation and readiness remain open. |
| api.dll:200fb207–200fb2ac | No stack arguments. Calls 20124051 with mode 0 when DWORD 201f9130 is nonzero, else with 1 then 2. Only after that does it test 201f7d80. Zero runs listener configuration at 200fb243, then the EXE.xml and DLL.xml readers at 200fb290 and 200fb297. |
| api.dll:200fb248–200fb297 | Each reader receives two four-byte slots, tests only the bytes at EBP+8 and EBP+0xc, and uses RET 8. The flags reduce to [201f8e90] != 0 for progress and [201f8fe0] != 0 for prompting. Tests of 201f8e98 and 201f8fe8 select between two nonzero intermediate values. DLL recovery prompts take precedence. Global writer provenance remains open. |
| api.dll:20124051–20124216 | Takes one DWORD and uses RET 4. Reads the Trusted section of the profile at 2020d248 with requests starting at 64 bytes and doubling on truncation up to 1 MiB. Splits entries with _mbsstr and the literal at 200f240c, then uses atoi. Mode 0 selects all parsed entries. Other modes select entries when mode equals -value or value minus 3. Selected keys go to 20130b6b with section Trusted and value 0. This supersedes the retained C view that omitted the mode and scan. |
| api.dll:200fbb94, 200fc2b7 | Both resolve the XML filename with selector 0x1a, check attributes, and call the simprop.dll property reader with the same schema/document GUIDs as SimConnect.xml. A successful disabled flag skips the document or entry. Slots +0x38 and +0x3c count and fetch children by GUID 200f25e4. Entries are visited in document order. Release uses slot +8. The nine add-on-specific GUIDs at 200f2564–200f25e4 were read from the PE. |
| api.dll:200fbdc4–200fc242 | EXE checks attributes and requires the first case-insensitive .exe match to be the final four characters. Missing text defaults to empty. The unsigned combined path/arguments length minus five is checked against 0x7ff0. A document or entry manual-load flag or the second export flag selects FSAlert. Result 0x80 skips the entry. Otherwise the first export flag enables the reusable progress dialog. API ordinal 305 must accept (0, 1, path, NULL). |
| api.dll:200fbdd4–200fbdeb, 200fc0a6–200fc22e | EXE creation flags are 0x04000020 or 0x04000030. A successful console-property read selects the latter even when its output is false. A failed read with nonzero output also selects it. CreateProcessA receives the mutable quoted path, space and arguments, inherits handles and uses the parent's environment and directory. The routine closes both handles after the attempt and does not wait for process exit. An existing progress dialog adds a two-second delay. |
| api.dll:200fc4df–200fcbc6 | DLL applies the .dll suffix and attribute checks, then selects SimConnectStart/Stop or DLLStart/Stop with XML overrides. It derives the launch marker name at 20123917. Registry value 1 under the current user's FSX Launch key selects the recovery prompt. Version fields read at 200fd09e are CompanyName, ProductName and ProductVersion. IDNO skips the entry. Otherwise the ordinary branch uses the document, entry and export flags. |
| api.dll:200fca78–200fcc35 | File versions 10.0.60905.0, 10.0.61110.1, 10.0.61355.0 and 10.0.61357.0 seed the trust call's in/out DWORD with 0x80000000. If the accepted result has bit 0 set, the DLL must report 10.0.61637.0. A mismatch calls 20123ff6 and skips loading. |
| api.dll:200fcc3a–200fcf36 | LoadLibraryExA with flag 1 probes the module without resolving DLL references. A duplicate handle skips normal loading. The probe requires both exports and releases its reference. A nonempty launch marker is set after the successful probe is released, including when exports are missing. The normal load resolves both exports again and requires Start. The optional Panels data export receives panels.dll ordinal 93. SimConnect Start receives one stack argument and must return zero. Legacy Start takes no arguments. The success record is four DWORDs for type, module, Start and Stop. Failure releases the loaded reference. The common tail clears a nonempty launch marker and delays when a progress dialog exists. |
| api.dll:20120f29, 200fb2ad–200fb302 | The original shutdown prefix calls add-on teardown. It first invokes 200f77d0 for service dispatch, then traverses the module table in reverse, calls non-null Stop with no arguments, frees each module and resets storage through 200fd324. |

Pinned SHA256 for api.dll is
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Pinned SHA256 for main.dll is
44400C43A5D07C390A7028654862754730050EE9F7822B37AA207FCE463F8344.

## Source and verification

src/api-pilot/addon_startup.cpp implements ordinal 322 in the factory
source pass and connects ConfigureServiceListenersSourcePilot before
both readers. It shares property-set ownership with listener
configuration, uses standard containers for buffers and the module
list, guards null property sets and only closes non-null EXE handles.
The profile writer, trust decisions and trust-key derivation remain
source boundaries alongside the external FSUI, Panels and SimConnect
calls. The stop body is emitted in the COFF object. Its caller remains
inside the unreconstructed shutdown prefix. Ghidra was untouched,
with no new unsaved edits.

VC8 x86 run api-addon-startup-20261112-003 passed all 20 existing
comparators. The XML reader symbols carry @8 and the FSAlert relocation
agrees with the stdcall import in fsui.dll. The first compile exposed
the DWORD versus unsigned-int pointer mismatch at the path helper.
The successful build contains the correction. COFF SHA256 is
4D37824ABAAE83D26476860B6AE17AC1F27F68A9491882191580F8882D153A8D.
The accepted ledger remains 5,263 code bytes and 288 metadata bytes.
Add-on body matching, full linking and runtime verification remain open.

Suggested Ghidra names, unapplied. In both readers, param_1 →
showProgress and param_2 → forcePrompt based on the modeless creation
and manual-load branches. In 200fbb94, local_41c → documentProperties
and local_428 → entryProperties based on the two output slots. In
200fc2b7, iStack_e18 → startConvention based on dispatch at 200fce0d.
The proposed global names remain provisional.

The next package starts at API ordinal 305, 20123c41, and follows
trust-key construction 20123917, profile lookup/write and the trust
decision UI. MAIN startup orchestration is a separate module boundary.

The trust package now connects both readers to ordinal 305's source
model and reuses the existing profile parser and writer. The WinTrust
provider and protected key path remain boundaries. See api-addon-trust.md.
