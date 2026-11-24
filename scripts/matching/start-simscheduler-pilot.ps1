param(
    [Parameter(Mandatory)][ValidatePattern('^simscheduler-[A-Za-z0-9_-]+$')][string]$RunId,
    [ValidateRange(1,900)][int]$TimeoutSeconds = 180,
    [switch]$DispatcherLtcg,
    [switch]$DispatcherVc8Rtm,
    [switch]$TimeStateLtcg,
    [switch]$TimeStateWords,
    [switch]$TimeStateCopy,
    [ValidateSet('Frame','Copy')][string]$TimePropagation,
    [ValidateSet('Frame','Import')][string]$FlightSave,
    [ValidateSet('Import','PropagationCopy')][string]$FlightLoad,
    [switch]$ContextLtcg,
    [switch]$ContextFrame,
    [ValidateSet('Default','Frame')][string]$PendingRegistration,
    [switch]$EntryHelpersLtcg,
    [switch]$EntryInsertionLtcg,
    [switch]$ContextLifecycleFactory,
    [switch]$ContextVirtualAbi,
    [switch]$NativeContext,
    [ValidateSet('Default','Frame')][string]$ContextConstructorLtcg,
    [switch]$BorrowedContextStorage,
    [switch]$EarlyContextName,
    [switch]$ContextContainerInit,
    [switch]$InternalContextConstructor,
    [switch]$ContextUpdateLtcg,
    [ValidateSet('Exported','SeparateLtcg','SeparateCoff')][string]$ContextUpdateBoundary,
    [ValidateSet('Default','Aligned')][string]$FallbackContext,
    [ValidateSet('Default','FastFp')][string]$ContextAccessors,
    [switch]$SchedulerObjectAccessors,
    [switch]$ContextReset,
    [switch]$SchedulerObjectLifecycle,
    [switch]$PoolLifecycle,
    [switch]$PoolMetrics,
    [switch]$FreeList,
    [switch]$PoolLock,
    [switch]$UnwindHelpers,
    [switch]$ContextStorageCleanupLtcg,
    [switch]$RegistryUnwind,
    [ValidateSet('SP1','RTM','RTMSse2','RTMConversion')][string]$DllStartup
)
$ErrorActionPreference = 'Stop'
if ($TimeStateWords -or $TimeStateCopy) { $TimeStateLtcg = $true }
if ($ContextFrame) { $ContextLtcg = $true }
if ($EntryInsertionLtcg) { $EntryHelpersLtcg = $true }
if ($TimeStateWords -and $TimeStateCopy) { throw 'Select one time-state representation' }
if ($DispatcherVc8Rtm -and !($DispatcherLtcg -or $UnwindHelpers -or $ContextStorageCleanupLtcg)) {
    throw 'The RTM tool profile requires a dispatcher or unwind profile'
}
if (($BorrowedContextStorage -or $EarlyContextName -or $ContextContainerInit) -and !$ContextConstructorLtcg) {
    throw 'Constructor experiments require ContextConstructorLtcg'
}
if ($InternalContextConstructor -and ($ContextConstructorLtcg -ne 'Frame' -or !$BorrowedContextStorage)) {
    throw 'InternalContextConstructor requires the frame and borrowed-storage constructor profiles'
}
if ($ContextUpdateLtcg) {
    if ($BorrowedContextStorage -or $EarlyContextName -or $ContextContainerInit) {
        throw 'ContextUpdateLtcg builds the current source'
    }
    $ContextConstructorLtcg = 'Frame'
}
if ($ContextUpdateBoundary -and !$ContextUpdateLtcg) {
    throw 'ContextUpdateBoundary requires ContextUpdateLtcg'
}
if ($SchedulerObjectAccessors -and (
    $DispatcherLtcg -or $TimeStateLtcg -or $TimePropagation -or $FlightSave -or $FlightLoad -or
    $ContextLtcg -or $ContextConstructorLtcg -or $PendingRegistration -or $EntryHelpersLtcg -or
    $ContextLifecycleFactory -or $ContextVirtualAbi -or $NativeContext -or $FallbackContext -or
    $ContextAccessors -or $ContextReset -or $SchedulerObjectLifecycle -or $PoolLifecycle -or
    $PoolMetrics -or $FreeList -or $PoolLock -or $UnwindHelpers -or $ContextStorageCleanupLtcg -or $RegistryUnwind -or $DllStartup)) {
    throw 'Build SchedulerObjectAccessors in its own run'
}
$project = [IO.Path]::GetFullPath("$PSScriptRoot\..\..")
$share = Join-Path $project 'vm-share'
$compiler = if ($DispatcherVc8Rtm) { '14.00.50727.42' } else { '14.00.50727.762' }
$originalHash = (Get-FileHash "$project\original\simscheduler.dll").Hash
if ($originalHash -cne '16059B50E722BFA10357582DEF8D422A8222F620189E40C8878928A7DDAA9DD4') {
    throw 'Original simscheduler.dll identity changed'
}

