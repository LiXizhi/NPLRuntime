#!/bin/bash
olddir=`pwd`
mkdir -p ./bin/linux
cd bin/linux/
cmake ../../NPLRuntime/ && make
result=$?

cd $olddir

exit $result