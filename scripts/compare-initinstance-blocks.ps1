param([Parameter(Mandatory)][string]$BuildDirectory)
# Conservative CFG extraction and anchor pairing; no byte/semantic match claim.
$ErrorActionPreference='Stop'
$project=[IO.Path]::GetFullPath("$PSScriptRoot\..")
$a=@((Get-Content "$project\artifacts\fsx.exe\initinstance\original-instructions.json" -Raw|ConvertFrom-Json).instructions)
$diag=Get-Content "$BuildDirectory\initinstance-diagnostic.json" -Raw|ConvertFrom-Json
$b=@($diag.Rows|Where-Object RebuiltAddress|Select-Object RebuiltAddress,Rebuilt -Unique|ForEach-Object {
 [pscustomobject]@{address=$_.RebuiltAddress;instruction=$_.Rebuilt}
}|Sort-Object {[Convert]::ToUInt32($_.address,16)})
$refs=(Get-Content "$BuildDirectory\initinstance-references.json" -Raw|ConvertFrom-Json).References
function Canon([string]$s){
 $s=$s.ToLowerInvariant() -replace '\bjne\b','jnz' -replace '\bje\b','jz'
 $s=[regex]::Replace($s,'\b([0-9][0-9a-f]*)h\b',{param($m)'0x'+[Convert]::ToUInt64($m.Groups[1].Value,16).ToString('x')})
 $s=[regex]::Replace($s,'0x[0-9a-f]+',{param($m)'0x'+[Convert]::ToUInt64($m.Value.Substring(2),16).ToString('x')})
 $s=[regex]::Replace($s,'(?<![a-z0-9_])([0-9]+)(?![a-z0-9_])',{param($m)'0x'+[Convert]::ToUInt64($m.Value,10).ToString('x')})
 return ($s -replace '\s+',' ').Trim()
}
function Blocks($instructions,[string]$Side){
 $indices=@{};for($i=0;$i -lt $instructions.Count;$i++){$indices[[Convert]::ToUInt32($instructions[$i].address,16)]=$i}
 $leaders=@{0=$true};$targets=@{}
 for($i=0;$i -lt $instructions.Count;$i++){
  $s=$instructions[$i].instruction
  if($s -match '(?i)^j\w+\s+(?:0x)?([0-9a-f]+)$'){
   $dest=[Convert]::ToUInt32($Matches[1],16)
   if(!$indices.ContainsKey($dest)){throw "Branch outside supplied function: $s"}
   $leaders[$indices[$dest]]=$true;$targets[$i]=$dest
   if($i+1 -lt $instructions.Count){$leaders[$i+1]=$true}
  }elseif($s -match '(?i)^ret\b' -and $i+1 -lt $instructions.Count){$leaders[$i+1]=$true}
 }
 $starts=@($leaders.Keys|Sort-Object {[int]$_});$blocks=@()
 for($n=0;$n -lt $starts.Count;$n++){
  $first=[int]$starts[$n];$last=if($n+1 -lt $starts.Count){[int]$starts[$n+1]-1}else{$instructions.Count-1}
  $body=@($instructions[$first..$last]);$anchors=[Collections.Generic.List[string]]::new()
  foreach($r in $refs){
   $needle=if($Side -eq 'original'){'(?i)0x0*'+([Convert]::ToUInt32($r.OriginalTarget,16).ToString('x'))+'\b'}else{[regex]::Escape($r.RebuiltSymbol)}
   foreach($ins in $body){if($ins.instruction -match $needle){$anchors.Add('ref:'+ $r.RebuiltSymbol)}}
  }
  foreach($ins in $body){$s=Canon $ins.instruction;if($s -match '^push 0x(10|17|18|19|1c)$'){$anchors.Add('error-id:'+ $Matches[1])}}
  if($first -eq 0){$anchors.Add('function-entry')}
  $edges=@();$terminal=$instructions[$last].instruction
  if($targets.ContainsKey($last)){
   $edges+=,[pscustomobject]@{Kind=if($terminal -match '(?i)^jmp\b'){'jump'}else{'taken'};Target=('{0:X8}' -f $targets[$last])}
  }
  if($terminal -notmatch '(?i)^(ret|jmp)\b' -and $last+1 -lt $instructions.Count){
   $edges+=,[pscustomobject]@{Kind='fallthrough';Target=$instructions[$last+1].address}
  }
  $blocks+=,[pscustomobject]@{Start=$body[0].address;End=$body[-1].address;Count=$body.Count;Anchors=@($anchors);Edges=$edges;Instructions=$body;Terminal=$terminal}
 }
 if(($blocks|Measure-Object Count -Sum).Sum -ne $instructions.Count){throw 'CFG lost or duplicated instructions'}
 return $blocks
}
$original=@(Blocks $a original);$rebuilt=@(Blocks $b rebuilt)
# A unique anchor identifies a candidate pair only. Multiple anchors can disagree
# when one compiler splits/merges blocks; report conflicts instead of guessing.
$proposals=@()
$anchors=@($original.Anchors|Sort-Object -Unique)
foreach($anchor in $anchors){
 $left=@($original|Where-Object {$anchor -in $_.Anchors});$right=@($rebuilt|Where-Object {$anchor -in $_.Anchors})
 if($left.Count -eq 1 -and $right.Count -eq 1){$proposals+=,[pscustomobject]@{Anchor=$anchor;Original=$left[0].Start;Rebuilt=$right[0].Start;OriginalInstructions=$left[0].Count;RebuiltInstructions=$right[0].Count}}
}
$pairs=@{};$reverse=@{};$conflicts=[Collections.Generic.List[string]]::new()
function Add-Pair([string]$left,[string]$right){
 $left=$left.ToLowerInvariant();$right=$right.ToLowerInvariant()
 if(($pairs.ContainsKey($left) -and $pairs[$left] -ne $right) -or ($reverse.ContainsKey($right) -and $reverse[$right] -ne $left)){
  $conflicts.Add("Conflicting proposal: $left -> $right");return
 }
 $pairs[$left]=$right;$reverse[$right]=$left
}
foreach($p in $proposals){Add-Pair $p.Original $p.Rebuilt}
$leftBlocks=@{};foreach($block in $original){$leftBlocks[$block.Start.ToLowerInvariant()]=$block}
$rightBlocks=@{};foreach($block in $rebuilt){$rightBlocks[$block.Start.ToLowerInvariant()]=$block}
do {
 $before=$pairs.Count
 foreach($key in @($pairs.Keys)){
  $left=$leftBlocks[$key];$right=$rightBlocks[$pairs[$key]]
  if($left.Edges.Count -eq 1 -and $right.Edges.Count -eq 1){Add-Pair $left.Edges[0].Target $right.Edges[0].Target}
  elseif($left.Edges.Count -eq 2 -and $right.Edges.Count -eq 2){
   $lt=(Canon $left.Terminal).Split(' ')[0];$rt=(Canon $right.Terminal).Split(' ')[0]
   $inverse=($lt -eq 'jz' -and $rt -eq 'jnz') -or ($lt -eq 'jnz' -and $rt -eq 'jz')
   if($lt -eq $rt -or $inverse){foreach($edge in $left.Edges){
    $kind=if($inverse){if($edge.Kind -eq 'taken'){'fallthrough'}else{'taken'}}else{$edge.Kind}
    $other=@($right.Edges|Where-Object Kind -EQ $kind)
    if($other.Count -eq 1){Add-Pair $edge.Target $other[0].Target}
   }}
  }
 }
}while($pairs.Count -gt $before)
$edgeChecks=foreach($key in @($pairs.Keys|Sort-Object)){
 $left=$leftBlocks[$key];$right=$rightBlocks[$pairs[$key]]
 $expected=@($left.Edges|ForEach-Object {$pairs[$_.Target.ToLowerInvariant()]}|Sort-Object)
 $actual=@($right.Edges|ForEach-Object {$_.Target.ToLowerInvariant()}|Sort-Object)
 [pscustomobject]@{Original=$left.Start;Rebuilt=$right.Start;OriginalInstructions=$left.Count;RebuiltInstructions=$right.Count;SuccessorSetConsistent=([string]::Join(',', $expected) -eq [string]::Join(',', $actual));OriginalTerminal=$left.Terminal;RebuiltTerminal=$right.Terminal}
}
function Instruction-Sizes($blocks,[long]$end){
 $all=@($blocks.Instructions|Sort-Object {[Convert]::ToUInt32($_.address,16)})
 $sizes=@{}
 for($i=0;$i -lt $all.Count;$i++){
  $address=[Convert]::ToUInt32($all[$i].address,16)
  $next=if($i+1 -lt $all.Count){[Convert]::ToUInt32($all[$i+1].address,16)}else{$end}
  $size=$next-$address
  if($size -lt 1 -or $size -gt 15){throw 'Invalid x86 instruction span; cannot account for bytes'}
  if($all[$i].instruction -match '(?i)^j' -and $size -notin 2,5,6){throw 'Unexpected branch encoding size'}
  $sizes[$all[$i].address.ToLowerInvariant()]=$size
 }
 return $sizes
}
$originalSizes=Instruction-Sizes $original (0x010019e0L+769)
$rebuiltSizes=Instruction-Sizes $rebuilt ([Convert]::ToUInt32($diag.RebuiltVA,16)+$diag.RebuiltLength)
$accounting=foreach($pair in $edgeChecks){
 $left=$leftBlocks[$pair.Original.ToLowerInvariant()];$right=$rightBlocks[$pair.Rebuilt.ToLowerInvariant()]
 $leftBytes=0;$rightBytes=0;$leftBranches=0;$rightBranches=0
 foreach($ins in $left.Instructions){$size=$originalSizes[$ins.address.ToLowerInvariant()];$leftBytes+=$size;if($ins.instruction -match '(?i)^j'){$leftBranches+=$size}}
 foreach($ins in $right.Instructions){$size=$rebuiltSizes[$ins.address.ToLowerInvariant()];$rightBytes+=$size;if($ins.instruction -match '(?i)^j'){$rightBranches+=$size}}
 [pscustomobject]@{Original=$pair.Original;Rebuilt=$pair.Rebuilt;OriginalBytes=$leftBytes;RebuiltBytes=$rightBytes;Delta=$rightBytes-$leftBytes;OriginalBranchBytes=$leftBranches;RebuiltBranchBytes=$rightBranches;BranchDelta=$rightBranches-$leftBranches;OtherDelta=($rightBytes-$rightBranches)-($leftBytes-$leftBranches)}
}
$byteTotals=[pscustomobject]@{TotalDelta=($accounting|Measure-Object Delta -Sum).Sum;BranchDelta=($accounting|Measure-Object BranchDelta -Sum).Sum;OtherDelta=($accounting|Measure-Object OtherDelta -Sum).Sum;Scope='Length accounting only, from disassembler instruction spans; equal lengths are not equal byte contents.'}
$result=[ordered]@{DiagnosticOnly=$true;Scope='CFG from direct branches/returns, calls remain inside blocks. Reference/error-ID anchors seed pairs; equal or inverted JZ/JNZ and single-successor edges propagate candidates. Graph consistency is not instruction/semantic/byte equality.';OriginalBlockCount=$original.Count;RebuiltBlockCount=$rebuilt.Count;PairedBlocks=$pairs.Count;Conflicts=@($conflicts|Sort-Object -Unique);EdgeChecks=$edgeChecks;ByteTotals=$byteTotals;ByteAccounting=$accounting;Original=$original;Rebuilt=$rebuilt;AnchorProposals=$proposals}
$result|ConvertTo-Json -Depth 10|Set-Content "$BuildDirectory\initinstance-blocks.json"
$accounting|Export-Csv "$BuildDirectory\initinstance-block-bytes.csv" -NoTypeInformation
$html=@('<!doctype html><meta charset="utf-8"><title>InitInstance blocks</title><style>body{font-family:system-ui}table{width:100%;border-collapse:collapse}td,th{border:1px solid #bbb;padding:8px;vertical-align:top}pre{white-space:pre-wrap}article{border:1px solid #aaa;padding:12px;margin:12px 0}a{margin-right:8px}</style><h1>InitInstance control-flow blocks</h1><p>Diagnostic only. Anchor pairs are navigation aids, not certified matches. Each branch target links to its block.</p><h2>Unique anchor proposals</h2><table><tr><th>Anchor</th><th>Original</th><th>Rebuilt</th></tr>')
foreach($p in $proposals){$html+='<tr><td>'+[Net.WebUtility]::HtmlEncode($p.Anchor)+'</td><td><a href="#original-'+$p.Original+'">'+$p.Original+'</a></td><td><a href="#rebuilt-'+$p.Rebuilt+'">'+$p.Rebuilt+'</a></td></tr>'}
$html+='</table><table><tr><th>Original blocks</th><th>Rebuilt blocks</th></tr><tr>'
foreach($side in @(@{Name='original';Blocks=$original},@{Name='rebuilt';Blocks=$rebuilt})){
 $html+='<td>'
 foreach($block in $side.Blocks){
  $html+='<article id="'+$side.Name+'-'+$block.Start+'"><b>'+ $block.Start+' ('+$block.Count+' instructions)</b><pre>'
  foreach($ins in $block.Instructions){$html += [Net.WebUtility]::HtmlEncode($ins.address+' '+$ins.instruction)+"`n"}
  $html+='</pre>'
  foreach($edge in $block.Edges){$html+='<a href="#'+$side.Name+'-'+$edge.Target+'">'+$edge.Kind+': '+$edge.Target+'</a>'}
  $html+='</article>'
 }
 $html+='</td>'
}
$html+='</tr></table><h2>Propagated block pairs</h2><p>Pairs: '+$pairs.Count+'; conflicts: '+$result.Conflicts.Count+'. Successor consistency is structural only.</p><table><tr><th>Original</th><th>Rebuilt</th><th>Instruction counts</th><th>Successors consistent</th></tr>'
foreach($check in $edgeChecks){$html+='<tr><td><a href="#original-'+$check.Original+'">'+$check.Original+'</a></td><td><a href="#rebuilt-'+$check.Rebuilt+'">'+$check.Rebuilt+'</a></td><td>'+$check.OriginalInstructions+' / '+$check.RebuiltInstructions+'</td><td>'+$check.SuccessorSetConsistent+'</td></tr>'}
$html+='</table><h2>Encoded length accounting</h2><p>Length delta: '+$byteTotals.TotalDelta+' bytes; branches: '+$byteTotals.BranchDelta+'; other instructions: '+$byteTotals.OtherDelta+'. Equal lengths do not establish equal bytes.</p>'
$html+=($accounting|ConvertTo-Html -Fragment|Out-String)
$html|Set-Content "$BuildDirectory\initinstance-blocks.html"
[pscustomobject]@{OriginalBlocks=$original.Count;RebuiltBlocks=$rebuilt.Count;UniqueAnchorProposals=$proposals.Count;PairedBlocks=$pairs.Count;Conflicts=$result.Conflicts;EdgeMismatches=@($edgeChecks|Where-Object {!$_.SuccessorSetConsistent}).Count;Report="$BuildDirectory\initinstance-blocks.html"}|ConvertTo-Json
