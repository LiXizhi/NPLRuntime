# CLAUDE.md — NPLRuntime AI Agent Guide

This file orients AI coding agents working in the NPLRuntime repository. For detailed architecture and subsystem docs, see [`docs/`](docs/README.md).

## Project Summary

**NPLRuntime** is the open-source runtime for **ParaEngine**, a cross-platform 3D/2D game engine with headless server support. **NPL (Neural Parallel Language)** is the scripting layer — 100% Lua 5.1 syntax-compatible, with a unique async message-passing activation model.

- **License:** GNU GPL v2
- **Platforms:** Windows, Linux, macOS, Android, iOS
- **Upstream docs:** [GitHub Wiki](https://github.com/LiXizhi/NPLRuntime/wiki/SourceCodeOverview), [docs.paraengine.com](http://docs.paraengine.com)

## Repository Layout

| Path | Purpose |
|------|---------|
| `Client/trunk/ParaEngineClient/` | **Primary C++ source tree** — engine, NPL, renderer, bindings |
| `Client/CMakeLists.txt` | GUI client build (`npl` / `ParaEngineClient`) |
| `NPLRuntime/CMakeLists.txt` | Headless server build (`npls` / `ParaEngineServer`) |
| `Server/trunk/` | Vendored third-party libraries (Lua, Boost, curl, sqlite, etc.) |
| `Client/trunk/externals/` | Client-only deps (Bullet, Assimp, GLFW, GLEW, etc.) |
| `ParaWorld/bin32/` / `bin64/` | Build output directory |
| `docs/` | Architecture and component documentation (this repo) |

**Critical:** Client and server share the same engine sources under `Client/trunk/ParaEngineClient/`. The server CMake project compiles a subset of those files.

## Build & Run

```bash
# Linux server
./build_linux.sh
# Output: ParaWorld/bin64/ParaEngineServer (symlinked as npls)

# Windows client (requires VS, DirectX SDK, Boost)
build_win32.bat
# Output: ParaWorld/bin32|64/ParaEngineClient (npl)
```

CMake options worth knowing:
- `NPLRUNTIME_RENDERER`: `DIRECTX` (Windows) or `OPENGL` (macOS/Linux)
- `NPLRUNTIME_LUAJIT20` / `NPLRUNTIME_LUAJIT21` / `NPLRUNTIME_LUA51`
- `NPLRUNTIME_PHYSICS`, `NPLRUNTIME_SUPPORT_FBX`, `NPL_STATIC_LINK_ALL`

```bash
# Run GUI client
npl [filename] [parameters...]

# Run headless server / interpreter
npls hello.npl
npls -i script.npl          # interpreter mode
npls -d                     # daemon/service mode (Linux)
```

## Entry Points

| Binary | Source | Role |
|--------|--------|------|
| Client (`npl`) | `Engine/ParaWorld.cpp` → `WinMain()` | Full 3D client with rendering |
| Server (`npls`) | `Engine/ParaEngineServer.cpp` → `main()` | Headless NPL runtime |
| macOS client | `platform/mac/mac_main.cpp` | macOS `main()` |
| Headless service | `Core/ParaEngineService.cpp` | Timer-driven NPL loop (no window) |

**Bootstrap flow:** Command line → `CParaEngineApp::Init()` → `CBootStrapper` loads `bootstrapper.xml` → activates main loop script every ~0.5s (default `(gl)script/gameinterface.lua`).

## NPL Activation Model

NPL uses **activation** (async message passing), not traditional function calls:

```lua
NPL.activate("(gl)script/helloworld.npl", {data="hello"})
-- In the target file:
this(msg) {
    print(msg.data)
}
```

- `(gl)` prefix = local file activation
- Remote activation uses NID (network identity) addressing
- Each `.npl`/`.lua` file is a "neuron" with isolated execution context
- **Never share Lua states across threads** — use NPL message passing

Key classes: `CNPLRuntime`, `CNPLRuntimeState`, `CNPLDispatcher`, `CNPLConnection`. See [`docs/npl-runtime.md`](docs/npl-runtime.md).

## Code Conventions

### Namespaces
- `ParaEngine::` — Core engine, 3D, assets
- `NPL::` — Scripting runtime, networking

### Key Base Types
- `CBaseObject` / `IGameObject` — Scene graph nodes (`3dengine/BaseObject.h`)
- `CSceneObject` — Scene root singleton (`3dengine/SceneObject.h`)
- `AssetEntity<T>` — Reference-counted assets (always pair `addref()` / `Release()`)
- `IAttributeFields` — Reflection/attribute system

### C++ Activation Plugins
Native callbacks must follow the naming pattern `NPL_activate_XXXX_cpp`:

```cpp
NPL::NPLReturnCode NPL_activate_mymodule_cpp(INPLRuntimeState* pState) {
    return NPL::NPL_OK;
}
// Activated via: NPL.activate("mymodule.cpp", {data=123})
```

### Directory Map (engine source)

| Directory | Role |
|-----------|------|
| `Core/` | App lifecycle, globals, assets, plugins, bootstrapper |
| `Engine/` | Application entry, game-specific logic |
| `NPL/` | Multi-threaded scripting runtime, networking |
| `3dengine/` | Scene graph, cameras, lights, physics hooks, characters |
| `2dengine/` | GUI widgets, input, IME |
| `renderer/` | DirectX/OpenGL device abstraction |
| `ParaXModel/` | Native 3D format, FBX/glTF, skeletal animation |
| `terrain/` | Global terrain, LOD, texture splatting |
| `BlockEngine/` | Minecraft-style voxel blocks |
| `VoxelMesh/` | Voxel terrain meshing |
| `ParaScriptBindings/` | luabind C++→Lua API |
| `IO/` | Virtual file system, zip archives |
| `protocol/` | Network message formats |

Full component reference: [`docs/components.md`](docs/components.md).

## Script API (Lua Namespaces)

Registered via luabind in `ParaScriptBindings/`:

`NPL`, `ParaScene`, `ParaTerrain`, `ParaCamera`, `ParaAsset`, `ParaWorld`, `ParaBlockWorld`, `ParaSelection`, `ParaUI`, `ParaAudio`, `ParaNetwork`, `ParaEngine`, `ParaBootStrapper`, `ParaIO`, `ParaXML`, `ParaGlobal`, `Config`, and more.

See [`docs/script-bindings.md`](docs/script-bindings.md).

## Renderer

Compile-time backend selection via `NPLRUNTIME_RENDERER`:
- **DirectX 9:** `RenderDeviceDirectX`, `.fx` shaders (Windows)
- **OpenGL:** `RenderDeviceOpenGL`, `.fx.glsl` shaders (Linux/macOS/mobile)

Unified interface: `renderer/RenderDevice.h`. See [`docs/renderer.md`](docs/renderer.md).

## Critical Constraints

1. **Lua 5.1 only** — NPL is not compatible with Lua 5.2+ syntax (no `goto`, no `\z` escapes, etc.)
2. **Boost toolchain match** — Boost must be built with the same compiler used for NPLRuntime (static linking)
3. **Thread safety** — One Lua state per `CNPLRuntimeState`; communicate via NPL activation only
4. **Asset ref counting** — Always release assets after use
5. **Vendored libs** — Do not modify `Server/trunk/*` unless upgrading dependencies

## Common Tasks

### Add a new Lua API
1. Add binding in `ParaScriptBindings/ParaScripting*.cpp` using luabind `module(L) [ namespace_("...") ]`
2. Register loader in `NPLScriptingState` init (e.g. `LoadHAPI_SceneManager()`)

### Add a scene object type
1. Derive from `CBaseObject`, implement `Draw()`, register `CObjectType<T>`
2. Attach via `CGlobals::GetScene()->AttachObject(obj)`

### Add a server-only feature
1. Edit sources under `Client/trunk/ParaEngineClient/`
2. Ensure file is included in `NPLRuntime/ParaEngineServer/CMakeLists.txt` source list

## Documentation Index

| Doc | Contents |
|-----|----------|
| [`docs/README.md`](docs/README.md) | Documentation index |
| [`docs/architecture.md`](docs/architecture.md) | System architecture overview |
| [`docs/npl-runtime.md`](docs/npl-runtime.md) | NPL messaging, runtime states, networking |
| [`docs/scene-graph-and-3d-engine.md`](docs/scene-graph-and-3d-engine.md) | Scene graph, 3D subsystems |
| [`docs/renderer.md`](docs/renderer.md) | Rendering pipeline, shaders |
| [`docs/script-bindings.md`](docs/script-bindings.md) | Lua API reference |
| [`docs/build-and-deploy.md`](docs/build-and-deploy.md) | Build system, CI, deployment |
| [`docs/components.md`](docs/components.md) | Full component directory map |
| [`docs/paraengine-client/`](docs/paraengine-client/README.md) | **Deep dive: Core, 3dengine, GUI, BlockEngine, assets, IO, render pipeline** |

## When in Doubt

- Read existing code in the same subsystem before adding new patterns
- Check `Client/trunk/ParaEngineClient/NPL/NPL_readme.txt` for NPL network internals
- Check `Client/trunk/ParaEngineClient/doc/` for legacy internal docs (events, bootstrapping)
- Prefer minimal, focused diffs that match surrounding style
