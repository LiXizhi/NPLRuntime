rem author: lixizhi@yeah.net
rem date: 2016.2.26
rem desc: to install dependencies, please see `.travis.xml`
rem guide: add `BOOST_ROOT` to environment variable, such as 'D:\lxzsrc\NPLRuntime\Server\trunk\boost_1_60_0', make sure to prebuilt your boost library with `./b2`
rem build: open `NPLRuntime/cmakelist.txt` with cmake-gui or run this bat file from visual c++ env command line.
  
pushd .
mkdir bin\win32
cd bin\win32
call "D:\Program Files (x86)\CMake\bin\cmake.exe" ../../NPLRuntime/
popd
