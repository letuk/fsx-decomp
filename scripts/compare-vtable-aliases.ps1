param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$original="$PSScriptRoot\..\original\fsx.exe"
$a=Open-PE $original;$b=Open-PE "$BuildDirectory\fsx.exe"
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
function Symbol([string]$name){$m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($name)+'\s+([0-9a-fA-F]{8})\b');if(!$m.Success){throw "Missing symbol $name"};return [Convert]::ToInt64($m.Groups[1].Value,16)}
$tables=@(@{Name='App';Original=0x01001398;Rebuilt=(Symbol '??_7FSXAppPilot@@6B@');Count=48},@{Name='Filter';Original=0x01001458;Rebuilt=(Symbol '??_7FSXMessageFilterPilot@@6B@');Count=28})
$targets=foreach($table in $tables){foreach($i in 0..($table.Count-1)){
 [pscustomobject]@{Label=$table.Name+'+'+('{0:X2}' -f (4*$i));Original=(U32 $a ($table.Original+4*$i));Rebuilt=(U32 $b ($table.Rebuilt+4*$i))}
}}
$differences=@();$shared=0
for($i=0;$i -lt $targets.Count;$i++){for($j=$i+1;$j -lt $targets.Count;$j++){
 $oldSame=$targets[$i].Original -eq $targets[$j].Original;$newSame=$targets[$i].Rebuilt -eq $targets[$j].Rebuilt
 if($oldSame){$shared++}
 if($oldSame -ne $newSame){$differences+=,[pscustomobject]@{First=$targets[$i].Label;Second=$targets[$j].Label;OriginalShared=$oldSame;RebuiltShared=$newSame}}
}}
[pscustomobject]@{OriginalSHA256=(Get-FileHash $original).Hash;RebuiltSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;Slots=$targets.Count;Pairs=($targets.Count*($targets.Count-1)/2);OriginalSharedPairs=$shared;AliasPatternDifferences=$differences.Count;Differences=$differences;Scope='Address-sharing relations across observed48 application and28 filter slots only. No callee identity/behavior, original linker-option identity, whole-vtable byte match or whole-file equality proof.'}|ConvertTo-Json -Depth 5
