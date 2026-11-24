param(
    [string]$Original = "$PSScriptRoot\..\original\fsx.exe",
    [string]$Pilot = "$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support\fsx.exe",
    [string]$Disassembly = "$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support\disassembly.txt"
)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$pilotImageBase=(Open-PE $Pilot).Base
function Read-Body([string]$Path, [uint32]$Rva) {
    $b = [IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $Path))
    $pe = [BitConverter]::ToInt32($b,60)
    $table = $pe + 24 + [BitConverter]::ToUInt16($b,$pe+20)
    for ($i=0; $i -lt [BitConverter]::ToUInt16($b,$pe+6); $i++) {
        $s=$table+40*$i
        $va=[BitConverter]::ToUInt32($b,$s+12)
        $size=[BitConverter]::ToUInt32($b,$s+16)
        if ($Rva -ge $va -and ($Rva+72) -le ($va+$size)) {
            $offset=[BitConverter]::ToUInt32($b,$s+20)+$Rva-$va
            return ,([byte[]]$b[$offset..($offset+71)])
        }
    }
    throw 'Function range not mapped'
}
# DUMPBIN resolves this internal SDK symbol even when it is absent from MAP publics.
$listing=Get-Content -LiteralPath $Disassembly -Raw
$symbol=[regex]::Match($listing,'\?StringCopyWorkerA@@YGJPADIPBD@Z:\r?\n\s+([0-9A-Fa-f]{8}):')
if (-not $symbol.Success) { throw 'SDK worker entry not identified in disassembly' }
$pilotVA=[Convert]::ToUInt32($symbol.Groups[1].Value,16)
$a=Read-Body $Original 0x1600
$b=Read-Body $Pilot ($pilotVA-$pilotImageBase)
$differences=@(for ($i=0; $i -lt 72; $i++) { if ($a[$i] -ne $b[$i]) { $i } })
[pscustomobject]@{
    OriginalSHA256=(Get-FileHash -LiteralPath $Original -Algorithm SHA256).Hash
    PilotSHA256=(Get-FileHash -LiteralPath $Pilot -Algorithm SHA256).Hash
    OriginalVA='01001600'; PilotVA=('{0:X8}' -f $pilotVA); Length=72
    DifferingOffsets=$differences; RawExactMatch=($differences.Count -eq 0)
    Scope='72-byte copy-worker body only; no bytes normalized or masked; not whole-file equality.'
} | ConvertTo-Json -Depth 4
