# Original PE stack values and bounded cut

2026-11-02. Verified original fsx.exe SHA256
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735
against actual XP stack-original-20261102-001. /STACK:0x40000,0x2000 reproduces
original reserve262144 and commit8192 bytes, replacing1048576/4096 defaults.
Source hashes are unchanged from34; sole build.cmd delta is the /STACK token.
Worker job/PDB paths still differ; no raw-layout difference is attributed solely
to the stack option. Source/compiler/library/other-option settings unchanged.

All existing regressions and InitInstance diagnostics pass. Known ShowError2
bytes and InitInstance769/777,43 nonbranch differences remain. New startup gate
checks requested Original/Default stack mode and records actual/original fields.
Actual34 rejected under Original expectation; actual35 rejected under Default
expectation (artifacts/fsx.exe/stack-negative-controls.json). No candidate was executed.

Default -StackMode Original; explicit Default preserves the omitted-option
control. Golden/replay/inventory35-original-stack preserves actual source,
request/receipt, command/log, EXE/PDB/MAP, diagnostics and comparisons. SHA256:
465CFF301B8FE7DA2B084110C1D9878E23D992297D0CA796F4FF06454D35119E.
Baseline34 retained. Inventory/vtable checks pass35. No Ghidra changes needed;
last saved original model unchanged. Worker ready, no active job.

Cut requested by the user's remaining usage: no further experiment started.
Next: investigate a supported way to reproduce basename-only PDB debug metadata,
then separate observed PE flag/version changes. Preserve code/import/data-order,
virtual-size and certificate/resource differences as open; no whole-EXE match.
