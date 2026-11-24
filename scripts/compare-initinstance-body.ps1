param([Parameter(Mandatory)][string]$BuildDirectory)
# Strict linked-body comparison for the recovered 246-instruction layout.
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot/lib/pe-image.ps1"
$root = Split-Path $PSScriptRoot
$originalFile = "$root/original/fsx.exe"
$candidateFile = "$BuildDirectory/fsx.exe"
$originalHash = (Get-FileHash $originalFile).Hash
if ($originalHash -ne '61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735') {
    throw 'Original fsx.exe identity changed'
}
$old = Open-PE $originalFile
$new = Open-PE $candidateFile
Load-Imports $old
Load-Imports $new
$symbols = [Collections.Generic.Dictionary[string,long]]::new([StringComparer]::Ordinal)
$literals = @{}
foreach ($line in Get-Content "$BuildDirectory/fsx.map") {
    if ($line -match '^\s*\w{4}:\w{8}\s+(\S+)\s+([\da-fA-F]{8})\b') {
        $name = $Matches[1]
        $va = [Convert]::ToInt64($Matches[2],16)
        $symbols[$name] = $va
        if ($name.StartsWith('??_C@') -or $name -ceq '?emptyErrorCaption@@3QBDB') { $literals[$va] = $name }
    }
}
$entry = 0x010019e0L
$rebuilt = $symbols['?InitInstance@FSXAppPilot@@UAEHXZ']
$length = 769
$original = Bytes $old $entry $length
$candidate = Bytes $new $rebuilt $length
$normalized = $candidate.Clone()
$diagnostic = Get-Content "$BuildDirectory/initinstance-diagnostic.json" -Raw | ConvertFrom-Json
if ($diagnostic.RebuiltLength -ne $length -or [Convert]::ToInt64($diagnostic.RebuiltVA,16) -ne $rebuilt) {
    throw 'The rebuilt body does not span the expected 769 bytes at the MAP symbol'
}
$rebuiltOffsets = @($diagnostic.Rows | Where-Object RebuiltAddress | ForEach-Object {
    [Convert]::ToInt64($_.RebuiltAddress,16) - $rebuilt
})
$targets = @{}
foreach ($ref in (Get-Content "$BuildDirectory/initinstance-references.json" -Raw | ConvertFrom-Json).References) {
    $targets[[Convert]::ToInt64($ref.OriginalTarget,16)] = $ref.RebuiltSymbol
}
$targets[0x01004004L] = '___security_cookie'
$targets[0x01004388L] = '?g_mainWindowPilot@@3VCWnd@@A'
$targets[0x01004400L] = '?g_fsxAppPilot@@3VFSXAppPilot@@A'
$stringVAs = 0x01001304,0x01001314,0x01001324,0x01001338,0x01001348,0x01001350,0x01001360,0x010014c8

function Resolve-Reference([long]$left, [long]$right, [bool]$isCall) {
    if ($left -in $stringVAs) {
        if (!$literals.ContainsKey($right) -or (CString $old $left) -cne (CString $new $right)) {
            throw ('String contents or MAP literal differ at {0:X8}' -f $right)
        }
        return $literals[$right]
    }
    $symbol = $targets[$left]
    if (!$symbol -or !$symbols.ContainsKey($symbol)) { throw ('Unknown target {0:X8}' -f $left) }
    $expected = $symbols[$symbol]
    if ($left -eq 0x01004400) { $expected += 0x20 } # CWinThread::m_pMainWnd in the global application
    if ($right -ne $expected) { throw ('Expected {0} at {1:X8}, found {2:X8}' -f $symbol,$expected,$right) }
    if ($old.Imports.ContainsKey($left)) {
        if ($old.Imports[$left] -cne $new.Imports[$right]) { throw "Import identity differs for $symbol" }
        return $old.Imports[$left]
    }
    if ($isCall -and [BitConverter]::ToUInt16((Bytes $old $left 2),0) -eq 0x25ff) {
        $identity = $old.Imports[[long](U32 $old ($left+2))]
        if (!$identity -or [BitConverter]::ToUInt16((Bytes $new $right 2),0) -ne 0x25ff -or
            $new.Imports[[long](U32 $new ($right+2))] -cne $identity) { throw "Import thunk differs for $symbol" }
        return "$symbol -> $identity"
    }
    return $symbol
}

