param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
$onexitJson=. "$PSScriptRoot\compare-crt-onexit.ps1" -BuildDirectory $BuildDirectory
$onexitReport=$onexitJson|ConvertFrom-Json
$fake=Symbol '?_AtlGetThreadACPFake@ATL@@YGIXZ';$real=Symbol '?_AtlGetThreadACPReal@ATL@@YGIXZ';$thunk=Symbol '?_AtlGetThreadACPThunk@ATL@@YGIXZ';$cache=Symbol '?g_pfnGetThreadACP@ATL@@3P6GIXZA'
Require ((U32 $a 0x01004000) -eq 0x01001f92 -and (U32 $b $cache) -eq $thunk) 'ATL cached getter initial target differs'
Equal-Bytes (Bytes $a 0x01001f89 4) (Bytes $b $real 4) 'ATL real codepage getter'
function Compare-AtlBody([string]$name,[long]$old,[long]$new,[int]$length,$fields){
 $x=Bytes $a $old $length;$y=Bytes $b $new $length;$references=@()
 foreach($f in $fields){$offset=$f[0];$kind=$f[1];$expectedOld=$f[2];$expectedNew=$f[3];$role=$f[4]
  if($kind -eq 'call'){$ox=Rel $a $old $offset;$ny=Rel $b $new $offset}else{$ox=[long][BitConverter]::ToUInt32($x,$offset);$ny=[long][BitConverter]::ToUInt32($y,$offset)}
  if($kind -eq 'iat'){$expectedNew=$ny;Require ((Iat $a $expectedOld) -ceq $role -and (Iat $b $ny) -ceq $role) 'ATL import identity differs'}
  Require ($ox -eq $expectedOld -and $ny -eq $expectedNew) "ATL target differs: $role"
  $references+=@{Offset=$offset;Kind=$kind;Role=$role;OriginalTarget=('{0:X8}' -f $ox);RebuiltTarget=('{0:X8}' -f $ny)};[Array]::Copy($x,$offset,$y,$offset,4)
 }
 Equal-Bytes $x $y $name
 return @{Name=$name;OriginalVA=('{0:X8}' -f $old);RebuiltVA=('{0:X8}' -f $new);Bytes=$length;VerifiedAddressFields=$fields.Count;AddressNormalizedExactMatch=$true;References=$references}
}
$cookie=Symbol '___security_cookie';$check=Symbol '@__security_check_cookie@4'
$fakeReport=Compare-AtlBody 'ATL fallback codepage getter' 0x01001f1f $fake 101 @(@(7,'absolute',0x01004004,$cookie,'security cookie'),@(0x15,'iat',0x01001048,0,'kernel32.dll!GetThreadLocale'),@(0x27,'iat',0x01001044,0,'kernel32.dll!GetLocaleInfoA'),@(0x50,'iat',0x01001040,0,'kernel32.dll!GetACP'),@(0x5f,'call',0x0100263d,$check,'verified security cookie checker'))
$thunkReport=Compare-AtlBody 'ATL lazy selecting thunk' 0x01001f92 $thunk 100 @(@(0x0a,'absolute',0x01004004,$cookie,'security cookie'),@(0x26,'iat',0x0100104c,0,'kernel32.dll!GetVersionExA'),@(0x3b,'absolute',0x01001f89,$real,'verified real getter'),@(0x42,'absolute',0x01001f1f,$fake,'verified fallback getter'),@(0x48,'absolute',0x01004000,$cache,'same cached getter slot'),@(0x4e,'iat',0x01001000,0,'kernel32.dll!InterlockedExchange'),@(0x54,'absolute',0x01004000,$cache,'same slot invoked after exchange'),@(0x5e,'call',0x0100263d,$check,'verified security cookie checker'))
Require ((Bytes $a 0x01001f84 5|Where-Object {$_ -ne 0xcc}).Count -eq 0 -and (Bytes $a 0x01001f8d 5|Where-Object {$_ -ne 0xcc}).Count -eq 0) 'Original ATL inter-function gap content changed'
[pscustomobject]@{OriginalSHA256=$onexitReport.OriginalSHA256;RebuiltSHA256=$onexitReport.RebuiltSHA256;Fake=$fakeReport;Real=@{OriginalVA='01001F89';RebuiltVA=('{0:X8}' -f $real);Bytes=4;RawExactMatch=$true;ReturnValue=3};Thunk=$thunkReport;CachedGetterInitialTargetVerified=$true;PaddingObservation=@{OriginalFakeToRealGap=5;OriginalRealToThunkGap=5;OriginalGapBytes='CC';RebuiltFakeToRealGap=$real-($fake+101);RebuiltRealToThunkGap=$thunk-($real+4);OriginalFunctionPaddingOption='Hypothesis only; precise linker option unproven'};Scope='Complete stated ATL bodies, selected imports, cookie-check target and cached getter initial pointer/alias uses checked. Five-byte original gaps explicitly remain a layout difference; no whole EXE/linker/source/compiler identity or runtime behavior claim. Existing18 application/helper/wrapper count unchanged; library identifications separately reported.'}|ConvertTo-Json -Depth 8
