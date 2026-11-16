@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem ============================================================================
rem NPLRuntimeUnified -- Windows Build & Run Entry Point
rem
rem Builds or runs NPLRuntime targets from a standard Windows terminal (cmd,
rem PowerShell, VS Code task, or AI agent shell -- no Developer Command Prompt
rem required).
rem
rem Usage:
rem   build_win32.bat                              Build Windows Client (Release)
rem   build_win32.bat [client] [Release|Debug]     Build Windows Client -> build-win64\bin\
rem   build_win32.bat debug                        Shorthand for: client Debug
rem   build_win32.bat server [Release|Debug]       Build Headless Server -> build-server\bin\
rem   build_win32.bat web [build|serve|run|all]    Build/serve WebAssembly via WSL + Python
rem   build_win32.bat android [gradle_task]        Build Android APK (Platform\AndroidStudio)
rem   build_win32.bat picovr [gradle_task]         Build PICO VR OpenXR APK (Platform\PicoVR)
rem   build_win32.bat run [renderer] [world_pid]   Launch Paracraft Client (default: auto 530)
rem   build_win32.bat run-server [script.lua]      Run ParaEngineServer (default: helloworld.lua)
rem   build_win32.bat test [Release|Debug]         Run CTest verification suite in build-win64
rem   build_win32.bat clean [client|server]        Clean build-win64 or build-server
rem   build_win32.bat --help                       Show this help message
rem
rem Options (for client/server builds):
rem   --configure / -c     Force CMake re-configuration before building
rem   --clean-first        Clean target before building (rebuild)
rem   --install-deps       Auto-download CMake / Boost 1.85 if missing
rem ============================================================================

set "REPO_ROOT=%~dp0"
if "%REPO_ROOT:~-1%"=="\" set "REPO_ROOT=%REPO_ROOT:~0,-1%"

rem Parse primary command
set "MODE=client"
set "CONFIG=Release"
set "FORCE_CONFIGURE=0"
set "CLEAN_FIRST=0"
set "INSTALL_DEPS=0"
set "ENABLE_OCE=0"
set "ENABLE_ROBOTICS=1"

if /I "%~1"=="--help" goto :usage
if /I "%~1"=="-h" goto :usage
if /I "%~1"=="/?" goto :usage
if /I "%~1"=="help" goto :usage

if /I "%~1"=="web" (
    shift
    goto :cmd_web
)
if /I "%~1"=="android" (
    shift
    goto :cmd_android
)
if /I "%~1"=="picovr" (
    shift
    goto :cmd_picovr
)
if /I "%~1"=="run" (
    shift
    goto :cmd_run_client
)
if /I "%~1"=="run-server" (
    shift
    goto :cmd_run_server
)
if /I "%~1"=="test" (
    shift
    goto :cmd_test
)
if /I "%~1"=="clean" (
    shift
    goto :cmd_clean
)

:parse_build_args
if "%~1"=="" goto :args_parsed
if /I "%~1"=="client" (
    set "MODE=client"
    shift
    goto :parse_build_args
)
if /I "%~1"=="server" (
    set "MODE=server"
    shift
    goto :parse_build_args
)
if /I "%~1"=="debug" (
    set "CONFIG=Debug"
    shift
    goto :parse_build_args
)
if /I "%~1"=="release" (
    set "CONFIG=Release"
    shift
    goto :parse_build_args
)
if /I "%~1"=="relwithdebinfo" (
    set "CONFIG=RelWithDebInfo"
    shift
    goto :parse_build_args
)
if /I "%~1"=="minsizerel" (
    set "CONFIG=MinSizeRel"
    shift
    goto :parse_build_args
)
if /I "%~1"=="--configure" (
    set "FORCE_CONFIGURE=1"
    shift
    goto :parse_build_args
)
if /I "%~1"=="-c" (
    set "FORCE_CONFIGURE=1"
    shift
    goto :parse_build_args
)
if /I "%~1"=="--clean-first" (
    set "CLEAN_FIRST=1"
    shift
    goto :parse_build_args
)
if /I "%~1"=="--install-deps" (
    set "INSTALL_DEPS=1"
    shift
    goto :parse_build_args
)
if /I "%~1"=="--oce" (
    set "ENABLE_OCE=1"
    shift
    goto :parse_build_args
)
if /I "%~1"=="--robotics" (
    set "ENABLE_ROBOTICS=1"
    shift
    goto :parse_build_args
)
if /I "%~1"=="--no-robotics" (
    set "ENABLE_ROBOTICS=0"
    shift
    goto :parse_build_args
)
echo [build_win32] Unknown argument: %~1
echo Run "build_win32.bat --help" for usage.
exit /b 1

