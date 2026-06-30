# Script Bindings (Lua/NPL API)

ParaEngine exposes its C++ functionality to NPL/Lua scripts via **luabind** bindings in `Client/trunk/ParaEngineClient/ParaScriptBindings/` (~52 files).

## Loading Sequence

During runtime state initialization, `CNPLScriptingState` calls loader functions that register API namespaces:

```cpp
LoadHAPI_NPL();              // NPL core API
LoadHAPI_SceneManager();     // ParaScene
LoadHAPI_ResourceManager();  // ParaAsset
LoadHAPI_Globals();          // ParaGlobal, Config
LoadHAPI_UI();               // ParaUI, ParaPainter
LoadHAPI_Audio();            // ParaAudio
LoadHAPI_Network();          // ParaNetwork
LoadHAPI_AI();               // AI bindings
// ... additional loaders
```

Each loader registers one or more Lua namespaces via luabind's `module(L) [ namespace_("Name") ]` pattern.

## API Namespaces

### Core Engine

| Namespace | Source File | Key Functions |
|-----------|------------|---------------|
| `NPL` | `ParaScripting3.cpp` | `activate()`, `StartNetServer()`, `AddPublicFile()`, `CreateRuntimeState()`, timers, channels |
| `ParaEngine` | `ParaScripting5.cpp` | Engine settings, frame rate, app control |
| `ParaGlobal` | `ParaScripting5.cpp` | Global variables, logging, time |
| `Config` | `ParaScripting2.cpp` | Runtime configuration read/write |
| `ParaBootStrapper` | `ParaScripting5.cpp` | Bootstrapper XML loading |
| `ParaMisc` | `ParaScripting2.cpp` | Miscellaneous utilities |

### Scene & 3D

| Namespace | Source File | Key Functions |
|-----------|------------|---------------|
| `ParaScene` | `ParaScripting.cpp` | Scene objects, events, picking, object creation/deletion |
| `ParaCamera` | `ParaScripting3.cpp` | Camera control, follow, orbit |
| `ParaTerrain` | `ParaScripting3.cpp` | Terrain height editing, texture splatting, LOD |
| `ParaWorld` | `ParaScripting4.cpp` | World-level operations |
| `ParaBlockWorld` | `ParaScripting4.cpp` | Voxel block placement/removal |
| `ParaSelection` | `ParaScripting4.cpp` | Object selection and picking |
| `ParaAsset` | `ParaScripting3.cpp` | Asset loading (`LoadParaX`, textures, effects) |

### 2D & UI

| Namespace | Source File | Key Functions |
|-----------|------------|---------------|
| `ParaUI` | `ParaScripting2.cpp` | GUI creation (buttons, sliders, scrollbars, windows) |
| `ParaPainter` | `ParaScripting2.cpp` | 2D drawing (lines, rects, text) |
| `ParaMovie` | `ParaScripting5.cpp` | Flash/SWF movie playback |

### I/O & Data

| Namespace | Source File | Key Functions |
|-----------|------------|---------------|
| `ParaIO` | `ParaScripting5.cpp` | File I/O, virtual file system |
| `ParaXML` | `ParaScripting5.cpp` | XML parsing and generation |
| `ParaIPC` | `ParaScripting3.cpp` | Inter-process communication |

### Network & Communication

| Namespace | Source File | Key Functions |
|-----------|------------|---------------|
| `ParaNetwork` | `ParaScripting2.cpp` | HTTP requests, network utilities |
| `ParaBrowserManager` | `ParaScripting4.cpp` | Embedded HTML browser control |
| `JabberClientManager` | `ParaScripting4.cpp` | Jabber/XMPP instant messaging |

### Audio

| Namespace | Source File | Key Functions |
|-----------|------------|---------------|
| `ParaAudio` | `ParaScripting2.cpp` | Sound playback, 3D audio, music |

### Specialized Bindings

Dedicated files for subsystem-specific APIs:

| File | Domain |
|------|--------|
| `ParaScriptingCharacter.cpp/.h` | Character animation, bone controllers |
| `ParaScriptingGraphics.cpp/.h` | Low-level graphics operations |
| `ParaScriptingTerrain.cpp/.h` | Advanced terrain operations |
| `ParaScriptingBlockWorld.cpp/.h` | Block world scripting |
| `ParaScriptingHTMLBrowser.cpp/.h` | HTML browser widget |
| `ParaScriptingFlashPlayer.cpp/.h` | Flash player integration |
| `ParaScriptingMovie.cpp/.h` | Movie playback |
| `ParaScriptingIC.cpp/.h` | InfoCenter database |
| `ParaScriptingTest.cpp/.h` | Test/debug utilities |

