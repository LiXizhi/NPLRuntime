# Core Subsystem

Location: `Client/trunk/ParaEngineClient/Core/` (~88 headers, matching `.cpp` files)

The Core module is the engine kernel — application lifecycle, global state, asset management, reflection/attributes, plugins, events, and platform abstraction. Every other subsystem depends on Core.

## Key Classes

### Application Lifecycle

| Class | File | Role |
|-------|------|------|
| `CParaEngineAppBase` | `ParaEngineAppBase.h` | Shared app base: CLI parsing, directory discovery, touch/mouse mode |
| `CParaEngineApp` | `Engine/ParaEngineApp.cpp` | Full client: DirectX/OpenGL device, main loop, render |
| `CParaEngineService` | `ParaEngineService.h/.cpp` | Headless service: Boost.Asio 50ms timer, no window |
| `CParaEngineCore` | `ParaEngineCore.h/.cpp` | Core initialization, object factory registration |
| `CParaEngineServerApp` | `ParaEngineServerApp.h` | Server-specific app wrapper |

Entry flow:
```
WinMain/main → CParaEngineAppBase::InitCommon()
             → CParaEngineApp::Init() / OneTimeSceneInit()
             → InitDeviceObjects() / RestoreDeviceObjects()
             → CBootStrapper::LoadFromFile()
             → main loop (FrameMove + Render)
```

### Globals & Configuration

| Class | File | Role |
|-------|------|------|
| `CGlobals` | `Globals.h/.cpp` | Static accessors to all engine singletons |
| `CCommandLineParams` | `CommandLineParams.h` | CLI name/value pair storage |
| `CBootStrapper` | `BootStrapper.h/.cpp` | Bootstrapper XML → main loop script path |
| `CFrameRateController` | `FrameRateController.h` | Independent timers: FRC_GAME, FRC_SIM, FRC_IO, FRC_RENDER |
| `ParaEngineSettings` | `3dengine/ParaEngineSettings.h` | Runtime settings (linked from Core init) |

`CGlobals` provides:
```cpp
CGlobals::GetScene()           // CSceneObject*
CGlobals::GetGUI()             // CGUIRoot*
CGlobals::GetNPLRuntime()      // CNPLRuntime*
CGlobals::GetAssetManager()    // various AssetManager<T>
CGlobals::GetEffectManager()   // CEffectManager*
CGlobals::GetRenderDevice()    // RenderDevice*
CGlobals::GetGameTime()        // accumulated game seconds
CGlobals::GetFrameRateController(FRC_GAME)
```

### Asset System

| Class | File | Role |
|-------|------|------|
| `AssetEntity<T>` | `AssetEntity.h` | Reference-counted asset base |
| `AssetManager<T>` | `AssetManager.h` | Singleton per asset type; name/key maps |
| `ContentLoaders` | `ContentLoaders.h` | Async loader registry |
| `ContentLoaderParaX` | `ContentLoaderParaX.h` | ParaX model async loader |
| `ContentLoaderTexture` | `ContentLoaderTexture.h` | Texture async loader |
| `ContentLoaderMesh` | `ContentLoaderMesh.h` | Static mesh loader |
| `TextureEntity` | `TextureEntity.h` | 2D texture asset (+ DX/GL variants) |
| `ImageEntity` | `ImageEntity.h` | Raw image data |
| `DatabaseEntity` | `DatabaseEntity.h` | SQLite database asset |
| `SpriteFontEntity` | `SpriteFontEntity.h` | Bitmap font (+ DX/GL variants) |
| `DynamicVertexBufferEntity` | `DynamicVertexBufferEntity.h` | GPU dynamic VB (+ DX/GL) |
| `MiscEntity` | `MiscEntity.h` | Catch-all misc assets |
| `ParaWorldAsset` | `ParaWorldAsset.h` | World-level asset bundle |

Asset lifecycle:
```
LoadAsync(key) → background IDataLoader → IDataProcessor → AssetEntity in manager
Use: entity->addref() ... entity->Release()
Unload: Release() until ref count 0 → deleted from manager
```

All assets load **asynchronously by default** (wiki: NPLRuntimeAPI).

### Reflection / Attribute System

| Class | File | Role |
|-------|------|------|
| `IAttributeFields` | `IAttributeFields.h` | Base interface for reflected objects |
| `CAttributeClass` | `AttributeClass.h` | Class metadata (fields, methods) |
| `CAttributeField` | `AttributeField.h` | Single field descriptor |
| `CDynamicAttributeField` | `DynamicAttributeField.h` | Runtime-added fields |
| `CDynamicAttributesSet` | `DynamicAttributesSet.h` | Dynamic field collection |
| `CAttributesManager` | `AttributesManager.h` | Global attribute class registry |
| `CAttributeClassIDTable` | `AttributeClassIDTable.h` | Class ID constants |
| `CAttributeModelProxy` | `AttributeModelProxy.h` | Proxy for attribute editing |
| `ObjectFactory` | `ObjectFactory.h` | Runtime object creation by class ID |

Pattern used throughout engine — scene objects, GUI widgets, block world, and assets all implement `IAttributeFields` with `InstallFields()` registering `ATTRIBUTE_METHOD*` macros for script/editor access.

### Events

| Class | File | Role |
|-------|------|------|
| `CEventsCenter` | `EventsCenter.h` | Central event dispatch |
| `CEventHandler` | `EventHandler.h` | Event handler registration |
| `Events_def.h` | `Events_def.h` | Event type enums (editor, network, mouse, key) |
| `EventClasses.h` | `EventClasses.h` | Event class hierarchy |
| `IEvent` | `IEvent.h` | Event interface |

Event categories (from `doc/EventsReference.txt`):

