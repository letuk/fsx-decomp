# Project instructions

- Write project documentation in English.
- Preserve evidence from the original binaries, Ghidra and previous build runs. Resolve contradictions explicitly and mark superseded findings.
- Store analysis exports, comparison evidence and Ghidra backups in `artifacts/<binary-name>/`. Use `artifacts/shared/` for cross-module inventories and shared toolchain evidence.

## Reverse-engineering notes

- Before analysis, read the latest relevant entries in `notes/re/STATUS.md` and the relevant topic documents under `notes/re/<binary-name>/` for each module in scope. Use `notes/re/shared/` for relevant cross-module or general workflow notes. Read and edit only the documents and sections relevant to the current work.
- Put new single-module topic files in the directory named after the binary, creating it as needed. Include the module name in each new topic filename. Put cross-module topics in `notes/re/shared/` and include all participating module names in the filename, for example `fsx-api-interface.md`. General workflow notes may use a descriptive filename without module names.
- Consult the relevant entries in `notes/re/SOURCES.md` for provenance and verification limits. Treat historical chat findings as historically reported until independently verified.
- Record each finding in the relevant topic document with the binary, address or full type path, evidence and status (`live verified`, `historically reported`, `hypothesis` or `superseded`). Label reconstructed names and minimum layouts as such.
- Record significant changes actually made in `notes/re/CHANGELOG.md`. After meaningful analysis or reconstruction work, update the relevant entries in `notes/re/STATUS.md` with progress, open questions and the known Ghidra save state.

## Ghidra and MCP

- Use Ghidra MCP to inspect the target program and its current state before editing. Specify the program explicitly when multiple programs are open.
- The user normally controls Ghidra tab changes; do not switch programs unprompted.
- Preserve established names and types. Verify VTable function types and target signatures separately. Read back significant edits. Project notes are separate from Ghidra database saves and backups.
- Suggest useful variable renames with the current name, proposed name and brief evidence. Label provisional interpretations and distinguish suggestions from applied renames.

## Source reconstruction

- Reconstruct C or C++ from verified listing, calling convention, type and data evidence. Use the module's existing source and build profile. Keep experiments bounded and preserve the source snapshot, build command, hashes and results for comparison.
- Aim for readable, idiomatic C or C++ that a developer using the original toolchain could plausibly have written. Use structured control flow, descriptive names and types supported by the evidence. Recover operations, ownership and error handling from the observed behavior. Treat inferred names and source structure as reconstructions.
- Annotate each reconstructed function definition with its verified original binary and entry VA, for example `// Original: fsx.exe 0x010019e0`. Label reconstruction-only helpers and wrappers explicitly. If one source definition represents multiple original entry points, list the verified addresses and identify their roles.
- Use C++ classes when the evidence supports instance state, ownership, inheritance or virtual dispatch. Express abstract interfaces with virtual and pure virtual members where supported by the evidence, and model construction and destruction with C++ lifetime semantics. Reserve C-style structs and explicit function tables for verified C interfaces or concrete ABI constraints. Document any such constraint and verify vtable layout, slot signatures and target signatures separately.
- Organize new source into cohesive translation units where clear subsystem or ownership boundaries are supported by the evidence. Keep closely related functions and small helpers together with their owning code. Preserve source layout and linkage where verified build evidence shows they affect matching.
- Keep inline assembly and `goto` out of reconstructed source. Avoid instruction-by-instruction translations, artificial temporary variables, opaque macros and casts used solely to reproduce register allocation. Use supported intrinsics or calling-convention annotations where the behavior or ABI warrants them.
- Pursue byte matches through plausible source forms and supported build options. Exclude hard-coded machine-code bytes, binary patching and dummy operations added solely to force a match.

## VM builds and verification

- Dispatch builds from the host through the shared `vm-share/queue/` using the existing module workflow. In the Windows XP VM, launch `vm-share/RUN-WORKER.bat` once if needed and leave the worker open. Run one controller at a time under the shared lock. See `scripts/matching/README.md` for dispatch and comparison, and `vm-share/README.md` for artifact locations.
- Check the worker receipt, build log and artifact type. Compare the output with the identified original binary using the relevant comparator and rerun established regression checks. Record the verified scope and remaining differences. Track COFF object matches, linked image matches and runtime verification separately.
- For each reconstructed function or explicitly scoped code range, use the build and comparison workflow to pursue an exact byte match or a normalized match with individually validated address or relocation fields. Either result is sufficient for the stated scope. Let the evidence and the remaining plausible approaches guide further trials. If differences remain when work on a target stops, preserve the best build and comparison evidence and record those differences in the topic notes and dashboard.
- For changes to worker output naming, follow the shared worker plan in `vm-share/README.md`.
- Keep function inventories and comparison reports available to the local dashboard. For results it cannot import directly, provide the address, scope and evidence link in the module's dashboard data as described in `tools/re-dashboard/README.md`. Regenerate the dashboard after updating its inputs.
- For reconstructed functions, retain the original address, the Ghidra name when available and the reconstructed source symbol in the dashboard mapping. Add an explicit source mapping when automatic resolution is unavailable or ambiguous.
- When adding or updating a rebuilt listing in the dashboard, also export the corresponding original function listing via Ghidra MCP and retain it in the module inventory or dashboard JSON. Verify the target module and entry address. After regeneration, check that both listings are present. If the original listing is unavailable, record the reason in the comparison scope.
- For new or revised global data, VTables and types, retain the verified module export or extend the module's dashboard data as described in `tools/re-dashboard/README.md`. Identify globals by original address and types by full Ghidra path. Record the verified scope, links to supporting evidence and any provisional fields. Regenerate the dashboard and check the affected entries.
