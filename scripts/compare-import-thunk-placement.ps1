param([string]$BuildDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support",[long]$ExpectedDelta=-32)
$ErrorActionPreference='Stop'
$inventory=(& "$PSScriptRoot\inventory-runtime-order.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
. "$PSScriptRoot\lib\pe-image.ps1"
$original="$PSScriptRoot\..\original\fsx.exe"
if((Get-FileHash $original).Hash -cne '61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735'){throw 'Original identity changed'}
$a=Open-PE $original;$b=Open-PE "$BuildDirectory\fsx.exe";Load-Imports $a;Load-Imports $b
if($inventory.OriginalThunks -ne 80 -or $inventory.RebuiltThunks -ne 80 -or $inventory.AllThunks.Count -ne 80){throw 'Expected exactly 80 uniquely matched import thunks'}
function Count-CC($pe,[long]$entry){for($i=0;$i -lt 32;$i++){if((Bytes $pe ($entry+6+$i) 1)[0] -ne 0xcc){return $i}};throw 'Padding scan reached bound'}
$rows=foreach($r in $inventory.AllThunks){
 $av=[Convert]::ToInt64($r.OriginalVA,16);$bv=[Convert]::ToInt64($r.RebuiltVA,16)
 if($bv-$av -ne $ExpectedDelta){throw "Thunk placement differs: $($r.Identity)"}
 foreach($pair in @(@($a,$av),@($b,$bv))){$x=Bytes $pair[0] $pair[1] 6;if($x[0] -ne 255 -or $x[1] -ne 37 -or $pair[0].Imports[[long][BitConverter]::ToUInt32($x,2)] -cne $r.Identity){throw 'FF25/IAT target mismatch'}}
 $oldCC=Count-CC $a $av;$newCC=Count-CC $b $bv
 [pscustomobject]@{Identity=$r.Identity;OriginalVA=$r.OriginalVA;RebuiltVA=$r.RebuiltVA;Delta=$bv-$av;SixByteBodyTargetValidated=$true;OriginalTrailingCCBytes=$oldCC;RebuiltTrailingCCBytes=$newCC;TrailingCCMatches=($oldCC -eq $newCC)}
}
[pscustomobject]@{OriginalSHA256=(Get-FileHash $original).Hash;RebuiltSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;ValidatedThunks=$rows.Count;UniformDelta=$ExpectedDelta;TrailingCCMismatches=@($rows|Where-Object {!$_.TrailingCCMatches}).Count;Thunks=@($rows);Scope='All raw FF25 stubs in executable sections matched by unique imported identity; exact opcode and four-byte IAT operand validated against each image imports. Placement and bounded immediate CC runs checked. IAT addresses differ by identity mapping; not raw byte equality, whole-code coverage, import-table order or runtime equivalence.'}|ConvertTo-Json -Depth 6
