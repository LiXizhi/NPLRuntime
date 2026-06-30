# NPL Ecosystem

NPLRuntime is only the C++ runtime layer. The full NPL ecosystem includes the scripting language, standard libraries, application packages, editors, and debugging tools documented on the [NPLRuntime Wiki](https://github.com/LiXizhi/NPLRuntime/wiki) and at http://docs.paraengine.com/.

This document captures wiki-derived context for understanding how the C++ engine (`ParaEngineClient`) relates to the script and tool layers above it.

---

## What is NPL?

**NPL (Neural Parallel Language)** is a general-purpose scripting language with **100% Lua 5.1 syntax compatibility**, extended with:

- Message-passing **activation** between neuron files (local threads and remote peers)
- User-mode **preemptive** and **non-preemptive** code mixing
- Built-in **ParaEngine** 3D/2D/game engine (this C++ codebase)
- Rich C/C++ native API exposed via luabind

Design goals (from [WhatIsNPL wiki](https://github.com/LiXizhi/NPLRuntime/wiki/WhatIsNPL)):

- Brain-inspired concurrency: neurons (files) + connections (activations), no shared memory, no callbacks
- Single language for client GUI, 3D graphics, web server, database, distributed frameworks
- Cross-platform deployment smaller than Java/C#/Electron
- Near C/C++ performance via LuaJIT + zero-cost C++ binding

### Compared to plain Lua

| Aspect | Lua | NPL |
|--------|-----|-----|
| Syntax | Baseline | 100% compatible |
| Standalone runtime | Embedding-focused | Full general-purpose runtime |
| Concurrency | Coroutines only | Multi-thread + network activation |
| 3D engine | None built-in | ParaEngine included |
| Libraries | External | 1M+ lines open-source NPL libs |

---

## Neuron File Model

Every `.npl` or `.lua` file can become a **neuron file** — an addressable message handler:

```lua
-- helloworld.npl
NPL.activate("(gl)helloworld.npl", {data="hello world!"})
this(msg) {
   if msg then print(msg.data or "") end
}
```

Equivalent explicit form:

```lua
local function activate()
   if msg then print(msg.data or "") end
end
NPL.this(activate)
```

### Activation addressing

| Target | Syntax | Meaning |
|--------|--------|---------|
| Main thread | `(gl)path/file.npl` | Global runtime state |
| Worker | `(worker1)path/file.npl` | Named thread |
| Remote | `(192.168.1.1:8080)path/file.npl` | TCP peer |
| DNS | `(gl)path/file.npl@myserver` | Resolved via NPL DNS |

Communication is **async, one-direction, no callback** — like neural synapses.

---

## Runtime Executables

| Command | Binary | Mode |
|---------|--------|------|
| `npl [script] [args]` | `ParaEngineClient.exe` / platform client | GUI client |
| `npls [script] [args]` | `ParaEngineServer` | Headless server |

Linux shebang support:

```lua
#!/usr/local/bin/npl
print("hello world")
ParaGlobal.Exit(0)
```

---

## NPL Package System

NPL libraries are distributed as:

1. **Source folders** in `npl_packages/` (development)
2. **`.pkg` zip archives** (deployment)
3. **Precompiled bytecode** in `packages/` (fast load)

### Core open-source packages

| Package | Repository | Content |
|---------|------------|---------|
| `main` | https://github.com/NPLPackages/main | NPL standard library |
| `paracraft` | https://github.com/NPLPackages/paracraft | Paracraft game source |

Clone into app root:

```bash
cd npl_packages
git clone https://github.com/NPLPackages/main.git
git clone https://github.com/NPLPackages/paracraft.git
```

More packages: https://github.com/NPLPackages

---

## Standard Library Layout (`main` package)

From [SourceCodeOverview wiki](https://github.com/LiXizhi/NPLRuntime/wiki/SourceCodeOverview):

### Core

| Path | Purpose |
|------|---------|
| `script/ide/commonlib.lua` | Common includes (no GUI) |
| `script/ide/System/` | Core system library |
| `script/ide/STL/` | Data structures (list, map, etc.) |
| `script/ide/timer.lua` | Timer utilities |
| `script/ide/math/` | Vector, matrix math (script-side) |

### Object orientation

| Path | Purpose |
|------|---------|
| `script/ide/oo.lua` | Class inheritance |
| `script/ide/System/Core/ToolBase.lua` | Base class with signals/properties |

### Web server (NPL Code Wiki backend)

| Path | Purpose |
|------|---------|
| `script/apps/WebServer/WebServer.lua` | HTTP server entry |
| `script/apps/WebServer/admin/` | PHP-like web framework in NPL |

Start standalone:

```bash
npl script/apps/WebServer/WebServer.lua
# http://127.0.0.1:8099/
```

---

## Paracraft Script Layout

Paracraft is an IDE and game written entirely in NPL. Its source maps to ParaEngine C++ subsystems:

| NPL path | C++ counterpart | Content |
|----------|-----------------|---------|
| `script/apps/Aries/Creator/Game/` | Overall game loop | Main game logic |
| `.../Areas/` | `2dengine/GUIRoot` | Desktop GUI |
| `.../Entity/` | `3dengine/BaseObject` | Movable entities |
| `.../blocks/` | `BlockEngine/BlockTemplate` | Block type definitions |
| `.../Commands/` | NPL command handlers | In-game commands |
| `.../Tasks/` | GUI + commands | Commands with UI |
| `.../GUI/` | `2dengine/*` | Editor interfaces |
| `.../Network/` | `NPL/NPLNetServer` | Multiplayer |
| `.../Shaders/` | `shaders/`, `EffectManager` | GPU shaders |
| `.../Mod/` | Plugin API | Mod/plugin interface |
| `.../SceneContext/` | Input routing | Keyboard/mouse |

Paracraft is the primary consumer of `BlockEngine/`, `BMaxModel/`, and voxel rendering paths in ParaEngineClient.

---

## Development Environment

### Recommended setup (Windows)

1. Clone [ParaCraftSDK](https://github.com/LiXizhi/ParaCraftSDK)
2. Run `redist\paracraft.exe` — installs Paracraft to `./redist`
3. Run `NPLRuntime\install.bat` — copies runtime to `./NPLRuntime/win/bin`, adds to PATH

### Editors and plugins

| Tool | Plugin | Purpose |
|------|--------|---------|
| Visual Studio | [NPL Language Service](https://visualstudiogallery.msdn.microsoft.com/7782dc20-924a-4726-8656-d876cdbb3417) | Syntax, IntelliSense |
| Visual Studio | [NPL Debugger](https://visualstudiogallery.msdn.microsoft.com/7ebe665c-4f1d-41fd-91e1-52176cf2d9db) | Attach to running process |
| VS Code | [vscode-npl-debug](https://github.com/tatfook/vscode-npl-debug) | Cross-platform debug |
| Any browser | NPL Code Wiki (built-in) | HTTP debugger/editor |

### NPL Code Wiki features

Built into runtime — see [NPLCodeWiki wiki](https://github.com/LiXizhi/NPLRuntime/wiki/NPLCodeWiki):

| Feature | Access | Notes |
|---------|--------|-------|
| HTTP debugger | Attach button | F10/F11 step; main thread only |
| Code editor | Menu → View → Code Editor | Multi-tab, Ctrl+S save |
| Object inspector | Menu → View → Object Inspect | Edit runtime objects |
| Console | F11 in Paracraft | Evaluate code at runtime |
| Log viewer | Menu → Tools → Log | Tails `log.txt` |

Remote debugging: point browser at device's Code Wiki URL (requires source on device).

### LuaJIT tail call caveat

When debugging with LuaJIT, tail calls lose stack frames. Wrap tail calls:

```lua
-- Wrong (tail call — stack lost):
return print_error_call_stack()

-- Right (preserves stack):
return (print_error_call_stack())
```

---

## Application Deployment Structure

```
AppRoot/                     ← always set as working directory
├── bin/                     ← 32-bit npl + DLLs
├── bin64/                   ← 64-bit npl + DLLs
├── packages/                ← precompiled NPL libs
├── script/                  ← your scripts
├── config/                  ← engine XML config
├── npl_packages/            ← dev: package source
├── database/                ← SQLite (optional)
├── *.pkg                    ← runtime packages
└── paraengine.sig
```

Deploy `bin/` + `packages/` + your scripts together. Scripts reference paths relative to AppRoot.

---

## Built-in C++ Plugins

Shipped with NPLRuntime (see [plugins.md](plugins.md)):

| Plugin | Technology | NPL access |
|--------|------------|------------|
| Bullet physics | PhysicsBT | Scene/character APIs |
| OpenAL audio | cAudioEngine | `ParaScriptingAudio` |
| SQLite | ParaSqlite | InfoCenter, direct SQL |
| Mono/.NET | NPLMono2 | C# interop |
| libcurl | curllua | HTTP client |
| assimp | FBX/OBJ/COLLADA | Model loading |

Optional: MySQL, PostgreSQL (Linux builds), OpenCASCADE CAD.

---

## Learning Path

Recommended order for new developers:

1. [TutorialHelloWorld](https://github.com/LiXizhi/NPLRuntime/wiki/TutorialHelloWorld) — run first script
2. [WhatIsNPL](https://github.com/LiXizhi/NPLRuntime/wiki/WhatIsNPL) — language philosophy
3. [SourceCodeOverview](https://github.com/LiXizhi/NPLRuntime/wiki/SourceCodeOverview) — NPL script layout
4. [paraengine-client.md](paraengine-client.md) — C++ engine deep dive (this repo)
5. Clone `NPLPackages/main` + `NPLPackages/paracraft` — read real code
6. [ParacraftSDK wiki](https://github.com/LiXizhi/ParaCraftSDK/wiki) — mod/plugin tutorials
7. [NPL Code Wiki](https://github.com/LiXizhi/NPLRuntime/wiki/NPLCodeWiki) — live debugging

---

## External Resources

| Resource | URL |
|----------|-----|
| NPLRuntime Wiki | https://github.com/LiXizhi/NPLRuntime/wiki |
| ParaEngine Docs (RTD) | http://docs.paraengine.com/ |
| NPL Packages org | https://github.com/NPLPackages |
| ParacraftSDK | https://github.com/LiXizhi/ParaCraftSDK |
| Paracraft | http://www.paracraft.cn |
| NPL QQ group | 543474853 |

---

## Related

- [paraengine-client.md](paraengine-client.md) — C++ ParaEngineClient deep reference
- [npl-runtime.md](npl-runtime.md) — CNPLRuntime C++ internals
- [architecture.md](architecture.md) — system architecture
