# Components Reference

Complete directory map of `Client/trunk/ParaEngineClient/` — the primary C++ source tree shared by client and server builds.

For detailed subsystem documentation (class-level reference, render pipeline, block world dimensions, etc.), see **[paraengine-client/](paraengine-client/README.md)**.

## Top-Level Layout

```
Client/trunk/ParaEngineClient/
├── Core/               Engine kernel and platform abstraction
├── Engine/             Application entry points and game logic
├── NPL/                Neural Parallel Language runtime
├── ParaScriptBindings/ C++ to Lua API bindings
├── 3dengine/           Scene graph and 3D rendering objects
├── 2dengine/           2D GUI widgets and input
├── renderer/           Graphics device abstraction
├── shaders/            HLSL and GLSL shader files
├── ParaXModel/         Native 3D format and model loading
├── BMaxModel/          Blender Max format parser
├── VoxelMesh/          Voxel terrain meshing
├── BlockEngine/        Minecraft-style block world
├── terrain/            Global terrain system
├── PaintEngine/        2D painting and raster operations
├── IO/                 Virtual file system and archives
├── math/               Vector, matrix, collision math
├── protocol/           Network protocol messages
├── Framework/          Input system framework
├── AutoRigger/         Automatic character rigging
├── WebSocket/          WebSocket networking
├── WebBrowser/         Embedded HTML browser (legacy IE)
├── curllua/            Lua bindings for libcurl
├── debugtools/         Profiler and debug utilities
├── ic/                 InfoCenter database manager
├── CommonFramework/    Data tables and shared framework
├── common/             DirectX sample utilities
├── dir_monitor/        Directory change monitoring
├── util/               Logging, threading, utilities
├── platform/           OS-specific code (win32, mac)
├── CadModel/           CAD model import
├── mdxfile/            Legacy MDX model support
├── res/                SQL templates, embedded resources
├── doc/                Legacy internal documentation
└── flashplayer/        Flash player plugin (legacy)
```

## Core/

Engine kernel — application lifecycle, globals, assets, plugins.

| File/Class | Role |
|------------|------|
| `ParaEngineApp.h/.cpp` | Main application class (init, main loop, device) |
| `ParaEngineAppImp.h/.cpp` | Application implementation details |
| `ParaEngineService.h/.cpp` | Headless service mode (timer-driven NPL loop) |
| `ParaEngineCore.h/.cpp` | Core engine initialization |
| `CGlobals` (`Globals.h/.cpp`) | Global singleton accessors |
| `BootStrapper.h/.cpp` | Bootstrapper XML loading, main loop script |
| `AssetManager.h/.cpp` | Asset loading, caching, reference counting |
| `PluginAPI.h` | Dynamic plugin loading interface |
| `CommandLineParams.h/.cpp` | Command-line argument parsing |
| `FrameRateController.h/.cpp` | Frame rate limiting |
| `INPLRuntime.h` | NPL runtime interface |
| `IAttributeFields.h` | Reflection/attribute system |
| `ParameterBlock.h/.cpp` | Named parameter storage |
| `TextureEntity*.h` | Texture asset entities (DX/GL) |
| `IBatchedElementDraw.h` | Debug/editor batched drawing |

## Engine/

Application entry points and game-specific logic (client-heavy).

| File | Role |
|------|------|
| `ParaWorld.cpp` | Client entry: `WinMain()` |
| `ParaEngineServer.cpp` | Server entry: `main()` |
| `ParaEngineApp.cpp` | Application initialization chain |
| `CharacterDBProvider.cpp` | Character database |
| `Modifier.cpp` | Game modifiers |
| `ParaXStaticBase.cpp` | Static ParaX base (shared client/server) |

## NPL/

Neural Parallel Language runtime — see [npl-runtime.md](npl-runtime.md).

