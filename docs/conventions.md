# Coding Conventions

Established patterns in the NPLRuntime / ParaEngine codebase. Match these when adding or modifying code.

## File Organization

- **Feature directories** — `.h` and `.cpp` files live together in subsystem folders (`3dengine/`, `NPL/`, etc.)
- **No separate include/src split** for ParaEngine
- **Precompiled header** — include `ParaEngine.h` (or project PCH) as the first include in every `.cpp`
- **Externals** — third-party code in `NPLRuntime/externals/`; avoid modifying unless upgrading deps

## Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes | `C` prefix | `CSceneObject`, `CNPLRuntime` |
| Interfaces | `I` prefix | `IParaEngineApp`, `IRenderDevice` |
| Assets | `*Entity` suffix | `TextureEntity`, `MeshEntity`, `ParaXEntity` |
| Managers | `*Manager` suffix | `TextureAssetManager`, `EffectManager` |
| Singletons | `GetInstance()` or `GetSingleton()` | `CNPLRuntime::GetInstance()` |
| Member vars | `m_` prefix | `m_pDevice`, `m_nPriority` |
| String params | `s` prefix common | `sFileName`, `sIdentifier` |
| Count/index | `n` prefix common | `nRowIndex`, `nColumnIndex` |

## Smart Pointers and Memory

### Intrusive pointers (preferred)

```cpp
#include "util/intrusive_ptr.h"

asset_ptr<TextureEntity> tex;   // assets — thread-safe ref count
ref_ptr<CBaseObject> obj;       // general ref-counted engine objects
```

Base classes:

- `intrusive_ptr_thread_safe_base` — atomic ref count (shared across threads)
- `intrusive_ptr_single_thread_base` — main-thread only (cheaper)

### Cleanup macros

```cpp
SAFE_DELETE(ptr);
SAFE_RELEASE(com_ptr);   // COM / DirectX interfaces
```

### Object pools

Hot-path allocations use pools — see `util/ParaMemPool.h`. Avoid raw `new`/`delete` in per-frame code.

### Avoid

- `std::shared_ptr` for engine objects (use intrusive_ptr)
- `using namespace std;` in headers
- Raw owning pointers without clear lifetime documentation

## Asset References

Always use the asset manager — do not load files directly:

```cpp
// Good
TextureEntity* tex = CParaWorldAsset::GetSingleton()->GetTexture(path);

// Bad — bypasses cache, ref counting, device restore
// load texture manually from disk
```

Check for existing asset before creating duplicates:

```cpp
if (auto* existing = assetMgr->GetTexture(path)) return existing;
```

## Path Handling

Never hard-code absolute paths:

```cpp
// Good
string path = CFileUtils::GetWritablePath() + "save.dat";
CParaFile::OpenAssetFile("model/character.x");

// Bad
FILE* f = fopen("C:\\Games\\Paracraft\\model.x", "rb");
```

Use `CPathReplaceables` for configurable path aliases.

## Rendering Access

All GPU operations through the abstract device:

```cpp
IRenderDevice* device = CGlobals::GetRenderDevice();
// use device methods

// Bad — outside RenderSystem/
// glBindTexture(...);
// device->GetD3DDevice()->SetTexture(...);
```

DirectX-only code must be guarded:

```cpp
#ifdef USE_DIRECTX_RENDERER
    // DirectX-specific
#endif
```

## Device Lifecycle

GPU resource classes must implement the four-phase lifecycle and register with asset manager where appropriate:

```cpp
HRESULT InitDeviceObjects();
HRESULT RestoreDeviceObjects();
HRESULT InvalidateDeviceObjects();
HRESULT DeleteDeviceObjects();
```

Called from `CParaEngineAppBase` on context create, reset, loss, and shutdown.

## Platform Guards

```cpp
#ifdef USE_OPENGL_RENDERER
#elif defined(USE_DIRECTX_RENDERER)
#endif

#ifdef EMSCRIPTEN
    // WebAssembly
#endif

#ifndef EMSCRIPTEN_SINGLE_THREAD
    // Boost.Thread — real threads
#else
    // CoroutineThread — cooperative
#endif

#ifdef PARAENGINE_MOBILE
    // iOS + Android shared
#endif

#ifdef WIN32
#elif defined(LINUX)
#elif defined(__APPLE__)
#endif
```

