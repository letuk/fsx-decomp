param(
    [string]$Original = "$PSScriptRoot\..\original\fsx.exe",
    [string]$Pilot = "$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support\fsx.exe",
    [string]$Disassembly = "$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support\disassembly.txt"
)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$pilotImageBase=(Open-PE $Pilot).Base
function Read-FormatterBody([string]$Path, [uint32]$Rva) {
    $b = [IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $Path))
    $pe = [BitConverter]::ToInt32($b,60)
    $table = $pe + 24 + [BitConverter]::ToUInt16($b,$pe+20)
    for ($i=0; $i -lt [BitConverter]::ToUInt16($b,$pe+6); $i++) {
        $s=$table+40*$i
        $va=[BitConverter]::ToUInt32($b,$s+12)
        $size=[BitConverter]::ToUInt32($b,$s+16)
        if ($Rva -ge $va -and ($Rva+92) -le ($va+$size)) {
            $offset=[BitConverter]::ToUInt32($b,$s+20)+$Rva-$va
            return ,([byte[]]$b[$offset..($offset+91)])
        }
    }
    throw 'Function range not mapped'
}
# DUMPBIN resolves this internal SDK symbol even when it is absent from MAP publics.
$listing=Get-Content -LiteralPath $Disassembly -Raw
$symbol=[regex]::Match($listing,'\?StringCchPrintfA@@YAJPADIPBDZZ:\r?\n\s+([0-9A-Fa-f]{8}):')
if (-not $symbol.Success) { throw 'SDK formatter entry not identified in disassembly' }
$pilotVA=[Convert]::ToUInt32($symbol.Groups[1].Value,16)
$a=Read-FormatterBody $Original 0x1590
$b=Read-FormatterBody $Pilot ($pilotVA-$pilotImageBase)
$rawDifferences=@(for ($i=0; $i -lt 92; $i++) { if ($a[$i] -ne $b[$i]) { $i } })
$originalIAT=[BitConverter]::ToUInt32($a,52)
$pilotIAT=[BitConverter]::ToUInt32($b,52)
$mapText=Get-Content -LiteralPath ([IO.Path]::ChangeExtension($Pilot,'map')) -Raw
$target=[regex]::Match($mapText,'(?m)^\s+[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+__imp___vsnprintf\s+([0-9a-fA-F]{8})\b')
if ($originalIAT -ne 0x01001208 -or -not $target.Success -or $pilotIAT -ne [Convert]::ToUInt32($target.Groups[1].Value,16)) { throw '_vsnprintf target mapping failed' }
$normalized=[byte[]]$b.Clone()
[Array]::Copy($a,52,$normalized,52,4)
$differences=@(for($i=0;$i -lt 92;$i++){if($a[$i] -ne $normalized[$i]){$i}})
[pscustomobject]@{
    OriginalSHA256=(Get-FileHash -LiteralPath $Original -Algorithm SHA256).Hash
    PilotSHA256=(Get-FileHash -LiteralPath $Pilot -Algorithm SHA256).Hash
    OriginalVA='01001590'; PilotVA=('{0:X8}' -f $pilotVA); Length=92
    DifferingOffsets=$differences; AddressNormalizedExactMatch=($differences.Count -eq 0); RawDifferingBytes=$rawDifferences.Count; MappedAddressFieldsVerified=1
    Scope='92-byte formatter body only; only _vsnprintf IAT operand at offset 52 mapped and target verified; not whole-file equality.'
} | ConvertTo-Json -Depth 4
