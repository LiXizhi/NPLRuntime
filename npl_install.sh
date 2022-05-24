#!/bin/bash
# author: lixizhi@yeah.net
# date: 2016.2.26
# desc: To install dependencies, please see `.travis.yml`
# you can run line by line manually, or just run this file

apt-get update
apt-get install -y build-essential cmake git-core
# please install libcurl-dev manually using openssh
#apt-get install -y libcurl-dev
#apt-get install -y pkg-config mono-devel
#apt-get install -y mysql-client libmysqlclient-dev
if [ "$CXX" = "g++" ]; then apt-get install -y g++-4.9; fi
if [ "$CXX" = "g++" ]; then export CXX="g++-4.9" CC="gcc-4.9"; fi
apt-get install -y libssl-dev libssh2-1-dev
apt-get install -y libreadline6 libreadline6-dev
apt-get install -y freeglut3 freeglut3-dev libglew1.5 libglew1.5-dev libglu1-mesa libglu1-mesa-dev libgl1-mesa-glx libgl1-mesa-dev libbz2-1.0 libbz2-dev


mkdir ./bin -p
pushd ./bin

if [ ! -d ./cmake-3.12.2 ]; then
	if [ ! -f ./cmake-3.12.2.tar.gz ]; then
		wget https://cmake.org/files/v3.12/cmake-3.12.2.tar.gz --no-check-certificate
	fi
	tar --zxvf -xf cmake-3.12.2.tar.gz
fi

cd cmake-3.12.2
./bootstrap
make
make install
cd ..

#if [ ! -d ./jpeg-9c ]; then
#	if [ ! -f ./jpegsrc.v9c.tar.gz ]; then
#		wget http://www.ijg.org/files/jpegsrc.v9c.tar.gz --no-check-certificate
#	fi
#	tar --zxvf -xf jpegsrc.v9c.tar.gz
#fi

#cd jpeg-9c
#./configure
#make
#make install
#cd ..

#if [ ! -d ./curl-7.61.1 ]; then
#	if [ ! -f ./curl-7.61.1.tar.gz ]; then
#		wget https://curl.haxx.se/download/curl-7.61.1.tar.gz --no-check-certificate
#	fi
#	tar --zxvf -xf curl-7.61.1.tar.gz
#fi

cd curl-7.61.1
./configure
make
make install
cd ..

# install boost from latest source url. boost version should be 1.55.0 or above
if [ ! -d ./boost_1_78_0 ]; then
	if [ ! -f ./boost_1_78_0.tar.bz2 ]; then
		wget http://dl.bintray.com/boostorg/release/1.78.0/source/boost_1_78_0.tar.bz2 --no-check-certificate
	fi
	tar --bzip2 -xf boost_1_78_0.tar.bz2
fi


cd boost_1_78_0
./bootstrap.sh --with-libraries="thread,date_time,filesystem,system,chrono,signals,serialization,iostreams,regex,log"
./b2 link=static threading=multi variant=release
./b2 install
popd

# Build with cmake
chmod +x ./build_linux.sh
sync
./build_linux.sh ${1:-1} ${2:-Release}
