# Web 版（Emscripten）开发调试指南

修改 C++ 引擎或 NPL/Lua 脚本后，在本机浏览器中构建并测试 Web 版 ParaCraft（单线程版 `ParaCraftSingleThread`）。
适用于 macOS 和 Linux / WSL。一键脚本：[`scripts/dev_web.sh`](../scripts/dev_web.sh)。

```bash
scripts/dev_web.sh all      # 重新构建 C++ + 打包脚本 + 刷新缓存哈希
scripts/dev_web.sh serve    # 启动本地服务器，打开 http://127.0.0.1:8088/npl/webparacraft/index.html
scripts/dev_web.sh serve --https   # HTTPS（测试 WebXR），打开 https://127.0.0.1:8088/npl/webparacraft/index.html
```

---

## 1. 整体结构

涉及三个仓库，页面最终只读取 `webparacraft/auto_generate_files/` 里的文件：

```
NPLRuntime (本仓库, cp_old)            paracraft_script (Lua 源码, dev)
   │ emmake make ParaCraftSingleThread    │ 改动的 script/ config/ Mod/ 文件
   ▼                                      ▼
ParaCraftSingleThread.js/.wasm      覆盖到 Jenkins 发布的脚本包之上
   │                                      │ (main150727.pkg 等) → file_packager
   └──────────────┬───────────────────────┘
                  ▼
   ~/webparacraft/auto_generate_files/
       ParaCraftSingleThread.js / .wasm      ← 你的引擎
       paracraft_asset_script.data / .js     ← 发布脚本包 + 你改的脚本
       paracraft_asset.data / ...            ← 美术资源（不变，用 Jenkins 版）
       paracraft_sitefile_md5sums.js         ← 每个文件的哈希，决定浏览器缓存
                  │
                  ▼
   scripts/serve_webparacraft.py  →  http://127.0.0.1:8088/npl/webparacraft/
```

| 目录 | 内容 | 默认位置 |
|------|------|----------|
| NPLRuntime | C++ 引擎（本仓库） | 任意 |
| webparacraft | 网页、JS 桥、`auto_generate_files/` 构建产物 | `~/webparacraft` |
| paracraft_script | NPL/Lua 脚本源码（`script/`、`Mod/`、`config/`） | `~/CODE/paracraft_script` |
| emsdk | Emscripten 工具链 | `~/emsdk/emsdk` |
| Boost 1.85 | 用 emscripten 编译的 Boost | `~/boost_1_85_0` |

---

## 2. 一次性环境准备

### 2.1 emsdk 4.0.10

emsdk 需要 Python ≥ 3.10。macOS 自带的是 3.9，需要先安装：`brew install python@3.12`。

```bash
export EMSDK_PYTHON=/opt/homebrew/bin/python3.12   # 仅 macOS；Linux 自带 python 够新可省略
mkdir -p ~/emsdk && cd ~/emsdk
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install 4.0.10 && ./emsdk activate 4.0.10
```

`dev_web.sh` 会自动找到合适的 Python 并 `source emsdk_env.sh`，日常不需要手动设置环境。

### 2.2 Boost 1.85

```bash
cd ~ && curl -fLO https://archives.boost.io/release/1.85.0/source/boost_1_85_0.tar.gz
tar xzf boost_1_85_0.tar.gz && cd boost_1_85_0
source ~/emsdk/emsdk/emsdk_env.sh
./bootstrap.sh
./b2 -a toolset=emscripten runtime-link=static exception-handling=off \
  cxxflags='-fexceptions -fdeclspec -sNO_DISABLE_EXCEPTION_CATCHING -pthread' \
  cflags='-fexceptions -fdeclspec -sNO_DISABLE_EXCEPTION_CATCHING -pthread' \
  linkflags='-fexceptions -fdeclspec -sNO_DISABLE_EXCEPTION_CATCHING -pthread' \
  --with-thread --with-filesystem --with-system --with-chrono --with-serialization \
  --with-iostreams --with-locale --with-date_time -j8
```

