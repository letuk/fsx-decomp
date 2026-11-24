param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$original="$PSScriptRoot\..\original\fsx.exe"
if((Get-FileHash $original).Hash -ne '61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735'){throw 'Original identity differs'}
$a=Open-PE $original;$b=Open-PE "$BuildDirectory\fsx.exe";Load-Imports $a;Load-Imports $b
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
function Require($condition,[string]$message){if(!$condition){throw $message}}
function Symbol([string]$name){$m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($name)+'\s+([0-9a-fA-F]{8})\b');Require $m.Success "Missing symbol $name";return [Convert]::ToInt64($m.Groups[1].Value,16)}
function Rel($pe,[long]$entry,[int]$offset){return $entry+$offset+4+[BitConverter]::ToInt32((Bytes $pe ($entry+$offset) 4),0)}
function Iat($pe,[long]$entry){$identity=$pe.Imports[$entry];Require ($null -ne $identity) 'Missing import identity';return $identity}
function Thunk($pe,[long]$entry){$x=Bytes $pe $entry 6;Require ($x[0] -eq 255 -and $x[1] -eq 37) 'Expected FF25 thunk';return Iat $pe ([long][BitConverter]::ToUInt32($x,2))}
function Equal-Bytes($x,$y,[string]$name){Require ([Convert]::ToBase64String($x) -ceq [Convert]::ToBase64String($y)) "$name non-reference bytes differ"}
function Require-BodyBoundary($pe,[long]$start,[int]$length,[long]$next,[string]$name){
 $gap=$next-$start-$length
 Require ($gap -in 0,5) "$name unexpected body-to-next-entry gap"
 if($gap){Require (@(Bytes $pe ($start+$length) $gap|Where-Object {$_ -ne 0xcc}).Count -eq 0) "$name gap is not CC padding"}
 return $gap
}
function Initial-Dword($pe,[long]$va){
 $header=[BitConverter]::ToInt32($pe.Bytes,60);$table=$header+24+[BitConverter]::ToUInt16($pe.Bytes,$header+20);$rva=$va-$pe.Base
 for($i=0;$i -lt $pe.Sections.Count;$i++){
  $s=$table+40*$i;$start=[BitConverter]::ToUInt32($pe.Bytes,$s+12);$virtual=[BitConverter]::ToUInt32($pe.Bytes,$s+8);$raw=[BitConverter]::ToUInt32($pe.Bytes,$s+16)
  if($rva -ge $start -and $rva+4 -le $start+[Math]::Max($virtual,$raw)){
   Require (([BitConverter]::ToUInt32($pe.Bytes,$s+36) -band 0x80000000L) -ne 0) 'Expected writable CRT state'
   if($rva+4 -le $start+$raw){return U32 $pe $va}
   Require ($rva -ge $start+$raw) 'Partly file-backed state unsupported';return 0L
  }
 }
 throw 'CRT state outside mapped image'
}
$entry=Symbol '_pre_c_init';$crt=Symbol '___tmainCRTStartup'
function Require-GuiCrtMember([string]$symbol){
 $entryPattern='(?im)^\s*\S+\s+'+[regex]::Escape($symbol)+'\s+\S+\s+f\s+'
 if($map -match ($entryPattern+'(?:msvcrt|selected-crt):crtexew\.obj\s*$')){return}
 $member=Join-Path $BuildDirectory 'writable-crtexew.obj'
 Require (($map -match ($entryPattern+'writable-crtexew\.obj\s*$')) -and
  (Test-Path -LiteralPath $member) -and
  (Get-FileHash -LiteralPath $member).Hash -ceq '11C52C814C36E11E9CA55DD1955D70DE2FCC516DDE1181F35EFC4F3710A66309') 'Expected GUI CRT member provenance'
}
Require-GuiCrtMember '_pre_c_init'
Require ((U32 $a 0x01001254) -eq 0x01002547 -and (U32 $b ((Symbol '___xi_a')+4)) -eq $entry) 'C initializer registration differs'
$preCGap=Require-BodyBoundary $b $entry 226 (Symbol '_WinMainCRTStartup') 'pre_c_init'
$math=Symbol '__matherr';$argv=Symbol '__setargv';$fp=Symbol '__setdefaultprecision';$rtc=Symbol '__RTC_Initialize'
Equal-Bytes (Bytes $a 0x010029cc 3) (Bytes $b $math 3) 'CRT math-error fallback'
Equal-Bytes (Bytes $a 0x010029cc 3) (Bytes $b $argv 3) 'CRT argument-setup fallback'
$x=Bytes $a 0x0100299e 41;$y=Bytes $b $fp 41
foreach($pair in @(@(15,0x01002bec,'msvcr80.dll!_controlfp_s'),@(32,0x01002be0,'msvcr80.dll!_invoke_watson'))){
 $offset=$pair[0];Require ((Rel $a 0x0100299e $offset) -eq $pair[1]) 'Original FP dependency changed'
 Require ((Thunk $a $pair[1]) -ceq $pair[2] -and (Thunk $b (Rel $b $fp $offset)) -ceq $pair[2]) 'FP dependency import differs'
 [Array]::Copy($x,$offset,$y,$offset,4)
}
Equal-Bytes $x $y 'CRT precision helper'
$x=Bytes $a 0x010027d9 36;$y=Bytes $b $rtc 36
$rtcStart=Symbol '___rtc_iaa';$rtcEnd=Symbol '___rtc_izz'
Require ($rtcEnd-$rtcStart -eq 4 -and (U32 $b $rtcStart) -eq 0 -and (U32 $b $rtcEnd) -eq 0) 'Rebuilt RTC initializer range is not empty'
Require ((U32 $a 0x01001568) -eq 0 -and (U32 $a 0x0100156c) -eq 0) 'Original RTC sentinels differ'
foreach($offset in @(3,8)){Require ([BitConverter]::ToUInt32($x,$offset) -eq 0x0100156c -and [BitConverter]::ToUInt32($y,$offset) -eq $rtcEnd) 'RTC range target differs';[Array]::Copy($x,$offset,$y,$offset,4)}
Equal-Bytes $x $y 'CRT RTC initializer walker'
$x=Bytes $a 0x01002547 226;$y=Bytes $b $entry 226
$oldManaged=[long][BitConverter]::ToUInt32($x,0x63);$newManaged=[long][BitConverter]::ToUInt32($y,0x63)
Require ($oldManaged -eq 0x01004030) 'Original managed-state address changed'
# This private CRT flag lacks a public MAP symbol. Verify its other use in
# tmainCRTStartup, plus writable state and identical initial value below.
function Verify-ManagedUse($pe,[long]$start,[int]$length,[long]$state){
 $code=Bytes $pe $start $length;$count=0
 for($i=0;$i+7 -le $code.Length;$i++){if($code[$i] -eq 0x83 -and $code[$i+1] -eq 0x3d -and [BitConverter]::ToUInt32($code,$i+2) -eq $state -and $code[$i+6] -eq 0){$count++}}
 Require ($count -eq 2) 'Managed-state CRT startup consumers differ'
}
Verify-ManagedUse $a 0x01002363 484 $oldManaged
Verify-ManagedUse $b $crt ($entry-$crt) $newManaged
$references=[Collections.Generic.List[object]]::new()
function Normalize([int]$offset,[long]$old,[long]$new,[string]$kind,[string]$role){
 if($kind -eq 'call'){$ox=Rel $a 0x01002547 $offset;$ny=Rel $b $entry $offset}
 else{$ox=[long][BitConverter]::ToUInt32($x,$offset);$ny=[long][BitConverter]::ToUInt32($y,$offset)}
 Require ($ox -eq $old -and $ny -eq $new) "Reference target differs: $role"
 if($kind -eq 'state'){Require ((Initial-Dword $a $old) -eq (Initial-Dword $b $new)) "Initial CRT state differs: $role"}
 $references.Add([pscustomobject]@{Offset=$offset;Kind=$kind;Role=$role;OriginalTarget=('{0:X8}' -f $old);RebuiltTarget=('{0:X8}' -f $new)})
 [Array]::Copy($x,$offset,$y,$offset,4)
}
foreach($f in @(@(3,0),@(0x10,0x3c),@(0x16,0),@(0x23,0x18),@(0x39,0x84),@(0x44,0xf8),@(0x4c,0x74),@(0x57,0xe8))){Normalize $f[0] ($a.Base+$f[1]) ($b.Base+$f[1]) 'image' 'image-relative PE header location'}
Normalize 0x63 $oldManaged $newManaged 'state' 'managed-image flag with CRT startup consumer'
foreach($f in @(@(0x69,0x010011bc),@(0x71,0x010011c0),@(0x83,0x010011c4),@(0x91,0x010011c8),@(0x9e,0x010011cc),@(0xc3,0x010011d0),@(0xda,0x010011d4))){
 $offset=$f[0];$ny=[long][BitConverter]::ToUInt32($y,$offset);$identity=Iat $a $f[1];Require ($identity -ceq (Iat $b $ny)) 'CRT import identity differs';Normalize $offset $f[1] $ny 'iat' $identity
}
foreach($f in @(@(0x78,0x01004494,'___onexitend'),@(0x7d,0x01004498,'___onexitbegin'),@(0x89,0x01004050,'__fmode'),@(0x97,0x0100404c,'__commode'),@(0xa5,0x01004488,'__adjust_fdiv'),@(0xb5,0x0100401c,'___defaultmatherr'),@(0xcf,0x01004018,'___globallocalestatus'))){Normalize $f[0] $f[1] (Symbol $f[2]) 'state' $f[2]}
Normalize 0xaa 0x010027d9 $rtc 'call' 'verified RTC initializer walker'
Normalize 0xaf 0x010029cc $argv 'call' 'verified argument-setup fallback; original shares the zero-return matherr body'
Normalize 0xbd 0x010029cc $math 'pointer' 'same verified math-error fallback callback'
Normalize 0xc9 0x0100299e $fp 'call' 'verified precision helper'
Equal-Bytes $x $y 'pre_c_init'
[pscustomobject]@{OriginalSHA256=(Get-FileHash $original).Hash;RebuiltSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;Name='CRT pre_c_init';OriginalVA='01002547';RebuiltVA=('{0:X8}' -f $entry);Length=226;AddressNormalizedExactMatch=$true;VerifiedAddressFields=$references.Count;References=@($references);Dependencies=@(@{Name='matherr fallback';OriginalVA='010029CC';Bytes=3;RawExactMatch=$true},@{Name='setdefaultprecision';OriginalVA='0100299E';Bytes=41;AddressNormalizedExactMatch=$true},@{Name='RTC Initialize walker';OriginalVA='010027D9';Bytes=36;AddressNormalizedExactMatch=$true;EmptyCallbackRangeVerified=$true});FollowingGapBytes=$preCGap;Scope='Complete 226-byte CRT pre_c_init body after target-validated normalization. PE-header offsets, imports, symbol-resolved state with initial values, private managed flag consumer, registered initializer slot and three dependency bodies verified. No whole CRT/EXE, external implementation, original compiler identity or runtime execution claim. Separate library identification; existing 18 reconstructed/helper/wrapper body count is unchanged.'}|ConvertTo-Json -Depth 7
