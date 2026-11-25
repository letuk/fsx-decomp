param(
    [Parameter(Mandatory)][ValidatePattern('^sim1-[A-Za-z0-9_-]+$')][string]$RunId,
    [ValidateSet('Fast', 'Precise')][string]$Profile = 'Fast',
    [ValidateSet('Integration', 'Position', 'PositionLinked', 'ForcesLinked', 'ThrustLinked', 'PropulsionLinked', 'Turbine', 'Electrical', 'AerodynamicMoment')][string]$Target = 'Integration',
    [ValidateRange(1,900)][int]$TimeoutSeconds = 180
)

$ErrorActionPreference = 'Stop'
$project = [IO.Path]::GetFullPath("$PSScriptRoot\..\..")
$share = Join-Path $project 'vm-share'
$originalHash = (Get-FileHash "$project\original\sim1.dll").Hash
if ($originalHash -cne '18BE04C6A85A3CCF3C5DAE1411C652FA15CCBEA62E0913780A1B784A9D3F3FC8') {
    throw 'Original sim1.dll identity changed'
}

$component = 'state-integration'
$commandComponent = $component
$artifactKind = 'COFF object'
$sourceName = 'state_integration.cpp'
$entries = @('20c43110', '20c42d90', '20bdeec0', '20bdef30', '20bdefa0')
if ($Target -eq 'AerodynamicMoment') {
    $component = 'aerodynamic-moment'
    $commandComponent = $component
    $sourceName = 'aerodynamic_moment.cpp'
    $entries = @('20c013a0', '20c013e6', '20c0140b', '20c01438', '20c01462', '20c0147b', '20c01494', '20c014bc', '20c0155d', '20c016ad', '20c01733', '20c0177b', '20c01890', '20c01c55', '20c01fde', '20c02025', '20c020a0', '20c02265', '20c023cf', '20c02645', '20c02680', '20c02702', '20c4fcb0', '20c4ff30', '20c4fce0', '20c4fe00', '20c50020', '20c500c0', '20c501a0', '20c50270', '20c50290', '20c502b0', '20c502d0', '20c502f0', '20c50870', '20c50890', '20c50dc0', '20c50de0', '20c50e00', '20bfb840', '20bcbf60')
    $entries += @('20c00a00', '20c00d80', '20c00f40', '20c45510', '20c4564f', '20c4f640', '20c4fa50', '20c4fa90')
    $entries += @('20c00530', '20c0054f')
    if (!$PSBoundParameters.ContainsKey('Profile')) { $Profile = 'Precise' }
}
if ($Target -eq 'Turbine') {
    $component = 'turbine'
    $commandComponent = $component
    $sourceName = 'engine_forces.cpp'
    $entries = @('20c2bc20', '20c47420', '20c47720', '20c1cbf0', '20c48250', '20c48270', '20c485c0', '20c48710', '20c47ae0', '20bcc030', '20c47ba0', '20c47c90', '20c480b0', '20c48150')
    if (!$PSBoundParameters.ContainsKey('Profile')) { $Profile = 'Precise' }
}
if ($Target -eq 'Electrical') {
    $component = 'electrical-circuit'
    $commandComponent = $component
    $sourceName = 'electrical.cpp'
    $entries = @('20bfee30', '20bfef40')
    if (!$PSBoundParameters.ContainsKey('Profile')) { $Profile = 'Precise' }
}
if ($Target -in @('Position', 'PositionLinked')) {
    $component = 'position'
    $commandComponent = $component
    $sourceName = 'position_attitude.cpp'
    $entries = @('20bd1d90', '20bd1bd0', '20bd1c90')
    if (!$PSBoundParameters.ContainsKey('Profile')) { $Profile = 'Precise' }
}
if ($Target -eq 'PositionLinked') {
    $commandComponent = 'position-linked'
    $artifactKind = 'PE32 DLL'
}
if ($Target -eq 'ForcesLinked') {
    $component = 'forces'
    $commandComponent = 'forces-linked'
    $artifactKind = 'PE32 DLL'
    $sourceName = 'flight_forces.cpp'
    $entries = @('20bf6a20', '20bccae0', '20bccef0', '20c42d90')
    if (!$PSBoundParameters.ContainsKey('Profile')) { $Profile = 'Precise' }
}
if ($Target -in @('ThrustLinked', 'PropulsionLinked')) {
    $component = 'thrust'
    $commandComponent = 'thrust-linked'
    $artifactKind = 'PE32 DLL'
    $sourceName = 'engine_forces.cpp'
    $entries = @('20c01010')
    if ($Target -eq 'PropulsionLinked') {
        $component = 'propulsion'
        $entries += '20c010f0'
    }
    if (!$PSBoundParameters.ContainsKey('Profile')) { $Profile = 'Precise' }
}

