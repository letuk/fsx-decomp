param([Parameter(Mandatory=$true)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
$project=[IO.Path]::GetFullPath("$PSScriptRoot\..\..")
$run=[IO.Path]::GetFullPath($BuildDirectory)
$checks=@('compare-api-lifecycle-object.cjs','compare-api-path-wrapper.cjs',
 'compare-api-initialization-error.cjs','compare-api-disk-dispatch.cjs',
 'compare-api-startup-stop.cjs','compare-api-path-string-helpers.cjs',
 'compare-api-main-keyboard-hook.cjs','compare-api-font-enumeration-close.cjs',
 'compare-api-foreground-idle-hook.cjs',
 'compare-api-cab-stream-wrappers.cjs',
 'compare-api-cab-stream-methods.cjs',
 'compare-api-cab-pool-registration.cjs',
 'compare-api-cab-stream-tables.cjs',
 'compare-api-cab-buffer-registration.cjs',
 'compare-api-file-stream-cache-lifecycle.cjs',
 'compare-api-configuration-write-wrappers.cjs',
 'compare-api-private-heap-release.cjs',
 'compare-api-window-update-region.cjs',
 'compare-api-panel-memory-dc.cjs',
 'compare-api-source-pass.cjs')
Push-Location $project
try {
 $results=foreach($check in $checks){
  $output=& node "$project\scripts\$check" $run 2>&1
  $output|Set-Content "$run\$check.log"
  if($LASTEXITCODE -ne 0){throw "Regression failed: $check; see retained log"}
  @{Check=$check;Passed=$true}
 }
 $warnings=Select-String -Path "$run\worker-output\build-log.txt" -Pattern 'error C|C4700|C4701|C4715|C4716'
 if($warnings){throw 'Compiler reported an error or uninitialized/missing return contract'}
 $results|ConvertTo-Json|Set-Content "$run\source-pass-regression.json"
 Write-Output "Passed $($checks.Count) comparators; no uninitialized/missing-return warnings."
}finally{Pop-Location}