> **注意 `exception-handling=off`。** 机器上的 node ≥ 16 时（emsdk 4.x 自带 node 24），Boost 的
> emscripten 工具集会自动加 `-fwasm-exceptions`，与引擎使用的 JS 异常
> （`-sNO_DISABLE_EXCEPTION_CATCHING`）冲突，报错
> `DISABLE_EXCEPTION_CATCHING=0 is not compatible with -fwasm-exceptions`。
> 关掉工具集自带的异常参数后，异常模式完全由我们传入的 flags 决定。
> `build_emscripten.sh` 里的命令缺少这个参数。

### 2.3 克隆网页和脚本仓库（浅克隆）

```bash
git clone --depth 1 http://code.kp-para.cn/paracraft/webparacraft.git ~/webparacraft
git clone --depth 1 http://code.kp-para.cn/paracraft/paracraft_script.git ~/CODE/paracraft_script
```

### 2.4 初始化

```bash
scripts/dev_web.sh setup
```

它会：

1. 预编译 emscripten 的 zlib / libjpeg / libpng / sdl2 端口（全新 SDK 上 CMake 的 `find_package(ZLIB)` 需要它们）；
2. 用 `HTML_ROOT=~/webparacraft/auto_generate_files` 配置 `build/emscripten_single_thread`；
3. 从 webparacraft 的 **git HEAD** 解包 Jenkins 发布的脚本包（`main150727.pkg`、`ParacraftBuildinMod.zip`、`config.txt`、`version.txt`、`assets_manifest.txt`）到 `build/emscripten_single_thread/web_script_base/`；
4. 记录 paracraft_script 当前 commit，作为之后“改了哪些脚本”的比较基准。

之后第一次 `all` 会完整编译一遍引擎（约 4 分钟，M 系列 10 核），以后都是增量编译。

---

## 3. 日常开发循环

| 命令 | 作用 |
|------|------|
| `scripts/dev_web.sh all` | 默认。`cpp` + `script` + 刷新哈希 |
| `scripts/dev_web.sh cpp` | 只增量编译 C++，复制 `.js/.wasm`，刷新哈希 |
| `scripts/dev_web.sh script` | 只重新打包脚本，刷新哈希（不用编译 C++，几秒钟） |
| `scripts/dev_web.sh script --all` | 把整个 `script/ config/ Mod/` 都覆盖上去（约 100 MB，见 §5） |
| `scripts/dev_web.sh serve` | 启动服务器，端口 8088，本机和局域网都可访问（见 §3.4）。`PORT=xxxx` 改端口，`HOST=127.0.0.1` 只允许本机，`--https` 自动生成证书并启用 HTTPS，或用 `CERT=... KEY=...` 指定证书 |
| `scripts/dev_web.sh status` | 查看哪些产物已替换成本地构建、哪些脚本会被覆盖 |
| `scripts/dev_web.sh restore` | 把 `auto_generate_files/` 还原成 Jenkins 版本 |

### 3.1 改 C++

1. 在 `NPLRuntime/` 下修改代码。
2. `scripts/dev_web.sh cpp`
3. 浏览器刷新页面。

只改 `library_webxr.js` 等 `--js-library` 文件时，make 不一定会重新链接。先
`touch NPLRuntime/Platform/SDL/emscripten/main.cpp` 再构建。

### 3.2 改 NPL/Lua 脚本

1. 在 `~/CODE/paracraft_script` 中正常修改（和桌面版开发一样，提交到 `dev` 分支）。
2. `scripts/dev_web.sh script`。脚本会列出所有被覆盖的文件：
   ```
   [dev_web]   overlay script/apps/Aries/Creator/Game/xxx.lua
   [dev_web] overlaid 1 file(s) on the released script package
   ```
3. 浏览器刷新页面。

被覆盖的文件 = 相对 setup 时记录的基准 commit **有改动的文件（无论是否已提交）+ 未跟踪的新文件**，
范围限定在 `script/`、`config/`、`Mod/`。

### 3.3 浏览器端

打开 **http://127.0.0.1:8088/npl/webparacraft/index.html**，然后在 DevTools 中：

- **Application → Service workers → 勾选 “Bypass for network”**（开发期间一直勾着）。
- 如果之前用别的方式打开过 `localhost` 页面并出现 `NPL WebServer request timeout` / 502，
  在那个地址上执行 **Application → Storage → Clear site data**。

