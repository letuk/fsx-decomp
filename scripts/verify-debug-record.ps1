param([Parameter(Mandatory)][string]$BuildDirectory,[Parameter(Mandatory)][ValidateSet('Worker','Basename')][string]$PdbPathMode)
$ErrorActionPreference='Stop'
$pe=(& "$PSScriptRoot\compare-pe-imports.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
$old=@($pe.OriginalAuxiliaryMetadata.DebugRecords);$new=@($pe.RebuiltAuxiliaryMetadata.DebugRecords)
if($old.Count -ne 1 -or $new.Count -ne 1){throw 'Expected exactly one debug directory record'}
$a=$old[0];$b=$new[0]
if($a.Type -ne 2 -or $b.Type -ne 2 -or $a.Signature -cne 'RSDS' -or $b.Signature -cne 'RSDS' -or $a.Age -ne 1 -or $b.Age -ne 1){throw 'CodeView type/signature/age differs from observed shape'}
if($a.PdbPath -cne 'fsx.pdb' -or $a.Bytes -ne 32){throw 'Original debug record anchor differs'}
if($b.Bytes -ne 24+[Text.Encoding]::ASCII.GetByteCount($b.PdbPath)+1){throw 'Debug record size does not match terminated path extent'}
if($PdbPathMode -eq 'Basename' -and ($b.PdbPath -cne 'fsx.pdb' -or $b.Bytes -ne 32)){throw 'Debug path/extent differs from required basename mode'}
if($PdbPathMode -eq 'Worker' -and $b.PdbPath -notlike 'C:\fsx-build\matching-jobs\*\fsx.pdb'){throw 'Debug path differs from required worker mode'}
if(!(Test-Path -LiteralPath "$BuildDirectory\fsx.pdb")){throw 'Linked PDB artifact missing'}
[pscustomobject]@{Mode=$PdbPathMode;RequestedModeVerified=$true;OriginalSHA256=$pe.OriginalSHA256;RebuiltSHA256=$pe.RebuiltSHA256;OriginalRecord=$a;RebuiltRecord=$b;PathMatchesOriginal=($a.PdbPath -ceq $b.PdbPath);RecordSizeMatchesOriginal=($a.Bytes -eq $b.Bytes);GuidMatchesOriginal=($a.GuidRawBytes -ceq $b.GuidRawBytes);TimestampMatchesOriginal=($a.Timestamp -eq $b.Timestamp);DataRvaMatchesOriginal=($a.DataRVA -ceq $b.DataRVA);RawFileOffsetMatchesOriginal=($a.FileOffset -eq $b.FileOffset);Scope='Requested embedded PDB path/terminated record extent, CodeView type/RSDS/age and PDB artifact presence checked. Original GUID/timestamp differences remain reported; no PDB content identity, debugger session, deterministic whole build or runtime claim.'}|ConvertTo-Json -Depth 6
