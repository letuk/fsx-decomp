param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference = 'Stop'

$ledger = (. "$PSScriptRoot/inventory-writable-data.ps1" -BuildDirectory $BuildDirectory) |
    ConvertFrom-Json
$report = Get-Content (Join-Path $BuildDirectory 'comparison.json') -Raw | ConvertFrom-Json
$old = $report.PeImports.OriginalHeader.Sections | Where-Object Name -CEQ '.data'
$new = $report.PeImports.RebuiltHeader.Sections | Where-Object Name -CEQ '.data'
Require ($a.Base -eq $b.Base -and $old.RVA -eq $new.RVA -and
    $old.VirtualSize -eq $new.VirtualSize -and $old.RawSize -eq $new.RawSize -and
    $old.RawOffset -eq $new.RawOffset -and $old.Flags -ceq $new.Flags) 'Writable section geometry differs'
Require ($old.VirtualSize -ge $old.RawSize) 'Unexpected writable section extent'

$entry = $a.Base + $old.RVA
$oldBytes = Bytes $a $entry $old.RawSize
$newBytes = Bytes $b $entry $new.RawSize
$atl = $ledger.ValidatedRanges | Where-Object Role -CEQ 'ATL cached codepage getter'
$oldPointerVA = [Convert]::ToInt64($atl.OriginalVA, 16)
$newPointerVA = [Convert]::ToInt64($atl.RebuiltVA, 16)
$offset = $oldPointerVA - $entry
Require ($atl.Bytes -eq 4 -and $oldPointerVA -eq $newPointerVA -and
    $offset -ge 0 -and $offset + 4 -le $old.RawSize) 'ATL pointer field placement differs'
$oldTarget = [BitConverter]::ToUInt32($oldBytes, $offset)
$newTarget = [BitConverter]::ToUInt32($newBytes, $offset)
Require ($report.AtlCodepage.CachedGetterInitialTargetVerified -and
    $oldTarget -eq [Convert]::ToUInt32($report.AtlCodepage.Thunk.OriginalVA, 16) -and
    $newTarget -eq [Convert]::ToUInt32($report.AtlCodepage.Thunk.RebuiltVA, 16)) 'ATL pointer target differs'

$rawDifferences = @(for ($i = 0; $i -lt $old.RawSize; $i++) {
    if ($oldBytes[$i] -ne $newBytes[$i]) { $i }
})
$normalized = $newBytes.Clone()
[Array]::Copy($oldBytes, $offset, $normalized, $offset, 4)
Equal-Bytes $oldBytes $normalized 'Complete file-backed writable section'

[ordered]@{
    OriginalSHA256 = $ledger.OriginalSHA256
    RebuiltSHA256 = $ledger.RebuiltSHA256
    Section = $old
    Status = if (!$rawDifferences.Count) { 'Exact' } else { 'Normalized' }
    RawDifferingOffsets = $rawDifferences
    ValidatedFields = @(@{
        Offset = $offset
        OriginalTarget = '{0:x8}' -f $oldTarget
        RebuiltTarget = '{0:x8}' -f $newTarget
        Identity = 'ATL lazy selecting thunk'
    })
    ImplicitZeroFillBytes = $old.VirtualSize - $old.RawSize
    ShiftedObjects = @($ledger.ValidatedRanges | Where-Object Delta -ne 0)
    Scope = 'Complete equal-geometry writable section initial bytes and virtual zero-fill. Individual symbol placement and runtime values are separate.'
} | ConvertTo-Json -Depth 8
