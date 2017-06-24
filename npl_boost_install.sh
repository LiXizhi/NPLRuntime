#!/bin/bash

mkdir ./bin -p
pushd ./bin
wget http://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.bz2 --no-check-certificate
tar --bzip2 -xf boost_1_61_0.tar.bz2
cd boost_1_61_0
./bootstrap.sh --with-libraries="thread,date_time,filesystem,system,chrono,signals,serialization,iostreams,regex"
./b2 link=static threading=multi variant=release
./b2 install
