# API add-on trust decisions

2026-11-12. Live verified against the pinned FSX Acceleration API PE
SHA256 9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Fifteen retained listings under artifacts/api.dll/api-source-pass-20261107/functions
match 5,193 listed instruction bytes. Additional PE reads verify the
export, imports, seed, protected descriptors and shutdown entry.

| Binary and address | Verified finding |
|---|---|
| api.dll:20123c41 | Ordinal 305 takes four DWORD stack slots with RET 0x10. A nonzero first slot invokes the Trusted reset at 20124051 with the second slot and returns 1. Otherwise it resolves the key from slot 3 and uses slot 4 as an optional in/out DWORD. Path or key resolution failure returns 0. |
| api.dll:20123ce7–20123d2b | The profile read at 2013174d uses the default stream-backed configuration and section Trusted. The output starts at zero. On status zero a negative value marks a cached decision, and equality with mode or mode + 3 marks acceptance. The equality tests run even when the key was absent and the initialized zero survived. Without incoming bit 31, cached decisions return immediately. The retained C view omitted the signed test and general equality cases. |
| api.dll:20123d30–20123dd6 | Lazily allocates the provider object via 20147f31, registers the group-6 shutdown callback at 201238c8 once, and sets phase 2020de94 to 7. A successful noninteractive verification with adjusted policy flags accepts. Otherwise classifier result 3 accepts and ORs bit 0 into the optional flags. When classification differs and a decision was cached, the function returns that decision. |
| api.dll:20123ddc–20123f83 | Without a cached decision, a failed interactive verification writes -mode and returns 0. On success, the original retries noninteractive verification. If that still fails, it formats resource 0x1a with the compacted path and calls FSAlert. Response 0x40 writes mode. Other responses write mode + 3. Both accept the current load. Other acceptance paths write mode. The return status from the profile writer does not control the decision. |
| api.dll:20123917–20123c40 | Fastcall-shaped routine with key in ECX, normalized-path output in EDX, then input path and two capacities on the stack. A missing path-output buffer selects a MAX_PATH scratch buffer. It resolves and checks the path, then compares the exact normalized path prefix and cached path length at 2020e300 and 2020e2fc. The same path reuses the key without reopening the file. |
| api.dll:20123a7d–20123bf7 | Ordinary key generation obtains the CryptoAPI provider, hashes the 16-byte seed at 2020e2ec followed by the file in 2 KiB chunks with CALG_SHA1, and encodes the digest with the nibble table at 200f2c38. The suffix starts with a dot. The routine checks GetLastError before closing the hash and file and only appends on zero. It can therefore retain the path without a digest. The cached provider is owned by the trust shutdown path. |
| api.dll:20123a06–20123a7d | An all-zero seed matches the record at 200f2c4c and enters the protected region through 201ecc2f. The original PE's seed bytes are initially zero. Descriptors at 20123a38 and 20123a75 both contain 0x1db8 and zero. The first native resume at 20123a40 tests EAX and branches to the ordinary hash code at 20123a7d on a negative result. The second gateway is 201ecc87. Protected-region effects and any additional continuation remain open. See api-initialization-flow.md for the shared gateway. |
| api.dll:20123ff6, 20124051, 20130b6b | Clear resolves a key and writes numeric zero. The section reset also writes numeric zero for selected keys. Both use the existing integer writer with parser-cache invalidation and the profile write retry. This refines the term reset used in api-addon-startup.md. |
| api.dll:20147f31, 20123536, 20123694 | The constructor binds WinVerifyTrustEx, the WinTrust policy flag pair, and the two provider-state/signer helpers from WINTRUST.DLL. Verification and classification consume that table. Classification has a second dynamic dependency on Crypt32.dll through 201234e2. These routines are the next provider package. |
| api.dll:200fc05d, 200fcb01, 200fc7b9 | Verified internal calls from the EXE and DLL readers to 20123c41 and the DLL reader to 20123917. |
| panels.dll:20b6c45c | The original instruction calls through IAT 20b3128c, which imports api.dll ordinal 305. Caller semantics remain open. The checked panels.dll SHA256 is 66F881EFF61318FFE6B0213E55BE21362A5B1B35B441D57E080D31DF0BFC9E58. |

## Source and verification

src/api-pilot/addon_trust.cpp connects both XML paths to the decision
body, DLL launch-marker key resolution and trust clearing. The
profile read reuses ReadSelectedConfigurationIntPilot. Reset and
writes reuse WriteConfigurationIntPilot and the existing default
profile storage. The source uses the existing resolver and resource
allocator. The ordinary hash models a complete 20-byte digest and
keeps the observed GetLastError gate. The protected key path and
provider operations remain explicit source boundaries, with the
provider context and phase storage unresolved at link time. The
provider boundary must preserve the distinction between an allocated
object and a successfully bound provider table.

VC8 x86 run api-addon-trust-20261112-001 passed all 20 existing
comparators. COFF SHA256 is
DF7582BE47B43217BD035152A0FFFAD585D979ABA0A282132F0DBD9BE3CE2BDD.
The new entry points emit the expected @16, fastcall @20 and @4
decorations. COFF relocations confirm reuse of the existing parser
and integer writer. The accepted ledger remains 5,263 code bytes
and 288 metadata bytes. New body matches, full DLL linking and
runtime verification remain open. Ghidra received no edits and has
no new unsaved changes from this work.

Suggested Ghidra names, unapplied. At 20123c41, param_1 →
resetStoredDecisions, param_2 → decisionMode, param_3 → addonPath,
and param_4 → trustFlags. The reset branch, key call and bit
tests support the names. At 20123917, local_95c → hashHandle and
local_20 → digest based on the CryptoAPI output slots.
The role of phase value 7 remains provisional.

Next package starts with the WinTrust provider lifecycle at 20147f31
and 201238c8, then verifies the noninteractive/interactive requests
at 20123536 and signer classification at 20123694. The protected key
gateway and the panels.dll caller remain separate boundaries.

The provider package now connects the WinTrust verification and
classification bodies and the group-6 release callback.
See api-addon-trust-provider.md for the controller boundary.
