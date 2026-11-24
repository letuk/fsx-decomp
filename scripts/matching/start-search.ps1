param(
 [string]$RunId = ('show-error-' + (Get-Date -Format 'yyyyMMdd-HHmmss')),
 [int]$MaxCandidates = 24,
 [int]$JobTimeoutSeconds = 900,
 [switch]$PrepareOnly,
 [switch]$ReviewedSource,
 [switch]$AfxInitializeProbe,
 [switch]$ToolchainDiagnostics,
 [switch]$WritableDataDiagnostics,
 [ValidateSet('InstalledSP1','PinnedSP1','RTM')][string]$CompilerToolset = 'InstalledSP1',
 [switch]$MergeBssIntoData,
 [ValidateSet('Auto','InstalledSP1','RTM')][string]$CrtLibrary = 'Auto',
 [string]$SourceDirectory = "$PSScriptRoot\..\..\src\fsx",
 [ValidateSet('NOICF','ICF')][string]$LinkFolding = 'ICF',
 [ValidateSet(0,5)][int]$FunctionPadding = 5,
 [ValidateSet('Enabled','Disabled')][string]$Rtti = 'Disabled',
 [ValidateSet('Default','Original')][string]$ImageBaseMode = 'Original',
 [ValidateSet('Separate','RdataIntoText')][string]$SectionLayout = 'RdataIntoText',
 [ValidateSet('Default','Original')][string]$StackMode = 'Original',
 [ValidateSet('Worker','Basename')][string]$PdbPathMode = 'Basename',
 [ValidateSet('Default','LargeAddress','LargeAddressNx','Original')][string]$HeaderFlagsMode = 'Original',
 [ValidateSet('Default','Original')][string]$ImageVersionMode = 'Original',
 [ValidateSet('Default','Original')][string]$OsVersionMode = 'Original',
 [ValidateSet(0,32)][int]$DebugGapProbeBytes = 0,
 [ValidateSet('Auto','Default','FirstThree','ApplicationPrefix','ApplicationAll','MfcThunks','CrtStartupProbe')][string]$FunctionOrder = 'Auto',
 [ValidateSet('Auto','Static','External')][string]$CallbackLinkage = 'Auto',
 [ValidateSet('Auto','Default','EntryInclude')][string]$CrtStartupSelection = 'Auto',
 [ValidateSet('Default','OriginalIncludes','CrtDirectIncludes','IatOrder')][string]$ImportSelection = 'Default',
 [ValidateSet('Default','AtlCacheInclude','AtlLibraryFirst','AtlObjectFirst','AtlObjectMfcNolib','AtlCookieObjectsFirst','AtlObjectCrtLibraryFirst','AtlObjectSplitMfcBss','AtlObjectSplitMfcBssTail')][string]$WritableDataSelection = 'Default',
 [ValidateSet('Auto','InstalledSP1','RTM')][string]$MfcSupportLibrary = 'Auto',
 [ValidateSet('ConsolePilot','MfcWindows')][string]$BuildProfile = 'MfcWindows',
 [ValidateSet('NonGL','GL','SameTU')][string]$GetterCompilation = 'GL',
 [ValidateSet('GL','NonGL')][string]$ContextCompilation = 'GL',
 [ValidateSet('ShowErrorMessage','InitInstance','AppLifecycle','StaticInitialization')][string]$Target = 'ShowErrorMessage',
 [string]$ShareRoot = "$PSScriptRoot\..\..\vm-share"
)
$ErrorActionPreference='Stop'
if($RunId -notmatch '^[A-Za-z0-9_-]+$'){throw 'Invalid run ID'}
if($MaxCandidates -lt 1 -or $MaxCandidates -gt 24){throw 'Candidate limit must be 1..24'}
if($Target -ne 'ShowErrorMessage' -and $MaxCandidates -ne 1){throw 'This target currently supports one reviewed source proposal per run'}
if($Target -eq 'StaticInitialization' -and $BuildProfile -ne 'MfcWindows'){throw 'StaticInitialization target requires the MfcWindows profile'}
if($AfxInitializeProbe -and ($BuildProfile -ne 'MfcWindows' -or $GetterCompilation -eq 'SameTU')){throw 'AfxInitialize probe requires the MfcWindows profile and a separate getter translation unit'}
if($ContextCompilation -eq 'NonGL' -and $BuildProfile -ne 'MfcWindows'){throw 'Native context diagnostic is reviewed only for MfcWindows'}
if($DebugGapProbeBytes -and ($BuildProfile -ne 'MfcWindows' -or $SectionLayout -ne 'RdataIntoText')){throw 'Debug-gap probe requires the reviewed merged MfcWindows layout'}
if($FunctionOrder -eq 'Auto'){$FunctionOrder=if($BuildProfile -eq 'MfcWindows'){'MfcThunks'}else{'Default'}}
if($CallbackLinkage -eq 'Auto'){$CallbackLinkage=if($FunctionOrder -in 'ApplicationAll','MfcThunks','CrtStartupProbe'){'External'}else{'Static'}}
if($FunctionOrder -ne 'Default' -and $BuildProfile -ne 'MfcWindows'){throw 'Reviewed application order requires MfcWindows; use Default for console control'}
if($CrtLibrary -eq 'Auto'){$CrtLibrary=if($BuildProfile -eq 'MfcWindows'){'RTM'}else{'InstalledSP1'}}
if($MfcSupportLibrary -eq 'Auto'){$MfcSupportLibrary=if($BuildProfile -eq 'MfcWindows' -and $CrtLibrary -eq 'RTM'){'RTM'}else{'InstalledSP1'}}
if($CrtStartupSelection -eq 'Auto'){$CrtStartupSelection=if($BuildProfile -eq 'MfcWindows' -and $CrtLibrary -eq 'RTM'){'EntryInclude'}else{'Default'}}
if($CrtStartupSelection -ne 'Default' -and ($BuildProfile -ne 'MfcWindows' -or $CrtLibrary -ne 'RTM')){throw 'CRT selection probe requires pinned RTM MfcWindows'}
if($ImportSelection -ne 'Default' -and ($BuildProfile -ne 'MfcWindows' -or $CrtLibrary -ne 'RTM')){throw 'Import selection probe requires pinned RTM MfcWindows'}
if($WritableDataSelection -ne 'Default' -and ($BuildProfile -ne 'MfcWindows' -or $CrtLibrary -ne 'RTM')){throw 'Writable-data selection probe requires pinned RTM MfcWindows'}
if($MfcSupportLibrary -eq 'RTM' -and ($BuildProfile -ne 'MfcWindows' -or $CrtLibrary -ne 'RTM')){throw 'MFC support-library probe requires pinned RTM MfcWindows'}
if($MergeBssIntoData -and $BuildProfile -ne 'MfcWindows'){throw 'BSS merge probe requires MfcWindows'}
$project=[IO.Path]::GetFullPath("$PSScriptRoot\..\..")
$compilerVersion=if($CompilerToolset -eq 'RTM'){'14.00.50727.42'}else{'14.00.50727.762'}
$share=[IO.Path]::GetFullPath($ShareRoot)
$run=Join-Path $share "modules\fsx\runs\$RunId"
$queue=Join-Path $share 'queue'
New-Item -ItemType Directory -Path "$share\modules\fsx\runs" -Force|Out-Null
try{$controllerLock=[IO.File]::Open("$share\controller.lock",[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)}catch{throw 'Another search controller is already running for this shared folder'}
if(Test-Path -LiteralPath $run){throw 'Run already exists; use a new ID. Existing results will not be overwritten.'}
if((Get-FileHash "$project\original\fsx.exe").Hash -ne '61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735'){throw 'Original binary identity changed'}
New-Item -ItemType Directory -Path "$run\baseline-source",$queue -Force|Out-Null
Get-ChildItem -LiteralPath $SourceDirectory -File|Where-Object {$_.Extension -in '.cpp','.h'}|Copy-Item -Destination "$run\baseline-source"
$originalSource=[IO.File]::ReadAllText("$run\baseline-source\show_error_message.cpp")
$legacyTemplate=$originalSource.Contains('int formatStatus;') -and $originalSource.Contains('if (formatStatus >= 0) {')
if(!$legacyTemplate -or $ReviewedSource -or $Target -ne 'ShowErrorMessage' -or $MaxCandidates -eq 1){
 $plans=@(@{Type='reviewed';Initialize=$null;Condition='as supplied';Source=$originalSource;SourceSHA256=(Get-FileHash "$run\baseline-source\show_error_message.cpp").Hash})
}else{
 $plans=@();$seen=@{}
 foreach($initialize in @($false,$true)){
  foreach($type in @('int','HRESULT','long')){
   foreach($condition in @('formatStatus >= 0','formatStatus > -1','!(formatStatus < 0)','(static_cast<unsigned long>(formatStatus) & 0x80000000UL) == 0')){
    $decl=$type+' formatStatus'+$(if($initialize){' = 0'}else{''})+';'
    $source=$originalSource.Replace('int formatStatus;',$decl).Replace('if (formatStatus >= 0) {','if ('+$condition+') {')
    $sha=[Security.Cryptography.SHA256]::Create();try{$hash=[BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes($source))).Replace('-','')}finally{$sha.Dispose()}
    if(!$seen.ContainsKey($hash)){$seen[$hash]=$true;$plans+=@{Type=$type;Initialize=$initialize;Condition=$condition;Source=$source;SourceSHA256=$hash}}
   }
  }
 }
}
$plans=@($plans|Select-Object -First $MaxCandidates)
$state=[ordered]@{RunId=$RunId;Target=$Target;Status='prepared';CandidateLimit=$plans.Count;Completed=0;BestDifferences=$null;BestJob=$null;CurrentJob=$null;Reason=$null;Results=@();StartedUtc=[DateTime]::UtcNow.ToString('o')}
function Save-State {
 $state.UpdatedUtc=[DateTime]::UtcNow.ToString('o')
 $json=$state|ConvertTo-Json -Depth 9
 [IO.File]::WriteAllText("$run\status.json.tmp",$json,[Text.UTF8Encoding]::new($false))
 Move-Item -LiteralPath "$run\status.json.tmp" -Destination "$run\status.json" -Force
 $state.Results|Select-Object Job,Type,Initialize,Condition,Status,Differences|Export-Csv "$run\results.csv" -NoTypeInformation
 $report=@('# Matching search', '', "Run: $RunId", "Target: $Target", "Status: $($state.Status)", "Completed: $($state.Completed) / $($state.CandidateLimit)", "Best verified differing bytes: $($state.BestDifferences)", "Best candidate: $($state.BestJob)", "Stop reason: $($state.Reason)", '', 'InitInstance whole-body results are in initinstance-body.json when the instruction layout matches.', 'This run never executes candidate EXEs or modifies canonical source/Ghidra.', '', 'See results.csv and candidates/<job>/comparison.json for evidence.')
 [IO.File]::WriteAllLines("$run\REPORT.md",$report,[Text.UTF8Encoding]::new($false))
}
function Stop-Run([string]$Status,[string]$Reason){$state.Status=$Status;$state.Reason=$Reason;$state.CurrentJob=$null;Save-State;Write-Output "$Status : $Reason"}
function Publish-Job($plan,[int]$index){
 $id=$RunId+'-'+('{0:D3}' -f $index)
 $job=Join-Path $queue $id
 if(Test-Path -LiteralPath $job){throw 'Job ID collision'}
 New-Item -ItemType Directory -Path "$job\source","$run\candidates\$id" -Force|Out-Null
 Get-ChildItem "$run\baseline-source" -File|Copy-Item -Destination "$job\source"
 if($WritableDataSelection -eq 'AtlObjectMfcNolib'){
  foreach($reserved in 'stdafx.h','mfc_nolib.cpp'){
   if(Test-Path "$job\source\$reserved"){throw "MFC nolib compilation would overwrite staged input $reserved"}
  }
  [IO.File]::WriteAllText("$job\source\stdafx.h", "#include <windows.h>`r`n")
 }
 if($CallbackLinkage -eq 'External'){
  $callbackFile="$job\source\startup_context_pilot.cpp"
  $callbackText=[IO.File]::ReadAllText($callbackFile)
  $callbackNeedle='static MSG* WINAPI GetCurrentMessagePilot()'
  if(([regex]::Matches($callbackText,[regex]::Escape($callbackNeedle))).Count -ne 1){throw 'Reviewed static callback definition changed'}
  [IO.File]::WriteAllText($callbackFile,$callbackText.Replace($callbackNeedle,'MSG* WINAPI GetCurrentMessagePilot()'),[Text.UTF8Encoding]::new($false))
 }
 [IO.File]::WriteAllText("$job\source\show_error_message.cpp",$plan.Source,[Text.UTF8Encoding]::new($false))
 if($FunctionOrder -ne 'Default'){
  $order=@('?StringCchPrintfA@@YAJPADIPBDZZ','?StringCopyWorkerA@@YGJPADIPBD@Z','?GetMessageMap@FSXAppPilot@@MBEPBUAFX_MSGMAP@@XZ')
  if($FunctionOrder -in 'ApplicationPrefix','ApplicationAll','MfcThunks','CrtStartupProbe'){
   $order+=@('??0FSXAppPilot@@QAE@XZ','??_GFSXAppPilot@@UAEPAXI@Z','?ExitInstance@FSXAppPilot@@UAEHXZ','?CheckWindows51SP2OrLater@@YGHXZ','?CheckWindows52SP1OrLater@@YGHXZ','??_GFSXMessageFilterPilot@@UAEPAXI@Z','?OnMessagePending@FSXMessageFilterPilot@@UAEHPBUtagMSG@@@Z','?InitializeOleAndMessageFilterPilot@@YAHXZ','?InitInstance@FSXAppPilot@@UAEHXZ')
  }
  if($FunctionOrder -in 'ApplicationAll','MfcThunks','CrtStartupProbe'){
   $order+=@('?GetCurrentMessagePilot@@YGPAUtagMSG@@XZ','?Run@FSXAppPilot@@UAEHXZ','ShowErrorMessage@24','?Serialize@CObject@@UAEXAAVCArchive@@@Z','?Dump@CObject@@UBEXAAVCDumpContext@@@Z','?AssertValid@CObject@@UBEXXZ','??3CObject@@SGXPAX@Z','?_AtlGetThreadACPFake@ATL@@YGIXZ','?_AtlGetThreadACPReal@ATL@@YGIXZ','?_AtlGetThreadACPThunk@ATL@@YGIXZ')
  }
  if($FunctionOrder -in 'MfcThunks','CrtStartupProbe'){
   $thunkOrder=@(Get-Content "$PSScriptRoot\mfc-thunk-order.json" -Raw|ConvertFrom-Json)
   if($thunkOrder.Count -ne 66){throw 'Reviewed MFC thunk manifest changed'}
   $order+=@($thunkOrder|ForEach-Object Symbol)
  }
  if($FunctionOrder -eq 'CrtStartupProbe'){
   # Negative/eligibility probe: file-local CRT names are not assumed orderable.
   $order+=@('pre_cpp_init','__tmainCRTStartup','pre_c_init','WinMainCRTStartup','@__security_check_cookie@4','_onexit','atexit')
  }
  # Response data, not a C++ include. .h retains existing worker copy/hash verification.
  [IO.File]::WriteAllLines("$job\source\function-order-input.h",$order,[Text.Encoding]::ASCII)
 }
 if($ImportSelection -eq 'IatOrder'){
  Copy-Item -LiteralPath "$PSScriptRoot\fsx-iat-order.rsp" -Destination "$job\source\import-selection-input.h"
 }elseif($ImportSelection -ne 'Default'){
  $imports=@(Get-Content "$PSScriptRoot\import-selection.json" -Raw|ConvertFrom-Json)
  if($imports.Count -ne 137){throw 'Reviewed import manifest changed'}
  if($ImportSelection -eq 'CrtDirectIncludes'){
   $imports=@($imports|Where-Object {$_.Identity -cin @('msvcr80.dll!_splitpath_s','msvcr80.dll!_vsnprintf','msvcr80.dll!_setmbcp')})
   if($imports.Count -ne 3){throw 'Expected three reviewed direct CRT imports'}
  }
  [IO.File]::WriteAllLines("$job\source\import-selection-input.h",@($imports|ForEach-Object {'/INCLUDE:'+$_.Symbol}),[Text.Encoding]::ASCII)
 }
 if($DebugGapProbeBytes -eq 32){
  $probe='// Diagnostic layout probe only; NOT recovered original source or variable.'+"`r`n"+'#pragma section(".rdata$r", read)'+"`r`n"+'extern "C" { __declspec(allocate(".rdata$r")) extern const unsigned char fsx_debug_gap_probe[32] = {0}; }'+"`r`n"
  [IO.File]::WriteAllText("$job\source\debug_gap_probe.cpp",$probe,[Text.UTF8Encoding]::new($false))
 }
 Get-ChildItem "$job\source" -File|Get-FileHash -Algorithm SHA256|Select-Object @{Name='File';Expression={[IO.Path]::GetFileName($_.Path)}},Hash|ConvertTo-Json|Set-Content "$job\source-hashes.json"
 $buildTemplate=if($BuildProfile -eq 'MfcWindows'){'guest-build-mfc.cmd'}else{'guest-build.cmd'}
 $buildCommand=[IO.File]::ReadAllText((Join-Path $PSScriptRoot $buildTemplate))
 if($FunctionOrder -ne 'Default'){
  $buildCommand=$buildCommand.Replace('link /nologo ', 'link /nologo /ORDER:@function-order-input.h ')
 }
 if($DebugGapProbeBytes -eq 32){
  $buildCommand=$buildCommand.Replace('link /nologo ',("cl /nologo /W4 /O2 /Zl /GR- /c debug_gap_probe.cpp`r`nif errorlevel 1 exit 3`r`nlink /nologo /INCLUDE:_fsx_debug_gap_probe debug_gap_probe.obj "))
 }
 if($Rtti -eq 'Disabled'){$buildCommand=$buildCommand.Replace('cl /nologo ', 'cl /nologo /GR- ')}
 if($GetterCompilation -in 'GL','NonGL'){
  $needle='/Zi(?: /GL)? /c application_interface_pilot\.cpp'
  if(([regex]::Matches($buildCommand,$needle)).Count -ne 1){throw 'Getter compile template changed'}
  $replacement=if($GetterCompilation -eq 'GL'){'/Zi /GL /c application_interface_pilot.cpp'}else{'/Zi /c application_interface_pilot.cpp'}
  $buildCommand=[regex]::Replace($buildCommand,$needle,$replacement)
 }elseif($GetterCompilation -eq 'SameTU'){
  if(!(Get-Content "$job\source\startup_context_pilot.cpp" -Raw).Contains('#include "application_interface_pilot.cpp"')){throw 'SameTU proposal must include the getter definition'}
  $buildCommand=[regex]::Replace($buildCommand,'(?m)^cl [^\r\n]* /c application_interface_pilot\.cpp\r?\n','')
  $buildCommand=$buildCommand.Replace(' application_interface_pilot.obj','')
 }
 if($ContextCompilation -eq 'NonGL'){
  $pattern='(?m)^(cl [^\r\n]*) /GL( [^\r\n]* /c startup_context_pilot\.cpp)'
  if(([regex]::Matches($buildCommand,$pattern)).Count -ne 1){throw 'Context GL compile template changed'}
  $buildCommand=[regex]::Replace($buildCommand,$pattern,'$1 /GL-$2')
 }
 if(([regex]::Matches($buildCommand,'/OPT:(NOICF|ICF)\b')).Count -ne 1){throw 'Canonical folding option changed; review experiment template'}
 $buildCommand=[regex]::Replace($buildCommand,'/OPT:(NOICF|ICF)\b',('/OPT:'+$LinkFolding))
 if($FunctionPadding -eq 5){$buildCommand=$buildCommand.Replace('link /nologo ', 'link /nologo /FUNCTIONPADMIN:5 ')}
 if($ImageBaseMode -eq 'Original'){$buildCommand=$buildCommand.Replace('link /nologo ', 'link /nologo /BASE:0x01000000 ')}
 if($SectionLayout -eq 'RdataIntoText'){$buildCommand=$buildCommand.Replace('link /nologo ', 'link /nologo /MERGE:.rdata=.text ')}
 if($StackMode -eq 'Original'){$buildCommand=$buildCommand.Replace('link /nologo ', 'link /nologo /STACK:0x40000,0x2000 ')}
 if($PdbPathMode -eq 'Basename'){$buildCommand=$buildCommand.Replace('link /nologo ', 'link /nologo /PDBALTPATH:fsx.pdb ')}
 $headerOptions=@()
 if($HeaderFlagsMode -ne 'Default'){$headerOptions+='/LARGEADDRESSAWARE'}
 if($HeaderFlagsMode -in 'LargeAddressNx','Original'){$headerOptions+='/NXCOMPAT'}
 if($HeaderFlagsMode -eq 'Original'){$headerOptions+='/TSAWARE'}
 if($ImageVersionMode -eq 'Original'){$headerOptions+='/VERSION:5.2'}
 if($OsVersionMode -eq 'Original'){$headerOptions+='/OSVERSION:5.2'}
 if($headerOptions.Count){$buildCommand=$buildCommand.Replace('link /nologo ',('link /nologo '+($headerOptions -join ' ')+' '))}
 $buildCommand=$buildCommand.Replace('echo Matching search: SP1, pinned baseline flags',"echo Matching search: SP1, /OPT:$LinkFolding`r`ntype build.cmd")
 $crtLibraryHash=$null
 if($CrtLibrary -eq 'RTM'){
  $crtPath=Join-Path $share 'toolchains\vc80-rtm\msvcrt.lib'
  $crtLibraryHash=(Get-FileHash -LiteralPath $crtPath).Hash
  if($crtLibraryHash -ne 'B1456D9A7E17865E3DC35663E0377BE4C7B8BE97A372210CD064F9F5635DFFEA'){throw 'Pinned RTM CRT library identity differs'}
  # Keep the established MFC allocation overrides ahead of the explicit CRT.
  $buildCommand=$buildCommand.Replace('link /nologo ', 'link /nologo mfc80.lib mfcs80.lib "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\msvcrt.lib" ')
 }
 if($CrtStartupSelection -eq 'EntryInclude'){
  $buildCommand=$buildCommand.Replace('link /nologo ','link /nologo /INCLUDE:_WinMainCRTStartup ')
 }
 if($ImportSelection -ne 'Default'){
  $buildCommand=$buildCommand.Replace('link /nologo ','link /nologo @import-selection-input.h ')
 }
 if($WritableDataSelection -eq 'AtlCacheInclude'){
  $buildCommand=$buildCommand.Replace('link /nologo ','link /nologo /INCLUDE:?g_pfnGetThreadACP@ATL@@3P6GIXZA ')
 }
 if($WritableDataSelection -eq 'AtlLibraryFirst'){
  $needle='mfc80.lib mfcs80.lib "'
  if(([regex]::Matches($buildCommand,[regex]::Escape($needle))).Count -ne 1){throw 'Pinned library-order template changed'}
  $buildCommand=$buildCommand.Replace($needle,'mfc80.lib mfcs80.lib atls.lib "')
 }
 if($WritableDataSelection -in 'AtlObjectFirst','AtlObjectMfcNolib','AtlCookieObjectsFirst','AtlObjectCrtLibraryFirst','AtlObjectSplitMfcBss','AtlObjectSplitMfcBssTail'){
  $extractions=@(
   'lib /nologo /extract:"INTEL\Release_1\atltrace.obj" /out:writable-atltrace.obj "%VCINSTALLDIR%\atlmfc\lib\atls.lib"',
   'if errorlevel 1 exit 6',
   'if exist "%VCINSTALLDIR%\atlmfc\lib\atls.pdb" copy /Y "%VCINSTALLDIR%\atlmfc\lib\atls.pdb" atls.pdb',
   'if errorlevel 1 exit 6'
  )
  $orderedObjects='writable-atltrace.obj'
  if($WritableDataSelection -eq 'AtlCookieObjectsFirst'){
   $extractions+=@(
    'lib /nologo /extract:"build\intel\dll_obj\gs_cookie.obj" /out:writable-gs-cookie.obj "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\msvcrt.lib"',
    'if errorlevel 1 exit 6'
   )
   $orderedObjects+=' writable-gs-cookie.obj'
  }
  if($WritableDataSelection -like 'AtlObjectSplitMfcBss*'){
   if($MfcSupportLibrary -ne 'RTM'){throw 'Split MFC BSS trial requires pinned RTM MFC support'}
   $extractions+=@(
    'lib /nologo /extract:"INTEL\$DLL.W\nolib.obj" /out:writable-nolib.obj "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\mfcs80.lib"',
    'if errorlevel 1 exit 6',
    'copy /Y "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\mfcs80.pdb" mfcs80.pdb',
    'if errorlevel 1 exit 6'
   )
   $orderedObjects+=' writable-nolib.obj'
   foreach($member in 'crtexew','wildcard','_newmode','xncommod','xtxtmode'){
    $extractions+='lib /nologo /extract:"build\intel\dll_obj\'+$member+'.obj" /out:writable-'+$member+'.obj "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\msvcrt.lib"'
    $extractions+='if errorlevel 1 exit 6'
    $orderedObjects+=' writable-'+$member+'.obj'
   }
   $extractions+=@(
    ('copy /Y writable-crtexew.obj "\\vmware-host\Shared Folders\fsx-build\queue\'+$id+'\writable-crtexew.obj"'),
    'if errorlevel 1 exit 6'
   )
   if($WritableDataSelection -eq 'AtlObjectSplitMfcBssTail'){
    foreach($member in 'stdafx','gs_report','appmodul'){
     $library=if($member -eq 'gs_report'){'msvcrt'}else{'mfcs80'}
     $archivePath=if($member -eq 'gs_report'){'build\intel\dll_obj\'}else{'INTEL\$DLL.W\'}
     $extractions+='lib /nologo /extract:"'+$archivePath+$member+'.obj" /out:writable-'+$member+'.obj "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\'+$library+'.lib"'
     $extractions+='if errorlevel 1 exit 6'
     $orderedObjects+=' writable-'+$member+'.obj'
    }
    $extractions+=@(
     ('copy /Y writable-appmodul.obj "\\vmware-host\Shared Folders\fsx-build\queue\'+$id+'\writable-appmodul.obj"'),
     'if errorlevel 1 exit 6'
    )
   }
  }
  $buildCommand=$buildCommand.Replace('link /nologo ', (($extractions -join "`r`n")+"`r`nlink /nologo $orderedObjects "))
 }
 $mfcSupportHash=$null;$mfcSupportPdbHash=$null
 if($MfcSupportLibrary -eq 'RTM'){
  $mfcSupportHash=(Get-FileHash "$share\toolchains\vc80-rtm\mfcs80.lib").Hash
  if($mfcSupportHash -cne '8B0BE145F8256766954D36DBC43486F549FFCFE8F5C7F58BB2D7B0929458708A'){throw 'Pinned RTM MFC support-library identity differs'}
  $mfcSupportPdbHash=(Get-FileHash "$share\toolchains\vc80-rtm\mfcs80.pdb").Hash
  if($mfcSupportPdbHash -cne '0AEE35F17868DCFD122D4FDF96F3821EDC46E4688875324F93C39C3D5696A0D1'){throw 'Pinned RTM MFC support-PDB identity differs'}
  if(([regex]::Matches($buildCommand,' mfcs80\.lib ')).Count -ne 1){throw 'MFC support-library command changed'}
  $buildCommand=$buildCommand.Replace(' mfcs80.lib ',' "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\mfcs80.lib" ')
 }
 if($MergeBssIntoData){$buildCommand=$buildCommand.Replace('link /nologo ','link /nologo /MERGE:.bss=.data ')}
 if($WritableDataSelection -eq 'AtlObjectCrtLibraryFirst'){
  $orderPattern='(?m)( mfc80\.lib (?:"[^"\r\n]+\\mfcs80\.lib"|mfcs80\.lib)) ("[^"\r\n]+\\msvcrt\.lib")'
  if(([regex]::Matches($buildCommand,$orderPattern)).Count -ne 1){throw 'Pinned MFC/CRT library-order template changed'}
  $buildCommand=[regex]::Replace($buildCommand,$orderPattern,' $2$1')
 }
 if($ImportSelection -eq 'IatOrder'){
  # Resolve MFC support members before the dynamic MFC import archive.
  $buildCommand=[regex]::Replace($buildCommand,' mfc80\.lib ("[^"\r\n]+\\mfcs80\.lib"|mfcs80\.lib) ',' $1 mfc80.lib ')
 }
 if($ToolchainDiagnostics){
  $buildCommand=$buildCommand.Replace('link /nologo ','link /nologo /VERBOSE:LIB ')
  $diagnostics=@'
echo === CRT source evidence ===
if exist "%VCINSTALLDIR%\crt\src\unhandld.cpp" type "%VCINSTALLDIR%\crt\src\unhandld.cpp"
if exist "%VCINSTALLDIR%\crt\src\ehvecdtr.cpp" findstr /n /i "terminate unhandled pragma include" "%VCINSTALLDIR%\crt\src\ehvecdtr.cpp"
echo === Compiled object directives ===
dumpbin /DIRECTIVES startup_context_pilot.obj
echo === Diagnostic non-GL object, never linked ===
cl /nologo /O2 /Oy- /hotpatch /GS /MD /Zi /GL- /EHsc /D_AFXDLL /c startup_context_pilot.cpp /Fodiagnostic-context.obj
if errorlevel 1 exit 6
dumpbin /DIRECTIVES diagnostic-context.obj
echo === CRT archive members ===
for /f "delims=" %%M in ('lib /nologo /list "%VCINSTALLDIR%\lib\msvcrt.lib" ^| findstr /i "unhandld.obj ehvecdtr.obj throw.obj cinitexe.obj crtexew.obj"') do (
 echo MEMBER %%M
 lib /nologo /extract:"%%M" /out:crt-probe.obj "%VCINSTALLDIR%\lib\msvcrt.lib"
 dumpbin /HEADERS /SYMBOLS /RELOCATIONS /DIRECTIVES crt-probe.obj
)
echo === End toolchain diagnostics ===
'@
  $buildCommand=$buildCommand.Replace('link /nologo /VERBOSE:LIB ',($diagnostics+"`r`nlink /nologo /VERBOSE:LIB "))
 }
 if($WritableDataDiagnostics){
  $dataDiagnostics=@'
echo === Writable data diagnostics; probe objects never linked ===
if not exist "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-sp1" mkdir "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-sp1"
for %%L in (mfc80.lib mfcs80.lib atls.lib) do (
 if not exist "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-sp1\%%L" copy "%VCINSTALLDIR%\atlmfc\lib\%%L" "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-sp1\%%L"
 fc /b "%VCINSTALLDIR%\atlmfc\lib\%%L" "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-sp1\%%L" >nul
 if errorlevel 1 exit 6
)
findstr /s /n /i /c:"forceMFCManifest" "%VCINSTALLDIR%\atlmfc\include\*.h"
cl /nologo /GR- /O2 /Oy- /hotpatch /GS /MD /Zi /GL- /EHsc /D_AFXDLL /c startup_context_pilot.cpp /Fodata-context-probe.obj
if errorlevel 1 exit 6
dumpbin /HEADERS /SYMBOLS data-context-probe.obj
for /f "delims=" %%M in ('lib /nologo /list "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\msvcrt.lib" ^| findstr /i /c:"natstart.obj" /c:"gs_cookie.obj" /c:"gs_report.obj"') do (
 echo DATA MEMBER %%M
 lib /nologo /extract:"%%M" /out:data-member-probe.obj "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\msvcrt.lib"
 if errorlevel 1 exit 6
 dumpbin /HEADERS /SYMBOLS data-member-probe.obj
)
for /f "delims=" %%M in ('lib /nologo /list "%VCINSTALLDIR%\atlmfc\lib\atls.lib" ^| findstr /i /c:"atltrace.obj"') do (
 echo DATA MEMBER %%M
 lib /nologo /extract:"%%M" /out:data-member-probe.obj "%VCINSTALLDIR%\atlmfc\lib\atls.lib"
 if errorlevel 1 exit 6
 dumpbin /HEADERS /SYMBOLS data-member-probe.obj
)
echo === End writable data diagnostics ===
'@
  $buildCommand=$buildCommand.Replace('link /nologo ',($dataDiagnostics+"`r`nlink /nologo "))
 }
 if($CompilerToolset -ne 'InstalledSP1'){
  $toolset=if($CompilerToolset -eq 'RTM'){'rtm'}else{'sp1'}
  $manifest=Get-Content "$project\artifacts\shared\toolchain-media\vc8-$toolset-x86-tool-identities.json" -Raw|ConvertFrom-Json
  $toolDirectory="vc8-$toolset"
  $setup=@("mkdir $toolDirectory", 'if errorlevel 1 exit 2')
  foreach($tool in $manifest){
   if((Get-FileHash "$share\toolchains\vc80-$toolset\x86\$($tool.file)").Hash -ne $tool.sha256){throw "$CompilerToolset tool identity differs: $($tool.file)"}
   $setup+='copy /Y "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-'+$toolset+'\x86\'+$tool.file+'" '+$toolDirectory+'\'
   $setup+='if errorlevel 1 exit 2'
  }
  $setup+=@("mkdir $toolDirectory\1033","copy /Y $toolDirectory\*ui.dll $toolDirectory\1033\",'if errorlevel 1 exit 2',('set "PATH=%CD%\'+$toolDirectory+';%PATH%"'))
  $buildCommand=[regex]::Replace($buildCommand,'(?m)^cl\r?$', (($setup + 'cl') -join "`r`n").TrimEnd())
  $buildCommand=$buildCommand.Replace('Matching search: SP1',"Matching search: $CompilerToolset")
 }
 if($AfxInitializeProbe){
  $compile='cl /nologo /GR- /W4 /O1 /Oy /GS /MD /Zi /GL- /EHsc /D_AFXDLL /c afx_initialize.cpp'
  $buildCommand=$buildCommand.Replace('link /nologo ',"$compile`r`nif errorlevel 1 exit 3`r`nlink /nologo /INCLUDE:?InitializeMfcModuleState@@YGHHK@Z ")
  $buildCommand=$buildCommand.Replace(' application_interface_pilot.obj kernel32.lib',' application_interface_pilot.obj afx_initialize.obj kernel32.lib')
 }
 if($WritableDataSelection -eq 'AtlObjectMfcNolib'){
  $compileNolib=@(
   'copy /y "%VCINSTALLDIR%\atlmfc\src\mfc\nolib.cpp" mfc_nolib.cpp'
   'if errorlevel 1 exit 3'
   'cl /nologo /O2 /Oy- /hotpatch /GS /MD /Zi /GL /EHsc /D_AFXDLL /c mfc_nolib.cpp'
   'if errorlevel 1 exit 3'
  ) -join "`r`n"
  if(!$buildCommand.Contains(' kernel32.lib')){throw 'Missing MFC link template'}
  $buildCommand=$buildCommand.Replace('link /nologo ', "$compileNolib`r`nlink /nologo ").Replace(' kernel32.lib',' mfc_nolib.obj kernel32.lib')
 }
 [IO.File]::WriteAllText("$job\build.cmd",$buildCommand,[Text.ASCIIEncoding]::new())
 $meta=@{Job=$id;RunId=$RunId;Index=$index;Type=$plan.Type;Initialize=$plan.Initialize;Condition=$plan.Condition;SourceSHA256=$plan.SourceSHA256;ExpectedCompiler=$compilerVersion;LinkFolding=$LinkFolding;FunctionPadding=$FunctionPadding;ImageBaseMode=$ImageBaseMode;SectionLayout=$SectionLayout;StackMode=$StackMode;PdbPathMode=$PdbPathMode;HeaderFlagsMode=$HeaderFlagsMode;ImageVersionMode=$ImageVersionMode;OsVersionMode=$OsVersionMode;DebugGapProbeBytes=$DebugGapProbeBytes;FunctionOrder=$FunctionOrder;CallbackLinkage=$CallbackLinkage;CrtStartupSelection=$CrtStartupSelection;ImportSelection=$ImportSelection;WritableDataSelection=$WritableDataSelection;MfcSupportLibrary=$MfcSupportLibrary;MfcSupportLibrarySHA256=$mfcSupportHash;MfcSupportPdbSHA256=$mfcSupportPdbHash;MergeBssIntoData=[bool]$MergeBssIntoData;Rtti=$Rtti;BuildProfile=$BuildProfile;GetterCompilation=$GetterCompilation;ContextCompilation=$ContextCompilation;CrtLibrary=$CrtLibrary;CrtLibrarySHA256=$crtLibraryHash;ToolchainDiagnostics=[bool]$ToolchainDiagnostics;WritableDataDiagnostics=[bool]$WritableDataDiagnostics;BuildCommandSHA256=(Get-FileHash "$job\build.cmd").Hash}
 $meta.CompilerToolset=$CompilerToolset
 if($AfxInitializeProbe){$meta.AfxInitializeProbe=$true}
 $meta|ConvertTo-Json|Set-Content "$job\request.json"
 Copy-Item "$job\request.json","$job\source-hashes.json" "$run\candidates\$id"
 $state.CurrentJob=$id;$state.Status='waiting_for_guest';Save-State
 [IO.File]::WriteAllText("$job\ready",$id,[Text.Encoding]::ASCII)
 return @{Id=$id;Path=$job;Plan=$plan}
}
Save-State
$active=Publish-Job $plans[0] 1
if($PrepareOnly){Write-Output "Prepared $($active.Id); run folder: $run";exit 0}
try {
 for($index=0;$index -lt $plans.Count;$index++){
  if(Test-Path "$run\STOP"){Stop-Run 'stopped' 'User stop requested; no further job queued.';exit 0}
  if($index -gt 0){$active=Publish-Job $plans[$index] ($index+1)}
  $deadline=[DateTime]::UtcNow.AddSeconds($JobTimeoutSeconds)
  while(!(Test-Path "$($active.Path)\done")){
   if(Test-Path "$run\STOP"){Stop-Run 'stopped' 'User stop requested. A running guest compiler may finish its current job.';exit 0}
   if([DateTime]::UtcNow -gt $deadline){Stop-Run 'timed_out' 'Guest did not complete before deadline. No next job queued.';exit 1}
   Start-Sleep -Seconds 1
  }
  $receipt=Get-Content "$($active.Path)\receipt.json" -Raw|ConvertFrom-Json
  if($receipt.Job -ne $active.Id){throw 'Receipt identity mismatch'}
  $output="$($active.Path)\output"
  $destination="$run\candidates\$($active.Id)"
  foreach($member in 'writable-crtexew.obj','writable-appmodul.obj'){
   $extracted=Join-Path $active.Path $member
   if(Test-Path -LiteralPath $extracted){
    Copy-Item -LiteralPath $extracted -Destination $output
   }
  }
  Get-ChildItem $output -File|Copy-Item -Destination $destination
  Copy-Item -LiteralPath "$($active.Path)\receipt.json","$($active.Path)\build.cmd" -Destination $destination
  New-Item -ItemType Directory -Path "$destination\source" -Force|Out-Null
  Get-ChildItem "$($active.Path)\source" -File|Copy-Item -Destination "$destination\source"
  if($receipt.ExitCode -ne 0){Stop-Run 'build_failed' "Guest compiler failed for $($active.Id); see build-log.txt.";exit 1}
  foreach($entry in @(Get-Content "$($active.Path)\source-hashes.json" -Raw|ConvertFrom-Json)){
   $compiled=Join-Path "$output\compiled-source" $entry.File
   if(!(Test-Path -LiteralPath $compiled) -or (Get-FileHash -LiteralPath $compiled).Hash -ne $entry.Hash){throw "Compiled source identity mismatch: $($entry.File)"}
  }
  foreach($file in @('fsx.exe','fsx.map','fsx.pdb','disassembly.txt','build-log.txt')){if(!(Test-Path "$output\$file")){throw "Missing output $file"}}
  if($WritableDataSelection -eq 'AtlObjectMfcNolib'){
   $mfcSource="$output\compiled-source\mfc_nolib.cpp"
   if((Get-FileHash -LiteralPath $mfcSource).Hash -ne '6EE17D493C2E0D6441591C66B4DEA10236CBAC170D12A4F73A40F535497B5B14'){throw 'MFC nolib.cpp differs from the verified RTM source'}
   Copy-Item -LiteralPath $mfcSource -Destination "$destination\source"
  }
  if((Get-Content "$output\build-log.txt" -Raw) -notmatch ('Compiler Version '+[regex]::Escape($compilerVersion)+'\b')){throw "Compiler identity differs from $compilerVersion"}
  $dumpbinVersion=$compilerVersion -replace '^14\.', '8.'
  if($CompilerToolset -ne 'InstalledSP1' -and (Get-Content "$output\disassembly.txt" -TotalCount 5|Out-String) -notmatch ([regex]::Escape($dumpbinVersion)+'\b')){throw "$CompilerToolset dumpbin identity differs"}
  $result=[ordered]@{Job=$active.Id;Type=$active.Plan.Type;Initialize=$active.Plan.Initialize;Condition=$active.Plan.Condition;Status='';Differences=$null;SourceSHA256=$active.Plan.SourceSHA256;BinarySHA256=(Get-FileHash "$output\fsx.exe").Hash}
  try {
   $startup=(& "$project\scripts\verify-startup-profile.ps1" -BuildDirectory $output -BuildProfile $BuildProfile -ExpectedStackMode $StackMode -ExpectedImageBase $(if($ImageBaseMode -eq 'Original'){0x01000000}else{0x00400000})|Out-String)|ConvertFrom-Json
   $functionOrderReport=$null
   if($FunctionOrder -ne 'Default'){$functionOrderReport=(& "$project\scripts\verify-function-order.ps1" -BuildDirectory $output -OrderFile "$($active.Path)\source\function-order-input.h"|Out-String)|ConvertFrom-Json}
   $sectionLayoutReport=(& "$project\scripts\verify-section-layout.ps1" -BuildDirectory $output -SectionLayout $SectionLayout|Out-String)|ConvertFrom-Json
   $pdbDebug=(& "$project\scripts\verify-debug-record.ps1" -BuildDirectory $output -PdbPathMode $PdbPathMode|Out-String)|ConvertFrom-Json
   $peHeaderSettings=(& "$project\scripts\verify-pe-header-settings.ps1" -BuildDirectory $output -HeaderFlagsMode $HeaderFlagsMode -ImageVersionMode $ImageVersionMode -OsVersionMode $OsVersionMode|Out-String)|ConvertFrom-Json
   $staticInitialization=$null
   $preCInit=$null
   $preCppInit=$null
   $crtStartup=$null
   $crtImageHelpers=$null
   $crtFailureStack=$null
   $crtOnexit=$null
   $atlCodepage=$null
   $runtimeRemainders=$null
   $rttiMetadata=$null
   $dataConstants=$null
   $peImports=$null
   if($BuildProfile -eq 'MfcWindows'){
    $staticInitialization=(& "$project\scripts\compare-static-glue.ps1" -BuildDirectory $output -RequireOriginalOrder|Out-String)|ConvertFrom-Json
    if($CrtLibrary -eq 'RTM' -and !$staticInitialization.FullCInitializationTableMatchesOriginal){throw 'RTM C initializer table roles/order regressed'}
    $preCInit=(& "$project\scripts\compare-pre-c-init.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
    $preCppInit=(& "$project\scripts\compare-pre-cpp-init.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
    $crtStartup=(& "$project\scripts\compare-crt-startup.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
    $crtImageHelpers=(& "$project\scripts\compare-crt-image-helpers.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
    $crtFailureStack=(& "$project\scripts\compare-crt-failure-stack.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
    $crtOnexit=(& "$project\scripts\compare-crt-onexit.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
    $atlCodepage=(& "$project\scripts\compare-atl-codepage.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
    $runtimeRemainders=(& "$project\scripts\compare-runtime-remainders.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
    $rttiMetadata=(& "$project\scripts\compare-rtti.ps1" -BuildDirectory $output -RequireOriginalShape:($Rtti -eq 'Disabled')|Out-String)|ConvertFrom-Json
    $dataConstants=(& "$project\scripts\compare-data-constants.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
    $peImports=(& "$project\scripts\compare-pe-imports.ps1" -BuildDirectory $output -RequireImportContents|Out-String)|ConvertFrom-Json
   }elseif($Target -eq 'StaticInitialization'){throw 'StaticInitialization target requires the MfcWindows profile'}
   $version=(& "$project\scripts\compare-version-pilot.ps1" -Pilot "$output\fsx.exe" -Map "$output\fsx.map"|Out-String)|ConvertFrom-Json
   $copy=(& "$project\scripts\compare-copy-pilot.ps1" -Pilot "$output\fsx.exe" -Disassembly "$output\disassembly.txt"|Out-String)|ConvertFrom-Json
   $format=(& "$project\scripts\compare-formatter-pilot.ps1" -Pilot "$output\fsx.exe" -Disassembly "$output\disassembly.txt"|Out-String)|ConvertFrom-Json
   $ole=(& "$project\scripts\compare-ole-filter.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
   $lifecycle=(& "$project\scripts\compare-app-lifecycle.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
   $dispatch=(& "$project\scripts\compare-app-dispatch.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
   $aliases=(& "$project\scripts\compare-vtable-aliases.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
   if($LinkFolding -eq 'ICF' -and $aliases.AliasPatternDifferences -ne 0){throw 'Vtable address-sharing pattern differs from original'}
   if(!$lifecycle.Constructor.AddressNormalizedExactMatch -or !$lifecycle.Destructor.AddressNormalizedExactMatch){throw 'Application lifecycle match regressed'}
   if(!$lifecycle.ExitInstance.AddressNormalizedExactMatch){throw 'ExitInstance match missing or regressed'}
   if(@($version.Results|Where-Object {!$_.AddressNormalizedExactMatch}).Count -or !$copy.RawExactMatch -or !$format.AddressNormalizedExactMatch -or !$ole.InitializerAddressNormalizedExactMatch -or !$ole.DeletingDestructorAddressNormalizedExactMatch -or !$ole.PendingOverrideRawExactMatch -or !$ole.ExceptionMetadataAndCleanupVerified){throw 'A previously verified match regressed'}
   $showComparison=(& "$project\scripts\compare-show-error.ps1" -BuildDirectory $output|Out-String)|ConvertFrom-Json
   if($AfxInitializeProbe){
    & "$project\scripts\compare-afx-initialize.ps1" -BuildDirectory $output -RequireSeparate:($LinkFolding -eq 'NOICF') | Set-Content "$destination\afx-initialize.json"
   }
   if($Target -ne 'ShowErrorMessage' -and @($showComparison.RemainingDifferingOffsets).Count -gt 2){throw 'ShowErrorMessage regressed beyond its two-byte baseline'}
   if($Target -eq 'InitInstance'){
    if(@($showComparison.RemainingDifferingOffsets).Count -gt 2){throw 'ShowErrorMessage regressed beyond its two-byte baseline'}
    $diagnostic=(& "$project\scripts\compare-initinstance.ps1" -BuildDirectory $output -OutputDirectory $destination|Out-String)|ConvertFrom-Json
    & "$project\scripts\inventory-initinstance-references.ps1" -BuildDirectory $destination | Out-Null
    $blocks=(& "$project\scripts\compare-initinstance-blocks.ps1" -BuildDirectory $destination|Out-String)|ConvertFrom-Json
    $operands=(& "$project\scripts\compare-initinstance-operands.ps1" -BuildDirectory $destination|Out-String)|ConvertFrom-Json
    $result.InitInstance=$diagnostic
    $result.ControlFlowDiagnostic=$blocks
    $result.OperandDiagnostic=$operands
   }
   @{Startup=$startup;PdbDebug=$pdbDebug;PeHeaderSettings=$peHeaderSettings;FunctionOrder=$functionOrderReport;SectionLayout=$sectionLayoutReport;StaticInitialization=$staticInitialization;PreCInit=$preCInit;PreCppInit=$preCppInit;CrtStartup=$crtStartup;CrtImageHelpers=$crtImageHelpers;CrtFailureStack=$crtFailureStack;CrtOnexit=$crtOnexit;AtlCodepage=$atlCodepage;RuntimeRemainders=$runtimeRemainders;RttiMetadata=$rttiMetadata;DataConstants=$dataConstants;PeImports=$peImports;Version=$version;Copy=$copy;Formatter=$format;Ole=$ole;Lifecycle=$lifecycle;Dispatch=$dispatch;VTableAliases=$aliases;Target=$showComparison}|ConvertTo-Json -Depth 11|Set-Content "$destination\comparison.json"
   $result.Differences=@($showComparison.RemainingDifferingOffsets).Count;$result.Status='compared'
   if($Target -in 'AppLifecycle','StaticInitialization'){
    $result.ShowErrorRegressionDifferences=$result.Differences
    $result.Differences=0
    $state.BestDifferences=0;$state.BestJob=$active.Id
   }elseif($Target -eq 'InitInstance'){
    $result.ShowErrorRegressionDifferences=$result.Differences
    $result.Differences=$null
    $result.Status='diagnostic'
    if($diagnostic.RebuiltLength -eq $diagnostic.OriginalLength -and $blocks.EdgeMismatches -eq 0){
     $layout=(Get-Content "$destination\initinstance-diagnostic.json" -Raw|ConvertFrom-Json).Rows
     $startVA=[Convert]::ToInt64($diagnostic.RebuiltVA,16)
     $originalOffsets=@($layout|Where-Object OriginalAddress|ForEach-Object {[Convert]::ToInt64($_.OriginalAddress,16)-0x010019e0})
     $rebuiltOffsets=@($layout|Where-Object RebuiltAddress|ForEach-Object {[Convert]::ToInt64($_.RebuiltAddress,16)-$startVA})
     if(($originalOffsets -join ',') -eq ($rebuiltOffsets -join ',')){
      $bodyJson=& "$project\scripts\compare-initinstance-body.ps1" -BuildDirectory $destination
      $bodyJson|Set-Content "$destination\initinstance-body.json"
      $body=$bodyJson|ConvertFrom-Json
      $result.BodyStatus=$body.status
      $result.Differences=$body.remainingDifferingOffsets.Count
      $result.Status='compared'
      $state.BestDifferences=$result.Differences
      $state.BestJob=$active.Id
     }
    }
   }elseif($null -eq $state.BestDifferences -or $result.Differences -lt $state.BestDifferences){$state.BestDifferences=$result.Differences;$state.BestJob=$active.Id}
  }catch{
   $result.Status='rejected_comparison';$result.Error=$_.Exception.Message
   if($index -eq 0){$state.Results+=,$result;Stop-Run 'baseline_failed' $result.Error;exit 1}
  }
  $state.Results+=,$result;$state.Completed=$index+1;Save-State
  if($result.Status -eq 'compared' -and $result.Differences -eq 0){Stop-Run 'matched' 'Target matched after verified address mapping and all regression gates passed.';exit 0}
  if($Target -eq 'InitInstance'){Stop-Run 'diagnostic_complete' 'Reviewed InitInstance proposal built and compared. Regression checks passed.';exit 0}
 }
 Stop-Run 'exhausted' 'All implemented source proposals were tested; this is not proof that no other reconstruction can match.'
}catch{Stop-Run 'error' $_.Exception.Message;exit 1}finally{$controllerLock.Dispose()}

