param([Parameter(Mandatory)][string]$ControlDirectory,[Parameter(Mandatory)][string]$BasenameDirectory,[Parameter(Mandatory)][string]$RepeatDirectory)
$ErrorActionPreference='Stop'
$control=(& "$PSScriptRoot\verify-debug-record.ps1" -BuildDirectory $ControlDirectory -PdbPathMode Worker|Out-String)|ConvertFrom-Json
$first=(& "$PSScriptRoot\verify-debug-record.ps1" -BuildDirectory $BasenameDirectory -PdbPathMode Basename|Out-String)|ConvertFrom-Json
$repeat=(& "$PSScriptRoot\verify-debug-record.ps1" -BuildDirectory $RepeatDirectory -PdbPathMode Basename|Out-String)|ConvertFrom-Json
$cc=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"));$fc=[IO.File]::ReadAllText((Resolve-Path "$BasenameDirectory\build.cmd"));$rc=[IO.File]::ReadAllText((Resolve-Path "$RepeatDirectory\build.cmd"))
if($cc -cne $fc.Replace('/PDBALTPATH:fsx.pdb ','') -or $fc -cne $rc){throw 'Commands differ beyond altpath option/control'}
function Source-Identity([string]$dir){(Get-Content "$dir\source-hashes.json" -Raw|ConvertFrom-Json|Sort-Object File|ConvertTo-Json -Compress)}
if((Source-Identity $ControlDirectory) -cne (Source-Identity $BasenameDirectory) -or (Source-Identity $BasenameDirectory) -cne (Source-Identity $RepeatDirectory)){throw 'Source hashes differ'}
. "$PSScriptRoot\lib\pe-image.ps1"
$a=Open-PE "$BasenameDirectory\fsx.exe";$b=Open-PE "$RepeatDirectory\fsx.exe"
if($a.Bytes.Length -ne $b.Bytes.Length -or $a.Base -ne $b.Base){throw 'Repeat file extent/base changed'}
function File-Offset($p,[long]$rva){foreach($s in $p.Sections){if($rva -ge $s[0] -and $rva -lt $s[0]+$s[1]){return $s[2]+$rva-$s[0]}};throw 'Unmapped repeat RVA'}
function Metadata-Ranges($p,$record){$pe=[BitConverter]::ToInt32($p.Bytes,60);$debugRva=[BitConverter]::ToUInt32($p.Bytes,$pe+24+96+6*8);@(@{Role='COFF timestamp';Offset=$pe+8;Bytes=4},@{Role='Debug directory timestamp';Offset=(File-Offset $p $debugRva)+4;Bytes=4},@{Role='RSDS GUID';Offset=$record.FileOffset+4;Bytes=16})}
$ar=@(Metadata-Ranges $a $first.RebuiltRecord);$br=@(Metadata-Ranges $b $repeat.RebuiltRecord)
if(($ar|ConvertTo-Json -Compress) -cne ($br|ConvertTo-Json -Compress)){throw 'Repeat metadata locations changed'}
$ignored=@{};foreach($r in $ar){for($i=0;$i -lt $r.Bytes;$i++){$ignored[[long]($r.Offset+$i)]=$true}}
$other=@();$rawDifferences=0;for($i=0;$i -lt $a.Bytes.Length;$i++){if($a.Bytes[$i] -ne $b.Bytes[$i]){$rawDifferences++;if(!$ignored.ContainsKey([long]$i)){$other+=$i}}}
[pscustomobject]@{ControlSHA256=$control.RebuiltSHA256;BasenameSHA256=$first.RebuiltSHA256;RepeatSHA256=$repeat.RebuiltSHA256;IdenticalSourceFiles=$true;OnlyControlCommandChange='/PDBALTPATH:fsx.pdb';BasenameAndRepeatCommandsIdentical=$true;OriginalRecord=$first.OriginalRecord;BasenameRecord=$first.RebuiltRecord;RepeatRecord=$repeat.RebuiltRecord;RepeatRawDifferingBytes=$rawDifferences;RepeatExcludedMetadataFields=$ar;RepeatOtherDifferingOffsets=$other;RepeatEqualOutsideKnownMetadata=($other.Count -eq 0);Scope='Actual builds in differently named job directories. Path/type/RSDS/age/extent checked against original shape. Repeat comparison excludes only parsed COFF/debug timestamps and RSDS GUID at unchanged semantic offsets. This is not original GUID/timestamp/PDB identity or deterministic whole-file equality.'}|ConvertTo-Json -Depth 8
