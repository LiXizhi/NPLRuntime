# Install NPL Runtime

You can redistribute NPLRuntime side-by-side with your scripts and resource files on windows/linux/iOS, etc.
However, this section is only about installing NPL runtime to your development computer and setting up a recommended coding environment for NPL. 

## Install on Windows
### Install from Windows Installer 
It is very easy to install on windows. The win32 executable of NPL runtime is currently released in two forms, choose one that is most suitable to you:
- Download windows 32bits/64bits installer exe, which contains the current master branch version. [Click to download](https://github.com/LiXizhi/NPLRuntime/releases). It will add `npl` to your path. 
- Download ParacraftSDK which is recommended for paracraft mod dev. See below.

[Paracraft](http://www.paracraft.cn) is an IDE written completely in NPL. 
Follow the steps:
* Download [ParacraftSDK](https://github.com/LiXizhi/ParaCraftSDK/archive/master.zip)
  * or you can clone it with following commands (you need [git](https://git-scm.com/) installed): 
```bash
# clone repository & sub modules
git clone https://github.com/LiXizhi/ParaCraftSDK.git
cd ParaCraftSDK
git submodule init
git submodule update
# Go into the repository
cd ParaCraftSDK
# install paracraft
redist\paracraft.exe
# install NPLRuntime
NPLRuntime\install.bat
```
 * `redist\paracraft.exe` will install the most recent version of `Paracraft` to  `./redist` folder
   * if you failed to install, please manually download from [Paracraft](http://www.paracraft.cn) and copy all files to `redist` folder.
 * Inside `./NPLRuntime/install.bat`
   * The NPL Runtime will be copied from `./redist` folder to `./NPLRuntime/win/bin`
   * The above path will be added to your `%path%` variable, so that you can run npl script from any folder. 

#### Update NPL Runtime to latest version
- If you use NPLRuntime installer, simply [download and reinstall it here](https://bit.ly/3IZv2BM)
- If you use paracraftSDK, we regularly update `paracraft` (weekly) on windows platform, you can update to latest version of NPL Runtime following following steps. 
  - run `redist\paracraft.exe` to update paracraft's NPL runtime in `redist` folder
  - run `NPLRuntime\install.bat` to copy NPL runtime executable from `redist` to `NPLRuntime/win/bin` folder
- Compiling from source code is always the latest version (recommended for serious developers), see next section.

#### On 64bits version
To build 64bits version of NPLRuntime, you need to build from source code (see next section), or download prebuild version from [http://www.paracraft.cn](http://www.paracraft.cn). 

Our current build target for public users is still 32bits on windows platform. 

> Please note that all compiled and non-compiled NPL/lua code can be loaded by both 32bits/64bits version of NPL runtime across all supported platforms (windows/linux/mac/android/ios) etc. However, if you use any C++ plugins (such as mysql, mono, etc), they must be built for each platform and CPU architecture. 

### Install on Windows From Source
```
git clone https://github.com/LiXizhi/NPLRuntime.git
cd NPLRuntime && git submodule update --init --recursive
build_win32.bat
```

> see [build_win32.bat](https://github.com/LiXizhi/NPLRuntime/blob/master/build_win32.bat) and [appveyor.yml](https://github.com/LiXizhi/NPLRuntime/blob/master/appveyor.yml) or our [build output](https://ci.appveyor.com/project/DarrenGZY/nplruntime-e8wud) for details

#### More Build Options Under Windows
* You can download and build [boost](http://www.boost.org/) from source code. Version `1.55.0`, `1.60.0`, `1.64.0` are tested, but the latest version should be fine.
Suppose you have unzipped the boost to `D:\boost`, you can build with following command.
```
b2 runtime-link=static --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
```
or just 
```
b2 runtime-link=static
```
and then add a system environment variable called `BOOST_ROOT`, with value `D:\boost`(or where your boost is located), so that our build-tool will find boost for you.
* Download [NPLRuntime source code](https://github.com/LiXizhi/NPLRuntime), or clone it by running:
```
git clone https://github.com/LiXizhi/NPLRuntime.git
```
* Download [cmake](https://cmake.org/), and build either `server` or `client` version. They share the same code base but with different `cmakelist.txt`; `client` version has everything `server` has plus graphics API. 
   * `Server` version: run `NPLRuntime/NPLRuntime/CMakeList.txt`  in `NPLRuntime/bin/win32` folder (any folder is fine, but do not use the `cmakelist.txt` source folder)
   * `Client` version: run `NPLRuntime/Client/CMakeList.txt`  in `NPLRuntime/bin/Client` folder (any folder is fine, but do not use the `cmakelist.txt` source folder), please refer to [build_win32.bat](https://github.com/LiXizhi/NPLRuntime/blob/master/build_win32.bat) for details
       * **REQUIRED:** Install latest DirectX9 SDK in default location [here](https://github.com/LiXizhi/NPLRuntime/blob/master/NPLRuntime/ParaEngineClient/cmake/DirectX.cmake). 
       * There is also a cmake option called `ParaEngineClient_DLL` which will build NPLRuntime as shared library, instead of executable. 
* If successful, you will have visual studio solution files generated. Open it with visual studio and build.
* The final executable is in `./ParaWorld/bin/` directory.  

### Debugging NPLRuntime with visual studio 
You need to install and update Paracraft to the latest version.  After you have successfully build NPLRuntime. Open the project property page of ParaEngineClient in visual studio, and set the debugging's working directory to the paracraft redist folder. You can also specify some custom command line parameters there.

#### Build options for Windows XP
Windows XP is no longer supported officially. But you are still able to build with old visual studio 2017. First one needs to install windows XP support in visual studio installer, then we need to turn on `NPLRUNTIME_WINXP` and `NPLRUNTIME_STATIC_LIB`, and turn off `NPLRUNTIME_FREEIMAGE` and `NPLRUNTIME_CAUDIOENGINE` in cmake. After visual studio solution files are generated, we need to manually select all projects and change platform toolset to WindowsXP (v141_xp). Then we are able to generate an executables for windows XP. Known issues are that caudioengine.dll needs to be removed and libcurl does not support https website under windows XP. 

To force boost to build 32bits lib for vs 2017, one may need to run
```
b2 toolset=msvc-14.1 address-model=32 runtime-link=static
```
Please note, you can build with higher version of visual studio (such as 2022) with toolset v141_xp selected. You need to first install that toolset with visual studio installer and manually rename boost link library to match v141 version in embed-resource and paraengineclient's solution property page. 

#### How to Debug NPLRuntime C++ source code 
When you build NPLRuntime from source code, all binary file(dlls) are automatically copied to `./ParaWorld/bin/` directory. All debug version files have `_d` appended to the end of the dll or exe files, such as `sqlite_d.dll`, etc. 

To debug your application, you need to start your application using the executable in `./ParaWorld/bin/`, such as `paraengineclient_d.exe`. You also need to specify a working directory where your application resides, such as `./redist` folder in `ParacraftSDK`. Alternatively, one can also copy all dependent `*_d.dll` files from `./ParaWorld/bin/` to your application's working directory.

Applications written in NPL are usually in pure NPL scripts. So you can download and install any NPL powered application (such as [this one](http://www.paracraft.cn) ), and set your executable's working directory to it before debugging from source code. `ParacraftSDK` also contains a good collection of example NPL programs. Please see the video at [[TutorialWebSite]] for install guide.
 
## Install on Linux (No Graphics)

> Click here to download precompiled [NPL Runtime linux 64bits release](https://github.com/LiXizhi/NPLRuntime/releases).

Under linux, it is highly recommended to build NPL from source code with latest packages and build tools on your dev/deployment machine. 
* Download [NPLRuntime source code](https://github.com/LiXizhi/NPLRuntime), or clone it by running:
```
git clone https://github.com/LiXizhi/NPLRuntime.git
cd NPLRuntime && git submodule init && git submodule update
```
* Install third-party packages and latest build tools. 
  * see [.travis.yml](https://github.com/LiXizhi/NPLRuntime/blob/master/.travis.yml) or our [build output](https://travis-ci.org/LiXizhi/NPLRuntime) for reference (on debian/ubuntu). In most cases, you need `gcc 4.9 or above, cmake, bzip2, boost, libcurl`. Optionally, you can install `mysql, mono, postgresql` if want to build NPL plugins for them. 
* Run [./build_linux.sh](https://github.com/LiXizhi/NPLRuntime/blob/master/build_linux.sh) from the root directory. Be patient to wait for build to complete. 
```
./build_linux.sh
```
or to enable parallel build of 6 concurrent make jobs, do following
```
./build_linux.sh  6
```
> Please note, you need to have at least 2GB memory to build with gcc, or you need to step up [swap memory](http://www.cyberciti.biz/faq/linux-add-a-swap-file-howto/) 
  
* The final executable is in `./ParaWorld/bin64/` directory. A symbolic link to the executable is created in `/usr/local/bin/npl`, so that you can run NPL script from anywhere.

## Install on Linux with Graphics
Make sure you have prepared linux packages in the previous step.
One needs to install [SDL2](https://www.libsdl.org/) and then run cmake like above. Currently, this is tested on ubuntu20@WSL2 on windows 11.

```
# compile and install SDL
source build_sdl2.sh
# build and install Boost
./npl_boost_install.sh
# config and build NPLRuntime
cmake -S NPLRuntime -B out -DLINUX=TRUE -DSDL_ROOT=${SDL_ROOT}
# NPLRuntime output file is in ./paraworld/bin64/ParaCraftSDL2
cmake --build out
# run bin64/ParaCraftSDL2 with working directory set to a paracraft root folder (with config/, database/, *.pkg, paraengine.sig)
```


## Build with Emscripten (WebASM & Web GL)
First you need a linux environment such as WSL ubuntu, and then download emscripten and build boost, such as version 1.85.
see the auto build script in `./build_emscripten.sh` in the root folder or follow the instructions below.

### build emscripten env
#### prepare emsdk 
```
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest && source ./emsdk_env.sh
```
#### prepare boost
download boost 1.85 or above and build with:
```
cd ~/
7za x boost_xxx.7z
cd boost_xxx
./bootstrap.sh
./b2 -a toolset=emscripten runtime-link=static cxxflags='-fexceptions' cflags='-fexceptions' linkflags='-fexceptions' --with-thread --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-locale -j4
```
the output file is under stage/lib folder.

#### compile boost with cmake
Download [boost-1.85.0-cmake.zip](https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.zip) 
1. CMakeLists.txt 添加  `set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdeclspec -sNO_DISABLE_EXCEPTION_CATCHING -pthread")`
2. emcmake cmake -S . -B build/emscripten -DCMAKE_BUILD_TYPE=Release -DBOOST_INCLUDE_LIBRARIES="filesystem;chrono;date_time;serialization;system"
3. cd build/emscripten; emmake make
4. cp stage/lib/*.a ../../stage/lib

#### install cmake
better use the latest version of cmake like above 3.29. 
* Download [cmake](https://cmake.org/)
Modify and use following command to add to PATH.
```
export PATH=/home/lixizhi/cmake-3.29.1-linux-x86_64/bin:$PATH
```

#### build NPLRuntime 
make sure you are on the cp_old branch, instead of the default dev branch. 
```
git clone git@github.com:LiXizhi/NPLRuntime.git
cd NPLRuntime
git checkout cp_old
emcmake cmake \
  -S ./NPLRuntime \
  -B ./build/emscripten_single_thread \
  -DBUILD_TESTING=OFF \
  -DEMSCRIPTEN=ON \
  -DEMSCRIPTEN_SINGLE_THREAD=ON \
  -DCMAKE_BUILD_TYPE=RELEASE \
  -DBOOST_ROOT="/home/lixizhi/boost_1_85_0" \
  -DAPP_ROOT="/home/lixizhi/paracraft" \
  -DHTML_ROOT="/home/lixizhi/webparacraft"

# goto the build directory like in above cmake command
cd build/emscripten_single_thread

# now build with 8 parallel workers. if your memory is limited, use a smaller number. 
emmake make -j 8   
```
- DBOOST_ROOT should be your boost root folder 
- DAPP_ROOT is paracraft app root folder. 
- remove DEMSCRIPTEN_SINGLE_THREAD if you want to build multithreaded version
- the output files are at `build\emscripten\bin\ParaCraft.html, js, wasm, worker.js`, which are static web files.

#### ParaCraft Deploy(using Nginx as example)
1. Config Nignx, create a new dev site like emscripten.keepwork.com, none-localhost website needs a https protocol.
2. specify a web root like: root /root;
3. 将根目录配置成静态文件访问目录, 并添加特定响应头, 示例代码如下:

```
location / { 
    # 下两行所有环境均需要
    add_header 'Cross-Origin-Embedder-Policy' 'require-corp';
    add_header 'Cross-Origin-Opener-Policy' 'same-origin';
    # 下面三行部署测试需要, 避免浏览器缓存文件
    add_header 'Cache-Control' 'no-cache';
    add_header 'Pragma' 'no-cache';
    add_header 'Expires' '0';
    # 通过url访问静态文件配置
    try_files $uri $uri/ =404;
}   
```

#### CI 自动部署触发条件
1. script 脚本更新  main150727.pkg => ${APP_ROOT}/main150727.pkg
2. buildinmod 内嵌模块更新 paracraftbuildinomod.zip => ${APP_ROOT}/npl_packages/ParacraftBuildiMod.zip   注意文件路径大小
3. NPLRuntime emscripten 分支更新
上述代码发生跟改自动触发编译部署(开发环境)
**资源更新可能也需要触发**


## Install on MAC OSX
We can build either the server or client version on OSX. 

For server version:
```
./npl_boost_install.sh
./build_mac_server.sh 6
```

For client version:
```
./build_mac_client.sh
```
It will generate an XCode project from cmakefiles under `./build/mac` folder.
Please put all your asset files under `NPLRuntime/Platform/OSX/assets` folder. 
Then one can build it from XCode.

### MAC App Sandbox API
In order to submit to App Store, one must enable app sandbox. To do so, one must manually set application settings in `Capabilities` tab and enable App Sandbox: 
- Network: incoming and outgoing connections both client and server
- Bluetooth
- User selected file: readonly.  

App sandbox API will not take effect if app is not code-signed, so one must also enable automatic code signing in the settings page. However, one must also add `--deep` flag to `Other Code Signing Flags` in build settings to enable signing all libraries as well.

## Install using docker   
Docker is the world’s leading software container platform. We can use Docker to run and manage apps side-by-side in isolated containers to get better compute density. This is an example in ubuntu 16.04.
* Install docker
```
#sudo apt-get update
#sudo apt-get install docker.io
```
* Search NPLRuntime images and pull it to localhost from docker hub
```
#docker search npl-runtime
#docker pull xuntian/ubuntu-npl-runtime
```
* Run NPLRuntime images
```
#docker run -dti xuntian/ubuntu-npl-runtime bash
```

## Build for Android
> Switch to `cp_old` branch to build for android. 

[Paracraft For Android中文版指南](InstallGuideFor_Android_Chinese)

We will build android apk with visual studio and Nvidia Nsight for visual studio. Nsight allows one to easily debug real device in visual studio. CMake natively supports generating target for visual studio + Nsight. But before that, we need to cross-compile some libraries like boost. We recommend you using boost 1.69 or above and cmake 3.11 or above. 

First one needs to build boost for android.
```
NPLRuntime\externals\boost\build_android.bat
```

Second, run CMake and select Android and clang++ for the toolchain. see following for details:
```
build_android_tegra.bat
```

Open visual studio, and build and run.

For Nsight, you need to configure NDK, android SDK, ant, JDK in visual studio. 
```
E:\Tools\android-ndk-r9d
C:\Program Files (x86)\Android\android-sdk
E:\Tools\apache-ant-1.9.4
C:\ProgramData\Chocolatey\lib\gradle\tools\gradle-4.6
D:\Program Files (x86)\Java\jdk1.8.0_11
d:\Program Files (x86)\Xoreax\IncrediBuild
```

Put all your android asset files to `NPLRuntime/Platform/Android/assets` folder. 
For an example of how an asset folder looks like, please see paracraft build file [here](https://github.com/tatfook/ParacraftAssets), please switch to Android, iOS, win32 branch as needed.

Next run CMake and specify `Android` as cross-platform toolchain name. This will generate a visual studio solution. And you are ready to build either the release or debug version. 

> Note: Nsight 3.5 + visual studio 2017

## Build for iOS

[Paracraft For iOS中文版指南](InstallGuideFor_iOS_chinese)

We can build for a simulator or device. A device build is recommended. First you need to build boost for all iOS architectures by running `./NPLRuntime/externals/boost/build_ios_and_osx.sh`
The shell script will automatically download boost and compile it to `./NPLRuntime/externals/boost/prebuild` 
folder

Then we need to run `./build_ios_os.sh` to generate xcode project file from cmake files. The generated XCode project file is located in `./build/ios/ios_os`

Like Android, one needs to put all your asset files to `NPLRuntime/Platform/iOS/assets` folder. Please note, do not use symbolic links in the assets folder. Then build and run your app, it may take a while to build. 

For an example of how an asset folder looks like, please see paracraft build file [here](https://github.com/tatfook/ParacraftAssets), please switch to android, iOS, win32 branch as needed.

## Editors and Debuggers
You can use your preferred editors that support lua syntax highlighting. 
NPL Runtime has a built-in web server called [[NPL Code Wiki|NPLCodeWiki]], it allows you to edit and debug via HTTP from any web browser on any platform.
* [[NPL Code Wiki|NPLCodeWiki]]: build-in debugger and editor.

We have developed npl-debugger-plugins for following editors:
* [Visual Studio Code](https://code.visualstudio.com/): a very good free/cross-platform editor
* [Visual Studio Community Edition](https://www.visualstudio.com/): the free version of `visual studio` (windows only) This is the one I used most. 


We have developed several useful open-source NPL language plugins for visual studio products:
In visual studio, select `menu::Tools::Extensions`, and search online for `npl` to install following plugins:
* [NPL Language service for vs code](https://github.com/tatfook/vscode-npl-debug) 
* [NPL/Lua language service for visual studio](https://visualstudiogallery.msdn.microsoft.com/7782dc20-924a-4726-8656-d876cdbb3417): used by thousands of developers worldwide.
   * Download [Source code](https://github.com/LiXizhi/NPL)
* [NPL Debugger for visual studio](https://visualstudiogallery.msdn.microsoft.com/7ebe665c-4f1d-41fd-91e1-52176cf2d9db): set break points and attach to running NPL process.
   * Download [Source code](https://github.com/LiXizhi/NPL)
* [NPL/ParaEngine Tools](https://visualstudiogallery.msdn.microsoft.com/accd022b-ec37-4614-8c08-30e291d28bd5) for visual studio: misc tools

![NPL language service](https://i1.visualstudiogallery.msdn.s-msft.com/7782dc20-924a-4726-8656-d876cdbb3417/image/file/168690/1/npl_language_service.png)

![Editors](https://visualstudiogallery.msdn.microsoft.com/site/view/file/136918/1/peeditor2.png)

## Example Code
[ParacraftSDK](https://github.com/LiXizhi/ParaCraftSDK) contains many example projects written in NPL. 
* See [ParacraftSDK wiki](https://github.com/LiXizhi/ParaCraftSDK/wiki) site for more details.
* You may now get your hand dirty by this [tutorial](https://github.com/LiXizhi/ParaCraftSDK/wiki/TutorialSimplePlugin) or continue reading here.