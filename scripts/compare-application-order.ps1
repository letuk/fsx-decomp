param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
$order=(& "$PSScriptRoot\verify-function-order.ps1" -BuildDirectory $BuildDirectory -OrderFile "$BuildDirectory\source\function-order-input.h"|Out-String)|ConvertFrom-Json
$original=@(0x01001590L,0x01001600L,0x01001650L,0x01001660L,0x010016d0L,0x01001720L,0x01001760L,0x01001800L,0x010018a0L,0x010018d0L,0x010018e0L,0x010019e0L)
if($order.Count -in 22,88){$original+=@(0x01001cf0L,0x01001d00L,0x01001d30L,0x01001ef0L,0x01001ef0L,0x01001f00L,0x01001f10L,0x01001f1fL,0x01001f89L,0x01001f92L)}
if($order.Count -eq 88){$original+=@(Get-Content "$PSScriptRoot\matching\mfc-thunk-order.json" -Raw|ConvertFrom-Json|ForEach-Object {[Convert]::ToInt64($_.OriginalVA,16)})}
if($order.Count -notin 3,12,22,88){throw 'Unreviewed function-order scope'}
$rows=for($i=0;$i -lt $order.Count;$i++){$r=$order.Functions[$i];$va=[Convert]::ToInt64($r.RebuiltVA,16);[pscustomobject]@{Symbol=$r.Symbol;OriginalVA=('{0:X8}' -f $original[$i]);RebuiltVA=$r.RebuiltVA;Delta=$va-$original[$i];OriginalAddressMatches=($va -eq $original[$i])}}
$deltas=@($rows.Delta|Select-Object -Unique)
[pscustomobject]@{RebuiltSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;Count=$order.Count;RequestedOrderVerified=$true;Functions=@($rows);UniformAddressDelta=($deltas.Count -eq 1);Delta=$(if($deltas.Count -eq 1){$deltas[0]}else{$null});AllOriginalAddressesMatch=(@($rows|Where-Object {!$_.OriginalAddressMatches}).Count -eq 0);Scope='Known original entry identities/order from existing body/slot evidence versus requested decorated MAP symbols. Measures selected entry positions, not whole function bytes, unlisted callback/SDK order, imports, unique original link flags or whole EXE equality.'}|ConvertTo-Json -Depth 6
