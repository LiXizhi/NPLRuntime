rem author: lixizhi@yeah.net
rem date: 2016.2.26
rem desc: to install dependencies, please see `.travis.xml`

pushd .
mkdir bin\win32
cd bin\win32
call "D:\Program Files (x86)\CMake\bin\cmake.exe" ../../NPLRuntime/
popd
