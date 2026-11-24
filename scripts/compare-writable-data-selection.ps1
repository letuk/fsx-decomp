param([Parameter(Mandatory)][string]$BuildDirectory,[string]$ControlDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\40-crt-entry-selection")
$ErrorActionPreference='Stop'
$old=@(Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json);$new=@(Get-Content "$BuildDirectory\source-hashes.json" -Raw|ConvertFrom-Json)
if(($old|Sort-Object File|ConvertTo-Json -Compress) -cne ($new|Sort-Object File|ConvertTo-Json -Compress)){throw 'Compiled input hashes changed'}
$request=Get-Content "$BuildDirectory\request.json" -Raw|ConvertFrom-Json
$before=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"));$after=[IO.File]::ReadAllText((Resolve-Path "$BuildDirectory\build.cmd"));$normalized=$after
$changes=@()
switch -CaseSensitive ($request.WritableDataSelection){
 'AtlCacheInclude' {$needle='/INCLUDE:?g_pfnGetThreadACP@ATL@@3P6GIXZA ';if(([regex]::Matches($normalized,[regex]::Escape($needle))).Count -ne 1){throw 'Missing single ATL INCLUDE'};$normalized=$normalized.Replace($needle,'');$changes+='ATL cache INCLUDE'}
 'AtlLibraryFirst' {$needle='mfc80.lib mfcs80.lib atls.lib "';if(([regex]::Matches($normalized,[regex]::Escape($needle))).Count -ne 1){throw 'Missing single explicit ATL library'};$normalized=$normalized.Replace($needle,'mfc80.lib mfcs80.lib "');$changes+='Explicit atls.lib after mfcs80 before CRT'}
 'Default' {}
 default {throw 'Unknown writable-data selection'}
}
if($request.MergeBssIntoData){$needle='/MERGE:.bss=.data ';if(([regex]::Matches($normalized,[regex]::Escape($needle))).Count -ne 1){throw 'Missing single BSS merge'};$normalized=$normalized.Replace($needle,'');$changes+='Explicit .bss into .data merge'}
if(!$changes.Count -or $normalized -cne $before){throw 'Unexpected build command difference'}
$layout=(& "$PSScriptRoot\inventory-writable-data.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
$thunks=(& "$PSScriptRoot\compare-import-thunk-placement.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
$c=Get-Content "$BuildDirectory\comparison.json" -Raw|ConvertFrom-Json
$slots=@{};foreach($e in $c.PeImports.OriginalImportEntries){$slots[$e.Identity]=$e.Index}
[pscustomobject]@{RebuiltSHA256=$layout.RebuiltSHA256;ControlSHA256=(Get-FileHash "$ControlDirectory\fsx.exe").Hash;CompiledInputsIdentical=$true;OnlyBuildCommandChanges=$changes;DataLayout=$layout;ThunkCount=$thunks.ValidatedThunks;ThunkDelta=$thunks.UniformDelta;TrailingCCMismatches=$thunks.TrailingCCMismatches;RebuiltDataSection=($c.PeImports.RebuiltHeader.Sections|Where-Object Name -CEQ '.data');ImportSlotDifferences=@($c.PeImports.ImportEntries|Where-Object {$slots[$_.Identity] -ne $_.Index}).Count;LinkerWarnings=@(Get-Content "$BuildDirectory\build-log.txt"|Where-Object {$_ -match 'warning LNK'});Scope='Isolated compiled-input/command controls plus existing data/body gates, 80-thunk checks and exact reported header/layout values. Passing normalized-body gates does not imply unchanged PE extents, matched original layout, original linkage or runtime behavior.'}|ConvertTo-Json -Depth 10
