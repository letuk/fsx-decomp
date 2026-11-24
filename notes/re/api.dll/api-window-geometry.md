# Monitor selection, window geometry and display-error response

2026-11-08 resource corroboration: original language.dll de-DE stringsf455/f457
confirm minimum-resolution prompt and unsupported24-bit-monitor warning.
See api-module-references.md for hash/extraction; runtime locale not inferred.

2026-11-03. Original api.dll, base200e0000, SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
All addresses original VAs. Status: live/file assembly verified, native source
builds verified; names/context field meanings reconstructed. No new matching
coverage or runtime execution in this package.

## Observed layouts and interfaces

MONITORINFOEXA is72 bytes, not the40-byte MONITORINFO formerly inferred in
decompiled locals. Device name begins at offset40. Existing platform type
/WinDef.h/tagRECT16, HMONITOR4, HDC4 preserved. Native SDK C_ASSERT verifies
MONITORINFOEXA72 and reconstructed context offsets/size at compile time.

| Context offset | Observed field | Bytes |
|---|---|---:|
|00|requested device name|32|
|20|primary monitor seen flag|4|
|24|color-depth state|4|
|28|fallback monitor info incl device name|72|
|70|requested/selected monitor info incl device name|72|

Reconstructed context extent184 is fully evidenced by accesses and local zeroing;
this is not an original class name or complete surrounding object claim.

| Entry | Observed ABI | Result |
|---|---|---|
|201214ad|stdcall4 monitor,HDC,RECT*,context*;RET16|BOOL continue enumeration|
|201215fd|stdcall2 requested72*,fallback72*;RET8|0/-1/b0001/b0002|
|20121796|custom monitor inECX, output RECT* stack4;RET4|0 or selection status|
|201213c4|stdcall1 status;RET4|1 or80004005h; no observed zero result|

## Enumeration and selection

201215fd zeroes context; copies existing requested device name32 when cbSize
nonzero and clears requested cbSize. GetDC(NULL) failure returns-1 after that
clear. GetSystemMetrics(51h)==0 selects per-monitor color checking (state1);
otherwise GetDeviceCaps PLANES*BITSPIXEL==24 selects rejection state2, else0.
It calls EnumDisplayMonitors and releases DC. State2 returnsb0001 before output
copies. Otherwise copies selected/fallback records72 and requires at least one
cbSize72, elseb0002. Enum API return is not separately checked.

Callback201214ad sets local cbSize72 and calls GetMonitorInfoA without checking
BOOL. State1 inspects monitor DC;24 bits sets state2 and stops enumeration.
Primary monitor sets primarySeen; eligible monitor dimensions must be>=1024x720.
A primary eligible record replaces fallback; nonprimary eligible record fills it
only when fallback absent. Requested device name is compared with lstrcmpA
(case-sensitive); a matching name is cleared even if resolution is inadequate.
An eligible match fills selected. Early-stop rules retain enumeration when all
monitor color formats still need checking. Source implements these rules without
goto or fabricated monitor selection policy.

## Restore or choose bounds

20121796 reads MAIN/Location into shared buffer2020d770 from config2020d248.
sscanf_s format200f29dc is %d,%d,%d,%d,%s, with four RECT fields then32-byte
device name. Parse count other than5 empties bounds; provided monitor populates
requested info, NULL clears cbSize. Selection helper returns its nonzero status.

Valid requested monitor offsets saved bounds by rcWork origin, then tests its
top-left against a SetRect built directly from GetSystemMetrics4c/4d/4e/4f.
Original passes virtual-screen width/height as right/bottom, without adding
left/top; source preserves this observed expression. Invalid saved origin may
switch nonprimary selection to primary fallback, then clears bounds.

Empty bounds choose1024x720 by default. Monitor width<=1024 instead chooses
three quarters of both monitor dimensions using signed division. It centers
against rcMonitor dimensions plus rcWork origin, sets right/bottom, calls
AdjustWindowRect(stylecf0000h, menuTRUE), copies output RECT and returns0.
There are no added API-success checks or clamps. Source normal-flow geometry
uses ordinary stack2 boundary, explicitly different from original ECX/stack ABI.

## Display error response

201213c4 uses string resourcef457 for b0001 andf455 formatted with1024,720
for b0002. Other status returns80004005h. Callback2020de78 receives
(0,message,address2020d468,4,65h,value2020e028); response6 triggers20123315
with desk.cpl desk,@Settings /Action, show1, wait1 and no output handle.
Launch failure yields80004005h, otherwise response stays1.

