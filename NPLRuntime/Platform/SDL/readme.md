
编译SDL跨平台Paracraft进入项目根目录

cmake -S NPLRuntime -B out -DLINUX=TRUE -DSDL_ROOT=xxxx

SDL_ROOT 值为SDL库安装目录, 下载SDL源码直接cmake编译安装即可, 参考根目录下的build_sdl2.sh脚本


### 编译部署
#### emsdk 环境搭建
./emsdk install latest
./emsdk activate latest && source ./emsdk_env.sh
<!-- https://emscripten.org/docs/getting_started/downloads.html -->

#### NPLRuntime 编译
```
1. 编译boost 下载boost.1.81.0源码, 解压并进入源码目录, 执行如下指令编译
	.\bootstrap.bat   
	\b2 toolset=emscripten runtime-link=static --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --locale
	进入生成的 stage/lib 目录, 对每个xxx.bc文件执行命令 emar -q xxx.a xxx.bc  将.bc转成.a文件
2. 进入NPLRuntime根目录: (git clone git@github.com:LiXizhi/NPLRuntime.git; cd NPLRuntime; git checkout emscripten)
	emcmake cmake -S NPLRuntime -B build\emscripten -DEMSCRIPTEN=ON -DCMAKE_BUILD_TYPE=RELEASE -DBOOST_ROOT="boost_directory" -DAPP_ROOT="app_diectory"
	cd build\emscripten
	emmake make

# 编译boost1.85.0
1. CMakeLists.txt 添加  `set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdeclspec -sNO_DISABLE_EXCEPTION_CATCHING -pthread")`
2. emcmake cmake -S . -B build/emscripten -DCMAKE_BUILD_TYPE=Release -DBOOST_INCLUDE_LIBRARIES="filesystem;chrono;date_time;serialization;system"
3. cd build/emscripten; emmake make
4. cp stage/lib/*.a ../../stage/lib
```
**boost_directory 为boost根目录**
**app_diectory 为应用程序根目录, 此目录需放置ParaCraft所需相关文件**
**boost 报[-Wc++11-narrowing]错误 为类型转换精度缩小不支持, 编辑相关报错文件, 进行强制类型转换修复**
**编译成功后在build\emscripten\bin\ParaCraft.html, js, wasm, worker.js等网页静态文件**

#### ParaCraft 部署(以Nginx服务为例)
1. 配置Nignx配置文件, 新建站点 emscripten.keepwork.com 非localhost站点使用https协议
2. 指定站点根目录, 如: root /root;
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
4. iframe 嵌套使用, 当使用多线程(SharedArrayBuffer)时, 网站需被设置为跨源隔离. 即
``` 
    add_header 'Cross-Origin-Embedder-Policy' 'require-corp';
    add_header 'Cross-Origin-Opener-Policy' 'same-origin';
    # 被嵌网站是否支持内嵌同站跨域
    add_header 'Cross-Origin-Resource-Policy' 'same-site';
```
``` 
    # iframe 表签使用方式
    <iframe width="1280" height="320" src="https://webparacraft.keepwork.com" allow="cross-origin-isolated"></iframe>
```
<!-- https://web.dev/coop-coep/?utm_source=devtools -->
<!-- https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Headers/Content-Security-Policy -->

#### CI 自动部署触发条件
1. script 脚本更新  main150727.pkg => ${APP_ROOT}/main150727.pkg
2. buildinmod 内嵌模块更新 paracraftbuildinomod.zip => ${APP_ROOT}/npl_packages/ParacraftBuildiMod.zip   注意文件路径大小
3. NPLRuntime emscripten 分支更新
上述代码发生跟改自动触发编译部署(开发环境)
**资源更新可能也需要触发**

## EMSCRIPTEN

1. 作品展示: webpacraft.keepwork.com
2. 工具介绍与安装  官网(https://emscripten.org/index.html)
    emscripten 是一套WebAssembly编译器工具链(emcc, em++, emcongfiure...)
    https://www.codercto.com/a/59841.html
3. 编译
    -- linux 常规编译
    emconfigure ./configure; emmake make;
    -- cmake 交叉编译
    emcamke cmake; emmake make;
    
    编译选项:
        -sFETCH -lidbfs.js -pthread 内置标准库
        -sUSE_SDL=2 -sUSE_ZLIB=1 -sUSE_LIBJPEG=1 -sUSE_LIBPNG 内置外部部(开源第三方预编译, 会动态下载) 
        -Og 编译优化 
        -sPTHREAD_POOL_SIZE=32 线程数指定(当程序启动线程大于指定值, 程序会出现死锁)
        --preload-file --embed-file 内置资源文件指定
4. 移值
    窗口系统 (SDL)
    canvas <=> SDL_Window

    渲染方式(opengles 2.0, 3.0)
    需要安装opengles规范使用相关API, 先绑定缓冲区再设置顶点属性 

    多线程
    重在编译选项 PTHREAD_POOL_SIZE 要设置, 程序不要启动多于此设置的线程

    网络HTTP请求(fetch)
    内置 https://github.dev/emscripten-core/emscripten/blob/main/system/include/emscripten/fetch.h
    C++ curl

    文件系统
    内存文件系统, idbfs文件系统(持久化)
    idbfs 类似liunx磁盘挂载使用  文件改名不可跨不同文件系统(linux可以)  冲刷到磁盘需要自行调用FS.sync(false) 内存同步到磁盘 FS.sync(true) 从磁盘同步到内存
    预加载文件 --preload-file --embed-file 

    网络TCP(websocket)
    内置websocket: https://github.dev/emscripten-core/emscripten/blob/main/system/include/emscripten/websocket.h
    websockify  9110 127.0.0.1:8818  -- tcp代理
5. 可执行文件
    .html 浏览器执行
    .js   nodejs执行, 也自行定制html(加载js)用于浏览器执行
    浏览器执行需要 canvas 标签, js 文件模式使用 Module 对象, 需要将 canvas 标签放置 Moudel.canvas 上
6. 部署
    生成的所有静态文件放置web服务器的静态文件目录即可. 多线程程序添加如下响应头:
    add_header 'Cross-Origin-Embedder-Policy' 'require-corp';
    add_header 'Cross-Origin-Opener-Policy' 'same-origin';

    静态文件缓存策略: 常规前端项目文件缓存策略.