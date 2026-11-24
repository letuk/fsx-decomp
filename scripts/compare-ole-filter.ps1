param([string]$BuildDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\23-ole-filter-verified-matches")
$ErrorActionPreference='Stop'
function Open-PE([string]$Path){
 $b=[IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $Path));$pe=[BitConverter]::ToInt32($b,60);$opt=$pe+24
 $sections=@();$table=$opt+[BitConverter]::ToUInt16($b,$pe+20)
 for($i=0;$i -lt [BitConverter]::ToUInt16($b,$pe+6);$i++){$s=$table+40*$i;$sections+=,@([BitConverter]::ToUInt32($b,$s+12),[BitConverter]::ToUInt32($b,$s+16),[BitConverter]::ToUInt32($b,$s+20))}
 return @{Bytes=$b;Base=[BitConverter]::ToUInt32($b,$opt+28);Sections=$sections;ImportRVA=[BitConverter]::ToUInt32($b,$opt+104);Imports=@{}}
}
function Bytes($P,[long]$VA,[int]$Length){
 $rva=$VA-$P.Base
 foreach($s in $P.Sections){if($rva -ge $s[0] -and ($rva+$Length) -le ($s[0]+$s[1])){$o=$s[2]+$rva-$s[0];return ,([byte[]]$P.Bytes[$o..($o+$Length-1)])}}
 throw ('Unmapped VA {0:X}' -f $VA)
}
function U32($P,[long]$VA){return [BitConverter]::ToUInt32((Bytes $P $VA 4),0)}
function CString($P,[long]$VA){$chars=@();for($i=0;$i -lt 512;$i++){$c=(Bytes $P ($VA+$i) 1)[0];if(!$c){return [Text.Encoding]::ASCII.GetString([byte[]]$chars)};$chars+=$c};throw 'Unterminated name'}
function Load-Imports($P){
 for($d=$P.Base+$P.ImportRVA;;$d+=20){$oft=U32 $P $d;$name=U32 $P ($d+12);$ft=U32 $P ($d+16);if(!$name){break};$module=(CString $P ($P.Base+$name)).ToLowerInvariant();if(!$oft){$oft=$ft}
  for($i=0;;$i++){$t=U32 $P ($P.Base+$oft+4*$i);if(!$t){break};if($t -band 0x80000000L){$symbol='#'+($t -band 65535)}else{$symbol=CString $P ($P.Base+$t+2)};$P.Imports[[long]($P.Base+$ft+4*$i)]=$module+'!'+$symbol}
 }
}
$a=Open-PE "$PSScriptRoot\..\original\fsx.exe";$b=Open-PE (Join-Path $BuildDirectory 'fsx.exe');Load-Imports $a;Load-Imports $b
$map=Get-Content (Join-Path $BuildDirectory 'fsx.map') -Raw
function Symbol([string]$Name){$m=[regex]::Match($map,'(?m)^\s+\w{4}:\w{8}\s+'+[regex]::Escape($Name)+'\s+([0-9a-fA-F]{8})\b');if(!$m.Success){throw "Missing symbol $Name"};return [Convert]::ToInt64($m.Groups[1].Value,16)}
function Target($P,[long]$VA,[int]$Offset){return $VA+$Offset+4+[BitConverter]::ToInt32((Bytes $P ($VA+$Offset) 4),0)}
function Import-At($P,[long]$IAT){$v=$P.Imports[$IAT];if(!$v){throw ('No IAT entry {0:X}' -f $IAT)};return $v}
function Thunk($P,[long]$VA){$v=Bytes $P $VA 6;if($v[0] -ne 255 -or $v[1] -ne 37){throw ('Not an import thunk {0:X}' -f $VA)};return Import-At $P ([BitConverter]::ToUInt32($v,2))}
function Require($Condition,[string]$Message){if(!$Condition){throw $Message}}
function Equal-Bytes($X,$Y){return [Convert]::ToBase64String($X) -ceq [Convert]::ToBase64String($Y)}
function Compare-Body([long]$Original,[long]$Pilot,[int]$Length,$Offsets){$x=Bytes $a $Original $Length;$y=Bytes $b $Pilot $Length;foreach($o in $Offsets){[Array]::Copy($x,$o,$y,$o,4)};Require (Equal-Bytes $x $y) ('Body mismatch at '+('{0:X}' -f $Original))}
function Compare-ImportedCall([long]$Original,[long]$Pilot,[int]$Offset){Require ((Thunk $a (Target $a $Original $Offset)) -ceq (Thunk $b (Target $b $Pilot $Offset))) 'Imported call target mismatch'}

