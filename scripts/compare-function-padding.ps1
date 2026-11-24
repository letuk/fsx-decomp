param([Parameter(Mandatory)][string]$ControlDirectory,[Parameter(Mandatory)][string]$PaddedDirectory)
$ErrorActionPreference='Stop'
$controlGate=(& "$PSScriptRoot\compare-runtime-remainders.ps1" -BuildDirectory $ControlDirectory|Out-String)|ConvertFrom-Json
$paddedGate=(& "$PSScriptRoot\compare-runtime-remainders.ps1" -BuildDirectory $PaddedDirectory|Out-String)|ConvertFrom-Json
. "$PSScriptRoot\lib\pe-image.ps1"
$original=Open-PE "$PSScriptRoot\..\original\fsx.exe";$control=Open-PE "$ControlDirectory\fsx.exe";$padded=Open-PE "$PaddedDirectory\fsx.exe"
$cm=Get-Content "$ControlDirectory\fsx.map" -Raw;$pm=Get-Content "$PaddedDirectory\fsx.map" -Raw
function Map-VA([string]$map,[string]$name){$m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($name)+'\s+([0-9a-fA-F]{8})\b');if(!$m.Success){throw "Missing symbol $name"};[Convert]::ToInt64($m.Groups[1].Value,16)}
function Gap($pe,[long]$entry,[int]$length,[long]$next){$gap=$next-$entry-$length;if($gap -lt 0 -or $gap -gt 64){return $null};if($gap -and @(Bytes $pe ($entry+$length) $gap|Where-Object {$_ -ne 0xcc}).Count){return $null};return $gap}
$samples=@(
 @('ATL fallback to real',0x01001f1f,101,0x01001f89,'?_AtlGetThreadACPFake@ATL@@YGIXZ','?_AtlGetThreadACPReal@ATL@@YGIXZ'),
 @('ATL real to thunk',0x01001f89,4,0x01001f92,'?_AtlGetThreadACPReal@ATL@@YGIXZ','?_AtlGetThreadACPThunk@ATL@@YGIXZ'),
 @('pre_cpp_init to CRT main',0x01002313,75,0x01002363,'_pre_cpp_init','___tmainCRTStartup'),
 @('CRT main region to pre_c_init',0x01002363,479,0x01002547,'___tmainCRTStartup','_pre_c_init'),
 @('pre_c_init to entry',0x01002547,226,0x0100262e,'_pre_c_init','_WinMainCRTStartup'),
 @('Entry to cookie checker',0x0100262e,10,0x0100263d,'_WinMainCRTStartup','@__security_check_cookie@4'),
 @('Cookie checker to onexit',0x0100263d,15,0x01002651,'@__security_check_cookie@4','__onexit'),
 @('Onexit region to atexit',0x01002651,159,0x010026f5,'__onexit','_atexit'),
 @('atexit to GS prologue',0x010026f5,18,0x0100270c,'_atexit','__SEH_prolog4_GS'),
 @('GS prologue to epilogue',0x0100270c,72,0x01002754,'__SEH_prolog4_GS','__SEH_epilog4_GS'),
 @('GS epilogue to EH4 dispatcher',0x01002754,15,0x01002768,'__SEH_epilog4_GS','__except_handler4'),
 @('RTC init to term',0x010027d9,36,0x01002802,'__RTC_Initialize','__RTC_Terminate'),
 @('PE validation to section finder',0x01002840,41,0x01002870,'__ValidateImageBase','__FindPESection'),
 @('Precision region to zero stub',0x0100299e,41,0x010029cc,'__setdefaultprecision','__matherr'),
 @('Zero stub to cookie init',0x010029cc,3,0x010029d4,'__matherr','___security_init_cookie'),
 @('Cookie init to GS failure',0x010029d4,148,0x01002a6d,'___security_init_cookie','___report_gsfailure')
)
$rows=foreach($s in $samples){$cg=Gap $control (Map-VA $cm $s[4]) $s[2] (Map-VA $cm $s[5]);$pg=Gap $padded (Map-VA $pm $s[4]) $s[2] (Map-VA $pm $s[5]);$og=Gap $original $s[1] $s[2] $s[3];[pscustomobject]@{Name=$s[0];OriginalGap=$og;ControlGap=$cg;PaddedGap=$pg;ControlMatches=($cg -eq $og);PaddedMatches=($pg -eq $og);MeasuredGapBytesCC=$true;SequenceNotAdjacent=($null -eq $cg -or $null -eq $pg)}}
$cc=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"));$pc=[IO.File]::ReadAllText((Resolve-Path "$PaddedDirectory\build.cmd"))
if($cc -cne $pc.Replace('/FUNCTIONPADMIN:5 ','')){throw 'Build commands differ beyond padding option'}
$cs=Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json;$ps=Get-Content "$PaddedDirectory\source-hashes.json" -Raw|ConvertFrom-Json
if(($cs|Sort-Object File|ConvertTo-Json -Compress) -cne ($ps|Sort-Object File|ConvertTo-Json -Compress)){throw 'Source inputs differ'}
[pscustomobject]@{OriginalSHA256=$controlGate.OriginalSHA256;ControlSHA256=$controlGate.RebuiltSHA256;PaddedSHA256=$paddedGate.RebuiltSHA256;IdenticalSourceFiles=$true;OnlyBuildCommandChange='/FUNCTIONPADMIN:5';Samples=$rows;ControlMatchingGaps=@($rows|Where-Object ControlMatches).Count;PaddedMatchingGaps=@($rows|Where-Object PaddedMatches).Count;ComparableSamples=@($rows|Where-Object {!$_.SequenceNotAdjacent}).Count;SampleCount=$rows.Count;Scope='Selected target-verified runtime regions and observed CC gap samples. Null gaps indicate nonadjacency/order/intervening code, not a measured CC gap. This does not establish whole image layout, full source identity or the original linker command.'}|ConvertTo-Json -Depth 7
