#Requires -Version 3.0
param (
    [bool]$shouldPause = $false
)

$wevtutil = "$env:WINDIR\system32\wevtutil.exe"
$basedir = Split-Path -Path $PSCommandPath -Parent
$manifest = "$basedir\UnityETWProvider.man"

# This requires elevation, so check if we have it, and if not, relaunch
$myWindowsID = [System.Security.Principal.WindowsIdentity]::GetCurrent()
$myWindowsPrincipal = New-Object System.Security.Principal.WindowsPrincipal($myWindowsID)
$adminRole = [System.Security.Principal.WindowsBuiltInRole]::Administrator

if (! $myWindowsPrincipal.IsInRole($adminRole))
{
    # Relaunch this script through elevation, and make it pause when it's done so the user can read the result
    $args = $MyInvocation.MyCommand.Definition + " -shouldPause:1"
    Start-Process "PowerShell" $args -Verb RunAs
    exit
}

$pinfo = New-Object System.Diagnostics.ProcessStartInfo
$pinfo.FileName = $wevtutil
$pinfo.RedirectStandardOutput = $true
$pinfo.RedirectStandardError = $true
$pinfo.UseShellExecute = $false
$pinfo.Arguments = @("uninstall-manifest", $manifest)

$process = New-Object System.Diagnostics.Process
$process.StartInfo = $pinfo
$process.Start() | Out-Null
$process.WaitForExit();

$stdout = $process.StandardOutput.ReadToEnd()
$stderr = $process.StandardError.ReadToEnd()

If ( $process.ExitCode -ne 0 )
{
    $errCode = $process.ExitCode
    Write-Error "An error of type $errCode occurred when uninstalling the provider."
    Write-Error $stderr
}
ElseIf ( $stdout -ne "" )
{
    Write-Output $stdout
}
else
{
    Write-Output "The Unity ETW provider was uninstalled successfully."    
}
If ($shouldPause)
{
    Pause
}