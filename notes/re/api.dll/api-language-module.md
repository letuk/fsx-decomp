# Language resource module initialization

2026-11-03. Original api.dll SHA256
9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003,
base200e0000. Live assembly/native decode verified; working names reconstructed.

Entry2013e64f stored50-byte function is only the first integrity transfer. Its
old void/no-op decompilation is superseded as whole-routine behavior. New
call-opaque normal/success inventory199 instructions/729 native bytes plus eight
16-byte descriptors covers2013e64f..2013e9a7 without gaps. Known following class
procedure starts2013e9a8. Return0 on language DLL load failure,1 after success.
Integrity-failure/detached exception behavior and callees remain outside inventory.

Generic flow reader now recognizes the observed short signed LEA displacement
form at2013e7f7 in addition to32-bit forms. It still validates entire register/
stack templates and known gateway targets; no arbitrary skip. Both prior init
465/1596 and main-window398/1346 inventories regress unchanged with zero stops.
scripts/verify-api-native-flow.ps1 verifies bytes/non-overlap/descriptor separation
and optional contiguous coverage; both language and main-window artifacts pass.
This inventory is analysis evidence, not rebuilt-code coverage.

## Actual flow

Existing module2020e028 is passed to FreeLibrary when nonNULL; it is not
immediately zeroed. With nonempty config filename2020d248, GetPrivateProfileStringA
reads LANGUAGE/DLL using default path buffer2020d140 into local260 buffer.
Nonempty drive-qualified or UNC path is used directly. Otherwise base2020d360
is copied to shared scratch2020d770 and local path appended within capacity260.
No extra separator inserted and helper returns ignored. Empty config filename
uses default path2020d140 directly.

LoadLibraryA result overwrites2020e028. Failure captures GetLastError once,
formats original error literal200f36a8 using file scratch2020d770 plus code twice,
and MessageBoxA(NULL,text,"ERROR",10h), then returns0. Scratch is used in error
report even when actual load path was the default or absolute local string;
do not substitute the active path as an unsolicited behavior fix.
Success invokes20138550(resource3,2020d468,100h), then returns1. Original-file
extraction now confirms resource3 is "Microsoft Flight Simulator" in the supplied
language.dll (de-DE). This supersedes the unextracted-text state; runtime module
and locale selection remain configuration-dependent. See api-module-references.md.

Global sharing matters: scratch2020d770 is also the location-string buffer used
by geometry. Resource module2020e028 is the word forwarded by error callbacks.
Current source names for these roles are separate unresolved external aliases;
final module integration must unify identities before claiming linked behavior.
No complete global-buffer sizes are inferred solely from observed capacities.

## Source and limits

language_module.cpp normal-flow source model builds357 bytes in
api-language-module-20261103-001. It deliberately does not reconstruct integrity
gateways/failure paths; custom200fb1ab formatter (EAX capacity/EDI output plus
stack varargs) remains an explicitly unresolved source boundary. Consequently
there is no full-body comparison acceptance or added coverage. Initial source
question stops at1, no padding/flag experiments.

Object SHA256FB938EC036F981E17B8F86C62951BEA02A45811585030ECAA9EAE0918B358194.
Seven accepted-body comparators pass on this shared object; complete code1028,
metadata44, Initialize partial123 unchanged. Raw source/native reference evidence
in run language-module-diagnostic.json; flow and original-byte verification in
artifacts/api.dll/api-language-module-{flow,verification}-20261103.json. Initial stored
function snapshot is in api-monitor-geometry-analysis-20261103.json.

Language database body/signature have not been changed or expanded. Monitor
typing revision710 is last saved-state evidence. No candidate execution, program
navigation or memory patches. Suggested rename FUN_2013e64f -> LoadLanguageModule
is provisional and not applied while stored body remains partial.
Next font-resource lifecycle20121083 and its enumeration interfaces, then
initialization dependency integration. Continue existing autonomous workflow.
