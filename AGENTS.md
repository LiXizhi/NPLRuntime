# AGENTS.md — NPLRuntimeUnified / ParaEngine

Guidance for AI coding agents working in `NPLRuntimeUnified` (`unified-crossplatform` branch).

## Overview

`NPLRuntimeUnified` unifies the Windows/DirectX `dev` branch and cross-platform `cp-old` branch of **NPLRuntime / ParaEngine** (C++11–20 + Lua 5.1 / LuaJIT 2.1) into a single cross-platform codebase supporting:
- **Desktop:** Windows (`ParaEngineClient` OpenGL/DirectX), Linux/macOS (`ParaCraftSDL2`)
- **Server:** Headless `ParaEngineServer` (`-DNPLRUNTIME_SERVER=ON`)
- **Web (WebAssembly):** `ParaCraftSingleThread.{js,wasm}` built with **Emscripten 7.0+** (WebGL2 + SDL2 + MuJoCo + ONNX Runtime 1.27.1), mounting local Paracraft files directly from disk without `.data` bundles or external `webparacraft` repo clones.
- **Mobile:** Android, iOS, HarmonyOS

## Haqi / 魔法哈奇

For Haqi native or browser launches, remembered login, C++ and NPL profiling in the same plaza, or testing the official homepage and downloads, start with [Running and testing Haqi](docs/haqi-testing.md). It identifies the repository for each layer and links the launchers, fixtures and reference captures. The Project 530 workflow below uses the Paracraft shell.

## Repository Layout

```text
NPLRuntimeUnified/
├── NPLRuntime/                  # Main CMake root
│   ├── ParaEngine/              # Core engine (3dengine, 2dengine, Core, NPL, ParaScriptBindings, etc.)
│   ├── RenderSystem/            # RenderSystemOpenGL, RenderSystemD3D9, RenderSystemNull, RenderSystemRHICore
│   ├── Platform/                # OS hosts (Windows, SDL/emscripten, Linux, Android, iOS, HarmonyOS)
│   ├── Plugins/                 # Bullet physics, cAudioEngine, SQLite, etc.
│   ├── externals/               # Vendored third-party libraries
│   └── cmake/                   # CMake helpers (WebRobotDefaults.cmake, WebRobotOnnx.cmake, etc.)
├── Server/trunk/                # Legacy server tree + vendored MuJoCo & onnxruntime-1.27.1
├── web/                         # Lightweight (~560 KB) self-contained WebParacraft shell
│   ├── index.html               # Entry HTML (/npl/webparacraft/index.html)
│   ├── keepwork/local.html      # Keepwork local-dev compatible entry HTML
│   ├── ServiceWorker.js         # Embedded NPL WebServer route bridge (no .wasm/.js CacheStorage caching)
│   ├── src/local_disk_fs.js     # Local-disk -> Emscripten MEMFS driver (?searchroot=...&searchpath=...)
│   ├── src/                     # index.js, emscripten.js, emscripten_api.js, webview.js, NPL.js
│   ├── libs/                    # Runtime JS libs (isLoading, tone, socket.io, mqtt)
│   └── auto_generate_files/     # Git-ignored build output: ParaCraftSingleThread.{js,wasm}
├── scripts/
│   ├── dev_web.ps1              # Windows PowerShell wrapper (WSL Emscripten build + native HTTP server)
│   ├── dev_web.sh               # Linux/WSL/macOS Emscripten configure/build/serve script
│   └── serve_webparacraft.py    # Local HTTP + direct /fs/manifest & /fs/file server (port 8088)
├── .vscode/
│   ├── tasks.json               # VSCode build & run tasks (Windows client + Web Project 530)
│   └── launch.json              # VSCode debug/launch configs
└── docs/
    └── web-dev-workflow.md      # Detailed WebAssembly local-disk workflow guide
```

---

## WebAssembly (Emscripten 7.0+) Build & Local-Disk Run

### Key Architecture (Zero `.data` Bundles, Zero MCP Dependency)

Instead of packing hundreds of megabytes with Emscripten `file_packager` or cloning `webparacraft`, the browser mounts an existing local desktop Paracraft directory (such as `C:/lxzsrc/ParaEngine/ParaWorld/` or `C:/Paracraft/`) using the standard URL query conventions:
- `?searchroot=C:/lxzsrc/ParaEngine/ParaWorld/` — local Paracraft directory on disk
- `&searchpath=script/` — optional comma/semicolon-separated loose directory filter (core root `.pkg` archives, top-level `config/*`, `npl_packages/*.zip`, `database/`, `fonts/`, and `Mod/` are always included automatically)
- `&pid=530` or `&world=worlds/DesignHouse/...` — world/project to load on startup

