param([Parameter(Mandatory)][string]$BuildDirectory,
      [string]$OutputDirectory = $BuildDirectory)
# Diagnostic alignment only. Address/branch tokens are deliberately abstracted;
# this is NOT an exact-match acceptance check or a semantic equivalence proof.
$ErrorActionPreference='Stop'
$project=[IO.Path]::GetFullPath("$PSScriptRoot\..")
$original=Get-Content "$project\artifacts\fsx.exe\initinstance\original-instructions.json" -Raw|ConvertFrom-Json
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
$entry=[regex]::Match($map,'(?m)^\s*\S+\s+\?InitInstance@FSXAppPilot@@UAEHXZ\s+([0-9A-Fa-f]{8})')
if(!$entry.Success){throw 'InitInstance MAP symbol missing'}
$start=[Convert]::ToUInt32($entry.Groups[1].Value,16)
$candidate=@();$collect=$false
foreach($line in Get-Content "$BuildDirectory\disassembly.txt"){
 if($line -match '^\?InitInstance@FSXAppPilot@@UAEHXZ:'){$collect=$true;continue}
 if(!$collect){continue}
 if($line -match '^\S.*:\s*$'){break}
 if($line -match '^\s+([0-9A-Fa-f]{8}):\s+((?:[0-9A-Fa-f]{2} )+)\s+([a-z][a-z0-9]*\s*.*)$'){
  $text=$Matches[3].Trim();if($text -match '^int\s+3$'){continue}
  $candidate+=,[pscustomobject]@{address=$Matches[1];instruction=$text;bytes=$Matches[2].Trim()}
 }
}
if(!$candidate.Count -or $candidate[-1].instruction -notmatch '^ret\b'){throw 'Unexpected function boundary'}
function Normalize([string]$Instruction){
 $s=$Instruction.ToLowerInvariant()
 $s=$s -replace '\bjne\b','jnz' -replace '\bje\b','jz'
 $s=[regex]::Replace($s,'\b([0-9][0-9a-f]*)h\b', {param($m) '0x'+([Convert]::ToUInt64($m.Groups[1].Value,16).ToString('x'))})
 $s=[regex]::Replace($s,'0x[0-9a-f]+', {param($m) '0x'+([Convert]::ToUInt64($m.Value.Substring(2),16).ToString('x'))})
 if($s -match '^j[a-z]+\s+') {return ($s -replace '\s+.*$',' <branch>')}
 if($s -match '^call\s+(?:0x[0-9a-f]+|[?@_])') {return 'call <direct-target>'}
 $s=$s -replace '\[[^\]]*[?@_][^\]]*\]','[<reference>]'
 $s=$s -replace '\b(?:offset)\s+\S+','<reference>'
 $s=[regex]::Replace($s,'0x[0-9a-f]+', {param($m) if([Convert]::ToUInt64($m.Value.Substring(2),16) -ge 0x10000){'<reference>'}else{$m.Value}})
 $s=[regex]::Replace($s,'(?<![a-z0-9_])([0-9]+)(?![a-z0-9_])', {param($m) '0x'+([Convert]::ToUInt64($m.Value,10).ToString('x'))})
 $s=$s -replace '\b(?:dword|word|byte)\s+ptr\s*',''
 $s=$s -replace '\bds:','' -replace '\s+',''
 return $s
}
$a=@($original.instructions);$b=@($candidate)
$an=@($a|ForEach-Object {Normalize $_.instruction});$bn=@($b|ForEach-Object {Normalize $_.instruction})
$dp=[int[,]]::new($a.Count+1,$b.Count+1)
for($i=$a.Count-1;$i -ge 0;$i--){for($j=$b.Count-1;$j -ge 0;$j--){
 if($an[$i] -eq $bn[$j]){$dp[$i,$j]=1+$dp[($i+1),($j+1)]}else{$dp[$i,$j]=[Math]::Max($dp[($i+1),$j],$dp[$i,($j+1)])}
}}
$rows=@();$i=0;$j=0
while($i -lt $a.Count -or $j -lt $b.Count){
 if($i -lt $a.Count -and $j -lt $b.Count -and $an[$i] -eq $bn[$j]){
  $rows+=,[pscustomobject]@{Kind='aligned';OriginalAddress=$a[$i].address;Original=$a[$i].instruction;RebuiltAddress=$b[$j].address;Rebuilt=$b[$j].instruction};$i++;$j++
 }elseif($j -ge $b.Count -or ($i -lt $a.Count -and $dp[($i+1),$j] -ge $dp[$i,($j+1)])){
  $rows+=,[pscustomobject]@{Kind='original-only';OriginalAddress=$a[$i].address;Original=$a[$i].instruction;RebuiltAddress='';Rebuilt=''};$i++
 }else{$rows+=,[pscustomobject]@{Kind='rebuilt-only';OriginalAddress='';Original='';RebuiltAddress=$b[$j].address;Rebuilt=$b[$j].instruction};$j++}
}
$last=$b[-1];$length=[Convert]::ToUInt32($last.address,16)-$start+($last.bytes -split ' ').Count
$result=[ordered]@{Target='InitInstance';DiagnosticOnly=$true;OriginalVA='010019e0';OriginalLength=769;RebuiltVA=('{0:X8}' -f $start);RebuiltLength=$length;OriginalInstructions=$a.Count;RebuiltInstructions=$b.Count;AlignedInstructionTokens=$dp[0,0];OriginalFrame='0x554';RebuiltFrame=($b|Where-Object instruction -Match '^sub\s+esp,'|Select-Object -First 1).instruction;Scope='LCS diagnostic: address and branch targets abstracted, no verified reference mapping, no exact-match score';Rows=$rows}
New-Item -ItemType Directory -Path $OutputDirectory -Force|Out-Null
$result|ConvertTo-Json -Depth 6|Set-Content "$OutputDirectory\initinstance-diagnostic.json"
$body=foreach($row in $rows){'<tr class="'+$row.Kind+'"><td>'+[Net.WebUtility]::HtmlEncode($row.OriginalAddress+' '+$row.Original)+'</td><td>'+[Net.WebUtility]::HtmlEncode($row.RebuiltAddress+' '+$row.Rebuilt)+'</td></tr>'}
@('<!doctype html><meta charset="utf-8"><title>InitInstance diagnostic</title><style>body{font-family:monospace}table{border-collapse:collapse;width:100%}td{padding:3px 8px;white-space:pre}.original-only{background:#ffe1e1}.rebuilt-only{background:#e0f1ff}td{border:1px solid #ddd}</style><h1>InitInstance instruction alignment</h1><p>Diagnostic only. Addresses and branch targets abstracted. This does not certify a match.</p><table><tr><th>Original fsx.exe</th><th>Rebuilt</th></tr>')+$body+@('</table>')|Set-Content "$OutputDirectory\initinstance-diagnostic.html"
$result.Remove('Rows');$result|ConvertTo-Json -Depth 3