$init=Symbol '?InitializeOleAndMessageFilterPilot@@YAHXZ'
$vt=Symbol '??_7FSXMessageFilterPilot@@6B@'
$pending=Symbol '?OnMessagePending@FSXMessageFilterPilot@@UAEHPBUtagMSG@@@Z'
$destructor=U32 $b ($vt+4)
Compare-Body 0x010018d0 $pending 8 @()
Compare-ImportedCall 0x010018a0 $destructor 9
Compare-ImportedCall 0x010018a0 $destructor 21
Compare-Body 0x010018a0 $destructor 35 @(9,21)
$slots=@()
for($i=0;$i -lt 28;$i++){
 $x=U32 $a (0x01001458+4*$i);$y=U32 $b ($vt+4*$i)
 if($i -eq 1){Require ($x -eq 0x010018a0 -and $y -eq $destructor) 'Destructor slot mismatch';$kind='verified deleting destructor'}
 elseif($i -eq 24){Require ($x -eq 0x010018d0 -and $y -eq $pending) 'Pending slot mismatch';$kind='raw exact TRUE override'}
 elseif($i -in 2,4){Require (Equal-Bytes (Bytes $a $x 3) (Bytes $b $y 3)) 'RET4 leaf mismatch';$kind='raw exact RET4 leaf'}
 elseif($i -eq 3){Require (Equal-Bytes (Bytes $a $x 1) (Bytes $b $y 1)) 'RET leaf mismatch';$kind='raw exact RET leaf'}
 else{$kind=Thunk $a $x;Require ($kind -ceq (Thunk $b $y)) "Vtable import mismatch slot $i"}
 $slots+=[pscustomobject]@{Offset=('{0:X2}' -f (4*$i));Identity=$kind}
}
$handler=Symbol '__ehhandler$?InitializeOleAndMessageFilterPilot@@YAHXZ'
Require ((U32 $a 0x010018e8) -eq 0x01002ca8 -and (U32 $b ($init+8)) -eq $handler) 'Handler reference mismatch'
Require ((U32 $a 0x010018f8) -eq 0x01004004 -and (U32 $b ($init+0x18)) -eq (Symbol '___security_cookie')) 'Cookie mismatch'
foreach($o in @(0x2d,0x46)){Require ((Import-At $a (U32 $a (0x010018e0+$o))) -ceq (Import-At $b (U32 $b ($init+$o)))) 'COM IAT mismatch'}
foreach($o in @(0x53,0x5c,0x7c,0x92,0xb1,0xbb,0xc3)){Compare-ImportedCall 0x010018e0 $init $o}
Require ((U32 $a 0x01001978) -eq 0x01001458 -and (U32 $b ($init+0x98)) -eq $vt) 'Vtable reference mismatch'
Compare-Body 0x010018e0 $init 237 @(8,0x18,0x2d,0x46,0x53,0x5c,0x7c,0x92,0x98,0xb1,0xbb,0xc3)

Require ((Target $a 0x01002ca8 13) -eq 0x0100263d -and (Target $b $handler 13) -eq (Symbol '@__security_check_cookie@4')) 'Handler cookie target mismatch'
Require ((Thunk $a (Target $a 0x01002ca8 23)) -ceq (Thunk $b (Target $b $handler 23))) 'CxxFrameHandler import mismatch'
$infoA=U32 $a (0x01002ca8+18);$infoB=U32 $b ($handler+18)
$mapA=U32 $a ($infoA+8);$mapB=U32 $b ($infoB+8)
$x=Bytes $a $infoA 36;$y=Bytes $b $infoB 36;[Array]::Copy($x,8,$y,8,4);Require (Equal-Bytes $x $y) 'FuncInfo content mismatch'
Require ((U32 $a $mapA) -eq 0xffffffffL -and (U32 $b $mapB) -eq 0xffffffffL) 'Unwind state mismatch'
$actionA=U32 $a ($mapA+4);$actionB=U32 $b ($mapB+4)
$deleteA=Target $a $actionA 4;$deleteB=Target $b $actionB 4
Compare-Body $actionA $actionB 8 @(4)
Compare-ImportedCall $deleteA $deleteB 4
Compare-Body $deleteA $deleteB 10 @(4)
Compare-Body 0x01002ca8 $handler 27 @(13,18,23)
[pscustomobject]@{
 OriginalSHA256=(Get-FileHash "$PSScriptRoot\..\original\fsx.exe" -Algorithm SHA256).Hash
 PilotSHA256=(Get-FileHash (Join-Path $BuildDirectory 'fsx.exe') -Algorithm SHA256).Hash
 InitializerLength=237;InitializerAddressNormalizedExactMatch=$true
 PendingOverrideLength=8;PendingOverrideRawExactMatch=$true
 DeletingDestructorLength=35;DeletingDestructorAddressNormalizedExactMatch=$true
 VTableSlotsVerified=$slots;ExceptionMetadataAndCleanupVerified=$true
 Scope='Referenced import DLL and ordinal/name identities, 28 vtable slots, deleting destructor, FuncInfo/unwind/cleanup and handler checked. RTTI and external DLL implementation bytes are outside scope; reconstructed class names are not original names.'
}|ConvertTo-Json -Depth 6
