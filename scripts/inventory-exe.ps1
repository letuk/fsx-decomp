param([string]$Export="$PSScriptRoot\..\artifacts\fsx.exe\exe-inventory\ghidra-export.json",
      [string]$OutputDirectory="$PSScriptRoot\..\artifacts\fsx.exe\exe-inventory",
      [string]$ReferenceBuild="$PSScriptRoot\..\vm-share\modules\fsx\archive\41-rtm-mfc-support")
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$path="$PSScriptRoot\..\original\fsx.exe";$pe=Open-PE $path;Load-Imports $pe
$model=Get-Content -LiteralPath $Export -Raw|ConvertFrom-Json
if($model.program -ne 'fsx.exe' -or $model.imageBase -ne '01000000'){throw 'Unexpected Ghidra program export'}
New-Item -ItemType Directory -Path $OutputDirectory -Force|Out-Null
$raw=$pe.Bytes;$header=[BitConverter]::ToInt32($raw,60);$table=$header+24+[BitConverter]::ToUInt16($raw,$header+20)
$sections=for($i=0;$i -lt [BitConverter]::ToUInt16($raw,$header+6);$i++){
 $s=$table+40*$i
 [pscustomobject]@{Name=[Text.Encoding]::ASCII.GetString($raw,$s,8).Trim([char]0);RVA=[long][BitConverter]::ToUInt32($raw,$s+12);VirtualBytes=[BitConverter]::ToUInt32($raw,$s+8);RawBytes=[BitConverter]::ToUInt32($raw,$s+16);RawOffset=[long][BitConverter]::ToUInt32($raw,$s+20);Flags=[long][BitConverter]::ToUInt32($raw,$s+36)}
}
$sections|Export-Csv "$OutputDirectory\pe-sections.csv" -NoTypeInformation
$listedAddresses=@($model.unassignedInstructions|ForEach-Object address)
$thunks=foreach($section in $sections|Where-Object {$_.Flags -band 0x20000000}){
 for($i=0;$i+6 -le $section.RawBytes;$i++){
  $o=$section.RawOffset+$i
  if($raw[$o] -ne 255 -or $raw[$o+1] -ne 37){continue}
  $iat=[long][BitConverter]::ToUInt32($raw,$o+2)
  if(!$pe.Imports.ContainsKey($iat)){continue}
  $address=('{0:x8}' -f ($pe.Base+$section.RVA+$i))
  $stored=@($model.functions|Where-Object entry -EQ $address)
  $instruction=@($stored.instructions|Where-Object { $_.address -eq $address -and $_.length -eq 6 })
  [pscustomobject]@{Address=$address;Identity=$pe.Imports[$iat];StoredFunction=($address -in $model.functions.entry);ListedInstruction=($address -in $listedAddresses -or $instruction.Count -eq 1);CompleteStoredBody=(@($stored|Where-Object bodyBytes -EQ 6).Count -eq 1 -and $instruction.Count -eq 1)}
 }
}
$thunks|Export-Csv "$OutputDirectory\import-thunks.csv" -NoTypeInformation
$thunkByAddress=@{};foreach($t in $thunks){$thunkByAddress[$t.Address]=$t.Identity}
$known=@{
 '01001590'='SDK formatter';'01001600'='SDK copy worker';'01001650'='Application GetMessageMap';
 '01001660'='Application event constructor';'010016d0'='Application deleting destructor';
 '01001720'='Application ExitInstance';'01001760'='Windows version helper 5.1';'01001800'='Windows version helper 5.2';
 '010018a0'='OLE filter deleting destructor';'010018d0'='OLE pending override';'010018e0'='OLE/filter initialization';
 '01001cf0'='Current-message callback';'01001d00'='Application Run'
}
$near=@{'010019e0'='Application InitInstance';'01001d30'='ShowErrorMessage'}
$glue=@{
 '01001f10'='OLE EH delete cleanup';'01002ca0'='OLE unwind action';'01002ca8'='OLE EH handler';
 '01002ce0'='Global CWnd construction and atexit registration';'01002d00'='Global application construction and atexit registration';
 '01002d16'='MFC app-state dynamic initializer';'01002d40'='Global CWnd destruction';'01002d50'='Global application destruction'
}
$runtime=@{
 '010026e7'='CRT lock release (_unlock(8))';'010026f5'='CRT atexit callback adapter';
 '01002c0d'='MFC AfxInitialize (SDK counterpart)';'01002313'='CRT pre_cpp_init';
 '01002768'='CRT except_handler4 forwarding helper';'01002802'='CRT RTC termination walker'
 '01002547'='CRT pre_c_init';'010027d9'='CRT RTC initializer walker';
 '0100299e'='CRT default floating-point precision helper';'010029cc'='CRT shared zero-return setargv/matherr fallback'
 '010024dd'='Embedded CRT main exception filter/handler code';'01002529'='Embedded CRT startup-failure filter/handler code'
 '010028fd'='Embedded CRT image-helper access-violation filter/handler code'
 '01001f89'='ATL real thread codepage getter (returns3)'
 '0100270c'='CRT GS SEH prologue';'01002754'='CRT GS SEH epilogue';'01002c50'='CRT 64-bit multiplication helper'
 '010029c2'='Unreachable stack cleanup after noreturn Watson call within verified precision region'
}
function Row([string]$address,[int]$length,[string]$origin,[string]$name,[string]$instructions){
 $address=$address.ToLowerInvariant();$category='Open';$role='Unclassified code/fragment';$evidence='unverified';$identity=''
 if($known.ContainsKey($address)){$category='Reconstructed';$role=$known[$address];$evidence='complete body matched; see dedicated comparison scope'}
 elseif($near.ContainsKey($address)){$category='Application near-match';$role=$near[$address];$evidence='reconstructed, not exact'}
 elseif($thunkByAddress.ContainsKey($address)){$category='Import thunk';$identity=$thunkByAddress[$address];$role=$identity;$evidence='raw FF25 instruction and actual PE IAT identity'}
 elseif($address -in '01001ef0','01001f00'){$category='SDK inline stub';$role=if($address -eq '01001ef0'){'Serialize/Dump RET4'}else{'AssertValid RET'};$evidence='raw stub equality checked by vtable comparator'}
 elseif($glue.ContainsKey($address)){$category='Compiler/static-object glue';$role=$glue[$address];$evidence='observed calls/data flow; not separately counted as source match'}
 elseif($runtime.ContainsKey($address)){$category='Runtime/framework';$role=$runtime[$address];$evidence='runtime role from instructions/imports and SDK counterpart; not blanket byte equality'}
 elseif($name -match '^ATL::'){$category='Runtime/framework';$role=$name;$evidence='existing Ghidra model plus SDK library counterpart; original name not proven'}
 elseif($name -match 'security|onexit|alloca|SEH|ValidateImage|FindPE|IsNonwritable|tmainCRT|^entry$|report_gs'){$category='Runtime/framework';$role=$name;$evidence='existing model/runtime context; not a new project-code match'}
 elseif($address -eq '01002c03'){$category='MFC startup wrapper';$role='WinMain tail wrapper';$evidence='E9 to AfxWinMain import thunk, existing model'}
 [pscustomobject]@{Address=$address;Bytes=$length;Origin=$origin;Category=$category;Role=$role;Evidence=$evidence;ModelName=$name;ImportIdentity=$identity;Instructions=$instructions}
}
$rows=@()
foreach($f in $model.functions){$rows+=Row $f.entry $f.bodyBytes 'Stored Ghidra function' $f.name ($f.instructions.text -join '; ')}
$groups=@();$group=$null
foreach($ins in $model.unassignedInstructions){
 $address=[Convert]::ToInt64($ins.address,16)
 if($null -eq $group -or $group.End -ne $address){if($null -ne $group){$groups+=$group};$group=[pscustomobject]@{Start=$ins.address;End=$address;Bytes=0;Instructions=@()}}
 $group.End=$address+$ins.length;$group.Bytes+=$ins.length;$group.Instructions+=($ins.address+' '+$ins.text)
}
if($null -ne $group){$groups+=$group}
foreach($g in $groups){$rows+=Row $g.Start $g.Bytes 'Unassigned contiguous listing range' '' ($g.Instructions -join '; ')}
foreach($t in $thunks|Where-Object {!$_.ListedInstruction}){$rows+=Row $t.Address 6 'Raw PE thunk, not disassembled in model' '' 'FF25 import dispatch'}
# A runtime routine can be present even in Ghidra's undefined bytes. This one is
# validated by its entire fixed, relocation-free body against the linked CRT.
$ref=Open-PE "$ReferenceBuild\fsx.exe";$refMap=Get-Content "$ReferenceBuild\fsx.map" -Raw
Load-Imports $ref
function Library-Symbol([string]$name){$m=[regex]::Match($refMap,'(?m)^\s*\S+\s+'+[regex]::Escape($name)+'\s+([0-9a-fA-F]{8})\b');if(!$m.Success){throw "Missing library symbol $name"};return [Convert]::ToInt64($m.Groups[1].Value,16)}
function Library-Thunk($image,[long]$entry){$x=Bytes $image $entry 6;if($x[0] -ne 255 -or $x[1] -ne 37){throw 'Expected import thunk in library identification'};return $image.Imports[[long][BitConverter]::ToUInt32($x,2)]}
$afxEntry=Library-Symbol '?AfxInitialize@@YGHHK@Z'
$afxOld=Bytes $pe 0x01002c0d 38;$afxNew=Bytes $ref $afxEntry 38
$oldCall=0x01002c0dL+5+[BitConverter]::ToInt32($afxOld,1);$newCall=$afxEntry+5+[BitConverter]::ToInt32($afxNew,1)
if((Library-Thunk $pe $oldCall) -cne (Library-Thunk $ref $newCall)){throw 'AfxInitialize module-state import differs'}
if($pe.Imports[[long][BitConverter]::ToUInt32($afxOld,0x1b)] -cne $ref.Imports[[long][BitConverter]::ToUInt32($afxNew,0x1b)]){throw 'AfxInitialize setmbcp import differs'}
foreach($offset in 1,0x1b){[Array]::Copy($afxOld,$offset,$afxNew,$offset,4)}
if([Convert]::ToBase64String($afxOld) -cne [Convert]::ToBase64String($afxNew)){throw 'AfxInitialize body identification failed'}
foreach($row in $rows|Where-Object Address -EQ '01002c0d'){$row.Evidence='38-byte normalized SDK body equality, module-state and setmbcp import identities validated; not counted in thirteen source/helper bodies'}
$mul=[regex]::Match($refMap,'(?m)^\s*\S+\s+__allmul\s+([0-9a-fA-F]{8})\b')
if(!$mul.Success){throw 'Reference CRT __allmul missing'}
$mulBytes=Bytes $ref ([Convert]::ToInt64($mul.Groups[1].Value,16)) 52
$mulExact=[Convert]::ToBase64String((Bytes $pe 0x01002c50 52)) -ceq [Convert]::ToBase64String($mulBytes)
if(!$mulExact){throw 'Raw CRT multiply identification no longer holds'}
if('01002c50' -notin $model.functions.entry){$rows += [pscustomobject]@{Address='01002c50';Bytes=52;Origin='Raw bytes, undefined in Ghidra listing';Category='Runtime/framework';Role='CRT __allmul';Evidence='52 raw bytes equal linked msvcrt:llmul.obj body; separate library identification';ModelName='';ImportIdentity='';Instructions='Not disassembled by this inventory'}}
$rows=@($rows|Sort-Object Address)
$static=(& "$PSScriptRoot\compare-static-glue.ps1" -BuildDirectory $ReferenceBuild -RequireOriginalOrder|Out-String)|ConvertFrom-Json
$preC=(& "$PSScriptRoot\compare-pre-c-init.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json
$preCpp=(& "$PSScriptRoot\compare-pre-cpp-init.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json
$crtStartup=(& "$PSScriptRoot\compare-crt-startup.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json
$crtImageHelpers=(& "$PSScriptRoot\compare-crt-image-helpers.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json
$crtFailureStack=(& "$PSScriptRoot\compare-crt-failure-stack.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json
$crtOnexit=(& "$PSScriptRoot\compare-crt-onexit.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json
$atlCodepage=(& "$PSScriptRoot\compare-atl-codepage.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json
$runtimeRemainders=(& "$PSScriptRoot\compare-runtime-remainders.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json
$rttiMetadata=(& "$PSScriptRoot\compare-rtti.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json
$dataConstants=(& "$PSScriptRoot\compare-data-constants.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json
$peImports=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $ReferenceBuild -RequireImportContents|Out-String)|ConvertFrom-Json
$sectionLayout=(& "$PSScriptRoot\verify-section-layout.ps1" -BuildDirectory $ReferenceBuild -SectionLayout $(if('.rdata' -cin $peImports.RebuiltHeader.Sections.Name){'Separate'}else{'RdataIntoText'})|Out-String)|ConvertFrom-Json
$pdbDebug=(& "$PSScriptRoot\verify-debug-record.ps1" -BuildDirectory $ReferenceBuild -PdbPathMode $(if($peImports.RebuiltAuxiliaryMetadata.DebugRecords[0].PdbPath -ceq 'fsx.pdb'){'Basename'}else{'Worker'})|Out-String)|ConvertFrom-Json
$flagsMode=switch($peImports.RebuiltHeader.DllFlags){'8100'{'Original'}'0100'{'LargeAddressNx'}default{if($peImports.RebuiltHeader.CoffFlags -ceq '0123'){'LargeAddress'}else{'Default'}}}
$peHeaderSettings=(& "$PSScriptRoot\verify-pe-header-settings.ps1" -BuildDirectory $ReferenceBuild -HeaderFlagsMode $flagsMode -ImageVersionMode $(if($peImports.RebuiltHeader.ImageVersion -ceq '5.2'){'Original'}else{'Default'}) -OsVersionMode $(if($peImports.RebuiltHeader.OperatingSystemVersion -ceq '5.2'){'Original'}else{'Default'})|Out-String)|ConvertFrom-Json
$applicationOrder=$null
if(Test-Path -LiteralPath "$ReferenceBuild\source\function-order-input.h"){$applicationOrder=(& "$PSScriptRoot\compare-application-order.ps1" -BuildDirectory $ReferenceBuild|Out-String)|ConvertFrom-Json}
foreach($row in $rows|Where-Object Address -in '0100270c','01002754','01002c50'){$row.Evidence='GS helpers72/15 normalized-exact with2/2 checked targets; multiplication52 raw-exact. Complete stored bodies; special helper ABIs remain explicit. See runtime-remainders.md.'}
foreach($row in $rows|Where-Object Address -in '01001f1f','01001f89','01001f92'){$row.Evidence='ATL bodies101/4/100 verified, 5/0/8 mapped fields plus cached getter initial target/imports. Real getter is an independent identified entry, superseding fragment hypothesis; original5-byte gaps remain a layout difference. See atl-codepage.md.'}
foreach($row in $rows|Where-Object Address -in '01002651','010026e7','010026f5'){$row.Evidence='CRT onexit150/cleanup9/atexit18 normalized-exact, 14/1/1 verified fields, EH4 finally28/cleanup link and import/global mappings checked. Callback types distinguished; see crt-onexit.md.'}
foreach($row in $rows|Where-Object Address -in '01002a6d','01002790','010027a6','01002bb0'){$row.Evidence='CRT failure/stack comparator passes: reporter260 with33 validated fields and context/exception/pointer data; aligned probes22/22 normalized, base probe43 raw. Specialized EAX/ESP ABI preserved; see crt-failure-stack.md.'}
foreach($row in $rows|Where-Object Address -in '01002768','01002840','01002870','010028b7','010028fd','0100263d'){$row.Evidence='CRT image/dispatch comparator passes: dispatch35 normalized, PE leaves41/66 raw, nonwritable region108 including EH23 and scope28, cookie checker15 normalized. Stored parent85 remains distinct from EH labels; report_gsfailure/imported dispatch bodies excluded. See crt-image-helpers.md.'}
foreach($row in $rows|Where-Object Address -in '01002363','010024dd','01002529'){$row.Evidence='Complete enclosing 479-byte CRT region normalized-exact with 39 validated references and EH4 table. Stored parent is 425 normal-flow bytes; embedded EH code is separately labeled/listed. See full-crt-startup.md.'}
foreach($row in $rows|Where-Object Address -in '0100262e','01002940','01002985','010029d4'){$row.Evidence='CRT startup dependency verified: PE entry10, SEH prologue69, epilogue20, cookie initializer148 bytes by address; see full-crt-startup.md. No full CRT/EXE claim.'}
foreach($row in $rows|Where-Object Address -EQ '01002547'){$row.Evidence='Complete 226-byte normalized-exact CRT library identification, 27 validated reference fields; see pre-c-init.md. Separate from 18 application/helper/wrapper count.'}
foreach($row in $rows|Where-Object Address -EQ '01002313'){$row.Evidence='Complete 75-byte normalized-exact CRT library identification, 12 validated reference fields; stored function now fully bounded. See pre-cpp-init-and-exe-gaps.md.'}
foreach($row in $rows|Where-Object Address -in '010027d9','0100299e','010029cc','01002802'){$row.Evidence='Complete verified CRT dependency body (36/41/3/36 bytes by role); stored function bounds completed. Shared zero stub has setargv/matherr alias roles; see pre-cpp-init-and-exe-gaps.md.'}
foreach($body in $static.Bodies){
 foreach($row in $rows|Where-Object Address -EQ $body.OriginalVA){$row.Evidence=('Complete wrapper body normalized-exact; validated references and C++ initializer table order. C initializer table role/order match: '+$static.FullCInitializationTableMatchesOriginal+'. See static-initialization-matching.md and crt-library-context.md.')}
}
if(@($rows|ForEach-Object Address|Select-Object -Unique).Count -ne $rows.Count){throw 'Duplicate inventory addresses'}
$rows|Export-Csv "$OutputDirectory\inventory.csv" -NoTypeInformation
$undefined=foreach($range in $model.undefinedRanges){
 $bytes=Bytes $pe ([Convert]::ToInt64($range.address,16)) $range.length
 $nonpadding=@($bytes|Where-Object {$_ -ne 0 -and $_ -ne 0xcc}).Count
 $kind=if(!$nonpadding){'Only 00/CC bytes; padding or zero data'}elseif([Convert]::ToInt64($range.address,16) -lt 0x01001590){'Data-like area; do not treat as code by default'}else{'Open runtime/code/data range; contains undecoded bytes'}
 [pscustomobject]@{Address=$range.address;Bytes=$range.length;NonZeroNonCCBytes=$nonpadding;Classification=$kind;Prefix=[BitConverter]::ToString($bytes[0..([Math]::Min(15,$bytes.Length-1))])}
}
$undefined|Export-Csv "$OutputDirectory\undefined-ranges.csv" -NoTypeInformation
$resource=($sections|Where-Object Name -EQ '.rsrc').RawBytes
$summary=[ordered]@{OriginalSHA256=(Get-FileHash $path).Hash;ModelFunctions=$model.modelFunctionCount;InternalFunctions=$model.functions.Count;ExternalDeclarations=$model.externalFunctionCount;UnassignedInstructions=$model.unassignedInstructions.Count;UnassignedListingRanges=$groups.Count;RawImportThunks=$thunks.Count;UndisassembledImportThunks=@($thunks|Where-Object {!$_.ListedInstruction}).Count;ImportThunksWithoutStoredFunctions=@($thunks|Where-Object {!$_.StoredFunction}).Count;StoredImportThunksWithIncompleteBodies=@($thunks|Where-Object {$_.StoredFunction -and !$_.CompleteStoredBody}).Count;Rows=$rows.Count;RowCounts=@($rows|Group-Object Category|Select-Object Name,Count);ExecutableCoverage=$model.executableBlocks;UndefinedNonZeroRanges=@($undefined|Where-Object NonZeroNonCCBytes -GT 0).Count;RttiObservedShapeMatches=$rttiMetadata.ObservedAbsenceShapeMatches;VerifiedDataStringCount=$dataConstants.StringCount;VerifiedDataStringBytes=$dataConstants.StringBytes;ApplicationOrderedEntryCount=$(if($applicationOrder){$applicationOrder.Count}else{0});ApplicationOrderUniformDelta=$(if($applicationOrder){$applicationOrder.Delta}else{$null});SelectedPeHeaderFieldsMatchOriginal=$peHeaderSettings.AllSelectedFieldsMatchOriginal;PdbPathMatchesOriginal=$pdbDebug.PathMatchesOriginal;PdbRecordSizeMatchesOriginal=$pdbDebug.RecordSizeMatchesOriginal;NonResourceSectionTopologyMatches=$sectionLayout.NonResourceSectionNamesAndOrderMatch;CompleteImportCount=$peImports.ImportCount;ImportContentsMatch=$peImports.ImportContentsMatch;ImportContentsAndOrderMatch=$peImports.ImportContentsAndOrderMatch;ResourceRawBytes=$resource;ResourcePercentOfFile=[Math]::Round(100*$resource/$pe.Bytes.Length,2);Scope='Rows mix stored functions, listing fragments and raw thunk/runtime discoveries, NOT a recovered function count. Undefined executable bytes remain open; existing symbol names and provisional runtime roles are not original-name proofs.'}
$summary|ConvertTo-Json -Depth 6|Set-Content "$OutputDirectory\summary.json"
$intro='<h1>FSX EXE code inventory</h1><p>Stored functions, unassigned instruction ranges and raw discoveries are separate origins. Categories include provisional runtime identifications. This is not proof that every executable byte or original function boundary is recovered.</p>'
$rows|Select-Object Address,Bytes,Origin,Category,Role,Evidence,ModelName,ImportIdentity|ConvertTo-Html -Title 'FSX EXE inventory' -PreContent $intro|Set-Content "$OutputDirectory\inventory.html"
$summary|ConvertTo-Json -Depth 6
