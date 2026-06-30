# NPLRuntime Documentation

Technical documentation for the NPLRuntime / ParaEngine codebase. These docs are intended for developers and AI agents reviewing or modifying the project.

## Quick Links

| Document | Description |
|----------|-------------|
| [Architecture Overview](architecture.md) | High-level system design, dual build paths, data flow |
| [NPL Runtime](npl-runtime.md) | Scripting language, activation model, networking, threading |
| [Scene Graph & 3D Engine](scene-graph-and-3d-engine.md) | Scene management, objects, cameras, physics, terrain |
| [Renderer](renderer.md) | DirectX/OpenGL abstraction, shaders, render pipeline |
| [Script Bindings](script-bindings.md) | Lua/NPL API namespaces exposed to scripts |
| [Build & Deploy](build-and-deploy.md) | CMake, platforms, CI, output layout |
| [Components](components.md) | Full directory map of `ParaEngineClient/` |

## External Resources

- [Project README](../README.md) — install and quick start
- [Install Guide (Wiki)](https://github.com/LiXizhi/NPLRuntime/wiki/InstallGuide)
- [Source Code Overview (Wiki)](https://github.com/LiXizhi/NPLRuntime/wiki/SourceCodeOverview)
- [What is NPL? (Wiki)](https://github.com/LiXizhi/NPLRuntime/wiki/WhatIsNPL)
- [NPL Docs (ReadTheDocs)](http://docs.paraengine.com)
- [Code Docs (Doxygen)](https://codedocs.xyz/LiXizhi/NPLRuntime)

## Legacy Internal Docs

Older documentation lives under `Client/trunk/ParaEngineClient/doc/`:

| File | Topic |
|------|-------|
| `ParaEngineBootStrapping` | Bootstrapper XML and boot order |
| `EventsReference.txt` | Event system (mouse, key, UI, network) |
| `NPLDebugging` | NPL debugging notes |
| `ReadMe.txt` | Legacy 2011 source overview |
| `NPL/NPL_readme.txt` | NPL network layer call graph |

## For AI Agents

See [`CLAUDE.md`](../CLAUDE.md) at the repository root for a concise agent-oriented guide with conventions, constraints, and common task patterns.
