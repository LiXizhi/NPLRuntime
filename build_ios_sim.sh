pushd .
mkdir -p ./build/ios_sim/
cd build/ios_sim
cmake ../../NPLRuntime -G "Xcode" -DCMAKE_TOOLCHAIN_FILE=../../NPLRuntime/cmake/ios.toolchain.cmake -DIOS_PLATFORM=SIMULATOR -DENABLE_BITCODE=OFF
popd