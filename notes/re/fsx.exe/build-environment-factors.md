# Build host, target OS and other matching factors

2026-11-03. Explanation/provenance note, not a recovered original build recipe.
Target under discussion is the pinned original x86 fsx.exe; active build uses
XP with installed SP1 compiler/linker/MFC and pinned RTM CRT library.

## Host OS does not identify target configuration

Acceleration's release-era association with Vista does not establish the original
build-host OS. We have not recovered that host identity. Current XP builds emit
the observed OS/image header versions5.2 via explicit linker options while
preserving subsystem4.0; these fields cannot independently identify the host.

On-disk Windows FileVersionInfo read of the pinned original reports
10.0.61637.0 (FSX-Xpack.20070926-1421) for file/product version. That is observed
resource metadata consistent with the user's Xpack context, not a host-OS or
independent historical-build-environment proof. Resources were only read.

The host can affect tooling installation, selected INCLUDE/LIB paths, scripts,
generated files, path/locale/time metadata and any host-dependent build logic.
Inference for this project: fixing exact tool/input/configuration identities is
currently a stronger lead than replacing XP solely because of release timing.
This is not a proof that every possible tool invocation is OS-independent.

Microsoft documents that SDK header revisions and explicit WINVER/_WIN32_WINNT
definitions control conditional API declarations:
[Using the Windows Headers](https://learn.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers).
Current reconstructed startup explicitly defines _WIN32_WINNT0x0501; that is our
input, not recovered original macro provenance. A target-version definition,
installed SDK and runtime OS are separate variables. Runtime Windows versions
affect executed API paths/behavior; they do not by themselves show how original
machine instructions were generated.

## Factors beyond the compiler executable

| Factor | Matching relevance and current evidence |
|---|---|
|Linker version and settings|Section/COMDAT/function placement, folding, imports and PE fields. Current merge/base/padding/header/order trials give direct measured effects.|
|CRT/MFC/ATL libraries and service packs|Already-compiled member bodies, startup registrations and imports. The RTM CRT library change removed the extra XIY initializer; no host change was needed. Headers/MFC remain SP1.|
|Windows/other SDK headers, macros and ABI settings|Inline code, conditional definitions, types/packing/calling convention. DirectX SDK can matter where its code/headers are actually included; a dynamic DLL-name string alone is not proof of a build-host SDK.|
|Translation-unit/caller context and optimization|Inlining, lifetime/register allocation, branch/code scheduling and reachable code. /GL/LTCG exposes cross-module information; current getter/TU experiments already show context matters.|
|PGO profile data|Can affect optimization and placement; possible factor, not established for this FSX binary. No original training data recovered.|
|Object/library input order and retention|Can influence which members/aliases are selected and their placement; ordinary import order remains unresolved despite equal contents.|
|Build environment and release processing|PDB path/GUID/timestamps, generated resources, checksums/signing and possible post-link processing. Basename-path repeats directly demonstrated path-length effects; original gap/release cause remains unknown.|

General optimization explanation is supported by Microsoft's
[/GL reference](https://learn.microsoft.com/en-us/cpp/build/reference/gl-whole-program-optimization)
and [/LTCG reference](https://learn.microsoft.com/en-us/cpp/build/reference/ltcg-link-time-code-generation).
Modern option lists are not assumed to be valid VC8 switches without testing.
Target architecture remains x86; host architecture/OS is not inferred from that.

No host replacement is currently required by evidence. A later controlled host
comparison is possible after preserving identical tool binaries, inputs/options
and relevant environment, but it is not a diagnosis for the remaining TEST/CMP
or InitInstance differences. No OS-switch experiment was performed here.
