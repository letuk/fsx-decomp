param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
# Reuses/gates both initializer comparisons and their PE/import/state helpers.
$cppJson=. "$PSScriptRoot\compare-pre-cpp-init.ps1" -BuildDirectory $BuildDirectory
$cppReport=$cppJson|ConvertFrom-Json
$preCReport=$preC
$crt=Symbol '___tmainCRTStartup';$entry=Symbol '_WinMainCRTStartup'
$crtRegionGap=Require-BodyBoundary $b $crt 479 (Symbol '_pre_c_init') 'Full CRT startup region'
$prolog=Symbol '__SEH_prolog4';$epilog=Symbol '__SEH_epilog4';$cookieInit=Symbol '___security_init_cookie'
$cookie=Symbol '___security_cookie';$complement=Symbol '___security_cookie_complement';$handler=Symbol '__except_handler4'
Require ((Initial-Dword $a 0x01004004) -eq (Initial-Dword $b $cookie) -and (Initial-Dword $a 0x01004008) -eq (Initial-Dword $b $complement)) 'Initial cookie pair differs'
$x=Bytes $a 0x01002940 69;$y=Bytes $b $prolog 69
foreach($f in @(@(1,0x01002768,$handler),@(0x1e,0x01004004,$cookie))){Require ([BitConverter]::ToUInt32($x,$f[0]) -eq $f[1] -and [BitConverter]::ToUInt32($y,$f[0]) -eq $f[2]) 'SEH prologue target differs';[Array]::Copy($x,$f[0],$y,$f[0],4)}
Equal-Bytes $x $y 'SEH prologue'
Equal-Bytes (Bytes $a 0x01002985 20) (Bytes $b $epilog 20) 'SEH epilogue'
$x=Bytes $a 0x010029d4 148;$y=Bytes $b $cookieInit 148
foreach($f in @(@(7,0x01004004,$cookie),@(0x2a,0x01004008,$complement),@(0x83,0x01004004,$cookie),@(0x8b,0x01004008,$complement))){Require ([BitConverter]::ToUInt32($x,$f[0]) -eq $f[1] -and [BitConverter]::ToUInt32($y,$f[0]) -eq $f[2]) 'Cookie-init state target differs';[Array]::Copy($x,$f[0],$y,$f[0],4)}
foreach($f in @(@(0x37,0x0100106c),@(0x43,0x01001068),@(0x4b,0x01001064),@(0x53,0x01001060),@(0x5f,0x0100105c))){$new=[long][BitConverter]::ToUInt32($y,$f[0]);Require ([BitConverter]::ToUInt32($x,$f[0]) -eq $f[1] -and (Iat $a $f[1]) -ceq (Iat $b $new)) 'Cookie entropy import differs';[Array]::Copy($x,$f[0],$y,$f[0],4)}
Equal-Bytes $x $y 'Security cookie initializer'
$winMain=Symbol '_WinMain@16';$mfc=Symbol '?AfxWinMain@@YGHPAUHINSTANCE__@@0PADH@Z'
Require ((Bytes $a 0x01002c03 1)[0] -eq 0xe9 -and (Bytes $b $winMain 1)[0] -eq 0xe9) 'Expected MFC WinMain tail wrapper'
Require ((Thunk $a (Rel $a 0x01002c03 1)) -ceq 'mfc80.dll!#1207' -and (Rel $b $winMain 1) -eq $mfc -and (Thunk $b $mfc) -ceq 'mfc80.dll!#1207') 'MFC entry identity differs'
$x=Bytes $a 0x01002363 479;$y=Bytes $b $crt 479
$scope=[long][BitConverter]::ToUInt32($y,3)
$oldScope=Bytes $a 0x01002d80 40;$newScope=Bytes $b $scope 40
$scopeFields=@(@(0x14,0x1c6),@(0x18,0x1ca),@(0x20,0x17a),@(0x24,0x18e))
foreach($f in $scopeFields){Require ([BitConverter]::ToUInt32($oldScope,$f[0]) -eq 0x01002363+$f[1] -and [BitConverter]::ToUInt32($newScope,$f[0]) -eq $crt+$f[1]) 'SEH filter/handler target differs';[Array]::Copy($oldScope,$f[0],$newScope,$f[0],4)}
Equal-Bytes $oldScope $newScope '40-byte EH4 scope table'
$state=Symbol '___native_startup_state';$lock=Symbol '___native_startup_lock';$tls=Symbol '___dyn_tls_init_callback'
$hasCctors=[long][BitConverter]::ToUInt32($y,0xa1);$mainResult=[long][BitConverter]::ToUInt32($y,0x141)
$managed=[Convert]::ToInt64(($preCReport.References|Where-Object Offset -eq 0x63).RebuiltTarget,16)
Require (@(@($state,$lock,$tls,$hasCctors,$mainResult,$managed)|Select-Object -Unique).Count -eq 6) 'Private/startup globals unexpectedly alias'
$references=[Collections.Generic.List[object]]::new()
function Normalize-Startup([int]$offset,[long]$old,[long]$new,[string]$kind,[string]$role){
 if($kind -eq 'call'){$ox=Rel $a 0x01002363 $offset;$ny=Rel $b $crt $offset}else{$ox=[long][BitConverter]::ToUInt32($x,$offset);$ny=[long][BitConverter]::ToUInt32($y,$offset)}
 Require ($ox -eq $old -and $ny -eq $new) "CRT startup target differs: $role"
 if($kind -eq 'state'){Require ((Initial-Dword $a $old) -eq (Initial-Dword $b $new)) "CRT startup initial state differs: $role"}
 $references.Add([pscustomobject]@{Offset=$offset;Kind=$kind;Role=$role;OriginalTarget=('{0:X8}' -f $old);RebuiltTarget=('{0:X8}' -f $new)})
 [Array]::Copy($x,$offset,$y,$offset,4)
}
Normalize-Startup 3 0x01002d80 $scope 'scope' 'verified EH4 table including four region-local filter/handler targets'
Normalize-Startup 8 0x01002940 $prolog 'call' 'verified 69-byte SEH prologue'
foreach($f in @(@(0x1a,0x01001058),@(0x40,0x01001054),@(0x5a,0x01001050),@(0xd1,0x01001000),@(0xfa,0x010011e0),@(0x151,0x010011e4),@(0x16a,0x010011e8),@(0x1a5,0x010011f0),@(0x1b4,0x010011f4))){$new=[long][BitConverter]::ToUInt32($y,$f[0]);$identity=Iat $a $f[1];Require ($identity -ceq (Iat $b $new)) 'CRT startup IAT identity differs';Normalize-Startup $f[0] $f[1] $new 'iat' $identity}
Normalize-Startup 0x36 0x01004490 $lock 'state' 'native startup lock'
foreach($offset in @(0x64,0x77,0x81,0xa6,0xc1)){Normalize-Startup $offset 0x0100448c $state 'state' 'native startup state'}
foreach($offset in @(0xa1,0x1ab)){Normalize-Startup $offset 0x01004040 $hasCctors 'state' 'private already-initialized/has-cctors flag'}
foreach($offset in @(0xd7,0xdf,0xf5)){Normalize-Startup $offset 0x0100449c $tls 'state' 'dynamic TLS initializer callback slot'}
foreach($offset in @(0x141,0x195,0x1c0)){Normalize-Startup $offset 0x0100403c $mainResult 'state' 'private application return/exception code'}
foreach($offset in @(0x147,0x19b)){Normalize-Startup $offset 0x01004030 $managed 'state' 'same managed-image flag verified in pre_c_init'}
foreach($f in @(@(0x86,0x01001258,'___xi_z'),@(0x8b,0x01001250,'___xi_a'),@(0xaf,0x0100124c,'___xc_z'),@(0xb4,0x01001238,'___xc_a'))){Normalize-Startup $f[0] $f[1] (Symbol $f[2]) 'table' $f[2]}
foreach($f in @(@(0x6f,0x010027ce,'__amsg_exit'),@(0x90,0x01002934,'__initterm_e'),@(0xb9,0x01002928,'__initterm'),@(0x187,0x0100282c,'__XcptFilter'))){$new=Symbol $f[2];Require ((Thunk $a $f[1]) -ceq (Thunk $b $new)) 'CRT startup thunk import differs';Normalize-Startup $f[0] $f[1] $new 'call' $f[2]}
Normalize-Startup 0xe4 0x010028b7 (Symbol '__IsNonwritableInCurrentImage') 'call' 'identified PE image helper; full callee body not certified here'
Normalize-Startup 0x137 $a.Base $b.Base 'image' 'own image base passed to WinMain'
Normalize-Startup 0x13c 0x01002c03 $winMain 'call' 'verified MFC WinMain tail wrapper'
Normalize-Startup 0x1da 0x01002985 $epilog 'call' 'raw-exact 20-byte SEH epilogue'
Equal-Bytes $x $y '479-byte CRT startup region including embedded SEH code'
$oldOpt=[BitConverter]::ToInt32($a.Bytes,60)+24;$newOpt=[BitConverter]::ToInt32($b.Bytes,60)+24
Require ($a.Base+[BitConverter]::ToUInt32($a.Bytes,$oldOpt+16) -eq 0x0100262e -and $b.Base+[BitConverter]::ToUInt32($b.Bytes,$newOpt+16) -eq $entry) 'PE entry point differs from checked wrapper'
$oldEntry=Bytes $a 0x0100262e 10;$newEntry=Bytes $b $entry 10
Require ($oldEntry[0] -eq 0xe8 -and $newEntry[0] -eq 0xe8 -and (Rel $a 0x0100262e 1) -eq 0x010029d4 -and (Rel $b $entry 1) -eq $cookieInit) 'Entry cookie-init call differs'
Require ($oldEntry[5] -eq 0xe9 -and $newEntry[5] -eq 0xe9 -and (Rel $a 0x0100262e 6) -eq 0x01002363 -and (Rel $b $entry 6) -eq $crt) 'Entry CRT jump differs'
foreach($offset in @(1,6)){[Array]::Copy($oldEntry,$offset,$newEntry,$offset,4)};Equal-Bytes $oldEntry $newEntry '10-byte PE entry wrapper'
[pscustomobject]@{OriginalSHA256=$cppReport.OriginalSHA256;RebuiltSHA256=$cppReport.RebuiltSHA256;Name='CRT startup region including embedded EH4 filter/handler code';OriginalVA='01002363';RebuiltVA=('{0:X8}' -f $crt);RegionLength=479;VerifiedAddressFields=$references.Count;RegionAddressNormalizedExactMatch=$true;References=@($references);Eh4ScopeTableBytes=40;Eh4ScopeTablePointerFields=4;Eh4ScopeTableVerified=$true;EntryWrapperBytes=10;EntryWrapperVerified=$true;SecurityCookieInitializerBytes=148;SecurityCookieInitializerVerified=$true;SehPrologueBytes=69;SehPrologueVerified=$true;SehEpilogueBytes=20;SehEpilogueRawExactMatch=$true;MfcWinMainWrapperVerified=$true;FollowingGapBytes=$crtRegionGap;Scope='Complete contiguous 479-byte CRT region, including embedded SEH filter/handler code outside the 425-byte stored function, plus checked EH4 table and PE entry. Cookie initializer, SEH prologue/epilogue and MFC wrapper checked as stated. PE image helper and except_handler4 bodies remain separate uncertified dependencies; imported DLL implementations, entire CRT/EXE/layout, runtime behavior and original compiler identity excluded. Existing 18 reconstructed/helper/wrapper count unchanged.'}|ConvertTo-Json -Depth 7
