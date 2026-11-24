param([Parameter(Mandatory)][string]$ControlDirectory,[Parameter(Mandatory)][string]$OriginalBaseDirectory)
$ErrorActionPreference='Stop'
$control=(& "$PSScriptRoot\compare-data-constants.ps1" -BuildDirectory $ControlDirectory|Out-String)|ConvertFrom-Json
$rebased=(& "$PSScriptRoot\compare-data-constants.ps1" -BuildDirectory $OriginalBaseDirectory|Out-String)|ConvertFrom-Json
. "$PSScriptRoot\lib\pe-image.ps1"
$c=Open-PE "$ControlDirectory\fsx.exe";$r=Open-PE "$OriginalBaseDirectory\fsx.exe"
if($c.Base -ne 0x00400000 -or $r.Base -ne 0x01000000){throw 'Unexpected control/original image bases'}
$cc=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"));$rc=[IO.File]::ReadAllText((Resolve-Path "$OriginalBaseDirectory\build.cmd"))
if($cc -cne $rc.Replace('/BASE:0x01000000 ','')){throw 'Commands differ beyond base option'}
$cs=Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json;$rs=Get-Content "$OriginalBaseDirectory\source-hashes.json" -Raw|ConvertFrom-Json
if(($cs|Sort-Object File|ConvertTo-Json -Compress) -cne ($rs|Sort-Object File|ConvertTo-Json -Compress)){throw 'Sources differ'}
function Map-Rvas([string]$path,$pe){$symbols=@{};foreach($m in [regex]::Matches((Get-Content $path -Raw),'(?m)^\s*\S+\s+(\S+)\s+([0-9a-fA-F]{8})\b')){$va=[Convert]::ToInt64($m.Groups[2].Value,16);if($va -ge $pe.Base -and $va -lt $pe.Base+0x10000){$symbols[$m.Groups[1].Value]=$va-$pe.Base}};return $symbols}
$cm=Map-Rvas "$ControlDirectory\fsx.map" $c;$rm=Map-Rvas "$OriginalBaseDirectory\fsx.map" $r
$missing=@($cm.Keys|Where-Object {!$rm.ContainsKey($_)});$extra=@($rm.Keys|Where-Object {!$cm.ContainsKey($_)});$differences=@(foreach($name in $cm.Keys){if($rm.ContainsKey($name) -and $rm[$name] -ne $cm[$name]){[pscustomobject]@{Symbol=$name;ControlRva=$cm[$name];OriginalBaseRva=$rm[$name]}}})
$cp=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $ControlDirectory -RequireImportContents|Out-String)|ConvertFrom-Json;$rp=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $OriginalBaseDirectory -RequireImportContents|Out-String)|ConvertFrom-Json
$debugLengthEqual=($cp.RebuiltAuxiliaryMetadata.DebugRecords[0].Bytes -eq $rp.RebuiltAuxiliaryMetadata.DebugRecords[0].Bytes)
[pscustomobject]@{ControlSHA256=$control.RebuiltSHA256;OriginalBaseSHA256=$rebased.RebuiltSHA256;IdenticalSourceFiles=$true;OnlyBuildCommandChange='/BASE:0x01000000';ControlImageBase=('{0:X8}' -f $c.Base);RebuiltImageBase=('{0:X8}' -f $r.Base);MappedControlSymbols=$cm.Count;MappedOriginalBaseSymbols=$rm.Count;MissingSymbols=$missing;ExtraSymbols=$extra;RvaDifferences=$differences;AllMappedRvasAgree=(!$missing.Count -and !$extra.Count -and !$differences.Count);DebugRecordLengthsAgree=$debugLengthEqual;ControlPdbPath=$cp.RebuiltAuxiliaryMetadata.DebugRecords[0].PdbPath;OriginalBasePdbPath=$rp.RebuiltAuxiliaryMetadata.DebugRecords[0].PdbPath;Scope='Source/command-controlled base experiment; both runtime/data/import-content gates pass. Same-length job names control embedded PDB path length, but GUID/path/timestamps still differ. Symbol RVA equality is reported separately from actual image-base equality; no whole binary match or runtime execution claim.'}|ConvertTo-Json -Depth 7
