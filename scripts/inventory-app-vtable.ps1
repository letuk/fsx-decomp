param([string]$BuildDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support",
      [string]$OutputDirectory="$PSScriptRoot\..\artifacts\fsx.exe\vtables")
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$original="$PSScriptRoot\..\original\fsx.exe"
$a=Open-PE $original;$b=Open-PE "$BuildDirectory\fsx.exe";Load-Imports $a;Load-Imports $b
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
$lifecycle=(& "$PSScriptRoot\compare-app-lifecycle.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
$dispatch=$null
if($map.Contains('?GetMessageMap@FSXAppPilot@@') -and $map.Contains('?Run@FSXAppPilot@@')){
 $dispatch=(& "$PSScriptRoot\compare-app-dispatch.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
}
$v=[regex]::Match($map,'(?m)^\s*\S+\s+\?\?_7FSXAppPilot@@6B@\s+([0-9a-fA-F]{8})')
if(!$v.Success){throw 'Rebuilt application vtable missing'}
$vt=[Convert]::ToInt64($v.Groups[1].Value,16)
function Import-Thunk($pe,[long]$target){$bytes=Bytes $pe $target 6;if($bytes[0] -eq 255 -and $bytes[1] -eq 37){return $pe.Imports[[long][BitConverter]::ToUInt32($bytes,2)]};return ''}
$roles=@{4='scalar deleting destructor';8='CObject::Serialize';12='CObject::AssertValid';16='CObject::Dump';48='GetMessageMap';88='InitInstance';92='Run';112='ExitInstance'}
$rows=foreach($slot in 0..47){
 $offset=4*$slot;$x=U32 $a (0x01001398+$offset);$y=U32 $b ($vt+$offset)
 $xi=Import-Thunk $a $x;$yi=Import-Thunk $b $y
 $symbols=@([regex]::Matches($map,'(?im)^\s*\S+\s+(\S+)\s+'+('{0:X8}' -f $y)+'\b')|ForEach-Object {$_.Groups[1].Value})
 $category='';$status='';$role=$roles[$offset]
 if($xi){$category='MFC import';$status=if($xi -ceq $yi){'DLL+ordinal/name identity verified'}else{'import mismatch'};if(!$role){$role=$symbols -join ' / '}}
 elseif($offset -in 8,12,16){
  $category='SDK leaf stub';$length=if($offset -eq 12){1}else{3}
  $status=if([Convert]::ToBase64String((Bytes $a $x $length)) -ceq [Convert]::ToBase64String((Bytes $b $y $length))){'raw body equal'}else{'stub differs'}
 }elseif($offset -in 4,112){$category='application override';$verified=if($offset -eq 4){$lifecycle.Destructor.AddressNormalizedExactMatch}else{$lifecycle.ExitInstance.AddressNormalizedExactMatch};$status=if($verified){'normalized body match; lifecycle comparator checked'}else{'body match not established'}}
 elseif($offset -eq 88){$category='application override';$status='not certified; see InitInstance diagnostics'}
 elseif($offset -in 48,92){$category='application override';$verified=if($offset -eq 48){$dispatch.GetMessageMap.AddressNormalizedExactMatch}else{$dispatch.Run.AddressNormalizedExactMatch};$status=if($verified){'normalized body match; dispatch comparator checked'}elseif($yi){'missing override: rebuilt inherits MFC default'}else{'local rebuilt override; body comparison required'}}
 else{throw ('Unclassified local slot {0:X}' -f $offset)}
 [pscustomobject]@{Slot=$slot;Offset=('{0:X2}' -f $offset);OriginalTarget=('{0:X8}' -f $x);RebuiltTarget=('{0:X8}' -f $y);Role=$role;Category=$category;Status=$status;OriginalImport=$xi;RebuiltImport=$yi;RebuiltSymbols=($symbols -join ' / ')}
}
if(@($rows|Where-Object Status -EQ 'import mismatch').Count){throw 'An inherited import identity differs'}
New-Item -ItemType Directory -Path $OutputDirectory -Force|Out-Null
$rows|Export-Csv "$OutputDirectory\application-vtable.csv" -NoTypeInformation
[pscustomobject]@{OriginalSHA256=(Get-FileHash $original).Hash;RebuiltSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;OriginalTable='01001398';OriginalRangeEnd='01001457';NextKnownTable='01001458 (OLE filter)';RebuiltTable=('{0:X8}' -f $vt);Scope='48 observed slots ending before known OLE table, checked against actual SDK-based rebuilt table. No original class-name or full object-size claim. Body match statuses refer to separately validated lifecycle evidence.';Slots=$rows}|ConvertTo-Json -Depth 5|Set-Content "$OutputDirectory\application-vtable.json"
$rows|ConvertTo-Html -Title 'FSX application vtable inventory' -PreContent '<h1>FSX application vtable inventory</h1><p>Observed slots, not proof of complete original class layout. Import identities are DLL+ordinal/name; body matches have separately documented scope.</p>'|Set-Content "$OutputDirectory\application-vtable.html"
$rows|Group-Object Category|Select-Object Name,Count|ConvertTo-Json