:args_parsed
call :ensure_cmake || exit /b 1

if /I "%MODE%"=="server" (
    set "BUILD_DIR=%REPO_ROOT%\build-server"
    set "CMAKE_MODE_ARGS=-DNPLRUNTIME_SERVER=ON"
    set "EXPECTED_BIN=%REPO_ROOT%\build-server\bin\ParaEngineServer.exe"
) else (
    set "BUILD_DIR=%REPO_ROOT%\build-win64"
    set "CMAKE_MODE_ARGS=-DNPLRUNTIME_SERVER=OFF -DNPLRUNTIME_RENDERER=OPENGL"
    if "%ENABLE_ROBOTICS%"=="1" (
        set "CMAKE_MODE_ARGS=!CMAKE_MODE_ARGS! -DNPLRUNTIME_ENABLE_ROBOTICS=ON"
    ) else (
        set "CMAKE_MODE_ARGS=!CMAKE_MODE_ARGS! -DNPLRUNTIME_ENABLE_ROBOTICS=OFF -DNPLRUNTIME_MUJOCO=OFF -DNPLRUNTIME_ONNXRUNTIME=OFF"
    )
    if /I "%CONFIG%"=="Debug" (
        set "EXPECTED_BIN=%REPO_ROOT%\build-win64\bin\ParaEngineClient_d.exe"
    ) else (
        set "EXPECTED_BIN=%REPO_ROOT%\build-win64\bin\ParaEngineClient.exe"
    )
)
if "%ENABLE_OCE%"=="1" set "CMAKE_MODE_ARGS=%CMAKE_MODE_ARGS% -DNPLRUNTIME_OCE=ON"

set "NEED_CONFIGURE=%FORCE_CONFIGURE%"
if not exist "%BUILD_DIR%\CMakeCache.txt" set "NEED_CONFIGURE=1"
if /I "%MODE%"=="client" if "%ENABLE_ROBOTICS%"=="1" if exist "%BUILD_DIR%\CMakeCache.txt" (
    findstr /C:"NPLRUNTIME_ENABLE_ROBOTICS:BOOL=ON" "%BUILD_DIR%\CMakeCache.txt" >nul 2>&1 || (
        echo [build_win32] Enabling MuJoCo + ONNX Runtime in "%BUILD_DIR%"...
        set "NEED_CONFIGURE=1"
    )
)

if "%NEED_CONFIGURE%"=="1" (
    call :resolve_boost || exit /b 1
    echo [build_win32] Configuring %MODE% in "%BUILD_DIR%" ^(BOOST_ROOT=!BOOST_ROOT!^)...
    set "GEN_ARGS=-A x64"
    if defined CMAKE_GENERATOR (
        set "GEN_ARGS=-G "%CMAKE_GENERATOR%" -A x64"
    )
    cmake -S "%REPO_ROOT%\NPLRuntime" -B "%BUILD_DIR%" !GEN_ARGS! -DBOOST_ROOT="!BOOST_ROOT!" %CMAKE_MODE_ARGS%
    if errorlevel 1 (
        echo [build_win32] ERROR: CMake configuration failed.
        exit /b 1
    )
)

set "CLEAN_ARG="
if "%CLEAN_FIRST%"=="1" set "CLEAN_ARG=--clean-first"

echo [build_win32] Building %MODE% ^(%CONFIG%^) in "%BUILD_DIR%"...
cmake --build "%BUILD_DIR%" --config %CONFIG% %CLEAN_ARG% --parallel -- /v:minimal
if errorlevel 1 (
    echo [build_win32] ERROR: Build failed.
    exit /b 1
)

echo.
echo [build_win32] Build succeeded!
echo   Binary : %EXPECTED_BIN%
echo   Copied : %REPO_ROOT%\ParaWorld\bin64\
endlocal
exit /b 0

rem ============================================================================
rem Subcommands
rem ============================================================================

:cmd_web
set "WEB_ACTION=%~1"
if "%WEB_ACTION%"=="" set "WEB_ACTION=build"
if not "%~1"=="" shift
set "WEB_REST="
:collect_web_args
if "%~1"=="" goto :run_web_ps1
set "WEB_REST=!WEB_REST! "%~1""
shift
goto :collect_web_args
:run_web_ps1
powershell -NoProfile -ExecutionPolicy Bypass -File "%REPO_ROOT%\scripts\dev_web.ps1" %WEB_ACTION% %WEB_REST%
exit /b %ERRORLEVEL%

