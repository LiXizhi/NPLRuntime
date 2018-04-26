@echo off
@title Building Boost for Win32
rem check boost
IF NOT EXIST prebuild/win32/src/boost_1_64_0 (  
    echo Not found boost at prebuild/win32/src/boost_1_64_0
	goto :EOF
)

rem check vscmd
if "%WindowsSdkDir%" == "" (
    echo Start this batch script from Visual Studio Command Prompt
    goto :EOF
)
pushd .
cd prebuild/win32/src/boost_1_64_0
rem clean
echo Cleaning everything
rem build
echo Building for win32
call bootstrap.bat
b2 stage link=static runtime-link=static threading=multi cflags=-D_WIN32_WINNT=0x0501 --stagedir=build_win32/stage --with-libraries="thread date_time filesystem system chrono signals regex serialization iostreams log"
popd
rem done
echo =====================
echo done


