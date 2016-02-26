#!/bin/bash
olddir=`pwd`
mkdir -p ./bin/linux
cd bin/linux/
cmake ../../NPLRuntime/ && make -j3
result=$?

cd $olddir

exit $result