# First API.DLL body pilot and heap slice

2026-11-03. Original API hash
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Status: live verified original bytes and actual XP-built comparison fragments.

## API-02 first acceptance

`src/api-pilot/window_handle.cpp` reconstructs GetWindowHandle as an ordinary
__thiscall member. `window_state.cpp` provides independent global storage.
The class is a fragment, not the original complete class or VTable declaration.
The first native VS2005 SP1 .762 build produced the full21-byte original body
at original20120535 / rebuilt200e1000 after exactly one target-validated DWORD
mapping at offset13: original HWND slot2020e010 -> rebuilt200e3000.
Both initial storage values are zero. Branches, return4, frame and all remaining
bytes match. Changed-opcode and wrong-target controls are rejected. This is one
complete normalized body match, not module/runtime or original-source equality.

Run:vm-share/modules/api/runs/api-window-20261103-001. DLL SHA256
CE0C3E803BFCBA376549FA190134CE9748C1766F409CED4941F4C0760614414B.
Accepted comparator:scripts/compare-api-window.ps1. No original body byte patch.

Build/dispatch:scripts/matching/start-api-window-pilot.ps1 and
guest-build-api-window.cmd reuse the existing worker protocol and controller
lock. The worker's fixed output name is fsx.exe; the produced PE is explicitly
/DLL /NOENTRY, copied on the host to api-pilot.dll. This is a transport filename,
not an EXE rebuild or an installable API.DLL. No worker/VM changes were needed.
No DLL startup, factory/export144 or full module export surface is implemented.
The method's decorated export keeps this small pilot reachable. No entry point
or candidate execution; no CRT/MFC EXE default/library/order adoption.

API runs now live under vm-share/modules/api/runs, completed job snapshots under
modules/api/jobs. Canonical names are api-pilot.dll/map/pdb; original worker
fsx.* files/logs/disassembly/guest sources are under worker-output. Relocation
and original hashes are recorded in modules/api/relocation-20261103.json;
captured commands and binary/debug content were preserved.

Each run retains actual staged and guest-copied source, source hashes, generated
command/request/receipt/log, DLL/PDB/MAP/disassembly/headers, comparison, status
and output-hashes.json. Command identifies compiled translation units; staging
hashes can also include files not compiled by that particular profile.

## API-03 initial dependency reconstruction

Fresh listing/ABI snapshot:artifacts/api.dll/api-heap-live-20261103.json. Reconstructed
`heap.cpp` and `heap_state.cpp` implement the observed initializer2013623e and
allocator20135dd9. Initializer requests an8MiB private heap and LFH mode2 for
private/process/CRT heaps, increments the shared call count and marks initialized.
Allocator initializes when needed, selects zero-fill flag8, stores output,
returns-1 for failed mode4, otherwise retries once and reports resource8 before
TerminateProcess on repeated failure. The return after TerminateProcess is
retained because the original body contains it; no invented noreturn ABI.
The260-byte error text array is a reconstructed minimum based on LoadStringA's
capacity, not a recovered original global size.

Globals2020e050/54/58 and2020e028 have original initial DWORD0 (recorded in
artifacts/api.dll/api-heap-original-state-20261103.json). Type/name/lifetime semantics
remain reconstructed. Shared count increment/decrement is observed; ownership
invariants and concurrency are not yet fully established.

Actual native run api-heap-20261103-001, SHA256
E5477ECC4DD79784EB73367A49FF6FB2885F20EBF21ACF06B273616F27BA83F1.
Initializer original144/rebuilt146 bytes; allocator original174/rebuilt174.
Both are NOT certified matches. Example residuals: original initializer loads
initialized flag into EAX, TESTs it and reuses zero EAX for HeapCreate arguments;
reconstruction emits CMP memory,0 and two PUSH0, adding2 bytes. Allocator uses
different heap-pointer/output-pointer temporary registers. Equal length is
not instruction/body equality. GetWindowHandle's full21-byte gate still passes.

One concrete context question was tested next: identical sources with /GL plus
/LTCG instead of native /GL-. Actual run api-heap-gl-20261103-002, SHA256
DC3FB008F83B2E883F63E0BCBC50230EF8E90F49ED4E6E063D141F3C50CA959B.
Source hashes are identical; generated command changes only those two tokens.
Both DUMPBIN symbolic instruction sequences contain124 instructions and are
identical (diagnostic only). GetWindowHandle gate/negative controls pass again.
The hypothesis did not improve the heap schedules: stopped after the single
GL control, no third permutation. No preference for GL or original flag claim.
Plans/results:artifacts/api.dll/api-heap-context-{plan,result}-20261103.*.

## Next bounded work

Keep first native window pilot as the accepted21-byte reference. Heap native
and GL fragments are retained controls, not promoted matching baselines. Before
another source trial, inspect original caller/global-use context or a specific
source-level lifetime explanation for the MOV/TEST versus CMP form. Do not add
volatile or arbitrary register constraints merely to force bytes.

API-02's GetApplication portion remains pending. Its heap dependencies now have
reviewed source candidates, but the four-method VTable integration and dispatcher
definitions must be provided with honest per-body evidence rather than fake
dependency implementations. API-04 wrapper/outer-method reconstruction is a
useful next independent code question while the heap match context is unresolved.
No claim that the entire eight-package roadmap is complete.
