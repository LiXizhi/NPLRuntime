# ParaEngine - AI Coding Agent Instructions

## Project Overview

**ParaEngine** is a high-performance, cross-platform 3D game engine with NPL (Neural Parallel Language) scripting support. The engine is written in C++ and provides a complete 3D/2D game development framework with an embedded Lua-compatible scripting runtime.

- **ParaEngine**: The 3D game engine (C++ core in `NPLRuntime/ParaEngine/` folder)
- **NPL**: Neural Parallel Language - 100% Lua-compatible scripting with message-passing concurrency
- **Platforms**: Windows, Linux, Android, iOS, Emscripten (WebAssembly)
- **Builds**: Client (full 3D/2D GUI) and Server (headless) variants

**Architecture**: Monolithic C++ engine organized into subsystems (rendering, physics, terrain, GUI, networking, asset management) with Lua scripting bindings exposed via NPL API.

## Critical Build Workflows

### Platform-Specific Builds

**Linux Server Build**:
```bash
./build_linux.sh [jobs]  # e.g., ./build_linux.sh 6 for parallel build
# Output: ParaWorld/bin64/ParaEngineServer
# Installs to: /usr/local/bin/npl (symlink)
```

**Windows Client Build**:
```bat
# Run from Visual Studio Developer Command Prompt
build_win32.bat
# Creates: build/win32/ directory with cmake files
```

**Emscripten (WebAssembly)**:
```bash
# Boost must be pre-compiled for emscripten
emcmake cmake -S NPLRuntime -B build/emscripten -DEMSCRIPTEN=ON -DCURL_ENABLE_SSL=OFF -DBOOST_ROOT="path/to/boost"
cd build/emscripten && emmake make
```

### Key CMake Options

All builds use `NPLRuntime/CMakeLists.txt` as entry point. Critical options:

- `NPLRUNTIME_SERVER=ON` - Server build (no DirectX/GUI)
- `NPLRUNTIME_RENDERER` - DIRECTX/OPENGL/NULL (auto-selected by platform)
- `NPLRUNTIME_STATIC_LIB` - Static vs shared library linking
- `NPLRUNTIME_LUAJIT21/LUAJIT20/LUA51` - Lua runtime version (platform-specific defaults)
- `NPLRUNTIME_SUPPORT_FBX` - Assimp 3D model loading
- `NPLRUNTIME_PHYSICS` - Bullet physics integration

**Server default**: LuaJIT 2.1 with GC64, minimal features  
**Client default**: LuaJIT 2.0.4 (Win32), full features

## Code Organization & Patterns

### Directory Structure

**Most engine code is in `NPLRuntime/ParaEngine/`** - this is the ParaEngine 3D game engine core:

- `NPLRuntime/ParaEngine/` - **ParaEngine 3D game engine** (all subsystems below)
  - `3dengine/` - Scene management, objects (BaseObject, BipedObject)
  - `2dengine/` - GUI system (GUIRoot, GUIBase hierarchy)
  - `Engine/` - Core app lifecycle (ParaEngineCore, DirectXEngine)
  - `NPL/` - NPL runtime and networking (NPLRuntime, NPLNetServer)
  - `terrain/` - Terrain system (Terrain, GlobalTerrain, TerrainBlock)
  - `BlockEngine/` - Voxel/block world (Minecraft-style)
  - `BMaxModel/` - BMax 3D modeling format
  - `IO/` - File system, archives, async loading
  - `ParaScriptBindings/` - Lua API bindings
  - `Core/` - Globals, singletons (CGlobals, ParaEngineCore)
  - `renderer/` - Rendering abstraction (DirectX/OpenGL)
- `NPLRuntime/externals/` - Third-party libs (boost, bullet, assimp, FreeImage, etc.)
- `NPLRuntime/tests/` - Runtime tests (helloworld.lua)

### Namespace Conventions

**All engine code lives in `namespace ParaEngine`**. Standard pattern:
```cpp
using namespace ParaEngine;  // At file scope in .cpp files
```

**NPL runtime uses `namespace NPL`** for networking/scripting components.

### Singleton Access Pattern

