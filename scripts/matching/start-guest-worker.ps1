param(
    [string]$GuestUser = 'Robin',
    [PSCredential]$GuestCredential,
    [string]$VMXPath = 'G:\Virtual Machines\Windows XP Professional\Windows XP Professional.vmx',
    [string]$VMRun = 'C:\Program Files\VMware\VMware Workstation\vmrun.exe',
    [string]$GuestWorker = '\\vmware-host\Shared Folders\fsx-build\worker.vbs'
)
# GuestUser denotes an explicitly passwordless account. Supply GuestCredential
# instead for password-protected accounts; credentials are never persisted.
$ErrorActionPreference = 'Stop'
$share = [IO.Path]::GetFullPath("$PSScriptRoot\..\..\vm-share")
if (!(Test-Path -LiteralPath $VMRun) -or !(Test-Path -LiteralPath $VMXPath)) { throw 'VMware executable or VM configuration not found' }
if (Test-Path -LiteralPath "$share\STOP-WORKER") { throw 'STOP-WORKER exists; remove it intentionally before starting the worker' }
$login = $GuestUser
$password = ''
if ($GuestCredential) { $login = $GuestCredential.UserName; $password = $GuestCredential.GetNetworkCredential().Password }
function Invoke-VMRun([string[]]$Command, [switch]$Authenticate) {
    $info = [Diagnostics.ProcessStartInfo]::new()
    $info.FileName = $VMRun
    $info.UseShellExecute = $false
    $info.CreateNoWindow = $true
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    foreach ($arg in @('-T','ws')) { $info.ArgumentList.Add($arg) }
    if ($Authenticate) {
        foreach ($arg in @('-gu',$login,'-gp',$password)) { $info.ArgumentList.Add($arg) }
    }
    foreach ($arg in $Command) { $info.ArgumentList.Add($arg) }
    $process = [Diagnostics.Process]::Start($info)
    $stdout = $process.StandardOutput.ReadToEndAsync()
    $stderr = $process.StandardError.ReadToEndAsync()
    if (!$process.WaitForExit(30000)) { $process.Kill(); throw 'VMware command timed out' }
    $output = $stdout.GetAwaiter().GetResult() + $stderr.GetAwaiter().GetResult()
    if ($process.ExitCode -ne 0) { throw "VMware command failed: $output" }
    $process.Dispose()
    return $output
}
try {
    $tools = Invoke-VMRun @('checkToolsState',$VMXPath)
    if ($tools.Trim() -ne 'running') { throw 'Start the VM and wait for VMware Tools before retrying' }
    $processes = Invoke-VMRun @('listProcessesInGuest',$VMXPath) -Authenticate
    function Has-Worker([string]$Processes) {
        return @($Processes -split '\r?\n' | Where-Object {
            $_ -match '(?i)cscript\.exe' -and $_.IndexOf($GuestWorker,[StringComparison]::OrdinalIgnoreCase) -ge 0
        }).Count -gt 0
    }
    if (Has-Worker $processes) { Write-Output 'XP build worker already running'; return }
    Invoke-VMRun @('runProgramInGuest',$VMXPath,'-noWait','-interactive','C:\WINDOWS\system32\cscript.exe','//nologo',$GuestWorker,'/takeover') -Authenticate | Out-Null
    for ($attempt=0; $attempt -lt 10; $attempt++) {
        Start-Sleep -Seconds 1
        $processes = Invoke-VMRun @('listProcessesInGuest',$VMXPath) -Authenticate
        if (Has-Worker $processes) { Write-Output 'XP build worker started from host'; return }
    }
    throw 'Worker process did not stay running; inspect the guest console and shared-folder access'
} finally { $password = $null }

