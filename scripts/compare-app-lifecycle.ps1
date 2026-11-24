param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$original="$PSScriptRoot\..\original\fsx.exe";$pilot=Join-Path $BuildDirectory 'fsx.exe'
$a=Open-PE $original;$b=Open-PE $pilot;Load-Imports $a;Load-Imports $b
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
$listing=Get-Content "$BuildDirectory\disassembly.txt"
function Require($condition,[string]$message){if(!$condition){throw $message}}
function Symbol([string]$name){$m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($name)+'\s+([0-9a-fA-F]{8})\b');Require $m.Success "Missing symbol $name";return [Convert]::ToInt64($m.Groups[1].Value,16)}
function Relative($pe,[long]$entry,[int]$offset){return $entry+$offset+4+[BitConverter]::ToInt32((Bytes $pe ($entry+$offset) 4),0)}
function Import-Identity($pe,[long]$iat){$name=$pe.Imports[$iat];Require ($null -ne $name) 'Missing IAT entry';return $name}
function Thunk-Identity($pe,[long]$entry){$x=Bytes $pe $entry 6;Require ($x[0] -eq 255 -and $x[1] -eq 37) 'Expected import thunk';return Import-Identity $pe ([BitConverter]::ToUInt32($x,2))}
function Body-Length([long]$entry){
 $started=$false;$end=$null
 # DUMPBIN may omit a label for a following static function. Bound the scan
 # with the next actual MAP symbol instead of consuming that function's JMP.
 $next=@([regex]::Matches($map,'(?m)^\s*\S+\s+\S+\s+([0-9a-fA-F]{8})\b')|ForEach-Object {[Convert]::ToInt64($_.Groups[1].Value,16)}|Where-Object {$_ -gt $entry}|Sort-Object|Select-Object -First 1)
 Require ($next.Count -eq 1) 'Next mapped boundary not found'
 foreach($line in $listing){
  if($line -match '^\s+([0-9a-fA-F]{8}):\s+(?:[0-9a-fA-F]{2} )+\s+([a-z][a-z0-9]*)\b(.*)$'){
   $va=[Convert]::ToInt64($Matches[1],16)
   if($started -and $va -ge $next[0]){break}
   if($va -eq $entry){$started=$true}
   if($started -and $Matches[2] -eq 'ret'){$end=$va+$(if($Matches[3].Trim()) {3}else{1})}
   if($started -and $Matches[2] -eq 'jmp' -and $Matches[3].Trim() -match '^\?'){$end=$va+5}
  }elseif($started -and $line -match '^\S.*:\s*$'){break}
 }
 Require ($null -ne $end) 'Function return boundary not found'
 return $end-$entry
}
$ctor=Symbol '??0FSXAppPilot@@QAE@XZ';$dtor=Symbol '??_GFSXAppPilot@@UAEPAXI@Z'
$object=Symbol '?g_fsxAppPilot@@3VFSXAppPilot@@A';$vt=Symbol '??_7FSXAppPilot@@6B@'
Require ((U32 $a 0x010013f0) -eq 0x010019e0 -and (U32 $b ($vt+0x58)) -eq (Symbol '?InitInstance@FSXAppPilot@@UAEHXZ')) 'InitInstance vtable association changed'
Require ((U32 $a 0x0100139c) -eq 0x010016d0 -and (U32 $b ($vt+4)) -eq $dtor) 'Deleting destructor vtable association changed'
$identities=[Collections.Generic.List[object]]::new()
function Compare-LifecycleBody([string]$name,[long]$old,[long]$new,[int]$length,$fields){
 Require ((Body-Length $new) -eq $length) "$name function length differs"
 $x=Bytes $a $old $length;$y=Bytes $b $new $length
 foreach($field in $fields){
  $offset=[int]$field[0];$expected=[long]$field[1];$kind=$field[2]
  if($kind -eq 'call'){$ox=Relative $a $old $offset;$ny=Relative $b $new $offset;Require ($ox -eq $expected) 'Original call target changed';$identity=Thunk-Identity $a $ox;Require ($identity -ceq (Thunk-Identity $b $ny)) 'Imported call identity differs'}
  else{
   $ox=[long][BitConverter]::ToUInt32($x,$offset);$ny=[long][BitConverter]::ToUInt32($y,$offset);Require ($ox -eq $expected) 'Original absolute target changed'
   switch($kind){
    'object' {Require ($ny -eq $object) 'Object mapping differs';$identity='mapped application global'}
    'event' {Require ($ny -eq $object+0xa4) 'Event mapping differs';$identity='mapped event field +0xa4'}
    'main-window' {Require ($ny -eq $object+0x20) 'Main window pointer mapping differs';$identity='mapped main-window pointer +0x20'}
    'getter' {Require ($ny -eq (Symbol '?pfnGetApplication@@3P6APAVApplicationPilot@@XZA')) 'Application getter mapping differs';$identity='mapped application getter global'}
    'vtable' {Require ($ny -eq $vt) 'Vtable mapping differs';$identity='mapped derived vtable; selected slots checked'}
    'string' {Require ((CString $a $ox) -ceq 'FS98MAIN' -and (CString $b $ny) -ceq 'FS98MAIN') 'Event name differs';$identity='FS98MAIN exact string'}
    'iat' {$identity=Import-Identity $a $ox;Require ($identity -ceq (Import-Identity $b $ny)) 'IAT identity differs'}
    default {throw 'Unknown mapping kind'}
   }
  }
  $identities.Add([pscustomobject]@{Function=$name;Offset=('{0:X}' -f $offset);Identity=$identity})
  [Array]::Copy($x,$offset,$y,$offset,4)
 }
 $differences=@(for($i=0;$i -lt $length;$i++){if($x[$i] -ne $y[$i]){$i}})
 Require (!$differences.Count) "$name non-reference bytes differ: $differences"
 return [pscustomobject]@{Name=$name;OriginalVA=('{0:X8}' -f $old);RebuiltVA=('{0:X8}' -f $new);Length=$length;VerifiedAddressFields=$fields.Count;AddressNormalizedExactMatch=$true}
}
$constructor=Compare-LifecycleBody 'Application event constructor' 0x01001660 $ctor 95 @(
 @(3,0x010043e0,'object'),@(8,0x0100202c,'call'),@(0x0d,0x010012f8,'string'),
 @(0x19,0x010043e0,'object'),@(0x1d,0x01001398,'vtable'),@(0x23,0x01004484,'event'),
 @(0x2d,0x01001008,'iat'),@(0x32,0x01004484,'event'),@(0x38,0x01001004,'iat'),
 @(0x44,0x01004484,'event'),@(0x4b,0x0100100c,'iat'),@(0x51,0x01004484,'event'),@(0x5a,0x010043e0,'object'))
