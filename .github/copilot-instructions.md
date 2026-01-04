# NPLRuntime AI Coding Agent Instructions

## Project Overview
**ParaEngine** is a cross-platform 3D game engine with full-featured 2D support and server capabilities. It uses **Neural Parallel Language (NPL)** as its scripting language - a high-performance language that is 100% Lua-compatible. The engine supports Windows, Linux, Android, and iOS platforms, providing a complete solution for building multiplayer 3D games and interactive applications.

### Key Components
- **ParaEngineClient**: Full 3D game engine with scene graph, physics (Bullet3), particle systems, terrain, skeletal animation, and DirectX/OpenGL rendering (`Client/trunk/ParaEngineClient/`)
- **ParaEngineServer**: Headless server for multiplayer games and NPL applications (`NPLRuntime/ParaEngineServer/`)
- **NPL Language Runtime**: Multi-threaded scripting engine with async message passing (`Client/trunk/ParaEngineClient/NPL/`)
- **3D Engine Systems**: Scene management, cameras, lighting, shadows, post-effects (`3dengine/`, `renderer/`, `terrain/`)
- **Output Directory**: Built binaries output to `ParaWorld/bin32/` or `ParaWorld/bin64/`

## Architecture Highlights

### Game Engine Architecture
- **Scene Graph**: Hierarchical 3D scene organization with `CBaseObject` nodes
- **Asset Management**: Reference-counted entities for textures, models, meshes via `AssetEntity<T>`
- **Rendering Pipeline**: Abstracted renderer supporting DirectX (Windows) and OpenGL (Linux/Mac/Mobile)
- **Physics Integration**: Optional Bullet3 physics engine for collision detection and rigid body dynamics
- **Animation System**: Skeletal animation, particle systems, and custom animation controllers
- **Terrain System**: Large-scale terrain rendering with LOD, texture splatting, and height maps
- **Character System**: Biped controllers, custom character models, and facial animation support

### NPL Scripting Language & Activation Model
NPL is ParaEngine's scripting language, using a unique "activation" pattern for async message passing:
```lua
-- Activate a neuron file with data (async message passing)
NPL.activate("(gl)script/helloworld.npl", {data="hello"})
-- In the activated file:
this(msg) {
    print(msg.data)  -- Access passed data
}
```
- Files are "activated" rather than called, creating isolated execution contexts
- Each activation is a message to a neuron file (`.npl` or `.lua` files)
- Supports local `(gl)` and remote network activation for multiplayer games
- The `this(msg)` function receives activation messages
- Ideal for game event handlers, AI behaviors, and networked game logic

### Multi-Runtime State Architecture
- **Runtime States**: Multiple isolated Lua states can run in parallel threads
- **Dispatcher**: `CNPLDispatcher` routes messages between runtime states and network
- **Thread Safety**: Each runtime state (`CNPLRuntimeState`) owns its Lua state
- See `Client/trunk/ParaEngineClient/NPL/NPLRuntime.h` and `NPLRuntimeState.h`

### Dual Build Paths
1. **Client Build** (`Client/CMakeLists.txt`): Full ParaEngine game engine with 3D rendering, physics, audio, terrain, particles
2. **Server Build** (`NPLRuntime/CMakeLists.txt`): Lightweight headless server for multiplayer game backends and NPL services


## Code Conventions

### Namespaces
- `ParaEngine::`: Core engine and 3D components
- `NPL::`: Neural Parallel Language runtime
- Most headers declare `using namespace ParaEngine;` in implementation files

### Key Base Classes
- `IGameObject`: Base for all scene objects (`3dengine/IGameObject.h`)
- `CBaseObject`: Scene graph node (`3dengine/BaseObject.h`)
- `AssetEntity<T>`: Asset reference counting pattern
- `IAttributeFields`: Reflection/attribute system for engine objects

