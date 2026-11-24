param([string]$ControlDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\37-original-header",
 [string]$ProbeDirectory="$PSScriptRoot\..\vm-share\modules\fsx\runs\debug-gap-probe32-20261103\candidates\debug-gap-probe32-20261103-001")
$ErrorActionPreference='Stop'
$control=(& "$PSScriptRoot\compare-data-constants.ps1" -BuildDirectory $ControlDirectory|Out-String)|ConvertFrom-Json
$probe=(& "$PSScriptRoot\compare-data-constants.ps1" -BuildDirectory $ProbeDirectory|Out-String)|ConvertFrom-Json
$cp=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $ControlDirectory -RequireImportContents|Out-String)|ConvertFrom-Json
$pp=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $ProbeDirectory -RequireImportContents|Out-String)|ConvertFrom-Json
$oldHashes=@(Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json);$newHashes=@(Get-Content "$ProbeDirectory\source-hashes.json" -Raw|ConvertFrom-Json)
$extra=@($newHashes|Where-Object File -CEQ 'debug_gap_probe.cpp');$base=@($newHashes|Where-Object File -CNE 'debug_gap_probe.cpp')
if($extra.Count -ne 1 -or ($oldHashes|Sort-Object File|ConvertTo-Json -Compress) -cne ($base|Sort-Object File|ConvertTo-Json -Compress)){throw 'Unexpected source changes'}
$cmd=[IO.File]::ReadAllText((Resolve-Path "$ProbeDirectory\build.cmd"));$cmd=[regex]::Replace($cmd,'(?m)^cl [^\r\n]* /c debug_gap_probe\.cpp\r?\nif errorlevel 1 exit 3\r?\n','');$cmd=$cmd.Replace('/INCLUDE:_fsx_debug_gap_probe debug_gap_probe.obj ','')
if($cmd -cne [IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"))){throw 'Additional command changes beyond probe compile/link'}
. "$PSScriptRoot\lib\pe-image.ps1"
$a=Open-PE "$ControlDirectory\fsx.exe";$b=Open-PE "$ProbeDirectory\fsx.exe";$original=Open-PE "$PSScriptRoot\..\original\fsx.exe"
function Symbols([string]$path,$pe){$result=@{};foreach($m in [regex]::Matches((Get-Content $path -Raw),'(?m)^\s*\S+\s+(\S+)\s+([0-9a-fA-F]{8})\b')){$va=[Convert]::ToInt64($m.Groups[2].Value,16);if($va -ge $pe.Base -and $va -lt $pe.Base+0x10000){$result[$m.Groups[1].Value]=$va}};return $result}
$cs=Symbols "$ControlDirectory\fsx.map" $a;$ps=Symbols "$ProbeDirectory\fsx.map" $b;$differences=@();$shifted=0;$unchanged=0
foreach($name in $cs.Keys){if(!$ps.ContainsKey($name)){throw 'Missing probe symbol'};$expected=if($cs[$name] -ge 0x01001538 -and $cs[$name] -lt 0x01004000){$cs[$name]+32}else{$cs[$name]};if($ps[$name] -ne $expected){$differences+=@{Symbol=$name;Expected=('{0:X8}' -f $expected);Actual=('{0:X8}' -f $ps[$name])}}elseif($expected -eq $cs[$name]){$unchanged++}else{$shifted++}}
$extraSymbols=@($ps.Keys|Where-Object {!$cs.ContainsKey($_)})
if($extraSymbols.Count -ne 1 -or $extraSymbols[0] -cne '_fsx_debug_gap_probe'){throw 'Unexpected added image symbols'}
$rows=foreach($p in @(@('Original',$original),@('Control',$a),@('Probe',$b))){$pe=$p[1];$seh=U32 $pe ($pe.Base+0x14d0+64);$start=$pe.Base+0x1538;$length=$seh-$start;if(@(Bytes $pe $start $length|Where-Object {$_ -ne 0}).Count){throw 'Observed gap not all zero'};[pscustomobject]@{Role=$p[0];SafeSehVA=('{0:X8}' -f $seh);GapStart='01001538';GapBytes=$length;GapAllZero=$true}}
[pscustomobject]@{ControlSHA256=$control.RebuiltSHA256;ProbeSHA256=$probe.RebuiltSHA256;OriginalSourcesUnchanged=$true;AddedDiagnosticSource=$extra[0];CommonSymbols=$cs.Count;ShiftedBy32Symbols=$shifted;UnchangedSymbols=$unchanged;UnexpectedSymbolDifferences=$differences;AdditionalSymbols=$extraSymbols;AllCommonSymbolsHaveExpectedPlacement=($differences.Count -eq 0);Gaps=@($rows);OriginalSections=$pp.OriginalHeader.Sections;ProbeSections=$pp.RebuiltHeader.Sections;ProbeEntryDelta=$pp.RebuiltHeader.EntryRVA-$pp.OriginalHeader.EntryRVA;ImportContentsAndOrderMatch=$pp.ImportContentsAndOrderMatch;Status='Diagnostic only; not selected as golden default';Scope='Extra explicitly synthetic32-byte object in .rdata$r, with all original C++ input hashes preserved and additional compile/include/object commands identified. Tests layout effect, not original source variable, original linker/release cause or whole binary identity.'}|ConvertTo-Json -Depth 9
