#!/bin/bash
# author: lixizhi@yeah.net
# date: 2016.2.26
# desc: for install dependencies, please see `.travis.xml` 

pushd .
mkdir -p ./bin/linux
cd bin/linux/
cmake ../../NPLRuntime/ && make -j3
result=$?
popd

echo output file is at ./ParaWorld/bin64/
ls ParaWorld/bin64/
exit $result