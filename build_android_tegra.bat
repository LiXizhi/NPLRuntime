pushd .
rem Build android 
mkdir build\android_tegra\
cd build\android_tegra

call "cmake.exe" -G "Visual Studio 14 2015" -DCMAKE_SYSTEM_NAME=Android ../../NPLRuntime
popd
