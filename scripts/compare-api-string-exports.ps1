param(
    [Parameter(Mandatory=$true)][string]$ControlRun,
    [Parameter(Mandatory=$true)][string]$LtcgRun,
    [Parameter(Mandatory=$true)][string]$Evidence,
    [string[]]$VariantRuns = @()
)
. "$PSScriptRoot/lib/pe-image.ps1"

function Require($condition, [string]$message) {
    if (!$condition) { throw $message }
}

function ExportEntry($image, [int]$ordinal) {
    $pe = [BitConverter]::ToInt32($image.Bytes, 60)
    Require (($image.Bytes[$pe + 23] -band 0x20) -ne 0) 'Expected DLL image'
    $dir = $image.Base + [BitConverter]::ToUInt32($image.Bytes, $pe + 120)
    $slot = $ordinal - (U32 $image ($dir + 16))
    Require ($slot -ge 0 -and $slot -lt (U32 $image ($dir + 20))) 'Ordinal out of range'
    $table = $image.Base + (U32 $image ($dir + 28))
    return $image.Base + (U32 $image ($table + 4 * $slot))
}

function Hex($bytes) { return [BitConverter]::ToString([byte[]]$bytes).Replace('-', '').ToLowerInvariant() }

function RelativeTarget($image, [long]$address) {
    $data = Bytes $image $address 5
    Require ($data[0] -eq 0xe8) 'Expected direct CALL'
    return $address + 5 + [BitConverter]::ToInt32($data, 1)
}

$originalPath = "$PSScriptRoot/../original/api.dll"
$originalHash = (Get-FileHash $originalPath).Hash
Require ($originalHash -eq '9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003') 'Original API changed'
$original = Open-PE $originalPath
$originalCopy = ExportEntry $original 157
$originalAppend = ExportEntry $original 218
Require ($originalCopy -eq 0x20137e17 -and $originalAppend -eq 0x20137ec0) 'Unexpected original export entries'
Require ((RelativeTarget $original ($originalAppend + 50)) -eq $originalCopy) 'Original append target differs'
Load-Imports $original
$expectedImports = @('msvcr80.dll!_getmbcp', 'kernel32.dll!lstrcpynA', 'msvcr80.dll!_ismbblead')
$originalOffsets = @(12, 31, 66)
for ($i = 0; $i -lt $originalOffsets.Count; $i++) {
    $call = Bytes $original ($originalCopy + $originalOffsets[$i]) 6
    Require ((Hex $call[0..1]) -eq 'ff15') 'Expected original IAT CALL'
    Require ($original.Imports[[long][BitConverter]::ToUInt32($call, 2)] -eq $expectedImports[$i]) 'Original import differs'
}
$originalAppendBody = Bytes $original $originalAppend 62

