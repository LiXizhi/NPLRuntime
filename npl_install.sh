#!/bin/bash
# author: lixizhi@yeah.net
# date: 2016.2.26
# desc: To install dependencies, please see `.travis.yml`
# you can run line by line manually, or just run this file

apt-get update
apt-get install -y build-essential wget 
# please install libcurl-dev manually using openssh
# apt-get install -y libcurl-dev
# apt-get install -y pkg-config mono-devel
# apt-get install -y mysql-client libmysqlclient-dev
# if [ "$CXX" = "g++" ]; then apt-get install -y g++-4.9; fi
# if [ "$CXX" = "g++" ]; then export CXX="g++-4.9" CC="gcc-4.9"; fi
apt-get install -y libssl-dev libssh2-1-dev libcurl4-openssl-dev
apt-get install -y libreadline6 libreadline6-dev
apt-get install -y freeglut3 freeglut3-dev libglew1.5 libglew1.5-dev libglu1-mesa libglu1-mesa-dev libgl1-mesa-glx libgl1-mesa-dev libbz2-1.0 libbz2-dev

# install boost from latest source url. boost version should be 1.55.0 or above
mkdir ./bin -p
pushd ./bin
if [ ! -d ./boost_1_69_0 ]; then
	if [ ! -f ./boost_1_69_0.tar.bz2 ]; then
		wget http://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.bz2 --no-check-certificate
	fi
	tar --bzip2 -xf boost_1_69_0.tar.bz2
fi
cd boost_1_69_0
./bootstrap.sh --with-libraries="thread,date_time,filesystem,system,chrono,serialization,iostreams,regex"
./b2 link=static threading=multi variant=release
./b2 install
popd

# install cmake 3.15.7 form source
pushd ./bin
if [ ! -d ./cmake-3.15.7 ]; then
	if [ ! -f ./cmake-3.15.7.tar.gz ]; then
		wget https://cmake.org/files/v3.15/cmake-3.15.7.tar.gz
	fi
	tar -xzvf cmake-3.15.7.tar.gz
fi
cd cmake-3.15.7
./bootstrap
make
make install
popd

# Build with cmake
chmod +x ./build_linux.sh
./build_linux.sh ${1:-4} ${2:-Release}
