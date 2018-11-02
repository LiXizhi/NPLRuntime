pushd .
mkdir -p ./build/mac/
cd build/mac
cmake -G "Xcode" -DNPLRUNTIME_RENDERER=OPENGL -DMAC_SERVER=OFF ../../NPLRuntime
popd
