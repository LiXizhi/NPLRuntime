# Build & Deploy

NPLRuntime uses **CMake** as its build system with platform-specific scripts and CI integration.

## Build Entry Points

| Product | CMake Root | Script | Output |
|---------|-----------|--------|--------|
| Server (headless) | `NPLRuntime/CMakeLists.txt` | `build_linux.sh` | `ParaWorld/bin64/ParaEngineServer` |
| Client (GUI) | `Client/CMakeLists.txt` | `build_win32.bat` | `ParaWorld/bin32\|64\ParaEngineClient` |
| macOS client | `Client/CMakeLists.txt` | `build_mac_client.sh` | macOS app bundle |
| macOS server | `NPLRuntime/CMakeLists.txt` | `build_mac_server.sh` | `ParaWorld/bin64/ParaEngineServer` |

## Quick Start

### Linux Server

```bash
git clone https://github.com/LiXizhi/NPLRuntime.git
cd NPLRuntime
./build_linux.sh
# Binary: ParaWorld/bin64/ParaEngineServer
# Symlink: /usr/local/bin/npl
```

Dependencies (Debian/Ubuntu):
```bash
apt-get install cmake build-essential libboost-all-dev libssl-dev \
    libssh2-1-dev freeglut3-dev libglew-dev libglu1-mesa-dev libgl1-mesa-dev
```

### Windows Client

```batch
git clone https://github.com/LiXizhi/NPLRuntime.git
cd NPLRuntime
build_win32.bat
```

Requirements:
- Visual Studio (2015+)
- DirectX SDK (June 2010) — auto-installed by script
- Boost 1.55+ built with matching MSVC toolchain

### macOS

```bash
./build_mac_client.sh   # GUI client with OpenGL
./build_mac_server.sh   # Headless server
```

Requirements: Xcode, Boost, LuaJIT 2.1

## CMake Options

### Server (`NPLRuntime/CMakeLists.txt`)

| Option | Default | Description |
|--------|---------|-------------|
| `NPL_STATIC_LINK_ALL` | ON | Static link Boost, sqlite, curl (single deployable binary) |
| `NPLRUNTIME_LUASQL` | OFF | Build luasql plugin |

### Client (`Client/CMakeLists.txt`)

| Option | Default | Description |
|--------|---------|-------------|
| `NPLRUNTIME_RENDERER` | DIRECTX (Win) / OPENGL (Mac) | Graphics backend |
| `PARAENGINE_CLIENT_DLL` | varies | Build as DLL vs standalone EXE |
| `NPLRUNTIME_STATIC_LIB` | OFF | Single static binary vs plugin DLLs |
| `NPLRUNTIME_PHYSICS` | OFF | Enable Bullet3 physics plugin |
| `NPLRUNTIME_SUPPORT_FBX` | OFF | Enable Assimp FBX/OBJ import |
| `NPLRUNTIME_LUAJIT20` | OFF | Use LuaJIT 2.0 |
| `NPLRUNTIME_LUAJIT21` | varies | Use LuaJIT 2.1 |
| `NPLRUNTIME_LUA51` | varies | Use Lua 5.1.4 |
| `NPLRUNTIME_CAUDIOENGINE` | OFF | Enable cAudio engine |
| `NPLRUNTIME_DARKNET` | OFF | Enable Darknet (YOLO) plugin |
| `NPLRUNTIME_WEBVIEW` | OFF | Enable embedded webview |

## Bundled Dependencies

### Server (`Server/trunk/`)

Built as CMake subdirectories:

| Library | Version | Purpose |
|---------|---------|---------|
| lua-5.1.4 | 5.1.4 | Lua interpreter (+ NPL compiler) |
| LuaJIT | 2.0.4 | JIT Lua |
| LuaJIT-2.1 | 2.1 | JIT Lua (GC64) |
| luabind | 0.9 / 0.9.2beta | C++↔Lua bindings |
| boost | 1.61 / 1.65 | Threading, filesystem, regex, etc. |
| curl | 7.47.1 | HTTP client (Windows bundled) |
| zlib | 1.2.3 / 1.2.11 | Compression |
| sqlite | 3.6.23.1 | Embedded database |
| jsoncpp | 0.5.0 | JSON parsing |
| tinyxpath | 1.3.1 | XML/XPath |
| pcl/common, pcl/features | — | Point cloud (AutoRigger) |
| Pinocchio | — | Skeleton auto-rigging |

### Client-only (`Client/trunk/externals/`)

| Library | Purpose |
|---------|---------|
| assimp 5.0.1 | 3D model import (FBX, OBJ, glTF) |
| bullet3 | Physics engine |
| glew 2.1.0 | OpenGL extension loading |
| glfw 3.2.1 | Window creation (OpenGL) |
| FreeImage 3.12.0 | Image loading |
| antlr | Parser generator (CadModel) |
| darknet | YOLO object detection (optional) |

