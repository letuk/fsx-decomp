param([Parameter(Mandatory)][string]$BuildDirectory,[Parameter(Mandatory)][string]$OrderFile)
$ErrorActionPreference='Stop'
$names=@(Get-Content -LiteralPath $OrderFile|Where-Object {$_.Trim().Length -gt 0})
if(!$names.Count){throw 'Empty function order input'}
$map=Get-Content "$BuildDirectory\fsx.map" -Raw;$log=Get-Content "$BuildDirectory\build-log.txt" -Raw
if($log -match 'warning LNK(4037|4044)'){throw 'Linker ignored or could not resolve an ordering input'}
$rows=@();$last=0L;$previous=''
foreach($name in $names){$symbol=if($name.StartsWith('?') -or $name.StartsWith('@')){$name}else{'_'+$name};$m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($symbol)+'\s+([0-9a-fA-F]{8})\b');if(!$m.Success){throw "Missing ordered symbol $symbol"};$va=[Convert]::ToInt64($m.Groups[1].Value,16);$knownAlias=($previous -ceq '?Serialize@CObject@@UAEXAAVCArchive@@@Z' -and $symbol -ceq '?Dump@CObject@@UBEXAAVCDumpContext@@@Z');if($va -lt $last -or ($va -eq $last -and !$knownAlias)){throw "Actual placement violates order at $symbol"};$rows+=[pscustomobject]@{Symbol=$symbol;RebuiltVA=('{0:X8}' -f $va)};$last=$va;$previous=$symbol}
[pscustomobject]@{RequestedOrderVerified=$true;ResponseFileSHA256=(Get-FileHash -LiteralPath $OrderFile).Hash;Count=$rows.Count;Functions=$rows;Scope='Actual ordered MAP placements and linker acceptance for the stated response entries only. No absolute original address, unlisted/static function order, body, import order or original linker command claim.'}|ConvertTo-Json -Depth 5
