param(
    [string]$Original = "$PSScriptRoot\..\original\fsx.exe",
    [string]$Pilot = "$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support\fsx.exe",
    [string]$Map = "$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support\fsx.map"
)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$pilotImageBase=(Open-PE $Pilot).Base
function Read-Range([string]$Path, [uint32]$Rva) {
    $bytes = [IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $Path))
    $pe = [BitConverter]::ToInt32($bytes, 60)
    $table = $pe + 24 + [BitConverter]::ToUInt16($bytes, $pe + 20)
    for ($i = 0; $i -lt [BitConverter]::ToUInt16($bytes, $pe + 6); $i++) {
        $section = $table + 40 * $i
        $va = [BitConverter]::ToUInt32($bytes, $section + 12)
        $size = [BitConverter]::ToUInt32($bytes, $section + 16)
        if ($Rva -ge $va -and ($Rva + 144) -le ($va + $size)) {
            $offset = [BitConverter]::ToUInt32($bytes, $section + 20) + $Rva - $va
            return ,([byte[]]$bytes[$offset..($offset + 143)])
        }
    }
    throw "Unmapped range: $Path RVA $Rva"
}
$mapText = Get-Content -LiteralPath $Map -Raw
function Symbol-VA([string]$Name) {
    $pattern = '(?m)^\s+[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+' + [regex]::Escape($Name) + '\s+([0-9a-fA-F]{8})\b'
    $match = [regex]::Match($mapText, $pattern)
    if (-not $match.Success) { throw "MAP symbol missing: $Name" }
    return [Convert]::ToUInt32($match.Groups[1].Value, 16)
}
# Fixed comparison scope: these two known 144-byte functions and five verified
# address operand fields. This is not a general-purpose disassembler or scorer.
$fields = @(
    @{ Offset=12; Name='___security_cookie'; Original=0x01004004; Relative=$false },
    @{ Offset=37; Name='_memset'; Original=0x01002c44; Relative=$true },
    @{ Offset=43; Name='__imp__VerSetConditionMask@16'; Original=0x01001010; Relative=$false },
    @{ Offset=125; Name='__imp__VerifyVersionInfoA@16'; Original=0x01001014; Relative=$false },
    @{ Offset=136; Name='@__security_check_cookie@4'; Original=0x0100263d; Relative=$true }
)
$results = foreach ($item in @(
    @{ Name='CheckWindows51SP2OrLater'; OriginalVA=0x01001760 },
    @{ Name='CheckWindows52SP1OrLater'; OriginalVA=0x01001800 }
)) {
    $pilotVA = Symbol-VA ('?' + $item.Name + '@@YGHXZ')
    $originalBytes = Read-Range $Original ($item.OriginalVA - 0x01000000)
    $pilotBytes = Read-Range $Pilot ($pilotVA - $pilotImageBase)
    $normalized = [byte[]]$pilotBytes.Clone()
    foreach ($field in $fields) {
        $offset = $field.Offset
        $originalTarget = [long][BitConverter]::ToUInt32($originalBytes, $offset)
        $pilotTarget = [long][BitConverter]::ToUInt32($pilotBytes, $offset)
        if ($field.Relative) {
            $originalTarget = [long]$item.OriginalVA + $offset + 4 + [BitConverter]::ToInt32($originalBytes, $offset)
            $pilotTarget = [long]$pilotVA + $offset + 4 + [BitConverter]::ToInt32($pilotBytes, $offset)
        }
        if ($originalTarget -ne $field.Original -or $pilotTarget -ne (Symbol-VA $field.Name)) {
            throw "Address mapping failed: $($item.Name) $($field.Name)"
        }
        [Array]::Copy($originalBytes, $offset, $normalized, $offset, 4)
    }
    $rawDifferences = @()
    $remaining = @()
    for ($i=0; $i -lt 144; $i++) {
        if ($originalBytes[$i] -ne $pilotBytes[$i]) { $rawDifferences += $i }
        if ($originalBytes[$i] -ne $normalized[$i]) { $remaining += $i }
    }
    [pscustomobject]@{
        Function=$item.Name; OriginalVA=('{0:X8}' -f $item.OriginalVA)
        PilotVA=('{0:X8}' -f $pilotVA); Length=144
        RawDifferingBytes=$rawDifferences.Count
        MappedAddressFieldsVerified=$fields.Count
        RemainingDifferingOffsets=$remaining
        AddressNormalizedExactMatch=($remaining.Count -eq 0)
    }
}
[pscustomobject]@{
    OriginalSHA256=(Get-FileHash -LiteralPath $Original -Algorithm SHA256).Hash
    PilotSHA256=(Get-FileHash -LiteralPath $Pilot -Algorithm SHA256).Hash
    Scope='Two 144-byte helper bodies; five address fields mapped through known original targets and pilot MAP symbols. Not full-file equality or runtime proof.'
    Results=@($results)
} | ConvertTo-Json -Depth 6
