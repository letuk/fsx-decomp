# Manual recompilation workflow

2026-11-01. Proposed manual workflow; no build toolchain installed or tested.

## Verified inputs and limitations

Live file inspection: all 14 files in original/ have PE Machine 0x014c and optional-header linker version 8.0. Most FSX modules report 10.0.61637.0 (FSX-Xpack.20070926-1421); xuipc.dll reports 10.0.61261.0 and sldlext.dll reports 6.0.5469.59. Do not assume identical toolchain settings across modules.

ASCII scans find MFC80.DLL, MSVCR80.dll, Microsoft.VC80.CRT, Microsoft.VC80.MFC and version text 8.0.50608.0 in fsx.exe. api.dll includes MSVCR80.dll, MSVCP80.dll and VC80 CRT text. These are string observations, not parsed imports/manifests. Together with linker 8.0 they support a VS2005/VC8-family starting point. Exact compiler build, service pack and options remain hypotheses; runtime version does not uniquely identify the compiler.

On-disk SHA-256:

- original/fsx.exe: 61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735
- original/api.dll: 9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003

Ghidra live check: only fsx.exe open, base 0x01000000, 188 functions. get_function_documentation for 0x01001760 returns hash daffc3740373c76a73d312bb78d592e0945c3debfac07e8f4d881803db288d59. Its algorithm/semantics are unverified; do not equate it to SHA-256 or infer file patching from the differing strings. Establish input identity through executable hash properties and pilot-byte comparison before matching by address. No Ghidra edits/navigation/save. Last documented unsaved state was not freshly checked.

## Environment and first experiment

Use a snapshot of a Windows VM with a legitimately acquired VS2005 C++ toolchain, x86 SDK and matching headers/libraries. Include MFC development components for later EXE reconstruction; redistributable DLLs are insufficient. Keep compiler candidates (RTM/SP1 as available) separate and record actual compiler/linker versions and hashes. Edit/analyze on the host; build inside the VM. A portable compiler directory or Wine is an alternative to investigate, not a verified setup. Modern MSVC/clang-cl may help semantic experiments but do not replace VC8 for matching code generation.

1. Pin originals and verify Ghidra identity; separately save/back up the database. Preserve original files.
2. Reconstruct ordinary C++ for fsx.exe CheckWindows51SP2OrLater 0x01001760 and sibling 0x01001800 using ../fsx.exe/startup.md and original instructions. Names are reconstructed. Use period SDK integer 64-bit mask types. Let the compiler emit cookie machinery rather than transcribing decompiler artifacts.
3. Start with a batch build invoking vcvars32, cl and link. Link a comparison-only fsx.exe plus its own PDB with pilot functions kept referenced. This need not run as FSX. Record full commands, environment and output hashes. Add a build generator later, keeping flags explicit.
4. Evaluate a small option matrix: /O1 vs /O2, /Ob, /Oi, /Oy, /GS, /MD vs /MT and /GL plus /LTCG. These are experiments, not recovered flags. Change one dimension at a time. Produce symbols without switching to Debug instrumentation; verify effects of debug-information options.
5. Compare final linked instructions, registers, stack frame, control flow, constants and referenced data. Normalize only justified relocation/address differences through a reference mapping. Track raw equality separately. Keep source/settings/results for each attempt.
6. Diagnose compiler/settings, ABI/types/layouts, source expression, library/header version and missing caller context before modifying source. Validate semantics separately; a diff score is not a semantic proof.
7. Expand to InitInstance, exports and DLL implementations module by module. A runnable EXE additionally requires MFC/startup integration, vtables, resources/manifests and runtime testing with the full installation.

The formatter at 0x01001590 is a later pilot: verified EDI/EAX inputs are consistent with optimization, but /GL or /LTCG is not proven. The Ghidra __fsx_edi_eax_varargs convention is analysis metadata, not original C++ syntax. Matching may require caller and callee compilation together. StrSafe provenance/version remains provisional.

## Comparison and dependencies

Evaluate [reccmp](https://github.com/isledecomp/reccmp) on a linked pilot/PDB. It maps source annotations to original addresses and compares rebuilt binaries. Upstream explicitly says newer MSVC support is in progress: validate VC8/PDB compatibility before adoption. An original PDB is not required for address-annotated matching. Fallback: DUMPBIN/Ghidra disassembly with manual mapped-range comparison. [objdiff](https://github.com/encounter/objdiff) primarily compares relocatable objects; a linked original EXE does not supply original object boundaries/relocations automatically.

Inventory ordinary/delay imports, exports/ordinals, resources, manifests and debug records, then supplement with LoadLibrary/GetProcAddress analysis. Import tables alone miss explicit dynamic loads. Static libraries/header implementations require fingerprint/source comparison. Reuse matching CRT/MFC/SDK libraries rather than rewriting compiler/library machinery. SDK import libraries or carefully reconstructed .def/import libraries can serve ordinary imports; explicit dynamic loading requires ABI-correct declarations. Existing Application types are minimum reconstructions, not proven complete layouts.

Track acceptance levels separately: compiling source; reviewed semantic equivalence; normalized function match; module code/data match; full file equality. Full-file equality adds link order/layout, resources, imports, timestamps/debug paths, padding and possibly signatures. Matching instructions do not prove verbatim source recovery; identifiers, comments and many source formulations are lost.

Primary references: [Microsoft /GL](https://learn.microsoft.com/en-us/cpp/build/reference/gl-whole-program-optimization), [internal calling conventions](https://devblogs.microsoft.com/oldnewthing/20141010-00/?p=43873), [older VS downloads](https://visualstudio.microsoft.com/vs/older-downloads/).

## Toolchain pilot update (2026-11-01)
Guest-produced vm-share/toolchain-check.txt verifies XP 5.1.2600, cl 14.00.50727.42, link 8.00.50727.42, successful optimized x86 smoke build/PDB and execution exit 0. Initial MFC check had a path-joining defect; missing MFC is not established. Corrected check staged. Added reconstructed src/fsx/version_checks.cpp (fsx.exe 0x01001760/0x01001800), separate comparison harness and manual build-version-pilot.bat. Experimental baseline /O2 /GS /MD /Zi without /GL; original flags not proven. Pilot not compiled yet. Ghidra MCP connection refused, so no fresh function/byte or saved-state verification; source relies on documented previous live findings. No Ghidra edits/save. Host inputs staged in vm-share/pilot-input; guest outputs expected in vm-share/pilot-output. The pilot fsx.exe must never replace the original installation executable.
