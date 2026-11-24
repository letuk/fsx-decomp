# API WinTrust provider

2026-11-12. Live verified against the pinned FSX Acceleration api.dll,
SHA256 9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Ten retained listings under artifacts/api.dll/api-source-pass-20261107/functions
match 2,274 instruction bytes. The phase callback, full-shutdown call
and controller table slots were checked directly in the same PE.

| api.dll address | Verified behavior |
|---|---|
| 20123d30–20123da5, 20147f31–20147fd7 | Allocates a 0x20-byte provider record on the private heap and dynamically binds WINTRUST.DLL. Slots +0x04 through +0x14 hold WinVerifyTrustEx, the policy get/set pair and the two provider-state/signer helpers. A missing export unloads the DLL and clears the table. The allocated record survives bind failure. |
| 201234e2–20123535 | Requires the WinTrust table. Lazily loads Crypt32.dll into +0x18 and resolves CertVerifyCertificateChainPolicy into +0x1c. A failed binding clears both slots, leaving later requests eligible to retry. |
| 201232d0–20123314 | Owner selection first calls fsui.dll ordinal 6 through IAT 200e12e0. It then tests the controller predicate at slot +0x1c and the flag 2020e040. A successful controller test on the main-window path returns 2020e010. The final fallback is GetDesktopWindow. |
| 20123536–20123691 | Requires a nonempty ANSI path terminated within 260 bytes, converts it with code page 0 and constructs a 0x10-byte file record and a 0x30-byte trust request. The GUID at 200f0038 is the generic Authenticode verification action. The request uses file choice and state action zero. UI choice is 1 for interactive and 2 otherwise. A separate flag saves the current WinTrust policy flags, ORs 0x40000 into them, and restores the original DWORD after the call. Interactive verification calls controller slots +0x4c and +0x50 around WinVerifyTrustEx. The result is true for a nonnegative HRESULT. |
| 20123694–2012387b | Requests noninteractive VERIFY state and requires exact status zero before reading that state. Reads signer index zero with no countersigner and takes its chain context from +0x28. Policy 7 with zero policy error yields class 3. Policy 2 is tried only after policy 7 returns true with a nonzero policy error, and yields class 4 on true with zero error. Statuses 0x800b0001 through 0x800b0004 and 0x800b0100 yield class 2. Other failures or missing data yield 1. The successful VERIFY path sends CLOSE even if the provider data or signer is absent. The failed VERIFY path skips CLOSE in the original. |
| 20123d6f–20123da5, 201238c8–20123916 | Ordinal 305 registers the callback once in application group 6 at 2020bb48 and sets phase 2020de94 to 7. The callback ends with RET 8. At zero it returns. At one it releases the hash provider and the WinTrust record before decrementing. Other nonzero values only decrement. The value counts callback invocations in this code. |
| 2012387e–201238c7, 20123fa9–20123ff5, 20120f40 | Releases the CryptoAPI context, unloads the WinTrust and Crypt32 modules, frees the record, and clears the published pointers. The direct full-shutdown routine then removes the callback from group 6. The original relative call at 20120f40 is in the application shutdown prefix. |
| 200f2cfc, 200f2d2c, 200f2d30 | The actual controller table maps +0x1c to 20124ddd, which returns receiver byte +0x3b in AL, and maps the interactive pair to 20125979 and 201259d8. Both interactive calls pass the receiver in ECX and no stack arguments. Direct bytes at the targets show increment and decrement of receiver DWORD +0x4c. Full modal-window behavior is the next boundary. |

SDK type interpretation follows Microsoft WinVerifyTrustEx,
WINTRUST_DATA and CertVerifyCertificateChainPolicy documentation. The
binary establishes the specific flags, branches and request sizes.
The x86 object asserts the observed request sizes and the signer-chain
offset against the VC8 SDK headers.

## Source and verification

src/api-pilot/addon_trust_provider.cpp implements binding, verification,
classification, callback registration and release. Ordinal 305 uses
these bodies, and the ordinary trust-key hash now shares the context
owned by provider shutdown. Group-6 registration reuses the existing
application callback owner. The provider allocates through the private
heap. The source uses a bounded stack buffer for the wide path and
checks conversion success. It also guards a missing controller before
the interactive slot calls. The original full-shutdown call remains
inside the unreconstructed prefix in the source. FSUI ordinal 6 and
controller slots remain external or table-dispatched interfaces.

VC8 x86 build api-addon-trust-provider-20261112-002 passed all 20
existing comparators. The earlier object compiled with a VC8 POD
initialization diagnostic. The retained build explicitly zeroes the
provider storage and has no diagnostic for that file. COFF SHA256 is
DEFF479105325866CDDB898578D45438972AA3501F1134FC3A8BAC7E82B4FCD7.
The callback emits an @8 symbol. The accepted ledger remains 5,263
code bytes and 288 metadata bytes. New body matches, full DLL linking,
and runtime behavior on the original Windows target remain open.
Ghidra received no edits and there are no new unsaved changes.

Suggested Ghidra names, unapplied. At 20123536, param_2 →
applyPolicyFlags and param_3 → interactive, based on the policy
save/restore and UI/slot selection. At 20123694, local_c →
signerClass based on the four return values. The observed controller
DWORD +0x4c currently appears as closeField4c in the pilot.
modalDepth is provisional pending the complete target and caller review.

Next package starts at the main-window modal entry/exit pair
20125979 and 201259d8, including their nesting, controller mode and
interaction with main-window close. The protected trust-key path and
the panels.dll caller remain separate boundaries.

The modal controller package connects the WinTrust call to both
reconstructed targets and resolves the modal depth and saved focus.
See api-main-window-modal.md.
