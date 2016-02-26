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

exit $result