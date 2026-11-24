param([Parameter(Mandatory)][string]$BuildDirectory)
# Reference inventory, not an instruction mapping or byte-match certificate.
$ErrorActionPreference='Stop'
$project=[IO.Path]::GetFullPath("$PSScriptRoot\..")
$original=(Get-Content "$project\artifacts\fsx.exe\initinstance\original-instructions.json" -Raw|ConvertFrom-Json).instructions
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
$diagnostic=Get-Content "$BuildDirectory\initinstance-diagnostic.json" -Raw|ConvertFrom-Json
$rebuilt=@($diagnostic.Rows|Where-Object RebuiltAddress|Select-Object RebuiltAddress,Rebuilt -Unique)
$items=@(
 @('010020a4','?AfxEnableControlContainer@@YAXPAVCOccManager@@@Z'),
 @('01001760','?CheckWindows51SP2OrLater@@YGHXZ'),
 @('01001800','?CheckWindows52SP1OrLater@@YGHXZ'),
 @('01002c44','_memset'),
 @('010018e0','?InitializeOleAndMessageFilterPilot@@YAHXZ'),
 @('010020b0','?AfxGetModuleState@@YGPAVAFX_MODULE_STATE@@XZ'),
 @('010020bc','?Attach@CWnd@@QAEHPAUHWND__@@@Z'),
 @('0100263d','@__security_check_cookie@4'),
 @('01001d30','_ShowErrorMessage@24'),
 @('01001018','__imp__LoadLibraryA@4'),
 @('0100101c','__imp__GetProcAddress@8'),
 @('01001020','__imp__FreeLibrary@4'),
 @('01001028','__imp__GetModuleHandleA@4'),
 @('01001024','__imp__GetModuleFileNameA@12'),
 @('01001200','__imp___makepath_s'),
 @('01001204','__imp___splitpath_s'),
 @('0100102c','__imp__CreateActCtxA@4'),
 @('01001030','__imp__ReleaseActCtx@4'),
 @('01004384','?pfnGetApplication@@3P6APAVApplicationPilot@@XZA')
)
$rows=foreach($item in $items){
 $symbol=$item[1];$va=[Convert]::ToUInt32($item[0],16)
 $match=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($symbol)+'\s+([0-9A-Fa-f]{8})\b')
 if(!$match.Success){throw "Missing MAP symbol: $symbol"}
 $fromOriginal=@($original|Where-Object { $_.instruction -match ('(?i)0x0*'+$va.ToString('x')+'\b') })
 $fromRebuilt=@($rebuilt|Where-Object { $_.Rebuilt.Contains($symbol) })
 if(!$fromOriginal.Count -or !$fromRebuilt.Count){throw "Missing function reference: $symbol"}
 [pscustomobject]@{OriginalTarget=$item[0];RebuiltTarget=$match.Groups[1].Value;RebuiltSymbol=$symbol;OriginalReferenceCount=$fromOriginal.Count;RebuiltReferenceCount=$fromRebuilt.Count;SameReferenceCount=($fromOriginal.Count -eq $fromRebuilt.Count)}
}
$report=[ordered]@{Scope='Inventory only: original known caller targets and rebuilt MAP/DUMPBIN symbol references. No positional mapping, callee-body equivalence or byte-match certification. Register-indirect calls and string/data identities are not covered.';References=$rows}
$report|ConvertTo-Json -Depth 5|Set-Content "$BuildDirectory\initinstance-references.json"
[pscustomobject]@{Targets=$rows.Count;CountDifferences=@($rows|Where-Object {!$_.SameReferenceCount}).Count;Report="$BuildDirectory\initinstance-references.json"}|ConvertTo-Json
