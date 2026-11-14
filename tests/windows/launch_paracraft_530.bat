@echo off
setlocal EnableExtensions

rem ============================================================================
rem Launch Paracraft (../ParaEngine/Paraworld/) on Windows with ParaRHI backend
rem Usage:
rem   launch_paracraft_530.bat [renderer] [world_pid] [--new-instance] [extra_args...]
rem
rem Supported renderers:
rem   auto | d3d12 | d3d12-warp | vulkan | d3d11 | d3d11-warp | opengl | webgpu | null
rem
rem Examples:
rem   launch_paracraft_530.bat d3d12
rem   launch_paracraft_530.bat vulkan 530 --new-instance
rem   launch_paracraft_530.bat d3d11-warp 530
rem ============================================================================

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..\..") do set "REPO_ROOT=%%~fI"

if not defined PARAWORLD_ROOT (
    for %%I in ("%REPO_ROOT%\..\ParaEngine\Paraworld") do set "PARAWORLD_ROOT=%%~fI"
)

if not exist "%PARAWORLD_ROOT%\paraengine.sig" (
    if not exist "%PARAWORLD_ROOT%\script" (
        echo [ERROR] Paraworld directory not found at "%PARAWORLD_ROOT%".
        echo         Set PARAWORLD_ROOT to your ParaEngine\Paraworld checkout path.
        exit /b 1
    )
)

if not defined PARAENGINE_EXE (
    if exist "%REPO_ROOT%\build-win64\bin\ParaEngineClient.exe" (
        set "PARAENGINE_EXE=%REPO_ROOT%\build-win64\bin\ParaEngineClient.exe"
    ) else if exist "%REPO_ROOT%\build\bin\ParaEngineClient.exe" (
        set "PARAENGINE_EXE=%REPO_ROOT%\build\bin\ParaEngineClient.exe"
    ) else if exist "%REPO_ROOT%\ParaWorld\bin64\ParaEngineClient.exe" (
        set "PARAENGINE_EXE=%REPO_ROOT%\ParaWorld\bin64\ParaEngineClient.exe"
    ) else (
        echo [ERROR] Built ParaEngineClient.exe not found under "%REPO_ROOT%\build-win64\bin\".
        echo         Build first with:
        echo           cmake -S "%REPO_ROOT%\NPLRuntime" -B "%REPO_ROOT%\build-win64"
        echo           cmake --build "%REPO_ROOT%\build-win64" --config Release -- /m
        exit /b 1
    )
)

set "REQ_RENDERER=%~1"
if "%REQ_RENDERER%"=="" set "REQ_RENDERER=auto"
if not "%~1"=="" shift

set "WORLD_PID=530"
set "NEW_INSTANCE=0"
set "EXTRA_ARGS="

:parse_args
if "%~1"=="" goto args_done
if /I "%~1"=="--new-instance" (
    set "NEW_INSTANCE=1"
    shift
    goto parse_args
)
rem If first positional arg after renderer is all digits, treat it as world PID
echo(%~1| findstr /R "^[0-9][0-9]*$" >nul
if not errorlevel 1 (
    set "WORLD_PID=%~1"
    shift
    goto parse_args
)
set "EXTRA_ARGS=%EXTRA_ARGS% %1"
shift
goto parse_args

:args_done
set "RHI_FLAG=--renderer=%REQ_RENDERER%"
if /I "%REQ_RENDERER%"=="d3d12-warp" set "RHI_FLAG=--renderer=d3d12 --d3d12-warp"
if /I "%REQ_RENDERER%"=="d3d12_warp" set "RHI_FLAG=--renderer=d3d12 --d3d12-warp"
if /I "%REQ_RENDERER%"=="d3d11-warp" set "RHI_FLAG=--renderer=d3d11 --d3d11-warp"
if /I "%REQ_RENDERER%"=="d3d11_warp" set "RHI_FLAG=--renderer=d3d11 --d3d11-warp"

pushd "%PARAWORLD_ROOT%" || exit /b 1

if "%NEW_INSTANCE%"=="1" goto launch

rem Match Paraworld\bin\paracraft-cli-530.bat: release port 8099 if occupied by a prior ParaEngineClient
powershell.exe -NoLogo -NoProfile -NonInteractive -Command ^
    "try { $deadline = (Get-Date).AddSeconds(30); while (@([System.Net.NetworkInformation.IPGlobalProperties]::GetIPGlobalProperties().GetActiveTcpListeners() | Where-Object { $_.Port -eq 8099 }).Count -gt 0) { if ((Get-Date) -ge $deadline) { throw 'Port 8099 is still listening after 30 seconds. Pass --new-instance to keep existing instances.' }; Get-Process -Name ParaEngineClient -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue; Start-Sleep -Milliseconds 250 }; exit 0 } catch { Write-Error $_; exit 1 }"
if errorlevel 1 (
    popd
    exit /b 1
)

:launch
echo [ParaRHI] Launching Paracraft (World PID %WORLD_PID%)
echo   Exe      : "%PARAENGINE_EXE%"
echo   WorkDir  : "%CD%"
echo   Renderer : %RHI_FLAG%

start "" "%PARAENGINE_EXE%" %RHI_FLAG% mc="true" debug="main" isDevEnv="true" noupdate="true" %EXTRA_ARGS% paracraft://cmd/loadworld/%WORLD_PID%

popd
endlocal
exit /b 0
