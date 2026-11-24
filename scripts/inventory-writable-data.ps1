param([string]$BuildDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support")
$ErrorActionPreference='Stop'
# Retains existing full body/reference/initial-value checks, exposing PE helpers.
$checked=. "$PSScriptRoot\compare-data-constants.ps1" -BuildDirectory $BuildDirectory
$data=$checked|ConvertFrom-Json
$c=Get-Content "$BuildDirectory\comparison.json" -Raw|ConvertFrom-Json
if($c.PeImports.RebuiltSHA256 -cne $data.RebuiltSHA256){throw 'Stale comparison file'}
$ranges=[Collections.Generic.List[object]]::new()
function Add-Range([string]$role,[long]$old,[long]$new,[int]$length,[string]$valueMode='Exact'){
 if($valueMode -eq 'Exact'){
  for($i=0;$i -lt $length;$i+=4){$x=Initial-Dword $a ($old+$i);$y=Initial-Dword $b ($new+$i);if($length -eq 1){$x=$x -band 255;$y=$y -band 255};if($x -ne $y){throw "Initial value differs: $role"}}
 }
 $ranges.Add([pscustomobject]@{Role=$role;OriginalVA=('{0:X8}' -f $old);RebuiltVA=('{0:X8}' -f $new);Bytes=$length;Delta=$new-$old;InitialValueCheck=$valueMode})
}
Add-Range 'ATL cached codepage getter' 0x01004000 (Symbol '?g_pfnGetThreadACP@ATL@@3P6GIXZA') 4 'Pointer target validated by existing ATL body gate'
Add-Range 'Security cookie' 0x01004004 (Symbol '___security_cookie') 4
Add-Range 'Security cookie complement' 0x01004008 (Symbol '___security_cookie_complement') 4
foreach($r in $data.CrtInitialStates){Add-Range $r.Role ([Convert]::ToInt64($r.OriginalVA,16)) ([Convert]::ToInt64($r.RebuiltVA,16)) 4}
$seen=@{};foreach($r in @($c.PreCppInit.References)+@($c.PreCInit.References)+@($c.CrtStartup.References)){
 if($r.Kind -cne 'state'){continue};$old=[Convert]::ToInt64($r.OriginalTarget,16);$new=[Convert]::ToInt64($r.RebuiltTarget,16)
 if($old -lt 0x01004000 -or $old -ge 0x010044a0 -or $seen.ContainsKey($old) -or @($ranges|Where-Object OriginalVA -CEQ $r.OriginalTarget).Count){continue};$seen[$old]=$true
 Add-Range $r.Role $old $new 4
}
$report=Symbol '___report_gsfailure'
Add-Range 'Failure exception record' 0x01004058 (U32 $b ($report+0x91)) 80
Add-Range 'Failure debugger flag' 0x010040a8 (U32 $b ($report+0xc0)) 4
Add-Range 'Failure context record' 0x010040b0 (U32 $b ($report+0x7d)) 716
$afxInit=Symbol '??__E_afxInitAppState@@YAXXZ'
$oldFlag=U32 $a (0x01002d16+13);$newFlag=U32 $b ($afxInit+13)
if($oldFlag -ne 0x0100437c -or $newFlag -ne (Symbol '?_afxInitAppState@@3DA')){throw 'MFC char flag target differs'}
Add-Range 'MFC application-state flag (one-byte store)' $oldFlag $newFlag 1
foreach($r in $data.ZeroInitializedSpans){Add-Range $r.Role ([Convert]::ToInt64($r.OriginalVA,16)) ([Convert]::ToInt64($r.RebuiltVA,16)) $r.Bytes}
$symbols=@([regex]::Matches($map,'(?m)^\s*0002:\S+\s+(\S+)\s+([0-9a-fA-F]{8})\s+([^\r\n]+)')|ForEach-Object {[pscustomobject]@{Symbol=$_.Groups[1].Value;VA=$_.Groups[2].Value.ToUpperInvariant();Owner=$_.Groups[3].Value.Trim()}})
$prefix=for($i=0;$i -lt 12;$i++){$va=0x01004000+4*$i;[pscustomobject]@{VA=('{0:X8}' -f $va);Original=('{0:X8}' -f (U32 $a $va));Rebuilt=('{0:X8}' -f (U32 $b $va))}}
$oldSection=$c.PeImports.OriginalHeader.Sections|Where-Object Name -CEQ '.data';$newSection=$c.PeImports.RebuiltHeader.Sections|Where-Object Name -CEQ '.data'
function Unassigned($pe,$section,[string]$addressProperty){
 $start=$pe.Base+$section.RVA;$covered=[bool[]]::new($section.VirtualSize)
 foreach($r in $ranges){$va=[Convert]::ToInt64($r.$addressProperty,16);$offset=$va-$start;if($offset -lt 0 -or $offset+$r.Bytes -gt $covered.Length){throw 'Mapped range outside writable section'};for($i=0;$i -lt $r.Bytes;$i++){$covered[$offset+$i]=$true}}
 $gaps=@();$i=0
 while($i -lt $covered.Length){if($covered[$i]){$i++;continue};$begin=$i;$nonzero=0;$preview=@();while($i -lt $covered.Length -and !$covered[$i]){$va=$start+$i;$aligned=$va-($va%4);$v=((Initial-Dword $pe $aligned) -shr (8*($va%4))) -band 255;if($v){$nonzero++};if($preview.Count -lt 16){$preview+=('{0:X2}' -f $v)};$i++};$gaps+=[pscustomobject]@{VA=('{0:X8}' -f ($start+$begin));Bytes=$i-$begin;NonzeroBytes=$nonzero;First16Bytes=($preview -join ' ')}}
 return [pscustomobject]@{MappedUniqueBytes=@($covered|Where-Object {$_}).Count;UnassignedBytes=@($covered|Where-Object {!$_}).Count;UnassignedRegions=$gaps}
}
$originalCoverage=Unassigned $a $oldSection 'OriginalVA';$rebuiltCoverage=Unassigned $b $newSection 'RebuiltVA'
[pscustomobject]@{OriginalSHA256=$data.OriginalSHA256;RebuiltSHA256=$data.RebuiltSHA256;OriginalVirtualSize=$oldSection.VirtualSize;RebuiltVirtualSize=$newSection.VirtualSize;VirtualSizeDelta=$newSection.VirtualSize-$oldSection.VirtualSize;OriginalRangeLedger=$originalCoverage;RebuiltRangeLedger=$rebuiltCoverage;ValidatedRanges=@($ranges|Sort-Object OriginalVA);RebuiltPublicOwners=$symbols;RawPrefixDwords=@($prefix);Scope='Known reference-validated writable spans, values and actual rebuilt MAP provenance. Initial zeros are pre-constructor storage, not full original class sizes. Raw zero gaps/unmapped original globals remain unassigned. Original COFF data/BSS split, padding owners, original private names and cause of total size difference are not inferred from PE zeros.'}|ConvertTo-Json -Depth 7
