#!/bin/bash
olddir=`pwd`
mkdir bin/linux
cd bin/linux/
cmake ../../NPLRuntime/
make
cd $olddir