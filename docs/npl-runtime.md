# NPL Runtime

NPL (Neural Parallel Language) is ParaEngine's scripting layer. It is 100% Lua 5.1 syntax-compatible but uses a unique **activation model** for async message passing instead of direct function calls.

## Core Concepts

### Activation vs Calling

Traditional Lua:
```lua
require("mymodule")
mymodule.doSomething({data="hello"})
```

NPL activation:
```lua
NPL.activate("(gl)script/mymodule.npl", {data="hello"})
-- In mymodule.npl:
this(msg) {
    print(msg.data)
}
```

Key differences:
- Files are **activated**, not imported — each activation creates an isolated context
- The `this(msg)` function receives the activation message
- Activations are **async** — they go through a message queue
- Local files use `(gl)` prefix; remote files use NID addressing

### Neuron Files

Each `.npl` or `.lua` file is a "neuron" — a script unit that can be activated independently. A neuron file maintains its own activation state (`NeuronFileState`) tracking whether it's currently processing a message.

### Runtime States

A **runtime state** (`CNPLRuntimeState`) is an isolated execution context with:
- Its own Lua state (`lua_State*`)
- Its own message input queue
- Its own thread (for worker states)
- A unique name (default: `(main)`)

The `(main)` state on the client also drives rendering.

## Key Classes

| Class | Header | Role |
|-------|--------|------|
| `CNPLRuntime` | `NPL/NPLRuntime.h` | Singleton facade for all NPL operations |
| `CNPLRuntimeState` | `NPL/NPLRuntimeState.h` | Per-thread Lua state + message queue |
| `CNPLDispatcher` | `NPL/NPLDispatcher.h` | Routes messages between states and network connections |
| `CNPLScriptingState` | `NPL/NPLScriptingState.h` | Lua state wrapper; loads API bindings |
| `CNPLNetServer` | `NPL/NPLNetServer.h` | TCP acceptor (Boost.Asio) |
| `CNPLConnection` | `NPL/NPLConnection.h` | Individual TCP connection |
| `CNPLNetUDPServer` | `NPL/NPLNetUDPServer.h` | UDP server |
| `NPLMsgIn` / `NPLMsgOut` | `NPL/NPLMsgIn.h`, `NPLMsgOut.h` | Wire message format |
| `NPLActivationFile` | `NPL/NPLActivationFile.h` | Per-file activation tracking |
| `CNPL_imp` | `NPL/NPL_imp.h` | `INPL` interface: activate, StartService, FrameMove |
| `CNPLNameSpaceBinding` | `NPL/NPLNameSpaceBinding.h` | DNS-style NID→address resolution |

## Message Flow

### Local Activation

```
NPL.activate("(gl)script/hello.npl", {data="hi"})
    → CNPLRuntime::NPL_Activate()
    → CNPLRuntimeState("(main)")::Activate_async(NPLMessage)
    → CNPLRuntimeState::SendMessage() → m_input_queue
    → FrameMove() processes queue
    → Neuron file's this(msg) handler executes
```

### Remote Activation

```
NPL.activate("server_nid", "(gl)script/hello.npl", {data="hi"})
    → CNPLRuntime::NPL_Activate()
    → CNPLDispatcher::Activate_Async(filename, code)
    → CNPLDispatcher::CreateGetNPLConnectionByNID(nid)
        → lookup m_active_connection_map
        → or m_pending_connection_map
        → or m_server_address_map → create new CNPLConnection
    → CNPLConnection::SendMessage(filename, code)
    → NPLMsgOut serialized and sent over TCP
```

### Incoming Messages

```
CNPLConnection::handle_read()
    → CNPLConnection::handleReceivedData()
    → m_parser.parse() until complete message
    → CNPLConnection::handleMessageIn()
    → CNPLDispatcher::DispatchMsg(NPLMsgIn)
    → GetRuntimeState(msg.rts_name)
    → CheckPubFile(msg.filename) — access control
    → CNPLRuntimeState::Activate_async(NPLMessage)
    → m_input_queue
```

## Wire Message Format

Messages use HTTP-like headers with a JSON body:

```
A (g1)script/hello.lua NPL/1.0
rts:(main)
User-Agent:NPL
Content-Length:16
{"hello world!"}
```

Fields:
- **First line:** Method, filename, protocol version
- **rts:** Target runtime state name
- **nid/tid:** Network identity / temporary identity (for unauthenticated connections)
- **Body:** JSON-encoded activation data

Parser: `NPL/NPLMsgIn_parser.h`

## Networking Setup

### Start a Network Server

```lua
NPL.StartNetServer("127.0.0.1", "60001")
```

### Expose Files for Remote Activation

