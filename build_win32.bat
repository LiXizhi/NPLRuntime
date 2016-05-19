rem author: lixizhi@yeah.net
rem date: 2016.2.26
rem desc: to install dependencies, please see `.travis.xml`
rem guide: add `BOOST_ROOT` to environment variable, such as 'D:\lxzsrc\NPLRuntime\Server\trunk\boost_1_60_0', 
rem        make sure to prebuilt your boost library with `./b2` or `b2 --build-type=complete`
rem build: open server version: `NPLRuntime/cmakelist.txt` with cmake-gui or run this bat file from visual c++ env command line.
rem build: open client version: `Client/cmakelist.txt` with cmake-gui (needs to install DirectX9 SDK in default location)
  
pushd .
mkdir bin\win32
cd bin\win32
call "D:\Program Files (x86)\CMake\bin\cmake.exe" ../../NPLRuntime/
popd
