#!/bin/bash
# author: lixizhi@yeah.net
# date: 2016.2.26
# desc: to install dependencies, please see `.travis.yml` 

# run cmake in ./NPLRuntime folder and make
pushd .
mkdir -p ./bin/linux
cd bin/linux/
# to build in parallel with 3 threads, use make -j3
cmake -DCMAKE_BUILD_TYPE=Release ../../NPLRuntime/ && make
result=$?
popd

if [ $result == 0 ]; then
    echo "output file is at ./ParaWorld/bin64/"
    
    pushd ParaWorld/bin64/
    # install to /usr/bin/npl
    ls -l
    npl_exe_path=/usr/bin/npl
    if [ -f ./ParaEngineServer ]; then
        if [ ! -e $npl_exe_path ] && [ ! -L $npl_exe_path ];  then
            ln -s $(pwd)/ParaEngineServer /usr/bin/npl
            echo "successfully installed npl runtime to $npl_exe_path"
        else
            echo "NPL runtime already exist at $npl_exe_path"
        fi
        ls -l $npl_exe_path
    fi
    popd
    
    # run all NPL tests 
    # npl NPLRuntime/tests/helloworld.lua
fi

exit $result