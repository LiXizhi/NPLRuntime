#!/bin/bash

# install boost from latest source url. boost version should be 1.73.0 or above
mkdir ./bin -p
pushd ./bin
if [ ! -d ./boost_1_73_0 ]; then
	if [ ! -f ./boost_1_73_0.tar.bz2 ]; then
		wget https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.bz2 --no-check-certificate
	fi
	tar --bzip2 -xf boost_1_73_0.tar.bz2
fi
cd boost_1_73_0
./bootstrap.sh --with-libraries="thread,date_time,filesystem,system,chrono,serialization,iostreams,regex"
./b2 link=static threading=multi variant=release
./b2 install
popd