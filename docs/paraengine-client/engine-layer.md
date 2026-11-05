# Engine Layer (Client-Specific)

Location: `Client/trunk/ParaEngineClient/Engine/` (~194 files, many in `Obsoleted/`)

The Engine directory contains client-specific application code: entry points, DirectX window management, game database providers, post-processing effects, and legacy subsystems. This directory is **mostly client-only** — the server build includes only `ParaEngineServer.cpp` and `ParaXStaticBase.cpp`.

## Entry Points

| File | Function | Role |
|------|----------|------|
| `ParaWorld.cpp` | `WinMain()` | Primary Windows client entry |
| `ParaEngineServer.cpp` | `main()` | Headless server entry (also used on macOS client) |
| `ParaEngineApp.cpp` | `CParaEngineApp` | Full application: device, loop, render |
| `D3DWindowDefault.cpp/.h` | — | Default DirectX window creation |
| `OSWindows.cpp/.h` | — | Windows OS integration |
| `guicon.cpp` | — | Console window for debug builds |
| `LibInterface.cpp` | — | DLL export interface for embedding |

### WinMain Flow (ParaWorld.cpp)

```
WinMain(hInst, ...)
├── Parse command line → CParaEngineApp
├── Detect server mode (-i, -d flags)
├── MULTITHREADED_APP_WINDOW or CUSTOM_APP_WINDOW mode
│   ├── CParaEngineApp creates HWND + message loop, OR
│   └── Host app provides HWND, forwards MsgProc
└── CParaEngineApp::Run(hInstance)
```