| File/Class | Role |
|------------|------|
| `NPLRuntime.h/.cpp` | Runtime singleton |
| `NPLRuntimeState.h/.cpp` | Per-thread Lua state |
| `NPLDispatcher.h/.cpp` | Message routing |
| `NPLConnection.h/.cpp` | TCP connection |
| `NPLNetServer.h/.cpp` | TCP server |
| `NPLNetUDPServer.h/.cpp` | UDP server |
| `NPLMsgIn.h`, `NPLMsgOut.h` | Wire messages |
| `NPLMsgIn_parser.h` | Message parser |
| `NPLScriptingState.h/.cpp` | Lua state wrapper |
| `NPLActivationFile.h` | Neuron file state |
| `NPL_imp.h/.cpp` | INPL interface |
| `AISimulator.h/.cpp` | AI simulation hook |
| `NPL_readme.txt` | Internal network documentation |

## ParaScriptBindings/

C++ to Lua API — see [script-bindings.md](script-bindings.md).

52 source files organized as `ParaScripting*.cpp/.h` plus specialized binding files for character, graphics, terrain, block world, browser, flash, movie, IC, and test utilities.

## 3dengine/

Scene graph and 3D objects (~195 files) — see [scene-graph-and-3d-engine.md](scene-graph-and-3d-engine.md).

Key classes: `CSceneObject`, `CBaseObject`, `CAutoCamera`, `CLightManager`, `ParaXAnimInstance`, `PhysicsWorld`, `SelectionManager`, `CMiniSceneGraph`, `ShadowMap`, `EnvironmentSim`.

## 2dengine/

2D GUI system and input handling.

| Component | Role |
|-----------|------|
| GUI widgets | Buttons, sliders, scrollbars, edit boxes, list boxes |
| `GUIKeyboardVirtual.h/.cpp` | Virtual keyboard (mobile) |
| `GUIWebBrowser.cpp` | Embedded browser widget |
| Input handling | DirectInput integration, touch input |
| IME support | Input method editor for CJK text |

Rendered as overlay on 3D scene via `SpriteRenderer`.

## renderer/

Graphics device abstraction — see [renderer.md](renderer.md).

Dual backend: `RenderDeviceDirectX` (Windows) and `RenderDeviceOpenGL` (Linux/macOS/mobile). Unified via `RenderDevice.h`.

## ParaXModel/

Native 3D model format and loading pipeline.

| Component | Role |
|-----------|------|
| ParaX format | Native skeletal animation format (`.x` files) |
| `ColladaModelLoader` | FBX/Collada import via Assimp |
| `glTFModelExporter` | glTF export |
| Animation system | Bone controllers, IK, blend trees |
| `ParticleSystemRef` | Particle system references |
| Model serialization | Save/load ParaX assets |

## terrain/

Large-scale terrain rendering.

| Component | Role |
|-----------|------|
| `CGlobalTerrain` | Terrain manager (in `ParaTerrain` namespace) |
| `CTerrainTile` | Individual terrain tiles |
| `TerrainGeoMipmapIndices` | LOD index generation |
| `TerrainFilters` | Heightmap processing filters |
| Texture factories | Multi-layer terrain texture splatting |

## BlockEngine/

Minecraft-style voxel block world.

| Component | Role |
|-----------|------|
| `BlockWorldClient` | Client-side block world |
| `BlockChunk` | 16×16×16 block chunks |
| `BlockModelProvider` | Block mesh generation |
| `StairModelProvider` | Stair/slab geometry |

## VoxelMesh/

Alternative voxel terrain with isosurface extraction.

| Component | Role |
|-----------|------|
| `VoxelTerrainManager` | Voxel terrain lifecycle |
| `IsoSurfaceBuilder` | Marching cubes meshing |

## IO/

Virtual file system and asset loading.

| Component | Role |
|-----------|------|
| Virtual file system | Unified access to disk, zip, memory files |
| Zip archives | Packaged asset loading |
| Async loading | Background asset loading threads |
| Asset manifest | Asset inventory and versioning |

## math/

Mathematical primitives.

