param([string]$BuildDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support")
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$a=Open-PE "$PSScriptRoot\..\original\fsx.exe";$b=Open-PE "$BuildDirectory\fsx.exe";Load-Imports $a;Load-Imports $b
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
$symbols=@([regex]::Matches($map,'(?m)^\s*\S+\s+(\S+)\s+([0-9a-fA-F]{8})\s+f\b([^\r\n]*)')|ForEach-Object {[pscustomobject]@{Name=$_.Groups[1].Value;VA=[Convert]::ToInt64($_.Groups[2].Value,16);Member=$_.Groups[3].Value.Trim()}})
function Thunks($p){$h=[BitConverter]::ToInt32($p.Bytes,60);$table=$h+24+[BitConverter]::ToUInt16($p.Bytes,$h+20);for($n=0;$n -lt $p.Sections.Count;$n++){if(!([BitConverter]::ToUInt32($p.Bytes,$table+40*$n+36) -band 0x20000000)){continue};$s=$p.Sections[$n];for($i=0;$i+6 -le $s[1];$i++){$o=$s[2]+$i;if($p.Bytes[$o] -eq 255 -and $p.Bytes[$o+1] -eq 37){$iat=[long][BitConverter]::ToUInt32($p.Bytes,$o+2);if($p.Imports.ContainsKey($iat)){[pscustomobject]@{VA=$p.Base+$s[0]+$i;Identity=$p.Imports[$iat]}}}}}}
$old=@(Thunks $a);$new=@(Thunks $b)
$allRows=foreach($t in $old){$match=@($new|Where-Object Identity -CEQ $t.Identity);if($match.Count -ne 1){throw 'Thunk identity not unique in rebuild'};$entry=$match[0].VA;$names=@($symbols|Where-Object VA -EQ $entry);[pscustomobject]@{OriginalVA=('{0:X8}' -f $t.VA);RebuiltVA=('{0:X8}' -f $entry);Identity=$t.Identity;Symbols=@($names.Name);Members=@($names.Member)}}
$rows=@($allRows|Where-Object {[Convert]::ToInt64($_.OriginalVA,16) -ge 0x01001ffc -and [Convert]::ToInt64($_.OriginalVA,16) -lt 0x01002313})
[pscustomobject]@{AllThunks=@($allRows);OriginalThunks=$old.Count;RebuiltThunks=$new.Count;PreCrtThunkCount=@($rows).Count;PreCrtThunks=@($rows);RuntimeMembers=@($symbols|Where-Object Member -Match '^msvcrt:'|Sort-Object VA);Scope='Original FF25 targets matched by exact import identity, unique rebuilt thunk and actual public MAP names/member provenance. Does not prove COMDAT eligibility or import entry order.'}|ConvertTo-Json -Depth 7
