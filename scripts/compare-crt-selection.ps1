param([Parameter(Mandatory)][string]$BuildDirectory,
 [string]$ControlDirectory="$PSScriptRoot\..\vm-share\modules\fsx\runs\order-mfc-thunks-20261103\candidates\order-mfc-thunks-20261103-001")
$ErrorActionPreference='Stop'
$old=@(Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json)
$new=@(Get-Content "$BuildDirectory\source-hashes.json" -Raw|ConvertFrom-Json)
if(($old|Sort-Object File|ConvertTo-Json -Compress) -cne ($new|Sort-Object File|ConvertTo-Json -Compress)){throw 'Compiled input hashes changed'}
$before=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"))
$after=[IO.File]::ReadAllText((Resolve-Path "$BuildDirectory\build.cmd"))
if(([regex]::Matches($after,'/INCLUDE:_WinMainCRTStartup ')).Count -ne 1 -or $after.Replace('/INCLUDE:_WinMainCRTStartup ','') -cne $before){throw 'Expected exactly the entry INCLUDE addition'}
$c=Get-Content "$BuildDirectory\comparison.json" -Raw|ConvertFrom-Json
$control=Get-Content "$ControlDirectory\comparison.json" -Raw|ConvertFrom-Json
$order=(& "$PSScriptRoot\verify-function-order.ps1" -BuildDirectory $BuildDirectory -OrderFile "$BuildDirectory\source\function-order-input.h"|Out-String)|ConvertFrom-Json
if($order.Count -ne 88){throw 'Expected 88 reviewed ordered symbols'}
if(($order.Functions|ConvertTo-Json -Compress) -cne ($control.FunctionOrder.Functions|ConvertTo-Json -Compress)){throw 'Ordered prefix placement changed'}
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
$pairs=@(
 @('_pre_cpp_init',0x01002313L),@('___tmainCRTStartup',0x01002363L),@('_pre_c_init',0x01002547L),
 @('_WinMainCRTStartup',0x0100262eL),@('@__security_check_cookie@4',0x0100263dL),@('__onexit',0x01002651L),@('_atexit',0x010026f5L),
 @('__SEH_prolog4_GS',0x0100270cL),@('__SEH_epilog4_GS',0x01002754L),@('__except_handler4',0x01002768L),
 @('__alloca_probe_16',0x01002790L),@('__alloca_probe_8',0x010027a6L),@('__amsg_exit',0x010027ceL),
 @('__RTC_Initialize',0x010027d9L),@('__RTC_Terminate',0x01002802L),@('__XcptFilter',0x0100282cL),
 @('__ValidateImageBase',0x01002840L),@('__FindPESection',0x01002870L),@('__IsNonwritableInCurrentImage',0x010028b7L),
 @('__initterm',0x01002928L),@('__initterm_e',0x01002934L),@('__SEH_prolog4',0x01002940L),@('__SEH_epilog4',0x01002985L),
 @('__setdefaultprecision',0x0100299eL),@('__matherr',0x010029ccL),@('__setargv',0x010029ccL),
 @('___security_init_cookie',0x010029d4L),@('___report_gsfailure',0x01002a6dL),
 @('__alloca_probe',0x01002bb0L),@('__chkstk',0x01002bb0L),@('__invoke_watson',0x01002be0L),@('__controlfp_s',0x01002becL),
 @('_WinMain@16',0x01002c03L),@('?AfxInitialize@@YGHHK@Z',0x01002c0dL),@('_memset',0x01002c44L),@('__allmul',0x01002c50L))
$rows=foreach($p in $pairs){$m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($p[0])+'\s+([0-9a-fA-F]{8})\b');if(!$m.Success){throw "Missing CRT symbol $($p[0])"};$va=[Convert]::ToInt64($m.Groups[1].Value,16);if($va-$p[1] -ne -32){throw "Unexpected CRT placement $($p[0])"};[pscustomobject]@{Symbol=$p[0];OriginalVA=('{0:X8}' -f $p[1]);RebuiltVA=('{0:X8}' -f $va);Delta=$va-$p[1]}}
$slots=@{};foreach($e in $c.PeImports.OriginalImportEntries){$slots[$e.Identity]=$e.Index}
if(!$c.PeImports.ImportContentsMatch){throw 'Import contents gate failed'}
[pscustomobject]@{RebuiltSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;ControlSHA256=(Get-FileHash "$ControlDirectory\fsx.exe").Hash;CompiledInputsIdentical=$true;OnlyBuildCommandChange='/INCLUDE:_WinMainCRTStartup';OrderedPrefixSymbolsUnchanged=88;AdditionalMappedSymbols=$rows.Count;UniformDelta=-32;Functions=@($rows);ImportSlotDifferences=@($c.PeImports.ImportEntries|Where-Object {$slots[$_.Identity] -ne $_.Index}).Count;Scope='Controlled CRT selection and entry placement using previously independently mapped body/reference identities and actual MAP provenance. Existing full comparison gates required separately. Does not certify all unlisted CRT thunks, padding, import order, original flags, source or whole EXE identity.'}|ConvertTo-Json -Depth 6
