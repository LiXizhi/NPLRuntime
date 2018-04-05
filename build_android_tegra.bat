pushd .
rem Build android 
mkdir bin\android_tegra\
cd bin\android_tegra

call "cmake.exe" -G "Visual Studio 15 2017" -DCMAKE_SYSTEM_NAME=Android -DNPLRUNTIME_PROJECT_NAME=NPLRuntime_android ../../NPLRuntime
popd
