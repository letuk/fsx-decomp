# Controlled identical-code-folding experiment

2026-11-02; status: verified actual XP/SP1 builds and offline PE comparisons.
Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Controls

NOICF control: preserved baseline25-application-dispatch-matches, SHA256
B66678A07A9C034483CB95035E94BAA73A68CEBB7AE4FCE05853BB2A37395791.
Experimental real build: icf-20261102-001. All staged source file hashes equal
baseline25; compiler14.00.50727.762 and compile flags unchanged. Linker option
changed only /OPT:NOICF -> /OPT:ICF. Additional diagnostic TYPE build.cmd prints
the actual guest-local command into the build log; it does not affect codegen.
Controller records LinkFolding and BuildCommandSHA256 in each request.
Debug timestamps/PDB identities are not controlled; no whole-file reproducibility
or compiler-only binary equality claim is made.

## Results

All13 normalized/raw function-body acceptance checks, existing OLE28-slot and
exception metadata checks, source integrity and receipt checks pass with ICF.
ShowErrorMessage retains the same two differences; InitInstance remains777 vs769
bytes,246 instructions/frame0x554 and the same non-branch diagnostics. No new
function-body match is credited to this flag change.

The application48 slots and filter28 slots were pooled. For every pair of slots
we compared whether the two pointers share an address within each binary:

| Build | Observed slots | Slot pairs | Sharing-pattern differences vs original |
|---|---:|---:|---:|
|NOICF baseline25|76|2850|4|
|ICF experiment/default|76|2850|0|

Original has22 shared-address pairs. Four relations differed under NOICF:
App+08/App+10, App+08/Filter+10, App+10/Filter+08, Filter+08/Filter+10.
With ICF all agree, including the original shared RET4 Serialize/Dump bodies.
Both shared and nonshared relationships were checked; this is not just a single
successful alias example. It remains a topology comparison, not proof of callee
behavior, literal table bytes, original linker command, or whole-file equality.
An equivalent original folding mechanism is still possible.

Reusable script compare-vtable-aliases.ps1 emits hashes, mismatches and scope.
It reproduces four differences on baseline25 and zero on the fresh ICF default
build. Future ICF runs also gate acceptance on zero sharing-pattern differences;
explicit NOICF runs retain the diagnostic without imposing the ICF topology gate.

## Adopted default and baseline

After reporting the result to the user, adopted /OPT:ICF in canonical
scripts/matching/guest-build.cmd and the controller's LinkFolding default.
Explicit -LinkFolding NOICF still stages a source-pinned control; source/compiler
settings are unchanged and the actual staged command is retained and logged.

Fresh canonical-default build icf-default-20261102-001 ends matched for the
reported lifecycle/dispatch bodies, with all13 existing body gates passed.
SHA256 A83BB7E93860C044E1B928B4D66492AA9AF4EF923F7C2C923A8BA146448F6EAB.
Golden baseline26-icf-vtable-sharing-matches preserves executable, PDB/MAP,
source, build command/receipt/log and comparisons. The alias comparator was added
after this build and executed offline on these exact outputs; alias evidence
was appended to comparison.json without altering binary or historical status.
The new gate is wired for subsequent runs; no additional compile is implied.
Baseline25 retains the prior NOICF control. Replay fixture and default inventory
now point to26. No original files or earlier baselines removed.

No Ghidra calls/edits/navigation/save or candidate execution in this experiment.
The XP worker performed compilation/linking only and remains ready; no run active.

## Next investigation

The linker-folding mismatch is resolved at the observed-table scope. Remaining
InitInstance/ShowErrorMessage differences are unaffected. A concrete next context
experiment is a Windows/MFC startup target rather than the current console
comparison harness with synthetic helper callers. Original startup reaches
AfxWinMain; retained pilot harness uses main and /SUBSYSTEM:CONSOLE. Test an
alternate startup without executing it, preserving all13 body gates, before
further arbitrary source rewrites or compiler-version guesses.