Resolve contradiction explicitly: main caller has a response0 retry edge, but
the inspected201213c4 body has no zero-return path. Do not describe normal
settings-dialog success as a proven retry; current body returns1 and caller
aborts on any nonzero response. Formatter20120210 is44 bytes, capacity inEDX
plus cdecl stack buffer/source/varargs and FormatMessageA; its decompiler extra
fastcall arguments are not proof of standard ABI. Source uses an explicit
unresolved formatter boundary.20123315 launches rundll32/Control_RunDLL with
optional waiting/message handling; its full461-byte implementation stays open.

## Source builds and bounded stops

2026-11-11 startup follow-up. The source entry
`ResolveMainWindowStartupBoundsPilot` connects the existing
resolver and error handler in the loop at `20122412`. A zero
geometry status succeeds. A zero error response would retry.
The observed error handler returns 1 or E_FAIL, so its current
paths abort startup even when the display settings applet opens.

The formatter at `20120210` now has a source implementation using
FormatMessageA with a cdecl argument list. The display applet
launcher at `20123315` selects ShellExecuteExA or controller
slot +0x68 and dispatches WM_PAINT while waiting. The source adds
checks for oversized system paths, missing process handles and
WAIT_FAILED. The non-waiting handle output preserves the original
store followed by CloseHandle. The geometry caller uses wait
mode with no handle output.

The PE check covered 1323 bytes across the four function listings
and 37 bytes of the caller phase. IAT slots `200e16f8`,
`200e1c74` and `200e1af0` identify FormatMessageA,
ShellExecuteExA and MsgWaitForMultipleObjects. The controller
slot +0x68 target remains an integration boundary.

`api-startup-geometry-20261111-001` passed all 20 comparators.
COFF SHA256 is
`7726C1D09F895FEF82C801DDCEB038878EDB76FEB720F6E25AFA7E761B6BDF95`.
New byte matches and runtime verification remain open. Ghidra
was unchanged. The next startup phase begins at `20122437`
with SIMPROP initialization and FS98MAIN creation.

- api-monitor-selection-20261103-001: monitor_selection.cpp, callback309 versus
  original336; selector407 versus409. Original callback aligns ESP to8, native
  prologue/frame/copy/register scheduling differ. Native SDK layout assertions
  pass. Object SHA2566F0A3FB9A67E21B0AB07FFF908CF4FDC67FE927DBE627AACE458E265D80DEAA3.
- api-window-geometry-20261103-001: window_geometry.cpp575 versus585,
  geometry_error.cpp233 versus233, but custom ECX geometry/EDX formatter source
  boundaries and scheduling are unmatched. Object SHA256
  06AD8BE0920D215E83C9DE00B9452324B8FB06E3989729D4DC4E19054B5E90FE.

Each initial source question stops at1; no buffer/flag/alignment/unused-local
matrix. Diagnostics retain raw bodies/references and compiled sources. Equal
length is not equality. Seven existing gates pass on geometry run; complete
code1028/metadata44 retained, partial Initialize123 separate. Language run later
regresses same gates; see api-language-module.md for latest shared snapshot.

## Actual Ghidra work and evidence

Created /RE/Api/MonitorInfoExAObserved72 and
/RE/Api/MonitorSelectionContextObserved184, preserving existing platform types.
Typed four targets above with observed storage, purge/results and comments;
existing FUN names preserved. Geometry uses custom storage; __stdcall there
models cleanup only. Callback function type and its target signature are not
silently replaced with an imported global prototype. Decompiled locals remain
partially split arrays; no claim all local storage has been manually repaired.

Script TypeApiMonitorGeometry.java verifies prior names/body sizes/parameters,
new type layout and custom ECX/stack argument positions. Backup
artifacts/api.dll/ghidra-backups/api-before-monitor-geometry-types-20261103.gzf (8542585 bytes).
Saved revision1793739846836:636d9e2e:710, unsaved=false. Four post-edit live spans
equal pinned original; no code/body/table/navigation changes.
Artifacts api-monitor-geometry-{analysis,typing}-20261103.json contain raw
dependencies and readbacks/bytes. Suggested names, not applied:
FUN_201214ad -> EnumerateStartupMonitor, FUN_201215fd -> SelectStartupMonitors,
FUN_20121796 -> ResolveWindowGeometry, FUN_201213c4 -> HandleGeometryError.
Next source-boundary integration requires new ABI evidence rather than resuming
closed shape trials. No additional original-installation input needed.
