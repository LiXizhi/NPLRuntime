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
    ./b2 install
    ./b2 link=static threading=multi variant=debug stage
    ./b2 install
    cd - 
    ls -l boost/stage/lib
fi

sudo apt-get update
sudo apt-get install -y build-essential wget 
sudo apt-get install -y libssl-dev libssh2-1-dev libcurl4-openssl-dev
sudo apt-get install -y libreadline6 libreadline6-dev
sudo apt-get install -y freeglut3 freeglut3-dev libglew1.5 libglew1.5-dev libglu1-mesa libglu1-mesa-dev libgl1-mesa-glx libgl1-mesa-dev libbz2-1.0 libbz2-dev

CURRENT_DIRECTORY=`pwd`

mkdir -p ./bin/linux
cd bin/linux/

cmake -DCMAKE_BUILD_TYPE=Release ../../NPLRuntime/ -DBOOST_ROOT=${CURRENT_DIRECTORY}/boost -DGITHUB_WORKFLOW=TRUE -DUSE_NULL_RENDERER=TRUE -DLINUX=TRUE
if [ $? -ne 0 ]; then
    exit 1
fi
make --jobs=1
if [ $? -ne 0 ]; then
    exit 1
fi
cd -
