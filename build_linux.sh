#!/bin/bash
# author: lixizhi@yeah.net
# date: 2016.2.26
# desc: to install dependencies, please see `.travis.yml`
# For boost: Download boost 1.55 or above and build with `./b2 link=static`

# run cmake in ./NPLRuntime folder and make
pushd .
mkdir -p ./bin/linux
cd bin/linux/

# to build in parallel with 3 threads, use make -j3 or `./build_linux.sh 3`
if [ $# -gt 0 ]; then
   JOBS="${1:-1}"
fi
echo "parallel build with ${JOBS:-1} jobs, you can set JOBS=6 or ./build_linux.sh 6"

cmake -DCMAKE_BUILD_TYPE=${2:-Release} -DNPLRUNTIME_SERVER=ON ../../NPLRuntime/ && make --jobs=${JOBS:-1}
result=$?
popd

if [ $result == 0 ]; then
    echo "build success! Output file is at ./ParaWorld/bin64/"

    pushd ParaWorld/bin64/
    ls -l
    npl_exe_path=/usr/local/bin/npl
	npl_lua_path=/usr/lib/libluajit21.so
	npl_sql_path=/usr/lib/libParaSqlite.so
	
    echo "install executable to $npl_exe_path"
    if [ -f ./libluajit21.so ]; then
        if [ ! -e $npl_lua_path ] && [ ! -L $npl_lua_path ];  then
            ln -s $(pwd)/libluajit21.so $npl_lua_path
        else
            echo "lua already exist at $npl_lua_path"
        fi
        ls -l $npl_lua_path
    fi
	
	if [ -f ./libParaSqlite.so ]; then
        if [ ! -e $npl_sql_path ] && [ ! -L $npl_sql_path ];  then
            ln -s $(pwd)/libParaSqlite.so $npl_sql_path
        else
            echo "libParaSqlite already exist at $npl_sql_path"
        fi
        ls -l $npl_sql_path
    fi
	
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
