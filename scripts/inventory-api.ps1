param([string]$OutputDirectory="$PSScriptRoot\..\artifacts\api.dll\api-inventory")
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$inputPath="$PSScriptRoot\..\original\api.dll"
$hash=(Get-FileHash $inputPath).Hash
if($hash -cne '9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003'){throw 'Unexpected api.dll identity'}
$p=Open-PE $inputPath;Load-Imports $p
$pe=[BitConverter]::ToInt32($p.Bytes,60);$opt=$pe+24
if($p.Base -ne 0x200e0000 -or [BitConverter]::ToUInt16($p.Bytes,$pe+4) -ne 0x14c -or [BitConverter]::ToUInt16($p.Bytes,$opt) -ne 0x10b){throw 'Unexpected architecture/base'}
$directories=foreach($i in 0..15){[pscustomobject]@{Index=$i;Address=[BitConverter]::ToUInt32($p.Bytes,$opt+96+8*$i);Size=[BitConverter]::ToUInt32($p.Bytes,$opt+100+8*$i);AddressKind=$(if($i -eq 4){'FileOffset'}else{'RVA'})}}
$sections=for($i=0;$i -lt $p.Sections.Count;$i++){$s=$opt+[BitConverter]::ToUInt16($p.Bytes,$pe+20)+40*$i;[pscustomobject]@{Name=[Text.Encoding]::ASCII.GetString($p.Bytes,$s,8).Trim([char]0);RVA=[BitConverter]::ToUInt32($p.Bytes,$s+12);VirtualSize=[BitConverter]::ToUInt32($p.Bytes,$s+8);RawSize=[BitConverter]::ToUInt32($p.Bytes,$s+16);RawOffset=[BitConverter]::ToUInt32($p.Bytes,$s+20);Flags=('{0:X8}' -f [BitConverter]::ToUInt32($p.Bytes,$s+36))}}
$ed=$directories[0];$e=$p.Base+$ed.Address;$ordinalBase=U32 $p ($e+16);$count=U32 $p ($e+20);$namesCount=U32 $p ($e+24);$eat=$p.Base+(U32 $p ($e+28));$nameTable=$p.Base+(U32 $p ($e+32));$ordTable=$p.Base+(U32 $p ($e+36));$names=@{}
for($i=0;$i -lt $namesCount;$i++){$index=[BitConverter]::ToUInt16((Bytes $p ($ordTable+2*$i) 2),0);if($index -ge $count){throw 'Export name ordinal outside EAT'};$names[[int]$index]=CString $p ($p.Base+(U32 $p ($nameTable+4*$i)))}
$exports=for($i=0;$i -lt $count;$i++){$rva=U32 $p ($eat+4*$i);if(!$rva){continue};$forwarder=$null;if($rva -ge $ed.Address -and $rva -lt $ed.Address+$ed.Size){$forwarder=CString $p ($p.Base+$rva)};[pscustomobject]@{Ordinal=$ordinalBase+$i;Name=$names[$i];RVA=('{0:X8}' -f $rva);VA=('{0:X8}' -f ($p.Base+$rva));Forwarder=$forwarder}}
$get=@($exports|Where-Object Name -CEQ 'GetApplication');if($get.Count -ne 1 -or $get[0].VA -cne '20120442'){throw ("GetApplication export anchor differs: " + (($exports|Where-Object { $_.Name -match 'Application' -or $_.VA -eq '20120442' })|ConvertTo-Json -Compress))}
$vtable=foreach($i in 0..3){[pscustomobject]@{Offset=4*$i;Target=('{0:X8}' -f (U32 $p (0x200f2b24+4*$i)))}}
$expected=@('2012048A','2012052A','20122817','20120535');for($i=0;$i -lt 4;$i++){if($vtable[$i].Target -cne $expected[$i]){throw 'Boundary vtable differs'}}
$anchors=foreach($r in @(@('GetApplication',0x20120442,72),@('Initialize',0x2012048a,160),@('Slot04 listing',0x2012052a,11),@('GetWindowHandle',0x20120535,21),@('Slot08 listing',0x20122817,0),@('Exception callback wrapper',0x2013e3c1,90))){if(!$r[2]){continue};$bytes=Bytes $p $r[1] $r[2];$sha=[Security.Cryptography.SHA256]::Create();try{$bodyHash=[BitConverter]::ToString($sha.ComputeHash($bytes)).Replace('-','')}finally{$sha.Dispose()};[pscustomobject]@{Role=$r[0];VA=('{0:X8}' -f $r[1]);Bytes=$r[2];SHA256=$bodyHash;Hex=[BitConverter]::ToString($bytes).Replace('-','')}}
$imports=foreach($key in $p.Imports.Keys|Sort-Object){[pscustomobject]@{IatVA=('{0:X8}' -f $key);Identity=$p.Imports[$key]}}
$result=[pscustomobject]@{SHA256=$hash;ImageBase=('{0:X8}' -f $p.Base);Machine='014C';LinkerVersion=('{0}.{1}' -f $p.Bytes[$opt+2],$p.Bytes[$opt+3]);EntryVA=('{0:X8}' -f ($p.Base+[BitConverter]::ToUInt32($p.Bytes,$opt+16)));CoffTimestamp=[BitConverter]::ToUInt32($p.Bytes,$pe+8);Sections=$sections;Directories=$directories;ExportAddressTableCount=$count;NamedExportCount=$namesCount;Exports=@($exports);Imports=@($imports);BoundaryVTable=$vtable;BoundaryBodies=$anchors;Scope='Original image inventory and exact original byte anchors only; no reconstructed DLL body match, complete function inventory, original type names or runtime proof. Slot04 length is a listing span, not a stored function claim.'}
New-Item -ItemType Directory -Path $OutputDirectory -Force|Out-Null
$result|ConvertTo-Json -Depth 8|Set-Content "$OutputDirectory\original.json"
$exports|Export-Csv "$OutputDirectory\exports.csv" -NoTypeInformation
$imports|Export-Csv "$OutputDirectory\imports.csv" -NoTypeInformation
[pscustomobject]@{SHA256=$hash;Exports=@($exports).Count;NamedExports=$namesCount;Imports=@($imports).Count;Sections=$sections;GetApplication=$get;BoundaryVTable=$vtable}|ConvertTo-Json -Depth 5


