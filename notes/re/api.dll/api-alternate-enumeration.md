# API CAB-backed file enumeration

Follow-up: path discovery, owner lifetime and normal cache source now exist;
see api-cab-owner-cache.md. Original unwind action is verified, but the fixed
compiler profile omits its native source counterpart. Resolver no longer remains
wholly external; owner image loading201449c4 is the next unresolved dependency.

2026-11-08. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
Live read-only state/signature checks and exported assembly were consulted.
Eight listings reverify against the original, retained in
artifacts/api.dll/api-alternate-enumeration-evidence-20261108.json. Listing extents are
model evidence, not blanket flow completeness. No DB edits; live revision
1793741866691:f31fc448:3 reports unsaved_changes=false.

## Verified interfaces

| Address | Contract / role |
|---|---|
|20145110|Path on stack, output in ESI; RET4; owner resolution then cursor creation|
|20144d5d|Owner in EAX, path/output on stack; RET8; allocates0x318 cursor, fetches first entry|
|20144df0|Owner on stack, cursor in EDI, output in ESI; RET4; advances before returning|
|201452e1|Path/relative-path output on stack; RET8; synchronized owner cache, move-to-front lookup|
|20145169|Input path in EDI, two output buffers on stack; RET8; .CAB discovery|
|201449c4|Owner in ESI, path in ECX; populates owner from loaded CAB image|
|2014420b|Checks MSCF header; uppercases names and writes cached lengths|

CAB identification supersedes the previously unclassified backend:20145169
constructs .CAB paths;2014420b checks M/S/C/F. Loaded image is modified at
2014446d..20144498: names uppercase, name length stored at entry+0xe. This is
not documented as an original on-disk field.

Owner count at0x118, first entry pointer at0x11c. Cursor0x318 contains owner,
index,current entry at0/4/8; directory/stem/extension260 at0xc/0x110/0x214.
Entry sizeLow at0, DOS date/time at0xa/0xc, cached length at0xe, name at0x10.
Advance=current+0x11+cachedLength. Unknown fields remain unknown; source asserts
observed offsets, not original type names or complete owner class size.

Iterator compares directory/extension case-insensitively; extension[1]=='*'
accepts any suffix. Stem is not tested. It zeros320 output bytes, copies the
entire stored name with260-byte termination limit, sets attributes0x81 and
sizeLow. Converted DOS time populates creation/access times ONLY; last-write
stays zero although outer2013468c reads it. Preserve this discrepancy. Cursor
advances for rejected and accepted entries; NULL cursor returns0.

## Source/build

src/api-pilot/alternate_enumeration.h/.cpp add structured opener, creation and
next-entry source, implementing two previously unresolved source boundaries.
Ordinary explicit stack lists do not implement original register ABI. Resolver
201452e1 and allocation-mode global2020d8ec remain unresolved; no fake cache,
success path, file I/O or register-forcing shim was introduced.

Run vm-share/modules/api/runs/api-alternate-enumeration-20261108-001.
Object SHA256 C69A9A7863DB89A348ADAB3AA0F4930949A94293195A27133D2CCF560F73572E.
Native/original: opener96/89, create144/147, next441/367; all unmatched. One
initial build; stop on known ABI blocker, no flag/padding/source-shape matrix.
Ten existing gates pass, no uninitialized/missing-return compiler warnings.
Coverage stays4450 complete code,44 metadata, Initialize partial123 separate.
COFF only, no linked DLL/runtime claim. Maintained new source has no goto.

## Suggestions and next work

Suggestions only, no Ghidra renames:20144d5d in_EAX -> cabOwner, param_1 ->
relativePattern, second stack argument -> findDataOutput (allocation/splitpath/
first-call evidence).20144df0 unaff_EDI -> cabCursor, unaff_ESI -> findDataOutput,
param_1 -> cabOwner. Reconstructed names; existing signatures left intact.

Next resolve20145169 path splitting and201452e1 cache/constructor ownership,
especially exception cleanup. Constructor2014483b normal return is now verified:
2014489b MOV EAX,ESI followed by RET4 returns the owner, superseding decompiler
void. It initializes fields114/118/11c/120 and path[0], inserts into cache under
the shared critical section, then calls201449c4. The cache resolver already holds
that same critical section and moves hits to its head, updating owner+4 via
2012de9b. Constructor load result is ignored; nonNULL owner does not itself prove
successful image loading. Exception cleanup remains open. Then assess compiler-generated
private conventions before another match trial. Font tuning remains deferred.
