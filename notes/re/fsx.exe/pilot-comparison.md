# Version helper comparison

## Experiment 1: 2026-11-01

Status: live verified from guest build outputs and read-only Ghidra assembly. No Ghidra edits/navigation/save.

Compiler 14.00.50727.42, linker 8.00.50727.42. Flags: /O2 /GS /MD /Zi, no /GL; separate helper and harness translation units. Link /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:NOICF /MACHINE:X86 /SUBSYSTEM:CONSOLE. MFC afxwin.h presence verified by corrected probe; previous missing-header message was a probe defect and is superseded.

| Reconstructed function | Original fsx.exe | Pilot fsx.exe | Length |
| --- | --- | --- | --- |
| CheckWindows51SP2OrLater | 0x01001760 | 0x00401000 | 144 / 144 bytes |
| CheckWindows52SP1OrLater | 0x01001800 | 0x00401090 | 144 / 144 bytes |

Original addresses/body ends verified through get_functions disassembly, explicit program fsx.exe. Pilot locations from fsx.map and DUMPBIN disassembly. Equal lengths are coincidental and do not establish a match.

Both original helpers begin MOV EDI,EDI; PUSH EBP; MOV EBP,ESP; SUB ESP,0xa8. Original cookie is XORed with EBP and stored at EBP-4. OSVERSIONINFOEXA starts at EBP-0xa8. Pilot omits this frame setup, allocates 0xa0 bytes with ESP-relative addressing and XORs its cookie with ESP. Epilogues differ correspondingly. Both retain ESI for VerSetConditionMask; the observed three mask calls, selected field flags/comparisons, structure values, VerifyVersionInfoA call and return/cookie flow agree at the reviewed structural level. This is not a dynamic equivalence test or a normalized exact match score.

Hypotheses: /Oy- should recover an EBP frame; /hotpatch should recover the hotpatch-compatible entry. Neither option has yet been demonstrated against this target. No need to change source or compiler revision before testing these focused differences. Linked address references naturally differ and require semantic mapping rather than treating every immediate as interchangeable.

Baseline outputs are preserved in vm-share/pilot-baseline-o2/. Experiment 2 is staged in build-version-pilot.bat, adding /Oy- and /hotpatch with unchanged source and linker options. Two distinct observed code-generation properties are tested together; their individual effects must be identified from the result, not inferred from a score improvement alone. Experiment 2 has not run yet.

Ghidra reports revision 1793546795596:8eb5e96a:3, unsaved_changes=false. This supersedes older unsaved-state reports for the current database session. No database save/backup performed by this review.

## Experiment 2: 2026-11-01
Live verified from guest output: /O2 /Oy- /hotpatch /GS /MD /Zi. Both functions remain 144 bytes at 0x00401000/0x00401090. Entry MOV EDI,EDI and EBP frame now agree with originals. Exact PE byte comparison of both ranges, excluding five identified address operand fields, leaves seven differing bytes at function offsets 0x07, 0x1d, 0x3c, 0x46, 0x50, 0x5b, 0x76. Address fields excluded: [0x0c,0x0f] security cookie global; [0x25,0x28] memset relative call; [0x2b,0x2e] VerSetConditionMask IAT; [0x7d,0x80] VerifyVersionInfoA IAT; [0x88,0x8b] security-check relative call. Pilot identities independently provided by MAP/DUMPBIN; original identities based on documented/live original listing. These exclusions are explicit target mappings, not blanket masking of constants.
Remaining difference is frame allocation 0xa0 vs original 0xa8 and six structure-relative displacements consistently shifted by eight bytes. Cookie remains EBP-4. Instruction encodings otherwise agree outside mapped address fields. This is not an exact match or runtime equivalence proof. No padding added to source; reason for frame difference remains open (compiler/options/context/source). Results archived in vm-share/pilot-frame-hotpatch/. Experiment 3 staged: /O1 replacing /O2 only; not run yet. No fresh Ghidra state check or database changes this turn.

## Experiment 3: 2026-11-01
Guest outputs verify /O1 /Oy- /hotpatch /GS /MD /Zi build success. First helper 0x00401000..0x0040108a (139 bytes), second 0x0040108b..0x00401113 (137 bytes), versus original 144 bytes each. DUMPBIN shows ESI reused for 0x9c before loading the mask API pointer, changed scheduling/register selection, and LEAVE epilogues. Frame allocation remains 0xa0, structure EBP-0xa0. This O1 configuration is a worse instruction match than experiment 2; it does not rule out all size-oriented configurations or prove original O2 use. Archived in vm-share/pilot-o1/.
Experiment 4 staged: restore experiment 2 O2 baseline and add /GL plus linker /LTCG; source unchanged. Link-time inlining/removal of standalone helpers is possible and must be checked through MAP/disassembly before range comparison. No original flags established, no Ghidra changes, no fresh database save-state check.

## Experiment 4: address-normalized exact match (2026-11-01)
Live verified from on-disk original and guest-produced linked EXE/MAP. Both original 144-byte helper bodies at 0x01001760/0x01001800 match rebuilt bodies at 0x00401000/0x00401090 exactly after mapping five address operand fields per function. Raw comparison differs in 16 bytes per function. Every excluded field was validated against its known original target and the rebuilt MAP symbol, including relative CALL targets; no stack offsets, opcodes or ordinary constants were masked. Frame size is now 0xa8 and structure starts EBP-0xa8. Neither helper was inlined away.
Compiler 14.00.50727.42; flags /O2 /Oy- /hotpatch /GS /MD /Zi /GL, linker /LTCG plus previous linker flags. This reproduces these functions; it supports but does not uniquely prove the original compiler/options. Original source wording and original zero-argument calling convention remain unproven. No source padding or instruction patches used. Normalized exact function-body match is not whole-file equality or independent runtime equivalence proof; CRT target internals and complete module layout are outside this comparison.
Reproducer: scripts/compare-version-pilot.ps1, default input paths. Machine-readable report: vm-share/pilot-ltcg-match/comparison.json. Archived EXE/PDB/MAP/disassembly/build log, batch and source: vm-share/pilot-ltcg-match/. Original SHA256 61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735; rebuilt SHA256 7DF0008D4A2E1228520000D48EAD90794CD286603AEE1C3693CDAE8A13417224. No Ghidra modifications/navigation/save or fresh database-state query this turn. Next milestone: retain this configuration and extend to another understood function; reccmp integration remains untested.

## Experiment 6: third matching function
SDK StringCopyWorkerA at rebuilt 0x00401180 matches original fsx.exe 0x01001600 exactly over 72 raw bytes, no normalization. Both version helpers remain exact after their five validated address mappings. See startup.md for provenance limits and scripts/compare-copy-pilot.ps1 for reproduction. Artifacts in vm-share/pilot-copy-06-match/. No further guest build required to certify this milestone.

## Experiment 8: four matching bodies
SDK formatter with _STRSAFE_USE_SECURE_CRT=0 matches original 92-byte body after one verified _vsnprintf IAT mapping. All three previous matches retained. Combined scope 452 bytes, not entire executable. Reports and full baseline archived in vm-share/modules/fsx/archive/08-four-matches; reproducer scripts/compare-formatter-pilot.ps1. See startup.md for provenance limits. No further guest run required for this milestone.
