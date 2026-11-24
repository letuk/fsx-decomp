# File and wrapper stream VTables

2026-11-09. Original api.dll SHA256 is
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Eleven original-byte verified listings cover1486 bytes. The evidence file
artifacts/api.dll/api-cab-stream-tables-evidence-20261109.json also records the table
and interface identifier bytes. Live queries confirm saved revision
1793741866691:2a9d629f:3. All source names are reconstructed.

The two source tables bind the 14 verified slots at200f2c70 and200f2d50.
They include query, reference count, read, write, seek, resize, copy, commit,
revert, both region methods, stat and clone. The const tables emit in COFF
.rdata. Existing read/seek/stat functions are reused with ABI-compatible x86
receiver casts. Shared reference increment200f40da already contributes to
the generated coverage and is counted only there.

Query accepts the four verified identifier byte sequences. Wrapper query
uses the underlying query/release pair for the special identifier, then
returns its own receiver on success. File release closes the handle before
returning the record to its pool. Wrapper release calls its established
destruction path. Refcounts use the original non-atomic query increment and
interlocked AddRef/Release operations.

File write2012438f calls WriteFile. Wrapper write20126f2d is limited to the
observed writable memory-backed path, advances the position and extends valid
buffer bytes. File resize201244d2 saves the position, seeks using the low
size word, sets EOF and restores the position, returning0 regardless of API
results. Wrapper resize and the shared revert and region methods return
80004001. Commit returns0. These fixed results are original code behavior.

Copy2012451d/20127084 uses 4096-byte chunks with separate read/write totals.
The source combines the forwarding entry and copy body, including the
observed order of tally updates around failure checks. Counts come from the
underlying read/write callbacks. File clone reopens the path with the mode's
access/share pair. Wrapper clone either shares existing memory while retaining
the original stream reference or wraps a cloned underlying stream. The
normal paths use the existing wrapper constructor. Original clone unwind
metadata and private copy/constructor ABIs remain outside acceptance.

The first build exposed duplicate mutable VTable declarations in the
producer. Removing those declarations lets the shared const declarations
govern the build. The retained run is api-cab-stream-tables-20261109-002,
object SHA256 D03BF4BF68A5E4DAC65FD976F4434CF51EB5B6621E97B2440F75E6D7653FE2F8.
The new comparison gate verifies WriteFile57 bytes with one IAT mapping at
200e190c and four raw bodies8+5+8+8. These add86 code bytes. All 28 table
relocations are verified independently against their original slot targets,
adding112 metadata bytes. Opcode, return, IAT and slot mutations reject.
Fourteen gates pass at4918 code bytes and288 metadata bytes. The larger new
methods are source models outside body acceptance. Runtime and final linked
placement remain open. The two builds addressed source integration and
compiler diagnostics without additional match variants.

Suggested Ghidra names include 201271d7 param_2 to outputClone and local_20
to clonedUnderlying, supported by the clone call and subsequent release.
20126f2d param_4 can become writtenCount based on the success and zero-count
failure stores. No edits were applied. Next connect the remaining buffer
cache container operations and owner reload contracts so the CAB path can
be reviewed end to end with explicit error and lifetime boundaries.
