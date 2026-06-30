# ParaEngine Subsystems

ParaEngine is the core static library (`ParaEngine` CMake target) containing the game engine, asset pipeline, and NPL integration. Source is organized by **feature directory** — headers and implementations colocate (no separate `src/` / `include/` split).

Built from `NPLRuntime/ParaEngine/CMakeLists.txt` via `ucm_add_dirs()`.

## Module Map

```
ParaEngine/
├── Core/              App lifecycle, assets, events, service
├── Framework/         Public interfaces and abstractions
├── NPL/               Script runtime (see npl-runtime.md)
├── ParaScriptBindings/ Lua API surface
├── 3dengine/          Scene graph, characters, viewports
├── 2dengine/          GUI widgets and layout
├── PaintEngine/       2D vector/raster painter
├── renderer/          API-agnostic rendering helpers
├── BlockEngine/       Voxel/block world
├── terrain/           Heightfield terrain
├── ParaXModel/        ParaX animated models
├── BMaxModel/         Block max voxel models
├── AutoRigger/        Character auto-rigging (Pinocchio)
├── IO/                Serial ports, file watcher
├── WebSocket/         WebSocket I/O
├── jabber/            XMPP client
├── protocol/          Network protocol helpers
├── InfoCenter/        In-engine info/logging hub
├── debugtools/        Profiler, debug utilities
├── dirmonitor/        Directory change monitoring
├── math/              Math utilities
├── util/              Logging, mutex, timers, memory pools
├── curllua/           curl Lua bindings (non-Emscripten)
├── OpenGLWrapper/     OpenGL-specific helpers
├── shaders/           .fx shader sources (embedded at build)
└── Engine/            DirectX-only extended systems
```

DirectX-only additions when `USE_DIRECTX_RENDERER`:

- `d3dcommon/`, `VoxelMesh/`, `CadModel/`

---

## Core

**Path:** `ParaEngine/Core/`

Central application and resource management.

| Class / File | Role |
|--------------|------|
| `CParaEngineAppBase` | Shared app base: Init, FrameMove, device lifecycle, input |
| `CParaEngineServerApp` | Headless server app variant |
| `CParaEngineService` | Non-GUI service loop (boost::asio timer) |
| `CParaWorldAsset` | **Asset manager singleton** — textures, meshes, fonts, effects |
| `CGlobals` | Static accessors to scene, runtime, render device, etc. |
| `CommandLineParams` | CLI parsing |
| `FrameRateController` | FPS limiting and frame timing |
| `ParaEngineService.cpp` | Server main loop integration |

### CParaEngineAppBase lifecycle

```
InitApp(window, cmdLine)
  → FindParaEngineDirectory()
  → BootStrapAndLoadConfig()
  → InitSystemModules()      // NPL, assets, scene, GUI
  → InitRenderEnvironment()
StartApp()
  → initial NPL activation
FrameMove(fTime)             // per frame
  → CNPLRuntime::Run()
  → scene/GUI update
  → Render()
StopApp() / Cleanup
```

Device lifecycle methods (called on GPU context create/destroy/loss):

- `InitDeviceObjects()`
- `RestoreDeviceObjects()`
- `InvalidateDeviceObjects()`
- `DeleteDeviceObjects()`

---

## Framework

**Path:** `ParaEngine/Framework/`

Public interfaces decoupling engine subsystems from implementations.

| Interface | Purpose |
|-----------|---------|
| `IParaEngineApp` | Application contract |
| `INPLRuntime` / `INPLRuntimeState` | NPL runtime abstraction |
| `IRenderDevice` / `IRenderWindow` | GPU device and window |
| `IParaPhysics` | Physics world (implemented by PhysicsBT plugin) |
| `IAttributeFields` | Reflection / attribute system |

Implementations live in Core, NPL, RenderSystem, and Plugins.

---

## Asset Management (CParaWorldAsset)

**Path:** `ParaEngine/Core/ParaWorldAsset.h`

The asset hub for all loaded resources.

### Sub-managers

| Manager | Asset type |
|---------|------------|
| `TextureAssetManager` | 2D/3D textures |
| Mesh loading | `MeshEntity`, `ParaXEntity` |
| `SpriteFontEntity` | Bitmap and vector fonts |
| `EffectManager` | Shader effects (.fx) |
| `BlockMaterialManager` | Block/voxel materials |
| `DynamicVertexBufferManager` | Dynamic VB pools |
| `SequenceManager` | Animation sequences |
| `DatabaseEntity` | Embedded databases |

DirectX-only:

- `HTMLBrowserManager`, `VoxelTerrainManager`, `CadModel`, occlusion queries

### Usage pattern

