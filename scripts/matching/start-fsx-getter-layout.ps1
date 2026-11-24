# Replay the packaged RTM comparison. New source proposals go through start-search.ps1.
param([Parameter(Mandatory)][ValidatePattern('^[A-Za-z0-9_-]+$')][string]$RunId)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path "$PSScriptRoot/../..").Path
$share = "$root/vm-share"
$evidence = "$root/artifacts/fsx.exe/getter-placement-20261119"
$verified = "$evidence/verified"
$job = "$share/queue/$RunId"
. "$root/scripts/lib/pe-image.ps1"

$lock = [IO.File]::Open("$share/controller.lock", 'OpenOrCreate', 'ReadWrite', 'None')
try {
    if (Test-Path -LiteralPath $job) { throw "Job already exists: $RunId" }
    if ((Get-FileHash "$verified/build.cmd").Hash -ne 'FF0F5B0538F63FD3D62FB71DEC3678A83C7EB11167BBA028500B65058F7934BA') {
        throw 'Packaged RTM recipe changed'
    }
    $inputs = Get-Content "$verified/source-hashes.json" -Raw | ConvertFrom-Json
    New-Item -ItemType Directory "$job/source" | Out-Null
    foreach ($inputFile in $inputs) {
        $name = $inputFile.file
        $dir = if ([IO.Path]::GetExtension($name) -in '.cpp','.h') { 'compiled-source' } else { 'packaging-inputs' }
        $source = "$verified/$dir/$name"
        if ((Get-FileHash -LiteralPath $source).Hash -ne $inputFile.Hash) { throw "Input changed: $name" }
        Copy-Item -LiteralPath $source -Destination "$job/source"
    }
    $command = [IO.File]::ReadAllText("$verified/build.cmd").Replace('getter-replay-20261119', $RunId)
    [IO.File]::WriteAllText("$job/build.cmd", $command, [Text.Encoding]::ASCII)
    Copy-Item "$verified/source-hashes.json" $job
    Set-Content "$job/ready" $RunId

    $deadline = [DateTime]::UtcNow.AddMinutes(3)
    while (!(Test-Path "$job/done")) {
        if ([DateTime]::UtcNow -ge $deadline) { throw "Worker timeout: $RunId" }
        Start-Sleep -Seconds 1
    }
    $receipt = Get-Content "$job/receipt.json" -Raw | ConvertFrom-Json
    if ($receipt.Job -cne $RunId -or $receipt.ExitCode -ne 0) { throw "Worker failed: $job/output/build-log.txt" }
    $output = "$job/output"
    foreach ($source in Get-ChildItem "$verified/compiled-source" -File) {
        if ((Get-FileHash "$output/compiled-source/$($source.Name)").Hash -ne (Get-FileHash $source.FullName).Hash) {
            throw "Guest source changed: $($source.Name)"
        }
    }
    if (!(Select-String -LiteralPath "$output/build-log.txt" -Pattern 'Compiler Version 14\.00\.50727\.42\b' -Quiet) -or
        !(Select-String -LiteralPath "$output/headers.txt" -Pattern 'File Type: EXECUTABLE IMAGE' -Quiet) -or
        !(Select-String -LiteralPath "$output/fsx.map" -Pattern '\?pfnGetApplication@@\S+\s+01004384\s+application_interface_pilot\.obj' -Quiet)) {
        throw 'Unexpected compiler, artifact type or getter map entry'
    }
    $rows = Get-Content "$evidence/audit.json" -Raw | ConvertFrom-Json
    if ((Get-FileHash "$verified/fsx.exe").Hash -ne $rows.rebuiltSHA256) { throw 'Archived comparison image changed' }
    $pe = Open-PE "$output/fsx.exe"
    $reference = Open-PE "$verified/fsx.exe"
    $ranges = @($rows.rows) + @(@{entry='0100127c';bytes=668}, @{entry='01004000';bytes=512})
    foreach ($range in $ranges) {
        $va = [Convert]::ToInt64($range.entry, 16)
        $left = Bytes $reference $va $range.bytes
        $right = Bytes $pe $va $range.bytes
        if ([Convert]::ToBase64String($left) -cne [Convert]::ToBase64String($right)) {
            throw "Archived comparison changed at $($range.entry)"
        }
    }
    Write-Output "Verified $RunId against the packaged RTM code and data ranges. Output: $output"
} finally {
    $lock.Dispose()
}
