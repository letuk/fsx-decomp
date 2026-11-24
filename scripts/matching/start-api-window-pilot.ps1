param([string]$RunId='api-window-20261103-001',[int]$TimeoutSeconds=180,[switch]$IncludeHeap,[switch]$WholeProgram,[switch]$LifecycleObject,[switch]$RegistryObject,[switch]$FactoryObject,[switch]$SourcePass,[switch]$NumberedNameAbiProbe,[switch]$NumberedNameSdkProbe,[switch]$AllocatedNameSdkProbe,[switch]$MultibyteCrtProbe,[switch]$Vc8Rtm,[switch]$CopyLeadIntProbe,[switch]$CopySharedReturnProbe,[switch]$CopySplitAdvanceProbe,[switch]$StringExportsProbe,[ValidateSet('Default','Size','NoGlobal','EarlyExit','TrailAdvance','SeparateSingleExit','ReturnBarrier','CombinedShape','SharedNul','StaggeredNul','SharedCursor','ParameterCursor','BranchStores','ParameterCombined','SharedZeroReturn','SharedZeroNul','StaggeredZeroNul','DelayedLeadAdvance','SourceBeforeCapacity','DestinationBeforeCapacity','CapacityParameter')][string]$StringCopyVariant='Default',[switch]$DuplicateHeapStringProbe,[switch]$DuplicateExpandedHeapProbe,[ValidateSet('Default','VolatileRead','ReadBarrier')][string]$HeapFlagProbe='Default',[switch]$HeapReallocatorProbe,[switch]$HeapRegistryLinkedProbe,[switch]$HeapRegistryInternalProbe,[ValidateSet('Default','Snapshot','SharedFailure','Both')][string]$RegistryBodyProbe='Default',[switch]$ConstructionDefaultsObject)
$ErrorActionPreference='Stop'
if($SourcePass -and !$FactoryObject){throw 'SourcePass requires FactoryObject for existing regression gates'}
if($ConstructionDefaultsObject -and (!$FactoryObject -or $SourcePass)){throw 'ConstructionDefaultsObject requires the factory compiler profile without SourcePass'}
if($NumberedNameAbiProbe -and !($SourcePass -and $FactoryObject)){throw 'NumberedNameAbiProbe requires the factory source-pass profile'}
if($RunId -notmatch '^[A-Za-z0-9_-]+$'){throw 'Invalid ID'}
if(($LifecycleObject -or $RegistryObject -or $FactoryObject) -and ($IncludeHeap -or $WholeProgram)){throw 'Object pilot has an independent fixed native compilation profile'}
if(([int][bool]$RegistryObject+[int][bool]$LifecycleObject+[int][bool]$FactoryObject) -gt 1){throw 'Choose one object profile'}
$isObject=$LifecycleObject -or $RegistryObject -or $FactoryObject
if($NumberedNameSdkProbe -and ($isObject -or $IncludeHeap)){throw 'NumberedNameSdkProbe uses the DLL fragment profile'}
if($AllocatedNameSdkProbe -and !$NumberedNameSdkProbe){throw 'AllocatedNameSdkProbe extends NumberedNameSdkProbe'}
if($DuplicateHeapStringProbe -and !($AllocatedNameSdkProbe -and $WholeProgram)){throw 'DuplicateHeapStringProbe uses the allocated-name LTCG profile'}
if($DuplicateExpandedHeapProbe -and !$DuplicateHeapStringProbe){throw 'DuplicateExpandedHeapProbe extends DuplicateHeapStringProbe'}
if($HeapReallocatorProbe -and !$DuplicateHeapStringProbe){throw 'HeapReallocatorProbe needs the linked heap profile'}
if($HeapRegistryLinkedProbe -and !$HeapReallocatorProbe){throw 'HeapRegistryLinkedProbe requires the separate reallocator'}
if($HeapRegistryInternalProbe -and !$HeapRegistryLinkedProbe){throw 'HeapRegistryInternalProbe requires the linked registry profile'}
if($RegistryBodyProbe -ne 'Default' -and !$HeapRegistryInternalProbe){throw 'RegistryBodyProbe requires the internal registry profile'}
if($HeapFlagProbe -ne 'Default' -and (!$DuplicateHeapStringProbe -or $DuplicateExpandedHeapProbe)){throw 'HeapFlagProbe needs both shared allocator callers'}
if($MultibyteCrtProbe -and ($isObject -or $IncludeHeap -or $NumberedNameSdkProbe -or $WholeProgram)){throw 'MultibyteCrtProbe uses the fixed static CRT DLL profile'}
if($Vc8Rtm -and !($MultibyteCrtProbe -or $AllocatedNameSdkProbe -or $FactoryObject)){throw 'Vc8Rtm requires the static CRT, allocated-name, or factory object profile'}
if($CopyLeadIntProbe -and !$AllocatedNameSdkProbe){throw 'CopyLeadIntProbe requires AllocatedNameSdkProbe'}
if($CopySharedReturnProbe -and !$CopyLeadIntProbe){throw 'CopySharedReturnProbe requires CopyLeadIntProbe'}
if($CopySplitAdvanceProbe -and (!$CopyLeadIntProbe -or $CopySharedReturnProbe)){throw 'CopySplitAdvanceProbe requires CopyLeadIntProbe without CopySharedReturnProbe'}
if($StringExportsProbe -and ($isObject -or $IncludeHeap -or $NumberedNameSdkProbe -or $MultibyteCrtProbe)){throw 'StringExportsProbe uses its own DLL fragment profile'}
if($StringCopyVariant -ne 'Default' -and (!$StringExportsProbe -or $WholeProgram)){throw 'StringCopyVariant requires the native string export profile'}
$compilerCandidate=if($Vc8Rtm){'14.00.50727.42'}else{'14.00.50727.762'}
$project=[IO.Path]::GetFullPath("$PSScriptRoot\..\..");$share=Join-Path $project 'vm-share'
if((Get-FileHash "$project\original\api.dll").Hash -cne '9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003'){throw 'Original identity changed'}
$worker=Get-Content "$share\worker-state.json" -Raw|ConvertFrom-Json
if($worker.Status -ne 'ready' -or $worker.Job){throw 'Worker not ready'}
$lock=[IO.File]::Open("$share\controller.lock",[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
try {
 $moduleRoot=Join-Path $share 'modules\api'
 New-Item -ItemType Directory -Path "$moduleRoot\runs","$moduleRoot\archive" -Force|Out-Null
 $run=Join-Path $moduleRoot "runs\$RunId";$job=Join-Path $share "queue\$RunId"
 if($RunId -notlike 'api-*'){throw 'API job IDs must start with api- in the shared queue'}
 if(Test-Path "$moduleRoot\jobs\$RunId"){throw 'Job already retained in module history'}
 if((Test-Path $run) -or (Test-Path $job)){throw 'Unique ID required'}
 New-Item -ItemType Directory -Path "$job\source",$run|Out-Null
 Copy-Item "$project\src\api-pilot\*" "$job\source"
 if($HeapFlagProbe -ne 'Default'){
  $heapPath="$job\source\heap.cpp"
  $heapText=[IO.File]::ReadAllText($heapPath)
  $flagCheck=[regex]::new('    const volatile int& initialized = g_heapInitializedPilot;\r?\n    if \(initialized == 0\)\r?\n        InitializeHeapManagerPilot\(\);')
  if($flagCheck.Matches($heapText).Count -ne 1){throw 'Allocator flag check baseline changed'}
  $replacement=switch($HeapFlagProbe){
   'VolatileRead'{"    const volatile int& initialized = g_heapInitializedPilot;`r`n    if (initialized == 0)`r`n        InitializeHeapManagerPilot();"}
   'ReadBarrier'{"    const int initialized = g_heapInitializedPilot;`r`n    _ReadWriteBarrier();`r`n    if (initialized == 0)`r`n        InitializeHeapManagerPilot();"}
  }
  $heapText=$flagCheck.Replace($heapText,$replacement)
  if($HeapFlagProbe -eq 'ReadBarrier'){
   $heapText="#include <intrin.h>`r`n#pragma intrinsic(_ReadWriteBarrier)`r`n"+$heapText
  }
  [IO.File]::WriteAllText($heapPath,$heapText)
 }
 if($CopyLeadIntProbe){
  $copySource="$job\source\resolved_path_copy.cpp"
  $copyText=[IO.File]::ReadAllText($copySource)
  $baseline='const bool lead = _ismbblead(static_cast<signed char>(*source)) != 0;'
  if(!$copyText.Contains($baseline)){throw 'Copy lead-byte baseline changed'}
  $copyText=$copyText.Replace($baseline,'const int lead = _ismbblead(static_cast<signed char>(*source));').Replace('if (lead) {','if (lead != 0) {')
  if($CopySplitAdvanceProbe){
   $advance='const char first = \*source\+\+;\s+\*cursor\+\+ = first;\s+--remaining;\s+if \(lead != 0\) \{'
   if(([regex]::Matches($copyText,$advance)).Count -ne 1){throw 'Copy loop baseline changed'}
   $copyText=$copyText -replace $advance, "const char first = *source;`r`n        *cursor = first;`r`n        --remaining;`r`n        if (lead != 0) {`r`n            ++cursor;`r`n            ++source;"
   $copyText=$copyText -replace '\} else if \(first == ''\\0''\) \{\s+break;\s+\}', "} else {`r`n            ++cursor;`r`n            ++source;`r`n            if (first == '\0') {`r`n                break;`r`n            }`r`n        }"
  }
  if($CopySharedReturnProbe){
   $copyText=$copyText -replace '(?s)    if \(_getmbcp\(\) == 0\).*?(?=    char\* cursor)', "    if (_getmbcp() == 0) {`r`n        destination = lstrcpynA(destination, source, capacity);`r`n    } else if (capacity != 0) {`r`n"
   $copyText=[regex]::Replace($copyText, '(?ms)(^    char\* cursor.*?^    \*cursor = .*?;\r?\n)(?=    return destination;)', [System.Text.RegularExpressions.MatchEvaluator]{
    param($loop)
    (($loop.Groups[1].Value.TrimEnd() -split '\r?\n' | ForEach-Object {"    $_"}) -join "`r`n") + "`r`n    }`r`n"
   })
  }
  [IO.File]::WriteAllText($copySource,$copyText)
 }
 if($NumberedNameAbiProbe){
  Copy-Item "$project\src\api-experiments\numbered_name_abi_probe.cpp" "$job\source"
  Add-Content "$job\source\factory_boundary.cpp" '#include "numbered_name_abi_probe.cpp"'
 }
 if($SourcePass){
  # Replay the frozen broad-pass inputs for the existing native regression checks.
  $sourcePassSnapshot=Join-Path $project 'artifacts/api.dll/api-source-pass-20261107/archived-source'
  Copy-Item -LiteralPath "$sourcePassSnapshot/leaf_candidates.cpp","$sourcePassSnapshot/scalar_types.h" -Destination "$job\source"
  Add-Content "$job\source\factory_boundary.cpp" '#include "leaf_candidates.cpp"'
  Copy-Item -LiteralPath "$sourcePassSnapshot/source-pass.json" -Destination "$run\source-pass.json"
  @{SourcePass=$true;Scope='Generated leaf candidates plus existing factory regression; no linked DLL or runtime claim'}|ConvertTo-Json|Set-Content "$run\source-pass-profile.json"
 }
 # XP cmd.exe requires conventional CRLF batch lines.
 $cmd=([IO.File]::ReadAllText("$PSScriptRoot\guest-build-api-window.cmd") -replace '\r?\n',"`r`n")
 if($RegistryObject){$cmd=([IO.File]::ReadAllText("$PSScriptRoot\guest-build-api-registry-object.cmd") -replace '\r?\n',"`r`n")}
 if($LifecycleObject){$cmd=([IO.File]::ReadAllText("$PSScriptRoot\guest-build-api-lifecycle-object.cmd") -replace '\r?\n',"`r`n")}
 if($FactoryObject){$cmd=([IO.File]::ReadAllText("$PSScriptRoot\guest-build-api-factory-object.cmd") -replace '\r?\n',"`r`n")}
 if($ConstructionDefaultsObject){
  $cmd=$cmd.Replace('factory_boundary','construction_default_copies').Replace('API factory/four-slot VTable native COFF','API construction defaults native COFF')
 }
 if($IncludeHeap){$cmd=$cmd.Replace('window_handle.cpp window_state.cpp','window_handle.cpp window_state.cpp heap.cpp heap_state.cpp').Replace('window_handle.obj window_state.obj','window_handle.obj window_state.obj heap.obj heap_state.obj kernel32.lib user32.lib msvcrt.lib')}
 if($StringExportsProbe){
  Copy-Item "$project\src\api-experiments\string_exports_probe.cpp" "$job\source"
  $cmd=$cmd.Replace('window_handle.cpp window_state.cpp','string_exports_probe.cpp resolved_path_copy.cpp path_append.cpp').Replace('window_handle.obj window_state.obj','string_exports_probe.obj resolved_path_copy.obj path_append.obj msvcrt.lib kernel32.lib')
  $cmd=$cmd.Replace('dumpbin /HEADERS fsx.exe','dumpbin /HEADERS /EXPORTS /IMPORTS fsx.exe')
  if($StringCopyVariant -ne 'Default'){
   $copyOptions=switch($StringCopyVariant){
    'Size'{'/O1'}
    'NoGlobal'{'/O2 /Og-'}
    'TrailAdvance'{'/O2 /DCOPY_TRAIL_ADVANCE'}
    'SeparateSingleExit'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT'}
    'ReturnBarrier'{'/O2 /DCOPY_RETURN_BARRIER'}
    'CombinedShape'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER'}
    'SharedNul'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARE_NUL'}
    'StaggeredNul'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARE_NUL /DCOPY_STAGGER_NUL'}
    'SharedCursor'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARE_NUL /DCOPY_STAGGER_NUL /DCOPY_SHARED_CURSOR'}
    'ParameterCursor'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARE_NUL /DCOPY_STAGGER_NUL /DCOPY_SHARED_CURSOR /DCOPY_PARAMETER_CURSOR'}
    'BranchStores'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARE_NUL /DCOPY_BRANCH_STORES'}
    'ParameterCombined'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARED_CURSOR /DCOPY_PARAMETER_CURSOR'}
    'SharedZeroReturn'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARED_CURSOR /DCOPY_PARAMETER_CURSOR /DCOPY_SHARED_ZERO_RETURN'}
    'SharedZeroNul'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARED_CURSOR /DCOPY_PARAMETER_CURSOR /DCOPY_SHARED_ZERO_RETURN /DCOPY_SHARE_NUL'}
    'StaggeredZeroNul'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARED_CURSOR /DCOPY_PARAMETER_CURSOR /DCOPY_SHARED_ZERO_RETURN /DCOPY_SHARE_NUL /DCOPY_STAGGER_NUL'}
    'DelayedLeadAdvance'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARED_CURSOR /DCOPY_PARAMETER_CURSOR /DCOPY_SHARED_ZERO_RETURN /DCOPY_SHARE_NUL /DCOPY_DELAY_LEAD_ADVANCE'}
    'SourceBeforeCapacity'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARED_CURSOR /DCOPY_PARAMETER_CURSOR /DCOPY_SHARED_ZERO_RETURN /DCOPY_SHARE_NUL /DCOPY_DELAY_LEAD_ADVANCE /DCOPY_SOURCE_BEFORE_CAPACITY'}
    'DestinationBeforeCapacity'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARED_CURSOR /DCOPY_PARAMETER_CURSOR /DCOPY_SHARED_ZERO_RETURN /DCOPY_SHARE_NUL /DCOPY_DELAY_LEAD_ADVANCE /DCOPY_DESTINATION_BEFORE_CAPACITY'}
    'CapacityParameter'{'/O2 /DCOPY_TRAIL_ADVANCE /DCOPY_SEPARATE_SINGLE_EXIT /DCOPY_RETURN_BARRIER /DCOPY_SHARED_CURSOR /DCOPY_PARAMETER_CURSOR /DCOPY_SHARED_ZERO_RETURN /DCOPY_SHARE_NUL /DCOPY_DELAY_LEAD_ADVANCE /DCOPY_CAPACITY_PARAMETER'}
    default{'/O2'}
   }
   if($StringCopyVariant -notin @('Size','NoGlobal')){
    Copy-Item "$project\src\api-experiments\copy_early_exit_probe.cpp" "$job\source\resolved_path_copy.cpp"
   }
   $cmd=$cmd.Replace('string_exports_probe.cpp resolved_path_copy.cpp path_append.cpp','string_exports_probe.cpp path_append.cpp')
   $cmd=$cmd.Replace('link /nologo ', "cl /nologo /W4 $copyOptions /Oy- /hotpatch /GS /MD /Zi /GL- /c resolved_path_copy.cpp`r`nif errorlevel 1 exit 3`r`nlink /nologo ")
  }
 }
 if($MultibyteCrtProbe){
  Copy-Item "$project\src\api-experiments\multibyte_crt_probe.cpp" "$job\source"
  $cmd=$cmd.Replace('/MD','/MT').Replace('window_handle.cpp window_state.cpp','multibyte_crt_probe.cpp').Replace('window_handle.obj window_state.obj','/VERBOSE:LIB multibyte_crt_probe.obj libcmt.lib kernel32.lib')
  $cmd=$cmd.Replace('dumpbin /HEADERS fsx.exe','dumpbin /HEADERS /EXPORTS /IMPORTS fsx.exe')
 }
 if($NumberedNameSdkProbe){
  Copy-Item "$project\src\api-experiments\numbered_name_sdk_probe.cpp" "$job\source"
  $cmd=$cmd.Replace('window_handle.cpp window_state.cpp','/showIncludes numbered_name_sdk_probe.cpp').Replace('window_handle.obj window_state.obj','numbered_name_sdk_probe.obj msvcrt.lib kernel32.lib')
  $cmd=$cmd.Replace('dumpbin /HEADERS fsx.exe','dumpbin /HEADERS /EXPORTS /IMPORTS fsx.exe')
  if($AllocatedNameSdkProbe){
   Copy-Item "$project\src\api-experiments\allocated_name_sdk_probe.cpp" "$job\source"
   $cmd=$cmd.Replace('numbered_name_sdk_probe.cpp','allocated_name_sdk_probe.cpp heap.cpp heap_state.cpp path_append.cpp').Replace('numbered_name_sdk_probe.obj msvcrt.lib kernel32.lib','allocated_name_sdk_probe.obj resolved_path_copy.obj heap.obj heap_state.obj path_append.obj msvcrt.lib kernel32.lib user32.lib')
   if($DuplicateHeapStringProbe){
    Copy-Item "$project\src\api-experiments\duplicate_heap_string_exports.cpp" "$job\source"
    if($DuplicateExpandedHeapProbe){
     Copy-Item "$project\src\api-experiments\duplicate_heap_string_expanded_probe.cpp" "$job\source\duplicate_heap_string.cpp"
    }
    $cmd=$cmd.Replace('allocated_name_sdk_probe.cpp','allocated_name_sdk_probe.cpp duplicate_heap_string_exports.cpp duplicate_heap_string.cpp')
    $cmd=$cmd.Replace('allocated_name_sdk_probe.obj','allocated_name_sdk_probe.obj duplicate_heap_string_exports.obj duplicate_heap_string.obj')
   }
  }
 }
 if($WholeProgram){$cmd=$cmd.Replace('/GL-','/GL').Replace('link /nologo ','link /nologo /LTCG ')}
 if($AllocatedNameSdkProbe){
  $cmd=$cmd.Replace('link /nologo ', "cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /GL- /c resolved_path_copy.cpp`r`nif errorlevel 1 exit 3`r`nlink /nologo ")
 }
 if($HeapReallocatorProbe){
  $cmd=$cmd.Replace('link /nologo ', "cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /GL- /c reallocate_boundary.cpp`r`nif errorlevel 1 exit 3`r`nlink /nologo ").Replace('resolved_path_copy.obj','resolved_path_copy.obj reallocate_boundary.obj')
 }
 if($HeapRegistryLinkedProbe){
  $cmd=$cmd.Replace('link /nologo ', "cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /GL /DAPI_SEPARATE_REALLOCATOR /c callback_registry.cpp`r`nif errorlevel 1 exit 3`r`nlink /nologo ")
  if($HeapRegistryInternalProbe){
   Copy-Item "$project\src\api-experiments\callback_registry_callers_probe.cpp" "$job\source"
   $registryPath="$job\source\callback_registry.cpp"
   $registrySource=[IO.File]::ReadAllText($registryPath).Replace('int __stdcall RegisterCallbackListOwnerBoundaryPilot(', '__declspec(noinline) int __stdcall RegisterCallbackListOwnerBoundaryPilot(')
   if($RegistryBodyProbe -in @('Snapshot','Both')){
    $registrySource=$registrySource -replace '        if \(\*owner == NULL\)\r?\n            return -1;\r?\n        \(\*owner\)->count = 0;', "        CallbackListDataBoundaryPilot* const allocated = *owner;`r`n        if (allocated == NULL)`r`n            return -1;`r`n        allocated->count = 0;"
   }
   if($RegistryBodyProbe -in @('SharedFailure','Both')){
    $registrySource=$registrySource -replace '    if \(owner == NULL\)\r?\n        return -1;', "    if (owner == NULL) {`r`n    failed:`r`n        return -1;`r`n    }"
    $registrySource=$registrySource.Replace('            return -1;', '            goto failed;')
   }
   [IO.File]::WriteAllText($registryPath,$registrySource)
   $cmd=$cmd.Replace('/c callback_registry.cpp','/c callback_registry.cpp callback_registry_callers_probe.cpp')
   $cmd=$cmd.Replace('resolved_path_copy.obj','resolved_path_copy.obj callback_registry.obj callback_registry_callers_probe.obj')
  }else{
   $cmd=$cmd.Replace('resolved_path_copy.obj','resolved_path_copy.obj callback_registry.obj /EXPORT:RegisterCallbackListProbe=?RegisterCallbackListOwnerBoundaryPilot@@YGHPAPAUCallbackListDataBoundaryPilot@@P6AXKK@ZHKK@Z')
  }
 }
 [IO.File]::WriteAllText("$job\build.cmd",$cmd,[Text.Encoding]::ASCII)
 Get-ChildItem "$job\source" -File|Get-FileHash|Select-Object @{n='File';e={[IO.Path]::GetFileName($_.Path)}},Hash|ConvertTo-Json|Set-Content "$job\source-hashes.json"
 # Capture the actual profile separately from transport filenames and file copies.
 @{Module='api.dll';ArtifactKind=$(if($isObject){'COFF object'}else{'DLL fragment'});FactoryObject=[bool]$FactoryObject;RegistryObject=[bool]$RegistryObject;LifecycleObject=[bool]$LifecycleObject;WholeProgram=[bool]$WholeProgram;ConstructionDefaultsObject=[bool]$ConstructionDefaultsObject;CompilerCandidate=$compilerCandidate;MultibyteCrtProbe=[bool]$MultibyteCrtProbe;IncludeHeap=[bool]$IncludeHeap;Hypothesis=$(if($ConstructionDefaultsObject){'Standalone native construction-default copies'}elseif($FactoryObject){'Placement construction of observed four-slot bridge yields original getter with validated virtual-table/heap references; no dependency substitutes'}elseif($RegistryObject){'Callback-list and reallocation source/dependency model; no whole-body match assumed; see written plan'}elseif($isObject){'Ordinary outer methods and SEH callback source yield complete relocation-validated COFF ranges; no dependency substitutes'}else{'Use the DLL/heap experiment plan for the selected source snapshot'});BuildCommandSHA256=(Get-FileHash "$job\build.cmd").Hash}|ConvertTo-Json|Set-Content "$job\experiment.json"
 @{Job=$RunId;Module='api.dll';ArtifactKind=$(if($isObject){'COFF object'}else{'DLL fragment'});FactoryObject=[bool]$FactoryObject;RegistryObject=[bool]$RegistryObject;LifecycleObject=[bool]$LifecycleObject;WholeProgram=[bool]$WholeProgram;IncludeHeap=[bool]$IncludeHeap;Scope=$(if($ConstructionDefaultsObject){'Standalone native COFF construction-default copies and initial data'}elseif($FactoryObject){'Factory/four-slot VTable COFF; unresolved implementations remain explicit'}elseif($RegistryObject){'Callback-list/reallocation COFF with unresolved heap dependencies'}elseif($isObject){'COFF lifecycle object with unresolved externals'}else{'Comparison-only DLL boundary/heap fragment'});ConstructionDefaultsObject=[bool]$ConstructionDefaultsObject;CompilerCandidate=$compilerCandidate;MultibyteCrtProbe=[bool]$MultibyteCrtProbe;BuildCommandSHA256=(Get-FileHash "$job\build.cmd").Hash;Hypothesis=$(if($ConstructionDefaultsObject){'One translation unit preserves independent caller and callee code generation'}elseif($FactoryObject){'Placement construction of observed four-slot bridge yields original getter with validated virtual-table/heap references; no dependency substitutes'}elseif($RegistryObject){'Callback-list and reallocation source/dependency model; no whole-body match assumed; see written plan'}elseif($isObject){'Ordinary forwarding methods and SEH wrapper match complete original ranges after validated COFF relocations'}elseif($IncludeHeap){'Ordinary independently reconstructed heap initializer/allocator with original branching and three DWORD LFH locals yields stated body/reference structure; window getter must retain its exact gate'}else{'O2 Oy- hotpatch with ordinary index-zero conditional yields original 21-byte body after one validated global-address mapping'});TrialLimit=3;Trial=1;TransportFilename=$(if($isObject){'fsx.exe transports a COFF object, never executed'}else{'fsx.exe transports a DLL PE, never executed'})}|ConvertTo-Json|Set-Content "$job\request.json"
 Copy-Item "$job\request.json","$job\source-hashes.json","$job\build.cmd","$job\experiment.json" $run
 Copy-Item -LiteralPath "$job\source" -Destination "$run\source" -Recurse
 Set-Content "$job\ready" $RunId
 $deadline=[DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
 while(!(Test-Path "$job\done")){if([DateTime]::UtcNow -gt $deadline){throw 'Worker timeout; inspect existing job, do not redispatch'};Start-Sleep -Seconds 1}
 Copy-Item "$job\receipt.json" $run
 New-Item -ItemType Directory -Path "$run\worker-output"|Out-Null
 Copy-Item "$job\output\*" "$run\worker-output" -Recurse
 $receipt=Get-Content "$run\receipt.json" -Raw|ConvertFrom-Json
 foreach($marker in @('working','done')){
  if(Test-Path -LiteralPath "$job\$marker"){Copy-Item -LiteralPath "$job\$marker" -Destination $run}
 }
 if($receipt.Job -cne $RunId -or $receipt.ExitCode -ne 0){
  @{Status='build_failed';ExitCode=$receipt.ExitCode;Scope='Compilation failed; no candidate accepted';Log='worker-output/build-log.txt'}|ConvertTo-Json|Set-Content "$run\status.json"
  @{Module='api.dll';ArtifactKind=$(if($isObject){'COFF object attempt'}else{'DLL fragment attempt'});RunId=$RunId;RunDirectory=$run;JobDirectory=$run;TransportJobDirectory=$job;CanonicalFiles=@();WorkerOutputDirectory='worker-output';Scope='Failed build retained with receipt/log/source; no accepted artifact'}|ConvertTo-Json|Set-Content "$run\module.json"
  throw 'Worker build failed; inspect retained log'
 }
 foreach($entry in (Get-Content "$run\source-hashes.json" -Raw|ConvertFrom-Json)){if((Get-FileHash "$run\worker-output\compiled-source\$($entry.File)").Hash -cne $entry.Hash){throw 'Compiled input mismatch'}}
 $required=if($isObject){@('fsx.exe','fsx.pdb','headers.txt','disassembly.txt')}else{@('fsx.exe','fsx.map','fsx.pdb','headers.txt','disassembly.txt')}
 foreach($file in $required){if(!(Test-Path "$run\worker-output\$file")){throw "Missing output $file"}}
 $compilerPattern='(?<![\d.])'+[regex]::Escape($compilerCandidate)+'(?![\d.])'
 if((Get-Content "$run\worker-output\build-log.txt" -Raw) -notmatch $compilerPattern){throw "Expected VC8 compiler $compilerCandidate; see retained build log"}
 if($Vc8Rtm -and (Get-Content "$run\worker-output\disassembly.txt" -Raw) -notmatch '(?<![\d.])8\.00\.50727\.42(?![\d.])'){throw 'Expected VC8 RTM DUMPBIN; see retained listing'}
 $pairs=if($ConstructionDefaultsObject){@(@('fsx.exe','api-construction-default-copies.obj'),@('fsx.pdb','api-construction-default-copies.pdb'))}elseif($FactoryObject){@(@('fsx.exe','api-factory.obj'),@('fsx.pdb','api-factory.pdb'))}elseif($RegistryObject){@(@('fsx.exe','api-registry.obj'),@('fsx.pdb','api-registry.pdb'))}elseif($LifecycleObject){@(@('fsx.exe','api-lifecycle.obj'),@('fsx.pdb','api-lifecycle.pdb'))}else{@(@('fsx.exe','api-pilot.dll'),@('fsx.map','api-pilot.map'),@('fsx.pdb','api-pilot.pdb'))}
 foreach($pair in $pairs){Copy-Item "$run\worker-output\$($pair[0])" "$run\$($pair[1])"}
 $canonical=if($ConstructionDefaultsObject){'api-construction-default-copies.obj'}elseif($FactoryObject){'api-factory.obj'}elseif($RegistryObject){'api-registry.obj'}elseif($LifecycleObject){'api-lifecycle.obj'}else{'api-pilot.dll'}
 @{Status='built_not_compared';SHA256=(Get-FileHash "$run\$canonical").Hash;Scope=$(if($isObject){'Native COFF object with unresolved dependencies; no linked DLL claim'}else{'Isolated DLL fragment, no application factory/export144 yet'})}|ConvertTo-Json|Set-Content "$run\status.json"
 @{Module='api.dll';ArtifactKind=$(if($isObject){'COFF object'}else{'DLL fragment'});RunId=$RunId;RunDirectory=$run;JobDirectory=$run;TransportJobDirectory=$job;CanonicalFiles=@($pairs|ForEach-Object {$_[1]});WorkerOutputDirectory='worker-output';Scope='Worker filenames are transport only; canonical extension/header identifies object versus DLL'}|ConvertTo-Json|Set-Content "$run\module.json"
 Write-Output $run
} finally {$lock.Dispose()}
