param([Parameter(Mandatory)][string]$BuildDirectory,[string]$ControlDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\38-application-order")
$ErrorActionPreference='Stop'
$order=(& "$PSScriptRoot\compare-application-order.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
if($order.Count -ne 22){throw 'Expected reviewed22-symbol scope'}
$old=@(Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json);$new=@(Get-Content "$BuildDirectory\source-hashes.json" -Raw|ConvertFrom-Json)
if((@($old.File|Sort-Object)|ConvertTo-Json -Compress) -cne (@($new.File|Sort-Object)|ConvertTo-Json -Compress)){throw 'Unexpected source input list'}
foreach($file in $old){if($file.File -cnotin @('startup_context_pilot.cpp','function-order-input.h')){if($file.Hash -cne ($new|Where-Object File -CEQ $file.File).Hash){throw 'Additional source change'}}}
$before=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\source\startup_context_pilot.cpp"));$after=[IO.File]::ReadAllText((Resolve-Path "$BuildDirectory\source\startup_context_pilot.cpp"))
if($before.Replace('static MSG* WINAPI GetCurrentMessagePilot()','MSG* WINAPI GetCurrentMessagePilot()') -cne $after){throw 'Additional startup source change'}
if([IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd")) -cne [IO.File]::ReadAllText((Resolve-Path "$BuildDirectory\build.cmd"))){throw 'Build command changed'}
$comparison=Get-Content "$BuildDirectory\comparison.json" -Raw|ConvertFrom-Json
$pe=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $BuildDirectory -RequireImportContents|Out-String)|ConvertFrom-Json
$oldSlots=@{};foreach($e in $pe.OriginalImportEntries){$oldSlots[$e.Identity]=$e.Index}
$count=@($pe.ImportEntries|Where-Object {$oldSlots[$_.Identity] -ne $_.Index}).Count
[pscustomobject]@{ControlSHA256=(Get-FileHash "$ControlDirectory\fsx.exe").Hash;Order=$order;OnlyStartupSourceEdit='Remove static from reconstructed callback definition; original linkage unknown';OtherCppAndHeadersUnchanged=$true;BuildCommandsIdentical=$true;OrderResponseChanged=$true;OriginalExportDirectoryAbsent=($pe.OriginalHeader.Directories[0].Size -eq 0);RebuiltExportDirectoryAbsent=($pe.RebuiltHeader.Directories[0].Size -eq 0);Callback=$comparison.Dispatch.Callback;ImportSlotDifferences=$count;Scope='Controlled reconstructed linkage and response-order changes, existing5-byte callback/reference gate preserved. External C++ linkage does not add a PE export here. No original linkage/symbol-name/whole binary equality claim.'}|ConvertTo-Json -Depth 8
