# ParaEngine/NPLRuntime Development Guide

## Project Overview
**ParaEngine** is a cross-platform 3D game engine with integrated **NPLRuntime** (Neural Parallel Language Runtime) - a high-performance Lua-based scripting environment. The engine supports Windows/Linux/macOS/iOS/Android/Emscripten/HarmonyOS and can build as both client (3D/2D GUI) and headless server applications.

**Key Facts:**
- C++11/17/20 codebase (~2005-present, mature production engine)
- Lua 5.1 compatible scripting (LuaJIT 2.0/2.1 or vanilla Lua 5.1)
- Multi-threaded NPL runtime with message-passing concurrency
- DirectX9 or OpenGL rendering backends (or null renderer for server)

## Architecture Overview

### Component Structure (all under [NPLRuntime/](NPLRuntime/))

**[ParaEngine/](NPLRuntime/ParaEngine/)** - Core engine (organized by subsystem, not src/include split):
- **NPL/** - Neural Parallel Language runtime, networking (`NPL::CNPLRuntime`, message queue, activation system)
- **3dengine/** - Scene graph, game objects (`CSceneObject`, `CBipedObject`, cameras, lighting, particle systems)
- **2dengine/** - GUI system (`GUIRoot`, Canvas, paint engine inspired by Qt)
- **IO/** - Async file loader, virtual filesystem, zip archives
- **BlockEngine/** - Voxel/block world system (Minecraft-like)
- **BMaxModel/** - Block max model format (voxel models)
- **ParaXModel/** - Proprietary ParaX animated model format (.x-inspired)
- **terrain/** - Tile-based terrain with LOD (quad-tree, geo-mipmapping)
- **ParaScriptBindings/** - Lua API bindings (luabind-based)
- **Engine/** - Platform-agnostic app lifecycle (`CParaEngineApp` base class)
- **Core/** - Asset management (`CParaWorldAsset` singleton), globals

**[RenderSystem/](NPLRuntime/RenderSystem/)** - Renderer abstraction:
- `d3d9/` - DirectX9 implementation
- `opengl/` - OpenGL 3.3+ implementation
- `null/` - Headless renderer for server builds

**[Platform/](NPLRuntime/Platform/)** - Entry points per platform:
- `Windows/`, `Linux/`, `iOS/`, `AndroidStudio/`, `HarmonyOS/`, `SDL/`, etc.
- Each contains `main()` or equivalent platform bootstrap code

**[externals/](NPLRuntime/externals/)** - Vendored dependencies (see External Dependencies section)

**[Plugins/](NPLRuntime/Plugins/)** - Optional modules (audio, SQLite, physics)

### Key Design Patterns

**Asset Management** (see [ParaEngine/Core/ParaWorldAsset.h](NPLRuntime/ParaEngine/Core/ParaWorldAsset.h)):
- Centralized singleton: `CGlobals::GetAssetManager()` or `CParaWorldAsset::GetInstance()`
- Asset types: `TextureEntity`, `MeshEntity`, `ParaXEntity`, `SpriteFontEntity`, etc.
- Reference counted via `asset_ptr<T>` (intrusive pointer wrapper)
- Assets keyed by file path; lazy-loaded on first access
- Garbage collection: `UnloadAsset()`, `GarbageCollectAll()`

**Smart Pointers** (see [ParaEngine/util/intrusive_ptr.h](NPLRuntime/ParaEngine/util/intrusive_ptr.h)):
- `asset_ptr<T>` - Alias for `boost::intrusive_ptr<T>` (assets, reference counted objects)
- `ref_ptr<T>` - Similar intrusive pointer for general use
- Base classes: `intrusive_ptr_thread_safe_base`, `intrusive_ptr_single_thread_base`
- Prefer intrusive pointers over `shared_ptr` for performance/footprint

**Terrain System** (see [ParaEngine/terrain/](NPLRuntime/ParaEngine/terrain/)):
- `CGlobalTerrain` - Singleton manager ([GlobalTerrain.h](NPLRuntime/ParaEngine/terrain/GlobalTerrain.h))
- `TerrainLattice` - Tiled terrain grid (dynamic loading/unloading)
- `CDynamicTerrainLoader` - Loads terrain tiles on-demand
- `TerrainBlock` - Quad-tree LOD node
- Supports height maps, multi-texturing, holes, region values

**Scene Graph** (see [ParaEngine/3dengine/SceneObject.cpp](NPLRuntime/ParaEngine/3dengine/SceneObject.cpp)):
- `CSceneObject` - Root scene manager (singleton via `CGlobals::GetScene()`)
- Game objects inherit from `CBaseObject` → `IGameObject`
- Quad-tree spatial partitioning for culling
- Update/Render phases: `Animate()`, `Draw()`, `FrameMove()`

## NPL Scripting System

### Message Passing Model
NPL scripts use **activation** for inter-thread/inter-process communication:

```lua
-- Activate a script file with message data
NPL.activate("(gl)script.npl", {data="hello", count=42})

-- Every .npl file has an implicit message handler
this(msg) {
   if msg then
      print(msg.data, msg.count)
   end
}
```

**Activation Prefixes:**
- `(gl)` - Global/main thread
- `(worker1)` - Named worker thread
- `(ip:port)` - Remote NPL process (network activation)
- No prefix - Same runtime state as caller

See [ParaEngine/ParaScriptBindings/ParaScriptingNPL.h](NPLRuntime/ParaEngine/ParaScriptBindings/ParaScriptingNPL.h#L182) for full API.

### Adding Lua Bindings
1. Implement C++ function in `ParaEngine/ParaScriptBindings/ParaScripting*.{h,cpp}`
2. Bind using luabind in appropriate `LoadHAPI_*()` function:
   ```cpp
   module(L)[
       class_<MyClass>("MyClass")
           .def("method", &MyClass::method)
   ];
   ```
3. Export to NPL global table in `ParaScriptingGlobal.cpp`
4. Document in header (NPL API functions have Doxygen-style comments)


## Coding Conventions

### File Organization
- **No src/include split** - `.h` and `.cpp` files colocate in feature directories
- **Precompiled header:** `ParaEngine.h` included first in all `.cpp` files
- **Asset files:** Store in `ParaWorld/` directory structure

### Naming Conventions
- Classes: `CMyClass` (C prefix for classes, I prefix for interfaces)
- Singletons: `GetInstance()` or accessed via `CGlobals::Get*()`
- Assets: `*Entity` suffix (e.g., `TextureEntity`, `MeshEntity`)
- Managers: `*Manager` suffix (e.g., `TextureAssetManager`)

### Platform Macros
Use guards for platform-specific code:
```cpp
#ifdef USE_DIRECTX_RENDERER
    // DirectX9-specific code
#endif

#ifdef EMSCRIPTEN
    // Emscripten/WebAssembly specific
#endif

#ifdef PARAENGINE_MOBILE
    // iOS/Android common code
#endif

#ifndef EMSCRIPTEN_SINGLE_THREAD
    // Multi-threaded code (Boost.Thread)
#endif
```

### Memory Management
- Assets are reference counted (see Smart Pointers above)
- Use `SAFE_DELETE(ptr)` and `SAFE_RELEASE(comptr)` macros
- Pool allocators for frequent small objects (see `util/ParaMemPool.h`)
- Avoid `new`/`delete` in hot paths; prefer object pooling

### Threading
- Main thread runs render loop in `CParaEngineApp::FrameMove()`
- NPL worker threads managed by `NPL::CNPLRuntime`
- Use `#ifndef EMSCRIPTEN_SINGLE_THREAD` to guard Boost.Thread code
- Emscripten can build single or multi-threaded (pthread pool)

## Common Workflows

### Cross-Platform File Access
- **Never hard-code paths** - Use `CFileUtils` or `CPathReplaceables`
- Writable directory: `CFileUtils::GetWritablePath()`
- Asset loading: `CParaFile::OpenAssetFile()` (handles zip archives)
- Example: `"model/character.x"` resolves via asset search paths

## External Dependencies

All vendored in [externals/](NPLRuntime/externals/), configured in [externals/CMakeLists.txt](NPLRuntime/externals/CMakeLists.txt):

**Core Libraries:**
- **Boost** (1.55-1.85) - Required: `thread`, `filesystem`, `system`, `chrono`, `serialization`, `iostreams`, `locale`
  - ⚠️ Emscripten builds need Boost compiled with `toolset=emscripten` (see [README](README.md#Emscripten))
- **Lua/LuaJIT** - Three options: LuaJIT 2.0.4, LuaJIT 2.1, or Lua 5.1.5 (build-time selection)
- **luabind** - C++/Lua binding generator (modified for NPL)

**Networking:**
- **cURL** - HTTP/HTTPS requests
- Custom NPL networking (RakNet-inspired, in `ParaEngine/NPL/`)

**Graphics:**
- **Assimp** (3.1.1, 4.0.0, 5.0.1) - FBX/COLLADA/OBJ model loading
- **FreeImage** (3.17.0, 3.18.0) - Texture loading (PNG, JPEG, DDS, etc.)
- **glad** - OpenGL loader
- **freetype** (2.8.1) - Font rendering

**Physics:**
- **Bullet** (2.75 or 3.x) - Collision detection, rigid body dynamics
- **Pinocchio** - Character rigging

**Other:**
- **jsoncpp** - JSON parsing
- **tinyxml/tinyxpath** - XML parsing
- **zlib** - Compression

## Important Gotchas

### Boost and Emscripten
Emscripten builds **require** Boost libraries compiled with Emscripten's `emcc`:
```bash
./bootstrap.sh
./b2 toolset=emscripten runtime-link=static \
  --with-thread --with-filesystem --with-system \
  --with-chrono --with-serialization --with-iostreams
```
Then convert `.bc` files to `.a`: `emar -q libname.a libname.bc`

### Lua 5.1 Compatibility
- NPL strictly targets Lua 5.1 semantics (even with LuaJIT)
- No `goto`, no `__gc` metamethod differences
- `_ENV` not available (Lua 5.2+ feature)
- Use `getfenv`/`setfenv` for environment manipulation

### C++ Standard Variations
- Windows/Android: C++17
- macOS/Linux/iOS: C++20
- Emscripten: C++20 (configurable)
- Never use C++14+ features without CMake checks for compatibility

### DirectX9 Limitations
- Fixed-function pipeline still used in some legacy code
- Shader Model 3.0 maximum
- Consider OpenGL path for modern rendering features

### Threading Caveats
- NPL messages are serialized (tables → strings), not shared memory
- Emscripten's pthread has limitations (stack size, SharedArrayBuffer)
- Use `EMSCRIPTEN_SINGLE_THREAD` build option for simpler debugging

## Critical Files Reference

- [NPLRuntime/CMakeLists.txt](NPLRuntime/CMakeLists.txt) - Root build configuration
- [NPLRuntime/ParaEngine/NPL/NPLRuntime.h](NPLRuntime/ParaEngine/NPL/NPLRuntime.h) - NPL runtime singleton
- [NPLRuntime/ParaEngine/Core/ParaWorldAsset.h](NPLRuntime/ParaEngine/Core/ParaWorldAsset.h) - Asset manager singleton
- [NPLRuntime/ParaEngine/Engine/ParaEngineApp.h](NPLRuntime/ParaEngine/Engine/ParaEngineApp.h) - Main application loop
- [NPLRuntime/Platform/Windows/src/ParaEngineApp.cpp](NPLRuntime/Platform/Windows/src/ParaEngineApp.cpp) - Windows entry point
- [NPLRuntime/ParaEngine/ParaScriptBindings/ParaScriptingGlobal.cpp](NPLRuntime/ParaEngine/ParaScriptBindings/ParaScriptingGlobal.cpp) - Lua global exports
- [README.md](README.md) - Build instructions, examples

## Anti-Patterns

❌ **Avoid:**
- Adding `using namespace std;` in headers
- Breaking Lua 5.1 compatibility (no `goto`, careful with metatables)
- Hard-coded file paths (use `CFileUtils` abstraction)
- Direct D3D/OpenGL calls outside RenderSystem (use `CGlobals::GetRenderDevice()`)
- Introducing C++14+ without platform checks
- `new`/`delete` in performance-critical code (prefer pooling)
- Blocking operations in NPL message handlers (blocks worker thread)

✅ **Do:**
- Use `asset_ptr<T>` for all asset references
- Call device lifecycle methods: `InitDeviceObjects()`, `RestoreDeviceObjects()`, `InvalidateDeviceObjects()`, `DeleteDeviceObjects()`
- Check `CGlobals::GetAssetManager()` for existing asset before creating new
- Prefer composition over inheritance for game objects
- Document Lua APIs with Doxygen comments
- Test on multiple platforms (Windows, Linux minimum)

## Additional Resources
- **Wiki:** https://github.com/LiXizhi/NPLRuntime/wiki/
