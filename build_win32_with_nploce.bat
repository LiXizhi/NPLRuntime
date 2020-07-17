rem author: lixizhi@yeah.net
rem date: 2016.2.26
rem desc: Run this file in developer command prompt x86 visual studio 2017 (search in desktop search box)
rem Install dependencies: (if DirectX9 SDK is not found, opengl is used)
rem guide: optionally one can add `BOOST_ROOT` to environment variable, such as 'D:\lxzsrc\NPLRuntime\Server\trunk\boost_1_60_0', 
rem        make sure to prebuilt your boost library like below
rem You can choose to build two versions: Client or Server. Only client build requires DirectX SDK or opengl installed. When building Client, there are CMAKE options to specify whether to build NPL runtime as dll or executable, whether to use static linking in one big executable or using several dlls, etc. 
rem - To build server version: open `NPLRuntime/cmakelist.txt` with cmake-gui
rem - To build client version: open `Client/cmakelist.txt` with cmake-gui or simply run this bat file from visual c++ env command line.

pushd .

rem Build main executable

if not exist NPLRuntime/Plugins/NplOce/NplOce (
	echo "clone NplOce!!!!!"
	git submodule add https://github.com/zhangleio/NplOce.git NPLRuntime/Plugins/NplOce/NplOce
)

if not exist NPLRuntime/Plugins/NplOce/NplOce-Deps(
	echo "clone NplOce-Deps!!!!"
	git submodule add https://github.com/zhangleio/NplOce-Deps.git NPLRuntime/Plugins/NplOce/NplOce-Deps
)

git submodule init
git submodule update


mkdir build\win32
cd build\win32

call "cmake.exe" ../../NPLRuntime -DNPLRUNTIME_OCE=TRUE
popd



