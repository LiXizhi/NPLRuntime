<#
.SYNOPSIS
    Build and serve NPLRuntimeUnified WebParacraft on Windows (WSL Emscripten build + native Windows HTTP server).

.EXAMPLE
    .\scripts\dev_web.ps1 build
    .\scripts\dev_web.ps1 serve -SearchRoot "C:/lxzsrc/ParaEngine/ParaWorld/" -SearchPath "script/" -ProjectId "530" -OpenBrowser
    .\scripts\dev_web.ps1 all -SearchRoot "C:/lxzsrc/ParaEngine/ParaWorld/" -SearchPath "script/" -ProjectId "530" -OpenBrowser
#>
[CmdletBinding()]
param(
    [ValidateSet("build", "configure", "serve", "run", "all")]
    [string]$Action = "build",
    [string]$SearchRoot = "",
    [string]$SearchPath = "script/",
    [string]$ProjectId = "530",
    [string]$World = "",
    [string]$EntryPage = "keepwork/local.html",
    [int]$Port = 8088,
    [string]$BindHost = "127.0.0.1",
    [string]$WslDistro = "Ubuntu",
    [switch]$NoRobot,
    [switch]$OpenBrowser
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot

function ConvertTo-WslPath([string]$WinPath) {
    $full = [IO.Path]::GetFullPath($WinPath)
    if ($full -match '^([A-Za-z]):\\(.*)$') {
        $drive = $Matches[1].ToLowerInvariant()
        $rest = $Matches[2].Replace('\', '/')
        return "/mnt/$drive/$rest"
    }
    throw "Cannot convert path to WSL path: $WinPath"
}

function Resolve-EffectiveSearchRoot([string]$ExplicitRoot) {
    if ($ExplicitRoot) {
        $norm = $ExplicitRoot.Replace('\', '/')
        if (-not $norm.EndsWith('/')) { $norm += '/' }
        return $norm
    }
    foreach ($cand in @("C:/lxzsrc/ParaEngine/ParaWorld/", "C:/Paracraft/")) {
        if (Test-Path -LiteralPath $cand) {
            return $cand
        }
    }
    return "C:/lxzsrc/ParaEngine/ParaWorld/"
}

function Build-LaunchUrl([string]$EffectiveRoot) {
    $connectHost = if ($BindHost -eq "0.0.0.0") { "127.0.0.1" } else { $BindHost }
    $page = $EntryPage.TrimStart('/')
    $queryParts = @("searchroot=$EffectiveRoot")
    if ($SearchPath) {
        $queryParts += "searchpath=$SearchPath"
    }
    if ($World) {
        $queryParts += "world=$World"
    } elseif ($ProjectId) {
        $queryParts += "pid=$ProjectId"
    }
    return "http://${connectHost}:${Port}/npl/webparacraft/${page}?" + ($queryParts -join "&")
}

function Test-LocalPortListening([string]$TargetHost, [int]$TargetPort) {
    $checkHost = if ($TargetHost -eq "0.0.0.0") { "127.0.0.1" } else { $TargetHost }
    $client = New-Object System.Net.Sockets.TcpClient
    try {
        $iar = $client.BeginConnect($checkHost, $TargetPort, $null, $null)
        $connected = $iar.AsyncWaitHandle.WaitOne(250, $false)
        if ($connected -and $client.Connected) {
            $client.EndConnect($iar)
            return $true
        }
        return $false
    } catch {
        return $false
    } finally {
        $client.Close()
    }
}

if ($Action -in @("configure", "build", "all")) {
    $wslRepo = ConvertTo-WslPath $RepoRoot
    $webRobot = if ($NoRobot) { "OFF" } else { "ON" }
    $subCmd = if ($Action -eq "configure") { "configure" } else { "cpp" }
    $distroArgs = @()
    if ($WslDistro) {
        $installed = (& wsl -l -q 2>$null) -replace "`0", "" | ForEach-Object { $_.Trim() } | Where-Object { $_ }
        if ($installed -contains $WslDistro) {
            $distroArgs = @("-d", $WslDistro)
        } else {
            $ubuntuMatch = $installed | Where-Object { $_ -like "Ubuntu*" } | Select-Object -First 1
            if ($ubuntuMatch) {
                $distroArgs = @("-d", $ubuntuMatch)
            }
        }
    }
    $distroLabel = if ($distroArgs.Count -ge 2) { $distroArgs[1] } else { "default" }
    Write-Host "[dev_web.ps1] Running WSL ($distroLabel): WEB_ROBOT=$webRobot scripts/dev_web.sh $subCmd"
    & wsl @distroArgs -- bash -lc "WEB_ROBOT=$webRobot '$wslRepo/scripts/dev_web.sh' $subCmd"
    if ($LASTEXITCODE -ne 0) {
        throw "WSL Emscripten build failed with exit code $LASTEXITCODE"
    }
}

if ($Action -in @("serve", "run", "all")) {
    $effectiveRoot = Resolve-EffectiveSearchRoot $SearchRoot
    $launchUrl = Build-LaunchUrl $effectiveRoot
    $shouldOpen = $OpenBrowser.IsPresent -or ($Action -eq "run")

    if (Test-LocalPortListening -TargetHost $BindHost -TargetPort $Port) {
        Write-Host "[dev_web.ps1] Server already running at http://${BindHost}:${Port} - reusing active instance."
        Write-Host "  Target URL: $launchUrl"
        if ($shouldOpen) {
            Write-Host "  Opening browser: $launchUrl"
            Start-Process $launchUrl | Out-Null
        }
        exit 0
    }

    $pyArgs = @(
        "-u",
        (Join-Path $PSScriptRoot "serve_webparacraft.py"),
        "--root", (Join-Path $RepoRoot "web"),
        "--port", "$Port",
        "--host", $BindHost,
        "--searchroot", $effectiveRoot.TrimEnd('/')
    )
    if ($shouldOpen) {
        $pyArgs += @("--open-url", $launchUrl)
    }
    Write-Host "[dev_web.ps1] Starting local server -> $launchUrl"
    & python @pyArgs
}
