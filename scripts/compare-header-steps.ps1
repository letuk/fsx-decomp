param([string]$ControlDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\36-basename-pdb")
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$steps=@(@('header-step1-large-20261103','/LARGEADDRESSAWARE','CoffFlags',22,2),@('header-step2-nx-20261103','/NXCOMPAT','DllFlags',94,2),@('header-step3-ts-20261103','/TSAWARE','DllFlags',94,2),@('header-step4-image-20261103','/VERSION:5.2','ImageVersion',68,4),@('header-step5-os-20261103','/OSVERSION:5.2','OsVersion',64,4))
function Sources([string]$dir){Get-Content "$dir\source-hashes.json" -Raw|ConvertFrom-Json|Sort-Object File|ConvertTo-Json -Compress}
function File-Offset($p,[long]$rva){foreach($s in $p.Sections){if($rva -ge $s[0] -and $rva -lt $s[0]+$s[1]){return $s[2]+$rva-$s[0]}};throw 'Unmapped metadata'}
function Fields($p,$step){$pe=[BitConverter]::ToInt32($p.Bytes,60);$debugRva=[BitConverter]::ToUInt32($p.Bytes,$pe+24+96+6*8);$debugFile=File-Offset $p $debugRva;$cvFile=[BitConverter]::ToUInt32($p.Bytes,$debugFile+24);@(@{Role=$step[2];Offset=$pe+$step[3];Bytes=$step[4]},@{Role='COFF timestamp';Offset=$pe+8;Bytes=4},@{Role='Debug timestamp';Offset=$debugFile+4;Bytes=4},@{Role='RSDS GUID';Offset=$cvFile+4;Bytes=16})}
$previous=$ControlDirectory;$rows=@()
foreach($step in $steps){
 $id=$step[0]+'-001';$dir="$PSScriptRoot\..\vm-share\modules\fsx\runs\$($step[0])\candidates\$id"
 $before=[IO.File]::ReadAllText((Resolve-Path "$previous\build.cmd"));$after=[IO.File]::ReadAllText((Resolve-Path "$dir\build.cmd"))
 if($before -cne $after.Replace(($step[1]+' '),'')){throw 'Step has additional command changes'}
 if((Sources $previous) -cne (Sources $dir)){throw 'Step source hashes differ'}
 $a=Open-PE "$previous\fsx.exe";$b=Open-PE "$dir\fsx.exe"
 if($a.Bytes.Length -ne $b.Bytes.Length){throw 'Step file extents differ'}
 $fa=@(Fields $a $step);$fb=@(Fields $b $step)
 if(($fa|ConvertTo-Json -Compress) -cne ($fb|ConvertTo-Json -Compress)){throw 'Metadata/header locations moved'}
 $allowed=@{};foreach($field in $fa){for($i=0;$i -lt $field.Bytes;$i++){$allowed[[long]($field.Offset+$i)]=$true}}
 $unexpected=@();$rawCount=0;for($i=0;$i -lt $a.Bytes.Length;$i++){if($a.Bytes[$i] -ne $b.Bytes[$i]){$rawCount++;if(!$allowed.ContainsKey([long]$i)){$unexpected+=$i}}}
 $comparison=Get-Content "$dir\comparison.json" -Raw|ConvertFrom-Json
 $rows+=[pscustomobject]@{Job=$id;AddedOption=$step[1];Header=$comparison.PeHeaderSettings.Rebuilt;SourceHashesIdentical=$true;OnlyCommandDelta=$step[1];RawDifferingBytes=$rawCount;ExpectedChangedFields=$fa;UnexpectedDifferingOffsets=$unexpected;EqualOutsideExpectedFields=($unexpected.Count -eq 0);SHA256=(Get-FileHash "$dir\fsx.exe").Hash}
 $previous=$dir
}
[pscustomobject]@{Steps=$rows;AllStepsEqualOutsideExpectedFields=(@($rows|Where-Object {!$_.EqualOutsideExpectedFields}).Count -eq 0);Scope='Five actual cumulative single-option builds, source and command identity checked per step. Whole-file pair comparison excludes only the intended header field plus parsed COFF/debug timestamps and RSDS GUID at unchanged locations. Full original equality, debug identity and original command uniqueness are not claimed.'}|ConvertTo-Json -Depth 9