测试 WebXR：安装 Immersive Web Emulator 扩展，DevTools 中打开 *WebXR* 面板选择 Quest 设备，
在页面上通过按钮触发 `isXR = true`（必须在用户点击之后）。详见
[webxr-emscripten.md](webxr-emscripten.md)。

Quest 真机：`adb reverse tcp:8088 tcp:8088`，然后在 Quest 浏览器中打开 `http://localhost:8088/npl/webparacraft/index.html`。

### 3.4 局域网访问

```bash
scripts/dev_web.sh serve              # 默认监听 0.0.0.0，启动时同时打印本机和局域网地址
```

在其它设备上打开 `http://<本机IP>:8088/index.html`（注意**没有** `/npl/webparacraft/`：非 localhost 时
页面从根路径 `/` 加载资源，服务器同时支持两种布局）。

局域网 IP 上的 `http://` **不是安全上下文**，浏览器会禁用 Service Worker、`SharedArrayBuffer` 和
**`navigator.xr`（WebXR）**。单线程版本身可以运行，但不能测试 WebXR。需要安全上下文时，三选一：

1. **Chrome 标志（最简单，适合局域网内其它电脑）**：在访问端打开
   `chrome://flags/#unsafely-treat-insecure-origin-as-secure`，填入 `http://<本机IP>:8088`，重启浏览器。
2. **HTTPS**：
   ```bash
   brew install mkcert                    # 推荐；只需一次
   scripts/dev_web.sh serve --https       # 打开 https://127.0.0.1:8088/npl/webparacraft/index.html
   ```
   `--https` 在 `build/web_cert/` 下生成覆盖 `localhost`、`127.0.0.1` 和本机局域网 IP 的证书，之后重复使用
   （局域网 IP 变化时重新生成）。也可以用 `CERT=... KEY=...` 指定已有证书。
   - 装了 mkcert：首次运行会执行 `mkcert -install`（需要输入一次系统密码），本机浏览器直接信任该证书。
     其它设备需要导入 `mkcert -CAROOT` 目录下的 `rootCA.pem` 才会信任。
   - 没装 mkcert：改用 openssl 生成自签名证书。浏览器会显示证书警告，点 *继续前往* 可以打开页面，
     但 **Chrome 不会在证书错误的页面上注册 Service Worker**，而本项目依赖 Service Worker，所以建议装 mkcert。
3. **Quest 用 USB**：`adb reverse` 后访问 `localhost`，天然是安全上下文（见上文）。

如果其它设备仍然连不上：检查 macOS 防火墙是否允许 Python 接收传入连接；如果访问端配置了 HTTP 代理，
需要把局域网地址加入代理的绕过列表。

---

## 4. 原理

### 4.1 为什么必须用 `/npl/webparacraft/` 和 8088 端口

在 `localhost` / `127.0.0.1` 上，`index.html` 把资源根设为 `<origin>/npl/webparacraft/`，并在根作用域
`/` 注册 `/ServiceWorker.js`。该 Service Worker 在本地只把 `/npl/webparacraft/*` 当作网页文件，
**其它所有请求（包括 `/`）都转发给浏览器内的 NPL WebServer**。因此：

- 用 `scripts/serve.py` 把仓库放在 `/` 下 → 资源 404，刷新后变成 `502 NPL WebServer request timeout`；
- `scripts/serve_webparacraft.py` 采用与 webparacraft 自带的 `.vscode/serve-webparacraft.ps1` 相同的布局
  （端口 8088，`keepwork/local.html` 也写死了这个地址），并加上 COOP/COEP 头。

### 4.2 为什么每次都要刷新哈希

`Module.locateFile` 给每个 `.js/.wasm/.data` 加上 `?hash=<md5>`，哈希来自
`auto_generate_files/paracraft_sitefile_md5sums.js`；Service Worker 对这些文件**缓存优先**。
文件变了但哈希没变时，浏览器会继续使用旧版本。`dev_web.sh` 在每次构建后都会运行
webparacraft 自带的 `calculate_md5.sh`。

### 4.3 为什么只覆盖改动的脚本就能生效