$destructor=Compare-LifecycleBody 'Application scalar deleting destructor' 0x010016d0 $dtor 70 @(
 @(0x12,0x01001398,'vtable'),@(0x1b,0x0100100c,'iat'),@(0x2c,0x01002038,'call'),@(0x38,0x01001ffc,'call'))
$exit=$null
if($map.Contains('?ExitInstance@FSXAppPilot@@UAEHXZ')){
 $entry=Symbol '?ExitInstance@FSXAppPilot@@UAEHXZ'
 Require ((U32 $a 0x01001408) -eq 0x01001720 -and (U32 $b ($vt+0x70)) -eq $entry) 'ExitInstance vtable association changed'
 $exit=Compare-LifecycleBody 'Application ExitInstance' 0x01001720 $entry 58 @(
  @(7,0x01004400,'main-window'),@(0x10,0x01002044,'call'),@(0x16,0x01004400,'main-window'),
  @(0x1f,0x01004384,'getter'),@(0x36,0x01002050,'call'))
}
[pscustomobject]@{OriginalSHA256=(Get-FileHash $original).Hash;RebuiltSHA256=(Get-FileHash $pilot).Hash;Constructor=$constructor;Destructor=$destructor;ExitInstance=$exit;VerifiedReferences=@($identities);Scope='Complete reported function bodies after target-validated address normalization. Import DLL+ordinal/name identities, event string and object/event/vtable mappings checked. Destructor/InitInstance and, if present, ExitInstance vtable slots checked; no full application vtable, RTTI, EH/CRT-registration metadata, external implementation or whole-file equality claim.'}|ConvertTo-Json -Depth 6

