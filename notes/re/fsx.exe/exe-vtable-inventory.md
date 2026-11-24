# FSX EXE application-vtable inventory

Latest update: the two missing overrides identified below were reconstructed and
verified in baseline25 on2026-11-02; see app-dispatch-matching.md. The sections
below describe the original baseline24 inventory. Generated inventory artifacts
now default to25 and show those two entries as matched.

2026-11-02; status: live verified original table/listing and offline PE/import
comparison against preserved SDK-based baseline24. Original fsx.exe SHA256:
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735.
Rebuilt baseline SHA256:
88A1121284B9953F5DC24E27C2D17934F018485C7A41BE2BF98B2317BE8B7586.

Observed application table0x01001398..0x01001457 has48 four-byte slots, ending
immediately before the known OLE filter table0x01001458. This is an observed
table range supported by the real SDK-based counterpart, not an inferred full
original class size/name. scripts/inventory-app-vtable.ps1 emits complete
CSV/JSON/HTML under artifacts/fsx.exe/vtables and reruns lifecycle checks for body claims.

| Category | Entries | Evidence |
|---|---:|---|
|MFC imports|40|Both targets are FF25 import thunks; DLL+ordinal/name match per slot|
|SDK leaf stubs|3|Serialize/Dump RET4 and AssertValid RET bodies equal raw bytes|
|Application-specific entries|5|Destructor, GetMessageMap, InitInstance, Run, ExitInstance|

The28-slot OLE filter table was already checked separately, including its
OnMessagePending override; see mfc-message-filter.md and compare-ole-filter.ps1.
Do not count imported MFC implementations as newly reconstructed EXE source.

## Application entries

| Offset | Original target | Role | Current source status |
|---|---|---|---|
|+0x04|0x010016d0|Deleting destructor|70-byte normalized match|
|+0x30|0x01001650|GetMessageMap|Missing own override; rebuild inherits CWinApp default|
|+0x58|0x010019e0|InitInstance|777 vs769 bytes, not matched|
|+0x5c|0x01001d00|Run|Missing own override; rebuild inherits CWinApp default|
|+0x70|0x01001720|ExitInstance|58-byte normalized match|

These roles are reconstructed using SDK virtual-slot correspondence and observed
behavior. No original class or method identifier recovery is claimed. Constructor
0x01001660 is separately matched and does not occupy a vtable entry.

### GetMessageMap

Existing instructions0x01001650: MOV EAX,0x010012f0; RET (six bytes).
The returned eight-byte AFX_MSGMAP-shaped record contains0x01002020 and
0x010012d8. First target is a thunk to MFC80 ordinal3683, identified in current
Ghidra imports as GetThisMessageMap; second target is an all-zero24-byte message
entry terminator. This strongly supports an empty BEGIN_MESSAGE_MAP-style map
chained to CWinApp, with DECLARE_MESSAGE_MAP generating the override. Exact
original macro syntax/file boundaries remain unproven.

### Run

Existing instructions0x01001d00..0x01001d22 (35 bytes):
1. Preserve this inESI; call pfnGetApplication at0x01004384.
2. Call returned Application vtable+8 with one explicit argument0x01001cf0.
3. Load own vtable+0x70, restore this, tail-jump to that virtual ExitInstance.

Callback0x01001cf0 is a five-byte JMP0x010020c8; latter thunk imports
MFC80 ordinal1074, named AfxGetCurrentMessage by the current import model.
Application slot+8 therefore accepts a message-source callback at this caller;
its existing reconstructed no-argument interface slot is incomplete. API target
semantics/signature should be independently rechecked before updating that type.
Unlike InitInstance's remaining encodings, both missing overrides are real gaps
in the current harness's application behavior. The harness is not runnable FSX.

## Folding observation

Original slots+8 (Serialize) and+0x10 (Dump) share target0x01001ef0, RET4.
Rebuilt equivalents point to0x00401000 and0x00401030, both the same RET4 bytes.
Original AssertValid at0x01001f00 is a separate bare RET. This observed sharing
suggests identical-code folding or equivalent compiler reuse. It does NOT prove
the original linker used /OPT:ICF; canonical link flags still /OPT:NOICF.
A future source-pinned /OPT:ICF experiment can test table topology and regressions
without changing declarations based on speculation.

## Priorities and unapplied suggestions

Stay with fsx.exe. First reconstruct GetMessageMap with the real MFC macros and
verify its static map and inherited base-map import, then Run plus the tiny
callback and the actual API slot+8 signature. Follow with an isolated folding
experiment. This is more concrete than more equivalent InitInstance expressions.

Current Ghidra model has instructions but no stored functions at0x01001650,
0x01001d00,0x01001cf0 and the SDK leaves. Suggested future creations/names:
FSXApp::GetMessageMap, FSXApp::Run and a provisional current-message callback
wrapper. Existing interface UnknownSlot08 -> RunMessageLoop is a provisional
rename suggestion supported by the caller; not applied. Do not impose an
unverified API target signature merely from the slot role.

No Ghidra database edits, disassembly creation, navigation, save or backup.
api.dll remained current with fsx.exe also open; all calls specify program.
No new guest build or canonical source/compiler change in this inventory turn.
