pushd .
rem Build android 
mkdir build\android_tegra\
cd build\android_tegra

call "cmake.exe" -G "Visual Studio 15 2017" -T clang-3.8  -DCMAKE_SYSTEM_NAME=Android -DNPLRUNTIME_PROJECT_NAME=NPLRuntimeAndroid ../../NPLRuntime
popd
