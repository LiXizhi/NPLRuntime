rem author: lixizhi@yeah.net
rem date: 2016.2.26
rem updated: 2026.3.17 - auto-detect VS version (supports VS2022/VS2026), latest cmake/boost, auto-install DirectX SDK
rem desc: Run this file in developer command prompt x86 visual studio (search in desktop search box)
rem Install dependencies: (if DirectX9 SDK is not found, it will be auto-installed)
rem guide: optionally one can add `BOOST_ROOT` to environment variable, such as 'D:\lxzsrc\NPLRuntime\bin\boost_1_87_0', 
rem        make sure to prebuilt your boost library like below
rem You can choose to build two versions: Client or Server. Only client build requires DirectX SDK or opengl installed. When building Client, there are CMAKE options to specify whether to build NPL runtime as dll or executable, whether to use static linking in one big executable or using several dlls, etc. 
rem - To build server version: open `NPLRuntime/cmakelist.txt` with cmake-gui
rem - To build client version: open `Client/cmakelist.txt` with cmake-gui or simply run this bat file from visual c++ env command line.

pushd .

rem ============================================================
rem Install DirectX SDK (June 2010) if not found
rem ============================================================
if DEFINED DXSDK_DIR (
	echo DXSDK_DIR already defined: %DXSDK_DIR%
	goto :skip_dxsdk
)
if EXIST "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3d9.h" (
	set "DXSDK_DIR=C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\"
	goto :skip_dxsdk
)
echo DirectX SDK not found. Downloading and installing...
if NOT EXIST "bin" mkdir bin
pushd bin
if NOT EXIST "DXSDK_Jun10.exe" (
	echo Downloading DirectX SDK June 2010 - this is about 572MB...
	powershell -Command "Invoke-WebRequest -Uri 'https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe' -OutFile 'DXSDK_Jun10.exe'"
)
echo Installing DirectX SDK - this may take a few minutes...
echo NOTE: If installation fails with S1023 error, uninstall Visual C++ 2010 Redistributable first.
start /wait DXSDK_Jun10.exe /U
popd
rem Refresh environment variable
for /f "tokens=2*" %%a in ('reg query "HKLM\SOFTWARE\Microsoft\DirectX" /v SDKDir 2^>nul') do set "DXSDK_DIR=%%b"
:skip_dxsdk
echo DXSDK_DIR=%DXSDK_DIR%

rem ============================================================
rem Install cmake (latest version) - skip if cmake is in PATH
rem ============================================================
where cmake >nul 2>nul
if %ERRORLEVEL% EQU 0 (
	echo CMake already installed in PATH:
	cmake --version
	echo Skipping CMake installation.
	goto :skip_cmake
)
if EXIST "bin\cmake\bin\cmake.exe" (
	echo Using local CMake in bin\cmake
	goto :skip_cmake
)
if NOT EXIST "bin" mkdir bin
pushd bin
echo Downloading CMake 3.32.3...
powershell -Command "Invoke-WebRequest 'https://github.com/Kitware/CMake/releases/download/v3.32.3/cmake-3.32.3-windows-x86_64.zip' -OutFile 'cmake.zip'"
echo Extracting CMake...
powershell -Command "Expand-Archive -Path 'cmake.zip' -DestinationPath '.' -Force"
if EXIST cmake-3.32.3-windows-x86_64 move cmake-3.32.3-windows-x86_64 cmake
cmake\bin\cmake.exe --version
del cmake.zip
popd
:skip_cmake

rem ============================================================
rem Install Boost (latest version) - skip if BOOST_ROOT is set
rem ============================================================
if DEFINED BOOST_ROOT (
	echo BOOST_ROOT already defined: %BOOST_ROOT%
	echo Skipping Boost installation.
	goto :skip_boost
)
if EXIST "bin\boost\stage\lib" (
	echo Using local Boost in bin\boost
	set BOOST_ROOT=%~dp0bin\boost
	goto :skip_boost
)
if NOT EXIST "bin" mkdir bin
pushd bin
echo Downloading Boost 1.87.0...
powershell -Command "Invoke-WebRequest 'https://archives.boost.io/release/1.87.0/source/boost_1_87_0.zip' -OutFile 'boost.zip'"
echo Extracting Boost - this may take a while...
powershell -Command "Expand-Archive -Path 'boost.zip' -DestinationPath '.' -Force"
move boost_1_87_0 boost
del boost.zip
cd boost
echo Building Boost libraries...
rem Use vc143 toolset for compatibility (Boost 1.87 doesn't recognize vc145/VS2026 yet)
call bootstrap.bat vc143
b2 toolset=msvc-14.3 runtime-link=static address-model=32
popd
set BOOST_ROOT=%~dp0bin\boost
:skip_boost

rem ============================================================
rem Build main executable
rem ============================================================
if NOT EXIST "bin\win32" mkdir bin\win32
cd bin\win32

rem Auto-detect Visual Studio version using vswhere
set VS_GENERATOR="Visual Studio 17 2022"
for /f "tokens=*" %%v in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property catalog_productLineVersion 2^>nul') do (
	if "%%v"=="18" set VS_GENERATOR="Visual Studio 18 2026"
	if "%%v"=="17" set VS_GENERATOR="Visual Studio 17 2022"
)
echo Using CMake generator: %VS_GENERATOR%

rem Use system cmake if available, then local cmake, then VS-bundled cmake
set CMAKE_CMD=
where cmake >nul 2>nul
if %ERRORLEVEL% EQU 0 (
	set CMAKE_CMD=cmake
) else if EXIST "..\cmake\bin\cmake.exe" (
	set CMAKE_CMD=..\cmake\bin\cmake.exe
) else (
	rem Fall back to VS-bundled cmake
	for /f "tokens=*" %%p in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do (
		if EXIST "%%p\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
			set CMAKE_CMD=%%p\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
		)
	)
)
if "%CMAKE_CMD%"=="" (
	echo ERROR: CMake not found. Install CMake or use Visual Studio with CMake component.
	goto :eof
)
echo Using CMake: %CMAKE_CMD%
call "%CMAKE_CMD%" -G %VS_GENERATOR% -A Win32 -T v143 ../../Client/
msbuild %~dp0bin\win32\CLIENT.sln /p:Configuration=Release /p:Platform=Win32 /m /v:minimal

popd

rem ============================================================
rem Create symlinks and copy files
rem ============================================================
if EXIST "ParaWorld\bin32\" (
	pushd ParaWorld\bin32\
	if NOT EXIST "npl.exe" (
		mklink npl.exe ParaEngineClient.exe 2>nul || copy ParaEngineClient.exe npl.exe
	)
	if NOT EXIST "nplc.bat" copy ..\..\npl_packages\main\script\ide\System\nplcmd\nplc.bat nplc.bat
	dir
	popd
) else (
	echo ParaWorld\bin32\ not found, skipping symlink creation.
)

echo Build complete.




