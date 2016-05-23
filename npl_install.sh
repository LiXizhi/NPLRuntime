#!/bin/bash
# author: lixizhi@yeah.net
# date: 2016.2.26
# desc: To install dependencies, please see `.travis.yml` 
# you can run line by line manually, or just run this file

apt-get update 
apt-get install  build-essential cmake git-core
#apt-get install  pkg-config mono-devel
#apt-get install  mysql-client libmysqlclient-dev
if [ "$CXX" = "g++" ]; then apt-get install  g++-4.9; fi
if [ "$CXX" = "g++" ]; then export CXX="g++-4.9" CC="gcc-4.9"; fi
apt-get install  libssl-dev libssh2-1-dev
apt-get install  freeglut3 freeglut3-dev libglew1.5 libglew1.5-dev libglu1-mesa libglu1-mesa-dev libgl1-mesa-glx libgl1-mesa-dev

# install boost from latest source url. boost version should be 1.55.0 or above
mkdir ./bin -p
pushd ./bin
wget http://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.bz2 --no-check-certificate
tar --bzip2 -xf boost_1_61_0.tar.bz2
cd boost_1_61_0
./bootstrap.sh --with-libraries="thread,date_time,filesystem,system,chrono,signals,serialization,iostreams,regex"
./b2 link=static threading=multi variant=release
./b2 install
popd

# Build with cmake 
chmod +x ./build_linux.sh 
./build_linux.sh