```cpp
CParaWorldAsset* assets = CParaWorldAsset::GetSingleton();
TextureEntity* tex = assets->GetTexture("textures/grass.png");
ParaXEntity* model = assets->GetParaXModel("model/character.x");
```

Assets are reference-counted via `asset_ptr<T>` (boost intrusive_ptr). Keyed by file path; optional shortcut alias.

### Features

- **Async loading:** `SetAsyncLoading(true)` — background loader thread
- **Asset manifest:** optional manifest-based resolution
- **Local file first:** `SetUseLocalFileFirst()` for dev override
- **Garbage collection:** `UnloadAsset()`, `GarbageCollectAll()`

### File loading

`CParaFile::OpenAssetFile()` resolves paths through virtual filesystem and `.pkg` zip archives.

---

## 3D Engine

**Path:** `ParaEngine/3dengine/`

Scene graph, cameras, characters, and world simulation wrapper.

| Component | Role |
|-----------|------|
| `CSceneObject` | Root scene manager (singleton via `CGlobals::GetScene()`) |
| `CBaseObject` / `IGameObject` | Game object hierarchy |
| `CViewportManager` | Multi-viewport rendering |
| `CBipedObject` | Character/biped animation |
| `CCharacterObject` | Character controller |
| `CPhysicsWorld` | Physics wrapper (delegates to PhysicsBT) |
| Quad-tree | Spatial partitioning for culling |

### Frame phases

```
FrameMove:
  Animate(fTime)     // animation, AI, physics prep
  // culling via quad-tree
Draw:
  Draw()             // submit render batches
```

Game objects support composition — prefer attaching components over deep inheritance.

---

## 2D Engine (GUI)

**Path:** `ParaEngine/2dengine/`

Qt-inspired GUI system for in-engine UI.

| Component | Role |
|-----------|------|
| `CGUIRoot` | Root widget tree manager |
| Widget classes | Buttons, labels, edit boxes, scroll areas, etc. |
| Font rendering | Via FreeType + texture glyphs |
| Layout | Anchor and flow layout managers |

Script API: `ParaScriptBindings/ParaScriptingGUI.cpp`

Touch vs mouse: `CParaEngineAppBase::IsTouchInputting()` tracks input mode (mobile vs desktop).

---

## PaintEngine

**Path:** `ParaEngine/PaintEngine/`

2D drawing primitives — lines, fills, text, images — used by GUI and custom 2D rendering.

Script API: `ParaScriptBindings/ParaScriptingPainter.cpp`

---

## Renderer (Engine-side)

**Path:** `ParaEngine/renderer/`

API-agnostic rendering layer sitting above RenderSystem backends.

| File | Role |
|------|------|
| `RenderCore.h` | Backend selection macros |
| `RenderCoreOpenGL.h` | OpenGL draw path |
| `SpriteRenderer*.cpp` | 2D sprite batching |
| `effect_file.h` | Effect/shader file loading |

Compiled paths depend on `USE_OPENGL_RENDERER` / `USE_DIRECTX_RENDERER`.

All GPU access should go through `CGlobals::GetRenderDevice()` — not raw API calls.

---

## BlockEngine

**Path:** `ParaEngine/BlockEngine/`

Minecraft-style voxel block world.

| Component | Role |
|-----------|------|
| `CBlockWorld` | World container and chunk management |
| Block templates | Block type definitions |
| Region system | Spatial regions for block data |
| `BlockMaterialManager` | Block texture/material lookup |

Script API: `ParaScriptBindings/ParaScriptingBlockWorld.cpp`

Used heavily by Paracraft.

---

## Terrain

**Path:** `ParaEngine/terrain/`

Heightfield terrain with level-of-detail.

| Component | Role |
|-----------|------|
| `CGlobalTerrain` | Singleton terrain manager |
| `TerrainLattice` | Tiled terrain grid |
| `CDynamicTerrainLoader` | On-demand tile loading |
| `TerrainBlock` | Quad-tree LOD node |

Features: height maps, multi-texturing, terrain holes, region values, geo-mipmapping.

Script API: `ParaScriptBindings/ParaScriptingTerrain.cpp`

---

## Model Formats

### ParaXModel

**Path:** `ParaEngine/ParaXModel/`

Proprietary animated model format (.x-inspired). Supports skeletal animation, LOD, attachments.

`ParaXEntity` loaded via asset manager.

### BMaxModel

**Path:** `ParaEngine/BMaxModel/`

Block max format for voxel-based models (Paracraft block models).

### FBX / external formats

Via assimp when `NPLRUNTIME_SUPPORT_FBX=ON`. Loaded through ParaX/ mesh pipeline.

