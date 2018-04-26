pushd .
mkdir -p ./build/ios_os/
cd build/ios_os
cmake ../../NPLRuntime -G "Xcode" -DCMAKE_TOOLCHAIN_FILE=../../NPLRuntime/cmake/ios.toolchain.cmake -DIOS_PLATFORM=OS -DENABLE_BITCODE=0 -DIOS_DEPLOYMENT_TARGET=10.0
popd
