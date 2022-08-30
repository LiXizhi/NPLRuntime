
编译SDL跨平台Paracraft进入项目根目录

cmake -S NPLRuntime -B out -DLINUX=TRUE -DSDL_ROOT=xxxx

SDL_ROOT 值为SDL库安装目录, 下载SDL源码直接cmake编译安装即可, 参考根目录下的build_sdl2.sh脚本