---

## AutoRigger

**Path:** `ParaEngine/AutoRigger/`

Automatic character rigging using Pinocchio library. Converts mesh to skeleton for animation.

---

## IO

**Path:** `ParaEngine/IO/`

| Feature | Role |
|---------|------|
| Async file loader | Background asset streaming |
| Virtual filesystem | Path replaceables, archive mounting |
| Serial port | Hardware serial I/O |
| `dirmonitor/` | Cross-platform file change notifications |

Script API: `ParaScriptBindings/ParaScriptingIO.cpp`

Path resolution: `CFileUtils`, `CPathReplaceables` — never hard-code absolute paths.

---

## WebSocket

**Path:** `ParaEngine/WebSocket/`

WebSocket reader/writer for browser and server communication. Includes Emscripten-specific path.

---

## Jabber / Protocol

| Path | Role |
|------|------|
| `jabber/` | XMPP client (`NPLJabberClient`) |
| `protocol/` | Low-level network protocol helpers |

Script API: `ParaScriptBindings/ParaScriptingNetwork.cpp`, `ParaScriptingIPC.cpp`

---

## Engine (DirectX-only)

**Path:** `ParaEngine/Engine/`

Extended systems available only with DirectX renderer:

| System | Role |
|--------|------|
| `DirectXEngine` | DirectX-specific engine extensions |
| `HTMLBrowserManager` | In-engine HTML rendering |
| DB providers | SQL/data providers |
| `VoxelMesh/` | DirectX voxel mesh rendering |
| `CadModel/` | CAD model display (optional OCE plugin) |

---

## ParaScriptBindings

**Path:** `ParaEngine/ParaScriptBindings/`

Luabind-based C++ → Lua API. Each file covers a subsystem:

| File | API domain |
|------|------------|
| `ParaScriptingGlobal.cpp` | Global table assembly |
| `ParaScriptingNPL.cpp` | NPL runtime API |
| `ParaScriptingScene.cpp` | 3D scene objects |
| `ParaScriptingGUI.cpp` | GUI widgets |
| `ParaScriptingBlockWorld.cpp` | Voxel world |
| `ParaScriptingTerrain.cpp` | Terrain |
| `ParaScriptingGraphics.cpp` | Render device, textures |
| `ParaScriptingAudio.cpp` | Audio (cAudioEngine) |
| `ParaScriptingIO.cpp` | File I/O |
| `ParaScriptingNetwork.cpp` | Networking |
| `ParaScriptingCharacter.cpp` | Characters |
| `ParaScriptingWorld.cpp` | World settings |
| `ParaScriptingCommon.cpp` | Shared utilities |
| `ParaScriptingWebView.cpp` | WebView2 (Windows) |
| `ParaScriptingHTMLBrowser.cpp` | HTML browser (DirectX) |

Entry point: `ParaScripting.cpp` — loads all HAPI modules.

---

## Debug and Utilities

| Path | Role |
|------|------|
| `debugtools/` | Profiler (`CProfiler`), debug overlays |
| `InfoCenter/` | Centralized info and log routing |
| `util/ParaMemPool.h` | Object pooling for hot paths |
| `util/intrusive_ptr.h` | `asset_ptr`, `ref_ptr` aliases |
| `util/mutex.h` | Cross-platform mutex |
| `math/` | Vectors, matrices, quaternions |

---

## Shaders

**Path:** `ParaEngine/shaders/`

HLSL `.fx` files compiled and embedded at build time (DirectX). OpenGL path uses GLSL via `GLSLCodeGen` and `fxParser` externals.

---

## Smart Pointers

Defined in `ParaEngine/util/intrusive_ptr.h`:

```cpp
typedef boost::intrusive_ptr<T> asset_ptr<T>;  // assets
typedef boost::intrusive_ptr<T> ref_ptr<T>;    // general ref-counted
```

Base classes:

- `intrusive_ptr_thread_safe_base` — atomic ref count
- `intrusive_ptr_single_thread_base` — non-atomic (main thread only)

Prefer intrusive pointers over `std::shared_ptr` for engine objects — lower overhead, explicit ownership.

---

## Attribute System (Reflection)

Many classes implement `IAttributeFields`:

```cpp
ATTRIBUTE_DEFINE_CLASS(CMyClass);
virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
```

Powers:

- In-engine object inspector
- NPL attribute access: `obj:GetField("PropertyName")`
- Serialization

---

## Related

- [architecture.md](architecture.md) — how subsystems connect
- [render-system.md](render-system.md) — GPU backend detail
- [npl-runtime.md](npl-runtime.md) — scripting runtime
- [conventions.md](conventions.md) — naming and patterns