| Type | Trigger | Script Code Example |
|------|---------|---------------------|
| Mouse | Click/move on 3D scene or GUI | `mouse_button="left"; mouse_x=X; mouse_y=Y` |
| Key | Key down/up | `virtual_key` from event_mapping.lua |
| UI frame move | GUI object rendered with delta > 0 | `deltatime = seconds` |
| Editor | Scene edit operations | `event_type = ED_SELECTION_CHANGED, ...` |
| Network | TCP connect/disconnect | `code=NPL_ConnectionEstablished, nid=...` |

Register from NPL:
```lua
ParaScene.RegisterEvent("_mclick_myhandler", ";myhandler.OnMouseClick();")
```

### Plugins

| Class | File | Role |
|-------|------|------|
| `CPluginManager` | `PluginManager.h` | Dynamic DLL loading |
| `PluginAPI.h` | `PluginAPI.h` | Plugin interface contract |
| `IParaEngineCore` | `IParaEngineCore.h` | Core interface for plugins |

Plugins loaded at runtime: `NPLMono2.dll`, `NPLRouter.dll`, `PhysicsBT.dll`, `cAudio` plugins, custom C++ DLLs via `NPL.activate("MyPlugin.dll", ...)`.

### Platform & IPC

| Class | File | Role |
|-------|------|------|
| `Platforms.h` / `ParaPlatformConfig.h` | Platform detection macros |
| `ParaPlatformMacros.h` | Compile-time platform flags |
| `CIPCManager` | `IPCManager.h` | Inter-process communication |
| `InterprocessMsg.h` | IPC message format |
| `CTouchEventSession` | `TouchEventSession.h` | Multi-touch session tracking |
| `UrlLoaders` | `UrlLoaders.cpp` | HTTP URL asset loading |

### Interfaces (for plugins and bindings)

| Interface | File | Purpose |
|-----------|------|---------|
| `INPL` | `INPL.h` | NPL facade |
| `INPLRuntime` | `INPLRuntime.h` (Core) | Runtime interface |
| `INPLRuntimeState` | `INPLRuntimeState.h` | Per-thread state |
| `INPLScriptingState` | `INPLScriptingState.h` | Lua state wrapper |
| `IParaEngineApp` | `IParaEngineApp.h` | Application interface |
| `IParaPhysics` | `IParaPhysics.h` | Physics plugin interface |
| `IParaAudioEngine` | `IParaAudioEngine.h` | Audio engine interface |
| `IParaWebService` | `IParaWebService.h` | Web service callbacks |
| `IParaDebugDraw` | `IParaDebugDraw.h` | Debug draw overlay |
| `IInfoCenter` | `IInfoCenter.h` | InfoCenter DB interface |
| `IEnvironmentSim` | `IEnvironmentSim.h` | Environment simulation |
| `IBatchedElementDraw` | `IBatchedElementDraw.h` | Debug batched drawing |

### Utilities in Core

| Class | Role |
|-------|------|
| `CObjScriptPool` | Per-object NPL script pool |
| `CObjectAutoReleasePool` | Frame-end auto-release for temp objects |
| `CDataProviderManager` | Data provider registry (DB tables) |
| `Variable` | Typed variable storage |
| `Array.h` | Engine array template |
| `unordered_ref_array.h` | Reference-counted object arrays |
| `NPLTypes.h` / `PEtypes.h` | Core type definitions |
| `NPLPackageConfig.h` | NPL package configuration |
| `luaSQLite.h` | Lua SQLite integration |

## BootStrapper Detail

`CBootStrapper` (`BootStrapper.h`):

```cpp
CBootStrapper::GetSingleton()->LoadFromFile("config/bootstrapper.xml");
// Reads:
//   <MainGameLoop>(gl)script/apps/MyApp/main_loop.lua</MainGameLoop>
//   <ConfigFile>config/config.safemode.txt</ConfigFile>  (optional)
```

The main loop script is activated periodically (~0.5s) and serves as the application heartbeat. All further boot logic is delegated to NPL scripts under `script/apps/`.

## Frame Rate Controllers

Four independent controllers allow subsystems to run at different rates:

| ID | Name | Used By |
|----|------|---------|
| `FRC_GAME` | Game time | Scene animation, pausable game clock |
| `FRC_SIM` | Simulation | AI simulator, NPL network tick |
| `FRC_IO` | I/O | Async asset loading callbacks |
| `FRC_RENDER` | Render | Rendering delta, asset render frame move |

When game is paused (`FRC_GAME::IsPaused()`), scene delta time in `AdvanceScene` is forced to 0.

## File Index

```
Core/
├── ParaEngine.h              Master include
├── ParaEngineAppBase.h       App base class
├── ParaEngineCore.h          Core init
├── ParaEngineService.h       Headless service
├── Globals.h / .cpp          Singleton accessors
├── BootStrapper.h / .cpp     Bootstrap XML
├── AssetManager.h            Asset template manager
├── AssetEntity.h             Ref-counted asset base
├── ContentLoaders.h          Async loader framework
├── ContentLoaderParaX.h      Model loader
├── ContentLoaderTexture.h    Texture loader
├── IAttributeFields.h          Reflection base
├── AttributeClass.h          Class metadata
├── EventsCenter.h            Event dispatch
├── Events_def.h              Event enums
├── PluginManager.h           DLL plugins
├── PluginAPI.h               Plugin contract
├── FrameRateController.h     Multi-rate timers
├── CommandLineParams.h       CLI parsing
├── TextureEntity*.h          Texture (base, DX, GL)
├── SpriteFontEntity*.h       Fonts (base, DX, GL)
└── INPL*.h, IPara*.h         Interface headers
```