$results = foreach ($run in @($ControlRun, $LtcgRun)) {
    $file = Join-Path $run 'api-pilot.dll'
    $image = Open-PE $file
    $copy = ExportEntry $image 157
    $append = ExportEntry $image 218
    $copyBody = Bytes $image $copy 157
    $appendBody = Bytes $image $append 62
    Require ((Hex $copyBody[154..156]) -eq 'c20c00' -and (Hex $appendBody[59..61]) -eq 'c20c00') 'Expected public RET 0x0c'
    Require ((RelativeTarget $image ($append + 50)) -eq $copy) 'Append must call the public copy entry'
    $normalized = [byte[]]$appendBody.Clone()
    [Array]::Copy($originalAppendBody, 51, $normalized, 51, 4)
    Require ((Hex $normalized) -ceq (Hex $originalAppendBody)) 'Append body differs after target mapping'
    Load-Imports $image
    $offsets = @(5, 16, 52)
    $opcodes = @('ff15', 'ff25', 'ff15')
    for ($i = 0; $i -lt $offsets.Count; $i++) {
        $call = Bytes $image ($copy + $offsets[$i]) 6
        Require ((Hex $call[0..1]) -eq $opcodes[$i]) 'Unexpected copy import instruction'
        Require ($image.Imports[[long][BitConverter]::ToUInt32($call, 2)] -eq $expectedImports[$i]) 'Rebuilt import differs'
    }
    @{
        run = Split-Path $run -Leaf
        dllSHA256 = (Get-FileHash $file).Hash
        copy = @{entry = ('{0:x8}' -f $copy); bytes = 157; hex = Hex $copyBody; singleBytePath = 'tail JMP'}
        append = @{entry = ('{0:x8}' -f $append); bytes = 62; hex = Hex $appendBody; mappedOffsets = @(51)}
    }
}
Require ($results[0].copy.hex -ceq $results[1].copy.hex) 'Copy differs between profiles'
Require ($results[0].append.hex -ceq $results[1].append.hex) 'Append differs between profiles'
foreach ($name in 'resolved_path_copy.cpp', 'path_append.cpp', 'string_exports_probe.cpp') {
    $left = (Get-FileHash "$ControlRun/worker-output/compiled-source/$name").Hash
    $right = (Get-FileHash "$LtcgRun/worker-output/compiled-source/$name").Hash
    Require ($left -eq $right) "Compiled source differs: $name"
}
$report = @{
    module = 'api.dll'; originalSHA256 = $originalHash
    original = @{
        copy = @{entry = '20137e17'; bytes = 169; hex = Hex (Bytes $original $originalCopy 169)}
        append = @{entry = '20137ec0'; bytes = 62; hex = Hex $originalAppendBody}
    }
    builds = $results
    scope = 'Raw equal linked bodies across SP1 profiles. Append matches original after validated REL32 mapping. Copy differs from original.'
}
if ($VariantRuns.Count) {
    $report.variants = @(foreach ($run in $VariantRuns) {
        $file = Join-Path $run 'api-pilot.dll'
        $image = Open-PE $file
        Load-Imports $image
        $copy = ExportEntry $image 157
        $append = ExportEntry $image 218
        Require ((RelativeTarget $image ($append + 50)) -eq $copy) 'Variant append target differs'
        $appendBody = Bytes $image $append 62
        [Array]::Copy($originalAppendBody, 51, $appendBody, 51, 4)
        Require ((Hex $appendBody) -ceq (Hex $originalAppendBody)) 'Variant append body differs'

        $listing = Get-Content "$run/worker-output/disassembly.txt" -Raw
        $block = [regex]::Match($listing, '(?ms)^_CopyResolvedPathPilot@12:\r?\n(.*?)(?=^\S.*?:\r?$)')
        Require $block.Success 'Copy symbol missing'
        $instructions = @()
        $length = 0
        foreach ($line in ($block.Groups[1].Value -split '\r?\n')) {
            if ($line -notmatch '^\s*([0-9A-F]{8}):\s+((?:[0-9A-F]{2} )*[0-9A-F]{2})\s{2,}(.+)$') { continue }
            $va = [Convert]::ToInt64($Matches[1], 16)
            $hex = ($Matches[2] -replace ' ', '').ToLowerInvariant()
            if ($hex -match '^(cc)+$') { break }
            $instruction = $Matches[3].Trim()
            Require ($va -eq ($copy + $length)) 'Noncontiguous copy listing'
            Require ((Hex (Bytes $image $va ($hex.Length / 2))) -ceq $hex) 'Listing differs from linked image'
            $instructions += ,@($va.ToString('x8'), $hex, $instruction)
            $length += $hex.Length / 2
        }
        Require ($length -gt 0 -and @($instructions | Where-Object {$_[1] -eq 'c20c00'}).Count -gt 0) 'Expected public RET 0x0c'
        $imports = @(foreach ($row in $instructions) {
            if ($row[1] -match '^ff(15|25)[0-9a-f]{8}$') {
                $address = [Convert]::ToInt64($row[0], 16)
                [pscustomobject]@{ name = $image.Imports[[long](U32 $image ($address + 2))]; opcode = $row[1].Substring(0, 4) }
            }
        })
        Require ((($imports | ForEach-Object {$_.name}) -join ',') -eq ($expectedImports -join ',')) 'Unexpected variant copy imports'
        $copyHex = Hex (Bytes $image $copy $length)
        $prefixMatch = $null
        $originalPrefix = Bytes $original $originalCopy 42
        if ($length -ge 42) {
            $prefix = Bytes $image $copy 42
            $prefixOffsets = @(14, 33)
            $prefixTargetsValid = $true
            for ($i = 0; $i -lt $prefixOffsets.Count; $i++) {
                $offset = $prefixOffsets[$i]
                if ((Hex $prefix[($offset - 2)..($offset - 1)]) -ne 'ff15' -or
                    $image.Imports[[long][BitConverter]::ToUInt32($prefix, $offset)] -ne $expectedImports[$i]) {
                    $prefixTargetsValid = $false
                }
            }
            if ($prefixTargetsValid) {
                foreach ($offset in $prefixOffsets) {
                    [Array]::Copy($originalPrefix, $offset, $prefix, $offset, 4)
                }
                if ((Hex $prefix) -ceq (Hex $originalPrefix)) {
                    $prefixMatch = @{bytes = 42; mappedOffsets = $prefixOffsets; targets = $expectedImports[0..1]}
                }
            }
        }
        @{
            run = Split-Path $run -Leaf
            dllSHA256 = (Get-FileHash $file).Hash
            copySourceSHA256 = (Get-FileHash "$run/worker-output/compiled-source/resolved_path_copy.cpp").Hash
            entry = ('{0:x8}' -f $copy); bytes = $length; hex = $copyHex
            sameAsControl = ($copyHex -ceq $results[0].copy.hex)
            singleBytePath = $(if ($imports[1].opcode -eq 'ff25') {'tail JMP'} else {'CALL'})
            normalizedEntryMatch = $prefixMatch
            appendMatches = $true
            rebuilt = $instructions
        }
    })
}
[IO.File]::WriteAllText([IO.Path]::GetFullPath($Evidence), ($report | ConvertTo-Json -Depth 8) + [Environment]::NewLine)
Write-Output 'Both linked bodies are raw equal across profiles. Append matches original after mapping its call; copy retains the tail jump.'
