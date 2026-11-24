# CRT pre_c_init identification and comparison

Follow-up: [pre-cpp-init-and-exe-gaps.md](pre-cpp-init-and-exe-gaps.md) completes
the three earlier helper placeholders, verifies pre_cpp_init/RTC termination,
and applies the approved g_crtManagedImageFlag rename. Placeholder/count and
suggestion descriptions below are the preserved earlier state.

2026-11-02. Status: live verified original memory, read-only pseudo decoding,
PE/MAP/import comparisons, bounded Ghidra edits and actual XP worker build.
Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Role and complete body

Original 0x01002547..0x01002628 is a complete 226-byte, 56-instruction CRT
pre_c_init counterpart. Actual slot 0x01001254 registers it in the C initializer
table; CRT _initterm_e runs that table before the C++ initializer table.
Rebuilt baseline30 resolves _pre_c_init at0x00401df5 from msvcrt:crtexew.obj.
All226 bytes match after normalizing27 independently validated address fields.
The selected RTM CRT already supplies this implementation; no new application
C++ source or manual assembly replacement was needed.

Behavior supported directly by decoded instructions and verified imports:

- Checks its own MZ/PE headers and optional-header magic; tests CLR data-directory
  presence and stores a managed-image flag at0x01004030. Both PE32 and PE32+
  header branches remain in the actual x86 machine code. Ghidra folds several
  branches away because this image's concrete headers are known; that does not
  mean those instructions are absent from the binary.
- Calls MSVCR80 __set_app_type(2), selecting GUI application startup behavior.
- Encodes -1 through _encode_pointer, storing the same encoded value in the
  onexit end/begin globals0x01004494/0x01004498.
- Copies default file/commit mode globals0x01004050/0x0100404c into the CRT
  locations returned by __p__fmode/__p__commode; copies _adjust_fdiv into0x01004488.
- Invokes the RTC initialization walker, default argument-setup fallback and
  default floating-point precision helper. Conditionally installs the matherr
  fallback according to0x0100401c, and calls _configthreadlocale(-1) only if
 0x01004018 equals -1. Returns integer0.

The managed flag has two verified zero-comparison consumers in CRT startup,
not one: the ordinary and exception-related exit paths. This is not a complete
certification of CRT startup's control flow or exit implementation.

## Dependency and reference evidence

| Original region | Bytes | Independently checked evidence |
|---|---:|---|
|0x010027d9 RTC Initialize walker|36|normalized body; both range pointers and empty RTC sentinel interval verified|
|0x0100299e default precision helper|41|normalized body; _controlfp_s and _invoke_watson DLL/name identities checked|
|0x010029cc shared zero-return stub|3|raw XOR EAX,EAX; RET equals rebuilt __setargv and __matherr bodies|

The linked rebuilt __setargv and __matherr symbols share the zero-return body.
Original pre_c_init likewise both calls0x010029cc without explicit arguments
and supplies0x010029cc as the math-error callback. Do not assign a unique original
source identifier or a single definitive parameter list based on this folded
body. The comparator maps the direct argument-setup call to __setargv and the
callback pointer to __matherr separately; it supports separate rebuilt addresses.

scripts/compare-pre-c-init.ps1 validates actual PE-header-relative addresses,
import identities, symbol-resolved globals and their initial values (including
zero-filled BSS), the private managed flag's startup consumers, initializer slot,
body end, and these three dependency bodies before masking any address bytes.
All other bytes, including branch encodings, literals and return, must match.
No whole CRT/EXE, external DLL implementation, runtime behavior, original compiler
revision or original FSX source-name equality is claimed.

Separate library identification: the established18 application/helper/wrapper
body count remains18/1114. pre_c_init226 bytes and its checked dependencies are
reported separately, like the earlier AfxInitialize/__allmul library findings.

## Verification and integration

- Baseline30 RTM-library comparison passes. Saved report:
  artifacts/fsx.exe/pre-c-init-comparison.json, also retained alongside baseline30.
- Installed-SP1 control baseline29 also passes this particular body check;
  artifacts/fsx.exe/pre-c-init-sp1-control.json. Thus this routine alone does not
  distinguish RTM vs SP1 library/compiler identity. The separate XIY discrepancy
  remains the reason for the selected RTM CRT context.
- Real XP/SP1 worker run pre-c-init-20261102-001 passes the new mandatory
  MfcWindows PreCInit gate and all18 established body regressions. Application
  source/compiler/linker/library choices unchanged. Existing near-matches remain
  open; no new InitInstance diagnostic sweep was performed.
- Negative control changed the rebuilt PUSH2 application-type literal to3 in a
  disposable local PE copy. The checker rejected non-reference bytes. Result:
  artifacts/fsx.exe/pre-c-init-negative-result.txt. Copy deleted, never executed.

Baseline30 remains selected; no new golden binary is necessary for this library
identification. Its existing comparison.json now includes a matching PreCInit
report for that same preserved binary, not a report copied from another EXE.

## Ghidra change and limitations

fsx.exe was user-selected, api.dll also open; no program switches or API edits.
Before editing, exported artifacts/fsx.exe/ghidra-backups/fsx-before-pre-c-init-20261102.gzf
(2192775 bytes). scripts/ghidra/DefinePreCInit.java defaults to read-only preflight:
checks program/base, exact live body SHA256
F8AAE926BA1E3E8B1E26F59B5CD479CD90336AC0B8EDE531C9AE0DD27A21CCBD,
registration pointer, existing symbols/functions/data and instruction boundaries.
Apply mode disassembles only the226-byte address set, creates explicitly bounded
int __cdecl pre_c_init(void), and adds an English evidence/scope comment.
No existing meaningful name/type was overwritten. Readback confirmed the bounds,
signature, comment and decompilation. Saved fsx.exe; final readback
1793625481881:8bd7bef0:291 reports unsaved_changes=false.

Observed follow-on model changes include three one-byte function placeholders
at0x010027d9/0x0100299e/0x010029cc; these do NOT represent complete Ghidra function
bodies despite the independently verified raw PE dependency bodies above. They
remain FUN_* names with unresolved signatures. Do not conflate PE-body evidence
with stored Ghidra coverage. Completing their bounded definitions is a useful
next cleanup; the folded stub needs its multiple roles documented before typing.

Refreshed model counts:202 total=67 internal stored functions+135 external
declarations;146 unassigned instructions in64 ranges;135 mixed-origin inventory
rows. Executable9728 bytes:4857 decoded instructions,3150 defined data,1721
undefined. Fourteen nonzero undefined ranges remain. These counts are model
state, not recovered original function counts. No candidate binary executed.
Worker ready; no queued/active job remains.

Suggestions only, not applied:

- DAT_01004030 -> g_crtManagedImageFlag: PE CLR-directory presence is stored here,
  then tested by CRT startup. Descriptive reconstructed name, not an original name.
- Decompiled puVar1 -> pCrtModeValue: reused pointer returned by __p__fmode and
  __p__commode. This is a suggested decompiler-local rename, not a stored edit.

Next bounded work: finish the three dependency placeholders using their verified
boundaries/alias roles, then inspect pre_cpp_init or proceed to API behavior.