### File Organization
- **Engine/**: Core engine initialization, main loop, application bootstrap (`ParaEngine.h`)
- **Core/**: Platform abstraction, file I/O, asset manager, plugin system
- **3dengine/**: Complete 3D scene graph - cameras, lights, meshes, terrain, particles, characters, physics objects
- **renderer/**: Graphics device abstraction layer - DirectX/OpenGL implementations, shaders, render targets
- **terrain/**: Terrain tile system, detail textures, environment simulation
- **ParaXModel/**: Skeletal animation system, bone controllers, model loading/serialization
- **physics/**: Bullet3 physics integration, collision detection, ray casting
- **NPL/**: Multi-threaded scripting runtime, networking, message dispatcher
- **ParaScriptBindings/**: Complete Lua/NPL API for game scripting - scene, GUI, network, physics

### Render Device Abstraction
- `RenderDevice.h`: Unified interface
- `RenderDeviceDirectX.h`: Windows DirectX implementation
- `RenderDeviceOpenGL.h`: Linux/Mac OpenGL implementation
- Effect files: `.fx` (DirectX HLSL) and `.glsl` (OpenGL)

## External Dependencies

### Embedded Libraries (in Server/trunk/ and Client/trunk/externals/)
- **Lua 5.1.4** and **LuaJIT 2.0/2.1**: Alternative Lua implementations
- **luabind 0.9**: C++/Lua binding (older version for compatibility)
- **Boost 1.55+**: Thread, filesystem, signals2, iostreams (must build with matching toolchain)
- **curl 7.47.1**: HTTP/network requests
- **sqlite 3.6.23.1**: Embedded database
- **Bullet3**: Physics engine (optional, client only)
- **Assimp 5.0.1**: 3D model import (FBX/OBJ/etc., optional)

### Build Boost
```bash
cd Server/trunk/boost_1_XX_0
./bootstrap.sh
./b2 link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex
```

## Common Patterns

### Game Object Creation
```cpp
// Creating a 3D mesh object in the scene
CMeshObject* pMesh = new CMeshObject();
pMesh->SetAssetFileName("models/character.x");
pMesh->SetPosition(DVector3(0, 0, 0));
CGlobals::GetScene()->AttachObject(pMesh);
```

### Asset Management
```cpp
AssetEntity<TextureEntity>* pTexture = CGlobals::GetAssetManager()->LoadTexture(...);
pTexture->addref();  // Manual ref counting
// ... use pTexture
pTexture->Release(); // Decrement
```

### NPL C++ Activation Files
Define native C++ activation callbacks:
```cpp
// Function name MUST match pattern: NPL_activate_XXXX_cpp
NPL::NPLReturnCode NPL_activate_mymodule_cpp(INPLRuntimeState* pState) {
    // Access activation message via pState
    return NPL::NPL_OK;
}
// Activated via: NPL.activate("mymodule.cpp", {data=123})
```

### Scene Objects
All game objects derive from `CBaseObject` and implement `IGameObject`:
```cpp
class MyObject : public CBaseObject {
    virtual HRESULT Draw(SceneState* sceneState) override;
    virtual IGameObject* GetType() { return &g_type; }
    static CObjectType<MyObject> g_type;
};
```

## Platform-Specific Notes

### Windows
- Use Visual Studio project files in `bin/win32/` after CMake generation
- DirectX SDK required for DirectX renderer
- Drive letters stripped in file paths for portability


## Critical Constraints

1. **Boost Version Matching**: Boost must be built with the SAME compiler toolchain used for NPLRuntime (static linking requirement)
2. **Lua Compatibility**: NPL is 100% Lua 5.1 compatible - don't use Lua 5.2+ features
3. **Thread Safety**: Never share Lua states across threads - use NPL message passing
4. **Asset Reference Counting**: Always pair `addref()` with `Release()` for assets
5. **CMake Generator**: Windows uses Visual Studio generators, Linux uses Unix Makefiles

## When in Doubt

- **Source Code Overview**: See project wiki at https://github.com/LiXizhi/NPLRuntime/wiki/SourceCodeOverview
