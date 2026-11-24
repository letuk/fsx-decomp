param(
    [Parameter(Mandatory)][string]$BuildDirectory,
    [switch]$RequireSeparate
)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot/lib/pe-image.ps1"
$originalPath = "$PSScriptRoot/../original/fsx.exe"
$candidatePath = Join-Path $BuildDirectory 'fsx.exe'
$originalHash = (Get-FileHash -LiteralPath $originalPath).Hash
if ($originalHash -ne '61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735') {
    throw 'Original fsx.exe identity changed'
}
$original = Open-PE $originalPath
$candidate = Open-PE $candidatePath
Load-Imports $original
Load-Imports $candidate
$map = Get-Content (Join-Path $BuildDirectory 'fsx.map') -Raw
$symbol = '?InitializeMfcModuleState@@YGHHK@Z'
$sourceEntry = [regex]::Match($map, '(?m)^\s*\S+\s+' + [regex]::Escape($symbol) + '\s+([0-9A-Fa-f]{8})\s+f\s+afx_initialize\.obj\s*$')
$sdkEntry = [regex]::Match($map, '(?m)^\s*\S+\s+\?AfxInitialize@@YGHHK@Z\s+([0-9A-Fa-f]{8})\s+f\s+mfcs80:appmodul\.obj\s*$')
if (!$sourceEntry.Success -or !$sdkEntry.Success) { throw 'Expected source and SDK MAP symbols' }
$oldVA = 0x01002c0d
$newVA = [Convert]::ToInt64($sourceEntry.Groups[1].Value, 16)
$sdkVA = [Convert]::ToInt64($sdkEntry.Groups[1].Value, 16)
if ($RequireSeparate -and $newVA -eq $sdkVA) { throw 'Source and SDK bodies folded' }

function Resolve-References($pe, [long]$entry) {
    $body = Bytes $pe $entry 38
    if ($body[0] -ne 0xe8 -or $body[25] -ne 0xff -or $body[26] -ne 0x15 -or
        ([Convert]::ToHexString($body[35..37])) -ne 'C20800') {
        throw 'Unexpected call encodings or stdcall return'
    }
    $thunk = $entry + 5 + [BitConverter]::ToInt32($body, 1)
    $jump = Bytes $pe $thunk 6
    if ($jump[0] -ne 0xff -or $jump[1] -ne 0x25) { throw 'Expected import thunk' }
    $stateImport = $pe.Imports[[long][BitConverter]::ToUInt32($jump, 2)]
    $mbcpImport = $pe.Imports[[long][BitConverter]::ToUInt32($body, 27)]
    if ($stateImport -cne 'mfc80.dll!#1084' -or $mbcpImport -cne 'msvcr80.dll!_setmbcp') {
        throw "Unexpected dependencies $stateImport and $mbcpImport"
    }
    @(
        @{Offset=1; TargetVA=('{0:x8}' -f $thunk); Identity=$stateImport},
        @{Offset=27; TargetVA=('{0:x8}' -f [BitConverter]::ToUInt32($body, 27)); Identity=$mbcpImport}
    )
}

$oldReferences = @(Resolve-References $original $oldVA)
$newReferences = @(Resolve-References $candidate $newVA)
$oldBytes = Bytes $original $oldVA 38
$newBytes = Bytes $candidate $newVA 38
$normalized = $newBytes.Clone()
foreach ($reference in $oldReferences) {
    [Array]::Copy($oldBytes, $reference.Offset, $normalized, $reference.Offset, 4)
}
$remaining = @(for ($i = 0; $i -lt 38; $i++) { if ($oldBytes[$i] -ne $normalized[$i]) { $i } })
if ($remaining.Count) { throw "AfxInitialize body differs at offsets $remaining" }
[ordered]@{
    OriginalVA = '{0:x8}' -f $oldVA
    RebuiltVA = '{0:x8}' -f $newVA
    SdkVA = '{0:x8}' -f $sdkVA
    Symbol = $symbol
    OriginalSHA256 = $originalHash
    RebuiltSHA256 = (Get-FileHash -LiteralPath $candidatePath).Hash
    Length = 38
    OriginalHex = [Convert]::ToHexString($oldBytes).ToLowerInvariant()
    RebuiltHex = [Convert]::ToHexString($newBytes).ToLowerInvariant()
    OriginalReferences = $oldReferences
    RebuiltReferences = $newReferences
    FoldedWithSdk = $newVA -eq $sdkVA
    RemainingDifferingOffsets = $remaining
    AddressNormalizedExactMatch = $true
} | ConvertTo-Json -Depth 5
