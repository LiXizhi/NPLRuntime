# Build System

CMake-based build configuration for NPLRuntime. All builds start from `NPLRuntime/CMakeLists.txt`.

## Requirements

| Tool | Version | Notes |
|------|---------|-------|
| CMake | ≥ 3.15 | 3.28+ recommended for Emscripten |
| C++ compiler | C++11–20 | Platform-dependent (see below) |
| Boost | 1.55–1.85 | thread, filesystem, system, chrono, serialization, iostreams, locale |
| Lua VM | LuaJIT 2.1 (default) | Or LuaJIT 2.0 / Lua 5.1 via CMake options |

### Platform-specific

| Platform | Extra requirements |
|----------|-------------------|
| Windows client (DirectX) | DirectX 9 SDK |
| Windows / Linux client (OpenGL) | OpenGL 3.3+ drivers |
| Linux SDL | SDL2 (`build_sdl2.sh`) |
| Emscripten | emsdk, Boost compiled with `toolset=emscripten` |
| Android | NDK, Boost for Android |
| iOS / macOS | Xcode, Boost for iOS/macOS |

Set `BOOST_ROOT` to your Boost installation root if not auto-detected.

## C++ Standard by Platform

Defined in `NPLRuntime/ParaEngine/CMakeLists.txt`:

| Platform | C++ Standard |
|----------|--------------|
| Windows, Android | C++17 |
| Linux, macOS, iOS, Emscripten | C++20 |
| Emscripten single-thread | C++20 |

Root `CMakeLists.txt` sets C++11 as baseline; ParaEngine overrides per platform.

## CMake Subdirectory Order

```cmake
add_subdirectory(externals)
add_subdirectory(Plugins)
add_subdirectory(ParaEngine)
add_subdirectory(RenderSystem)
add_subdirectory(Platform)
# + ParaEngineClientApp if PARAENGINE_CLIENT_DLL
```

## Primary CMake Options

### Build mode

| Option | Default | Description |
|--------|---------|-------------|
| `NPLRUNTIME_SERVER` | OFF | Headless server; forces `RENDERER=NULL` |
| `NPLRUNTIME_STATIC_LIB` | OFF (client) / OFF (server) | Static linking for main binary |
| `PARAENGINE_CLIENT_DLL` | OFF | Windows: build `WindowsApplication` as DLL |

### Renderer

| Option | Values | Set by |
|--------|--------|--------|
| `NPLRUNTIME_RENDERER` | `OPENGL`, `DIRECTX`, `NULL` | Platform + `NPLRUNTIME_SERVER` |

Platform defaults (`NPLRuntime/CMakeLists.txt`):

- **Server** → `NULL`
- **Windows** → `OPENGL` (DirectX also available)
- **Linux / macOS / iOS / Android / SDL / Emscripten** → `OPENGL`
- **Unknown** → `NULL`

### Lua VM

| Option | Description |
|--------|-------------|
| `NPLRUNTIME_LUAJIT21` | LuaJIT 2.1 with GC64 (default on server) |
| `NPLRUNTIME_LUAJIT20` | LuaJIT 2.0.4 |
| `NPLRUNTIME_LUA51` | Vanilla Lua 5.1.5 |

Only one should be ON.

### Features

| Option | Client default | Server default |
|--------|----------------|----------------|
| `NPLRUNTIME_PHYSICS` | ON | OFF |
| `NPLRUNTIME_AUDIO` | ON | OFF |
| `NPLRUNTIME_SUPPORT_FBX` | ON | OFF |
| `NPLRUNTIME_NPLMono2` | platform-dependent | OFF |
| `NPLRUNTIME_OCE` | OFF | OFF |
| `NPLRUNTIME_DARKNET` | OFF | OFF |
| `NPLRUNTIME_ICONV` | ON | ON |
| `NPLRUNTIME_HAQI_LAUNCHER` | Windows only | — |
| `NPLRUNTIME_WEBVIEW` | Windows only | — |

### Platform flags

| Flag | Effect |
|------|--------|
| `SDL` | SDL2 desktop client (`ParaCraftSDL2`) |
| `SDL_SINGLE_THREAD` | Single-threaded SDL build |
| `EMSCRIPTEN` | WebAssembly build |
| `EMSCRIPTEN_SINGLE_THREAD` | No pthread pool; uses coroutines |
| `ANDROID_STUDIO` | Android Studio Gradle project |
| `BUILD_FRAMEWORK` | iOS framework output |
| `MAC_SERVER` | macOS headless server |
| `LOCAL_DEBUG` | Developer-local paths (SDL root, Boost root) |

## CMake Targets

| Target | Condition | Output |
|--------|-----------|--------|
| `ParaEngine` | Always | Static library — core engine |
| `RenderSystemOpenGL` | `RENDERER=OPENGL` | OpenGL backend |
| `RenderSystemD3D9` | `RENDERER=DIRECTX` | DirectX 9 backend |
| `RenderSystemNull` | `RENDERER=NULL` | Headless stub |
| `WindowsApplication` | `WIN32` + client | `ParaEngineClient.exe` |
| `ParaEngineServer` | `NPLRUNTIME_SERVER` or `MAC_SERVER` | Headless server |
| `ParaCraftSDL2` | `SDL` or `LINUX` | SDL desktop client |
| `ParaCraft` | Emscripten multi-thread | `.html` + `.wasm` |
| `ParaCraftSingleThread` | Emscripten + `EMSCRIPTEN_SINGLE_THREAD` | Single-thread web |
| iOS / Android / HarmonyOS targets | Mobile flags | Platform-specific |
| `PhysicsBT` | `NPLRUNTIME_PHYSICS` | Bullet physics |
| `cAudioEngine` | `NPLRUNTIME_AUDIO` | OpenAL audio |
| `ParaSqlite` | Always | Embedded SQLite |
| `NPLMono2` | `NPLRUNTIME_NPLMono2` | .NET interop |

