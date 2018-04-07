pushd .
mkdir -p ./build/mac/
cd build/mac
cmake -G "Xcode" ../../NPLRuntime
popd
