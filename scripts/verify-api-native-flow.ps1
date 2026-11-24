param([Parameter(Mandatory=$true)][string]$FlowPath,[switch]$RequireContiguous)
$ErrorActionPreference='Stop'
. "$PSScriptRoot/lib/pe-image.ps1"
$original=Join-Path $PSScriptRoot '../original/api.dll'
$hash=(Get-FileHash -LiteralPath $original).Hash
if($hash -cne '9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003'){throw 'Wrong original'}
$p=Open-PE $original
$flow=Get-Content -LiteralPath $FlowPath -Raw | ConvertFrom-Json
$owners=@{}
foreach($row in $flow.instructions){
 $va=[Convert]::ToInt64($row.address,16)
 if([BitConverter]::ToString((Bytes $p $va $row.bytes)).Replace('-','') -cne $row.hex){throw ('Code mismatch '+$row.address)}
 for($j=0;$j -lt $row.bytes;$j++){if($owners.ContainsKey($va+$j)){throw 'Overlap'};$owners[$va+$j]='code'}
}
if($owners.Count -ne $flow.uniqueInstructionBytes){throw 'Count mismatch'}
$descriptorBytes=0
foreach($g in $flow.gateways){
 $va=[Convert]::ToInt64($g.descriptor,16)
 $n=[int]$g.descriptorBytes
 if($n -notin @(8,16) -or [Convert]::ToInt64($g.resume,16) -ne $va+$n){throw 'Invalid descriptor'}
 for($j=0;$j -lt $n;$j++){if($owners.ContainsKey($va+$j)){throw 'Descriptor/code overlap'};$owners[$va+$j]='descriptor'}
 $descriptorBytes+=$n
}
$lo=[long]($owners.Keys|Measure-Object -Minimum).Minimum
$hi=1+[long]($owners.Keys|Measure-Object -Maximum).Maximum
if($hi-$lo -gt 0x10000){throw 'Excessive interval'}
$unclassified=0
for($va=$lo;$va -lt $hi;$va++){if(!$owners.ContainsKey($va)){$unclassified++}}
if($RequireContiguous -and $unclassified){throw 'Inventory has interior gaps'}
if($flow.stops.Count){throw 'Inventory has unresolved stops'}
[ordered]@{originalSha256=$hash;entry=$flow.entry;instructions=$flow.instructionCount;verifiedInstructionBytes=$flow.uniqueInstructionBytes;descriptorBytes=$descriptorBytes;descriptors=$flow.gateways.Count;intervalStart=('{0:X8}' -f $lo);intervalEndExclusive=('{0:X8}' -f $hi);unclassifiedBytes=$unclassified;scope='Native original-byte inventory only, call-opaque success continuations; no source matching/integrity failure/runtime proof'}|ConvertTo-Json -Depth 5
