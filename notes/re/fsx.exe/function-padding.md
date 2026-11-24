# Controlled function-padding experiment

2026-11-02. Status: verified original PE bytes, actual XP builds, MAP/body/import
comparisons, source hashes and generated build commands. Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Result

/FUNCTIONPADMIN:5 reproduces ten previously missing five-byte CC gaps. Twelve
of sixteen selected symbol pairs are adjacent runtime regions in both builds;
all twelve now have the original gap length/content, versus two without the
option. Four pairs have intervening code/different ordering; reported null gaps
are not matches. This supports using the option, not proof of the original
linker command or whole layout. It supersedes the prior untested hypothesis.

| Original region/next entry | Original/control/padded gap |
|---|---|
|ATL fallback01001f1f/101 to real01001f89|5 / 0 / 5|
|ATL real01001f89/4 to selector01001f92|5 / 0 / 5|
|pre_cpp_init01002313/75 to CRT main01002363|5 / 0 / 5|
|CRT main01002363/479 to pre_c_init01002547|5 / 0 / 5|
|pre_c_init01002547/226 to entry0100262e|5 / 0 / 5|
|onexit region01002651/159 to atexit010026f5|5 / 0 / 5|
|GS prologue0100270c/72 to epilogue01002754|0 / 0 / 0|
|GS epilogue01002754/15 to dispatcher01002768|5 / 0 / 5|
|RTC init010027d9/36 to termination01002802|5 / 0 / 5|
|PE validation01002840/41 to section finder01002870|7 / 7 / 7|
|precision region0100299e/41 to zero stub010029cc|5 / 0 / 5|
|zero stub010029cc/3 to cookie init010029d4|5 / 0 / 5|

Every measured gap above has verified CC contents. The four unresolved original
pairs are entry->cookie checker, cookie checker->onexit, atexit->GS prologue and
cookie init->GS failure reporter. Their original gaps are five bytes. Code order
and other layout differences remain; the padding option does not solve them.

## Controls and comparison correction

Actual XP jobs: padding-control-20261102-001 passed all existing gates with the
option omitted. padding-five-20261102-001 linked successfully but stopped at an
old zero-gap comparator assumption. That diagnostic is retained.
padding-five-verified-20261102-001 subsequently passed updated regressions and
completed the InitInstance diagnostic.

Control/padded source-file hashes are identical. Generated build.cmd differs only
by /FUNCTIONPADMIN:5. Compiler/linker SP1, RTM msvcrt.lib, MFC precedence, /GL,
/hotpatch and ICF remain unchanged. scripts/compare-function-padding.ps1 records
this in artifacts/fsx.exe/function-padding-20261102.json. No candidate EXE was executed.

Five comparator checks conflated a body/region length with distance to the next
entry. Require-BodyBoundary now allows only gap0 or5, checks every gap byte for
CC, and reports FollowingGapBytes. Full body/target/metadata checks remain intact.
A negative control changed only the first CC after pre_c_init to NOP and was
rejected: "pre_c_init gap is not CC padding". Result recorded in
artifacts/fsx.exe/function-padding-negative-control.json; altered binary deleted.

All18/1114 application/helper/wrapper checks and separate CRT/ATL gates still
pass at their stated scopes. ShowErrorMessage retains two differing bytes.
InitInstance remains769 original versus777 rebuilt bytes,246 instructions each,
43 differing nonbranch pairs. Padding does not solve these code differences.

## Adopted baseline and remaining work

Host default is now -FunctionPadding 5; -FunctionPadding 0 restores the omitted
option control. The flag is injected into staged build.cmd; no worker restart
or source rewrite is required.

New golden/replay/inventory baseline: vm-share/modules/fsx/archive/31-function-padding,
preserving verified source, commands, request/receipt, EXE/PDB/MAP, disassembly,
headers, diagnostics and comparisons. EXE SHA256:
6F43081D0A0256092F3A10F107F3377E6ABE25F89D7060880C19575361B5FF90.
Baseline30 and all actual controls remain available. No Ghidra database edits
were necessary; its last saved model remains unchanged.

Next: RTTI and constant/writable data, followed by image base, section/import
placement, code order and PE header layout. This is no whole-file match score.
