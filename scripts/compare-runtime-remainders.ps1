param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
$previous=. "$PSScriptRoot\compare-atl-codepage.ps1" -BuildDirectory $BuildDirectory
$previousReport=$previous|ConvertFrom-Json
$prolog=Symbol '__SEH_prolog4_GS';$epilog=Symbol '__SEH_epilog4_GS';$mul=Symbol '__allmul'
foreach($pair in @(@('__SEH_prolog4_GS','sehprolg4gs.obj'),@('__SEH_epilog4_GS','sehprolg4gs.obj'),@('__allmul','llmul.obj'))){Require ($map -match ('(?im)^\s*\S+\s+'+[regex]::Escape($pair[0])+'\s+\S+.*(?:msvcrt|selected-crt):'+ [regex]::Escape($pair[1]))) 'Runtime MAP member provenance differs'}
$prologReport=Compare-AtlBody 'GS SEH prologue' 0x0100270c $prolog 72 @(@(1,'absolute',0x01002768,(Symbol '__except_handler4'),'verified EH4 forwarding helper'),@(0x1e,'absolute',0x01004004,(Symbol '___security_cookie'),'security cookie'))
$epilogReport=Compare-AtlBody 'GS SEH epilogue' 0x01002754 $epilog 15 @(@(6,'call',0x0100263d,(Symbol '@__security_check_cookie@4'),'verified cookie checker'),@(11,'call',0x01002985,(Symbol '__SEH_epilog4'),'verified common SEH epilogue'))
Equal-Bytes (Bytes $a 0x01002c50 52) (Bytes $b $mul 52) 'CRT 64-bit multiplication helper'
$thunks=foreach($pair in @(@(0x0100282c,'__XcptFilter','msvcr80.dll!_XcptFilter'),@(0x01002be0,'__invoke_watson','msvcr80.dll!_invoke_watson'),@(0x01002bec,'__controlfp_s','msvcr80.dll!_controlfp_s'))){
 $new=Symbol $pair[1];Require ((Thunk $a $pair[0]) -ceq $pair[2] -and (Thunk $b $new) -ceq $pair[2]) 'Remaining import thunk identity differs'
 [pscustomobject]@{OriginalVA=('{0:X8}' -f $pair[0]);RebuiltVA=('{0:X8}' -f $new);Bytes=6;Import=$pair[2];IdentityVerified=$true}
}
foreach($r in @(@(0x01002707,5),@(0x01002763,5),@(0x01002826,6),@(0x01002832,14),@(0x01002bdb,5),@(0x01002be6,6),@(0x01002bf2,6),@(0x01002c4a,6),@(0x01002c84,28),@(0x01002d7e,2))){Require (@(Bytes $a $r[0] $r[1]|Where-Object {$_ -ne 0xcc}).Count -eq 0) 'Original padding content differs'}
[pscustomobject]@{OriginalSHA256=$previousReport.OriginalSHA256;RebuiltSHA256=$previousReport.RebuiltSHA256;GsPrologue=$prologReport;GsEpilogue=$epilogReport;Allmul=@{OriginalVA='01002C50';RebuiltVA=('{0:X8}' -f $mul);Bytes=52;RawExactMatch=$true};ImportThunks=@($thunks);Scope='Three complete runtime bodies and three import thunk identities; stated original CC gaps verified. Earlier gates enforced through ATL comparator. No whole image, padding option or original source identity claim; application body count unchanged.'}|ConvertTo-Json -Depth 8
