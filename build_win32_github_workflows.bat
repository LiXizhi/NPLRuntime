
REM git submodule init
REM git submodule update --init --recursive

if NOT "%GITHUB_WORKFLOW%" == "" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
    mkdir build\win32
    cd build\win32
    cmake ..\..\Client -DNPLRUNTIME_PHYSICS=OFF -DBOOST_ROOT="%BOOST_ROOT%"
    cmake --build . -j 1
    REM msbuild .\CLIENT.sln /verbosity:minimal /property:Configuration=Release
) else (
    call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    mkdir build\win32
    cd build\win32
    cmake ..\..\Client -DUSE_OPENGL_RENDERER:BOOL=TRUE -DBOOST_ROOT="%BOOST_ROOT%"
    msbuild .\CLIENT.sln /verbosity:minimal /property:Configuration=Release
)