How `scripts/serve_webparacraft.py` + `web/src/local_disk_fs.js` work together:
1. **Direct HTTP disk endpoints (`127.0.0.1:8088`):** `serve_webparacraft.py` serves `/fs/manifest?root=...&searchpath=...` and `/fs/file?root=...&path=...` directly from Python (loopback-restricted by default, with HTTP `Range` support and automatic host CJK font fallback to `fonts/DroidSansFallback.ttf`). Keepwork MCP (`:8089`) is **not** required.
2. **Pre-boot MEMFS index + core prestage:** Before `Module._main()` runs, `local_disk_fs.js` builds a case-insensitive directory tree in MEMFS (`stat`/`readdir` are synchronous in-memory) and fetches root `.pkg` files (encrypted `.pkg` files are decrypted natively by `CParaEngine` C++), top-level `config/*` files, `npl_packages/*.zip`, and the CJK font in parallel.
   - *Important:* `shouldPrestageAtBoot` intentionally only prestages direct children of `config/` (`lower.indexOf("/", 7) < 0`) so legacy subdirectories like `ParaWorld/config/Aries/*` (~36,000 XML files) are fetched lazily on demand rather than blocking boot.
3. **On-demand loose file loading:** Loose `.lua`, `.xml`, and asset files are fetched lazily on first byte read via synchronous XHR in `MEMFS.stream_ops` and cached in RAM. Editing a local `.lua` file and refreshing the browser picks up the change immediately.

### Prerequisites (WSL Ubuntu on Windows, or Linux/macOS)

- **Emscripten SDK (`~/emsdk/emsdk`):** Verified with Emscripten `7.0.0` (`Clang 22`, `-std=c++20`).
  - Note: Emscripten `>= 4.0.10` removed legacy `-sUSE_WEBGPU=1`; `NPLRuntime/CMakeLists.txt` automatically disables `NPLRUNTIME_ENABLE_WEBGPU` on Emscripten `>= 4.0.10` and uses `RenderSystemOpenGL` (WebGL2).
- **Boost 1.85 (`~/boost_1_85_0`):** Static libraries built with `toolset=emscripten exception-handling=off cxxflags='-fexceptions -fdeclspec -sNO_DISABLE_EXCEPTION_CATCHING -pthread'` (see [`docs/web-dev-workflow.md`](docs/web-dev-workflow.md)).
- **WSL Build Directory:** Defaults to native ext4 `~/build/npl_unified_wasm` (~10x faster than building under `/mnt/c/...`), then copies `ParaCraftSingleThread.{js,wasm}` to `web/auto_generate_files/`.

### VSCode Tasks (Project 530)

Defined in [`.vscode/tasks.json`](.vscode/tasks.json):

| VSCode Task Label | Behavior |
|---|---|
| **`Web: Build and Run Project 530`** | Sequentially builds `ParaCraftSingleThread.{js,wasm}` via WSL Emscripten, starts (or reuses) `serve_webparacraft.py` on `127.0.0.1:8088`, and opens Project `530` in the browser |
| **`Web: Build WASM (Emscripten)`** | Incremental C++ -> WASM build (`scripts/dev_web.ps1 build`) into `web/auto_generate_files/` |
| **`Web: Run Project 530 (Local Disk)`** | Starts `serve_webparacraft.py` in the background (reusing port `8088` if already running) and opens Project `530` |
| **`Web: Serve (Local Disk)`** | Starts `serve_webparacraft.py` on `127.0.0.1:8088` without opening a browser tab (used by F5 launch config `Web: Launch Project 530 (Chrome)`) |
| **`Web: Configure WASM (Emscripten)`** | Re-runs `emcmake cmake` in WSL `~/build/npl_unified_wasm` |

### CLI Commands

**Windows (PowerShell):**
```powershell
# Build WASM only (WSL Ubuntu -> web/auto_generate_files/ParaCraftSingleThread.{js,wasm})
.\scripts\dev_web.ps1 build

# Start (or reuse) local HTTP server on 127.0.0.1:8088 and open Project 530 in browser
.\scripts\dev_web.ps1 run -SearchRoot "C:/lxzsrc/ParaEngine/ParaWorld/" -SearchPath "script/" -ProjectId "530"

# Build + Serve + Open Project 530 in one command
.\scripts\dev_web.ps1 all -SearchRoot "C:/lxzsrc/ParaEngine/ParaWorld/" -SearchPath "script/" -ProjectId "530" -OpenBrowser

# Start server in foreground without auto-opening browser
python -u .\scripts\serve_webparacraft.py --port 8088 --host 127.0.0.1 --searchroot "C:/lxzsrc/ParaEngine/ParaWorld"
```

**Linux / macOS / WSL:**
```bash
scripts/dev_web.sh build                                              # Incremental WASM build
WEB_ROBOT=OFF scripts/dev_web.sh build                                # Faster build without MuJoCo + ONNX Runtime
SEARCHROOT=C:/lxzsrc/ParaEngine/ParaWorld scripts/dev_web.sh serve    # Serve on 127.0.0.1:8088
```

### Browser Test URLs

