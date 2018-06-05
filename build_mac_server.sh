#!/bin/bash
# author: lixizhi@yeah.net
# date: 2016.2.26
# desc: to install dependencies, please see `.travis.yml`
# For boost: Download boost 1.55 or above and build with `./b2 link=static`

# run cmake in ./NPLRuntime folder and make
pushd .
mkdir -p ./bin/mac
cd bin/mac/
# to build in parallel with 3 threads, use make -j3

cmake -DCMAKE_BUILD_TYPE=Release -DNPLRUNTIME_RENDERER=NULL -DMAC_SERVER=ON ../../NPLRuntime/ && make --jobs=${JOBS:-1}
result=$?
popd

if [ $result == 0 ]; then
    echo "build success! Output file is at ./ParaWorld/bin64/"

    pushd ParaWorld/bin64/
    ls -l
    npl_exe_path=/usr/local/bin/npl
    echo "install executable to $npl_exe_path"
    if [ -f ./ParaEngineServer ]; then
        if [ ! -e $npl_exe_path ] && [ ! -L $npl_exe_path ];  then
            ln -s $(pwd)/ParaEngineServer $npl_exe_path
        else
            echo "NPL runtime already exist at $npl_exe_path"
        fi
        ls -l $npl_exe_path
    fi
    popd

    # run all NPL tests
    echo "you can test npl runtime by typing: npl NPLRuntime/tests/helloworld.lua"
fi

exit $result
