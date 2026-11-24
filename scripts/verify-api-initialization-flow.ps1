param([string]$FlowPath='artifacts/api.dll/api-initialization-flow-20261103.json', [string]$MemoryPath='artifacts/api.dll/api-initialization-memory-20261103.json')
$ErrorActionPreference='Stop'
. "$PSScriptRoot/lib/pe-image.ps1"
$original=Join-Path $PSScriptRoot '../original/api.dll'
$hash=(Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash
if($hash -ne '9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003'){throw 'Wrong original API image'}
$p=Open-PE $original
$memory=Get-Content -LiteralPath $MemoryPath -Raw | ConvertFrom-Json
foreach($span in $memory.memory){
 $raw=[BitConverter]::ToString((Bytes $p ([Convert]::ToInt64($span.address,16)) $span.length)).Replace('-','')
 if($raw -cne $span.hex){throw ('Live/original snapshot mismatch at '+$span.address)}
}
$flow=Get-Content -LiteralPath $FlowPath -Raw | ConvertFrom-Json
$owned=@{}
foreach($row in $flow.instructions){
 $va=[Convert]::ToInt64($row.address,16)
 $raw=[BitConverter]::ToString((Bytes $p $va $row.bytes)).Replace('-','')
 if($raw -cne $row.hex){throw ('Original byte mismatch at '+$row.address)}
 for($j=0;$j -lt $row.bytes;$j++){if($owned.ContainsKey($va+$j)){throw 'Overlapping instructions'};$owned[$va+$j]='instruction'}
}
if($owned.Count -ne $flow.uniqueInstructionBytes -or $flow.instructions.Count -ne $flow.instructionCount){throw 'Count mismatch'}
foreach($g in $flow.gateways){
 $descriptor=[Convert]::ToInt64($g.descriptor,16)
 if((U32 $p $descriptor) -ne [Convert]::ToInt64($g.descriptorRva,16) -or (U32 $p ($descriptor+4)) -ne $g.descriptorIndex){throw 'Descriptor mismatch'}
 if([Convert]::ToInt64($g.resume,16) -ne $descriptor+8){throw 'Wrong gateway continuation'}
 for($j=0;$j -lt 8;$j++){if($owned.ContainsKey($descriptor+$j)){throw 'Descriptor decoded as instruction'};$owned[$descriptor+$j]='descriptor'}
}
$gaps=@();$start=$null
for($va=0x20120869L;$va -lt 0x20120ed5L;$va++){
 if(!$owned.ContainsKey($va)){if($null -eq $start){$start=$va}}
 elseif($null -ne $start){$gaps+=@{start=('{0:X8}' -f $start);length=$va-$start};$start=$null}
}
if($null -ne $start){$gaps+=@{start=('{0:X8}' -f $start);length=0x20120ed5L-$start}}
[ordered]@{originalSha256=$hash;verifiedLiveMemorySpans=$memory.memory.Count;instructionCount=$flow.instructionCount;verifiedInstructionBytes=$flow.uniqueInstructionBytes;gatewayCount=$flow.gateways.Count;stops=$flow.stops;unclassifiedRanges=$gaps;scope='Original-byte and non-overlap inventory verification only; not reconstructed-code matching or whole-function completeness'} | ConvertTo-Json -Depth 8