:cmd_android
set "GRADLE_TASK=%~1"
if "%GRADLE_TASK%"=="" set "GRADLE_TASK=assembleDebug"
pushd "%REPO_ROOT%\NPLRuntime\Platform\AndroidStudio" || exit /b 1
call gradlew.bat %GRADLE_TASK%
set "RC=%ERRORLEVEL%"
popd
exit /b %RC%

:cmd_picovr
set "GRADLE_TASK=%~1"
if "%GRADLE_TASK%"=="" set "GRADLE_TASK=:PicoVR:assembleDebug"
pushd "%REPO_ROOT%\NPLRuntime\Platform\PicoVR" || exit /b 1
call gradlew.bat %GRADLE_TASK%
set "RC=%ERRORLEVEL%"
popd
exit /b %RC%

:cmd_run_client
call "%REPO_ROOT%\tests\windows\launch_paracraft_530.bat" %1 %2 %3 %4 %5 %6 %7 %8 %9
exit /b %ERRORLEVEL%

:cmd_run_server
set "SERVER_EXE=%REPO_ROOT%\build-server\bin\ParaEngineServer.exe"
if not exist "%SERVER_EXE%" (
    echo [build_win32] ERROR: "%SERVER_EXE%" not found. Run "build_win32.bat server" first.
    exit /b 1
)
set "SCRIPT_ARG=%~1"
if "%SCRIPT_ARG%"=="" set "SCRIPT_ARG=%REPO_ROOT%\NPLRuntime\tests\helloworld.lua"
echo [build_win32] Running ParaEngineServer with bootstrapper="%SCRIPT_ARG%"...
"%SERVER_EXE%" bootstrapper="%SCRIPT_ARG%"
exit /b %ERRORLEVEL%

:cmd_test
call :ensure_cmake || exit /b 1
set "TEST_CFG=%~1"
if "%TEST_CFG%"=="" set "TEST_CFG=Release"
ctest --test-dir "%REPO_ROOT%\build-win64" -C %TEST_CFG% --output-on-failure
exit /b %ERRORLEVEL%

:cmd_clean
call :ensure_cmake || exit /b 1
set "CLEAN_TARGET=%~1"
if /I "%CLEAN_TARGET%"=="server" (
    cmake --build "%REPO_ROOT%\build-server" --target clean --config Release
) else (
    cmake --build "%REPO_ROOT%\build-win64" --target clean --config Release
    cmake --build "%REPO_ROOT%\build-win64" --target clean --config Debug
)
exit /b %ERRORLEVEL%

rem ============================================================================
rem Helpers: CMake & Boost detection
rem ============================================================================

:ensure_cmake
cmake --version >nul 2>&1
if not errorlevel 1 exit /b 0
if exist "C:\Program Files\CMake\bin\cmake.exe" (
    set "PATH=C:\Program Files\CMake\bin;!PATH!"
    exit /b 0
)
if "%INSTALL_DEPS%"=="1" (
    call :install_cmake
    exit /b !ERRORLEVEL!
)
echo [build_win32] ERROR: CMake not found in PATH or "C:\Program Files\CMake\bin".
echo Install CMake ^>= 3.15 or re-run with --install-deps.
exit /b 1

:resolve_boost
if defined BOOST_ROOT (
    if exist "!BOOST_ROOT!" exit /b 0
)
for %%D in (
    "C:\lxzsrc\externals\boost_1_90_0"
    "C:\lxzsrc\externals\boost_1_85_0"
    "C:\local\boost_1_90_0"
    "C:\local\boost_1_85_0"
    "C:\Boost"
) do (
    if exist "%%~D" (
        set "BOOST_ROOT=%%~D"
        exit /b 0
    )
)
if "%INSTALL_DEPS%"=="1" (
    call :install_boost
    exit /b !ERRORLEVEL!
)
echo [build_win32] ERROR: Boost not found.
echo Set BOOST_ROOT to your Boost directory ^(e.g. C:\lxzsrc\externals\boost_1_90_0 or C:\local\boost_1_85_0^),
echo or re-run with --install-deps to download and build Boost 1.85.0 automatically.
exit /b 1

