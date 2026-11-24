param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
$startupJson=. "$PSScriptRoot\compare-crt-startup.ps1" -BuildDirectory $BuildDirectory
$startupReport=$startupJson|ConvertFrom-Json
$validate=Symbol '__ValidateImageBase';$find=Symbol '__FindPESection';$nonwrite=Symbol '__IsNonwritableInCurrentImage'
$dispatch=Symbol '__except_handler4';$check=Symbol '@__security_check_cookie@4';$cookie=Symbol '___security_cookie'
Equal-Bytes (Bytes $a 0x01002840 41) (Bytes $b $validate 41) 'ValidateImageBase'
Equal-Bytes (Bytes $a 0x01002870 66) (Bytes $b $find 66) 'FindPESection'
$x=Bytes $a 0x0100263d 15;$y=Bytes $b $check 15
Require ([BitConverter]::ToUInt32($x,2) -eq 0x01004004 -and [BitConverter]::ToUInt32($y,2) -eq $cookie) 'Security checker cookie target differs'
Require ((Rel $a 0x0100263d 11) -eq 0x01002a6d -and (Rel $b $check 11) -eq (Symbol '___report_gsfailure')) 'Security checker failure-report target differs'
foreach($offset in @(2,11)){[Array]::Copy($x,$offset,$y,$offset,4)}
Equal-Bytes $x $y 'Security cookie checker'
$x=Bytes $a 0x01002768 35;$y=Bytes $b $dispatch 35
foreach($f in @(@(0x11,0x0100263d,$check),@(0x16,0x01004004,$cookie))){Require ([BitConverter]::ToUInt32($x,$f[0]) -eq $f[1] -and [BitConverter]::ToUInt32($y,$f[0]) -eq $f[2]) 'Exception dispatch prefix target differs';[Array]::Copy($x,$f[0],$y,$f[0],4)}
$common=Symbol '__except_handler4_common'
Require ((Rel $a 0x01002768 0x1b) -eq 0x01002b9a -and (Rel $b $dispatch 0x1b) -eq $common) 'Exception dispatch common target differs'
Require ((Thunk $a 0x01002b9a) -ceq 'msvcr80.dll!_except_handler4_common' -and (Thunk $b $common) -ceq 'msvcr80.dll!_except_handler4_common') 'Exception dispatch import identity differs'
[Array]::Copy($x,0x1b,$y,0x1b,4);Equal-Bytes $x $y '35-byte exception dispatch wrapper'
$x=Bytes $a 0x010028b7 108;$y=Bytes $b $nonwrite 108
$imageRegionGap=Require-BodyBoundary $b $nonwrite 108 (Symbol '__initterm') 'Nonwritable-image region'
$scope=[long][BitConverter]::ToUInt32($y,3);$oldScope=Bytes $a 0x01002dc8 28;$newScope=Bytes $b $scope 28
foreach($f in @(@(0x14,0x46),@(0x18,0x5a))){Require ([BitConverter]::ToUInt32($oldScope,$f[0]) -eq 0x010028b7+$f[1] -and [BitConverter]::ToUInt32($newScope,$f[0]) -eq $nonwrite+$f[1]) 'PE helper scope target differs';[Array]::Copy($oldScope,$f[0],$newScope,$f[0],4)}
Equal-Bytes $oldScope $newScope '28-byte PE helper EH4 scope table'
$fields=@(@(3,0x01002dc8,$scope,'absolute'),@(8,0x01002940,(Symbol '__SEH_prolog4'),'call'),@(0x11,$a.Base,$b.Base,'absolute'),@(0x17,0x01002840,$validate,'call'),@(0x28,0x01002870,$find,'call'),@(0x67,0x01002985,(Symbol '__SEH_epilog4'),'call'))
foreach($f in $fields){$offset=$f[0];if($f[3] -eq 'call'){$old=Rel $a 0x010028b7 $offset;$new=Rel $b $nonwrite $offset}else{$old=[long][BitConverter]::ToUInt32($x,$offset);$new=[long][BitConverter]::ToUInt32($y,$offset)};Require ($old -eq $f[1] -and $new -eq $f[2]) 'PE helper region reference differs';[Array]::Copy($x,$offset,$y,$offset,4)}
Equal-Bytes $x $y '108-byte nonwritable region including exception code'
[pscustomobject]@{OriginalSHA256=$startupReport.OriginalSHA256;RebuiltSHA256=$startupReport.RebuiltSHA256;ValidateImageBase=@{OriginalVA='01002840';RebuiltVA=('{0:X8}' -f $validate);Bytes=41;RawExactMatch=$true};FindPESection=@{OriginalVA='01002870';RebuiltVA=('{0:X8}' -f $find);Bytes=66;RawExactMatch=$true};NonwritableRegion=@{OriginalVA='010028B7';RebuiltVA=('{0:X8}' -f $nonwrite);Bytes=108;VerifiedAddressFields=6;RegionAddressNormalizedExactMatch=$true;StoredNormalFlowBytes=85;EmbeddedExceptionBytes=23;ScopeTableBytes=28;ScopePointerFields=2;ScopeTableVerified=$true};ExceptionDispatch=@{OriginalVA='01002768';RebuiltVA=('{0:X8}' -f $dispatch);Bytes=35;VerifiedAddressFields=3;AddressNormalizedExactMatch=$true;CommonImportIdentity='msvcr80.dll!_except_handler4_common'};SecurityCookieCheck=@{OriginalVA='0100263D';RebuiltVA=('{0:X8}' -f $check);Bytes=15;VerifiedAddressFields=2;AddressNormalizedExactMatch=$true};FollowingGapBytes=$imageRegionGap;Scope='Complete stated CRT regions/bodies, including embedded PE-helper EH code and validated scope table. Imported exception-dispatch implementation and report_gsfailure body remain outside this report; report target mapped by original identity/current MAP. Not whole EXE/layout or runtime proof; library identifications separately reported from 18 existing application/helper/wrapper bodies.'}|ConvertTo-Json -Depth 7
