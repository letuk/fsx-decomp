param([string]$BuildDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support",
 [string]$PdbFile="$BuildDirectory\fsx.pdb")
$ErrorActionPreference='Stop'
# Narrow read-only MSF7/DBI reader. Layout references: LLVM PDB MSF/DBI docs
# and llvm/include/llvm/DebugInfo/PDB/Native/RawTypes.h. No PDB/EXE mutation.
$raw=[IO.File]::ReadAllBytes((Resolve-Path $PdbFile))
if(![Text.Encoding]::ASCII.GetString($raw,0,24).StartsWith('Microsoft C/C++ MSF 7.00')){throw 'Expected MSF7 PDB'}
$block=[BitConverter]::ToUInt32($raw,32);$directorySize=[BitConverter]::ToUInt32($raw,44);$blockMap=[BitConverter]::ToUInt32($raw,52)
if($block -notin 512,1024,2048,4096 -or ([long][BitConverter]::ToUInt32($raw,40)*$block) -ne $raw.Length){throw 'Unsupported MSF geometry'}
function Read-Blocks([long[]]$ids,[int]$length){$bytes=[byte[]]::new($length);$done=0;foreach($id in $ids){if($id -lt 0 -or ($id+1)*$block -gt $raw.Length){throw 'Invalid block'};$count=[Math]::Min($block,$length-$done);[Array]::Copy($raw,$id*$block,$bytes,$done,$count);$done+=$count};if($done -ne $length){throw 'Incomplete stream'};return ,$bytes}
$directoryBlocks=[int][Math]::Ceiling($directorySize/[double]$block)
if($directoryBlocks*4 -gt $block){throw 'Multi-page directory block maps are outside this reader scope'}
$ids=@(for($i=0;$i -lt $directoryBlocks;$i++){[BitConverter]::ToUInt32($raw,$blockMap*$block+4*$i)})
$directory=Read-Blocks $ids $directorySize;$streamCount=[BitConverter]::ToUInt32($directory,0)
if($streamCount -lt 4 -or 4+4*$streamCount -gt $directory.Length){throw 'Invalid stream directory'}
$streams=@();$cursor=4+4*$streamCount
for($i=0;$i -lt $streamCount;$i++){$size=[BitConverter]::ToUInt32($directory,4+4*$i);$count=if($size -eq [uint32]::MaxValue){0}else{[int][Math]::Ceiling($size/[double]$block)};if($cursor+4*$count -gt $directory.Length){throw 'Invalid stream block list'};$ids=@(for($j=0;$j -lt $count;$j++){[BitConverter]::ToUInt32($directory,$cursor+4*$j)});$streams+=@{Size=$size;Blocks=$ids};$cursor+=4*$count}
if($cursor -ne $directory.Length){throw 'Trailing stream-directory bytes'}
$info=Read-Blocks $streams[1].Blocks $streams[1].Size
$debug=(& "$PSScriptRoot\verify-debug-record.ps1" -BuildDirectory $BuildDirectory -PdbPathMode Basename|Out-String)|ConvertFrom-Json
$guid=([BitConverter]::ToString($info,12,16)).Replace('-','')
if($guid -cne $debug.RebuiltRecord.GuidRawBytes -or [BitConverter]::ToUInt32($info,8) -ne $debug.RebuiltRecord.Age){throw 'PDB does not match executable RSDS identity/age'}
$dbi=Read-Blocks $streams[3].Blocks $streams[3].Size
if($dbi.Length -lt 64 -or [BitConverter]::ToInt32($dbi,0) -ne -1 -or [BitConverter]::ToUInt16($dbi,58) -ne 0x14c){throw 'Unsupported DBI header'}
$modSize=[BitConverter]::ToUInt32($dbi,24);$scSize=[BitConverter]::ToUInt32($dbi,28);$modEnd=64+$modSize
if($modEnd+$scSize -gt $dbi.Length){throw 'Invalid DBI substream sizes'}
function Read-Name([ref]$offset,[int]$bound){$start=$offset.Value;while($offset.Value -lt $bound -and $dbi[$offset.Value]){$offset.Value++};if($offset.Value -ge $bound){throw 'Unterminated DBI module name'};$name=[Text.Encoding]::UTF8.GetString($dbi,$start,$offset.Value-$start);$offset.Value++;return $name}
$modules=@();$cursor=64
while($cursor -lt $modEnd){if($cursor+64 -gt $modEnd){throw 'Truncated module header'};$stream=[BitConverter]::ToUInt16($dbi,$cursor+34);$cursor+=64;$name=Read-Name ([ref]$cursor) $modEnd;$object=Read-Name ([ref]$cursor) $modEnd;$cursor=($cursor+3) -band -4;$modules+=[pscustomobject]@{Index=$modules.Count;Module=$name;Object=$object;DebugStream=$stream}}
if($cursor -ne $modEnd -or $scSize -lt 4){throw 'Invalid module/contribution substream'}
$version=[BitConverter]::ToUInt32($dbi,$modEnd)
if($version -ne (0xeffe0000L+19970605) -or ($scSize-4)%28){throw 'Unsupported section-contribution version/extent'}
$contributions=@()
for($cursor=$modEnd+4;$cursor -lt $modEnd+$scSize;$cursor+=28){$section=[BitConverter]::ToUInt16($dbi,$cursor);$offset=[BitConverter]::ToInt32($dbi,$cursor+4);$size=[BitConverter]::ToInt32($dbi,$cursor+8);$module=[BitConverter]::ToUInt16($dbi,$cursor+16);if($offset -lt 0 -or $size -lt 0 -or $module -ge $modules.Count){throw 'Invalid contribution'};$contributions+=[pscustomobject]@{Section=$section;Offset=$offset;Bytes=$size;Flags=('{0:X8}' -f [BitConverter]::ToUInt32($dbi,$cursor+12));ModuleIndex=$module;Module=$modules[$module].Module;Object=$modules[$module].Object}}
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
$mapChecks=@(foreach($m in [regex]::Matches($map,'(?m)^\s*0002:([0-9a-fA-F]+)\s+(\S+)\s+[0-9a-fA-F]{8}\s+([^\r\n]+)')){
 $offset=[Convert]::ToInt64($m.Groups[1].Value,16);$owner=$m.Groups[3].Value.Trim();$symbol=$m.Groups[2].Value
 $hit=@($contributions|Where-Object {$_.Section -eq 2 -and $_.Offset -le $offset -and $_.Offset+$_.Bytes -gt $offset})
 if($hit.Count -gt 1){throw 'Overlapping contributions at public data symbol'}
 if($hit.Count){$expected=if($owner -ceq '<common>'){'* Linker *'}else{$owner.Split(':')[-1]};$actual=[IO.Path]::GetFileName($hit[0].Module);if($actual -cne $expected){throw "PDB/MAP owner mismatch at $symbol"}}
 elseif($symbol -cnotin @('?pfnGetApplication@@3P6APAVApplicationPilot@@XZA','?g_mainWindowPilot@@3VCWnd@@A','?g_fsxAppPilot@@3VFSXAppPilot@@A')){throw "Unreviewed missing contribution at $symbol"}
 [pscustomobject]@{Symbol=$symbol;Offset=$offset;MapOwner=$owner;ContributionFound=($hit.Count -eq 1);PdbModule=$(if($hit.Count){$hit[0].Module}else{$null})}
})
[pscustomobject]@{ExeSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;PdbFile=(Resolve-Path $PdbFile).Path;PdbSHA256=(Get-FileHash $PdbFile).Hash;PdbGuid=$guid;PdbMatchesExe=$true;BlockSize=$block;DbiVersion=[BitConverter]::ToUInt32($dbi,4);Modules=$modules;Contributions=$contributions;PublicDataMapChecks=$mapChecks;Scope='Read-only MSF7/DBI v60 section contributions paired by RSDS GUID/age. Module indices are reported as stored and require MAP cross-checks. No original PDB, original contribution order, symbol sizes or padding causation claim.'}|ConvertTo-Json -Depth 6
