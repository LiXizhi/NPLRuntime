


if NOT "%GITHUB_WORKFLOW%" == "" (
    if EXIST "boost_1_78_0.7z" (
        set "PATH=%PATH%;C:\Program Files\7-Zip"
        REM powershell -Command "Invoke-WebRequest https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.7z -OutFile boost.7z"
        7z x boost_1_78_0.7z 
        cd boost_1_78_0
        bootstrap.bat
        b2 address-model=32 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        set BOOST_ROOT=%cd%\boost
        echo %BOOST_ROOT%
    )

    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86
    mkdir build\win32
    cd build\win32
    cmake ..\..\Client
    cat D:/a/NPLRuntime/NPLRuntime/build/win32/CMakeFiles/CMakeError.log
    REM cat D:/a/NPLRuntime/NPLRuntime/build/win32/CMakeFiles/CMakeOutput.log
    REM msbuild .\CLIENT.sln /verbosity:minimal /property:Configuration=Release
)