## NPL API Reference (Key Functions)

### Activation

```lua
-- Local activation
NPL.activate("(gl)script/handler.npl", {action="move", target=obj})

-- Remote activation
NPL.activate("server_nid", "(gl)script/handler.npl", {data="hello"})

-- Activate with runtime state
NPL.activate("(main)", "(gl)script/worker.npl", {task="process"})
```

### Network Server

```lua
NPL.StartNetServer("0.0.0.0", "60001")
NPL.AddPublicFile("script/server/handler.lua", 1)
NPL.AddNPLRuntimeAddress({host="192.168.1.100", port="60001", nid="game_server"})
NPL.accept(temporary_tid, authenticated_nid)
```

### Runtime States

```lua
local worker = NPL.CreateRuntimeState("worker_1", 0)
worker:Start()
-- Activate on worker:
NPL.activate("worker_1", "(gl)script/task.npl", {data="process"})
```

### Timers

```lua
NPL.SetTimer(callback_file, interval_ms, data)
NPL.KillTimer(timer_id)
```

## Scene API Reference (Key Functions)

### Object Management

```lua
-- Create object
local obj = ParaScene.CreateObject("mesh", "models/tree.x")
obj:SetPosition(100, 0, 200)
ParaScene.AttachObject(obj)

-- Find objects
local obj = ParaScene.GetObject("MyObjectName")
local objs = ParaScene.FindObjectsByTag("enemy")

-- Events
ParaScene.RegisterEvent("onmousedown", "script/handler.npl")
```

See `Client/trunk/ParaEngineClient/doc/EventsReference.txt` for the full event list.

## Object Scripting Interface

`IObjectScriptingInterface` — Generic per-object scripting bridge allowing any `CBaseObject` to expose methods and properties to Lua without dedicated binding files.

`local_bridge_manual.cpp` — Hand-written bindings for types that don't fit the auto-generation pattern.

## Adding New Bindings

1. Choose the appropriate `ParaScripting*.cpp` file (or create a new one)
2. Add luabind registration:

```cpp
void LoadHAPI_MyFeature(NPL::CNPLScriptingState* pState) {
    lua_State* L = pState->GetLuaState();
    module(L) [
        namespace_("ParaMyFeature") [
            def("DoSomething", &MyClass::DoSomething),
            class_<MyClass>("MyClass")
                .def("Method", &MyClass::Method)
        ]
    ];
}
```

3. Register the loader in `CNPLScriptingState` initialization
4. Add the `.cpp` file to CMakeLists.txt source list

## Binding Technology

- **luabind 0.9** (0.9.2beta on macOS) — C++ template library for Lua bindings
- **Lua 5.1 / LuaJIT** — Underlying script VM
- Bindings use luabind's `def`, `class_`, `property`, `enum_` macros

## Event System

ParaEngine uses an event-driven model for input and lifecycle:

| Event Category | Examples |
|---------------|---------|
| Mouse | `onmousedown`, `onmouseup`, `onclick`, `onmousemove` |
| Keyboard | `onkeydown`, `onkeyup`, `onkeypress` |
| UI | `onframemove`, `oncreate`, `ondestroy` |
| Network | Connection events, message received |
| Editor | Object selection, transform gizmos |

Register via `ParaScene.RegisterEvent(event_name, script_file)`.

Full reference: `Client/trunk/ParaEngineClient/doc/EventsReference.txt`

## File Organization

```
ParaScriptBindings/
├── ParaScripting.cpp          # ParaScene
├── ParaScripting2.cpp         # ParaAudio, ParaNetwork, ParaUI, ParaPainter, Config
├── ParaScripting3.cpp         # ParaTerrain, ParaCamera, ParaAsset, NPL, ParaIPC
├── ParaScripting4.cpp         # ParaWorld, ParaBlockWorld, ParaSelection, Browser, Jabber
├── ParaScripting5.cpp         # ParaXML, ParaGlobal, ParaBootStrapper, ParaEngine, ParaIO
├── ParaScriptingCharacter.cpp # Character animation
├── ParaScriptingGraphics.cpp  # Graphics operations
├── ParaScriptingTerrain.cpp   # Terrain operations
├── ParaScriptingBlockWorld.cpp# Block world
├── ParaScriptingHTMLBrowser.cpp
├── ParaScriptingFlashPlayer.cpp
├── ParaScriptingMovie.cpp
├── ParaScriptingIC.cpp
├── ParaScriptingTest.cpp
├── local_bridge_manual.cpp    # Manual bindings
└── ParaScripting.h            # Common headers and loader declarations
```
