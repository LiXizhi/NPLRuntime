pushd .
mkdir -p ./build/ios_sim/
cd build/ios_sim
cmake ../../NPLRuntime -DCMAKE_TOOLCHAIN_FILE=../../NPLRuntime/cmake/ios.toolchain.cmake -DIOS_PLATFORM=SIMULATOR
popd