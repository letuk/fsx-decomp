param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$original="$PSScriptRoot\..\original\fsx.exe";$pilot=Join-Path $BuildDirectory 'fsx.exe'
$a=Open-PE $original;$b=Open-PE $pilot;Load-Imports $a;Load-Imports $b
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
function Require($condition,[string]$message){if(!$condition){throw $message}}
function Symbol([string]$name){$m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($name)+'\s+([0-9a-fA-F]{8})\b');Require $m.Success "Missing symbol $name";return [Convert]::ToInt64($m.Groups[1].Value,16)}
function Thunk($pe,[long]$va){$x=Bytes $pe $va 6;Require ($x[0] -eq 255 -and $x[1] -eq 37) 'Expected import thunk';$name=$pe.Imports[[long][BitConverter]::ToUInt32($x,2)];Require ($null -ne $name) 'Missing IAT identity';return $name}
function Relative($pe,[long]$entry){return $entry+5+[BitConverter]::ToInt32((Bytes $pe ($entry+1) 4),0)}
function Equal($x,$y){return [Convert]::ToBase64String($x) -ceq [Convert]::ToBase64String($y)}
$vt=Symbol '??_7FSXAppPilot@@6B@'
$getMap=Symbol '?GetMessageMap@FSXAppPilot@@MBEPBUAFX_MSGMAP@@XZ'
Require ((U32 $a 0x010013c8) -eq 0x01001650 -and (U32 $b ($vt+0x30)) -eq $getMap) 'GetMessageMap vtable mapping differs'
$x=Bytes $a 0x01001650 6;$y=Bytes $b $getMap 6
Require ($x[0] -eq 0xb8 -and $y[0] -eq 0xb8 -and $x[5] -eq 0xc3 -and $y[5] -eq 0xc3) 'GetMessageMap straight-line body differs'
$mapA=U32 $a 0x01001651;$mapB=U32 $b ($getMap+1)
Require ($mapA -eq 0x010012f0) 'Original message-map address changed'
$baseA=U32 $a $mapA;$baseB=U32 $b $mapB
$baseIdentity=Thunk $a $baseA
Require ($baseIdentity -ceq 'mfc80.dll!#3683' -and $baseIdentity -ceq (Thunk $b $baseB)) 'Base message-map import identity differs'
$entriesA=U32 $a ($mapA+4);$entriesB=U32 $b ($mapB+4)
Require ($entriesA -eq 0x010012d8) 'Original message-entry address changed'
$zero=[byte[]]::new(24)
Require ((Equal (Bytes $a $entriesA 24) $zero) -and (Equal (Bytes $b $entriesB 24) $zero)) 'Message map is not the expected empty terminator'
[Array]::Copy($x,1,$y,1,4);Require (Equal $x $y) 'GetMessageMap body differs'

$run=Symbol '?Run@FSXAppPilot@@UAEHXZ'
$callback=Symbol '?GetCurrentMessagePilot@@YGPAUtagMSG@@XZ'
Require ((U32 $a 0x010013f4) -eq 0x01001d00 -and (U32 $b ($vt+0x5c)) -eq $run) 'Run vtable mapping differs'
Require ((U32 $a 0x01001408) -eq 0x01001720 -and (U32 $b ($vt+0x70)) -eq (Symbol '?ExitInstance@FSXAppPilot@@UAEHXZ')) 'Virtual exit association differs'
$x=Bytes $a 0x01001d00 35;$y=Bytes $b $run 35
Require ([BitConverter]::ToUInt32($x,7) -eq 0x01004384 -and [BitConverter]::ToUInt32($y,7) -eq (Symbol '?pfnGetApplication@@3P6APAVApplicationPilot@@XZA')) 'Run application-getter target differs'
Require ([BitConverter]::ToUInt32($x,0x13) -eq 0x01001cf0 -and [BitConverter]::ToUInt32($y,0x13) -eq $callback) 'Run callback target differs'
# The remaining bytes validate both indirect vtable calls, slots+8/+70,
# ECX setup and the final JMP EAX. No branch bytes are masked.
foreach($o in 7,0x13){[Array]::Copy($x,$o,$y,$o,4)}
Require (Equal $x $y) 'Run body differs beyond validated references'
$x=Bytes $a 0x01001cf0 5;$y=Bytes $b $callback 5
Require ($x[0] -eq 0xe9 -and $y[0] -eq 0xe9) 'Expected single tail-jump callback'
$targetA=Relative $a 0x01001cf0;$targetB=Relative $b $callback
Require ($targetA -eq 0x010020c8 -and $targetB -eq (Symbol '?AfxGetCurrentMessage@@YGPAUtagMSG@@XZ')) 'Callback thunk target differs'
$callbackIdentity=Thunk $a $targetA
Require ($callbackIdentity -ceq 'mfc80.dll!#1074' -and $callbackIdentity -ceq (Thunk $b $targetB)) 'Current-message import identity differs'
[Array]::Copy($x,1,$y,1,4);Require (Equal $x $y) 'Callback body differs'
[pscustomobject]@{
 OriginalSHA256=(Get-FileHash $original).Hash;RebuiltSHA256=(Get-FileHash $pilot).Hash
 GetMessageMap=[pscustomobject]@{OriginalVA='01001650';RebuiltVA=('{0:X8}' -f $getMap);Length=6;VerifiedAddressFields=1;AddressNormalizedExactMatch=$true}
 Run=[pscustomobject]@{OriginalVA='01001d00';RebuiltVA=('{0:X8}' -f $run);Length=35;VerifiedAddressFields=2;AddressNormalizedExactMatch=$true}
 Callback=[pscustomobject]@{OriginalVA='01001cf0';RebuiltVA=('{0:X8}' -f $callback);Length=5;VerifiedAddressFields=1;AddressNormalizedExactMatch=$true}
 MessageMapData=[pscustomobject]@{RecordSize=8;TerminatorSize=24;BaseImport=$baseIdentity;CallbackImport=$callbackIdentity;Verified=$true}
 Scope='Complete getter/Run/callback bodies with four validated address fields. Straight-line bodies and terminal RET/JMP instructions checked; virtual slots and empty AFX_MSGMAP record/terminator checked. Reconstructed source names, no original-name, full-vtable, RTTI/EH/CRT, external-body or whole-file equality claim.'
}|ConvertTo-Json -Depth 5
