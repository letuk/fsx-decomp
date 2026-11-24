param([Parameter(Mandatory=$true)][string]$RunId)
if($RunId -notmatch '^[A-Za-z0-9_-]+$'){throw 'Invalid run ID'}
$run=Join-Path "$PSScriptRoot\..\..\vm-share\modules\fsx\runs" $RunId
if(!(Test-Path -LiteralPath "$run\status.json")){throw 'Unknown run'}
New-Item -ItemType File -Path "$run\STOP" -Force|Out-Null
Write-Output 'Stop requested. Current guest compilation may finish; no next job will be queued.'