$lock = [IO.File]::Open("$share\controller.lock", 'OpenOrCreate', 'ReadWrite', 'None')
try {
    $worker = Get-Content "$share\worker-state.json" -Raw | ConvertFrom-Json
    if ($worker.Status -ne 'ready' -or $worker.Job) { throw 'Worker not ready' }

    $run = "$share\modules\sim1\runs\$RunId"
    $job = "$share\queue\$RunId"
    if ((Test-Path $run) -or (Test-Path $job)) { throw 'Choose a unique run ID' }
    New-Item -ItemType Directory -Path "$job\source", $run -Force | Out-Null
    $sharedHeader = 'flight_dynamics.h'
    Copy-Item "$project\src\sim1\$sourceName", "$project\src\sim1\$sharedHeader" "$job\source"
    $sourceHash = (Get-FileHash "$job\source\$sourceName").Hash
    $headerHash = (Get-FileHash "$job\source\$sharedHeader").Hash
    $contextName = 'sim1-position-call-context.cpp'
    $contextHash = $null
    if ($Target -eq 'PositionLinked') {
        Copy-Item "$PSScriptRoot\$contextName" "$job\source"
        $contextHash = (Get-FileHash "$job\source\$contextName").Hash
    }
    $extraHashes = [ordered]@{}
    if ($Target -in @('Electrical', 'Turbine', 'ThrustLinked', 'PropulsionLinked')) {
        $electricalHeader = 'electrical.h'
        Copy-Item "$project\src\sim1\$electricalHeader" "$job\source"
        $extraHashes[$electricalHeader] = (Get-FileHash "$job\source\$electricalHeader").Hash
    }
    if ($Target -in @('AerodynamicMoment', 'Turbine', 'ThrustLinked', 'PropulsionLinked')) {
        $curveHeader = 'aerodynamic_curve.h'
        Copy-Item "$project\src\sim1\$curveHeader" "$job\source"
        $extraHashes[$curveHeader] = (Get-FileHash "$job\source\$curveHeader").Hash
    }
    if ($Target -in @('AerodynamicMoment', 'ThrustLinked', 'PropulsionLinked')) {
        $curveDefinition = 'aerodynamic_curve_body.h'
        Copy-Item "$project\src\sim1\$curveDefinition" "$job\source"
        $extraHashes[$curveDefinition] = (Get-FileHash "$job\source\$curveDefinition").Hash
    }
    if ($Target -in @('ThrustLinked', 'PropulsionLinked')) {
        $electricalSource = 'electrical.cpp'
        Copy-Item "$project\src\sim1\$electricalSource" "$job\source"
        $extraHashes[$electricalSource] = (Get-FileHash "$job\source\$electricalSource").Hash
        $curveContext = 'sim1-curve-call-context.cpp'
        Copy-Item "$PSScriptRoot\$curveContext" "$job\source"
        $extraHashes[$curveContext] = (Get-FileHash "$job\source\$curveContext").Hash
    }
    if ($Target -eq 'ForcesLinked') {
        foreach ($extraPath in "$project\src\sim1\flight_forces.h",
                               "$project\src\sim1\state_integration.cpp",
                               "$PSScriptRoot\sim1-force-call-context.cpp") {
            $extra = Split-Path $extraPath -Leaf
            Copy-Item $extraPath "$job\source"
            $extraHashes[$extra] = (Get-FileHash "$job\source\$extra").Hash
        }
    }

    $build = [IO.File]::ReadAllText("$PSScriptRoot\guest-build-sim1-$commandComponent.cmd") -replace '\r?\n', "`r`n"
    if ($Target -eq 'PropulsionLinked') {
        $build = $build.Replace('/DSIM1_THRUST_COMPARISON', '/DSIM1_PROPULSION_COMPARISON')
    }
    if ($Profile -eq 'Precise') { $build = $build.Replace('/fp:fast', '/fp:precise') }
    [IO.File]::WriteAllText("$job\build.cmd", $build, [Text.Encoding]::ASCII)
    [ordered]@{
        Job = $RunId
        Module = 'sim1.dll'
        ArtifactKind = $artifactKind
        Target = $Target
        Profile = $Profile
        OriginalSHA256 = $originalHash
        SourceSHA256 = $sourceHash
        HeaderSHA256 = $headerHash
        ContextSHA256 = $contextHash
        ExtraSHA256 = $extraHashes
        BuildCommandSHA256 = (Get-FileHash "$job\build.cmd").Hash
        Compiler = '14.00.50727.42'
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
    if ((Get-FileHash "$run\worker-output\compiled-source\$sourceName").Hash -cne $sourceHash -or
        (Get-FileHash "$run\worker-output\compiled-source\$sharedHeader").Hash -cne $headerHash) {
        throw 'Compiled source differs from the staged source'
    }
    if ($contextHash -and (Get-FileHash "$run\worker-output\compiled-source\$contextName").Hash -cne $contextHash) {
        throw 'Compiled comparison caller differs from the staged source'
    }
    foreach ($extra in $extraHashes.Keys) {
        if ((Get-FileHash "$run\worker-output\compiled-source\$extra").Hash -cne $extraHashes[$extra]) {
            throw "Compiled $extra differs from the staged source"
        }
    }
    if ((Get-Content "$run\worker-output\build-log.txt" -Raw) -notmatch '14\.00\.50727\.42\b') {
        throw 'Unexpected compiler version'
    }

    if ($artifactKind -eq 'PE32 DLL') {
        Copy-Item "$run\worker-output\fsx.exe" "$run\sim1-$component.dll"
        Copy-Item "$run\worker-output\fsx.map" "$run\sim1-$component.map"
    } else {
        Copy-Item "$run\worker-output\fsx.exe" "$run\sim1-$component.obj"
    }
    Copy-Item "$run\worker-output\fsx.pdb" "$run\sim1-$component.pdb"
    node "$project\scripts\compare-sim1-$component.cjs" $run
    if ($LASTEXITCODE -ne 0) { throw 'Sim1 comparison failed' }
    if ($Target -eq 'Turbine') {
        node "$project\scripts\compare-sim1-induced-airspeed.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 induced airspeed comparison failed' }
        node "$project\scripts\compare-sim1-propeller-sync.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 propeller synchronization comparison failed' }
        node "$project\scripts\compare-sim1-propeller-speed.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 propeller speed comparison failed' }
        node "$project\scripts\compare-sim1-propeller-load.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 propeller load comparison failed' }
        node "$project\scripts\compare-sim1-propeller-thrust.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 propeller thrust comparison failed' }
        node "$project\scripts\compare-sim1-propeller-pitch.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 propeller pitch comparison failed' }
        node "$project\scripts\compare-sim1-propeller-feather.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 propeller feather comparison failed' }
        node "$project\scripts\compare-sim1-propeller-ground-selector.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 propeller ground selector comparison failed' }
        node "$project\scripts\compare-sim1-propeller-mapped-pass.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 mapped propeller pass comparison failed' }
        node "$project\scripts\compare-sim1-propeller-load-factor.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 propeller load factor comparison failed' }
        node "$project\scripts\compare-sim1-automatic-feather.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 automatic feather comparison failed' }
    }
    if ($Target -eq 'AerodynamicMoment') {
        node "$project\scripts\compare-sim1-moment-arm.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 moment arm comparison failed' }
        node "$project\scripts\compare-sim1-angle-curve.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 angle curve comparison failed' }
        node "$project\scripts\compare-sim1-baseline-moment.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 baseline moment comparison failed' }
        node "$project\scripts\compare-sim1-elevator-moment.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 elevator moment comparison failed' }
        node "$project\scripts\compare-sim1-pitch-rate.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 pitch-rate comparison failed' }
        node "$project\scripts\compare-sim1-alpha-rate.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 angle-of-attack rate comparison failed' }
        node "$project\scripts\compare-sim1-tail-incidence.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 tail-incidence moment comparison failed' }
        node "$project\scripts\compare-sim1-elevator-trim.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 elevator trim moment comparison failed' }
        node "$project\scripts\compare-sim1-pitch-sum.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 pitch sum comparison failed' }
        node "$project\scripts\compare-sim1-roll-sum.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 roll sum comparison failed' }
        node "$project\scripts\compare-sim1-yaw-sum.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 yaw sum comparison failed' }
        node "$project\scripts\compare-sim1-aerodynamic-force-sum.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 aerodynamic force sum comparison failed' }
        node "$project\scripts\compare-sim1-load-scales.cjs" $run
        if ($LASTEXITCODE -ne 0) { throw 'Sim1 aerodynamic load scales comparison failed' }
    }
} finally {
    $lock.Dispose()
}
