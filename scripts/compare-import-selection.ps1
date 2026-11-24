param([Parameter(Mandatory)][string]$BuildDirectory,
 [string]$ControlDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\40-crt-entry-selection",
 [long]$ExpectedThunkDelta=-32)
$ErrorActionPreference='Stop'
$old=@(Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json)
$new=@(Get-Content "$BuildDirectory\source-hashes.json" -Raw|ConvertFrom-Json)
$extra=@($new|Where-Object File -CNotIn @($old.File))
if($extra.Count -ne 1 -or $extra[0].File -cne 'import-selection-input.h' -or $new.Count -ne $old.Count+1){throw 'Unexpected added source input'}
foreach($f in $old){if($f.Hash -cne ($new|Where-Object File -CEQ $f.File).Hash){throw "Changed prior input $($f.File)"}}
$before=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"));$after=[IO.File]::ReadAllText((Resolve-Path "$BuildDirectory\build.cmd"))
$request=Get-Content "$BuildDirectory\request.json" -Raw|ConvertFrom-Json
if(([regex]::Matches($after,'@import-selection-input\.h ')).Count -ne 1){throw 'Unexpected import response count'}
$commandWithoutResponse=$after.Replace('@import-selection-input.h ','')
$commandChanges=@('@import-selection-input.h')
if($commandWithoutResponse -cne $before){
 $mfcSupportFirst=[regex]::Replace($before,' mfc80\.lib ("[^"\r\n]+\\mfcs80\.lib"|mfcs80\.lib) ',' $1 mfc80.lib ')
 if($request.ImportSelection -cne 'IatOrder' -or $commandWithoutResponse -cne $mfcSupportFirst){throw 'Unexpected build command change'}
 $commandChanges+='mfcs80.lib before mfc80.lib'
}
$manifest=@(Get-Content "$PSScriptRoot\matching\import-selection.json" -Raw|ConvertFrom-Json)
switch -CaseSensitive ($request.ImportSelection) {
 'IatOrder' { $expected=@(Get-Content "$PSScriptRoot\matching\fsx-iat-order.rsp") }
 'CrtDirectIncludes' {
  $expected=@($manifest|Where-Object {$_.Identity -cin @('msvcr80.dll!_splitpath_s','msvcr80.dll!_vsnprintf','msvcr80.dll!_setmbcp')}|ForEach-Object {'/INCLUDE:'+$_.Symbol})
 }
 'OriginalIncludes' { $expected=@($manifest|ForEach-Object {'/INCLUDE:'+$_.Symbol}) }
 default { throw 'Unknown selection mode' }
}
if(($expected|ConvertTo-Json -Compress) -cne (@(Get-Content "$BuildDirectory\source\import-selection-input.h")|ConvertTo-Json -Compress)){throw 'Include response differs from reviewed manifest'}
$pe=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $BuildDirectory -RequireImportContents|Out-String)|ConvertFrom-Json
$slots=@{};foreach($e in $pe.OriginalImportEntries){$slots[$e.Identity]=$e.IatVA}
$modules=@($pe.ImportEntries|Group-Object Module|ForEach-Object {[pscustomobject]@{Module=$_.Name;Count=$_.Count;DifferentSlots=@($_.Group|Where-Object {$slots[$_.Identity] -ine $_.IatVA}).Count}})
$inventory=(& "$PSScriptRoot\inventory-runtime-order.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
[pscustomobject]@{RebuiltSHA256=$pe.RebuiltSHA256;ControlSHA256=(Get-FileHash "$ControlDirectory\fsx.exe").Hash;ImportSelection=$request.ImportSelection;IncludedSymbols=$expected.Count;PriorCompiledInputsIdentical=$true;OnlyAddedInput='import-selection-input.h';OnlyBuildCommandChange=$(if($commandChanges.Count -eq 1){$commandChanges[0]});BuildCommandChanges=$commandChanges;ImportContentsMatch=$pe.ImportContentsMatch;ImportSlotDifferences=($modules|Measure-Object DifferentSlots -Sum).Sum;ModuleDifferences=$modules;ExpectedThunkDelta=$ExpectedThunkDelta;ThunksOutsideStandardDelta=@($inventory.AllThunks|Where-Object {[Convert]::ToInt64($_.RebuiltVA,16)-[Convert]::ToInt64($_.OriginalVA,16) -ne $ExpectedThunkDelta}).Count;FullComparisonProduced=(Test-Path "$BuildDirectory\comparison.json");Scope='Controlled import-selection experiment, import identities/order and thunk positions. FullComparisonProduced records completion of the existing controller gates; import-content success alone does not override a failed body/placement gate. No whole binary or runtime equivalence claim.'}|ConvertTo-Json -Depth 5
