# Web 版（Emscripten）本地磁盘直读开发指南

`NPLRuntimeUnified` 内置了轻量级 Web 宿主目录 [`web/`](../web/)（约 540 KB）和本地磁盘映射驱动 [`web/src/local_disk_fs.js`](../web/src/local_disk_fs.js)。
**无需克隆 345 MB 的 `webparacraft` 或 `paracraft_script` 仓库，也无需使用 `file_packager` 打包 `.data` 资源包**——浏览器直接读取本机桌面版 Paracraft 目录（如 `C:/lxzsrc/ParaEngine/ParaWorld/` 或 `C:/Paracraft/`），行为与桌面客户端一致。

---

## 1. 快速开始

### Windows（WSL 编译 + 本机浏览器测试）

```powershell
# 1. 在 WSL 中用最新 Emscripten 构建 ParaCraftSingleThread.{js,wasm}
.\scripts\dev_web.ps1 build

# 2. 启动（或复用已运行的）本地 Web + 磁盘直读服务并在浏览器打开 530 项目
.\scripts\dev_web.ps1 run -SearchRoot "C:/lxzsrc/ParaEngine/ParaWorld/" -SearchPath "script/" -ProjectId "530"

# 或者在 VSCode 中直接运行任务（Terminal -> Run Task...）：
#   - Web: Build and Run Project 530
#   - Web: Build WASM (Emscripten)
#   - Web: Run Project 530 (Local Disk)
```

### Linux / macOS / WSL

```bash
scripts/dev_web.sh build    # 编译 ParaCraftSingleThread.{js,wasm} 到 web/auto_generate_files/
scripts/dev_web.sh serve    # 启动本地服务（127.0.0.1:8088）
```

然后在浏览器打开（沿用已有的 `searchroot` / `searchpath` URL 约定）：

```text
# 挂载整个本地 ParaWorld 目录并进入 530 世界
http://127.0.0.1:8088/npl/webparacraft/index.html?searchroot=C:/lxzsrc/ParaEngine/ParaWorld/&pid=530

# 与 keepwork/local.html 约定完全一致，只覆盖 script/ 子树（同时自动加载根目录下的 .pkg / config / Mod）
http://127.0.0.1:8088/npl/webparacraft/keepwork/local.html?searchroot=C:/lxzsrc/ParaEngine/ParaWorld/&searchpath=script/&pid=530

# 直接使用已安装的桌面客户端目录 C:/Paracraft/ 打开本地空白世界
http://127.0.0.1:8088/npl/webparacraft/index.html?searchroot=C:/Paracraft/&world=worlds/DesignHouse/_emptyworld
```

---

## 2. 工作原理（零 `.data` 包、无需 Keepwork MCP）

```
本机 Paracraft 目录 (?searchroot=...)
  ├── main.pkg / main150727.pkg / npl_packages/ParacraftBuildinMod.zip
  ├── config/ / database/ / Mod/ / worlds/
  └── script/*.lua（散文件，优先级高于 .pkg 归档）
          │
          ▼  直接由本地 scripts/serve_webparacraft.py 提供（不经过 8089 MCP）
  GET /fs/manifest?root=<searchroot>&searchpath=<searchpath>
  GET /fs/file?root=<searchroot>&path=<relpath>
          │
          ▼  web/src/local_disk_fs.js
  1. preRun 阶段拉取目录清单，在 MEMFS 中建立内存索引与大小写无关查找表（stat/readdir 0 网络延迟）
  2. 并行异步预加载核心启动包（.pkg、ParacraftBuildinMod.zip、config、CJK 字体）
  3. 散落的 .lua / Mod / cache 文件在首次读取字节时按需拉取并缓存在 MEMFS；改完 .lua 刷新网页即生效
  4. IDBFS 挂载（/worlds、/Database、/apps）完成后自动合并本地目录且不覆盖浏览器存档
```

- **CJK 中文字体自动回退**：桌面版目录通常不含 `fonts/DroidSansFallback.ttf`，`serve_webparacraft.py` 会自动映射宿主机中文字体（如 Windows 的 `simhei.ttf` / `msyh.ttc`），也可通过 `--font-file` 手动指定。
- **无缓存污染**：`web/ServiceWorker.js` 仅转发内嵌 NPL WebServer（`/__npl_webserver__` 等路由），不对 `.js` / `.wasm` 做 CacheStorage 缓存，重编 C++ 或修改 Lua 后刷新即最新代码。

---

## 3. 环境准备（首次）

1. **Emscripten SDK（支持最新版如 7.0+）**：
   ```bash
   mkdir -p ~/emsdk && cd ~/emsdk
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk && ./emsdk install latest && ./emsdk activate latest
   ```
2. **Boost 1.85（静态编译，关闭 `-fwasm-exceptions` 以匹配 `-sNO_DISABLE_EXCEPTION_CATCHING`）**：
   ```bash
   cd ~/boost_1_85_0
   source ~/emsdk/emsdk/emsdk_env.sh
   ./bootstrap.sh
   ./b2 -a toolset=emscripten runtime-link=static exception-handling=off \
     cxxflags='-fexceptions -fdeclspec -sNO_DISABLE_EXCEPTION_CATCHING -pthread' \
     cflags='-fexceptions -fdeclspec -sNO_DISABLE_EXCEPTION_CATCHING -pthread' \
     linkflags='-fexceptions -fdeclspec -sNO_DISABLE_EXCEPTION_CATCHING -pthread' \
     --with-thread --with-filesystem --with-system --with-chrono --with-serialization \
     --with-iostreams --with-locale --with-date_time -j8
   ```

---

## 4. 常用选项与环境变量

| 参数 / 环境变量 | 默认值 | 说明 |
|---|---|---|
| `?searchroot=<path>` | 自动探测 `ParaWorld` / `C:/Paracraft` | URL 参数，指定本机 Paracraft 磁盘根目录 |
| `?searchpath=<dirs>` | 全部目录 | URL 参数，逗号或分号分隔的相对子目录（如 `script/;Mod/`） |
| `WEB_ROBOT=OFF` / `-NoRobot` | `ON` | 关闭 MuJoCo + ONNX Runtime Web 机器人模块以加快日常非机器人 C++ 编译 |
| `BUILD_DIR` | WSL 下 `~/build/npl_unified_wasm` | CMake 构建目录（WSL 下放在原生 ext4 文件系统比 `/mnt/c` 快约 10 倍） |
| `PORT` / `-Port` | `8088` | 本地 HTTP 监听端口 |
