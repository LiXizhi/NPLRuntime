# CLAUDE.md — NPLRuntime / ParaEngine

Guidance for AI assistants working in this repository.

## What This Project Is

**NPLRuntime** is the C++ runtime for **NPL (Neural Parallel Language)** — a high-performance scripting language with **100% Lua 5.1 syntax compatibility**. It powers **ParaEngine**, a cross-platform 3D/2D game engine used by [Paracraft](http://www.paracraft.cn) and related applications.

| Aspect | Detail |
|--------|--------|
| Language | C++11–20 (platform-dependent) |
| Scripting | Lua 5.1 / LuaJIT 2.0 / LuaJIT 2.1 |
| Platforms | Windows, Linux, macOS, iOS, Android, Emscripten, HarmonyOS |
| Build | CMake 3.15+, Boost required |
| Outputs | Client (`ParaEngineClient`, `ParaCraftSDL2`, mobile apps) or Server (`ParaEngineServer`, `npls`) |

## Repository Layout

```
NPLRuntimeCPOld/
├── NPLRuntime/          # Main CMake project (start here)
│   ├── ParaEngine/      # Core engine static library
│   ├── RenderSystem/    # OpenGL / D3D9 / Null render backends
│   ├── Platform/        # OS entry points (WinMain, main, JNI, NAPI, etc.)
│   ├── Plugins/         # Physics, audio, SQLite, Mono, OCE
│   ├── externals/       # Vendored third-party deps (avoid editing)
│   └── cmake/           # CMake helpers
├── Server/trunk/        # Legacy standalone server tree (prefer NPLRuntime + NPLRUNTIME_SERVER)
├── ParaWorld/           # Build output (bin32 / bin64) — created at build time
├── docs/                # Architecture and component documentation
├── build_*.bat / .sh    # Platform build wrappers
├── README.md            # User-facing overview
└── INSTALL.md           # Install and build guide
```

## Architecture at a Glance

```
Platform entry (WinMain / main / JNI)
    └── CParaEngineAppBase          # App lifecycle, frame loop
            ├── CNPLRuntime         # Message-passing script runtime
            ├── CParaWorldAsset     # Asset manager (textures, meshes, fonts…)
            ├── 3dengine / 2dengine # Scene graph + GUI
            └── renderer            # Draw calls → RenderSystem
                    └── RenderDeviceOpenGL | RenderDeviceD3D9 | RenderDeviceNull
```

**Client vs server** is controlled by CMake option `NPLRUNTIME_SERVER=ON`, which forces `NPLRUNTIME_RENDERER=NULL` and builds `ParaEngineServer` instead of a GUI client.

## Key Files (Read These First)

| File | Purpose |
|------|---------|
| `NPLRuntime/CMakeLists.txt` | Root build config, renderer/Lua/platform options |
| `NPLRuntime/ParaEngine/NPL/NPLRuntime.h` | NPL runtime singleton API |
| `NPLRuntime/ParaEngine/NPL/NPLRuntimeState.h` | Per-thread message-driven Lua state |
| `NPLRuntime/ParaEngine/Core/ParaEngineAppBase.h` | Shared app base (Init, FrameMove, device lifecycle) |
| `NPLRuntime/ParaEngine/Core/ParaWorldAsset.h` | Central asset manager |
| `NPLRuntime/ParaEngine/ParaScriptBindings/ParaScriptingGlobal.cpp` | Lua global API exports |
| `NPLRuntime/Platform/CMakeLists.txt` | Platform target selection |
| `NPLRuntime/ParaEngine/CMakeLists.txt` | ParaEngine library composition |

Full documentation: [docs/README.md](docs/README.md)

## Build Commands

### Windows client (OpenGL, VS2022)

```bat
build_win32.bat
```

Or manually:

```bat
cmake -S NPLRuntime -B build -DNPLRUNTIME_RENDERER=OPENGL
cmake --build build --config Release
```

Output: `ParaWorld/bin64/ParaEngineClient.exe` (or `bin32` on 32-bit builds).

### Linux server (headless)

```bash
./build_linux.sh
# equivalent:
cmake -S NPLRuntime -B build -DNPLRUNTIME_SERVER=ON
cmake --build build
```

Output: `ParaWorld/bin64/ParaEngineServer`

### Prerequisites

- **Boost** 1.55–1.85 (`BOOST_ROOT` env var or auto-download in `build_win32.bat`)
- **CMake** ≥ 3.15
- Client on Windows: DirectX9 SDK optional (OpenGL is default)
- Linux graphics: SDL2 (`build_sdl2.sh`)

See [INSTALL.md](INSTALL.md) and [docs/build-system.md](docs/build-system.md).

## NPL Scripting Model

NPL uses **activation** — asynchronous message passing between runtime states (threads/processes):

```lua
NPL.activate("(gl)helloworld.npl", {data="hello world!"})
this(msg) {
   if msg then print(msg.data or "") end
}
```

Activation prefixes: `(gl)` main thread, `(worker1)` named worker, `(ip:port)` remote process.

C++ bindings live in `NPLRuntime/ParaEngine/ParaScriptBindings/` (luabind-based). **Strict Lua 5.1 semantics** — no `goto`, no `_ENV`.

## Coding Conventions

- **Class prefix:** `CMyClass` (classes), `IMyInterface` (interfaces), `*Entity` (assets)
- **No src/include split** — `.h` and `.cpp` colocate in feature directories
- **Precompiled header:** include `ParaEngine.h` first in `.cpp` files
- **Smart pointers:** `asset_ptr<T>` / `ref_ptr<T>` (boost intrusive_ptr) for assets and ref-counted objects
- **Paths:** never hard-code; use `CFileUtils`, `CParaFile::OpenAssetFile()`, `CPathReplaceables`
- **Rendering:** use `CGlobals::GetRenderDevice()` — no raw D3D/OpenGL outside `RenderSystem/`
- **Threading:** guard Boost.Thread code with `#ifndef EMSCRIPTEN_SINGLE_THREAD`

## Common CMake Options

| Option | Default (client) | Notes |
|--------|------------------|-------|
| `NPLRUNTIME_SERVER` | OFF | Headless server build |
| `NPLRUNTIME_RENDERER` | OPENGL | OPENGL / DIRECTX / NULL |
| `NPLRUNTIME_STATIC_LIB` | OFF | Static vs shared linking |
| `PARAENGINE_CLIENT_DLL` | OFF | Build client as DLL |
| `NPLRUNTIME_LUAJIT21` | ON | LuaJIT 2.1 (GC64) |
| `NPLRUNTIME_PHYSICS` | ON (client) | Bullet plugin |
| `NPLRUNTIME_AUDIO` | ON (client) | cAudioEngine plugin |
| `NPLRUNTIME_SUPPORT_FBX` | ON (client) | assimp model loading |
| `EMSCRIPTEN_SINGLE_THREAD` | — | Web single-thread build |

## What to Avoid

- Editing `externals/` unless upgrading a dependency intentionally
- Breaking Lua 5.1 compatibility
- `using namespace std;` in headers
- Blocking I/O in NPL message handlers (blocks worker threads)
- `new`/`delete` in hot paths (use pools — see `util/ParaMemPool.h`)
- Direct GPU API calls outside `RenderSystem/`

## External Resources

- Wiki: https://github.com/LiXizhi/NPLRuntime/wiki/
- Docs site: http://docs.paraengine.com/
- Paracraft SDK examples: https://github.com/LiXizhi/ParaCraftSDK

## When Making Changes

1. Match surrounding code style and naming in the target subsystem folder.
2. For new Lua APIs: add C++ in `ParaScriptBindings/`, bind with luabind, export in `ParaScriptingGlobal.cpp`.
3. For platform-specific code: use macros (`USE_OPENGL_RENDERER`, `PARAENGINE_MOBILE`, `EMSCRIPTEN`, etc.).
4. Test at minimum on Windows (client) or Linux (server) before submitting.
5. Keep diffs focused — this is a large mature codebase (~2005–present).
