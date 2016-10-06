


pushd .
mkdir -p ./bin/mac_client
cd bin/mac_client/
# to build in parallel with 3 threads, use make -j3

cmake -DCMAKE_BUILD_TYPE=Release ../../Client/ && make --jobs=${JOBS:-1}
result=$?
popd
