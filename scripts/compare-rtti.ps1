param([Parameter(Mandatory)][string]$BuildDirectory,[switch]$RequireOriginalShape)
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$original="$PSScriptRoot\..\original\fsx.exe"
if((Get-FileHash $original).Hash -ne '61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735'){throw 'Original identity differs'}
$a=Open-PE $original;$b=Open-PE "$BuildDirectory\fsx.exe";$map=Get-Content "$BuildDirectory\fsx.map" -Raw
Load-Imports $a;Load-Imports $b
function Mapped($pe,[long]$va,[int]$length){foreach($s in $pe.Sections){if($va -ge $pe.Base+$s[0] -and $va+$length -le $pe.Base+$s[0]+$s[1]){return $true}};return $false}
function Has-Col($pe,[long]$table){
 $col=U32 $pe ($table-4)
 if(!(Mapped $pe $col 20)){return $false}
 if((U32 $pe $col) -ne 0 -or (U32 $pe ($col+4)) -ne 0 -or (U32 $pe ($col+8)) -ne 0){return $false}
 $td=U32 $pe ($col+12);$chd=U32 $pe ($col+16)
 if(!(Mapped $pe $td 12) -or !(Mapped $pe $chd 16)){return $false}
 $name=CString $pe ($td+8)
 return ($name.StartsWith('.?AV') -or $name.StartsWith('.?AU')) -and (U32 $pe $chd) -eq 0 -and (U32 $pe ($chd+8)) -ge 1 -and (U32 $pe ($chd+8)) -le 128
}
function Type-Names($pe){
 $raw=$pe.Bytes;$header=[BitConverter]::ToInt32($raw,60);$table=$header+24+[BitConverter]::ToUInt16($raw,$header+20)
 for($i=0;$i -lt $pe.Sections.Count;$i++){$section=$table+40*$i;$name=[Text.Encoding]::ASCII.GetString($raw,$section,8).Trim([char]0);if($name -eq '.rsrc'){continue};$s=$pe.Sections[$i];$text=[Text.Encoding]::ASCII.GetString($raw,$s[2],$s[1]);foreach($m in [regex]::Matches($text,'\.\?A[UV][\x21-\x7e]{1,100}\x00')){[pscustomobject]@{VA=('{0:X8}' -f ($pe.Base+$s[0]+$m.Index));Name=$m.Value.TrimEnd([char]0)}}}
}
$rows=foreach($pair in @(@('Application',0x01001398,'??_7FSXAppPilot@@6B@'),@('Message filter',0x01001458,'??_7FSXMessageFilterPilot@@6B@'))){
 $m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($pair[2])+'\s+([0-9a-fA-F]{8})\b');if(!$m.Success){throw 'Missing rebuilt vtable'};$vt=[Convert]::ToInt64($m.Groups[1].Value,16)
 [pscustomobject]@{Role=$pair[0];OriginalTable=('{0:X8}' -f $pair[1]);RebuiltTable=('{0:X8}' -f $vt);OriginalPrecedingWord=('{0:X8}' -f (U32 $a ($pair[1]-4)));RebuiltPrecedingWord=('{0:X8}' -f (U32 $b ($vt-4)));OriginalHasLocalCol=(Has-Col $a $pair[1]);RebuiltHasLocalCol=(Has-Col $b $vt)}
}
$oldNames=@(Type-Names $a);$newNames=@(Type-Names $b);$symbols=@([regex]::Matches($map,'(?m)^\s*\S+\s+(\?\?_R\S+)\s+[0-9a-fA-F]{8}\b')|ForEach-Object {$_.Groups[1].Value})
$importRows=foreach($identity in @('msvcr80.dll!?terminate@@YAXXZ','msvcr80.dll!?_type_info_dtor_internal_method@type_info@@QAEXXZ')){[pscustomobject]@{Identity=$identity;OriginalPresent=($identity -cin $a.Imports.Values);RebuiltPresent=($identity -cin $b.Imports.Values)}}
$match=($oldNames.Count -eq 0 -and $newNames.Count -eq 0 -and $symbols.Count -eq 0 -and @($rows|Where-Object {$_.OriginalHasLocalCol -or $_.RebuiltHasLocalCol}).Count -eq 0 -and @($importRows|Where-Object {$_.OriginalPresent -ne $_.RebuiltPresent}).Count -eq 0)
if($RequireOriginalShape -and !$match){throw 'Rebuilt local RTTI/type-name presence differs from observed original shape'}
[pscustomobject]@{OriginalSHA256=(Get-FileHash $original).Hash;RebuiltSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;VTables=$rows;OriginalTypeNames=$oldNames;RebuiltTypeNames=$newNames;RebuiltRttiSymbols=$symbols;SelectedImports=$importRows;ObservedAbsenceShapeMatches=$match;Scope='Two known primary vtable predecessor checks, valid local x86 COL recognition and non-resource-section decorated class/struct type-name scan, rebuilt MAP RTTI symbols and two selected import identities. Absence at these anchors and scanned names does not prove a unique original compiler flag, full class layout, external DLL RTTI absence or whole-data identity.'}|ConvertTo-Json -Depth 6
