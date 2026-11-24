param([Parameter(Mandatory)][string]$BuildDirectory,
      [Parameter(Mandatory)][ValidateSet('ConsolePilot','MfcWindows')][string]$BuildProfile,
      [uint32]$ExpectedImageBase=0,
      [ValidateSet('Any','Default','Original')][string]$ExpectedStackMode='Any')
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$a=Open-PE "$PSScriptRoot\..\original\fsx.exe";$b=Open-PE "$BuildDirectory\fsx.exe";Load-Imports $a;Load-Imports $b
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
function Require($condition,[string]$message){if(!$condition){throw $message}}
function Symbol([string]$name){$m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($name)+'\s+([0-9a-fA-F]{8})\b');Require $m.Success "Missing startup symbol $name";return [Convert]::ToInt64($m.Groups[1].Value,16)}
function Header($pe){$opt=[BitConverter]::ToInt32($pe.Bytes,60)+24;return @{Subsystem=[BitConverter]::ToUInt16($pe.Bytes,$opt+68);Entry=$pe.Base+[BitConverter]::ToUInt32($pe.Bytes,$opt+16)}}
$old=Header $a;$new=Header $b;$winMain=$null;$mfcIdentity=$null
if($ExpectedImageBase){Require ($b.Base -eq $ExpectedImageBase) 'PE image base differs from requested build mode'}
$oldOpt=[BitConverter]::ToInt32($a.Bytes,60)+24;$newOpt=[BitConverter]::ToInt32($b.Bytes,60)+24
$oldReserve=[BitConverter]::ToUInt32($a.Bytes,$oldOpt+72);$oldCommit=[BitConverter]::ToUInt32($a.Bytes,$oldOpt+76)
$newReserve=[BitConverter]::ToUInt32($b.Bytes,$newOpt+72);$newCommit=[BitConverter]::ToUInt32($b.Bytes,$newOpt+76)
if($ExpectedStackMode -eq 'Original'){Require ($oldReserve -eq 0x40000 -and $oldCommit -eq 0x2000 -and $newReserve -eq $oldReserve -and $newCommit -eq $oldCommit) 'PE stack values differ from original mode'}
if($ExpectedStackMode -eq 'Default'){Require ($newReserve -eq 0x100000 -and $newCommit -eq 0x1000) 'PE stack values differ from default mode'}
if($BuildProfile -eq 'MfcWindows'){
 Require ($old.Subsystem -eq 2 -and $new.Subsystem -eq 2) 'Expected Windows GUI subsystem'
 Require ($new.Entry -eq (Symbol '_WinMainCRTStartup')) 'Unexpected Windows CRT entry point'
 $winMain=Symbol '_WinMain@16';$mfc=Symbol '?AfxWinMain@@YGHPAUHINSTANCE__@@0PADH@Z'
 $body=Bytes $b $winMain 5;Require ($body[0] -eq 0xe9) 'Expected MFC WinMain tail jump'
 Require ($winMain+5+[BitConverter]::ToInt32($body,1) -eq $mfc) 'WinMain does not target AfxWinMain thunk'
 $thunk=Bytes $b $mfc 6;Require ($thunk[0] -eq 255 -and $thunk[1] -eq 37) 'Expected AfxWinMain import thunk'
 $mfcIdentity=$b.Imports[[long][BitConverter]::ToUInt32($thunk,2)]
 Require ($mfcIdentity -ceq 'mfc80.dll!#1207' -and $mfcIdentity -cin @($a.Imports.Values)) 'AfxWinMain import differs from original identity'
 $appmodul=Join-Path $BuildDirectory 'writable-appmodul.obj'
 $winMainPattern='(?im)^\s*\S+\s+_WinMain@16\s+\S+\s+f\s+'
 Require (($map -match ($winMainPattern+'mfcs80:appmodul\.obj\s*$')) -or
  (($map -match ($winMainPattern+'writable-appmodul\.obj\s*$')) -and
   (Test-Path -LiteralPath $appmodul) -and
   (Get-FileHash -LiteralPath $appmodul).Hash -ceq 'FFE820D4AEACDCD7DE4566C32F46AF0ECCA5045EB58E03DF03D188671B16FAC7')) 'WinMain is not supplied by the expected MFC startup object'
 Require ($map -notmatch '(?m)^\s*\S+\s+(_main|\?CopyErrorText_Pilot[^\s]*|\?RunFormatterPilot[^\s]*|\?g_pilot[^\s]*)\s+') 'Synthetic console/helper symbol still linked'
}else{
 Require ($new.Subsystem -eq 3) 'Expected console subsystem'
 Require ($new.Entry -eq (Symbol '_mainCRTStartup')) 'Unexpected console CRT entry point'
 $null=Symbol '_main'
}
[pscustomobject]@{BuildProfile=$BuildProfile;ImageBase=('{0:X8}' -f $b.Base);OriginalImageBase=('{0:X8}' -f $a.Base);ExpectedImageBase=('{0:X8}' -f $ExpectedImageBase);StackMode=$ExpectedStackMode;StackReserve=$newReserve;StackCommit=$newCommit;OriginalStackReserve=$oldReserve;OriginalStackCommit=$oldCommit;Subsystem=$new.Subsystem;OriginalSubsystem=$old.Subsystem;EntryVA=('{0:X8}' -f $new.Entry);OriginalEntryVA=('{0:X8}' -f $old.Entry);MfcWinMainVA=if($winMain){'{0:X8}' -f $winMain}else{$null};MfcImportIdentity=$mfcIdentity;Verified=$true;Scope='Static PE subsystem/entry and selected MAP/thunk checks. No CRT-body/registration equality, runtime execution or full FSX-replacement claim.'}|ConvertTo-Json -Depth 3