$lock = [IO.File]::Open("$share\controller.lock", 'OpenOrCreate', 'ReadWrite', 'None')
try {
    $worker = Get-Content "$share\worker-state.json" -Raw | ConvertFrom-Json
    if ($worker.Status -ne 'ready' -or $worker.Job) { throw 'Worker not ready' }
    $run = "$share\modules\simscheduler\runs\$RunId"
    $job = "$share\queue\$RunId"
    if ((Test-Path $run) -or (Test-Path $job)) { throw 'Choose a unique run ID' }
    New-Item -ItemType Directory -Path "$job\source", $run -Force | Out-Null
    $source = 'scheduler_object.cpp'
    $profile = 'Cpp'
    $artifactKind = 'COFF object'
    $buildScript = 'guest-build-simscheduler-object.cmd'
    $entries = @('20cc14d0', '20cc14e0', '20cc1560', '20cc15b0', '20cc1580', '20cc1530', '20cc1500', '20cc1460', '20cc1490', '20cc1400', '20cc13c0', '20cc1b90', '20cc3490', '20cc41d0')
    $entries += @('20cc2ce0', '20cc2e80', '20cc4050', '20cc3b70', '20cc3c90')
    $entries += @('20cc35d0', '20cc3770', '20cc3e00')
    $entries += @('20cc3bd0', '20cc3b00', '20cc3c20', '20cc3f50', '20cc40d0', '20cc42c0', '20cc4320')
    $entries += @('20cc27a0', '20cc2940', '20cc28c0', '20cc15d0', '20cc28f0')
    $entries += @('20cc1670', '20cc1f10', '20cc20b0', '20cc2240')
    $entries += @('20cc2720', '20cc2740')
    if ($FallbackContext) { $entries += '20cc1cc0' }
    if ($ContextAccessors) { $entries += '20cc2a80' }
    if ($SchedulerObjectAccessors) {
        $source = 'scheduler_object_accessors.cpp'
        $profile = 'ObjectAccessorsCpp'
        $entries = @('20cc19a0', '20cc19c0', '20cc19e0', '20cc1a00', '20cc1a20', '20cc1ad0', '20cc1af0', '20cc1b10')
    }
    if ($ContextReset) { $entries += '20cc2e20' }
    if ($SchedulerObjectLifecycle) { $entries += '20cc1600' }
    if ($PoolLifecycle) { $entries += '20cc72d0' }
    if ($PoolMetrics) { $entries += '20cc3a60' }
    if ($FreeList) { $entries += '20cc4d90' }
    if ($PoolLock) { $entries += '20cc54b0' }
    if ($UnwindHelpers) { $entries += '20cc15f0', '20cc1ef0' }
    if ($RegistryUnwind) { $entries += '20cc17d0', '20cc18c0' }
    if ($DispatcherLtcg -or $TimeStateLtcg -or $TimePropagation -or $FlightSave -or $FlightLoad) {
        $source = 'scheduler_dispatch_ltcg.cpp'
        $profile = 'CppLtcg'
        $artifactKind = 'PE32 DLL fragment'
        $buildScript = 'guest-build-simscheduler-ltcg.cmd'
        $entries = @('20cc1400')
    }
    if ($TimeStateLtcg) {
        $profile = 'TimeStateLtcg'
        $entries = @('20cc1b30')
    }
    if ($TimePropagation) {
        $profile = 'TimePropagationLtcg'
        $entries = @('20cc1a40')
    }
    if ($FlightSave) {
        $profile = 'FlightSaveLtcg'
        $entries = @('20cc48d0')
    }
    if ($FlightLoad) {
        $profile = 'FlightLoadLtcg'
        $entries = @('20cc4790')
    }
    if ($DispatcherLtcg) {
        $profile = 'CppLtcg'
        $entries = @('20cc1400')
    }
    if ($ContextLtcg) {
        $profile = 'ContextLtcg'
        $artifactKind = 'PE32 DLL fragment'
        $buildScript = 'guest-build-simscheduler-ltcg.cmd'
        $source = 'scheduler_object.cpp'
        $entries = @('20cc1b90', '20cc3050', '20cc30d0')
    }
    if ($ContextConstructorLtcg) {
        $profile = 'ContextConstructorLtcg'
        $artifactKind = 'PE32 DLL fragment'
        $buildScript = 'guest-build-simscheduler-ltcg.cmd'
        $source = 'scheduler_object.cpp'
        $entries = @('20cc27a0', '20cc15d0')
    }
    if ($PendingRegistration) {
        $profile = 'PendingRegistration'
        $artifactKind = 'PE32 DLL fragment'
        $buildScript = 'guest-build-simscheduler-ltcg.cmd'
        $source = 'scheduler_object.cpp'
        $entries = @('20cc3490', '20cc41d0')
    }
    if ($EntryHelpersLtcg) {
        $profile = 'EntryHelpersLtcg'
        $artifactKind = 'PE32 DLL fragment'
        $buildScript = 'guest-build-simscheduler-ltcg.cmd'
        $source = 'scheduler_object.cpp'
        $entries = @('20cc4050', '20cc3b70', '20cc3c90')
    }
    if ($EntryInsertionLtcg) {
        $profile = 'EntryInsertionLtcg'
        $entries = @('20cc3bd0', '20cc3b00', '20cc3c20', '20cc3f50', '20cc40d0', '20cc42c0', '20cc4320')
    }
    if ($DllStartup) {
        $source = 'scheduler_dll_startup.cpp'
        $profile = "DllStartup$DllStartup"
        $artifactKind = 'PE32 DLL startup probe'
        $buildScript = 'guest-build-simscheduler-dll-startup.cmd'
        $entries = @('20cc6988','20cc686d','20cc66c9','20cc4980','20cc49c0','20cc6978','20cc6c08','20cc6c4d','20cc6c8e','20cc6677')
        $entries += @('20cc69c2','20cc6a58','20cc6a66','20cc6a89','20cc6a9a','20cc6ac3')
        $entries += @('20cc6af0','20cc6b20','20cc6b67')
        $entries += @('20cc6bd8','20cc6be4','20cc6bf0','20cc6bfc')
        $entries += '20cc6c66', '20cc69ae', '20cc6d27', '20cc6e6c'
        $entries += '20cc6e30', '20cc6e3c', '20cc6e48', '20cc6e54', '20cc6e60'
        if ($DllStartup -in 'RTMSse2','RTMConversion') { $entries += @('20cc6f67','20cc6fbc','20cc7021') }
    }
    if ($ContextStorageCleanupLtcg) {
        $profile = 'ContextStorageCleanupLtcg'
        $artifactKind = 'PE32 DLL fragment'
        $buildScript = 'guest-build-simscheduler-ltcg.cmd'
        $source = 'scheduler_context_storage_ltcg.cpp'
        $entries = @('20cc15f0', '20cc71e9')
    }
    $sourcePath = "$project\src\simscheduler-pilot\$source"
    if ($source -in 'scheduler_dispatch_ltcg.cpp', 'scheduler_context_storage_ltcg.cpp') {
        $sourcePath = "$project\src\simscheduler-pilot\probes\$source"
    }
    $probeSourceRun = $null
    $companionSourceHashes = @{}
    if ($ContextStorageCleanupLtcg) {
        $moduleRoot = "$project\src\simscheduler-pilot"
        $canonical = [IO.File]::ReadAllText("$moduleRoot\scheduler_object.cpp")
        $contextHeader = [IO.File]::ReadAllText("$moduleRoot\scheduler_context.h")
        $declarations = $canonical.IndexOf('extern "C" const SchedulerContextTable schedulerContextTable;')
        $constructorEnd = $canonical.IndexOf('// simscheduler.dll 0x20cc27a0 (FUN_20cc27a0).')
        $destructorStart = $canonical.IndexOf('__declspec(noinline) SchedulerContext::~SchedulerContext()')
        $destructorEnd = $canonical.IndexOf('SchedulerContext* SchedulerContext::destroy(unsigned int deleteFlags)')
        if ($destructorEnd -gt $destructorStart) {
            $destructorEnd = $canonical.LastIndexOf('#if !defined(MATCH_NATIVE_CONTEXT)', $destructorEnd)
        }
        $poolTypes = $canonical.IndexOf("struct SchedulerPoolLock;")
        $poolTypesEnd = $canonical.IndexOf('extern "C" SchedulerEntryPool* schedulerEntryPool;')
        $acquireFunction = $canonical.IndexOf('static __forceinline int InitializeSchedulerStorageBlock(')
        if ($acquireFunction -lt 0) { throw 'Pool storage initialization missing' }
        $acquireCode = $canonical.LastIndexOf('template <class Block>', $acquireFunction)
        $releaseFunction = $canonical.IndexOf('static __forceinline void ReturnSchedulerPoolStorage')
        if ($releaseFunction -lt 0) { throw 'Pool storage return function missing' }
        $releaseStart = $canonical.LastIndexOf('template <class Block>', $releaseFunction)
        $releaseEnd = $canonical.IndexOf('extern "C" SchedulerTimedEntry* AcquireSchedulerEntry()', $releaseFunction)
        $contextAcquire = $canonical.IndexOf('extern "C" SchedulerContext* AcquireSchedulerContextStorage()', $releaseEnd)
        $contextAcquireEnd = $canonical.IndexOf('extern "C" void ReleaseSchedulerContextStorage(SchedulerContext* context)', $contextAcquire)
        $poolOperations = $canonical.IndexOf('#include <windows.h>')
        $poolOperationsEnd = $canonical.IndexOf('extern "C" SchedulerPoolLock* CreateSchedulerPoolLock()', $poolOperations)
        if ($declarations -lt 0 -or $constructorEnd -le $declarations -or $poolTypes -lt 0 -or
            $poolTypesEnd -le $poolTypes -or $releaseStart -lt 0 -or $releaseEnd -le $releaseStart -or
            $poolOperations -lt 0 -or $poolOperationsEnd -le $poolOperations -or $acquireCode -lt 0 -or
            $contextAcquire -le $releaseEnd -or $contextAcquireEnd -le $contextAcquire -or
            $destructorStart -lt 0 -or $destructorEnd -le $destructorStart) {
            throw 'Context storage projection ranges are missing'
        }
        $contextHeader = $contextHeader.Replace('    SchedulerContext(const char* contextName);',
            "    SchedulerContext(const char* contextName);`r`n    static void* operator new(unsigned int size) throw();`r`n    static void operator delete(void* storage);")
        $contextHeader = $contextHeader.Replace('    SchedulerContext* destroy(unsigned int flags);',
            '    __declspec(dllexport) SchedulerContext* destroy(unsigned int flags);')
        $projection = @(
            '#include <string.h>'
            '#include "scheduler_object.h"'
            '#include "scheduler_context.h"'
            'struct SchedulerTimeObserver;'
            'extern "C" {'
            '__declspec(dllexport) SchedulerObject schedulerObject;'
            '__declspec(dllexport) SchedulerContext* currentSchedulerContext;'
            '__declspec(dllexport) SchedulerTimeObserver* simConnectFallbackSetting;'
            'void __stdcall UpdateSimConnectFallback(SchedulerContext*, SchedulerTimeObserver*);'
            'extern const SchedulerContextTable schedulerContextTable = {0};'
            '__declspec(dllimport) void __stdcall ReleaseSchedulerContainerStorage(void**);'
            '}'
            $canonical.Substring($declarations, $constructorEnd - $declarations)
            $canonical.Substring($destructorStart, $destructorEnd - $destructorStart)
            $canonical.Substring($poolTypes, $poolTypesEnd - $poolTypes)
            'extern "C" {'
            '__declspec(dllexport) SchedulerContextPool* schedulerContextPool;'
            '__declspec(dllexport) void* schedulerPendingAllocator;'
            '__declspec(dllimport) void __stdcall AllocateSchedulerPoolStorage(int, unsigned int, void**, void*);'
            '__declspec(dllimport) long __stdcall DecrementSchedulerPoolCount(long*);'
            '__declspec(dllimport) void __stdcall BreakOnSchedulerPoolFailure();'
            'unsigned int __stdcall AcquireSchedulerPoolLock(SchedulerPoolLock*, int, int, void*, int, unsigned int, int);'
            'unsigned int __stdcall ReleaseSchedulerPoolLock(SchedulerPoolLock*, int);'
            'SchedulerPoolFreeList* __stdcall CreateSchedulerPoolFreeList(int, int);'
            'int DestroySchedulerPoolFreeList(SchedulerPoolFreeList*);'
            'void* PopSchedulerPoolEntry(SchedulerPoolFreeList*);'
            'int __stdcall PushSchedulerPoolEntry(SchedulerPoolFreeList*, void*);'
            '__declspec(dllimport) long __stdcall IncrementSchedulerPoolCount(long*);'
            '}'
            $canonical.Substring($acquireCode, $releaseStart - $acquireCode).
                Replace('ReleaseSchedulerPoolStorage(&storage)', 'ReleaseSchedulerContainerStorage(&storage)')
            $canonical.Substring($releaseStart, $releaseEnd - $releaseStart)
            $canonical.Substring($contextAcquire, $contextAcquireEnd - $contextAcquire).
                Replace('extern "C" SchedulerContext* AcquireSchedulerContextStorage()', 'extern "C" __declspec(noinline) SchedulerContext* AcquireSchedulerContextStorage()')
            'extern "C" { volatile signed char schedulerAtomicMode = 2; }'
            '#pragma optimize("y", off)'
            $canonical.Substring($poolOperations, $poolOperationsEnd - $poolOperations).
                Replace('extern "C" unsigned int __stdcall AcquireSchedulerPoolLock(', 'extern "C" __declspec(noinline) unsigned int __stdcall AcquireSchedulerPoolLock(').
                Replace('extern "C" unsigned int __stdcall ReleaseSchedulerPoolLock(', 'extern "C" __declspec(noinline) unsigned int __stdcall ReleaseSchedulerPoolLock(')
            '#pragma optimize("y", on)'
        ) -join "`r`n"
        [IO.File]::WriteAllText("$job\source\scheduler_context.h", $contextHeader, [Text.Encoding]::ASCII)
        [IO.File]::WriteAllText("$job\source\scheduler_context_storage_projection.h", $projection, [Text.Encoding]::ASCII)
        Copy-Item -LiteralPath "$moduleRoot\scheduler_object.h", $sourcePath -Destination "$job\source"
        foreach ($file in 'scheduler_context.h', 'scheduler_context_storage_projection.h', 'scheduler_object.h') {
            $companionSourceHashes[$file] = (Get-FileHash "$job\source\$file").Hash
        }
        $canonicalSourceHash = (Get-FileHash "$moduleRoot\scheduler_object.cpp").Hash
    } elseif ($ContextVirtualAbi) {
        $profile = 'ContextVirtualAbi'
        $entries = @()
        $sourcePath = "$project\artifacts\simscheduler.dll\context-virtual-abi-probe.cpp"
        Copy-Item -LiteralPath $sourcePath -Destination "$job\source\$source"
    } elseif ($ContextConstructorLtcg -and ($BorrowedContextStorage -or $EarlyContextName -or $ContextContainerInit)) {
        # Replay the experimental source without its conditional branches in the workspace code.
        $probeSourceRun = 'simscheduler-constructor-container-20261117-001'
        if ($InternalContextConstructor) { $probeSourceRun = 'simscheduler-constructor-internal-20261117-002' }
        $archive = "$share\modules\simscheduler\runs\$probeSourceRun"
        $archivedRequest = Get-Content "$archive\request.json" -Raw | ConvertFrom-Json
        $archivedSource = "$archive\worker-output\compiled-source\$source"
        if ($archivedRequest.Job -cne $probeSourceRun -or
            $archivedRequest.DispatcherProfile -ne 'ContextConstructorLtcg' -or
            $archivedRequest.OriginalSHA256 -cne $originalHash -or
            (Get-FileHash -LiteralPath $archivedSource).Hash -cne $archivedRequest.SourceSHA256) {
            throw "Archived constructor source does not match its request: $probeSourceRun"
        }
        Copy-Item -LiteralPath $archivedSource -Destination "$job\source\$source"
        $canonicalSourceHash = $archivedRequest.CanonicalSourceSHA256
    } elseif ($ContextConstructorLtcg) {
        # VC8 resolves the full LTCG input before REF can discard unused code.
        # Keep the constructor, its types and its cleanup dependencies from the same source.
        $canonical = [IO.File]::ReadAllText($sourcePath)
        $declarationsEnd = $canonical.IndexOf('#if defined(MATCH_CONTEXT_LTCG)')
        $codeStart = $canonical.IndexOf('extern "C" const SchedulerContextTable schedulerContextTable;')
        $codeEnd = $canonical.IndexOf('__declspec(noinline) SchedulerContext::~SchedulerContext()')
        if ($declarationsEnd -lt 0 -or $codeStart -le $declarationsEnd -or $codeEnd -le $codeStart) {
            throw 'Constructor source sections could not be located'
        }
        $probe = @(
            '#include <string.h>'
            $canonical.Substring(0, $declarationsEnd)
            'extern "C" {'
            '__declspec(dllexport) SchedulerObject schedulerObject;'
            'extern const SchedulerContextTable schedulerContextTable = {0};'
            '__declspec(dllimport) void __stdcall ReleaseSchedulerContainerStorage(void** buffer);'
            '}'
            $canonical.Substring($codeStart, $codeEnd - $codeStart)
        ) -join "`r`n"
        if ($ContextUpdateLtcg) {
            $updateStart = $canonical.IndexOf('// The first fastcall register is unused by the original helper.')
            $updateEnd = $canonical.IndexOf('extern "C" void __stdcall SetSchedulerSecondaryMode(')
            if ($updateStart -lt 0 -or $updateEnd -le $updateStart) { throw 'Update source range missing' }
            if ($ContextUpdateBoundary -like 'Separate*') {
                $helperStart = $canonical.IndexOf('extern "C" __declspec(noinline) void __fastcall PropagateSchedulerTime(', $updateStart)
                $setterStart = $canonical.IndexOf('// Original: simscheduler.dll 0x20cc2c40.', $updateStart)
                if ($helperStart -le $updateStart -or $setterStart -le $helperStart -or $setterStart -ge $updateEnd) {
                    throw 'Propagation helper source range missing'
                }
                $header = $canonical.Substring($updateStart, $helperStart - $updateStart) +
                    "extern `"C`" void __fastcall PropagateSchedulerTime(void*, SchedulerContext*, SchedulerPropagationTime);`r`n"
                $helper = "#include <string.h>`r`n#include `"scheduler_context.h`"`r`n#include `"scheduler_update_probe.h`"`r`n" +
                    $canonical.Substring($helperStart, $setterStart - $helperStart)
                [IO.File]::WriteAllText("$job\source\scheduler_update_probe.h", $header, [Text.Encoding]::ASCII)
                [IO.File]::WriteAllText("$job\source\scheduler_update_helper.cpp", $helper, [Text.Encoding]::ASCII)
                foreach ($file in 'scheduler_update_probe.h', 'scheduler_update_helper.cpp') {
                    $companionSourceHashes[$file] = (Get-FileHash "$job\source\$file").Hash
                }
                $probe += "`r`n#include `"scheduler_update_probe.h`"`r`n" +
                    $canonical.Substring($setterStart, $updateEnd - $setterStart)
            } else {
                $probe += "`r`n" + $canonical.Substring($updateStart, $updateEnd - $updateStart)
            }
        }
        $canonicalSourceHash = (Get-FileHash -LiteralPath $sourcePath).Hash
        [IO.File]::WriteAllText("$job\source\$source", $probe, [Text.Encoding]::ASCII)
    } else {
        Copy-Item -LiteralPath $sourcePath -Destination "$job\source"
    }
    $sourceHeaders = @{}
    if (!$probeSourceRun -and !$ContextVirtualAbi -and $source -in 'scheduler_object.cpp', 'scheduler_object_accessors.cpp') {
        $headers = @('scheduler_object.h')
        if ($source -eq 'scheduler_object.cpp') { $headers += 'scheduler_context.h' }
        foreach ($header in $headers) {
            Copy-Item -LiteralPath "$project\src\simscheduler-pilot\$header" -Destination "$job\source"
            $sourceHeaders[$header] = (Get-FileHash "$job\source\$header").Hash
        }
    }
    $build = [IO.File]::ReadAllText("$PSScriptRoot\$buildScript") -replace '\r?\n', "`r`n"
    if ($SchedulerObjectAccessors) {
        $build = $build.Replace('scheduler_object.', 'scheduler_object_accessors.').Replace('/Oy ', '/Oy- ')
    }
    if ($NativeContext) {
        if ($sourcePath -ne "$project\src\simscheduler-pilot\scheduler_object.cpp" -or
            $artifactKind -ne 'COFF object' -or $ContextLifecycleFactory -or $profile -ne 'Cpp') {
            throw 'NativeContext requires the standalone native COFF profile'
        }
        $profile = 'NativeContext'
        $entries = @()
        $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_NATIVE_CONTEXT /DMATCH_LIFECYCLE_FACTORY /c scheduler_object.cpp')
    }
    if ($DllStartup -like 'RTM*') {
        $crtHash = (Get-FileHash -LiteralPath "$share\toolchains\vc80-rtm\msvcrt.lib").Hash
        if ($crtHash -cne 'B1456D9A7E17865E3DC35663E0377BE4C7B8BE97A372210CD064F9F5635DFFEA') {
            throw 'Pinned RTM CRT library identity differs'
        }
        $build = $build.Replace('link /NOLOGO', 'link /NOLOGO "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\msvcrt.lib"')
    }
    if ($DllStartup -in 'RTMSse2','RTMConversion') {
        $build = $build.Replace('/OPT:NOICF', '/OPT:NOICF /INCLUDE:___sse2_available')
    }
    if ($DllStartup -eq 'RTMConversion') {
        $entries += @('20cc6e86','20cc6ea0','20cc6f50','20cc6f5c')
        $build = $build.Replace('/OPT:NOICF', '/OPT:NOICF /INCLUDE:__ftol2_sse /INCLUDE:_memcpy /INCLUDE:___CxxFrameHandler3 /INCLUDE:_memset')
    }
    if ($ContextLifecycleFactory) {
        $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_LIFECYCLE_FACTORY /c scheduler_object.cpp')
    }
    if ($TimeStateLtcg) {
        $build = $build.Replace('/c scheduler_dispatch_ltcg.cpp', '/DMATCH_TIME_STATE /c scheduler_dispatch_ltcg.cpp')
    }
    if ($TimeStateWords) {
        $build = $build.Replace('/c scheduler_dispatch_ltcg.cpp', '/DMATCH_TIME_STATE_WORDS /c scheduler_dispatch_ltcg.cpp')
    }
    if ($TimeStateCopy) {
        $build = $build.Replace('/c scheduler_dispatch_ltcg.cpp', '/DMATCH_TIME_STATE_COPY /c scheduler_dispatch_ltcg.cpp')
    }
    if ($TimePropagation) {
        $defines = '/DMATCH_PROPAGATION_FRAME'
        if ($TimePropagation -eq 'Copy') { $defines += ' /DMATCH_PROPAGATION_COPY' }
        $build = $build.Replace('/c scheduler_dispatch_ltcg.cpp', "$defines /c scheduler_dispatch_ltcg.cpp")
    }
    if ($FlightSave) {
        $defines = '/DMATCH_FLIGHT_SAVE_FRAME'
        if ($FlightSave -eq 'Import') { $defines += ' /DMATCH_FLIGHT_SAVE_IMPORT' }
        $build = $build.Replace('/c scheduler_dispatch_ltcg.cpp', "$defines /c scheduler_dispatch_ltcg.cpp")
    }
    if ($FlightLoad) {
        $defines = '/DMATCH_FLIGHT_LOAD_IMPORT'
        if ($FlightLoad -eq 'PropagationCopy') { $defines += ' /DMATCH_PROPAGATION_FRAME /DMATCH_PROPAGATION_COPY' }
        $build = $build.Replace('/c scheduler_dispatch_ltcg.cpp', "$defines /c scheduler_dispatch_ltcg.cpp")
    }
    if ($FlightSave -eq 'Import' -or $FlightLoad) {
        $importCommands = @(
            '>flight-probe.def echo LIBRARY flight.dll'
            '>>flight-probe.def echo EXPORTS'
            if ($FlightLoad) { '>>flight-probe.def echo readFlightState@16 @13 NONAME' }
            if ($FlightSave -eq 'Import') { '>>flight-probe.def echo WriteFlightState@16 @14 NONAME' }
            'lib /NOLOGO /MACHINE:X86 /DEF:flight-probe.def /OUT:flight-probe.lib'
            'if errorlevel 1 exit 4'
        ) -join "`r`n"
        $build = $build.Replace('link /NOLOGO', "$importCommands`r`nlink /NOLOGO")
        $build = $build.Replace('kernel32.lib', 'kernel32.lib flight-probe.lib')
    }
    if ($ContextLtcg) {
        $build = $build.Replace('scheduler_dispatch_ltcg', 'scheduler_object')
        $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_CONTEXT_LTCG /c scheduler_object.cpp')
        if ($ContextFrame) { $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_CONTEXT_FRAME /c scheduler_object.cpp') }
        $importCommands = @(
            '>scheduler-api.def echo LIBRARY api.dll'
            '>>scheduler-api.def echo EXPORTS'
            '>>scheduler-api.def echo schedulerApiState @304 NONAME DATA'
            '>>scheduler-api.def echo DispatchSchedulerApiEvent@12 @233 NONAME'
            'lib /NOLOGO /MACHINE:X86 /DEF:scheduler-api.def /OUT:scheduler-api.lib'
            'if errorlevel 1 exit 4'
        ) -join "`r`n"
        $build = $build.Replace('link /NOLOGO', "$importCommands`r`nlink /NOLOGO")
        $build = $build.Replace('kernel32.lib', 'kernel32.lib scheduler-api.lib')
    }
    if ($ContextConstructorLtcg) {
        $build = $build.Replace('scheduler_dispatch_ltcg', 'scheduler_object')
        $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_LIFECYCLE_FACTORY /c scheduler_object.cpp')
        if ($BorrowedContextStorage) {
            $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_BORROWED_CONTEXT_STORAGE /c scheduler_object.cpp')
        }
        if ($EarlyContextName) {
            $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_EARLY_CONTEXT_NAME /c scheduler_object.cpp')
        }
        if ($ContextContainerInit) {
            $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_CONTEXT_CONTAINER_INIT /c scheduler_object.cpp')
        }
        if ($ContextConstructorLtcg -eq 'Frame') {
            $build = $build.Replace('/Oy ', '/Oy- ')
            if ($build -notmatch '/Oy- ') { throw 'Constructor frame flag missing from build command' }
        }
        $importCommands = @(
            '>scheduler-api.def echo LIBRARY api.dll'
            '>>scheduler-api.def echo EXPORTS'
            '>>scheduler-api.def echo CopySchedulerContextName@12 @157 NONAME'
            '>>scheduler-api.def echo ReleaseSchedulerContainerStorage@4 @108 NONAME'
            'lib /NOLOGO /MACHINE:X86 /DEF:scheduler-api.def /OUT:scheduler-api.lib'
            'if errorlevel 1 exit 4'
        ) -join "`r`n"
        $build = $build.Replace('link /NOLOGO', "$importCommands`r`nlink /NOLOGO")
        $constructorExport = '_ConstructSchedulerContext@8'
        if ($InternalContextConstructor) {
            $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_INTERNAL_CONTEXT_CONSTRUCTOR /c scheduler_object.cpp')
            $constructorExport = '_InitializeBorrowedContext@12'
        }
        $build = $build.Replace('kernel32.lib', "kernel32.lib scheduler-api.lib /EXPORT:$constructorExport")
        if ($ContextUpdateLtcg) { $build = $build.Replace("/EXPORT:$constructorExport", "/EXPORT:$constructorExport /EXPORT:_SetSchedulerUpdateEnabled@8") }
        if ($ContextUpdateBoundary -eq 'Exported') {
            $build = $build.Replace('/EXPORT:_SetSchedulerUpdateEnabled@8', '/EXPORT:_SetSchedulerUpdateEnabled@8 /EXPORT:@PropagateSchedulerTime@16')
        }
        if ($ContextUpdateBoundary -like 'Separate*') {
            $compile = [regex]::Match($build, '(?m)^cl /nologo [^\r\n]* /c scheduler_object\.cpp(?=\r?$)').Value
            if (!$compile) { throw 'Update helper compile command missing' }
            $compile = $compile.Replace('scheduler_object.cpp', 'scheduler_update_helper.cpp')
            if ($ContextUpdateBoundary -eq 'SeparateCoff') { $compile = $compile.Replace('/GL ', '/GL- ') }
            $build = $build.Replace('link /NOLOGO', "$compile`r`nif errorlevel 1 exit 3`r`nlink /NOLOGO")
            $build = $build.Replace('scheduler_object.obj kernel32.lib', 'scheduler_object.obj scheduler_update_helper.obj kernel32.lib')
        }
    }
    if ($PendingRegistration) {
        $defines = '/DMATCH_PENDING_LTCG'
        if ($PendingRegistration -eq 'Frame') { $defines += ' /DMATCH_PENDING_FRAME' }
        $build = $build.Replace('scheduler_dispatch_ltcg', 'scheduler_object')
        $build = $build.Replace('/c scheduler_object.cpp', "$defines /c scheduler_object.cpp")
        $importCommands = @(
            '>scheduler-api.def echo LIBRARY api.dll'
            '>>scheduler-api.def echo EXPORTS'
            '>>scheduler-api.def echo AllocatePendingStorage@16 @106 NONAME'
            '>>scheduler-api.def echo ReleasePendingStorage@4 @108 NONAME'
            '>>scheduler-api.def echo MovePendingStorage@12 @110 NONAME'
            'lib /NOLOGO /MACHINE:X86 /DEF:scheduler-api.def /OUT:scheduler-api.lib'
            'if errorlevel 1 exit 4'
        ) -join "`r`n"
        $build = $build.Replace('link /NOLOGO', "$importCommands`r`nlink /NOLOGO")
        $build = $build.Replace('kernel32.lib', 'kernel32.lib scheduler-api.lib')
    }
    if ($EntryHelpersLtcg) {
        $build = $build.Replace('scheduler_dispatch_ltcg', 'scheduler_object')
        $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_ENTRY_HELPERS_LTCG /c scheduler_object.cpp')
        $importCommands = @(
            '>scheduler-api.def echo LIBRARY api.dll'
            '>>scheduler-api.def echo EXPORTS'
            '>>scheduler-api.def echo MovePendingStorage@12 @110 NONAME'
        )
        if ($EntryInsertionLtcg) {
            $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_INSERTION_LTCG /c scheduler_object.cpp')
            $importCommands += @(
                '>>scheduler-api.def echo AllocateSchedulerContainerStorage@16 @106 NONAME'
                '>>scheduler-api.def echo ReleaseSchedulerContainerStorage@4 @108 NONAME'
            )
        }
        $importCommands += @(
            'lib /NOLOGO /MACHINE:X86 /DEF:scheduler-api.def /OUT:scheduler-api.lib'
            'if errorlevel 1 exit 4'
        )
        $importCommands = $importCommands -join "`r`n"
        $build = $build.Replace('link /NOLOGO', "$importCommands`r`nlink /NOLOGO")
        $build = $build.Replace('kernel32.lib', 'kernel32.lib scheduler-api.lib')
    }
    if ($FallbackContext -eq 'Aligned') {
        $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_FALLBACK_ALIGNED /c scheduler_object.cpp')
    }
    if ($UnwindHelpers) {
        $build = $build.Replace('/c scheduler_object.cpp', '/DMATCH_UNWIND_HELPERS /c scheduler_object.cpp')
    }
    if ($ContextStorageCleanupLtcg) {
        $build = $build.Replace('scheduler_dispatch_ltcg', 'scheduler_context_storage_ltcg')
        $importCommands = @(
            '>scheduler-api.def echo LIBRARY api.dll'
            '>>scheduler-api.def echo EXPORTS'
            '>>scheduler-api.def echo CopySchedulerContextName@12 @157 NONAME'
            '>>scheduler-api.def echo ReleaseSchedulerContainerStorage@4 @108 NONAME'
            '>>scheduler-api.def echo AllocateSchedulerPoolStorage@16 @106 NONAME'
            'lib /NOLOGO /MACHINE:X86 /DEF:scheduler-api.def /OUT:scheduler-api.lib'
            'if errorlevel 1 exit 4'
            '>scheduler-storage-boundary.def echo LIBRARY scheduler-storage-boundary.dll'
            '>>scheduler-storage-boundary.def echo EXPORTS'
            '>>scheduler-storage-boundary.def echo UpdateSimConnectFallback@8'
            'lib /NOLOGO /MACHINE:X86 /DEF:scheduler-storage-boundary.def /OUT:scheduler-storage-boundary.lib'
            'if errorlevel 1 exit 4'
        ) -join "`r`n"
        $build = $build.Replace('link /NOLOGO', "$importCommands`r`nlink /NOLOGO")
        $poolImports = @(
            '/ALTERNATENAME:__imp__BreakOnSchedulerPoolFailure@0=__imp__DebugBreak@0'
            '/ALTERNATENAME:__imp__IncrementSchedulerPoolCount@4=__imp__InterlockedIncrement@4'
            '/ALTERNATENAME:__imp__DecrementSchedulerPoolCount@4=__imp__InterlockedDecrement@4'
        ) -join ' '
        $build = $build.Replace('kernel32.lib', "kernel32.lib scheduler-api.lib scheduler-storage-boundary.lib $poolImports")
    }
    if ($ContextAccessors -ne '' -and $ContextAccessors -ne 'Default') {
        $define = '/DMATCH_CONTEXT_ACCESSORS_' + $ContextAccessors.ToUpperInvariant()
        $build = $build.Replace('/c scheduler_object.cpp', "$define /c scheduler_object.cpp")
    }
    $rtmToolHashes = $null
    if ($DispatcherVc8Rtm) {
        $rtmToolHashes = @{}
        $rtmManifest = Get-Content "$project\artifacts\shared\toolchain-media\vc8-rtm-x86-tool-identities.json" -Raw | ConvertFrom-Json
        foreach ($tool in $rtmManifest) {
            $hash = (Get-FileHash "$share\toolchains\vc80-rtm\x86\$($tool.file)").Hash
            if ($hash -cne $tool.sha256) { throw "RTM tool identity differs: $($tool.file)" }
            $rtmToolHashes[$tool.file] = $hash
        }
        $rtmSetup = @(
            'mkdir vc8-rtm'
            'copy /Y "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\x86\*.*" vc8-rtm\'
            'if errorlevel 1 exit 2'
            'mkdir vc8-rtm\1033'
            'copy /Y vc8-rtm\*ui.dll vc8-rtm\1033\'
            'if errorlevel 1 exit 2'
            'set "PATH=%CD%\vc8-rtm;%PATH%"'
        ) -join "`r`n"
        $build = $build.Replace("`r`ncl`r`n", "`r`n$rtmSetup`r`ncl`r`n")
    }
    [IO.File]::WriteAllText("$job\build.cmd", $build, [Text.Encoding]::ASCII)
    $sourceHash = (Get-FileHash "$job\source\$source").Hash
    @{
        Job = $RunId
        Module = 'simscheduler.dll'
        ArtifactKind = $artifactKind
        OriginalSHA256 = $originalHash
        SourceSHA256 = $sourceHash
        SourceHeaderSHA256 = $sourceHeaders
        CompanionSourceSHA256 = $companionSourceHashes
        ContextUpdateBoundary = $ContextUpdateBoundary
        CanonicalSourceSHA256 = $canonicalSourceHash
        ProbeSourceRun = $probeSourceRun
        BuildCommandSHA256 = (Get-FileHash "$job\build.cmd").Hash
        Compiler = $compiler
        RtmToolSHA256 = $rtmToolHashes
        DispatcherProfile = $profile
        LifecycleFactory = [bool]($ContextLifecycleFactory -or $ContextConstructorLtcg)
        ConstructorFrame = $ContextConstructorLtcg
        CrtLibrarySHA256 = $crtHash
        Entries = $entries
    } | ConvertTo-Json | Set-Content "$job\request.json"
    Copy-Item "$job\request.json", "$job\build.cmd" $run
    Copy-Item "$job\source" $run -Recurse
    Set-Content "$job\ready" $RunId

    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    while (!(Test-Path "$job\done")) {
        if ([DateTime]::UtcNow -ge $deadline) { throw "Worker timeout for $RunId. Inspect the retained job before retrying." }
        Start-Sleep -Seconds 1
    }
    Copy-Item "$job\receipt.json" $run
    Copy-Item "$job\output" "$run\worker-output" -Recurse
    $receipt = Get-Content "$run\receipt.json" -Raw | ConvertFrom-Json
    if ($receipt.Job -cne $RunId -or $receipt.ExitCode -ne 0) { throw 'Worker build failed. See retained receipt and log.' }
    if ((Get-FileHash "$run\worker-output\compiled-source\$source").Hash -cne $sourceHash) {
        throw 'Compiled source hash differs from the staged source'
    }
    foreach ($header in $sourceHeaders.Keys) {
        if ((Get-FileHash "$run\worker-output\compiled-source\$header").Hash -cne $sourceHeaders[$header]) {
            throw "Compiled header hash differs from the staged header: $header"
        }
    }
    foreach ($file in $companionSourceHashes.Keys) {
        if ((Get-FileHash "$run\worker-output\compiled-source\$file").Hash -cne $companionSourceHashes[$file]) {
            throw "Compiled companion source differs from the staged source: $file"
        }
    }
    $compilerPattern = '(?<![\d.])' + [regex]::Escape($compiler) + '(?![\d.])'
    if ((Get-Content "$run\worker-output\build-log.txt" -Raw) -notmatch $compilerPattern) {
        throw "Expected compiler version $compiler"
    }
    if ($DispatcherVc8Rtm -and (Get-Content "$run\worker-output\disassembly.txt" -Raw) -notmatch '(?<![\d.])8\.00\.50727\.42(?![\d.])') {
        throw 'Expected VC8 RTM DUMPBIN'
    }
    if ($DllStartup) {
        Copy-Item "$run\worker-output\fsx.exe" "$run\simscheduler-startup.dll"
        node "$project\scripts\compare-simscheduler-dll-startup.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'SimScheduler DLL startup comparison failed' }
        return
    }
    if ($DispatcherLtcg -or $TimeStateLtcg -or $TimePropagation -or $FlightSave -or $FlightLoad -or $ContextLtcg -or $ContextConstructorLtcg -or $ContextStorageCleanupLtcg -or $PendingRegistration -or $EntryHelpersLtcg) {
        Copy-Item "$run\worker-output\fsx.exe" "$run\simscheduler-dispatch.dll"
        node "$project\scripts\compare-simscheduler-dispatch-ltcg.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'SimScheduler linked dispatcher comparison failed' }
        return
    }
    Copy-Item "$run\worker-output\fsx.exe" "$run\simscheduler-object.obj"
    Copy-Item "$run\worker-output\fsx.pdb" "$run\simscheduler-object.pdb"
    if ($ContextVirtualAbi) {
        return
    }
    if ($NativeContext) {
        node "$project\scripts\compare-simscheduler-native-context.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Native context ABI comparison failed' }
        return
    }
    node "$project\scripts\compare-simscheduler-object.cjs" $run
    if ($LASTEXITCODE -ne 0) { throw 'SimScheduler COFF comparison failed' }
} finally {
    $lock.Dispose()
}

