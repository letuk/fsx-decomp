param([Parameter(Mandatory=$true)][string]$TestShare,[string]$RunId='test-replay',[int]$Count=2,[switch]$FailBuild)
# Integration fixture only: replay saved real artifacts to test queue/state logic.
# This does not compile a variant or establish a new code match.
$ErrorActionPreference='Stop'
$fixture="$PSScriptRoot\..\..\vm-share\modules\fsx\archive\41-rtm-mfc-support"
$deadline=[DateTime]::UtcNow.AddSeconds(90)
$completed=0
while($completed -lt $Count -and [DateTime]::UtcNow -lt $deadline){
 $jobs=@(Get-ChildItem "$TestShare\queue" -Directory -ErrorAction SilentlyContinue|Where-Object {$_.Name.StartsWith($RunId+'-') -and (Test-Path "$($_.FullName)\ready")})
 foreach($job in $jobs){
  Move-Item "$($job.FullName)\ready" "$($job.FullName)\working"
  New-Item -ItemType Directory -Path "$($job.FullName)\output\compiled-source" -Force|Out-Null
  foreach($file in @('fsx.exe','fsx.pdb','fsx.map','disassembly.txt','headers.txt','build-log.txt')){Copy-Item "$fixture\$file" "$($job.FullName)\output"}
  Get-ChildItem "$($job.FullName)\source" -File|Copy-Item -Destination "$($job.FullName)\output\compiled-source"
  @{Job=$job.Name;ExitCode=$(if($FailBuild){3}else{0});FixtureReplay=$true}|ConvertTo-Json|Set-Content "$($job.FullName)\receipt.json"
  Set-Content "$($job.FullName)\done" $job.Name
  $completed++
 }
 Start-Sleep -Milliseconds 200
}
if($completed -ne $Count){throw 'Replay fixture timed out'}
