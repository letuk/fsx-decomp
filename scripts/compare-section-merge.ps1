param([Parameter(Mandatory)][string]$ControlDirectory,[Parameter(Mandatory)][string]$MergedDirectory)
$ErrorActionPreference='Stop'
$cdata=(& "$PSScriptRoot\compare-data-constants.ps1" -BuildDirectory $ControlDirectory|Out-String)|ConvertFrom-Json
$mdata=(& "$PSScriptRoot\compare-data-constants.ps1" -BuildDirectory $MergedDirectory|Out-String)|ConvertFrom-Json
$ci=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $ControlDirectory -RequireImportContents|Out-String)|ConvertFrom-Json
$mi=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $MergedDirectory -RequireImportContents|Out-String)|ConvertFrom-Json
$cl=(& "$PSScriptRoot\verify-section-layout.ps1" -BuildDirectory $ControlDirectory -SectionLayout Separate|Out-String)|ConvertFrom-Json
$ml=(& "$PSScriptRoot\verify-section-layout.ps1" -BuildDirectory $MergedDirectory -SectionLayout RdataIntoText|Out-String)|ConvertFrom-Json
$cc=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"));$mc=[IO.File]::ReadAllText((Resolve-Path "$MergedDirectory\build.cmd"))
if($cc -cne $mc.Replace('/MERGE:.rdata=.text ','')){throw 'Commands differ beyond merge option'}
$cs=Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json;$ms=Get-Content "$MergedDirectory\source-hashes.json" -Raw|ConvertFrom-Json
if(($cs|Sort-Object File|ConvertTo-Json -Compress) -cne ($ms|Sort-Object File|ConvertTo-Json -Compress)){throw 'Sources differ'}
$equalDebugLength=($ci.RebuiltAuxiliaryMetadata.DebugRecords[0].Bytes -eq $mi.RebuiltAuxiliaryMetadata.DebugRecords[0].Bytes)
if(!$equalDebugLength){throw 'PDB path-length control differs'}
$stringAddresses=@($mdata.Strings|Where-Object {$_.OriginalVA -ceq $_.RebuiltVA})
[pscustomobject]@{ControlSHA256=$cdata.RebuiltSHA256;MergedSHA256=$mdata.RebuiltSHA256;IdenticalSourceFiles=$true;OnlyBuildCommandChange='/MERGE:.rdata=.text';DebugRecordBytes=$mi.RebuiltAuxiliaryMetadata.DebugRecords[0].Bytes;DebugPathLengthsAgree=$equalDebugLength;ControlSections=$cl.RebuiltSections;MergedLayout=$ml;OriginalIatRVA=$mi.OriginalHeader.Directories[12].Address;MergedIatRVA=$mi.RebuiltHeader.Directories[12].Address;OriginalDebugDirectoryRVA=$mi.OriginalHeader.Directories[6].Address;MergedDebugDirectoryRVA=$mi.RebuiltHeader.Directories[6].Address;OriginalLoadConfigRVA=$mi.OriginalHeader.Directories[10].Address;MergedLoadConfigRVA=$mi.RebuiltHeader.Directories[10].Address;ExactAddressStrings=$stringAddresses;ExactStringAddressCount=$stringAddresses.Count;OriginalEntryRVA=$mi.OriginalHeader.EntryRVA;MergedEntryRVA=$mi.RebuiltHeader.EntryRVA;EntryRvaDelta=$mi.RebuiltHeader.EntryRVA-$mi.OriginalHeader.EntryRVA;ImportContentsMatch=$mi.ImportContentsMatch;ImportContentsAndOrderMatch=$mi.ImportContentsAndOrderMatch;Scope='Matched-source/command/PDB-length section-merge experiment with runtime/data/import gates. Non-resource section placement/raw-size/access matches are separate from differing virtual sizes, code/import order, debug identity/path and actual section contents; no whole binary equality claim.'}|ConvertTo-Json -Depth 9
