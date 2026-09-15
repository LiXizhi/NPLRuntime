# WASM 机器人原生依赖

此分支直接保存 MuJoCo 3.10.0 和 ONNX Runtime 1.27.1 的完整源码副本。

- 首次复制来源：NPLRuntime 桌面 pararobot 分支提交 `cb9dabb69` 中对应版本目录的 12,438 个 Git 跟踪文件。
- 原始第三方许可证、版本和构建文件保留在各版本目录内。
- WASM 构建只引用本 checkout 的 Server/trunk，不引用桌面 NPLRuntime 或目录链接。
- 版本目录作为源码提交；生成文件、静态库和 FetchContent 下载依赖放在外部构建目录。
- 使用 paracraft-64bit-cli 的 `build:web:wasm:robot:debug` 和 `test:web:robot`。工具链仍为 Emscripten 4.0.10，单线程 SIMD。
- CMake 的机器人依赖根路径检查拒绝其他 checkout；CLI 的 `-local` 构建目录隔离此前跨源码树的缓存。

世界及 Robot Lua 继续使用 HTTP loose 文件加载，不编入引擎 WASM。独立 C++ WASM 测试使用单独的测试 .data 文件，不会复制到站点的 auto_generate_files 目录。
