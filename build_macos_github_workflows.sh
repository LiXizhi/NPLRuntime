#!/bin/bash


if [ ! -d boost ]; then
    if [ ! -f boost_1_78_0.tar.bz2 ]; then
        wget https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.bz2 --no-check-certificate  -O boost_1_78_0.tar.bz2
    fi
	tar --bzip2 -xf boost_1_78_0.tar.bz2
    mv boost_1_78_0 boost
    cd boost
    ./bootstrap.sh --with-libraries="thread,date_time,filesystem,system,chrono,serialization,iostreams,regex,log"
    ./b2 link=static threading=multi variant=release stage
    ./b2 link=static threading=multi variant=debug stage
    cd - 
    ls -l boost/stage/lib
fi

CURRENT_DIRECTORY=`pwd`

mkdir -p ./bin/macos
cd bin/macos/

cmake -G "Xcode" -DCMAKE_BUILD_TYPE=Release ../../NPLRuntime/ -DBOOST_ROOT=${CURRENT_DIRECTORY}/boost -DGITHUB_WORKFLOW=TRUE -DNPLRUNTIME_RENDERER=NULL -DMAC_SERVER=TRUE
if [ $? -ne 0 ]; then
    exit 1
fi
#make --jobs=1
cmake --build .
if [ $? -ne 0 ]; then
    exit 1
fi
cd -
