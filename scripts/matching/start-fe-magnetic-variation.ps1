param(
    [Parameter(Mandatory)][ValidatePattern('^fe-[A-Za-z0-9_-]+$')][string]$RunId,
    [ValidateSet('Fast', 'Precise')][string]$Profile = 'Fast',
    [ValidateRange(1,900)][int]$TimeoutSeconds = 180
)

$ErrorActionPreference = 'Stop'
$project = [IO.Path]::GetFullPath("$PSScriptRoot\..\..")
$share = Join-Path $project 'vm-share'
$originalHash = (Get-FileHash "$project\original\fe.dll").Hash
if ($originalHash -cne 'BA04A13F5F6C377768B732E364A8AC4C3B61AFA54DAE824DF9614FA91532682C') {
    throw 'Original fe.dll identity changed'
}

$lock = [IO.File]::Open("$share\controller.lock", 'OpenOrCreate', 'ReadWrite', 'None')
try {
    $worker = Get-Content "$share\worker-state.json" -Raw | ConvertFrom-Json
    if ($worker.Status -ne 'ready' -or $worker.Job) { throw 'Worker not ready' }

    $run = "$share\modules\fe\runs\$RunId"
    $job = "$share\queue\$RunId"
    if ((Test-Path $run) -or (Test-Path $job)) { throw 'Choose a unique run ID' }
    New-Item -ItemType Directory -Path "$job\source", $run -Force | Out-Null
    $sourceHashes = [ordered]@{}
    foreach ($file in 'magnetic_variation.cpp', 'magnetic_grid.cpp') {
        Copy-Item "$project\src\fe\$file" "$job\source"
        $sourceHashes[$file] = (Get-FileHash "$job\source\$file").Hash
    }

    $build = [IO.File]::ReadAllText("$PSScriptRoot\guest-build-fe-magnetic-variation.cmd") -replace '\r?\n', "`r`n"
    if ($Profile -eq 'Precise') { $build = $build.Replace('/fp:fast', '/fp:precise') }
    [IO.File]::WriteAllText("$job\build.cmd", $build, [Text.Encoding]::ASCII)
    [ordered]@{
        Job = $RunId
        Module = 'fe.dll'
        ArtifactKind = 'PE32 DLL'
        Profile = $Profile
        OriginalSHA256 = $originalHash
        Sources = $sourceHashes
        BuildCommandSHA256 = (Get-FileHash "$job\build.cmd").Hash
        Compiler = '14.00.50727.42'
        Entries = @('203efc30', '203efc50', '203ec5d0', '203ec9a0', '203ecb90')
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
    foreach ($file in $sourceHashes.Keys) {
        if ((Get-FileHash "$run\worker-output\compiled-source\$file").Hash -cne $sourceHashes[$file]) {
            throw "Compiled source hash differs from the staged $file"
        }
    }
    if ((Get-Content "$run\worker-output\build-log.txt" -Raw) -notmatch '14\.00\.50727\.42\b') {
        throw 'Unexpected compiler version'
    }

    Copy-Item "$run\worker-output\fsx.exe" "$run\fe-magnetic-variation.dll"
    Copy-Item "$run\worker-output\fsx.map" "$run\fe-magnetic-variation.map"
    Copy-Item "$run\worker-output\fsx.pdb" "$run\fe-magnetic-variation.pdb"
    node "$project\scripts\compare-fe-magnetic-variation.cjs" $run
    if ($LASTEXITCODE -ne 0) { throw 'FE linked comparison failed' }
} finally {
    $lock.Dispose()
}
