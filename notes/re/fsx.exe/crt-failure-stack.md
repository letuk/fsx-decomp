# CRT GS-failure reporter and stack probes

2026-11-02. Status: live verified fsx.exe listing/types, PE/MAP/import/data
comparisons, real XP build, typed Ghidra data and saved readback. Original SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.

## Complete comparisons

| Original | Rebuilt baseline30 | Bytes | Result |
|---|---|---:|---|
|0x01002a6d ___report_gsfailure|0x00402054|260|normalized-exact,33 checked fields|
|0x01002790 __alloca_probe_16|0x00401f60|22|normalized-exact,checked tail jump|
|0x010027a6 __alloca_probe_8|0x00401f76|22|normalized-exact,checked tail jump|
|0x01002bb0 __alloca_probe|0x004023d0|43|raw-exact|

scripts/compare-crt-failure-stack.ps1 checks every other byte, cookie/complement
symbols, debugger-hook/import identities, register/record field offsets, initial
storage including BSS, and the actual exception/context pointer pair. No broad
relocation mask. These are CRT identifications, separate from the18/1114 counted
application/helper/wrapper bodies. Source, flags/libraries and baseline30 unchanged.

The reporter saves register/control state, sets exception code0xc0000409 and
noncontinuable flag1, stores the fault address and retains the cookie pair.
It checks debugger presence, calls the CRT debugger hook, resets/invokes Windows
unhandled-exception filtering, then calls GetCurrentProcess/TerminateProcess.
DLL implementations/runtime behavior are excluded. Existing noreturn/prototype
annotations remain; the complete checked body includes its trailing LEAVE/RET.

## Verified record data and specialized ABI

Existing standard x86 Ghidra Windows layouts independently verified: _CONTEXT716,
_EXCEPTION_RECORD80, _EXCEPTION_POINTERS8. Code accesses match Eax/Ecx/Edx/Ebx/
Esi/Edi, segments, EFlags/Ebp/Eip/Esp/ContextFlags and exception code/flags/address.

| Data role | Original | Rebuilt | Initial-data evidence |
|---|---|---|---|
|Exception record|0x01004058|0x00405138|80 zero bytes|
|Debugger flag|0x010040a8|0x00405188|zero; follows exception record|
|Context record|0x010040b0|0x00405190|716 zero bytes including BSS|
|Exception pointer pair|0x010012d0|0x004032e8|links to those exception/context records|

This models standard Windows data, not original FSX class names/sizes. Placements
still differ; there is no whole-data/linker-layout equality claim.

Aligned probes adjust the requested allocation for16-/8-byte alignment and
tail-jump to the base probe. Its specialized compiler ABI takes size in EAX,
touches pages in0x1000 steps, changes ESP and transfers the return address.
Existing unknown prototypes are preserved; no ordinary C/fastcall signature guessed.

## Integration and controls

New mandatory MfcWindows CrtFailureStack gate; real XP job
crt-failure-stack-20261102-001 passes every earlier body/CRT gate. Known application
nonmatches unchanged. Same-binary baseline30 comparison.json extended, with
crt-failure-stack-comparison.json preserved. No candidate/original EXE executed.
Negative controls changed the stack page step and exception-record pointer;
both rejected. Results: artifacts/fsx.exe/crt-failure-stack-negative-results.json.
Altered copies deleted. PowerShell scripts parsed successfully.

## Applied Ghidra work

api.dll remained user-selected; explicit fsx.exe targeting, no navigation/API edits.
Pre-edit backup: artifacts/fsx.exe/ghidra-backups/fsx-before-crt-failure-stack-20261102.gzf
(2217461 bytes). scripts/ghidra/DocumentCrtFailureStack.java checks the live body
hash, standard layouts, zero data/pointer links, overlaps/user symbols and existing
function boundaries; defaults to read-only preflight.

Applied _EXCEPTION_RECORD at0x01004058, _CONTEXT at0x010040b0 and
_EXCEPTION_POINTERS at0x010012d0. No user-defined symbols overwritten. Automatic
default labels now follow types: _EXCEPTION_RECORD_01004058, _CONTEXT_010040b0,
_EXCEPTION_POINTERS_010012d0. This is distinct from an explicit analyst rename.
Added evidence/data-role/ABI comments. Existing code, function bodies/prototypes,
cookie-check injection and g_crtManagedImageFlag preserved. Readback:80/716/8 bytes.
Saved revision1793647925456:5e7b737f:196 reports unsaved_changes=false.

Inventory208=73 internal+135 external declarations,137 mixed rows. Executable9728
bytes:5011 instructions/3162 defined data/1555 undefined;8 nonzero undefined
ranges remain. Typing the pointer pair classified four more data bytes, not code.
These are model counts, not matching percentages. Worker ready/no active job.

Suggestions only, not applied: _CONTEXT_010040b0 -> g_crtFailureContext;
_EXCEPTION_RECORD_01004058 -> g_crtFailureExceptionRecord;
_EXCEPTION_POINTERS_010012d0 -> g_crtFailureExceptionPointers. Evidence: verified
register fields and actual pointer links. Descriptive roles, not original names.

Next: full __onexit registration/locking/EH comparison, then ATL and remaining
data/RTTI/linker-layout work.
