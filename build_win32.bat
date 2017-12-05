rem author: lixizhi@yeah.net
rem date: 2016.2.26
rem desc: Run this file in developer command prompt x86 (search in desktop search box)
rem Install dependencies: (if DirectX9 SDK is not found, opengl is used)
rem guide: optionally one can add `BOOST_ROOT` to environment variable, such as 'D:\lxzsrc\NPLRuntime\Server\trunk\boost_1_60_0', 
rem        make sure to prebuilt your boost library like below
rem You can choose to build two versions: Client or Server. Only client build requires DirectX SDK installed. When building Client, there is a CMAKE option called ParaEngineClient_DLL to build NPL runtime as dll or executable. 
rem To build server version: open `NPLRuntime/cmakelist.txt` with cmake-gui or run this bat file from visual c++ env command line.
rem To build client version: open `Client/cmakelist.txt` with cmake-gui (needs to install DirectX9 SDK in default location and select ParaEngineClient_DLL cmake option)
  
pushd .

rem Install cmake
if NOT EXIST "bin\cmake" (
	pushd bin
	powershell -Command "Invoke-WebRequest https://cmake.org/files/v3.10/cmake-3.10.0-win64-x64.zip -OutFile cmake.zip"
	7z x cmake.zip -obin > nul
	move bin\cmake-* cmake
	cmake\bin\cmake.exe --version
	popd
)

rem Install Boost
if NOT EXIST "bin\boost" (
	pushd bin
	powershell -Command "Invoke-WebRequest http://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.7z -OutFile boost.7z"
	7z x boost.7z -obin > nul
	move bin\boost_* boost
	cd boost
	bootstrap.bat
	b2 address-model=32 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-signals --with-serialization --with-iostreams --with-regex stage
	popd
)
set BOOST_ROOT=%~dp0bin\boost

rem Build main executable
mkdir bin\client_win32
cd bin\client_win32

call "..\cmake\bin\cmake.exe" ../../Client/
msbuild  %~dp0\bin\client_win32\CLIENT.sln /verbosity:minimal /property:Configuration=Release

popd

dir ParaWorld/bin32



