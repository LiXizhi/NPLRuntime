
if NOT "%GITHUB_WORKFLOW%" == "" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86
    mkdir build\win32
    cd build\win32
    cmake ..\..\Client
    msbuild .\CLIENT.sln /verbosity:minimal /property:Configuration=Release
)
