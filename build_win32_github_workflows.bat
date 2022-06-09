
git submodule init

git submodule update --init --recursive

if NOT "%GITHUB_WORKFLOW%" == "" (
    if EXIST "boost_1_78_0.7z" (
        set "PATH=%PATH%;C:\Program Files\7-Zip"
        7x x boost_1_78_0.7z 
        cd boost_1_78_0
        bootstrap.bat
        b2 address-model=32 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        b2 address-model=64 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        set BOOST_ROOT=%cd%\boost_1_78_0
        cd ..
    )

    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
    mkdir build\win32
    cd build\win32
    cmake ..\..\Client -DUSE_OPENGL_RENDERER=true -DNPLRUNTIME_PHYSICS=OFF
    msbuild .\CLIENT.sln /verbosity:minimal /property:Configuration=Release
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
