# ParaEngineClient Deep Reference

`Client/trunk/ParaEngineClient/` is the primary C++ source tree (~1,500 files). Both the GUI client (`npl`) and headless server (`npls`) compile subsets of this directory.

This folder documents each major subsystem in detail, synthesizing source code analysis with the [NPLRuntime GitHub Wiki](https://github.com/LiXizhi/NPLRuntime/wiki) and legacy internal docs under `Client/trunk/ParaEngineClient/doc/`.

## Documents

| Document | Subsystem | Key Classes |
|----------|-----------|-------------|
| [Overview](overview.md) | Architecture, frame loop, embedding | `CParaEngineApp`, `CSceneObject`, `CGUIRoot` |
| [Core](core.md) | Engine kernel, assets, bootstrap, events | `CGlobals`, `AssetManager`, `CBootStrapper` |
| [3D Engine](3dengine.md) | Scene graph, cameras, characters, physics | `CBaseObject`, `CSceneObject`, `CBipedObject` |
| [2D GUI](2dengine-gui.md) | Widgets, input, IME, touch | `CGUIBase`, `CGUIRoot`, `CGUIButton` |
| [Rendering Pipeline](rendering-pipeline.md) | Per-frame render order, shaders | `AdvanceScene`, `PIPELINE_*`, `EffectManager` |
| [Block Engine](block-engine.md) | Voxel block world (Paracraft-style) | `CBlockWorld`, `BlockRegion`, `BlockChunk` |
| [ParaX Model & Assets](parax-model-and-assets.md) | 3D models, textures, async loading | `ParaXEntity`, `AssetManager`, `ContentLoader*` |
| [IO & Filesystem](io-and-filesystem.md) | Virtual FS, zip, async I/O | `CParaFile`, `CArchive`, `CZipArchive` |
| [Engine Layer](engine-layer.md) | Client entry, game DB providers, effects | `ParaWorld.cpp`, `*DBProvider`, post-effects |
| [NPL Integration](../npl-runtime.md) | Scripting runtime (shared module) | `CNPLRuntime`, `CNPLRuntimeState` |
| [Script Bindings](../script-bindings.md) | Lua API surface | `LoadHAPI_*`, `ParaScripting*.cpp` |

## CMake Source Groups

The authoritative list of which files compile into the client is `NPLRuntime/ParaEngineClient/CMakeLists.txt`. Source groups mirror the directory layout:

```
Core, Engine, NPL, 3dengine, 2dengine, renderer, ParaXModel,
terrain, BlockEngine, VoxelMesh, IO, ParaScriptBindings,
PaintEngine, math, util, protocol, Framework, AutoRigger,
WebSocket, dirmonitor, curllua, debugtools, ic, BMaxModel, CadModel
```

Client-only additions (DirectX build): full `Engine/`, `VoxelMesh/`, `CadModel/`, shader compilation (`fxc`), Windows resources (`res/`), DirectX utilities (`common/`).

## External Wiki References

| Wiki Page | Topic |
|-----------|-------|
| [Source Code Overview](https://github.com/LiXizhi/NPLRuntime/wiki/SourceCodeOverview) | Historical directory layout |
| [3D Overview](https://github.com/LiXizhi/NPLRuntime/wiki/3DOverview) | Render pipeline, shader customization |
| [NPLRuntimeAPI](https://github.com/LiXizhi/NPLRuntime/wiki/NPLRuntimeAPI) | C++ module map exposed to NPL |
| [Embedding NPLRuntime](https://github.com/LiXizhi/NPLRuntime/wiki/EmbeddingNPLRuntime) | DLL/static embedding for host apps |
| [Install Guide](https://github.com/LiXizhi/NPLRuntime/wiki/InstallGuide) | Build, debug, ParacraftSDK integration |
| [Tutorial HelloWorld](https://github.com/LiXizhi/NPLRuntime/wiki/TutorialHelloWorld) | First NPL script |

## Legacy Internal Docs

Located at `Client/trunk/ParaEngineClient/doc/`:

| File | Content |
|------|---------|
| `ParaEngineBootStrapping` | Bootstrapper XML format and boot order |
| `EventsReference.txt` | Mouse, key, UI, editor, network events |
| `NPLDebugging` | NPL debugging techniques |
| `config.txt` | Default startup settings |
| `NamespaceBind.txt` | NID DNS namespace examples |
| `_ParaMain`, `_ParaNPL`, `_ParaTerrain` | Doxygen configs for HTML/CHM reference |
