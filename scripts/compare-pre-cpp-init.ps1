param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
# Reuse the independently verified CRT context and read-only comparison helpers.
# Dot sourcing is deliberate: this also gates pre_c_init and its dependencies.
$preCJson=. "$PSScriptRoot\compare-pre-c-init.ps1" -BuildDirectory $BuildDirectory
$preC=$preCJson|ConvertFrom-Json
$entry=Symbol '_pre_cpp_init';$rtcTerminate=Symbol '__RTC_Terminate';$atexit=Symbol '_atexit'
$preCppGap=Require-BodyBoundary $b $entry 75 (Symbol '___tmainCRTStartup') 'pre_cpp_init'
Require ((U32 $a 0x0100123c) -eq 0x01002313 -and (U32 $b ((Symbol '___xc_a')+4)) -eq $entry) 'C++ initializer table association differs'
Require-GuiCrtMember '_pre_cpp_init'
# Verify the registered termination walker, including its empty callback range.
$x=Bytes $a 0x01002802 36;$y=Bytes $b $rtcTerminate 36
$rtcBegin=Symbol '___rtc_taa';$rtcEnd=Symbol '___rtc_tzz'
Require ($rtcEnd-$rtcBegin -eq 4 -and (U32 $b $rtcBegin) -eq 0 -and (U32 $b $rtcEnd) -eq 0) 'RTC termination callbacks are not empty'
Require ((U32 $a 0x01001570) -eq 0 -and (U32 $a 0x01001574) -eq 0) 'Original RTC termination sentinels differ'
foreach($offset in @(3,8)){Require ([BitConverter]::ToUInt32($x,$offset) -eq 0x01001574 -and [BitConverter]::ToUInt32($y,$offset) -eq $rtcEnd) 'RTC termination range target differs';[Array]::Copy($x,$offset,$y,$offset,4)}
Equal-Bytes $x $y 'RTC termination walker'
# Verify the called atexit adapter without certifying its internal onexit/EH body.
$x=Bytes $a 0x010026f5 18;$y=Bytes $b $atexit 18
Require ((Rel $a 0x010026f5 5) -eq 0x01002651 -and (Rel $b $atexit 5) -eq (Symbol '__onexit')) 'atexit registration target differs'
[Array]::Copy($x,5,$y,5,4);Equal-Bytes $x $y 'atexit adapter'
$x=Bytes $a 0x01002313 75;$y=Bytes $b $entry 75
$references=[Collections.Generic.List[object]]::new()
function Normalize-Cpp([int]$offset,[long]$old,[long]$new,[string]$kind,[string]$role){
 if($kind -eq 'call'){$ox=Rel $a 0x01002313 $offset;$ny=Rel $b $entry $offset}else{$ox=[long][BitConverter]::ToUInt32($x,$offset);$ny=[long][BitConverter]::ToUInt32($y,$offset)}
 Require ($ox -eq $old -and $ny -eq $new) "pre_cpp_init mapping differs: $role"
 if($kind -eq 'state'){Require ((Initial-Dword $a $old) -eq (Initial-Dword $b $new)) "CRT initial state differs: $role"}
 $references.Add([pscustomobject]@{Offset=$offset;Kind=$kind;Role=$role;OriginalTarget=('{0:X8}' -f $old);RebuiltTarget=('{0:X8}' -f $new)})
 [Array]::Copy($x,$offset,$y,$offset,4)
}
Normalize-Cpp 1 0x01002802 $rtcTerminate 'pointer' 'verified RTC termination callback'
Normalize-Cpp 6 0x010026f5 $atexit 'call' 'verified atexit adapter'
Normalize-Cpp 11 0x01004048 (Symbol '__newmode') 'state' 'CRT new allocation mode'
Normalize-Cpp 24 0x01004044 (Symbol '__dowildcard') 'state' 'CRT wildcard expansion option'
# Private CRT globals: establish mappings by the actual five stack-argument
# positions to import-identified __getmainargs, repeated references, writable
# storage and initial values. These are roles, not recovered original names or
# whole-data-section equality. Require separate destination slots.
$startupInfo=[long][BitConverter]::ToUInt32($y,18)
$env=[long][BitConverter]::ToUInt32($y,34);$argv=[long][BitConverter]::ToUInt32($y,39);$argc=[long][BitConverter]::ToUInt32($y,44)
$result=[long][BitConverter]::ToUInt32($y,60)
Require (@(@($startupInfo,$env,$argv,$argc,$result)|Select-Object -Unique).Count -eq 5) 'Private CRT destination slots unexpectedly alias'
Normalize-Cpp 18 0x01004038 $startupInfo 'state' 'startup-info address passed as argument five'
Normalize-Cpp 29 0x01004038 $startupInfo 'state' 'same startup-info newmode field'
Normalize-Cpp 34 0x01004028 $env 'state' 'environment output address, argument three'
Normalize-Cpp 39 0x0100402c $argv 'state' 'argv output address, argument two'
Normalize-Cpp 44 0x01004024 $argc 'state' 'argc output address, argument one'
Normalize-Cpp 60 0x01004034 $result 'state' 'stored getmainargs return status'
$iat=[long][BitConverter]::ToUInt32($y,50)
Require ((Iat $a 0x010011f8) -ceq 'msvcr80.dll!__getmainargs' -and (Iat $b $iat) -ceq 'msvcr80.dll!__getmainargs') 'getmainargs import differs'
Normalize-Cpp 50 0x010011f8 $iat 'iat' 'MSVCR80 __getmainargs'
Require ((Rel $a 0x01002313 69) -eq 0x010027ce -and (Thunk $a 0x010027ce) -ceq (Thunk $b (Rel $b $entry 69))) 'CRT argument-setup error exit differs'
Normalize-Cpp 69 0x010027ce (Rel $b $entry 69) 'call' 'import-identified CRT amsg_exit'
Equal-Bytes $x $y 'pre_cpp_init'
[pscustomobject]@{OriginalSHA256=$preC.OriginalSHA256;RebuiltSHA256=$preC.RebuiltSHA256;Name='CRT pre_cpp_init';OriginalVA='01002313';RebuiltVA=('{0:X8}' -f $entry);Length=75;VerifiedAddressFields=$references.Count;AddressNormalizedExactMatch=$true;References=@($references);RtcTerminateDependencyBytes=36;RtcTerminateDependencyVerified=$true;FollowingGapBytes=$preCppGap;Scope='Complete 75-byte CRT pre_cpp_init body after target-validated normalization, actual C++ table slot, callback/atexit dependencies and CRT import identities. Private globals mapped by getmainargs argument roles/repeated uses and initial state, not full data layout. No whole CRT/EXE, original-source/compiler identity or runtime claim. Separate library identification, not an increase to 18 reconstructed/helper/wrapper bodies.'}|ConvertTo-Json -Depth 7
