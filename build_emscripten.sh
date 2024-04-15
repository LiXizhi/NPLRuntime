#!/bin/bash
# author: LiXizhi
# date: 2024.4
# desc: you need to install cmake, emsdk, boost first, please refer to emscripten section of https://github.com/LiXizhi/NPLRuntime/wiki/InstallGuide

## assume emscripten sdk is installed in home directory
EMSDK_DIR=$HOME/emsdk
## set cmake build directory 
BUILD_DIR=./build/emscripten_single_thread
#BUILD_DIR=./build/emscripten

## assume boost dir
BOOST_DIR=$HOME/boost_1_85_0
## the paracraft root folder (all assets like pkg, world, etc. are in this folder)
PARACRAFT_APP_DIR=$HOME/paracraft
## the output html folder for deploying web paracraft
OUTPUT_HTML_DIR=$HOME/webparacraft

if [ ! -d $OUTPUT_HTML_DIR ]; then
    mkdir -p $OUTPUT_HTML_DIR
fi
if [ ! -d $PARACRAFT_APP_DIR ]; then
    mkdir -p $PARACRAFT_APP_DIR
fi

## check cmake version is above 3.29. 
cmake_version=`cmake --version | head -n 1 | cut -d ' ' -f 3 | cut -d '.' -f 1,2`
if [ $(echo "$cmake_version >= 3.29" | bc) -eq '0' ]; then
    echo "cmake $cmake_version is too low, please download cmake 3.29 or above and add it to PATH"
    # export PATH=$HOME/cmake-3.29.1-linux-x86_64/bin:$PATH
    exit 1
fi

## set up emsdk
if [ ! -d ${EMSDK_DIR} ]; then
    echo "emsdk is not installed in $EMSDK_DIR, we will install it first"
    mkdir -p $EMSDK_DIR
    pushd $EMSDK_DIR
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    popd
fi

pushd ${EMSDK_DIR}
source ./emsdk_env.sh
popd

## check if $BOOST_DIR/stage/lib exist
if [ -d $BOOST_DIR ]; then
    if [ ! -d $BOOST_DIR/stage/lib ]; then
        # build boost with emscripten toolset
        ./bootstrap.sh
        ./b2 toolset=emscripten runtime-link=static
    fi
else
    echo "boost is not installed in $BOOST_DIR, please install it first"
fi

# wait for user to press Y to run cmake, n to skip to make
read -p "Press Y to run cmake, n to skip to make: (Y/n)" -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ ! -d ${BUILD_DIR} ]; then
        mkdir -p ${BUILD_DIR}
    fi

    # run cmake
    emcmake cmake \
    -S ./NPLRuntime \
    -B $BUILD_DIR \
    -DBUILD_TESTING=OFF \
    -DEMSCRIPTEN=ON \
    -DEMSCRIPTEN_SINGLE_THREAD=ON \
    -DCMAKE_BUILD_TYPE=RELEASE \
    -DBOOST_ROOT="$BOOST_DIR" \
    -DAPP_ROOT="$PARACRAFT_APP_DIR" \
    -DHTML_ROOT="$OUTPUT_HTML_DIR"
fi

# wait for user to enter a parallel build number default to 1
read -p "Enter the number of parallel workers to build (default 1): " -r
if [[ $REPLY =~ ^[0-9]+$ ]]; then
    parallel_workers=$REPLY
else
    parallel_workers=1
fi
# make
pushd ${BUILD_DIR}
    emmake make -j $parallel_workers
popd