param([string]$BuildDirectory="$PSScriptRoot\..\vm-share\modules\api\runs\api-window-20261103-001")
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$original="$PSScriptRoot\..\original\api.dll"
if((Get-FileHash $original).Hash -cne '9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003'){throw 'Original identity differs'}
$a=Open-PE $original;$b=Open-PE "$BuildDirectory\api-pilot.dll"
$pe=[BitConverter]::ToInt32($b.Bytes,60)
if(!([BitConverter]::ToUInt16($b.Bytes,$pe+22) -band 0x2000) -or [BitConverter]::ToUInt32($b.Bytes,$pe+40) -ne 0){throw 'Expected no-entry DLL fragment'}
$map=Get-Content "$BuildDirectory\api-pilot.map" -Raw
function Symbol([string]$name){$names=@($name);if($name.Contains('ApplicationBoundaryPilot@@QAE')){$names+= $name.Replace('ApplicationBoundaryPilot@@QAE','ApplicationBoundaryPilot@@UAE')};$hits=@(foreach($n in $names){[regex]::Matches($map,'(?m)^\s*\S+\s+'+[regex]::Escape($n)+'\s+([0-9a-fA-F]{8})\b')});if($hits.Count -ne 1){throw "Missing/ambiguous symbol $name"};return [Convert]::ToUInt32($hits[0].Groups[1].Value,16)}
$entry=Symbol '?GetWindowHandle@ApplicationBoundaryPilot@@QAEPAUHWND__@@H@Z';$global=Symbol '?g_mainWindowHandlePilot@@3PAUHWND__@@A'
$x=Bytes $a 0x20120535 21;$y=Bytes $b $entry 21
function Accept([byte[]]$candidate,[long]$target){
 if([BitConverter]::ToUInt32($x,13) -ne 0x2020e010 -or [BitConverter]::ToUInt32($candidate,13) -ne $target){return $false}
 $copy=[byte[]]$candidate.Clone();[Array]::Copy($x,13,$copy,13,4)
 return [Convert]::ToBase64String($x) -ceq [Convert]::ToBase64String($copy)
}
if(!(Accept $y $global)){throw 'Full 21-byte mapped comparison failed'}
# Both globals start as zero. The candidate global is virtual zero-filled .bss.
if((U32 $a 0x2020e010) -ne 0){throw 'Original HWND initial state differs'}
$s=$b.Sections|Where-Object {$global -ge $b.Base+$_[0] -and $global -lt $b.Base+$_[0]+4096}
if(!$s -or $global -lt $b.Base+$s[0]+$s[1]){throw 'Expected candidate virtual zero-filled global'}
$bad=[byte[]]$y.Clone();$bad[5]=$bad[5] -bxor 1
if(Accept $bad $global){throw 'Opcode negative control accepted'}
if(Accept $y ($global+4)){throw 'Wrong-global negative control accepted'}
$r=[pscustomobject]@{OriginalSHA256=(Get-FileHash $original).Hash;RebuiltSHA256=(Get-FileHash "$BuildDirectory\api-pilot.dll").Hash;OriginalVA='20120535';RebuiltVA=('{0:X8}' -f $entry);Bytes=21;AddressNormalizedExactMatch=$true;ReferenceOffset=13;OriginalGlobal='2020E010';RebuiltGlobal=('{0:X8}' -f $global);InitialZeroStateVerified=$true;NegativeControls=@('changed opcode rejected','wrong global target rejected');Scope='Complete GetWindowHandle body; single MAP-resolved global field normalized only after exact target and initial-state checks. Normal branches/RET4/registers remain exact. No whole Application layout, original name, export144, DLL startup/runtime or whole-DLL match.'}
$r|ConvertTo-Json -Depth 5|Set-Content "$BuildDirectory\comparison.json"
$r|ConvertTo-Json -Depth 5
