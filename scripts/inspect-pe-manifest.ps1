param([string]$Binary="$PSScriptRoot\..\original\fsx.exe")
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$p=Open-PE $Binary;$opt=[BitConverter]::ToInt32($p.Bytes,60)+24
$resourceRVA=[BitConverter]::ToUInt32($p.Bytes,$opt+112);$resourceSize=[BitConverter]::ToUInt32($p.Bytes,$opt+116)
if(!$resourceRVA -or !$resourceSize){throw 'No resource directory'}
$root=$p.Base+$resourceRVA
function Walk([long]$relative,[string[]]$path){
 if($path.Count -gt 3 -or $relative+16 -gt $resourceSize){throw 'Invalid resource directory'}
 $header=Bytes $p ($root+$relative) 16;$count=[BitConverter]::ToUInt16($header,12)+[BitConverter]::ToUInt16($header,14)
 for($i=0;$i -lt $count;$i++){
  $entry=Bytes $p ($root+$relative+16+8*$i) 8;$id=[BitConverter]::ToUInt32($entry,0);$target=[BitConverter]::ToUInt32($entry,4)
  if($path.Count -eq 0 -and $id -ne 24){continue}
  if($id -band 0x80000000L){$s=$root+($id -band 0x7fffffffL);$length=[BitConverter]::ToUInt16((Bytes $p $s 2),0);$label=[Text.Encoding]::Unicode.GetString((Bytes $p ($s+2) (2*$length)))}else{$label=[string]$id}
  $next=@($path)+$label
  if($target -band 0x80000000L){Walk ($target -band 0x7fffffffL) $next;continue}
  if($target+16 -gt $resourceSize){throw 'Invalid resource data entry'}
  $d=Bytes $p ($root+$target) 16;$rva=[BitConverter]::ToUInt32($d,0);$length=[BitConverter]::ToUInt32($d,4);$raw=Bytes $p ($p.Base+$rva) $length
  $text=[Text.Encoding]::UTF8.GetString($raw).Trim([char]0);[xml]$xml=$text
  $dependencies=@($xml.SelectNodes("//*[local-name()='dependentAssembly']/*[local-name()='assemblyIdentity']")|ForEach-Object {[pscustomobject]@{Name=$_.GetAttribute('name');Version=$_.GetAttribute('version');Architecture=$_.GetAttribute('processorArchitecture');PublicKeyToken=$_.GetAttribute('publicKeyToken')}})
  [pscustomobject]@{ResourcePath=($next -join '/');RVA=('{0:X8}' -f $rva);Bytes=$length;CodePage=[BitConverter]::ToUInt32($d,8);Dependencies=$dependencies;Xml=$text}
 }
}
[pscustomobject]@{BinarySHA256=(Get-FileHash $Binary).Hash;Manifests=@(Walk 0 @());Scope='Read-only PE resource tree type24 traversal and embedded manifest XML. Dependency versions state requested assemblies; they do not identify compiler/build host, resolved runtime versions or original library files.'}|ConvertTo-Json -Depth 7