## Building Boost

Boost must be compiled with the **same compiler** used for NPLRuntime:

```bash
cd Server/trunk/boost_1_61_0
./bootstrap.sh
./b2 link=static threading=multi variant=release \
    --with-thread --with-date_time --with-filesystem \
    --with-system --with-chrono --with-serialization \
    --with-iostreams --with-regex
```

Output: `stage/lib64/` or `stage/lib32/`

Set `BOOST_ROOT` environment variable if using a non-default location.

## CMake Targets

### Server Build

```
NPLRuntime (project)
├── lua-5.1.4
├── LuaJIT
├── LuaJIT-2.1
├── luabind-0.9
├── sqlite-3.6.23.1
├── jsoncpp-0.5.0
├── tinyxpath_1_3_1
├── pcl/common, pcl/features
├── Pinocchio
├── ParaEngineServer          ← main executable
├── NPLMono2                  ← plugin
└── NPLRouter                 ← plugin
```

### Client Build

```
CLIENT (project)
├── [all server deps]
├── ParaEngineClient          ← main target (exe or dll)
├── ParaEngineClientApp       ← test harness (loads DLL)
├── PhysicsBT                 ← optional plugin
├── cAudio                    ← optional plugin
└── HaqiLauncher              ← optional launcher
```

## Output Layout

```
ParaWorld/
├── bin32/
│   ├── ParaEngineClient.exe
│   ├── ParaEngineClient.dll  (if PARAENGINE_CLIENT_DLL)
│   ├── ParaEngineServer.exe
│   ├── NPLMono2.dll
│   ├── NPLRouter.dll
│   ├── PhysicsBT.dll           (optional)
│   └── [shaders, scripts, assets in full deployment]
└── bin64/
    ├── ParaEngineClient
    ├── ParaEngineServer
    └── [plugins .so/.dylib]
```

## CI/CD

### AppVeyor (Windows)
- Config: `appveyor.yml`
- Builds Windows client
- Badge in README

### Travis CI (Linux)
- Config: `.travis.yml`
- Builds Linux server via `build_linux.sh`
- Installs: Boost, mono, OpenGL, curl, etc.

## Deployment

### Server Deployment

1. Build with `NPL_STATIC_LINK_ALL=ON` for a single binary
2. Copy `ParaEngineServer` to target machine
3. Deploy NPL scripts alongside binary
4. Optional: install plugins (`NPLMono2.so`, `NPLRouter.so`)
5. Linux: binary is relocatable (RPATH `$ORIGIN/`)

```bash
# Run as service
npls -d bootstrapper="script/apps/myapp/bootstrapper.xml"

# Run as interpreter
npls -i script/hello.npl
```

### Client Deployment

1. Build client with desired options
2. Copy entire `ParaWorld/bin32/` or `bin64/` directory
3. Include shaders, bootstrapper XML, and script packages
4. Windows: NSIS installers in `installer/` (`nplruntime_x86.nsi`, `nplruntime_x64.nsi`)

### Script Packages

Scripts are deployed separately from the binary:
- `npl_packages/` — git submodule for script packages (may be empty in dev checkout)
- Application scripts in `script/apps/` directories
- Bootstrapper XML specifies which app to run

## Platform-Specific Notes

### Windows
- Static MSVC runtime linking (`/MT`)
- DirectX SDK June 2010 required for client
- Drive letters stripped from file paths for portability
- CMake generator: Visual Studio (e.g., `-G "Visual Studio 16 2019"`)

### Linux
- System curl (not bundled)
- RPATH `$ORIGIN/` for relocatable binaries
- Symlink: `ParaEngineServer` → `/usr/local/bin/npl`
- CMake generator: Unix Makefiles

### macOS
- OpenGL + GLFW for rendering
- LuaJIT 2.1 + luabind 0.9.2beta required
- App bundle target for client distribution

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Boost link errors | Rebuild Boost with same compiler; check `BOOST_ROOT` |
| curl `curlbuild.h` fuzzy errors | Delete `Server/trunk/curl-7.47.1/include/curl/curlbuild.h` |
| DirectX SDK not found | Run `build_win32.bat` (auto-installs) or install June 2010 SDK |
| Lua version mismatch | Ensure only one Lua implementation is selected via CMake options |
| Missing shaders at runtime | Client: check shader build step; Server: check embed-resource |

## Manual CMake Build

```bash
# Server
mkdir -p bin/linux && cd bin/linux
cmake ../../NPLRuntime -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Client (Windows)
mkdir bin\win32 && cd bin\win32
cmake ..\..\Client -G "Visual Studio 16 2019" -A Win32
cmake --build . --config Release
```
