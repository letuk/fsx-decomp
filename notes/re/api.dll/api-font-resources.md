# API font resources and file enumeration

2026-11-08: alternate backend now identified as CAB-backed from .CAB discovery
and MSCF checks. Three structured source routines compile, unmatched due to
register ABI/scheduling; ten gates retain current4450 total. See
api-alternate-enumeration.md. The old1120 below is a historical subset.

2026-11-07 update: open20134495 and next2013468c now have structured source in
file_enumeration_open_next.cpp (native512/714 versus original503/669), with
suffix lookup201336ab, registration20133778, copy201350be and string duplicate
20137efe in suffix_registry.cpp. All compile; none of these six is yet accepted
as a body match. Close92-byte gate retained. The alternate opener's ESI output
and iterator's ESI output/EDI cursor are now explicit source boundaries, still
not implemented as the original register ABI. See api-source-pass.md for
evidence, sizes, naming suggestions and latest run. This supersedes the prior
"source not reconstructed" state, not the remaining dependency limitations.

Checkpoint recorded 2026-11-07 after an interrupted session. Original api.dll
SHA256 9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Addresses are original VAs; pilot names and observed types are reconstructed.

Live-verified analysis: 20121083 (733 bytes) adds/removes fonts using dynamically
resolved AddFontResourceExA/RemoveFontResourceExA with private flag 0x10, falling
back to legacy APIs. It enumerates fonts, skips directory marker -2, appends
.ttf to returned stems, and closes the enumeration. API results are ignored.
Open 20134495 (503 bytes), next 2013468c (669), close 20134929 (92), and suffix
lookup 201350be (86) were inspected. The alternate enumeration backend has
register arguments; its storage format is not yet identified.

Observed state332 contains handle at0, WIN32_FIND_DATAA at4, suffix selector at
0x144, and flags at0x148. Record300 contains name260, selector at0x104, size
low/high, writable, and two six-WORD timestamps at0x114/0x120. These are minimum
observed records, not original class names. SDK assertions validate pilot layouts.

Close source matches all92 bytes with four mapped references and three rejecting
controls. Final run: vm-share/modules/api/runs/api-font-resources-20261103-001;
object SHA256 CEB1F80139323AC6D7D2C2E2A14ECD70732F985C1FE8B89BA136E6DF379BCDB1.
Eight accepted-body comparators pass. Total complete disjoint code becomes1120;
metadata44 and Initialize partial123 remain separate. Font source891 versus
original733 is unmatched; stop the initial source-shape question at one trial.
Custom trim/copy boundaries and enumeration integration remain open. No runtime
or complete linked-DLL equivalence is claimed. Maintained source is structured.

Ghidra: scripts/ghidra/TypeApiFontEnumeration.java applied two observed types and
five stdcall signatures, preserving established function names. Pre-edit backup:
artifacts/api.dll/ghidra-backups/api-before-font-enumeration-types-20261103.gzf.
Earlier save requests timed out while additional modules were being analyzed.
Fresh readback on 2026-11-07 confirms all five signatures and revision
1793741866691:19c8b8ae:3, unsaved_changes=false; save_program reports no unsaved
changes. No manual save is currently needed. No program tab was switched.

Next: resolve enumeration/custom ABI dependencies before further font match
trials; consult newly loaded modules as references with explicit program targets.
Layout/import fine search remains deferred. Existing stop rules still apply.
