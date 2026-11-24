# Flight Simulator X Matching Decompilation

This project studies how Microsoft Flight Simulator X works by reconstructing
its native x86 code in C++. The goal is to rebuild `fsx.exe`, `api.dll`,
`sim1.dll`, and other modules to match the original binaries byte for byte.

The reference installation is **FSX Gold Edition with Acceleration**.
The local originals for the three modules above report file version
**`10.0.61637.0 (FSX-Xpack.20070926-1421)`**. Build and comparison scripts
use SHA-256 hashes to identify their reference inputs.

Work proceeds module by module. Current results cover individual functions,
code ranges, and data structures. A match for one scope does not establish
equivalence for an entire DLL or executable. See the
[current status](notes/re/STATUS.md) and [evidence notes](notes/re/SOURCES.md).

## Repository layout

```text
.
├── src/                  Reconstructed C++ and bounded experiments by module
├── notes/re/             Module findings, shared notes, provenance, and status
├── scripts/              Analysis, VM build, comparison, and regression scripts
├── tools/re-dashboard/   Local progress dashboard and its generator
├── original/             Local reference binaries, ignored by Git
├── ghidra/               Local Ghidra project, ignored by Git
├── vm-share/             Local XP worker, build queue, and run results, ignored by Git
└── artifacts/            Local exports, comparison evidence, and backups, ignored by Git
```

## From disassembly to a match

1. Inspect the original disassembly and decompiler output alongside callers,
   referenced data, and imports. The project has used Ghidra for this work.
   Equivalent evidence from other tools is welcome.
2. Reconstruct the behavior in idiomatic C++ using C++98/C++03-era conventions
   and the observed ABI. Use period SDK headers, Windows APIs, and static
   library members where the binary evidence supports them.
3. Build on a Windows XP worker with the Visual Studio 2005 x86 compiler and
   linker. **Visual C++ 2005 RTM (VC8) is the leading candidate** for the
   original toolchain. The project also uses SP1 in comparison builds.
   The exact original tool and library revisions, flags, and link recipe
   remain open and may vary by module.
4. Compare the rebuilt output with the pinned original. Record exact byte
   matches and normalized matches separately. For a normalized match, validate
   each relocation or address difference against its intended target.
   Track native COFF results, linked PE results, and runtime verification
   separately. Preserve the build inputs, command, hashes, and remaining
   differences so another contributor can reproduce the comparison.

Builds use original binaries and development tools supplied locally. Choose
a module workflow from the [build and comparison guide](scripts/matching/README.md).
The [toolchain notes](notes/re/fsx.exe/build-environment-factors.md) describe
the evidence and open questions for `fsx.exe`.

LLMs have assisted with analysis, source generation, and comparison tooling.
Match claims are backed by original listings, recorded builds, and comparator
results that contributors can review independently.

## Local dashboard

The dashboard is a simple offline view of progress by module. It uses local
inventories and comparison evidence to show function status, source locations,
and original and rebuilt listings where available. With those inputs in place,
run this from the repository root.

```sh
node tools/re-dashboard/generate.cjs
```

Open `tools/re-dashboard/dist/index.html` in a browser. The
[dashboard guide](tools/re-dashboard/README.md) describes its inputs and views.
Improvements to the dashboard and Git integration are welcome.

## Contributing

Contributions are welcome, including documented findings, cleaner
reconstructions, and reproducible build experiments. Keep each module-specific
commit focused on one module. Describe the function, data structure, build
change, or cleanup in the commit message. For matching work, record the
original address, build profile, verified scope, and discrepancies in the
relevant notes. Keep shared tooling changes in their own commits.

See the [project instructions](AGENTS.md) for evidence and build conventions.

## About this project

This is an independent educational project for learning how the software works.
It is unaffiliated with Microsoft. Original game files, proprietary toolchains,
and raw local analysis exports are kept outside Git. Use your own local copies
for analysis and builds.