Embedding modes documented in [EmbeddingNPLRuntime wiki](https://github.com/LiXizhi/NPLRuntime/wiki/EmbeddingNPLRuntime).

## CParaEngineApp

`Engine/ParaEngineApp.cpp` — the main client application class (extends `CParaEngineAppBase` + DirectX application base).

Key responsibilities:
- DirectX 9 device creation and reset (`m_d3dpp`, backbuffer)
- Viewport manager (`m_pViewportManager`)
- Root scene pointer (`m_pRootScene`)
- Main loop: `FrameMove()` + `Render()`
- Window mode, resolution, MSAA settings
- App focus handling (lower frame rate when unfocused)
- Server mode flag (`m_bServerMode` — skip rendering)
- Movie capture (`CMoviePlatform`)

See [rendering-pipeline.md](rendering-pipeline.md) for render flow details.

## Post-Processing Effects

Client-specific visual effects in Engine/:

| Class | File | Effect |
|-------|------|--------|
| `GlowEffect` | `GlowEffect.h` | Full-screen bloom/glow |
| `WaveEffect` | `WaveEffect.h/.cpp` | Screen wave distortion |
| `DropShadowRenderer` | `DropShadowRenderer.cpp` | Character drop shadows |
| `OcclusionQueryBank` | `OcclusionQueryBank.h` | GPU occlusion culling |
| `EdgeBuilder` | `EdgeBuilder.h` | Edge detection/outline |
| `DynamicRenderable` | `DynamicRenderable.cpp` | Dynamic mesh rendering |
| `DirectXEngine` | `DirectXEngine.cpp` | DirectX-specific engine helpers |

Invoked from `CSceneObject::RenderFullScreenGlowEffect()`, `RenderScreenWaveEffect()`, etc.

## Game Database Providers

Legacy MMO/RPG data providers (used by Paracraft and earlier games):

| Provider | File | Data |
|----------|------|------|
| `CharacterDBProvider` | `CharacterDBProvider.cpp` | Character templates |
| `ItemDBProvider` | `ItemDBProvider.cpp` | Item definitions |
| `NpcDatabase` | `NpcDatabase.cpp` | NPC templates |
| `QuestDBProvider` | `QuestDBProvider.cpp` | Quest data |
| `PetDBProvider` | `PetDBProvider.cpp` | Pet definitions |
| `PetAIDBProvider` | `PetAIDBProvider.h` | Pet AI behaviors |
| `ChestDBProvider` | `ChestDBProvider.cpp` | Treasure chest data |
| `FruitDBProvider` | `FruitDBProvider.h` | Collectible items |
| `KidsDBProvider` | `KidsDBProvider.h` | Kid-friendly content |
| `PuzzleDBProvider` | `PuzzleDBProvider.h` | Puzzle definitions |
| `TitleDBProvider` | `TitleDBProvider.cpp` | Player titles |
| `StringTable` | `StringTable.cpp` | Localized strings |
| `Modifier` | `Modifier.cpp` | Game modifiers/buffs |

These load data tables (often SQLite or XML) and expose them to NPL scripts via `DataProviderManager` in Core.

## Browser & Media

| Class | Role |
|-------|------|
| `HTMLBrowserManager` | Embedded IE browser manager |
| `FlashTextureManager` | Flash SWF rendering to texture |
| `FlashPlayer` bindings | Legacy Flash in GUI |
| `SpriteObject` | 2D sprite entities in 3D scene |
| `GuiConsole` | In-game debug console |
| `EditorHelper` | Scene editor utilities |

## VoxelMesh (Client DirectX build)

`Engine/VoxelMesh.h` — also compiled from `VoxelMesh/` directory in DirectX builds:
- Isosurface voxel meshing
- Metaball rendering
- Alternative to BlockEngine for smooth voxel surfaces

## Obsoleted Subsystems

`Engine/Obsoleted/` contains deprecated code kept for reference:

| Directory | Contents |
|-----------|----------|
| `Video/` | DirectShow video playback headers |
| `ParaGDI/` | Legacy GDI rendering |
| `AudioEngine.cpp` | Old audio engine (replaced by cAudio) |
| `RpgCharacter.*` | Old RPG character system |
| `GUICanvas.h` | Legacy canvas widget |
| `EnvSimServer.h` | Old environment sim server |
| `XFileAnimInstance.h` | Old X file animation |
| `ICGen.*` | Old InfoCenter generator |
| `ShapePlane.*` | Debug shape plane |
| `EmuCSParaEditGame.h` | Old editor game mode |

These are not compiled in modern builds but remain in source tree.

## Debug & Development

| File | Role |
|------|------|
| `vslua_debugger.h` | Visual Studio Lua debugger integration |
| `config.h` | Engine compile-time config |
| `resource.h` | Windows resource IDs |
| `ValueTracker.h` | Debug value tracking |

## Platform-Specific

| File | Platform | Role |
|------|----------|------|
| `D3DWindowDefault.*` | Windows DX | Default D3D window |
| `OSWindows.*` | Windows | OS utilities |
| `guicon.*` | Windows | Console attachment |
| `platform/mac/mac_main.cpp` | macOS | macOS entry (in platform/) |
| `platform/win32/*` | Windows GL | Win32 + GLAD for OpenGL on Windows |

## Static Linking Base

`ParaXStaticBase.cpp/.h` — static initialization for ParaX model system; compiled in both client and server builds.

## Relationship to Other Modules

```
Engine/ (this layer)
    ├── uses Core/ for app base, globals, assets
    ├── uses 3dengine/ for scene rendering
    ├── uses 2dengine/ for GUI
    ├── uses renderer/ for device
    ├── uses NPL/ for scripting tick
    └── provides entry points that wire everything together
```

Game logic itself lives in NPL scripts (`script/apps/`), not in Engine/. The DB providers supply data; scripts supply behavior.

## File Index (Active Files)

```
Engine/
├── ParaWorld.cpp              WinMain entry
├── ParaEngineServer.cpp       main() entry (server/mac)
├── ParaEngineApp.cpp          Main application class
├── ParaXStaticBase.cpp        Shared static init
├── D3DWindowDefault.h/.cpp    D3D window
├── OSWindows.h/.cpp           Windows OS
├── DirectXEngine.cpp          DX helpers
├── GlowEffect.h               Bloom post-effect
├── WaveEffect.h/.cpp          Wave distortion
├── DropShadowRenderer.cpp     Drop shadows
├── HTMLBrowserManager.cpp     Browser manager
├── FlashTextureManager.cpp    Flash textures
├── SpriteObject.h/.cpp        2D sprites
├── GuiConsole.h/.cpp          Debug console
├── EditorHelper.cpp           Editor tools
├── CharacterDBProvider.cpp    Character DB
├── ItemDBProvider.cpp           Item DB
├── NpcDatabase.cpp              NPC DB
├── QuestDBProvider.cpp          Quest DB
├── StringTable.cpp              Localization
├── Modifier.cpp                 Game modifiers
├── LibInterface.cpp             DLL exports
└── Obsoleted/                   Deprecated code (not compiled)
```
