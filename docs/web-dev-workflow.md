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

游戏通过 `ParaIO` 写入 `/temp` 的普通文件会自动保存到当前站点的 IndexedDB，刷新后恢复。资源清单按需下载的美术文件由 `CAssetManifest` 标记，其响应按浏览器的 HTTP 缓存规则复用，解包内容只在本次运行中使用。`/temp/cache` 内的其他业务文件也会正常保存。构建 Web 引擎后，可用 `python3 tests/web/test_temp_storage.py` 运行 Chrome 回归测试。

- **CJK 中文字体自动回退**：桌面版目录通常不含 `fonts/DroidSansFallback.ttf`，`serve_webparacraft.py` 会自动映射宿主机中文字体（如 Windows 的 `simhei.ttf` / `msyh.ttc`），也可通过 `--font-file` 手动指定。
- **同机再次启动**：每次读取新的磁盘清单。WASM 保存在 Cache Storage，服务器用 ETag 验证；未变时只返回 304，重编后在同一缓存位置替换。散文件按清单中的文件版本复用，启动脚本包按批次复用。修改 Lua 后刷新即可生效。
- **容量与验证**：在同一浏览器配置、同一站点和端口下复测。在开发者工具中执行 `await navigator.storage.estimate()` 查看站点存储估算（包含 Cache Storage 和 IndexedDB）。配额由浏览器决定；写入失败时仍使用本次下载的文件。用 `python3 tests/web/test_boot_cache.py` 验证重启、重编、同大小脚本修改和低配额。检查服务器日志中的 WASM 304 和 `/fs/batch` 请求数，页面资源计时中的零传输也可能来自 Service Worker 代为下载。

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

## 5. 默认纹理显存格式

静态图片通过统一纹理上传路径选择紧凑格式，WebGL 与桌面 RHI 共用此策略：

- DDS 的 DXT1/3/5 数据和全部 mip 层直接保留，支持 BC/S3TC 的设备不再先解压为 RGBA8。
- 普通 RGB/RGBA 图片默认按实际透明度选择 DXT1（不透明，4 bpp）或 DXT5（透明，8 bpp）；显式 DXT3 请求也会进行编码。保持原始分辨率，不缩小图片。
- 文件名以 `_32bits` 结尾（扩展名前，不区分大小写），且 `Enable32bitsTexture` 开启时，保留 RGBA8。显式指定未压缩格式也不会自动压缩。
- BC 上传要求设备支持且基础宽高都是 4 的倍数。不满足时，OpenGL/WebGL 使用 RGB565 或 RGBA4444（16 bpp）；其他 RHI 保留可用的未压缩格式。WebGL2/GLES3 使用明确的 16 位内部存储格式。
- 渲染目标、字体/单通道纹理及通过原始像素接口创建或更新的动态纹理不应用此静态图片策略。CPU 图片读取仍默认解压 DDS，供合成、读像素等操作使用。

压缩会损失少量色彩精度；需要无损 UI 图案时可使用 `_32bits`。无需修改磁盘上的图片或 DDS 文件，重编 WASM 后刷新页面即可使用新策略。

运行时默认使用快速编码模式。RGB 图片直接逐个 4×4 块编码，不创建整张 RGBA 中间图；转换缓冲区在上传后释放，支持 BC 的 GPU 直接使用 DDS 块，不重复编码。

运行 `test_texture_compression --benchmark` 可比较预热后普通/高质量编码的耗时、RGB PSNR 和字节数。基准使用合成图片，不能代表所有素材和设备；计时仅用于诊断，不作为 CTest 通过条件。
