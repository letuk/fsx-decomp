param([Parameter(Mandatory)][string]$BuildDirectory,[switch]$RequireOriginalOrder)
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$original="$PSScriptRoot\..\original\fsx.exe"
if((Get-FileHash $original).Hash -ne '61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735'){throw 'Original identity changed'}
$a=Open-PE $original;$b=Open-PE "$BuildDirectory\fsx.exe";Load-Imports $a;Load-Imports $b
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
function Require($condition,[string]$message){if(!$condition){throw $message}}
function Symbol([string]$name){$m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($name)+'\s+([0-9a-fA-F]{8})\b');Require $m.Success "Missing symbol $name";return [Convert]::ToInt64($m.Groups[1].Value,16)}
function Rel($pe,[long]$entry,[int]$offset){return $entry+$offset+4+[BitConverter]::ToInt32((Bytes $pe ($entry+$offset) 4),0)}
function Iat($pe,[long]$entry){$identity=$pe.Imports[$entry];Require ($null -ne $identity) 'Missing import identity';return $identity}
function Thunk($pe,[long]$entry){$x=Bytes $pe $entry 6;Require ($x[0] -eq 255 -and $x[1] -eq 37) 'Expected FF25 import thunk';return Iat $pe ([long][BitConverter]::ToUInt32($x,2))}
$window=Symbol '?g_mainWindowPilot@@3VCWnd@@A';$app=Symbol '?g_fsxAppPilot@@3VFSXAppPilot@@A'
$vt=Symbol '??_7FSXAppPilot@@6B@';$afxState=Symbol '?_afxInitAppState@@3DA'
$windowInit=Symbol '??__Eg_mainWindowPilot@@YAXXZ';$appInit=Symbol '??__Eg_fsxAppPilot@@YAXXZ'
$afxInit=Symbol '??__E_afxInitAppState@@YAXXZ';$windowDtor=Symbol '??__Fg_mainWindowPilot@@YAXXZ';$appDtor=Symbol '??__Fg_fsxAppPilot@@YAXXZ'
$appCtor=Symbol '??0FSXAppPilot@@QAE@XZ';$afxInitialize=Symbol '?AfxInitialize@@YGHHK@Z'
$atexit=Symbol '_atexit';$onexit=Symbol '__onexit'
# Identify the CRT adapter separately. Its internal _onexit implementation/EH
# metadata is NOT certified here; existing original function evidence and MAP
# provenance establish the dependency, not an arbitrary masked call target.
Require ($map -match '(?im)^\s*\S+\s+_atexit\s+\S+.*(?:msvcrt|selected-crt):atonexit\.obj') 'Unexpected atexit provenance'
Require ($map -match '(?im)^\s*\S+\s+__onexit\s+\S+.*(?:msvcrt|selected-crt):atonexit\.obj') 'Unexpected onexit provenance'
$oldAdapter=Bytes $a 0x010026f5 18;$newAdapter=Bytes $b $atexit 18
Require ((Rel $a 0x010026f5 5) -eq 0x01002651 -and (Rel $b $atexit 5) -eq $onexit) 'atexit onexit target differs'
[Array]::Copy($oldAdapter,5,$newAdapter,5,4)
Require ([Convert]::ToBase64String($oldAdapter) -ceq [Convert]::ToBase64String($newAdapter)) 'atexit adapter bytes differ'
# Independently validate the linked MFC initializer called by its static wrapper.
$x=Bytes $a 0x01002c0d 38;$y=Bytes $b $afxInitialize 38
Require ((Thunk $a (Rel $a 0x01002c0d 1)) -ceq (Thunk $b (Rel $b $afxInitialize 1))) 'AfxInitialize module-state import differs'
Require ((Iat $a ([long][BitConverter]::ToUInt32($x,0x1b))) -ceq (Iat $b ([long][BitConverter]::ToUInt32($y,0x1b)))) 'AfxInitialize setmbcp import differs'
foreach($offset in @(1,0x1b)){[Array]::Copy($x,$offset,$y,$offset,4)}
Require ([Convert]::ToBase64String($x) -ceq [Convert]::ToBase64String($y)) 'AfxInitialize dependency bytes differ'
$mappings=@{
 window=@(0x01004388L,$window);app=@(0x010043e0L,$app);event=@(0x01004484L,($app+0xa4));vtable=@(0x01001398L,$vt);
 windowDtor=@(0x01002d40L,$windowDtor);appDtor=@(0x01002d50L,$appDtor);afxState=@(0x0100437cL,$afxState);
 appCtor=@(0x01001660L,$appCtor);afxInitialize=@(0x01002c0dL,$afxInitialize);atexit=@(0x010026f5L,$atexit)
}
$references=[Collections.Generic.List[object]]::new()
function Compare-Glue([string]$name,[long]$old,[long]$new,[int]$length,$fields){
 $x=Bytes $a $old $length;$y=Bytes $b $new $length
 foreach($f in $fields){
  $offset=[int]$f[0];$kind=$f[1];$target=$f[2]
  if($kind -in 'call','import-call'){$ox=Rel $a $old $offset;$ny=Rel $b $new $offset}else{$ox=[long][BitConverter]::ToUInt32($x,$offset);$ny=[long][BitConverter]::ToUInt32($y,$offset)}
  if($kind -eq 'import-call'){Require ($ox -eq [long]$target) "$name original thunk changed";$identity=Thunk $a $ox;Require ($identity -ceq (Thunk $b $ny)) "$name import thunk differs"}
  elseif($kind -eq 'iat'){$identity=Iat $a $ox;Require ($ox -eq [long]$target -and $identity -ceq (Iat $b $ny)) "$name IAT differs"}
  else{$pair=$mappings[$target];Require ($ox -eq $pair[0] -and $ny -eq $pair[1]) "$name mapping differs: $target";$identity=$target}
  $references.Add([pscustomobject]@{Body=$name;Offset=$offset;Identity=$identity})
  [Array]::Copy($x,$offset,$y,$offset,4)
 }
 $differences=@(for($i=0;$i -lt $length;$i++){if($x[$i] -ne $y[$i]){$i}})
 Require (!$differences.Count) "$name non-reference bytes differ: $differences"
 return [pscustomobject]@{Name=$name;OriginalVA=('{0:X8}' -f $old);RebuiltVA=('{0:X8}' -f $new);Length=$length;VerifiedAddressFields=$fields.Count;AddressNormalizedExactMatch=$true}
}
$bodies=@(
 (Compare-Glue 'Global window initializer' 0x01002ce0 $windowInit 22 @(@(1,'absolute','window'),@(6,'import-call',0x01002008),@(11,'absolute','windowDtor'),@(16,'call','atexit')))
 (Compare-Glue 'Global application initializer' 0x01002d00 $appInit 17 @(@(1,'call','appCtor'),@(6,'absolute','appDtor'),@(11,'call','atexit')))
 (Compare-Glue 'MFC application-state initializer' 0x01002d16 $afxInit 18 @(@(8,'call','afxInitialize'),@(13,'absolute','afxState')))
 (Compare-Glue 'Global window destructor' 0x01002d40 $windowDtor 10 @(@(1,'absolute','window'),@(6,'import-call',0x01002014)))
 (Compare-Glue 'Global application destructor' 0x01002d50 $appDtor 46 @(@(1,'absolute','event'),@(9,'absolute','app'),@(13,'absolute','vtable'),@(22,'iat',0x0100100c),@(28,'absolute','event'),@(37,'absolute','app'),@(42,'import-call',0x01002038)))
)
$xcA=Symbol '___xc_a';$xcZ=Symbol '___xc_z';$xiA=Symbol '___xi_a';$xiZ=Symbol '___xi_z'
Require ($xcZ-$xcA -eq 20 -and $xiZ-$xiA -in 8,12) 'Unexpected CRT initializer table lengths'
$expectedOld=@(0,0x01002313,0x01002d16,0x01002ce0,0x01002d00,0)
$expectedNew=@(0,(Symbol '_pre_cpp_init'),$afxInit,$windowInit,$appInit,0)
$entries=@();$orderMatches=$true
for($i=0;$i -lt 6;$i++){
 $old=U32 $a (0x01001238+4*$i);$new=U32 $b ($xcA+4*$i)
 Require ($old -eq $expectedOld[$i]) 'Original XCA..XCZ table changed'
 if($new -ne $expectedNew[$i]){$orderMatches=$false}
 $entries+=,[pscustomobject]@{Index=$i;OriginalSlot=('{0:X8}' -f (0x01001238+4*$i));OriginalTarget=('{0:X8}' -f $old);RebuiltSlot=('{0:X8}' -f ($xcA+4*$i));RebuiltTarget=('{0:X8}' -f $new);ExpectedRebuiltTarget=('{0:X8}' -f $expectedNew[$i]);MatchesExpectedRole=($new -eq $expectedNew[$i])}
}
# C table has an additional SDK exception-filter initializer in this build.
# Report the difference explicitly; do not claim full C-table equality.
foreach($entry in @($xiA,$xiZ)){Require ((U32 $b $entry) -eq 0) 'Unexpected C initializer sentinel'}
$cTableMatches=($xiZ-$xiA -eq 8)
$additionalCInitializer=$null
if(!$cTableMatches){
 Require ((U32 $b ($xiA+8)) -eq (Symbol '___CxxSetUnhandledExceptionFilter')) 'Unexpected additional C initializer'
 $additionalCInitializer='___CxxSetUnhandledExceptionFilter at XIY; absent from original XIA..XIZ table'
}
Require ((U32 $a 0x01001250) -eq 0 -and (U32 $a 0x01001254) -eq 0x01002547 -and (U32 $a 0x01001258) -eq 0) 'Original C initializer table changed'
Require ((U32 $b ($xiA+4)) -eq (Symbol '_pre_c_init')) 'Unexpected pre_c_init table entry'
function Verify-TableCall($pe,[long]$start,[int]$length,[long]$first,[long]$last,[string]$identity){
 $code=Bytes $pe $start $length;$count=0
 for($i=0;$i+15 -le $code.Length;$i++){
  if($code[$i] -ne 0x68 -or $code[$i+5] -ne 0x68 -or $code[$i+10] -ne 0xe8){continue}
  if([BitConverter]::ToUInt32($code,$i+1) -ne $last -or [BitConverter]::ToUInt32($code,$i+6) -ne $first){continue}
  Require ((Thunk $pe (Rel $pe ($start+$i) 11)) -ceq $identity) 'CRT table-call import differs'
  $count++
 }
 Require ($count -eq 1) 'Expected one CRT call using the checked table boundaries'
}
$crt=Symbol '___tmainCRTStartup';$crtSpan=(Symbol '_pre_c_init')-$crt
Require ($crtSpan -gt 0 -and $crtSpan -lt 2048) 'Unexpected CRT startup span'
Verify-TableCall $a 0x01002363 484 0x01001238 0x0100124c 'msvcr80.dll!_initterm'
Verify-TableCall $a 0x01002363 484 0x01001250 0x01001258 'msvcr80.dll!_initterm_e'
Verify-TableCall $b $crt $crtSpan $xcA $xcZ 'msvcr80.dll!_initterm'
Verify-TableCall $b $crt $crtSpan $xiA $xiZ 'msvcr80.dll!_initterm_e'
if($RequireOriginalOrder){Require $orderMatches 'Global initialization order differs from original'}
[pscustomobject]@{OriginalSHA256=(Get-FileHash $original).Hash;RebuiltSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;Bodies=$bodies;WrapperBodyCount=5;WrapperBodyBytes=113;VerifiedReferences=@($references);CppInitializationOrderMatchesOriginal=$orderMatches;CppInitializationTable=$entries;CrtCallsUseCheckedTables=$true;CommonCInitializerRolesVerified=$true;FullCInitializationTableMatchesOriginal=$cTableMatches;AdditionalCInitializer=$additionalCInitializer;AtexitAdapterAndAfxInitializeDependencyVerified=$true;Scope='Five complete wrapper bodies with target-validated address normalization and actual C++ CRT table roles/order, including CRT table-call bounds/imports. C table roles/order reported separately; pre_c_init body not certified by this report. atexit adapter and SDK AfxInitialize checked as dependencies, not added to wrapper count. Internal _onexit/CRT startup bodies, external DLL implementations, RTTI/EH metadata, runtime execution, original source names and whole-file equality excluded.'}|ConvertTo-Json -Depth 7
