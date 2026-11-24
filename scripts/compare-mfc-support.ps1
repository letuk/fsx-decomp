param([Parameter(Mandatory)][string]$BuildDirectory,[string]$ControlDirectory="$PSScriptRoot\..\vm-share\modules\fsx\archive\40-crt-entry-selection")
$ErrorActionPreference='Stop'
$old=@(Get-Content "$ControlDirectory\source-hashes.json" -Raw|ConvertFrom-Json);$new=@(Get-Content "$BuildDirectory\source-hashes.json" -Raw|ConvertFrom-Json)
if(($old|Sort-Object File|ConvertTo-Json -Compress) -cne ($new|Sort-Object File|ConvertTo-Json -Compress)){throw 'Compiled source inputs changed'}
$before=[IO.File]::ReadAllText((Resolve-Path "$ControlDirectory\build.cmd"));$after=[IO.File]::ReadAllText((Resolve-Path "$BuildDirectory\build.cmd"))
$needle=' "\\vmware-host\Shared Folders\fsx-build\toolchains\vc80-rtm\mfcs80.lib" '
if(([regex]::Matches($after,[regex]::Escape($needle))).Count -ne 1 -or $after.Replace($needle,' mfcs80.lib ') -cne $before){throw 'Expected only MFC support library path replacement'}
$layout=(& "$PSScriptRoot\inventory-writable-data.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
$thunks=(& "$PSScriptRoot\compare-import-thunk-placement.ps1" -BuildDirectory $BuildDirectory|Out-String)|ConvertFrom-Json
$c=Get-Content "$BuildDirectory\comparison.json" -Raw|ConvertFrom-Json;$control=Get-Content "$ControlDirectory\comparison.json" -Raw|ConvertFrom-Json
if(($c.FunctionOrder.Functions|ConvertTo-Json -Compress) -cne ($control.FunctionOrder.Functions|ConvertTo-Json -Compress)){throw 'Ordered88-symbol prefix changed'}
if(($c.PeImports.ImportEntries|ConvertTo-Json -Compress) -cne ($control.PeImports.ImportEntries|ConvertTo-Json -Compress)){throw 'IAT contents or ordering changed'}
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
if($map -match '(?m)^\s*0002:\S+\s+__forceMFCManifest\s'){throw 'Manifest common symbol remains'}
$slots=@{};foreach($e in $c.PeImports.OriginalImportEntries){$slots[$e.Identity]=$e.Index}
$manifest=[xml](Get-Content "$BuildDirectory\fsx.exe.manifest" -Raw)
$dependencies=@($manifest.SelectNodes("//*[local-name()='dependentAssembly']/*[local-name()='assemblyIdentity']")|ForEach-Object {[pscustomobject]@{Name=$_.GetAttribute('name');Version=$_.GetAttribute('version')}})
[pscustomobject]@{RebuiltSHA256=$layout.RebuiltSHA256;ControlSHA256=$control.PeImports.RebuiltSHA256;CompiledSourcesIdentical=$true;OnlyCommandChange='Replace installed mfcs80.lib with pinned RTM mfcs80.lib';OrderedPrefix88Unchanged=$true;All80ThunkPositionsAndCCRunsMatch=($thunks.TrailingCCMismatches -eq 0);IatEntriesUnchanged=$true;ImportSlotDifferences=@($c.PeImports.ImportEntries|Where-Object {$slots[$_.Identity] -ne $_.Index}).Count;ManifestCommonSymbolAbsent=$true;DataVirtualSize=$layout.RebuiltVirtualSize;DataVirtualDelta=$layout.VirtualSizeDelta;MappedDataBytes=$layout.RebuiltRangeLedger.MappedUniqueBytes;UnassignedDataBytes=$layout.RebuiltRangeLedger.UnassignedBytes;GeneratedManifestDependencies=$dependencies;LinkerWarnings=@(Get-Content "$BuildDirectory\build-log.txt"|Where-Object {$_ -match 'warning LNK'});Scope='Controlled MFC support library replacement and existing full data/body gates; exact prefix/IAT controls and80-thunk checks. No original compiler/library attribution, original full data layout, manifest/resource equality or runtime equivalence claim.'}|ConvertTo-Json -Depth 7
