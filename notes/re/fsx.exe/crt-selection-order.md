# CRT startup selection and placement

2026-11-03. Status: live verified, original fsx.exe SHA256
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735,
pinned RTM msvcrt.lib B1456D9A7E17865E3DC35663E0377BE4C7B8BE97A372210CD064F9F5635DFFEA,
and actual XP SP1 compiler/linker outputs. No candidate runtime execution.

## Static-name limit, verified rather than assumed

Read-only COFF archive inspection finds six COMDAT .text sections in
build\intel\dll_obj\crtexew.obj. By contrast, sehprolg4gs.obj has a single
non-COMDAT .text section (87 bytes) and alloca16.obj a single non-COMDAT .text
section (44 bytes). COMDAT presence alone does not establish /ORDER eligibility
for a static name.

The actual order-crt-static-probe-20261103 job appends the three known static
names and public startup/exit symbols to the 88-symbol MFC response. LINK emits
exactly three LNK4037 warnings for _pre_cpp_init, ___tmainCRTStartup and
_pre_c_init. They still exist in the MAP static-symbol table, but /ORDER cannot
resolve them here. The unchanged mandatory warning gate rejects the build.
The public entry moves ahead of these statics; the result is not adopted.
This is a specific negative result, not evidence that their code is missing.

Evidence: scripts/inspect-crt-order-members.ps1,
artifacts/fsx.exe/crt-order-members-20261103.json, rejected job log/MAP/status. The archive
inspector is read-only and records member hashes, section sizes and flags.

## Positive single-option experiment

Starting with the accepted 88-symbol MfcThunks response, add only
/INCLUDE:_WinMainCRTStartup. No static CRT symbol is added to /ORDER, no object
is patched/extracted/recompiled and no CRT source is reconstructed in this step.
All compiled input hashes, including the response, are identical to the MFC
control; the only build.cmd difference is the stated INCLUDE option. MFC library
precedence and the pinned CRT library remain intact.

Actual job: order-crt-entry-include-20261103-001. EXE SHA256:
D720C7FB11CFD8E27E642C710FCE1DCD137436FF12C99B7E441A9C46453F75E2.
The crtexew contribution now precedes cookie-check/onexit contributions. This is
observed member-selection/placement behavior, not proof of the original command.

Examples (original -> rebuilt):

| Symbol | Original VA | Rebuilt VA |
|---|---|---|
| _pre_cpp_init | 01002313 | 010022F3 |
| ___tmainCRTStartup | 01002363 | 01002343 |
| _pre_c_init | 01002547 | 01002527 |
| _WinMainCRTStartup | 0100262E | 0100260E |
| @__security_check_cookie@4 | 0100263D | 0100261D |
| __onexit | 01002651 | 01002631 |
| _atexit | 010026F5 | 010026D5 |

compare-crt-selection.ps1 verifies 36 previously mapped runtime/framework symbols
at uniform -32-byte offsets, including these statics, SEH/stack/RTC/image helpers,
precision/cookie/report routines and selected later wrappers. Existing exact body
and reference gates remain required separately; MAP positions alone are not a
body match. All 88 ordered application/ATL/MFC symbols retain control placements.
Import identities/hints remain exact, while within-DLL slot differences fall from
128 to 103. Thus selection influences IAT ordering, but does not complete it.

## Adopted default and actual default build

FunctionOrder Auto now resolves MfcThunks for MfcWindows, Default for console.
CrtStartupSelection Auto resolves EntryInclude for MfcWindows with pinned RTM,
Default otherwise. CallbackLinkage Auto remains External for MfcThunks.
Explicit -CrtStartupSelection Default retains the old selection control;
-FunctionOrder ApplicationAll -CrtStartupSelection Default reproduces39 settings.
The negative CrtStartupProbe mode is diagnostic only. Synthetic gap remains0.

Actual default job: order-crt-selection-default-20261103-001, all existing gates
and InitInstance diagnostics pass. Golden/replay/inventory:
vm-share/modules/fsx/archive/40-crt-entry-selection. EXE SHA256:
323E3F7F56CD0A4423C08605DECCDFF5B8B3CF610EC9FA90670A198B3625F393.
Archive includes actual compiled source/response hashes, request/receipt,
build command/log, EXE/PDB/MAP, headers/disassembly and comparisons.39 and trials
are retained. Inventory and vtable checks pass40; worker ready.

Reports: artifacts/fsx.exe/crt-selection-20261103.json,
crt-selection-default-20261103.json, mfc-thunk-selection-default-20261103.json,
application-crt-selection-default-20261103.json. The MFC comparator permits only
the explicitly selected INCLUDE command difference while retaining FF25/IAT,
manifest, source and prior-prefix checks.

## Remaining limits and next step

Correction from the later complete scan: import-selection.md verifies all80
thunks, including debugger hook, already original-32 in40. The following earlier
claim about remaining thunk positions is superseded; IAT ordering remains open.

The uniform -32 offset is still the unresolved gap issue; no synthetic gap object
is enabled. This is selected symbol placement, not proof that the complete code
range, intervening thunks/padding or import order match. Some CRT import thunks
still occupy different positions, including debugger-hook placement. Next map
these remaining thunks by exact imported identity and relate them to archive
selection; preserve the established prefix and all current body gates. Then
investigate .data placement (+20 virtual bytes). Known ShowErrorMessage two-byte
and InitInstance769/777-byte,43 nonbranch-instruction differences remain.
Application body count remains18/1114. No original source identifier/link flag or
whole EXE equivalence claim follows.

No Ghidra edit, save, navigation, type/name or api.dll change; database save/backup
state unchanged. No new variable rename is supported by this placement work.
No VM UI/autostart or installed toolchain changes.
