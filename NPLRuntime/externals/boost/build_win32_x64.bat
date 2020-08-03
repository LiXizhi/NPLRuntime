@echo off
@title Building Boost for Win32_x64
rem check boost
IF NOT EXIST prebuild/win32/src/boost_1_73_0 (  
    echo Not found boost at prebuild/win32/src/boost_1_73_0
	goto :EOF
)

rem check vscmd
if "%WindowsSdkDir%" == "" (
    echo Start this batch script from Visual Studio Command Prompt x64
    goto :EOF
)
pushd .
cd prebuild/win32/src/boost_1_73_0
rem clean
echo Cleaning everything
IF EXIST build_win32/build/x64 (
	rmdir /s/q .\build_win32\build\x64
)
IF EXIST build_win32/stage/x64 (
	rmdir /s/q .\build_win32\stage\x64
)

rem build
echo Building for win64
call bootstrap.bat
b2 stage link=static runtime-link=static threading=multi address-model=64 --abbreviate-paths cflags=-D_WIN32_WINNT=0x0600 --build-dir=build_win32/build/x64 --stagedir=build_win32/stage/x64 --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-regex --with-serialization --with-iostreams --with-log
popd
rem done
echo =====================
echo done


