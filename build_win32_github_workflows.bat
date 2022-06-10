
REM git submodule init
REM git submodule update --init --recursive

if NOT "%GITHUB_WORKFLOW%" == "" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64

    if EXIST "boost_1_78_0.7z" (
        .\bin\7x.exe x boost_1_78_0.7z 
        cd boost_1_78_0
        bootstrap.bat
        b2 address-model=32 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        b2 address-model=64 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        cd ..
        set BOOST_ROOT=%cd%\boost_1_78_0
    )

    mkdir build\win32
    cd build\win32
    cmake ..\..\Client -DNPLRUNTIME_PHYSICS=OFF -DBOOST_ROOT=%BOOST_ROOT%
    cmake --build . -j 1
    REM msbuild .\CLIENT.sln /verbosity:minimal /property:Configuration=Release
) else (
    if EXIST "boost_1_78_0.7z" (
        .\bin\7z.exe x boost_1_78_0.7z 
        cd boost_1_78_0
        bootstrap.bat
        b2 address-model=32 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        b2 address-model=64 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        set BOOST_ROOT=%cd%\boost_1_78_0
        cd ..
    )

    call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    mkdir build\win32
    cd build\win32
    cmake ..\..\Client -DUSE_OPENGL_RENDERER:BOOL=TRUE
    msbuild .\CLIENT.sln /verbosity:minimal /property:Configuration=Release

)
