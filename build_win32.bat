rem author: lixizhi@yeah.net
rem date: 2016.2.26
rem desc: Install dependencies: (currently only BOOST and DirectX9 SDK)
rem guide: add `BOOST_ROOT` to environment variable, such as 'D:\lxzsrc\NPLRuntime\Server\trunk\boost_1_60_0', 
rem        make sure to prebuilt your boost library with `b2 --build-type=complete`
rem You can choose to build two versions: Client or Server. Only client build requires DirectX SDK installed. 
rem To build server version: open `NPLRuntime/cmakelist.txt` with cmake-gui or run this bat file from visual c++ env command line.
rem To build client version: open `Client/cmakelist.txt` with cmake-gui (needs to install DirectX9 SDK in default location)
  
pushd .
mkdir bin\win32
cd bin\win32
call "D:\Program Files (x86)\CMake\bin\cmake.exe" ../../NPLRuntime/
popd