| File | Types |
|------|-------|
| `ParaVector3.h/.inl` | 3D vectors |
| `ParaMatrix3.h` | 3×3 matrices |
| `ParaMathMatrix.h` | 4×4 matrices |
| `ParaMath.h` | Math utilities |
| `ParaAngle.h` | Angle representations |
| AABB colliders | Axis-aligned bounding boxes |
| Shape utilities | Ray, plane, frustum |

## Framework/

Input system and common framework.

| Component | Role |
|-----------|------|
| Input system | Keyboard, mouse, gamepad abstraction |
| Event dispatch | Input event routing to scene/GUI |

## AutoRigger/

Automatic character skeleton generation using PCL and Pinocchio (server-side).

## WebSocket/

WebSocket protocol support for NPL networking alternative to raw TCP.

## protocol/

Network protocol message definitions (protobuf-style).

## Server Plugins (`Server/trunk/`)

Built alongside the main runtime:

| Plugin | Directory | Role |
|--------|-----------|------|
| `NPLMono2` | `NPLMono/NPLMono2/` | Mono/.NET scripting backend |
| `NPLRouter` | `NPLRouter/` | NPL message router and DB proxy |
| `luasql` | `luasql/` | SQL database bindings (optional) |
| `TMInterface` | `TMInterface/` | TradeMark interface (disabled) |

## Client Plugins (`Client/trunk/`)

Optional client plugins:

| Plugin | Directory | Role |
|--------|-----------|------|
| `PhysicsBT` | `PhysicsBT/` | Bullet3 physics engine |
| `cAudio` | `cAudio_2.4.0/` | Audio engine (OpenAL-based) |
| `ParaEngineClientApp` | `ParaEngineClientApp/` | Test harness for client DLL |
| `HaqiLauncher` | — | Game launcher with auto-updater |

## Supporting Directories

| Directory | Role |
|-----------|------|
| `PaintEngine/` | 2D painting: transforms, regions, raster operations |
| `BMaxModel/` | Blender Max export format parser |
| `CadModel/` | CAD model import (via ANTLR parser) |
| `mdxfile/` | Legacy Warcraft MDX model format (deprecated) |
| `curllua/` | libcurl bindings for Lua HTTP requests |
| `debugtools/` | Performance profiler, debug overlays |
| `ic/` | InfoCenter — embedded SQLite database manager |
| `CommonFramework/` | Data tables (`BaseTable`), shared table utilities |
| `common/` | DirectX SDK sample utilities (`dxutil`, enumeration) |
| `dir_monitor/` | Cross-platform filesystem change notifications |
| `util/` | Logging, mutex, semaphores, threading, MD5, keyboard, string builders, memory pools |
| `platform/win32/` | Windows-specific window creation, registry |
| `platform/mac/` | macOS `main()`, bundle support |
| `res/` | SQL templates, character script templates, embedded resources |
| `doc/` | Legacy docs: bootstrapping, events, debugging, Doxygen configs |
| `flashplayer/` | Adobe Flash player plugin (legacy, deprecated) |
| `WebBrowser/` | IE-based embedded HTML browser (legacy) |
| `jabber/` | Jabber/XMPP client bindings |

## File Counts (Approximate)

| Directory | Files | Notes |
|-----------|-------|-------|
| `3dengine/` | ~195 | Largest subsystem |
| `ParaScriptBindings/` | ~52 | Lua API surface |
| `NPL/` | ~40 | Runtime core |
| `Core/` | ~35 | Engine kernel |
| `renderer/` | ~30 | Graphics backends |
| `2dengine/` | ~25 | GUI widgets |
| `ParaXModel/` | ~25 | Model pipeline |
| `terrain/` | ~15 | Terrain system |
| `util/` | ~20 | Utilities |
| **Total** | **~1,470** | Full ParaEngineClient tree |

## CMake Source Lists

Which files compile into each target is controlled by:
- **Client:** `NPLRuntime/ParaEngineClient/CMakeLists.txt` (included by `Client/CMakeLists.txt`)
- **Server:** `NPLRuntime/ParaEngineServer/CMakeLists.txt`

When adding new source files, update the appropriate CMakeLists.txt.