```text
# Project 530 using ParaWorld local scripts + packages (Keepwork local convention)
http://127.0.0.1:8088/npl/webparacraft/keepwork/local.html?searchroot=C:/lxzsrc/ParaEngine/ParaWorld/&searchpath=script/&pid=530

# Project 530 via root index.html
http://127.0.0.1:8088/npl/webparacraft/index.html?searchroot=C:/lxzsrc/ParaEngine/ParaWorld/&searchpath=script/&pid=530

# Offline local world using installed C:/Paracraft/ client directory
http://127.0.0.1:8088/npl/webparacraft/index.html?searchroot=C:/Paracraft/&world=worlds/DesignHouse/_emptyworld
```

---

## Native Desktop, Server & Mobile Builds

See [`README.md`](README.md) for the full Host × Target build & run matrix across Windows, Linux, macOS, WebAssembly, Android, PICO VR, iOS, and HarmonyOS.

### Windows Client & Server (VS 2022 / VS 2026, Multi-Backend ParaRHI)
- **VSCode Tasks ([`.vscode/tasks.json`](.vscode/tasks.json)):**
  - `Build ParaEngineClient (Debug)` *(default `Ctrl+Shift+B`)* -> `build-win64/bin/ParaEngineClient_d.exe`
  - `Build ParaEngineClient (Release)` -> `build-win64/bin/ParaEngineClient.exe`
  - `Windows: Configure ParaEngineClient (CMake)`
  - `Build ParaEngineServer (Release)` -> `build-server/bin/ParaEngineServer.exe`
- **Root Script (`build_win32.bat`) / Direct CMake:**
  ```bat
  build_win32.bat                        :: Release client -> build-win64\bin\ParaEngineClient.exe
  build_win32.bat debug                  :: Debug client   -> build-win64\bin\ParaEngineClient_d.exe
  build_win32.bat server                 :: Headless server -> build-server\bin\ParaEngineServer.exe
  build_win32.bat run auto 530           :: Launch World 530 (auto | d3d12 | vulkan | d3d11 | opengl | webgpu)
  build_win32.bat run-server             :: Run ParaEngineServer with NPLRuntime\tests\helloworld.lua
  build_win32.bat test                   :: Run CTest suite in build-win64
  :: Direct CMake equivalent:
  cmake -S NPLRuntime -B build-win64 -A x64 -DNPLRUNTIME_RENDERER=OPENGL
  cmake --build build-win64 --config Release --parallel -- /v:minimal
  ```

### Linux / macOS / WSL Client & Headless Server
- **macOS VSCode Tasks ([`.vscode/tasks.json`](.vscode/tasks.json)):**
  - `Mac: Build and Run Project 530` — builds `Paracraft.app` and launches Project `530` (`CodeBlockTest`)
  - `Mac: Build Paracraft` — incremental CMake build of `Paracraft` (Metal + OpenGL)
  - `Mac: Run Project 530` — launches Project `530` (`./build_linux.sh run auto 530`)
  - `Mac: Configure Paracraft (CMake)` — re-runs CMake configure
- **Root Script (`./build_linux.sh`):**
  ```bash
  ./build_linux.sh server                  # Headless server -> build-server/bin/ParaEngineServer
  ./build_linux.sh run-server              # Smoke-test ParaEngineServer with helloworld.lua
  ./build_linux.sh client                  # Desktop client (ParaCraftSDL2 on Linux, Paracraft.app on macOS)
  ./build_linux.sh run auto 530            # Launch World 530 (auto | metal | opengl on macOS; ParaCraftSDL2 on Linux)
  ./build_linux.sh web build               # Emscripten WASM -> web/auto_generate_files/
  ```

### Android emulator and iOS Simulator

For a standalone mobile build or smoke test on Apple Silicon, start with [Build and test Paracraft on Android and iOS](docs/mobile-emulator-testing.md). It gives the exact Gradle flavor, Xcode target, asset staging commands and expected runtime results. Use [the dependency recipes](docs/mobile-dependencies.md) on a fresh host. Verify Android touch and OpenGL ES, or create a local world with Metal on iOS, then background and resume the app on the selected emulator.

---

## Coding & Portability Conventions

1. **Cross-platform C++ types:** Avoid MSVC-only `__int64` or unguarded `<windows.h>` APIs in shared `ParaEngine/` headers; use `<cstdint>` (`int64_t`) and `os_calls.h` helpers so GCC/Clang/Emscripten C++20 builds succeed cleanly.
2. **File & asset access:** Always go through `CParaFile`, `CFileUtils`, and `CParaWorldAsset` (never hardcoded OS paths) so `.pkg` archives, `searchpath` overrides, and Emscripten `MEMFS`/`IDBFS` work identically across desktop and web.
3. **Rendering:** Use `CGlobals::GetRenderDevice()` / `RenderSystem` abstractions rather than raw DirectX or OpenGL calls in `ParaEngine/`.
4. **Threading:** Guard Boost/OS worker-thread code with `#ifndef EMSCRIPTEN_SINGLE_THREAD` when touching runtime threading paths.
5. **Lua 5.1 compatibility:** NPL requires strict Lua 5.1 syntax compatibility (no Lua 5.2+ `_ENV` or `goto`).
