# 单线程 Web 机器人构建

`EMSCRIPTEN=ON` 且 `EMSCRIPTEN_SINGLE_THREAD=ON` 时，cp_old 自动启用 MuJoCo/ONNX。配置时会将旧的 `NPLRUNTIME_WEB_ROBOT=OFF` 缓存更新为 ON；单线程 Web 不再通过这个旧开关生成缺少机器人接口的客户端。原生和多线程构建不自动启用。

使用现有 `emcmake cmake` 配置及 `cmake --build` 命令即可。源码必须完整包含本分支 `Server/trunk/mujoco-3.10.0` 和 `Server/trunk/onnxruntime-1.27.1`，工具链需满足依赖的 CMake 要求（当前至少 3.28）。已验证工具链为 Emscripten 4.0.10。

未设置 `ONNXRUNTIME_WASM_LIBRARY` 时，主构建自动在自己的 `robot-onnx/` 目录中配置并构建 ONNX，使用相同的 Emscripten 工具链和 Debug/Release 配置、单线程 SIMD 及异常支持。依赖的首次下载沿用进程代理环境，构建失败会阻止引擎链接。默认并行度为 4，可通过 `NPLRUNTIME_WEB_ROBOT_BUILD_JOBS` 调整。

也可以继续传入 `-DONNXRUNTIME_WASM_LIBRARY=<预编译库>/libonnxruntime_webassembly.a`。该库必须由匹配的工具链和配置构建，路径不存在时配置直接失败。不要把另一份源码 checkout 的 MuJoCo/ONNX 源目录传给 `ROBOT_DEPENDENCY_ROOT`。

发布配套的 JS/WASM 与最新 Lua 脚本包，并更新站点摘要。Release 优化本身不等于机器人能力完整；启动检查至少确认 `ParaMuJoCo.LoadModel` 和 `ParaONNXPolicy.CreateModel` 都是 function。

默认值及旧缓存合同检查：

```text
cmake -P NPLRuntime/tests/WebRobotDefaults.cmake
```

配置 `ROBOT_TEST_ASSET_ROOT` 为 MicroDuck walking 资产目录后，还可构建 `ParaRobotWasmSmoke`。进入构建目录下的 `robot-tests`，执行 `node ParaRobotWasmSmoke.js`，确保预加载器能够读取同目录的 `.data`，验证 MuJoCo 模型、ONNX 推理及资源释放。