Check C++ standard availability before using C++14+ features:

- Windows/Android: C++17
- Linux/macOS/iOS/Emscripten: C++20

## Threading Rules

| Context | Rule |
|---------|------|
| Main thread | Rendering, input, `FrameMove`, hosted NPL `Process()` |
| NPL worker threads | Message handlers, timers for that state |
| Net I/O threads | Connection read/write only; activate via dispatcher |
| Async loader | Asset decode; deliver to main thread for GPU upload |

Guard Boost.Thread includes:

```cpp
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread.hpp>
#endif
```

**Do not block** in NPL `this(msg)` handlers — blocks the worker thread's message pump.

NPL messages are **serialized tables**, not shared memory — design accordingly.

## NPL / Lua Bindings

### Lua version

Strict **Lua 5.1** compatibility:

- Use `getfenv`/`setfenv`, not `_ENV`
- No `goto`
- Test with LuaJIT 2.1 (default VM)

### Adding bindings

1. Implement C++ in `ParaScriptBindings/ParaScripting*.cpp`
2. Register in module's `LoadHAPI_*()`:

```cpp
module(L)[
    def("myFunc", &MyFunc),
    class_<MyClass>("MyClass")
        .def("method", &MyClass::method)
];
```

3. Export to globals in `ParaScriptingGlobal.cpp`
4. Add Doxygen comments — powers NPL Code Wiki auto-docs

### Attribute fields (reflection)

Engine classes exposing NPL properties:

```cpp
ATTRIBUTE_DEFINE_CLASS(CMyClass);

virtual int InstallFields(CAttributeClass* pClass, bool bOverride) {
    pClass->AddField("MyProperty", &CMyClass::MyProperty_s);
    return S_OK;
}
```

## Error Handling

- HRESULT pattern common in older code (`S_OK`, `E_FAIL`)
- Logging via `WRITE_LOG` / `AppLog()` / `CLogger`
- Prefer early return over deep nesting
- Do not over-engineer error paths for impossible states

## Comments

- Code should be mostly self-explanatory
- Comment non-obvious business logic, network protocol details, GPU state assumptions
- Doxygen on public API and all Lua-exported functions
- Avoid commented-out code blocks in commits

## CMake Conventions

When adding source files:

- ParaEngine: add directory to `ucm_add_dirs()` in `ParaEngine/CMakeLists.txt` or place file in existing recursive dir
- New plugin: `Plugins/MyPlugin/CMakeLists.txt` + guard in `Plugins/CMakeLists.txt`
- New platform: subdirectory under `Platform/` + condition in `Platform/CMakeLists.txt`

New CMake options: add near related options in root `NPLRuntime/CMakeLists.txt` with sensible defaults for client and server branches.

## Testing Expectations

No unified unit test suite in tree. Manual verification:

- Windows: build client, run against ParacraftSDK `redist/`
- Linux: build server, run `npls hello.npl`
- Verify Debug and Release configs when touching GPU or threading code

## Anti-Patterns

| Avoid | Prefer |
|-------|--------|
| Hard-coded paths | `CFileUtils`, `CParaFile::OpenAssetFile` |
| Raw GL/D3D calls outside RenderSystem | `CGlobals::GetRenderDevice()` |
| `shared_ptr` for assets | `asset_ptr<T>` |
| Blocking I/O in NPL handlers | Async activation + callback |
| C++14+ without platform checks | Guard or restrict to C++17-safe subset |
| Deep inheritance for game objects | Composition, component attachment |
| Modifying externals for app fixes | Patch in ParaEngine wrapper layer |
| Empty catch blocks | Log and handle or don't catch |

## Related

- [CLAUDE.md](../CLAUDE.md) — quick reference for AI assistants
- [npl-runtime.md](npl-runtime.md) — NPL threading and activation rules
- [paraengine-subsystems.md](paraengine-subsystems.md) — where code belongs
- [.github/copilot-instructions.md](../.github/copilot-instructions.md) — Copilot-specific guide
