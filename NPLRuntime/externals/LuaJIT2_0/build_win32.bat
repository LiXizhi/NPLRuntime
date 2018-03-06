@echo off

if "%WindowsSdkDir%" == "" (
    echo Start this batch script from Visual Studio Command Prompt
    goto :EOF
)

set DIR=%~dp0
set SRCDIR=%DIR%code
set DESTDIR=%DIR%prebuilt\win32

if exist %SRCDIR%\src\*.lib (
del %DESTDIR%\*.lib
)
if exist %SRCDIR%\src\*.dll (
del %DESTDIR%\*.dll
)

cd /d %SRCDIR%\src
call msvcbuild.bat

if exist %SRCDIR%\src\lua51.lib (
    copy %SRCDIR%\src\lua51.lib %DESTDIR%
)

if exist %SRCDIR%\src\lua51.dll (
    copy %SRCDIR%\src\lua51.dll %DESTDIR%
)

cd /d %DIR%