脚本在 Web 版中以 `main150727.pkg`（Jenkins 用 paracraft_script `master` 编译打包）的形式加载。
引擎默认**磁盘文件优先于归档文件**（`CParaFile::m_nDiskFilePriority = 0`，见
`NPLRuntime/ParaEngine/IO/ParaFile.h`），所以把改过的 `.lua` 按原路径放在 pkg 旁边，就会替代 pkg 里的同名文件，
不需要重新打 pkg。

---

## 5. 注意事项

- **脚本版本不一致。** 基础包来自 `master`（`main150727.pkg`），而你在 `dev` 上开发。如果改动依赖 `dev`
  上其它尚未发布的修改，需要把那些文件也覆盖上去（修改一下或用 `script --all`）。
  `--all` 会把约 100 MB 源码打进 `.data`，加载明显变慢，只在必要时使用。
- **删除的文件不会被删除。** 源码里删掉的文件，发布包里仍然存在。
- **`Mod/` 的覆盖未验证。** 内置 Mod 从 `npl_packages/ParacraftBuildinMod.zip` 加载，散文件是否优先取决于 Mod 加载方式。
- **美术资源包不重新打包。** `paracraft_asset.data` 等仍是 Jenkins 版本。
- **不要提交 `auto_generate_files/`。** 本地构建产物会让 webparacraft 工作区出现改动，提交前 / `git pull` 前先
  `scripts/dev_web.sh restore`。webparacraft 更新后重新执行 `setup`，以便解包新的发布脚本包。
- **paracraft_script 更新后**（`git pull`），重新执行 `setup` 更新比较基准，否则拉下来的所有改动都会被当成覆盖文件。
- 多线程版（`ParaCraft.js`）不在此流程内。

---

## 6. 故障排查

| 现象 | 原因 / 处理 |
|------|-------------|
| `emsdk requires python 3.10 or above` | 安装 Python 3.12，或设置 `EMSDK_PYTHON` |
| Boost：`DISABLE_EXCEPTION_CATCHING=0 is not compatible with -fwasm-exceptions` | b2 缺少 `exception-handling=off`（§2.2） |
| CMake：`Could NOT find ZLIB` | 未预编译 emscripten 端口；运行 `setup`，或 `embuilder build zlib` |
| 完整 `make` 报 `~/paracraft_script@/ does not exist` | 默认目标包含资源打包。只构建引擎：`make ParaCraftSingleThread`（`dev_web.sh` 就是这样做的） |
| 页面资源 404 | 没有通过 `/npl/webparacraft/` 访问（§4.1） |
| `502` / `NPL WebServer request timeout` | 旧的 Service Worker 接管了该 origin，执行 Clear site data |
| 改动不生效 | 没勾选 “Bypass for network”，或手动复制文件后没有刷新哈希 |
| 一直加载 / `SharedArrayBuffer is not defined` | 服务器没有 COOP/COEP 头，用 `dev_web.sh serve` |
| 局域网 `ERR_CONNECTION_REFUSED` | 服务器只监听 127.0.0.1（设置了 `HOST=127.0.0.1`），去掉该变量重启 `serve` |
| 局域网能打开但 WebXR 不可用 | 非安全上下文，见 §3.4 |

---

## 7. 环境变量

`dev_web.sh` 的所有路径都可以覆盖：

| 变量 | 默认值 |
|------|--------|
| `EMSDK_DIR` | `~/emsdk/emsdk` |
| `EMSDK_PYTHON` | 自动查找 ≥ 3.10 的 Python |
| `BOOST_DIR` | `~/boost_1_85_0` |
| `BUILD_DIR` | `<repo>/build/emscripten_single_thread` |
| `WEB_DIR` | `~/webparacraft` |
| `SCRIPT_SRC` | `~/CODE/paracraft_script` |
| `APP_ROOT` | `~/paracraft`（只传给 CMake） |
| `PORT` | `8088` |
| `HOST` | `0.0.0.0`（本机 + 局域网）；`127.0.0.1` 只允许本机 |
| `CERT` / `KEY` | 未设置；设置后用 HTTPS 提供服务（`serve --https` 时自动生成） |
| `CERT_DIR` | `<repo>/build/web_cert`（`serve --https` 生成的证书） |
| `JOBS` | CPU 核数 |