**CGlobals is the central singleton accessor**. All major subsystems accessed via static methods:
```cpp
CGlobals::GetAssetManager()    // Asset loading (textures, models, etc.)
CGlobals::GetScene()            // Current scene object
CGlobals::GetGUI()              // 2D GUI root
CGlobals::GetNPLRuntime()       // NPL script runtime
CGlobals::GetGlobalTerrain()    // Terrain system
CGlobals::GetPhysicsWorld()     // Bullet physics
```

### Asset Management

Uses **AssetManager template pattern**:
```cpp
TextureEntity* pTex = CGlobals::GetAssetManager()->LoadTexture("", filename, TextureEntity::StaticTexture);
ParaXEntity* pModel = CGlobals::GetAssetManager()->LoadParaX("", filename);
```

Manager classes inherit from `AssetManager<T>`:
- `CBlockMaterialManager : AssetManager<CBlockMaterial>`
- `BufferPickingManager : AssetManager<CBufferPicking>`

### Attribute System

Many engine classes implement `IAttributeFields` for reflection/serialization:
```cpp
class Terrain : public IAttributeFields { ... }
```

Common pattern:
- `GetChildAttributeObject()` for hierarchical access
- `InstallFields()` for attribute registration
- Used by scripting layer for property access

## Platform-Specific Considerations

### Preprocessor Macros

- `WIN32` - Windows builds
- `LINUX` - Linux builds (includes SDL builds on Win32)
- `ANDROID` - Android builds
- `IOS` - iOS builds
- `EMSCRIPTEN` - WebAssembly builds
- `EMSCRIPTEN_SINGLE_THREAD` / `SDL_SINGLE_THREAD` - Single-threaded variants
- `NPLRUNTIME_SERVER` - Server build (no rendering)
- `PARAENGINE_CLIENT` - Client build (default)

### Threading

Uses Boost threading primitives (`boost::thread`, `boost::shared_mutex`), **disabled in single-threaded builds** (Emscripten/SDL variants).

## NPL Language Patterns

NPL scripts are Lua with special activation mechanism:
```lua
-- Activate a neuron file (message passing)
NPL.activate("(gl)helloworld.npl", {data="hello world!"})

-- Neuron entry point
this(msg)
   if(msg) then
      print(msg.data or "");
   end
end
```

Activation format: `"(neuron_name)filename.npl"` where `(gl)` = global thread.

## Common Workflows

### Adding New 3D Object Types
1. Inherit from `CBaseObject` (in `3dengine/BaseObject.h`)
2. Implement in `3dengine/` subdirectory
3. Register with scene manager via `CGlobals::GetScene()`
4. Add script bindings in `ParaScriptBindings/`

### Adding GUI Components
1. Inherit from `CGUIBase` hierarchy (in `2dengine/`)
2. Implement rendering in platform-specific renderer (`GUIDirectX` or `GUIOpenGL`)
3. Register with `CGUIRoot` via `CGlobals::GetGUI()`

### Debugging
- Use `OUTPUT_LOG()` for engine logging (from `FileLogger.h`)
- NPL scripts: `print()` or `log()` functions
- Test with minimal script: `npl NPLRuntime/tests/helloworld.lua`

## External Dependencies

**Boost**: Required (v1.55+). Build subset with: `thread, date_time, filesystem, system, chrono, serialization, iostreams, regex`

**Critical externals** (in `NPLRuntime/externals/`):
- LuaJIT 2.0/2.1 or Lua 5.1 (depending on build config)
- Bullet 2.75/3.x (physics)
- Assimp 3.1/4.0/5.0 (3D model loading)
- FreeImage (texture loading)
- OpenSSL/curl (networking)
- SDL2 (cross-platform windowing, optional)

## Rendering Abstraction

Engine supports **DirectX 9 (Windows)** or **OpenGL 3.0+ (cross-platform)** via abstraction layer:
- `renderer/effect_file.h` - Shader abstraction
- `OpenGLWrapper/` - OpenGL implementation
- `d3dcommon/` - DirectX implementation

Renderer selected at CMake configure time via `NPLRUNTIME_RENDERER` option.

## Do Not

- **Never** use `&&` in PowerShell commands (use `;` or separate calls)
- **Never** mix Lua runtime versions in same build
- **Never** call `GetAssetManager()` before engine initialization
- **Avoid** modifying external dependency sources (use cmake overrides)