```lua
NPL.AddPublicFile("script/server/handler.lua", 1)
-- File ID must match on all communicating machines
```

### Register Trusted Server Addresses

```lua
NPL.AddNPLRuntimeAddress({
    host = "127.0.0.1",
    port = "60001",
    nid = "my_server_nid"
})
```

### Create Worker Threads

```lua
for i = 1, 10 do
    local worker = NPL.CreateRuntimeState("worker_" .. i, 0)
    worker:Start()
end
```

### Accept Authenticated Connections

Temporary connections get a `~XXX` NID until authenticated:
```lua
NPL.accept(temporary_tid, authenticated_nid)
```

## NID (Network Identity)

Each NPL runtime on the network is identified by a globally unique NID string. NPL does not verify or authenticate NIDs — that is the application's responsibility.

NIDs can be mapped to DNS-like namespace bindings (see `Client/trunk/ParaEngineClient/doc/NamespaceBind.txt` for legacy examples).

## Threading Model

```
Thread: Main (CNPLRuntimeState "(main)")
├── Lua state for primary script logic
├── Rendering (client only)
├── Spawns worker threads
└── Processes its own message queue

Thread: Worker N (CNPLRuntimeState "worker_N")
├── Independent Lua state
├── Independent message queue
└── No rendering
```

**Critical rule:** Never share Lua states across threads. All cross-thread communication goes through NPL message passing (activation).

Synchronization uses Boost threading primitives (`boost::shared_mutex` in `CNPLRuntime`).

## Extensibility

NPL supports three scripting backends:

### 1. Lua/NPL (Primary)
Standard `.npl`/`.lua` files with `this(msg)` activation handlers.

### 2. Mono/.NET (Plugin)
Via `NPLMono2.dll` — activate C# assemblies:
```lua
NPL.activate("MyAssembly.dll", {method="HandleMessage"})
```

### 3. C++ Native Plugins
Functions named `NPL_activate_XXXX_cpp`:
```cpp
NPL::NPLReturnCode NPL_activate_mymodule_cpp(INPLRuntimeState* pState) {
    // Access activation data via pState
    return NPL::NPL_OK;
}
```
Activated as: `NPL.activate("mymodule.cpp", {data=123})`

## Channels and Priority

NPL supports message channels with configurable priority and reliability:

```cpp
struct ChannelProperty {
    int m_nPriority;      // Message priority level
    int m_nReliability;   // Delivery guarantee (ordered, reliable, etc.)
};
```

Default: `MEDIUM_PRIORITY`, `RELIABLE_ORDERED`.

## Timers

NPL runtime supports script timers via the `NPL` Lua API. Timers fire activation messages to specified neuron files on schedule. Implemented in `CNPLRuntime` with Boost.Asio deadline timers.

## Compression

Large messages can be compressed. `CNPLRuntime` manages compression settings for network transport.

## Lua Compatibility

NPL targets **Lua 5.1** semantics. Supported Lua implementations:
- **Lua 5.1.4** (bundled, with NPL compiler extensions in `npl_compiler.c`)
- **LuaJIT 2.0.4** (bundled)
- **LuaJIT 2.1** (bundled, GC64 on 64-bit)

Do not use Lua 5.2+ features (e.g., `goto`, `\z` in strings, `_ENV`).

## File Prefixes

| Prefix | Meaning |
|--------|---------|
| `(gl)` | Global local — file on local filesystem |
| `(g1)`, `(g2)`, ... | Named local groups |
| NID string | Remote activation target |

## Debugging

- Legacy notes: `Client/trunk/ParaEngineClient/doc/NPLDebugging`
- Network call graph: `Client/trunk/ParaEngineClient/NPL/NPL_readme.txt`
- Log output via `ParaGlobal` / engine logging utilities

## Related Files

```
Client/trunk/ParaEngineClient/NPL/
├── NPLRuntime.h / NPLRuntime.cpp       # Main runtime singleton
├── NPLRuntimeState.h / .cpp            # Per-thread state
├── NPLDispatcher.h / .cpp              # Message routing
├── NPLConnection.h / .cpp              # TCP connection
├── NPLNetServer.h / .cpp               # TCP server
├── NPLMsgIn.h / NPLMsgOut.h           # Message types
├── NPLMsgIn_parser.h                   # Wire format parser
├── NPLActivationFile.h                 # Neuron state tracking
├── NPLScriptingState.h / .cpp          # Lua state management
├── NPL_imp.h / NPL_imp.cpp            # INPL interface
├── NPLCommon.h                         # Shared types
├── AISimulator.h / .cpp               # AI frame hook
└── NPL_readme.txt                      # Internal network docs
```
