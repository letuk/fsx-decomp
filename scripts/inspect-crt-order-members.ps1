param([string]$Library="$PSScriptRoot\..\vm-share\toolchains\vc80-rtm\msvcrt.lib",
 [string]$MemberPattern='(?i)(^|[\\/])(crtexew|secchk|atonexit|sehprolg4gs|alloca16|pesect|gs_report)\.obj/?$')
$ErrorActionPreference='Stop'
$b=[IO.File]::ReadAllBytes((Resolve-Path $Library));$ascii=[Text.Encoding]::ASCII
if($ascii.GetString($b,0,8) -cne "!<arch>`n"){throw 'Not a COFF archive'}
$longNames='';$rows=@()
for($offset=8;$offset+60 -le $b.Length;){
 $name=$ascii.GetString($b,$offset,16).Trim();$size=[int]::Parse($ascii.GetString($b,$offset+48,10).Trim());$data=$offset+60
 if($data+$size -gt $b.Length){throw 'Invalid member boundary'}
 if($name -ceq '//'){$longNames=$ascii.GetString($b,$data,$size)}
 elseif($name -match '^/(\d+)$'){$start=[int]$Matches[1];$end=$longNames.IndexOf([char]0,$start);if($end -lt 0){throw 'Missing long-name terminator'};$name=$longNames.Substring($start,$end-$start)}
 if($name -match $MemberPattern){
  if([BitConverter]::ToUInt16($b,$data) -ne 0x14c){throw 'Expected regular i386 COFF object'}
  $sections=@();$count=[BitConverter]::ToUInt16($b,$data+2);$table=$data+20+[BitConverter]::ToUInt16($b,$data+16)
  for($i=0;$i -lt $count;$i++){$s=$table+40*$i;$flags=[BitConverter]::ToUInt32($b,$s+36);$sections+=[pscustomobject]@{Name=$ascii.GetString($b,$s,8).TrimEnd([char]0);Size=[BitConverter]::ToUInt32($b,$s+16);Flags=('{0:X8}' -f $flags);Comdat=[bool]($flags -band 0x1000)}}
  $symbolOffset=$data+[BitConverter]::ToUInt32($b,$data+8);$symbolCount=[BitConverter]::ToUInt32($b,$data+12);$stringOffset=$symbolOffset+18*$symbolCount
  $symbols=@()
  for($i=0;$i -lt $symbolCount;){$s=$symbolOffset+18*$i;if($s+18 -gt $data+$size){throw 'Invalid symbol boundary'}
   if([BitConverter]::ToUInt32($b,$s) -eq 0){$start=$stringOffset+[BitConverter]::ToUInt32($b,$s+4);$end=$start;while($end -lt $data+$size -and $b[$end]){$end++};if($end -ge $data+$size){throw 'Invalid symbol string'};$symbolName=$ascii.GetString($b,$start,$end-$start)}else{$symbolName=$ascii.GetString($b,$s,8).TrimEnd([char]0)}
   $symbols+=[pscustomobject]@{Name=$symbolName;Value=[BitConverter]::ToUInt32($b,$s+8);Section=[BitConverter]::ToInt16($b,$s+12);StorageClass=$b[$s+16]};$i+=1+$b[$s+17]
  }
  for($i=0;$i -lt $sections.Count;$i++){$sections[$i]|Add-Member -NotePropertyName Symbols -NotePropertyValue @($symbols|Where-Object Section -EQ ($i+1))}
  $directives=@();for($i=0;$i -lt $count;$i++){if($sections[$i].Name -ceq '.drectve'){$s=$table+40*$i;$raw=$data+[BitConverter]::ToUInt32($b,$s+20);$length=$sections[$i].Size;if($raw+$length -gt $data+$size){throw 'Invalid directives boundary'};$directives+=$ascii.GetString($b,$raw,$length)}}
  $sha=[Security.Cryptography.SHA256]::Create();try{$hash=[BitConverter]::ToString($sha.ComputeHash($b,$data,$size)).Replace('-','')}finally{$sha.Dispose()}
  $rows+=[pscustomobject]@{Member=$name;Size=$size;SHA256=$hash;Sections=$sections;CommonSymbols=@($symbols|Where-Object {$_.Section -eq 0 -and $_.StorageClass -eq 2 -and $_.Value -gt 0});Directives=$directives}
 }
 $offset=$data+$size+($size%2)
}
[pscustomobject]@{LibrarySHA256=(Get-FileHash $Library).Hash;Members=$rows;Scope='Read-only archive member/COFF section and symbol inspection; COMDAT flags do not alone certify actual linker acceptance.'}|ConvertTo-Json -Depth 8
