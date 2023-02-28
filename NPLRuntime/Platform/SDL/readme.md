
编译SDL跨平台Paracraft进入项目根目录

cmake -S NPLRuntime -B out -DLINUX=TRUE -DSDL_ROOT=xxxx

SDL_ROOT 值为SDL库安装目录, 下载SDL源码直接cmake编译安装即可, 参考根目录下的build_sdl2.sh脚本


### 编译部署
#### emsdk 环境搭建
./emsdk install latest
./emsdk activate latest && source ./emsdk_env.sh

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

#### CI 自动部署触发条件
1. script 脚本更新  main150727.pkg => ${APP_ROOT}/main150727.pkg
2. buildinmod 内嵌模块更新 paracraftbuildinomod.zip => ${APP_ROOT}/npl_packages/ParacraftBuildiMod.zip   注意文件路径大小
3. NPLRuntime emscripten 分支更新
上述代码发生跟改自动触发编译部署(开发环境)
**资源更新可能也需要触发**