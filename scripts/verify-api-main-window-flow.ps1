param([string]$FlowPath='artifacts/api.dll/api-main-window-flow-20261103.json')
$ErrorActionPreference='Stop'
. "$PSScriptRoot/lib/pe-image.ps1"
$original=Join-Path $PSScriptRoot '../original/api.dll'
$hash=(Get-FileHash -LiteralPath $original).Hash
if($hash -cne '9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003'){throw 'Wrong original'}
$p=Open-PE $original
$f=Get-Content -LiteralPath $FlowPath -Raw | ConvertFrom-Json
$owners=@{}
foreach($r in $f.instructions){
 $va=[Convert]::ToInt64($r.address,16)
 $hex=[BitConverter]::ToString((Bytes $p $va $r.bytes)).Replace('-','')
 if($hex -cne $r.hex){throw ('Instruction mismatch '+$r.address)}
 for($j=0;$j -lt $r.bytes;$j++){if($owners.ContainsKey($va+$j)){throw 'Overlap'};$owners[$va+$j]='code'}
}
if($owners.Count -ne $f.uniqueInstructionBytes){throw 'Code count differs'}
$descriptors=@()
foreach($g in $f.gateways){
 $va=[Convert]::ToInt64($g.descriptor,16)
 if($g.gateway -cne '201ECCDF' -or $g.descriptorBytes -ne 16 -or [Convert]::ToInt64($g.resume,16) -ne $va+16){throw 'Unexpected gateway'}
 for($j=0;$j -lt 16;$j++){if($owners.ContainsKey($va+$j)){throw 'Descriptor decoded as code'};$owners[$va+$j]='descriptor'}
 $descriptors+=@{address=$g.descriptor;hex=[BitConverter]::ToString((Bytes $p $va 16)).Replace('-','')}
}
for($va=0x2012205dL;$va -lt 0x2012260fL;$va++){if(!$owners.ContainsKey($va)){throw ('Unclassified interior byte {0:X8}' -f $va)}}
if($f.stops.Count){throw 'Unresolved flow stops'}
[ordered]@{originalSha256=$hash;instructionCount=$f.instructionCount;verifiedInstructionBytes=$f.uniqueInstructionBytes;descriptorCount=$f.gateways.Count;descriptorBytes=16*$f.gateways.Count;intervalStart='2012205D';intervalEndExclusive='2012260F';descriptors=$descriptors;scope='Call-opaque success-path inventory, not reconstructed-code matching; integrity failure/exception paths excluded'}|ConvertTo-Json -Depth 7
