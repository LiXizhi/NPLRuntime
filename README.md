# NPLRuntime (Unified Cross-Platform ParaEngine)

**NPLRuntime** is the open-source C++17/20 runtime for **NPL (Neural Parallel Language)** — a high-performance, actor-style concurrent scripting language with **100% Lua 5.1 syntax compatibility** (powered by LuaJIT 2.0/2.1 or Lua 5.1). Together with **ParaEngine**, it powers [Paracraft](https://www.paracraft.cn), 3D/2D simulations, WebXR/OpenXR experiences, robotics (MuJoCo + ONNX Runtime), and distributed application servers across desktop, web, server, mobile, and VR platforms.

> **For AI Agents & Contributors:** Start with the **Quick Build & Run Matrix** below, or see [`AGENTS.md`](AGENTS.md), [`docs/build-system.md`](docs/build-system.md), and [`docs/web-dev-workflow.md`](docs/web-dev-workflow.md) for architecture and subsystem rules.

---

## Quick Build & Run Matrix (Any Host -> Any Target)

All CMake builds use `NPLRuntime/CMakeLists.txt` as the source root (`cmake -S NPLRuntime -B <build-dir>`). Two unified root entry points — **`build_win32.bat`** (Windows) and **`./build_linux.sh`** (Linux / macOS / WSL) — provide non-interactive one-line commands for every target:

| Target Platform | Host OS | One-Line Build Command | Output Artifact | Run / Verify Command |
| :--- | :--- | :--- | :--- | :--- |
| **Windows Client** *(D3D12 / Vulkan / D3D11 / OpenGL / WebGPU)* | Windows | `build_win32.bat` *(or `build_win32.bat debug`)* | `build-win64/bin/ParaEngineClient.exe` *(Debug: `ParaEngineClient_d.exe`)* | `build_win32.bat run auto 530` |
| **Headless Server (`ParaEngineServer`)** | Windows | `build_win32.bat server` | `build-server/bin/ParaEngineServer.exe` | `build_win32.bat run-server` |
| **Headless Server (`ParaEngineServer`)** | Linux / macOS / WSL | `./build_linux.sh server` | `build-server/bin/ParaEngineServer` | `./build_linux.sh run-server` |
| **Linux Desktop Client (`ParaCraftSDL2`)** | Linux | `./build_linux.sh client` *(or `--build-sdl2`)* | `build-client/bin/ParaCraftSDL2` | `./build-client/bin/ParaCraftSDL2` |
| **macOS Desktop Client** | macOS | `./build_linux.sh client` *(or `--xcode`)* | `build-client/bin/ParaEngine.app` | `open build-client/bin/ParaEngine.app` |
| **WebAssembly (Browser / WebGL2 / WebGPU)** | Windows (via WSL) | `build_win32.bat web build` | `web/auto_generate_files/ParaCraftSingleThread.{js,wasm}` | `build_win32.bat web run` |
| **WebAssembly (Browser / WebGL2 / WebGPU)** | Linux / macOS / WSL | `./build_linux.sh web build` | `web/auto_generate_files/ParaCraftSingleThread.{js,wasm}` | `./build_linux.sh web serve` |
| **Android APK** | Windows / Linux / macOS | `build_win32.bat android` / `./build_linux.sh android` | `NPLRuntime/Platform/AndroidStudio/app/build/outputs/apk/...` | `adb install -r <apk>` |
| **PICO VR (OpenXR APK)** | Windows / Linux / macOS | `build_win32.bat picovr` / `./build_linux.sh picovr` | `NPLRuntime/Platform/PicoVR/app/build/outputs/apk/...` | `adb install -r <apk>` |
| **iOS App / Framework** | macOS (Xcode) | `./build_linux.sh ios framework` *(or `device` / `sim`)* | `NPLRuntime/build_ios_framework/.../ParacraftFramework.framework` | Link in Xcode / deploy via Xcode |
| **HarmonyOS NEXT** | Windows / macOS | `cd NPLRuntime/Platform/HarmonyOS && npm install && npm run build:debug` | HarmonyOS HAP package (`entry/build/...`) | Run via DevEco Studio 5.0+ / `hdc` |

Run `build_win32.bat --help` (Windows) or `./build_linux.sh --help` (Linux/macOS/WSL) at any time to list all supported subcommands and environment variables.

---

## Prerequisites

1. **CMake >= 3.15** (`>= 3.28` recommended for Emscripten / modern Visual Studio).
2. **Boost (1.74 – 1.90)** with static libraries (`thread`, `filesystem`, `system`, `chrono`, `serialization`, `iostreams`, `locale`, `date_time`, `regex`):
   - Set `BOOST_ROOT` to your Boost directory, or place Boost in a standard path auto-detected by `build_win32.bat` (`C:\lxzsrc\externals\boost_1_90_0`, `C:\local\boost_1_85_0`, `C:\Boost`).
   - On Linux/macOS, system Boost packages (`libboost-all-dev` / `brew install boost`) work out of the box (or set `BOOST_ROOT` to a custom static Boost build).
3. **Platform Toolchains:**
   - **Windows:** Visual Studio 2022 or 2026 (`Desktop development with C++`, Windows 10/11 SDK). No Developer Command Prompt needed when using `build_win32.bat` or `cmake --build`.
   - **Linux:** GCC 10+ or Clang 12+, OpenGL/X11 dev headers, and `libsdl2-dev` (for desktop client).
   - **macOS / iOS:** Xcode 14+ & Command Line Tools.
   - **WebAssembly:** Emscripten SDK (`~/emsdk/emsdk`, verified with Emscripten `7.0+`) + Boost 1.85 built with `toolset=emscripten` + Python `>= 3.10` (for the local-disk dev server).
   - **Android / PICO VR:** JDK 17, Android SDK & NDK, and Android-cross-compiled Boost. For the standalone APK, export `BOOST_ROOT` with the target libraries staged under `android-build/stage/<abi>/lib`.
   - **HarmonyOS:** DevEco Studio 5.0+ (HarmonyOS SDK API 12) + Node.js 16+.

---

## Platform Build & Run Guides

### 1. Windows Desktop Client & Multi-Backend ParaRHI

A single `build-win64` build compiles `ParaEngineClient.exe` with **ParaRHI multi-backend support** (`DirectX 12`, `Vulkan`, `DirectX 11`, `OpenGL`, `WebGPU`, and `Null`).

```powershell
# Build Release client -> build-win64/bin/ParaEngineClient.exe (also copied to ParaWorld/bin64/)
.\build_win32.bat

# Build Debug client -> build-win64/bin/ParaEngineClient_d.exe
.\build_win32.bat debug

# Force CMake reconfigure or full clean rebuild
.\build_win32.bat client Release --configure
.\build_win32.bat client Release --clean-first

# Direct CMake equivalent (works with VS 2022 .sln and VS 2026 .slnx)
cmake -S NPLRuntime -B build-win64 -A x64 -DBOOST_ROOT="C:/lxzsrc/externals/boost_1_90_0" -DNPLRUNTIME_RENDERER=OPENGL
cmake --build build-win64 --config Release --parallel -- /v:minimal
```

**Run on Windows (World PID 530 or custom world, choosing any graphics API at runtime):**
```powershell
# Default auto priority (D3D12 -> Vulkan -> D3D11 -> OpenGL)
.\build_win32.bat run auto 530

# Force a specific backend: d3d12 | d3d12-warp | vulkan | d3d11 | d3d11-warp | opengl | webgpu | null
.\build_win32.bat run d3d12 530
.\build_win32.bat run vulkan 530
.\build_win32.bat run opengl 530

# Run CTest verification suite (RHI + subsystem tests)
.\build_win32.bat test Release
```
> Note: `build_win32.bat run` (backed by [`tests/windows/launch_paracraft_530.bat`](tests/windows/launch_paracraft_530.bat)) expects a Paracraft working directory at `..\ParaEngine\Paraworld` by default, or set `PARAWORLD_ROOT=C:\Paracraft`.

---

### 2. Headless Server (`ParaEngineServer` / `npls`) — Windows, Linux, macOS, Docker

Setting `-DNPLRUNTIME_SERVER=ON` forces `-DNPLRUNTIME_RENDERER=NULL` and builds the lightweight headless server binary with zero GPU/windowing dependencies.

**Windows:**
```powershell
.\build_win32.bat server
.\build_win32.bat run-server NPLRuntime/tests/helloworld.lua
```

**Linux / macOS / WSL:**
```bash
# Build headless server in build-server/ (uses all CPU cores automatically)
./build_linux.sh server

# Optionally symlink /usr/local/bin/npl -> ParaEngineServer
sudo ./build_linux.sh server --install

# Run an NPL/Lua script in headless server mode
./build_linux.sh run-server NPLRuntime/tests/helloworld.lua
```

**Docker:**
```bash
docker build -t nplruntime .
docker run --rm nplruntime npl NPLRuntime/tests/helloworld.lua
```

---

### 3. WebAssembly (`Emscripten 7.0+`) — Zero-Bundle Local-Disk Workflow

Web builds produce `ParaCraftSingleThread.{js,wasm}` inside `web/auto_generate_files/` and serve the self-contained [`web/`](web/) shell via [`scripts/serve_webparacraft.py`](scripts/serve_webparacraft.py) (`http://127.0.0.1:8088`). Instead of packing multi-hundred-MB `.data` bundles or cloning an external web repo, the browser mounts your local Paracraft directory directly via `?searchroot=...&searchpath=...`.

**From Windows (builds in WSL ext4 `~/build/npl_unified_wasm` for 10x speed, serves natively on Windows):**
```powershell
# Compile C++ -> WASM only
.\build_win32.bat web build

# Faster WASM build without MuJoCo + ONNX Runtime robotics modules
.\scripts\dev_web.ps1 build -NoRobot

# Start (or reuse) local server on 127.0.0.1:8088 and open Project 530 in browser
.\build_win32.bat web run -SearchRoot "C:/lxzsrc/ParaEngine/ParaWorld/" -SearchPath "script/" -ProjectId "530"

# Build + Serve + Open browser in one command
.\scripts\dev_web.ps1 all -SearchRoot "C:/lxzsrc/ParaEngine/ParaWorld/" -SearchPath "script/" -ProjectId "530" -OpenBrowser
```

**From Linux / macOS / WSL:**
```bash
./build_linux.sh web build                                            # Full WASM build
WEB_ROBOT=OFF ./build_linux.sh web build                              # Faster build without robotics
SEARCHROOT=/path/to/ParaWorld ./build_linux.sh web serve              # Serve on http://127.0.0.1:8088
SEARCHROOT=/path/to/ParaWorld ./build_linux.sh web serve --https      # HTTPS for WebXR / LAN headsets
```

**Browser Test URLs:**
- Keepwork local dev (Project 530): `http://127.0.0.1:8088/npl/webparacraft/keepwork/local.html?searchroot=C:/lxzsrc/ParaEngine/ParaWorld/&searchpath=script/&pid=530`
- Root shell (local world): `http://127.0.0.1:8088/npl/webparacraft/index.html?searchroot=C:/Paracraft/&world=worlds/DesignHouse/_emptyworld`

See [`docs/web-dev-workflow.md`](docs/web-dev-workflow.md) for Boost Emscripten setup and MEMFS lazy-loading details.

---

### 4. Linux & macOS Desktop GUI Clients

**Linux (SDL2 + OpenGL):**
```bash
# Using system SDL2 (apt install libsdl2-dev)
./build_linux.sh client

# Or download & build SDL2 2.0.22 locally into build-sdl2-deps/ + build ParaCraftSDL2
./build_linux.sh client --build-sdl2
```

**macOS (Cocoa + OpenGL / Xcode):**
```bash
# Command-line Makefile/Ninja client build into build-client/
./build_linux.sh client

# Or generate an Xcode project in build-client/
./build_linux.sh client --xcode
```

To keep the macOS Documents permission across local desktop rebuilds, use the
same Keychain signing identity for Debug and Release. Put its 40-character
certificate SHA-1 fingerprint on one line in
`~/.config/paracraft/macos-signing-identity`. New CMake build trees pick it
up for Make, Ninja, and Xcode. For an existing tree, configure with
`-DPARACRAFT_MACOS_SIGN_IDENTITY=<fingerprint>`. An explicit empty value
keeps the original Xcode team signing or command-line ad hoc signing.

---

### 5. Mobile & Standalone XR (Android, PICO VR, iOS, HarmonyOS)

On Apple Silicon, use the [Android and iOS emulator guide](docs/mobile-emulator-testing.md) to build the ARM64 APK or standalone simulator app, package Paracraft assets, and verify input, rendering and resume. A fresh setup also needs the [target-specific dependency recipes](docs/mobile-dependencies.md).

- **Android (`NPLRuntime/Platform/AndroidStudio`):**
  ```bash
  # Export BOOST_ROOT pointing to the Boost source tree with Android libraries.
  ./build_linux.sh android assembleDebug      # Linux/macOS
  build_win32.bat android assembleDebug       # Windows
  ```
- **PICO VR Native OpenXR (`NPLRuntime/Platform/PicoVR` — PICO 4 / PICO 4 Ultra / Neo3):**
  ```bash
  # Configure android.boost.root=/path/to/boost in NPLRuntime/Platform/PicoVR/local.properties
  ./build_linux.sh picovr :PicoVR:assembleDebug
  adb install -r NPLRuntime/Platform/PicoVR/app/build/outputs/apk/debug/paracraft-pico-debug-*.apk
  ```
  See [`NPLRuntime/Platform/PicoVR/README.md`](NPLRuntime/Platform/PicoVR/README.md) for OpenXR details.
- **iOS (`macOS` + `Xcode`):**
  ```bash
  ./build_linux.sh ios framework    # Builds ParacraftFramework.framework (Release, arm64)
  ./build_linux.sh ios device       # Generates Xcode device project in build-ios-device/
  ./build_linux.sh ios sim          # Generates Xcode simulator project in build-ios-sim/
  ```
- **HarmonyOS NEXT (`NPLRuntime/Platform/HarmonyOS`):**
  ```bash
  cd NPLRuntime/Platform/HarmonyOS
  npm install
  npm run build:debug         # or npm run build:release (DevEco Studio 5.0+ / SDK API 12)
  ```
  See [`NPLRuntime/Platform/HarmonyOS/README.md`](NPLRuntime/Platform/HarmonyOS/README.md).

---

## VS Code Tasks & Debugging

Configured in [`.vscode/tasks.json`](.vscode/tasks.json) and [`.vscode/launch.json`](.vscode/launch.json):

| VS Code Task Label | Purpose |
| :--- | :--- |
| **`Build ParaEngineClient (Debug)`** *(Default `Ctrl+Shift+B`)* | Incremental Windows `Debug` build in `build-win64` (`ParaEngineClient_d.exe`) |
| **`Build ParaEngineClient (Release)`** | Incremental Windows `Release` build in `build-win64` (`ParaEngineClient.exe`) |
| **`Windows: Configure ParaEngineClient (CMake)`** | Configures `build-win64` (`x64`, auto-detecting `BOOST_ROOT`) |
| **`Build ParaEngineServer (Release)`** | Builds headless `ParaEngineServer.exe` in `build-server` |
| **`Windows: Configure ParaEngineServer (CMake)`** | Configures `build-server` (`-DNPLRUNTIME_SERVER=ON`) |
| **`Web: Build and Run Project 530`** | Builds WASM via WSL -> starts/reuses local server on `:8088` -> opens Project 530 |
| **`Web: Build WASM (Emscripten)`** | Incremental C++ -> WASM build into `web/auto_generate_files/` |
| **`Web: Run Project 530 (Local Disk)`** | Starts local server on `:8088` and opens Project 530 in browser |

Press **`F5`** in VS Code with **`Debug ParaEngineClient`**, **`Release ParaEngineClient`**, or **`Web: Launch Project 530 (Chrome)`** selected to build and debug directly.

---

## Key CMake Options (`NPLRuntime/CMakeLists.txt`)

| Option | Default | Description |
| :--- | :--- | :--- |
| `NPLRUNTIME_SERVER` | `OFF` | Build headless `ParaEngineServer` (`ON` forces `NPLRUNTIME_RENDERER=NULL`) |
| `NPLRUNTIME_RENDERER` | `OPENGL` | Primary renderer family (`OPENGL`, `DIRECTX`, `NULL`) |
| `NPLRUNTIME_ENABLE_OPENGL` | `ON` (client) | Enable OpenGL / GLES / WebGL2 ParaRHI backend |
| `NPLRUNTIME_ENABLE_D3D11` | `ON` (Win32) | Enable Direct3D 11 ParaRHI backend |
| `NPLRUNTIME_ENABLE_D3D12` | `ON` (Win32) | Enable Direct3D 12 ParaRHI backend |
| `NPLRUNTIME_ENABLE_VULKAN` | `ON` (Win32) | Enable Vulkan ParaRHI backend |
| `NPLRUNTIME_ENABLE_METAL` | `ON` (Apple) | Enable Metal ParaRHI backend |
| `NPLRUNTIME_ENABLE_WEBGPU` | `ON` (Win32/Web) | Enable WebGPU ParaRHI backend |
| `NPLRUNTIME_LUAJIT21` / `LUAJIT20` / `LUA51` | Platform-specific | Select Lua VM implementation (only enable one) |
| `NPLRUNTIME_PHYSICS` | `ON` (client) | Build Bullet Physics (`PhysicsBT`) plugin |
| `NPLRUNTIME_AUDIO` | `ON` (client) | Build OpenAL (`cAudioEngine`) audio plugin |
| `NPLRUNTIME_WEB_ROBOT` | `ON` (Web) | Include MuJoCo + ONNX Runtime robotics support in WASM builds |
| `NPLRUNTIME_BUILD_TESTS` | `ON` | Enable ParaRHI & NPLRuntime CTest suite |
| `EMSCRIPTEN_SINGLE_THREAD` | `ON` (dev_web) | Single-thread Emscripten WASM build (`ParaCraftSingleThread`) |

---

## Minimal NPL Example

```lua
-- helloworld.lua: asynchronous actor activation in NPL (100% Lua 5.1 syntax compatible)
NPL.activate("(gl)helloworld.lua", { data = "Hello from NPLRuntime!" })

NPL.this(function()
    if msg then
        print(msg.data)
    end
end)
```

---

## Repository Map & Documentation

```text
NPLRuntimeUnified/
├── NPLRuntime/                  # Main CMake project root
│   ├── ParaEngine/              # Core 3D/2D engine, BlockEngine, NPL runtime, Lua bindings
│   ├── RenderSystem/            # ParaRHI backends (OpenGL, D3D11, D3D12, Vulkan, Metal, WebGPU, Null)
│   ├── Platform/                # OS hosts (Windows, SDL, Linux, OSX, AndroidStudio, PicoVR, iOS, HarmonyOS)
│   ├── Plugins/                 # PhysicsBT, cAudioEngine, ParaSqlite, NPLMono2, NplOce
│   ├── externals/               # Vendored third-party libraries
│   └── tests/                   # NPL scripts & ParaRHI CTest verification suite
├── web/                         # Self-contained WebParacraft shell + local_disk_fs.js MEMFS driver
├── scripts/                     # dev_web.ps1, dev_web.sh, serve_webparacraft.py
├── tests/windows/               # One-click Windows multi-renderer launchers (Project 530)
├── build_win32.bat              # Windows unified CLI build & run entry point
└── build_linux.sh               # Linux / macOS / WSL unified CLI build & run entry point
```

- **Agent & Contributor Guides:** [`AGENTS.md`](AGENTS.md) · [`CLAUDE.md`](CLAUDE.md)
- **Architecture & Subsystems:** [`docs/README.md`](docs/README.md) · [`docs/architecture.md`](docs/architecture.md) · [`docs/build-system.md`](docs/build-system.md) · [`docs/render-system.md`](docs/render-system.md)
- **WebAssembly Local-Disk Workflow:** [`docs/web-dev-workflow.md`](docs/web-dev-workflow.md)
- **Windows Multi-Backend Launchers:** [`tests/windows/README.md`](tests/windows/README.md)
- **License:** [MIT](LICENSE)
