# G2D controller transition and fatal graphics report

2026-11-12. Live verified in g2d.dll, SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3.
Read-only Ghidra pseudo-disassembly matches 1,347 pinned PE bytes across
the three controller targets, their two common helpers and ordinal 65.
Separate PE reads verify the table cells, imports and fallback strings.
API call sites are documented in api-main-window-graphics-transition.md.

| G2D address | Verified behavior |
|---|---|
| 20698050, 2069eb70, 2069ec10, 20689950 | Ordinal 4 returns the controller at 20709bd8. 2069eb70 publishes storage allocated with 0x30 bytes and installs table 20668d10. The other constructor path allocates 0x2c bytes, calls 20689950 and installs primary table 20668a00. These are observed allocations. |
| 20668d10, 20668a00 | Both primary tables point slots +0x10, +0x14 and +0x38 at 20698730, 20698780 and 20698c70, respectively. Both +0x10 and +0x38 return a status in EAX. The targets use an ECX receiver and no stack arguments. The backends differ at +0x44 and +0x4c. |
| 20698730 | If byte +0x1d is set and backend +0x44 returns false, or if byte +0x1a is set, returns zero. Otherwise calls +0x14 and then api.dll ordinal 276. A nonzero API result becomes -1. Backend +0x4c otherwise supplies the status, and a zero backend result sets byte +0x1d to one. The API caller treats any nonzero result as failure. |
| 20698780, 206987b0 | Byte +0x1a defers suspension by setting +0x19. Otherwise calls the suspend helper and clears +0x18. The helper checks +0x1d and backend +0x44, then if the window range is nonempty clears DWORD +0x14, copies the range and invokes +0x9c with 1 on each nonnull entry. |
| 20698c70, 20698ca0 | API ordinal 6 reads the quit flag. On quit the wrapper returns zero. Byte +0x1a defers refresh by setting +0x1b. Otherwise the helper gates on API ordinal 318's controller slots +0 and +8 and the G2D reentry byte at 20709bf0. It stores the API controller's +4 result at +0x18 and clears +0x1c. |
| 20698d63–20698ec9 | Refresh first visits the G2D window range using +0x48, conditional +0xc4, placement helper 206976d0 and +0xac. The replacement helper 20698a70 fills an HWND collection, then API ordinal 263 is called. Two later passes use +0x48 to gate +0xc0 and +0xc8 respectively. The collected HWNDs are destroyed. If +0x1c is set, calls API ordinal 233 with (0x10, 0x181, 0). Clears the reentry byte before returning zero. |
| 20699720–20699884 | Ordinal 65 loads title 0x9108 and text 0x9106 through API ordinal 154. Missing text is copied from the G2D fallback via API ordinal 157. Creates a thread with MessageBoxIndirectA and style 0x50030, waits indefinitely, reads its exit code and closes the thread handle. It then calls API ordinal 5 with LPARAM 1. |

Cross-module evidence for pinned api.dll. Ordinal 276 at 20128c1a
has a one-time initialization latch at 2020deb0 and returns -1 on
its negative prerequisite result. Ordinal 6 at 2012dff3 reads the
quit DWORD at 2020e044. Ordinal 318 at 20125334 returns the controller
pointer at 2020de98. Ordinal 5 at 2012dfbd sets the quit flag and
posts WM_COMMAND 0x110a for nonzero LPARAM. G2D's report requests
that API shutdown path. The API owns the message dispatch.

## Source and verification

src/g2d-pilot/controller_transition.cpp is compiled on its own as
a G2D COFF unit. API source stays in src/api-pilot. The new code models
the shared targets, reentry guard, window passes, and fatal report.
The observed controller prefix uses compile-time offset checks.
The source uses std::vector for snapshots and replacement HWNDs, a
typed thread entry for MessageBoxIndirectA and a guard for failed
CreateThread. Import binding, object publication, original table
placement and backend/window internals remain source boundaries.

VC8 x86 run g2d-controller-transition-20261112-002 succeeded with
/EHsc and no compiler warnings. The first build used the API object's
compile profile and warned about unwind semantics. The second enables
C++ unwinding for the separate G2D unit. Source hash matches the worker
snapshot. COFF symbols show ECX fastcall entries and separate ordinal
4 and 65 source bodies. COFF SHA256 is
A43A1EA940B40D80314FF80E8C1C2F773562041CE52309B78693824ACAE37406.
No G2D body comparator, final DLL link or runtime verification.
The API source and its existing comparison ledger are unaffected.
Ghidra stayed read-only and reported changed=false for g2d.dll.

Suggested Ghidra names, unapplied. DAT_20709bd8 to g2dController
follows both publication paths and ordinal 4. DAT_20709bf0 to
g2dRefreshInProgress follows the test, set and clear in 20698ca0.
The role of controller byte +0x1a beyond the two deferral gates
remains open. Source names describe observed roles.

Next package is the per-backend graphics initialization behind
+0x44 and +0x4c. Table 20668d10 points to 20696b90 and 20693730.
Table 20668a00 points to 20683270 and 20689b80. Window replacement
at 20698a70 and individual window-table implementations form a
separate boundary.

The backend initialization package now covers both +0x44 and +0x4c
pairs. See g2d-controller-backends.md for the window-construction boundary.
