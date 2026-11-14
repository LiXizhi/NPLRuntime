[CmdletBinding()]
param(
    [string[]]$Renderers = @("d3d12", "vulkan", "d3d11", "opengl"),
    [int]$WorldPid = 530,
    [int]$TimeoutSeconds = 90,
    [string]$ScreenshotDir = ""
)

$ErrorActionPreference = "Stop"
$env:PARAENGINE_NO_CRASH_DIALOG = "1"

$scriptDir = $PSScriptRoot
$launcherBat = Join-Path $scriptDir "launch_paracraft_530.bat"
$repoRoot = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
$exePath = if ($env:PARAENGINE_EXE) { (Resolve-Path $env:PARAENGINE_EXE).Path } else { (Resolve-Path (Join-Path $repoRoot "build-win64\bin\ParaEngineClient.exe")).Path }

if ($ScreenshotDir -ne "") {
    New-Item -ItemType Directory -Force -Path $ScreenshotDir | Out-Null
    $ScreenshotDir = (Resolve-Path $ScreenshotDir).Path
}

$expectMap = @{
    "d3d12"      = "Backend: D3D12"
    "d3d12-warp" = "Backend: D3D12 (WARP)"
    "vulkan"     = "Backend: Vulkan"
    "d3d11"      = "Backend: D3D11"
    "d3d11-warp" = "Backend: D3D11"
    "opengl"     = "Backend: OpenGL"
    "webgpu"     = "Backend:"
    "auto"       = "Backend:"
}

foreach ($r in $Renderers) {
    Write-Host "=== Testing Paracraft World PID $WorldPid with renderer '$r' ==="
    $priorIds = @(Get-Process -Name ParaEngineClient -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id)

    & cmd.exe /d /c "`"$launcherBat`" $r $WorldPid"
    if ($LASTEXITCODE -ne 0) {
        throw "Launcher failed for renderer '$r' (exit code $LASTEXITCODE)"
    }

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    $ready = $false
    $apiUrl = $null
    $proc = $null

    try {
        do {
            $proc = Get-Process -Name ParaEngineClient -ErrorAction SilentlyContinue |
                Where-Object { $_.Path -eq $exePath -and $_.Id -notin $priorIds } |
                Select-Object -First 1

            if ($proc) {
                $ports = @(Get-NetTCPConnection -State Listen -OwningProcess $proc.Id -ErrorAction SilentlyContinue | Select-Object -ExpandProperty LocalPort -Unique)
                foreach ($port in $ports) {
                    $candidate = "http://127.0.0.1:$port/ajax/paracraft_cli"
                    try {
                        $health = Invoke-RestMethod "$candidate`?action=health" -TimeoutSec 3
                        if ($health.ok -and $health.result.worldEntered -and $health.result.kpProjectId -eq $WorldPid) {
                            $ready = $true
                            $apiUrl = $candidate
                            break
                        }
                    } catch {
                        continue
                    }
                }
            }

            if (-not $ready) {
                Start-Sleep -Seconds 1
            }
        } until ($ready -or (Get-Date) -ge $deadline)

        if (-not $ready) {
            throw "Timed out after ${TimeoutSeconds}s waiting for World PID $WorldPid to enter on renderer '$r'."
        }

        $statsBody = @{
            action = "run_npl_code"
            params = @{ code = "return ParaEngine.GetStats(0);" }
        } | ConvertTo-Json -Compress

        $statsResp = Invoke-RestMethod $apiUrl -Method Post -ContentType "application/json" -Body $statsBody -TimeoutSec 15
        $statsStr = [string]$statsResp.result.result
        $expectedSub = $expectMap[$r.ToLowerInvariant()]
        if ($expectedSub -and $statsStr.IndexOf($expectedSub, [System.StringComparison]::OrdinalIgnoreCase) -lt 0) {
            throw "Backend mismatch for '$r': expected '$expectedSub', got '$statsStr'"
        }

        if ($ScreenshotDir -ne "") {
            $shotResp = Invoke-RestMethod $apiUrl -Method Post -ContentType "application/json" -Body '{"action":"screenshot","params":{"maxSide":800}}' -TimeoutSec 20
            if (-not $shotResp.ok -or -not $shotResp.result.ok -or -not $shotResp.result.base64) {
                throw "Screenshot capture failed for '$r': $($shotResp | ConvertTo-Json -Compress)"
            }
            $shotBytes = [Convert]::FromBase64String($shotResp.result.base64)
            $ext = if ($shotResp.result.mimeType -eq "image/png") { "png" } else { "jpg" }
            $shotFile = Join-Path $ScreenshotDir "paracraft_${WorldPid}_${r}.$ext"
            [System.IO.File]::WriteAllBytes($shotFile, $shotBytes)
            Write-Host "  Saved screenshot: $shotFile ($($shotResp.result.width)x$($shotResp.result.height), $($shotBytes.Length) bytes)"
        }

        Write-Host "PASS [$r]: World PID $WorldPid entered cleanly ($($statsStr.Trim()))"
    } finally {
        if ($proc -and -not $proc.HasExited) {
            Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
            $proc.WaitForExit(5000) | Out-Null
        }
    }
}

Write-Host "=== ALL $($Renderers.Count) RENDERERS ENTERED WORLD PID $WorldPid SUCCESSFULLY ==="
