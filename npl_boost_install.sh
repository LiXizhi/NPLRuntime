#!/bin/bash

# install boost from latest source url. boost version should be 1.55.0 or above

pushd ./NPLRuntime/externals/boost
python ./build_boost.py --platform linux
popd