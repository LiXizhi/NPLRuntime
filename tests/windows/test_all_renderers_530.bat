@echo off
setlocal
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0verify_all_renderers_530.ps1" %*
exit /b %ERRORLEVEL%
