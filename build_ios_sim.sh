pushd .
mkdir -p ./build/ios/
cd build/ios
cmake ../../NPLRuntime -G "Xcode" -DCMAKE_TOOLCHAIN_FILE=../../NPLRuntime/cmake/ios.toolchain.cmake -DIOS_PLATFORM=SIMULATOR -DENABLE_BITCODE=OFF -DIOS_DEPLOYMENT_TARGET=10.0
popd
