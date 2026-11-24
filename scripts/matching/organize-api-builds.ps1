# One-time relocation of the three completed initial API jobs. No builds/deletes.
$ErrorActionPreference='Stop'
$project=[IO.Path]::GetFullPath("$PSScriptRoot\..\..")
$share=Join-Path $project 'vm-share';$moduleRoot=Join-Path $share 'modules\api'
function Assert-WorkspacePath([string]$target){$full=[IO.Path]::GetFullPath($target);if(!$full.StartsWith($project+'\',[StringComparison]::OrdinalIgnoreCase)){throw "Path outside project: $full"};return $full}
$worker=Get-Content "$share\worker-state.json" -Raw|ConvertFrom-Json
if($worker.Status -ne 'ready' -or $worker.Job){throw 'Worker not idle'}
$lock=[IO.File]::Open("$share\controller.lock",[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
try {
 $ids=@('api-window-20261103-001','api-heap-20261103-001','api-heap-gl-20261103-002')
 foreach($id in $ids){
  $run="$moduleRoot\runs\$id"
  $manifest="$run\module.json"
  if((Test-Path -LiteralPath $manifest) -and ((Get-Content -LiteralPath $manifest -Raw|ConvertFrom-Json).JobDirectory -eq $run)){
   throw 'API build history is already consolidated under runs'
  }
 }
 # Preflight every source/destination before any relocation.
 foreach($id in $ids){$old=Assert-WorkspacePath "$share\runs\$id";$new=Assert-WorkspacePath "$moduleRoot\runs\$id";if((Test-Path $old) -eq (Test-Path $new)){throw 'Expected exactly one run location'};if(!(Test-Path "$share\queue\$id\done") -or (Test-Path "$share\queue\$id\ready")){throw 'Job not completed'};$r=Get-Content "$share\queue\$id\receipt.json" -Raw|ConvertFrom-Json;if($r.ExitCode -ne 0 -or $r.Job -cne $id){throw 'Unexpected receipt'};if((Test-Path "$moduleRoot\jobs\$id") -and @(Get-ChildItem "$moduleRoot\jobs\$id" -Force).Count){throw 'Job snapshot destination not empty'}}
 New-Item -ItemType Directory -Path "$moduleRoot\runs","$moduleRoot\jobs","$moduleRoot\archive" -Force|Out-Null
 $records=@()
 foreach($id in $ids){
  foreach($kind in @('runs','queue')){
   $old=Assert-WorkspacePath "$share\$kind\$id";$newKind=if($kind -eq 'queue'){'jobs'}else{'runs'};$new=Assert-WorkspacePath "$moduleRoot\$newKind\$id"
   $resumed=($kind -eq 'runs' -and !(Test-Path $old));$readRoot=if($resumed){$new}else{$old}
   $before=@(Get-ChildItem -LiteralPath $readRoot -Recurse -File|ForEach-Object {[pscustomobject]@{Relative=$_.FullName.Substring($readRoot.Length+1);SHA256=(Get-FileHash -LiteralPath $_.FullName).Hash}})
   if($kind -eq 'queue'){
    # The running worker keeps a directory enumeration handle. Preserve transport
    # folders and copy a verified module-owned job snapshot instead of restarting it.
    New-Item -ItemType Directory -Path $new -Force|Out-Null
    Copy-Item "$old\*" $new -Recurse
   }elseif(!$resumed){Move-Item -LiteralPath $old -Destination $new}
   if($resumed){foreach($entry in (Get-Content "$new\output-hashes.json" -Raw|ConvertFrom-Json)){if((Get-FileHash "$new\$($entry.File)").Hash -cne $entry.Hash){throw 'Resumed run differs from original output manifest'}}}
   foreach($f in $before){if((Get-FileHash -LiteralPath (Join-Path $new $f.Relative)).Hash -cne $f.SHA256){throw 'Relocation changed file bytes'}}
   $records+=@{Module='api.dll';OldPath=$old;NewPath=$new;Files=$before;AllHashesVerified=$true;Operation=$(if($kind -eq 'queue'){'verified snapshot; transport retained'}elseif($resumed){'resumed relocation; original output manifest checked'}else{'relocation'})}
  }
  $run=Assert-WorkspacePath "$moduleRoot\runs\$id"
  New-Item -ItemType Directory -Path "$run\worker-output"|Out-Null
  foreach($pair in @(@('fsx.map','api-pilot.map'),@('fsx.pdb','api-pilot.pdb'))){Copy-Item -LiteralPath "$run\$($pair[0])" -Destination "$run\$($pair[1])"}
  $relocated=@()
  foreach($name in @('fsx.exe','fsx.map','fsx.pdb','build-log.txt','headers.txt','disassembly.txt','compiled-source')){if(Test-Path -LiteralPath "$run\$name"){[void](Assert-WorkspacePath "$run\$name");[void](Assert-WorkspacePath "$run\worker-output\$name");Move-Item -LiteralPath "$run\$name" -Destination "$run\worker-output\$name";$relocated+=@{OldRelative=$name;NewRelative="worker-output/$name"}}}
  foreach($pair in @(@('fsx.exe','api-pilot.dll'),@('fsx.map','api-pilot.map'),@('fsx.pdb','api-pilot.pdb'))){if((Get-FileHash "$run\worker-output\$($pair[0])").Hash -cne (Get-FileHash "$run\$($pair[1])").Hash){throw 'Canonical alias hash mismatch'}}
  @{Module='api.dll';RunId=$id;RunDirectory=$run;JobDirectory="$moduleRoot\jobs\$id";CanonicalFiles=@('api-pilot.dll','api-pilot.map','api-pilot.pdb');WorkerOutputDirectory='worker-output';RelocatedFiles=$relocated;HistoricalManifest='output-hashes.json records original filenames; see relocation manifest';Scope='Completed original job/request/receipt/log bytes preserved; only filesystem organization and canonical aliases added'}|ConvertTo-Json -Depth 5|Set-Content "$run\module.json"
 }
 @{Date='2026-11-03';Module='api.dll';Records=$records;WithinRunRule='fsx.exe/map/pdb, build-log, headers, disassembly and compiled-source moved into worker-output; api-pilot.dll/map/pdb are canonical aliases';Scope='Filesystem relocation only; archived build commands, sources, receipts and binary content unchanged'}|ConvertTo-Json -Depth 8|Set-Content "$moduleRoot\relocation-20261103.json"
 Write-Output 'Three API runs relocated and completed jobs snapshotted; original hashes/canonical aliases verified. Shared transport queue retained.'
}finally{$lock.Dispose()}
