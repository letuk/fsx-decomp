param([string]$BuildDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support")
$ErrorActionPreference='Stop'
$original="$PSScriptRoot\..\original\fsx.exe"
$pilot=Join-Path $BuildDirectory 'fsx.exe'
. "$PSScriptRoot\lib\pe-image.ps1"
$pilotImageBase=(Open-PE $pilot).Base
$mapText=Get-Content (Join-Path $BuildDirectory 'fsx.map') -Raw
$listing=Get-Content (Join-Path $BuildDirectory 'disassembly.txt') -Raw
function Read-Bytes([string]$Path,[uint32]$Rva,[int]$Length){
    $b=[IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $Path))
    $pe=[BitConverter]::ToInt32($b,60)
    $table=$pe+24+[BitConverter]::ToUInt16($b,$pe+20)
    for($i=0;$i -lt [BitConverter]::ToUInt16($b,$pe+6);$i++){
        $s=$table+40*$i;$va=[BitConverter]::ToUInt32($b,$s+12)
        if($Rva -ge $va -and ($Rva+$Length) -le ($va+[BitConverter]::ToUInt32($b,$s+16))){
            $o=[BitConverter]::ToUInt32($b,$s+20)+$Rva-$va
            return ,([byte[]]$b[$o..($o+$Length-1)])
        }
    }
    throw 'Unmapped PE range'
}
function Symbol([string]$Name){
    $m=[regex]::Match($mapText,'(?m)^\s+\w{4}:\w{8}\s+'+[regex]::Escape($Name)+'\s+([0-9a-fA-F]{8})\b')
    if(-not $m.Success){throw "MAP symbol missing: $Name"}
    return [Convert]::ToUInt32($m.Groups[1].Value,16)
}
$entry=Symbol '_ShowErrorMessage@24'
$body=[regex]::Match($listing,'(?ms)^_ShowErrorMessage@24:\r?\n(.*?)(?=^\S.*?:\s*\r?$|\z)').Groups[1].Value
$returns=[regex]::Matches($body,'(?im)^\s*([\da-f]{8}): C2 18 00\s+ret\s+18h\s*$')
if(!$returns.Count -or [Convert]::ToUInt32($returns[-1].Groups[1].Value,16)+3 -ne $entry+437){
    throw 'Rebuilt callback does not end at the expected 437-byte boundary'
}
$a=Read-Bytes $original 0x1d30 437
$b=Read-Bytes $pilot ($entry-$pilotImageBase) 437
$n=[byte[]]$b.Clone()
# Offset, original target, reference kind, rebuilt target symbol.
$fields=@(
    @(0x0f,0x01004004,'absolute','___security_cookie'),
    @(0x75,0x01002c44,'relative','_memset'),
    @(0x8d,0x01001224,'absolute','__imp__LoadStringA@16'),
    @(0xa4,0x01001590,'relative','?StringCchPrintfA@@YAJPADIPBDZZ'),
    @(0xb7,0x01001600,'copy',''),
    @(0xc3,0x01001384,'string',''),
    @(0xd4,0x01001590,'relative','?StringCchPrintfA@@YAJPADIPBDZZ'),
    @(0xe4,0x01004384,'absolute','?pfnGetApplication@@3P6APAVApplicationPilot@@XZA'),
    @(0x103,0x0100121c,'absolute','__imp__GetLastActivePopup@4'),
    @(0x139,0x010020b0,'relative','?AfxGetModuleState@@YGPAVAFX_MODULE_STATE@@XZ'),
    @(0x159,0x01001220,'absolute','__imp__MessageBoxIndirectA@4'),
    @(0x174,0x01001034,'absolute','__imp__CreateThread@24'),
    @(0x183,0x01001038,'absolute','__imp__WaitForSingleObject@8'),
    @(0x18f,0x0100103c,'absolute','__imp__GetExitCodeThread@8'),
    @(0x196,0x0100100c,'absolute','__imp__CloseHandle@4'),
    @(0x1ab,0x0100263d,'relative','@__security_check_cookie@4')
)
$validated=@()
foreach($field in $fields){
    $o=$field[0];$kind=$field[2]
    $at=[long][BitConverter]::ToUInt32($a,$o);$bt=[long][BitConverter]::ToUInt32($b,$o)
    if($kind -in 'relative','copy'){
        $at=0x01001d30L+$o+4+[BitConverter]::ToInt32($a,$o)
        $bt=[long]$entry+$o+4+[BitConverter]::ToInt32($b,$o)
    }
    if($at -ne $field[1]){throw "Original target mismatch at $o"}
    if($kind -eq 'string'){
        $expected=[Text.Encoding]::ASCII.GetBytes("Error ID = %d`r`n%s"+[char]0)
        $sa=Read-Bytes $original ($at-0x01000000) $expected.Length
        $sb=Read-Bytes $pilot ($bt-$pilotImageBase) $expected.Length
        if([Convert]::ToBase64String($sa) -ne [Convert]::ToBase64String($expected) -or [Convert]::ToBase64String($sb) -ne [Convert]::ToBase64String($expected)){throw 'Fallback string mismatch'}
    }else{
        if($kind -eq 'copy'){
            $m=[regex]::Match($listing,'\?StringCopyWorkerA@@YGJPADIPBD@Z:\r?\n\s+([0-9a-fA-F]{8}):')
            if(-not $m.Success){throw 'Copy target missing'}
            $expectedTarget=[Convert]::ToUInt32($m.Groups[1].Value,16)
        }else{$expectedTarget=Symbol $field[3]}
        if($bt -ne $expectedTarget){throw "Pilot target mismatch at $o"}
    }
    $validated += [pscustomobject]@{Offset=$o;OriginalTarget=('{0:x8}' -f $at);RebuiltTarget=('{0:x8}' -f $bt);Kind=$kind;Symbol=$field[3]}
    [Array]::Copy($a,$o,$n,$o,4)
}
$remaining=@(for($i=0;$i -lt 437;$i++){if($a[$i] -ne $n[$i]){$i}})
[pscustomobject]@{
    OriginalSHA256=(Get-FileHash $original -Algorithm SHA256).Hash
    PilotSHA256=(Get-FileHash $pilot -Algorithm SHA256).Hash
    OriginalVA='01001d30';PilotVA=('{0:X8}' -f $entry);Length=437
    VerifiedAddressFields=$fields.Count;RemainingDifferingOffsets=$remaining
    Fields=$validated;RawDifferingBytes=@(for($i=0;$i -lt 437;$i++){if($a[$i] -ne $b[$i]){$i}}).Count
    AddressNormalizedExactMatch=($remaining.Count -eq 0)
    Scope='Function body only; 16 target fields mapped, fallback text verified. External target implementations and full-file equality excluded.'
}|ConvertTo-Json -Depth 4
