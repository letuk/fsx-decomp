param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
$failureJson=. "$PSScriptRoot\compare-crt-failure-stack.ps1" -BuildDirectory $BuildDirectory
$failureReport=$failureJson|ConvertFrom-Json
$onexit=Symbol '__onexit';$atexit=Symbol '_atexit';$cleanup=$onexit+150
$x=Bytes $a 0x010026e7 9;$y=Bytes $b $cleanup 9
Require ((Rel $a 0x010026e7 3) -eq 0x01002b76 -and (Rel $b $cleanup 3) -eq (Symbol '__unlock')) 'Exit-lock release target differs'
Require ((Thunk $a 0x01002b76) -ceq 'msvcr80.dll!_unlock' -and (Thunk $b (Symbol '__unlock')) -ceq 'msvcr80.dll!_unlock') 'Exit-lock release import differs'
[Array]::Copy($x,3,$y,3,4);Equal-Bytes $x $y '9-byte exit-lock cleanup'
$x=Bytes $a 0x010026f5 18;$y=Bytes $b $atexit 18
Require ((Rel $a 0x010026f5 5) -eq 0x01002651 -and (Rel $b $atexit 5) -eq $onexit) 'atexit callback adapter target differs'
[Array]::Copy($x,5,$y,5,4);Equal-Bytes $x $y '18-byte atexit adapter'
$x=Bytes $a 0x01002651 150;$y=Bytes $b $onexit 150
$onexitRegionGap=Require-BodyBoundary $b $onexit 159 $atexit 'Registration/cleanup region'
$scope=[long][BitConverter]::ToUInt32($y,3);$oldScope=Bytes $a 0x01002da8 28;$newScope=Bytes $b $scope 28
Require ((U32 $a 0x01002dbc) -eq 0 -and (U32 $b ($scope+20)) -eq 0) 'Expected finally record with null filter'
Require ([BitConverter]::ToUInt32($oldScope,24) -eq 0x010026e7 -and [BitConverter]::ToUInt32($newScope,24) -eq $cleanup) 'Finally cleanup pointer differs'
[Array]::Copy($oldScope,24,$newScope,24,4);Equal-Bytes $oldScope $newScope '28-byte onexit EH4 finally table'
$begin=Symbol '___onexitbegin';$end=Symbol '___onexitend'
Require ((Initial-Dword $a 0x01004498) -eq (Initial-Dword $b $begin) -and (Initial-Dword $a 0x01004494) -eq (Initial-Dword $b $end)) 'Encoded exit-table initial state differs'
$refs=[Collections.Generic.List[object]]::new()
function Normalize-Onexit([int]$offset,[long]$old,[long]$new,[string]$kind,[string]$role){
 if($kind -eq 'call'){$ox=Rel $a 0x01002651 $offset;$ny=Rel $b $onexit $offset}else{$ox=[long][BitConverter]::ToUInt32($x,$offset);$ny=[long][BitConverter]::ToUInt32($y,$offset)}
 Require ($ox -eq $old -and $ny -eq $new) "onexit target differs: $role"
 $refs.Add([pscustomobject]@{Offset=$offset;Kind=$kind;Role=$role;OriginalTarget=('{0:X8}' -f $old);RebuiltTarget=('{0:X8}' -f $new)})
 [Array]::Copy($x,$offset,$y,$offset,4)
}
Normalize-Onexit 3 0x01002da8 $scope 'scope' 'verified EH4 finally table'
Normalize-Onexit 8 0x01002940 (Symbol '__SEH_prolog4') 'call' 'verified SEH prologue'
foreach($offset in @(0x0e,0x3d,0x70)){Normalize-Onexit $offset 0x01004498 $begin 'state' 'same encoded onexit begin checked in pre_c_init'}
foreach($offset in @(0x48,0x7d)){Normalize-Onexit $offset 0x01004494 $end 'state' 'same encoded onexit end checked in pre_c_init'}
foreach($f in @(@(0x14,0x010011a8),@(0x28,0x010011ac),@(0x69,0x010011c0))){$new=[long][BitConverter]::ToUInt32($y,$f[0]);$id=Iat $a $f[1];Require ($id -ceq (Iat $b $new)) 'onexit IAT identity differs';Normalize-Onexit $f[0] $f[1] $new 'iat' $id}
foreach($f in @(@(0x32,0x01002b8e,'__lock'),@(0x5d,0x01002b82,'___dllonexit'))){$new=Symbol $f[2];Require ((Thunk $a $f[1]) -ceq (Thunk $b $new)) 'onexit thunk identity differs';Normalize-Onexit $f[0] $f[1] $new 'call' $f[2]}
Normalize-Onexit 0x89 0x010026e7 $cleanup 'call' 'verified exit-lock cleanup, also EH finally target'
Normalize-Onexit 0x91 0x01002985 (Symbol '__SEH_epilog4') 'call' 'verified SEH epilogue'
Equal-Bytes $x $y '150-byte onexit registration body'
[pscustomobject]@{OriginalSHA256=$failureReport.OriginalSHA256;RebuiltSHA256=$failureReport.RebuiltSHA256;Onexit=@{OriginalVA='01002651';RebuiltVA=('{0:X8}' -f $onexit);Bytes=150;VerifiedAddressFields=$refs.Count;AddressNormalizedExactMatch=$true;References=@($refs)};Cleanup=@{OriginalVA='010026E7';RebuiltVA=('{0:X8}' -f $cleanup);Bytes=9;VerifiedAddressFields=1;AddressNormalizedExactMatch=$true;LockNumber=8};AtexitAdapter=@{OriginalVA='010026F5';RebuiltVA=('{0:X8}' -f $atexit);Bytes=18;VerifiedAddressFields=1;AddressNormalizedExactMatch=$true};Eh4FinallyTableBytes=28;Eh4FinallyPointerVerified=$true;FollowingGapBytes=$onexitRegionGap;Scope='Complete stated CRT registration/cleanup/adapter bodies and EH4 finally record, encoded table global mappings, import identities and initial state checked. Imported runtime registration/lock/encoding implementations and actual runtime behavior excluded; no whole EXE/layout/original compiler proof. Existing18 application/helper/wrapper count unchanged.'}|ConvertTo-Json -Depth 8
