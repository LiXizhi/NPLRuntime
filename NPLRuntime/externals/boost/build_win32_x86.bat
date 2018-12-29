@echo off
@title Building Boost for Win32_x86
rem check boost
IF NOT EXIST prebuild/win32/src/boost_1_64_0 (  
    echo Not found boost at prebuild/win32/src/boost_1_64_0
	goto :EOF
)

rem check vscmd
if "%WindowsSdkDir%" == "" (
    echo Start this batch script from Visual Studio Command Prompt x86
    goto :EOF
)
pushd .
cd prebuild/win32/src/boost_1_64_0
rem clean
echo Cleaning everything
IF EXIST build_win32/build/x86 (
	rmdir /s/q .\build_win32\build\x86
)
IF EXIST build_win32/stage/x86 (
	rmdir /s/q .\build_win32\stage\x86
)
rem build
echo Building for win32
call bootstrap.bat
b2 stage link=static runtime-link=static threading=multi cflags=-D_WIN32_WINNT=0x0501 --abbreviate-paths --build-dir=build_win32/build/x86 --stagedir=build_win32/stage/x86 --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-regex --with-serialization --with-iostreams --with-log
popd
rem done
echo =====================
echo done