:install_cmake
echo [build_win32] Downloading CMake 3.28.0...
if not exist "%TEMP%\cmake-installer.msi" (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; (New-Object Net.WebClient).DownloadFile('https://github.com/Kitware/CMake/releases/download/v3.28.0/cmake-3.28.0-windows-x86_64.msi', '%TEMP%\cmake-installer.msi')" || exit /b 1
)
msiexec /i "%TEMP%\cmake-installer.msi" /quiet /qn /norestart
timeout /t 5 >nul
if exist "C:\Program Files\CMake\bin\cmake.exe" set "PATH=C:\Program Files\CMake\bin;!PATH!"
cmake --version >nul 2>&1
exit /b %ERRORLEVEL%

:install_boost
set "BOOST_INSTALL_DIR=C:\local\boost_1_85_0"
if not exist "C:\local" mkdir "C:\local"
if not exist "%BOOST_INSTALL_DIR%\bootstrap.bat" (
    if not exist "%TEMP%\boost_1_85_0.tar.bz2" (
        echo [build_win32] Downloading Boost 1.85.0...
        powershell -NoProfile -ExecutionPolicy Bypass -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; (New-Object Net.WebClient).DownloadFile('https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0.tar.bz2', '%TEMP%\boost_1_85_0.tar.bz2')" || exit /b 1
    )
    echo [build_win32] Extracting Boost to C:\local...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "tar -xf '%TEMP%\boost_1_85_0.tar.bz2' -C 'C:\local'" || exit /b 1
)
set "BOOST_ROOT=%BOOST_INSTALL_DIR%"
if not exist "%BOOST_ROOT%\stage\lib" (
    echo [build_win32] Building Boost static libraries in "%BOOST_ROOT%"...
    pushd "%BOOST_ROOT%" || exit /b 1
    call bootstrap.bat || (popd & exit /b 1)
    .\b2.exe runtime-link=static address-model=64 --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-locale --with-regex || (popd & exit /b 1)
    popd
)
exit /b 0

:usage
echo NPLRuntimeUnified -- Windows Build ^& Run Script
echo.
echo Usage:
echo   build_win32.bat                              Build Windows Client ^(Release^) -^> build-win64\bin\ParaEngineClient.exe
echo   build_win32.bat [client] [Release^|Debug]     Build Windows Client ^(OpenGL/D3D11/D3D12/Vulkan/WebGPU^)
echo   build_win32.bat debug                        Shorthand for: build_win32.bat client Debug
echo   build_win32.bat server [Release^|Debug]       Build Headless Server -^> build-server\bin\ParaEngineServer.exe
echo   build_win32.bat web [build^|serve^|run^|all]    Build/serve Emscripten WebAssembly ^(via WSL + scripts\dev_web.ps1^)
echo   build_win32.bat android [gradle_task]        Build Android APK ^(NPLRuntime\Platform\AndroidStudio^)
echo   build_win32.bat picovr [gradle_task]         Build PICO VR OpenXR APK ^(NPLRuntime\Platform\PicoVR^)
echo   build_win32.bat run [renderer] [world_pid]   Launch Paracraft Client ^(auto^|d3d12^|vulkan^|d3d11^|opengl^|webgpu^)
echo   build_win32.bat run-server [script.lua]      Run ParaEngineServer ^(default: NPLRuntime\tests\helloworld.lua^)
echo   build_win32.bat test [Release^|Debug]         Run CTest verification suite in build-win64
echo   build_win32.bat clean [client^|server]        Clean CMake build artifacts
echo.
echo Flags ^(for client/server builds^):
echo   --configure, -c      Re-run CMake configuration before building
echo   --clean-first        Clean target before building ^(full rebuild^)
echo   --oce                Enable OpenCASCADE CAD plugin ^(-DNPLRUNTIME_OCE=ON^)
echo   --no-robotics        Disable MuJoCo + ONNX Runtime robotics ^(default ON for 64-bit client^)
echo   --install-deps       Auto-download CMake and Boost 1.85.0 if not found locally
echo.
echo Environment Variables:
echo   BOOST_ROOT           Path to Boost root ^(auto-checks C:\lxzsrc\externals\boost_1_90_0, C:\local\boost_1_85_0^)
echo   CMAKE_GENERATOR      Optional CMake generator override ^(e.g. "Visual Studio 17 2022"^)
echo   PARAWORLD_ROOT       Paracraft working directory for "run" ^(default: ..\ParaEngine\Paraworld^)
endlocal
exit /b 0
