# NPL Runtime

The NPL (Neural Parallel Language) runtime is the scripting and concurrency core of NPLRuntime. It provides Lua 5.1–compatible execution with message-passing activation across threads and network peers.

## Overview

NPL extends Lua with:

- **Activation** — asynchronous message delivery to `.npl` neuron files
- **Runtime states** — isolated Lua environments, each optionally on its own thread
- **Distributed networking** — activate scripts on remote machines via TCP/UDP
- **Timers, channels, compression** — built into the runtime

All implementation lives under `NPLRuntime/ParaEngine/NPL/`.

## Key Classes

| Class | Header | Role |
|-------|--------|------|
| `CNPLRuntime` | `NPL/NPLRuntime.h` | Singleton runtime; Init/Run/Cleanup, networking |
| `CNPLRuntimeState` | `NPL/NPLRuntimeState.h` | Per-thread Lua state + message queue |
| `CNPLScriptingState` | `NPL/NPLScriptingState.h` | Lua stack operations, file loading |
| `CNPLDispatcher` | `NPL/NPLDispatcher.h` | Routes local vs remote activations |
| `CNPLNetServer` | `NPL/NPLNetServer.h` | TCP server for remote activation |
| `CNPLNetClient` | `NPL/NPLNetClient.h` | TCP client connections |
| `CNPLNetUDPServer` | `NPL/NPLNetUDPServer.h` | UDP activation channel |
| `CNPLConnection` | `NPL/NPLConnection.h` | Active network connection |
| `CNPLMessageQueue` | `NPL/NPLMessageQueue.h` | Per-state message FIFO |
| `CNeuronFileState` | `NPL/NeuronFileState.h` | Loaded neuron file tracking |

Public interface: `ParaEngine/Framework/Interface/INPLRuntime.h`, `INPLRuntimeState.h`.

## Lifecycle

```cpp
CNPLRuntime* runtime = CNPLRuntime::GetInstance();
runtime->Init();
// each frame (client):
runtime->Run(/* bToEnd */ false);
runtime->Cleanup();
```

`CNPLRuntime::Run()` processes:

1. Timer callbacks on all active states
2. Web service (HTTP) async results
3. Message queues for main-thread-hosted states (`Process()`)
4. Preemptive function scheduling

## Runtime States

A `CNPLRuntimeState` owns:

- A Lua VM stack (`CNPLScriptingState`)
- A message queue (`CNPLMessageQueue`)
- Timers, debug hooks, neuron file registry
- Optional dedicated thread

### State types

`NPLRuntimeStateType` (see `NPLCommon.h`):

- `NPLRuntimeStateType_NPL` — standard NPL state (default)
- External Lua states can be injected via `SetDefaultNPLStateType()` before `GetInstance()`

### Threading modes

Each state supports:

| Method | Behavior |
|--------|----------|
| `Run_Async()` | Spawn worker thread; blocking message loop |
| `Run()` | Blocking message loop on current thread |
| `Process()` | Non-blocking queue drain (one or more messages) |

**Client default:** `m_bHostMainStatesInFrameMove = true`

Main/global states are processed via `Process()` inside `CNPLRuntime::Run()` during each frame tick.

**Server default:** `SetHostMainStatesInFrameMove(false)`

Main states run on dedicated worker threads via `Run_Async()`. The service loop (`CParaEngineService`) drives periodic `FrameMove` via boost::asio timer.

### Emscripten single-thread

When `EMSCRIPTEN_SINGLE_THREAD` is defined:

- `boost::thread` → `CoroutineThread` (cooperative multitasking)
- No pthread pool required
- Simpler debugging; no SharedArrayBuffer requirement

## Activation Model

### NPL script pattern

Every `.npl` file implicitly defines a message handler:

```lua
-- helloworld.npl
this(msg) {
   if msg then
      print(msg.data or "")
   end
}
```

Activation from another script:

```lua
NPL.activate("(gl)helloworld.npl", {data="hello world!"})
```

### Neuron file name syntax

```
(runtimeState|gl)[connectionId:]filePath[@dnsRecord]
```

| Component | Example | Meaning |
|-----------|---------|---------|
| Runtime state | `(gl)` | Target state name; `gl` = global/main |
| Connection ID | `[0:]` | Optional connection routing |
| File path | `script/foo.npl` | Neuron file relative path |
| DNS record | `@myserver` | Resolve host via NPL DNS |

### C++ API

```cpp
// Async activation (queued)
runtime->NPL_Activate(filename, msg, sender);

// Sync local activation (immediate, same process)
runtime->ActivateLocalNow(filename, msg);

// HTTP URL in filename triggers web service callback path
```

Routing:

1. Parse `NPLFileName`
2. If local state exists → `CNPLRuntimeState::Activate_async`
3. If remote → `CNPLDispatcher::Activate_Async` → TCP or UDP

### Activation prefixes (Lua API)

Documented in `ParaScriptBindings/ParaScriptingNPL.h`:

