# NPLRuntime Documentation

Internal architecture and component reference for the NPLRuntime / ParaEngine codebase. These docs supplement the public wiki and [INSTALL.md](../INSTALL.md).

## Document Index

| Document | Description |
|----------|-------------|
| [paraengine-client.md](paraengine-client.md) | **Deep dive:** legacy `Client/trunk/ParaEngineClient` → modern `NPLRuntime/ParaEngine`, every module, startup flow |
| [npl-ecosystem.md](npl-ecosystem.md) | NPL language, packages, Paracraft scripts, Code Wiki, wiki-derived context |
| [architecture.md](architecture.md) | High-level system design, data flow, client vs server |
| [build-system.md](build-system.md) | CMake options, targets, build scripts, output paths |
| [npl-runtime.md](npl-runtime.md) | NPL language runtime: activation, threading, networking |
| [paraengine-subsystems.md](paraengine-subsystems.md) | ParaEngine modules: 3D, 2D, terrain, blocks, assets, IO |
| [render-system.md](render-system.md) | Render backends (OpenGL, D3D9, Null) and draw pipeline |
| [platform.md](platform.md) | Platform entry points and bootstrap per OS |
| [plugins.md](plugins.md) | Optional plugins: physics, audio, SQLite, Mono |
| [conventions.md](conventions.md) | Naming, memory, threading, and API binding patterns |

## Quick Reference

### Executables

| Binary | Build | Purpose |
|--------|-------|---------|
| `ParaEngineClient.exe` | Windows client | Full GUI client (DirectX or OpenGL) |
| `ParaEngineServer` | `NPLRUNTIME_SERVER=ON` | Headless NPL server (`npls`) |
| `ParaCraftSDL2` | Linux/Win + SDL | Cross-platform desktop client |
| `ParaCraft` / `ParaCraftSingleThread` | Emscripten | WebAssembly client |
| iOS / Android / HarmonyOS apps | Mobile CMake | Native mobile shells |

### Core Singletons

| Accessor | Class | Role |
|----------|-------|------|
| `CNPLRuntime::GetInstance()` | `CNPLRuntime` | Script runtime, networking, activation |
| `CParaWorldAsset::GetSingleton()` | `CParaWorldAsset` | Textures, meshes, fonts, effects |
| `CParaEngineAppBase::GetInstance()` | `IParaEngineApp` | Application frame loop |
| `CGlobals::GetScene()` | `CSceneObject` | 3D scene graph |

### Source Tree (non-externals)

```
NPLRuntime/                          # modern root (was Client/trunk/ParaEngineClient/)
├── ParaEngine/                      # engine core (was .../ParaEngineClient/ParaEngine/)
├── RenderSystem/                    # GPU backends
├── Platform/Windows/                # WinMain → ParaEngineClient.exe
├── Plugins/                         # physics, audio, sqlite
├── ParaEngineClientApp/             # DLL loader + IPC harness
└── cmake/                           # build helpers
```

> Legacy path `Client/trunk/ParaEngineClient/ParaEngine/` maps 1:1 to `NPLRuntime/ParaEngine/`. See [paraengine-client.md](paraengine-client.md).

## Related Files at Repository Root

- [CLAUDE.md](../CLAUDE.md) — AI assistant quick-start guide
- [README.md](../README.md) — Project overview and hello-world example
- [INSTALL.md](../INSTALL.md) — Platform install and build instructions
- [.github/copilot-instructions.md](../.github/copilot-instructions.md) — Copilot development guide

## External Documentation

- [NPLRuntime Wiki](https://github.com/LiXizhi/NPLRuntime/wiki/)
- [Source Code Overview (Wiki)](https://github.com/LiXizhi/NPLRuntime/wiki/SourceCodeOverview)
- [ParaEngine Docs (ReadTheDocs)](http://docs.paraengine.com/)