## Output Paths

Binaries are copied post-build to:

```
ParaWorld/bin32/    # 32-bit builds
ParaWorld/bin64/    # 64-bit builds
```

Debug builds append `_d` suffix to DLLs (e.g. `sqlite_d.dll`).

Linux server install script may symlink `/usr/local/bin/npl` → `ParaWorld/bin64/ParaEngineServer`.

## Root Build Scripts

| Script | Platform | Command summary |
|--------|----------|-----------------|
| `build_win32.bat` | Windows | VS2022 x64, `-DNPLRUNTIME_RENDERER=OPENGL`; auto-installs CMake/Boost |
| `build_linux.sh` | Linux | `-DNPLRUNTIME_SERVER=ON` → `ParaEngineServer` |
| `build_mac_server.sh` | macOS | Server: `NPLRUNTIME_SERVER=ON`, `MAC_SERVER=ON` |
| `build_mac_client.sh` | macOS | OpenGL client, Xcode generator |
| `build_emscripten.sh` | Web | Emscripten ParaCraft build |
| `build_ios_framework.sh` | iOS | Framework packaging |
| `build_ios_os.sh` | iOS | Device/simulator Xcode project |
| `build_sdl2.sh` | Linux/Win | Compile and install SDL2 |
| `npl_boost_install.sh` | Linux/macOS | Boost build helper |
| `build_android_tegra.bat` | Android | VS + Nsight cross-compile |

## Common Build Recipes

### Windows OpenGL client

```bat
build_win32.bat
```

Or:

```bat
cmake -S NPLRuntime -B build -G "Visual Studio 17 2022" -A x64 -DNPLRUNTIME_RENDERER=OPENGL
cmake --build build --config Release
```

### Linux headless server

```bash
./build_linux.sh
# or with parallel jobs:
./build_linux.sh 6
```

### Linux SDL client

```bash
source build_sdl2.sh
./npl_boost_install.sh
cmake -S NPLRuntime -B out -DLINUX=TRUE -DSDL_ROOT=${SDL_ROOT}
cmake --build out
# Run: ParaWorld/bin64/ParaCraftSDL2
```

### macOS server

```bash
./npl_boost_install.sh
./build_mac_server.sh 6
```

### Emscripten (WebAssembly)

```bash
# Requires emsdk + Boost built with toolset=emscripten
emcmake cmake -S NPLRuntime -B build/emscripten \
  -DEMSCRIPTEN=ON \
  -DEMSCRIPTEN_SINGLE_THREAD=ON \
  -DBOOST_ROOT=/path/to/boost \
  -DAPP_ROOT=/path/to/paracraft \
  -DHTML_ROOT=/path/to/webroot
cd build/emscripten && emmake make -j8
```

Output: `build/emscripten/bin/ParaCraft.html`, `.js`, `.wasm`, `worker.js`.

Web deployment requires COOP/COEP headers for SharedArrayBuffer (multithreaded builds). See [INSTALL.md](../INSTALL.md#build-with-emscripten-webasm--web-gl).

### Static server library

```bash
cmake -S NPLRuntime -B build \
  -DNPLRUNTIME_SERVER=ON \
  -DNPLRUNTIME_STATIC_LIB=ON
cmake --build build
```

## Platform Selection Logic

`NPLRuntime/Platform/CMakeLists.txt` routes to one platform subdirectory:

```
NPLRUNTIME_SERVER     → Platform/Linux/        (ParaEngineServer)
SDL or EMSCRIPTEN     → Platform/SDL/
ANDROID + ANDROID_STUDIO → Platform/AndroidStudio/
ANDROID               → Platform/Android/
IOS                   → Platform/iOS/
WIN32                 → Platform/Windows/
MAC_SERVER            → Platform/Linux/
APPLE                 → Platform/OSX/
LINUX                 → Platform/SDL/
else                  → Platform/Linux/
```

## RenderSystem Selection

`NPLRuntime/RenderSystem/CMakeLists.txt`:

```
NPLRUNTIME_RENDERER=OPENGL  → opengl/
NPLRUNTIME_RENDERER=DIRECTX → d3d9/
NPLRUNTIME_RENDERER=NULL    → null/
```

ParaEngine links against the selected RenderSystem target. DirectX-only code in `ParaEngine/Engine/` is compiled only when `USE_DIRECTX_RENDERER` is defined.

## Boost on Emscripten

Boost must be compiled with Emscripten's toolchain:

```bash
./bootstrap.sh
./b2 toolset=emscripten runtime-link=static cxxflags='-fexceptions' \
  --with-thread --with-filesystem --with-system --with-chrono \
  --with-serialization --with-iostreams --with-locale -j4
```

Convert `.bc` to `.a` if needed: `emar -q libname.a libname.bc`

Alternatively use [boost-cmake](https://github.com/boostorg/boost/releases) with `emcmake cmake`.

## Debugging from Source

1. Build Debug configuration.
2. Set working directory to an NPL app root (e.g. ParacraftSDK `redist/`).
3. Run `ParaEngineClient_d.exe` from `ParaWorld/bin/`.
4. Copy dependent `*_d.dll` files if not using ParaWorld/bin as cwd.

Visual Studio: set debugging working directory on the `WindowsApplication` project.

## CI References

- Travis CI (Linux): `.travis.yml`
- AppVeyor (Windows): `appveyor.yml`

## Related

- [INSTALL.md](../INSTALL.md) — user-facing install guide
- [architecture.md](architecture.md) — what each target does at runtime
- [platform.md](platform.md) — platform entry point details