| Prefix | Target |
|--------|--------|
| `(gl)` | Global/main runtime state |
| `(workerName)` | Named worker state |
| `(ip:port)` | Remote NPL process |
| No prefix | Same runtime state as caller |

## Message Format

Messages are Lua tables serialized for queueing and network transport. Tables are converted to strings for cross-thread/cross-process delivery — **not shared memory**.

Implications:

- Large payloads have serialization cost
- Avoid blocking operations in `this(msg)` handlers
- Keep messages reasonably sized

## Networking

### TCP server

```cpp
runtime->NPL_StartNetServer(port, /* ... */);
runtime->IsServerStarted();
runtime->GetHostIP();
runtime->GetHostPort();
```

`CNPLNetServer` manages accept thread and per-connection I/O threads.

### UDP server

```cpp
runtime->NPL_StartUDPServer(port);
```

Lower latency; used for specific activation patterns.

### Channels

`ChannelProperty` defines priority and reliability per channel (RakNet-inspired):

```cpp
struct ChannelProperty {
    int m_nPriority;      // e.g. MEDIUM_PRIORITY = 2
    int m_nReliability;   // e.g. RELIABLE_ORDERED = 3
};
```

### DNS records

NPL supports internal DNS for resolving `@record` in activation paths to IP:port pairs.

### Compression

Runtime supports optional packet compression for network activations.

## Timers

Each `CNPLRuntimeState` maintains a timer pool:

```cpp
// Via Lua API (ParaScriptingNPL.cpp):
NPL.SetTimer(callback, interval_ms, ...)
```

Timers fire during `CNPLRuntime::Run()` processing.

## Debugging

- `SetDebugHook` / `HasDebugHook` on runtime states
- `SetDebugTraceLevel` for trace verbosity
- Built-in NPL Code Wiki — HTTP-based editor/debugger
- VS Code / Visual Studio NPL debugger plugins (external repos)

Preemptive functions can be paused globally:

```cpp
state->PauseAllPreemptiveFunction(true);
```

## Script Bindings

Primary Lua API surface: `ParaEngine/ParaScriptBindings/ParaScriptingNPL.cpp`

Key exported functions:

- `NPL.activate` / `NPL.Activate`
- `NPL.load`, `NPL.reload`
- `NPL.startNetServer`, `NPL.connect`
- `NPL.setTimer`, `NPL.killTimer`
- `NPL.md5`, `NPL.encode`, `NPL.decode`
- Runtime state introspection

Global exports assembled in `ParaScriptingGlobal.cpp`.

## Frame Loop Integration

Client path (`ParaEngine/NPL/NPL_imp.cpp`):

```
CParaEngineAppBase::FrameMove()
    └── GetNPLRuntime()->Run(false)
```

Server path (`ParaEngine/Core/ParaEngineService.cpp`):

```
boost::asio timer → FrameMove()
    └── CNPLRuntime::Run()  (states on worker threads process independently)
```

## Thread Safety

`CNPLRuntime` uses `ParaEngine::mutex m_mutex` for state pool access. Many header methods are documented as thread-safe. Individual `CNPLRuntimeState` message queues are per-state — activations to different states can proceed concurrently.

## Lua Compatibility Rules

NPL strictly targets **Lua 5.1 semantics**:

| Allowed | Not allowed |
|---------|-------------|
| `getfenv` / `setfenv` | `_ENV` (Lua 5.2+) |
| LuaJIT extensions (when using LuaJIT) | `goto` statement |
| Standard 5.1 metatables | Lua 5.3+ integer division changes |

Default VM: **LuaJIT 2.1** with GC64 (`NPLRUNTIME_LUAJIT21=ON`).

Bindings use **luabind** (modified fork in externals).

## Adding New NPL APIs

1. Implement C++ function in `ParaScriptBindings/ParaScripting*.cpp`
2. Register with luabind in the module's `LoadHAPI_*()` function
3. Export to global table in `ParaScriptingGlobal.cpp`
4. Document with Doxygen-style comments (used by NPL Code Wiki)

Example binding pattern:

```cpp
module(L)[
    def("myFunction", &MyFunction),
    class_<MyClass>("MyClass")
        .def("method", &MyClass::method)
];
```

## Related Files

| File | Content |
|------|---------|
| `NPL/NPLRuntime.cpp` | Runtime singleton implementation |
| `NPL/NPLRuntimeState.cpp` | State threading and message processing |
| `NPL/NPLDispatcher.cpp` | Local/remote routing |
| `NPL/NPLCommon.h` | Shared types, priorities, packet structures |
| `NPL/NPLFileName.cpp` | Neuron file name parsing |
| `NPL/NPLActivationFile.cpp` | Activation file handling |
| `Framework/Interface/INPLRuntime.h` | Abstract runtime interface |

## Further Reading

- [architecture.md](architecture.md) — activation flow diagram
- [conventions.md](conventions.md) — Lua binding patterns
- [NPL Wiki: What is NPL?](https://github.com/LiXizhi/NPLRuntime/wiki/WhatIsNPL)
