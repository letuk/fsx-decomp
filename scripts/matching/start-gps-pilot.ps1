param(
    [Parameter(Mandatory)][ValidatePattern('^gps-[A-Za-z0-9_-]+$')][string]$RunId,
    [ValidateRange(1,900)][int]$TimeoutSeconds = 180
)
$ErrorActionPreference = 'Stop'
$project = [IO.Path]::GetFullPath("$PSScriptRoot\..\..")
$share = Join-Path $project 'vm-share'
$originalHash = (Get-FileHash "$project\original\gps.dll").Hash
if ($originalHash -cne 'E36BE481BF8A6A8A0A56EA72D57961B169050F70DE401766C17FB374EE74E609') {
    throw 'Original gps.dll identity changed'
}

$lock = [IO.File]::Open("$share\controller.lock", 'OpenOrCreate', 'ReadWrite', 'None')
try {
    $worker = Get-Content "$share\worker-state.json" -Raw | ConvertFrom-Json
    if ($worker.Status -ne 'ready' -or $worker.Job) { throw 'Worker not ready' }
    $run = "$share\modules\gps\runs\$RunId"
    $job = "$share\queue\$RunId"
    if ((Test-Path $run) -or (Test-Path $job)) { throw 'Choose a unique run ID' }
    New-Item -ItemType Directory -Path "$job\source", $run -Force | Out-Null
    Copy-Item "$project\src\gps-pilot\registration.cpp" "$job\source"
    $build = [IO.File]::ReadAllText("$PSScriptRoot\guest-build-gps-object.cmd") -replace '\r?\n', "`r`n"
    [IO.File]::WriteAllText("$job\build.cmd", $build, [Text.Encoding]::ASCII)
    $sourceHash = (Get-FileHash "$job\source\registration.cpp").Hash
    @{
        Job = $RunId
        Module = 'gps.dll'
        ArtifactKind = 'COFF object'
        OriginalSHA256 = $originalHash
        SourceSHA256 = $sourceHash
        BuildCommandSHA256 = (Get-FileHash "$job\build.cmd").Hash
        Compiler = '14.00.50727.762'
        Entries = @('20855f20', '20855fb0', '20856050', '208560e0', '208561b0', '2086a380', '2086a4c0', '2086aa10')
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
    if ((Get-FileHash "$run\worker-output\compiled-source\registration.cpp").Hash -cne $sourceHash) {
        throw 'Compiled source hash differs from the staged source'
    }
    if ((Get-Content "$run\worker-output\build-log.txt" -Raw) -notmatch '14\.00\.50727\.762') {
        throw 'Unexpected compiler version'
    }
    Copy-Item "$run\worker-output\fsx.exe" "$run\gps-registration.obj"
    Copy-Item "$run\worker-output\fsx.pdb" "$run\gps-registration.pdb"
    node "$project\scripts\compare-gps-registration.cjs" $run
    if ($LASTEXITCODE -ne 0) { throw 'GPS COFF comparison failed' }
} finally {
    $lock.Dispose()
}
