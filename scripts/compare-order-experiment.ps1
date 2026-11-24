param([Parameter(Mandatory)][string]$BuildDirectory,[string]$ControlDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\37-original-header")
$ErrorActionPreference='Stop'
$report=(& "$PSScriptRoot\compare-application-order.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
$old=@(Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json);$new=@(Get-Content "$BuildDirectory\source-hashes.json" -Raw|ConvertFrom-Json)
$response=@($new|Where-Object File -CEQ 'function-order-input.h');$cpp=@($new|Where-Object File -CNE 'function-order-input.h')
if($response.Count -ne 1 -or ($old|Sort-Object File|ConvertTo-Json -Compress) -cne ($cpp|Sort-Object File|ConvertTo-Json -Compress)){throw 'Unexpected application source/header changes'}
$a=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"));$b=[IO.File]::ReadAllText((Resolve-Path "$BuildDirectory\build.cmd"))
if($a -cne $b.Replace('/ORDER:@function-order-input.h ','')){throw 'Unexpected command change beyond function-order input'}
$pe=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $BuildDirectory -RequireImportContents|Out-String)|ConvertFrom-Json
$before=@{};foreach($entry in $pe.OriginalImportEntries){$before[$entry.Identity]=$entry.Index}
$different=@($pe.ImportEntries|Where-Object {$before[$_.Identity] -ne $_.Index}).Count
[pscustomobject]@{ControlSHA256=(Get-FileHash "$ControlDirectory\fsx.exe").Hash;OrderReport=$report;OriginalCppAndHeadersUnchanged=$true;AdditionalHashedResponseInput=$response[0];OnlyCommandChange='/ORDER:@function-order-input.h';ImportIdentitiesAtDifferentSlots=$different;Scope='Actual ordered build versus control37, original C++ inputs preserved and response data independently hashed/returned by worker. Selected entry placement measured separately from body/absolute reference/data/import matching.'}|ConvertTo-Json -Depth 8
