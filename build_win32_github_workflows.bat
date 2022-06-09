

if EXIST "boost.7z" (
    set "PATH=%PATH%;C:\Program Files\7-Zip"
	7z x boost.7z -obin > nul
	move boost_* boost
	cd boost
	bootstrap.bat
	b2 address-model=32 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
    set BOOST_ROOT=%~dp0bin\boost
    echo %BOOST_ROOT%
)

if NOT "%GITHUB_WORKFLOW%" == "" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86
    mkdir build\win32
    cd build\win32
    cmake ..\..\Client
    cat D:/a/NPLRuntime/NPLRuntime/build/win32/CMakeFiles/CMakeError.log
    REM cat D:/a/NPLRuntime/NPLRuntime/build/win32/CMakeFiles/CMakeOutput.log
    REM msbuild .\CLIENT.sln /verbosity:minimal /property:Configuration=Release
)
