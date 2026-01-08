@echo off
rem author: lixizhi@yeah.net
rem date: 2016.2.26
rem desc: Run this file in developer command prompt x86 visual studio 2017 (search in desktop search box)
rem Install dependencies: (if DirectX9 SDK is not found, opengl is used)
rem guide: optionally one can add `BOOST_ROOT` to environment variable, such as 'D:\lxzsrc\NPLRuntime\Server\trunk\boost_1_85_0', 
rem        make sure to prebuilt your boost library like below
rem You can choose to build two versions: Client or Server. Only client build requires DirectX SDK or opengl installed. When building Client, there are CMAKE options to specify whether to build NPL runtime as dll or executable, whether to use static linking in one big executable or using several dlls, etc. 
rem - To build server version: open `NPLRuntime/cmakelist.txt` with cmake-gui
rem - To build client version: open `Client/cmakelist.txt` with cmake-gui or simply run this bat file from visual c++ env command line.

setlocal enabledelayedexpansion

rem Check for VsDevCmd if msbuild is not found
msbuild -version >nul 2>&1
if errorlevel 1 (
    echo MsBuild not found, searching for VsDevCmd.bat...
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
    )
)

msbuild -version >nul 2>&1
if errorlevel 1 (
    echo Error: MsBuild not found. Please run this script in a Visual Studio Developer Command Prompt.
    exit /b 1
)

pushd .

rem Check and install CMake if not available
echo.
echo Checking for CMake...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo CMake not found. Installing CMake...
    if not exist "%TEMP%\cmake-installer.msi" (
        echo Downloading CMake...
        powershell -NoProfile -ExecutionPolicy Bypass -Command "$client = New-Object System.Net.WebClient; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12; $client.DownloadFile('https://github.com/Kitware/CMake/releases/download/v3.28.0/cmake-3.28.0-windows-x86_64.msi', '%TEMP%\cmake-installer.msi')" 2>nul || (
            echo Warning: Download failed. Continuing with existing cmake or skipping...
        )
    )
    if exist "%TEMP%\cmake-installer.msi" (
        echo Installing CMake from %TEMP%\cmake-installer.msi...
        msiexec /i "%TEMP%\cmake-installer.msi" /quiet /qn /norestart
        echo CMake installation triggered. Waiting for completion...
        timeout /t 5
        
        rem Try to find cmake in default path
        if exist "C:\Program Files\CMake\bin" set PATH=C:\Program Files\CMake\bin;!PATH!

        echo Verifying CMake installation...
        cmake --version >nul 2>&1
        if errorlevel 1 (
            echo Warning: CMake still not in PATH. Please add CMake to PATH manually.
        )
    )
) else (
    for /f "tokens=3" %%i in ('cmake --version ^| findstr "version"') do set CMAKE_VERSION=%%i
    echo CMake found: !CMAKE_VERSION!
)

rem Check and install Boost if not available
echo.
echo Checking for Boost...
if defined BOOST_ROOT (
    if exist "!BOOST_ROOT!" (
        echo Boost found at !BOOST_ROOT!
    ) else (
        echo BOOST_ROOT set but path does not exist. Downloading Boost...
        call :install_boost
    )
) else (
    if exist "C:\local\boost_1_85_0" (
        set BOOST_ROOT=C:\local\boost_1_85_0
        echo Boost found at !BOOST_ROOT!
    ) else if exist "C:\Boost" (
        set BOOST_ROOT=C:\Boost
        echo Boost found at !BOOST_ROOT!
    ) else (
        echo Boost not found. Downloading and extracting Boost...
        call :install_boost
    )
)

rem Build boost if necessary (Check for library existence)
if exist "!BOOST_ROOT!\bootstrap.bat" (
    if not exist "!BOOST_ROOT!\stage\lib\libboost_system-vc143-mt-s-x64-1_85.lib" (
        echo Building Boost Libraries at !BOOST_ROOT!...
        pushd "!BOOST_ROOT!"
        call bootstrap.bat
        if errorlevel 1 echo Bootstrap failed! & popd & exit /b 1
        
        echo Cleaning previous boost build...
        if exist bin.v2 rd /s /q bin.v2
        
        echo Running b2 to build libraries...
        .\b2.exe runtime-link=static
        if errorlevel 1 echo Boost build failed! & popd & exit /b 1
        popd
    ) else (
        echo Boost libraries appear to be present in stage/lib.
    )
)

rem Build main executable
echo.
echo Creating build directory...
if not exist build\win32 mkdir build\win32
cd build\win32

rem Ensure cmake is in PATH
set PATH=C:\Program Files\CMake\bin;%PATH%

echo.
echo Running CMake with BOOST_ROOT set to: !BOOST_ROOT!
rem Force Generator to VS2022 x64
cmake -G "Visual Studio 17 2022" -A x64 -DBOOST_ROOT="!BOOST_ROOT!" -DNPLRUNTIME_RENDERER=OPENGL ../../NPLRuntime
if errorlevel 1 (
    echo CMake configuration failed!
    popd
    exit /b 1
)

echo.
echo Building project...
msbuild /p:Configuration=Release /p:Platform=x64 /m
if errorlevel 1 (
    echo Build failed!
    popd
    exit /b 1
)

echo.
echo Build completed successfully!
popd
endlocal
exit /b 0

:install_boost
rem Install Boost to C:\local\boost_1_85_0
set BOOST_INSTALL_DIR=C:\local\boost_1_85_0
if not exist "C:\local" mkdir C:\local

if not exist "%TEMP%\boost_1_85_0.tar.bz2" (
    echo Downloading Boost 1.85.0...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "$client = New-Object System.Net.WebClient; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12; $client.DownloadFile('https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0.tar.bz2', '%TEMP%\boost_1_85_0.tar.bz2')" 2>nul || (
        echo Download failed. Trying alternative URL...
        powershell -NoProfile -ExecutionPolicy Bypass -Command "$client = New-Object System.Net.WebClient; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12; $client.DownloadFile('https://sourceforge.net/projects/boost/files/boost/1.85.0/boost_1_85_0.tar.bz2', '%TEMP%\boost_1_85_0.tar.bz2')" 2>nul || (
            echo Failed to download Boost. Please install manually.
            exit /b 1
        )
    )
)

if exist "%TEMP%\boost_1_85_0.tar.bz2" (
    echo Extracting Boost...
    if exist "C:\local\boost_1_85_0" (
        echo Boost already extracted, skipping extraction.
    ) else (
        powershell -NoProfile -ExecutionPolicy Bypass -Command "cd 'C:\local'; tar -xf '%TEMP%\boost_1_85_0.tar.bz2'" 2>nul || (
            echo Extraction failed. Boost may need to be installed manually.
            exit /b 1
        )
    )
    set BOOST_ROOT=!BOOST_INSTALL_DIR!
    echo Boost set to !BOOST_ROOT!
) else (
    echo Failed to download Boost. Please install manually or check internet connection.
    exit /b 1
)
exit /b 0



