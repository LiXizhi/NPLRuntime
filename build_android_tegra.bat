@echo off
echo building NPLRuntime for android with clang toolchain
echo NDK_ROOT: %NDK_ROOT%
echo BOOST_ROOT (optional): %BOOST_ROOT%
echo also need cmake, Python for windows, visual studio and Nsight tegra 3.5

rem prepare clang toolchain and build boost
pushd NPLRuntime\externals\boost\
call build_android.bat
popd

pushd .
echo generate visual studio solution with cmake  
if not exist build\android_tegra\ ( mkdir build\android_tegra\ )
cd build\android_tegra
call "cmake.exe" -G "Visual Studio 15 2017" -T clang-3.8 -DCMAKE_SYSTEM_NAME=Android -DNPLRUNTIME_PROJECT_NAME=NPLRuntimeAndroid ../../NPLRuntime
popd