$fields = @()
$offset = 0
$index = 0
$listing = 'artifacts/fsx.exe/fsx-inventory/functions/010019e0.asm'
foreach ($line in Get-Content "$root/$listing") {
    if ($line -notmatch '^([0-9a-f]{8})  ([0-9a-f]+)  (.+)$') { throw 'Invalid original listing row' }
    $va = [Convert]::ToInt64($Matches[1],16)
    $hex = $Matches[2]
    $size = [int]($hex.Length/2)
    if ($va -ne $entry+$offset -or $rebuiltOffsets[$index] -ne $offset -or
        [BitConverter]::ToString($original,$offset,$size).Replace('-','') -ine $hex) {
        throw ('Listing or instruction boundary differs at original {0:X8}' -f $va)
    }
    # Only the listed x86 operand encodings carry references in this pinned body.
    $positions = @(switch -Regex ($hex) {
        '^e8[0-9a-f]{8}$|^a[13][0-9a-f]{8}$' { 1 }
        '^68[0-9a-f]{8}$' { if ([BitConverter]::ToUInt32($original,$offset+1) -ge $old.Base) { 1 } }
        '^(8b(0d|35|3d)|391d|ff15)[0-9a-f]{8}$' { 2 }
        '^c705[0-9a-f]{16}$' { 2; 6 }
    })
    foreach ($position in $positions) {
        $at = $offset + $position
        $relative = $original[$offset] -eq 0xe8
        if ($relative) {
            $left = $entry+$at+4+[BitConverter]::ToInt32($original,$at)
            $right = $rebuilt+$at+4+[BitConverter]::ToInt32($candidate,$at)
        } else {
            $left = [long][BitConverter]::ToUInt32($original,$at)
            $right = [long][BitConverter]::ToUInt32($candidate,$at)
        }
        $identity = Resolve-Reference $left $right $relative
        $fields += [pscustomobject]@{offset=$at; originalTarget=('{0:x8}' -f $left);
            rebuiltTarget=('{0:x8}' -f $right); relativeCall=$relative; identity=$identity}
        [Array]::Copy($original,$at,$normalized,$at,4)
    }
    $index++
    $offset += $size
}
if ($offset -ne $length -or $index -ne 246 -or $index -ne $rebuiltOffsets.Count -or $fields.Count -ne 44) {
    throw 'Incomplete listing or reference inventory'
}
$rawDifferences = @()
$remaining = @()
for ($i=0; $i -lt $length; $i++) {
    if ($original[$i] -ne $candidate[$i]) { $rawDifferences += $i }
    if ($original[$i] -ne $normalized[$i]) { $remaining += $i }
}
$status = if (!$rawDifferences.Count) { 'Exact' } elseif (!$remaining.Count) { 'Normalized' } else { 'Different' }
[ordered]@{status=$status; binary='fsx.exe'; originalVA='010019e0'; rebuiltVA=('{0:x8}' -f $rebuilt);
    originalSHA256=$originalHash; rebuiltSHA256=(Get-FileHash $candidateFile).Hash; bytes=$length;
    instructions=$index; sourceSymbol='FSXAppPilot::InitInstance'; listing=$listing;
    rawDifferingBytes=$rawDifferences.Count; rawDifferingOffsets=$rawDifferences; validatedFields=$fields;
    remainingDifferingOffsets=$remaining; scope='Complete linked method. All internal branches compared verbatim. Only the listed 32-bit target fields are normalized.'} | ConvertTo-Json -Depth 5
