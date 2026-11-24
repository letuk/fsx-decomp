param([Parameter(Mandatory)][string]$BuildDirectory,[switch]$AllowEntryInclude,
 [string]$ControlDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\39-extended-function-order")
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$a=Open-PE "$PSScriptRoot\..\original\fsx.exe";$b=Open-PE "$BuildDirectory\fsx.exe";Load-Imports $a;Load-Imports $b
$order=(& "$PSScriptRoot\verify-function-order.ps1" -BuildDirectory $BuildDirectory -OrderFile "$BuildDirectory\source\function-order-input.h"|Out-String)|ConvertFrom-Json
if($order.Count -ne 88){throw 'Expected reviewed 22 application/ATL symbols plus 66 MFC thunks'}
$manifest=@(Get-Content "$PSScriptRoot\matching\mfc-thunk-order.json" -Raw|ConvertFrom-Json)
$before=@(Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json);$after=@(Get-Content "$BuildDirectory\source-hashes.json" -Raw|ConvertFrom-Json)
if((@($before.File|Sort-Object)|ConvertTo-Json -Compress) -cne (@($after.File|Sort-Object)|ConvertTo-Json -Compress)){throw 'Source input list changed'}
foreach($f in $before){if($f.File -cne 'function-order-input.h' -and $f.Hash -cne ($after|Where-Object File -CEQ $f.File).Hash){throw "Source changed: $($f.File)"}}
$controlCommand=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"))
$buildCommand=[IO.File]::ReadAllText((Resolve-Path "$BuildDirectory\build.cmd"))
$checkedCommand=$buildCommand
if($AllowEntryInclude){if(([regex]::Matches($buildCommand,'/INCLUDE:_WinMainCRTStartup ')).Count -ne 1){throw 'Expected exactly one entry INCLUDE'};$checkedCommand=$buildCommand.Replace('/INCLUDE:_WinMainCRTStartup ','')}
if($controlCommand -cne $checkedCommand){throw 'Build command changed beyond permitted entry INCLUDE'}
$prefix=@(Get-Content "$ControlDirectory\source\function-order-input.h")
$response=@(Get-Content "$BuildDirectory\source\function-order-input.h")
if(($prefix|ConvertTo-Json -Compress) -cne (@($response|Select-Object -First 22)|ConvertTo-Json -Compress)){throw 'Application order prefix changed'}
$control=(& "$PSScriptRoot\compare-application-order.ps1" -BuildDirectory $ControlDirectory|Out-String)|ConvertFrom-Json
for($i=0;$i -lt 22;$i++){if($order.Functions[$i].RebuiltVA -cne $control.Functions[$i].RebuiltVA){throw 'Application prefix placement changed'}}
$rows=for($i=0;$i -lt $manifest.Count;$i++){
 $m=$manifest[$i];$r=$order.Functions[22+$i]
 if($r.Symbol -cne $m.Symbol){throw 'Manifest/order symbol mismatch'}
 $av=[Convert]::ToInt64($m.OriginalVA,16);$bv=[Convert]::ToInt64($r.RebuiltVA,16)
 foreach($pair in @(@($a,$av),@($b,$bv))){
  $bytes=Bytes $pair[0] $pair[1] 6
  if($bytes[0] -ne 255 -or $bytes[1] -ne 37){throw 'Expected exact FF25 import thunk'}
  $iat=[long][BitConverter]::ToUInt32($bytes,2)
  if($pair[0].Imports[$iat] -cne $m.Identity){throw 'Thunk IAT identity mismatch'}
 }
 [pscustomobject]@{OriginalVA=$m.OriginalVA;RebuiltVA=$r.RebuiltVA;Delta=$bv-$av;Symbol=$m.Symbol;Identity=$m.Identity;SixByteThunkValidated=$true}
}
$deltas=@($rows.Delta|Select-Object -Unique)
$c=Get-Content "$BuildDirectory\comparison.json" -Raw|ConvertFrom-Json
$slots=@{};foreach($e in $c.PeImports.OriginalImportEntries){$slots[$e.Identity]=$e.Index}
[pscustomobject]@{RebuiltSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;ControlSHA256=(Get-FileHash "$ControlDirectory\fsx.exe").Hash;OrderedSymbols=$order.Count;ValidatedMfcThunks=$rows.Count;UniformAddressDelta=($deltas.Count -eq 1);Delta=$(if($deltas.Count -eq 1){$deltas[0]}else{$null});ApplicationPrefixPlacementUnchanged=$true;CppAndHeadersUnchanged=$true;BuildCommandsIdentical=($controlCommand -ceq $buildCommand);EntryIncludePermitted=[bool]$AllowEntryInclude;ImportSlotDifferences=@($c.PeImports.ImportEntries|Where-Object {$slots[$_.Identity] -ne $_.Index}).Count;Functions=@($rows);Scope='Original fsx.exe FF25 bodies and exact IAT identities versus actual ordered MAP symbols. Response input and optionally the separately controlled entry INCLUDE change from baseline39. No padding-byte, whole CRT order, import-table order, original linker command or runtime equality claim.'}|ConvertTo-Json -Depth 6
