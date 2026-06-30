# Plugins

Optional subsystems built as separate CMake targets and linked into ParaEngine when enabled. Router: `NPLRuntime/Plugins/CMakeLists.txt`.

## Plugin Overview

| Plugin | CMake option | Target | Default (client) | Default (server) |
|--------|--------------|--------|------------------|------------------|
| SQLite | (always) | `ParaSqlite` | ON | ON |
| Bullet Physics | `NPLRUNTIME_PHYSICS` | `PhysicsBT` | ON | OFF |
| Audio | `NPLRUNTIME_AUDIO` | `cAudioEngine` | ON | OFF |
| .NET Mono | `NPLRUNTIME_NPLMono2` | `NPLMono2` | platform-dependent | OFF |
| OpenCASCADE CAD | `NPLRUNTIME_OCE` | `NplOce` | OFF | OFF |

Server builds skip audio entirely (`NPLRUNTIME_SERVER` block in CMakeLists).

Static/mobile builds link plugins statically when `NPLRUNTIME_STATIC_LIB=ON` or on Android/iOS.

---

## ParaSqlite (SQLite)

**Path:** `NPLRuntime/Plugins/sqlite3/`

Always built. Embedded SQLite database engine exposed to NPL scripts.

| Component | Role |
|-----------|------|
| `ParaSqlite` | CMake target wrapping sqlite3 amalgamation |
| Script bindings | Database access from NPL (via ParaScriptBindings) |

Used for local data storage, caching, and game state persistence in Paracraft and related apps.

No CMake option to disable — core dependency.

---

## PhysicsBT (Bullet Physics)

**Path:** `NPLRuntime/Plugins/PhysicsBT/`

**Option:** `NPLRUNTIME_PHYSICS=ON`

Bullet 2.x physics integration implementing `IParaPhysics`.

| Component | Role |
|-----------|------|
| `ParaPhysicsWorld` | Physics world wrapper |
| Collision shapes | Box, sphere, mesh, compound |
| Rigid body dynamics | Forces, constraints, raycasts |
| `CPhysicsWorld` (3dengine) | Engine-side physics delegate |

### Dependencies

Vendored in `externals/bullet-2.75/` or `externals/bullet3/` (build configuration dependent).

### Script API

Physics exposed via scene/character bindings and direct physics API in ParaScriptBindings.

### Enable/disable

```cmake
-DNPLRUNTIME_PHYSICS=ON   # client default
-DNPLRUNTIME_PHYSICS=OFF  # server default, lightweight builds
```

---

## cAudioEngine (Audio)

**Path:** `NPLRuntime/Plugins/cAudioEngine/`

**Option:** `NPLRUNTIME_AUDIO=ON`

OpenAL-based audio engine with format decoders.

| Component | Role |
|-----------|------|
| OpenAL-soft | Cross-platform audio output |
| Vorbis/Ogg | `.ogg` decoding |
| MP3 decoder | MP3 playback |
| cAudio wrapper | 3D positional audio, streaming |

### Script API

`ParaScriptBindings/ParaScriptingAudio.cpp`

Functions for play/stop/pause, 3D listener position, volume control.

### Platform notes

- Disabled on server builds
- Windows XP builds: disable with `NPLRUNTIME_CAUDIOENGINE=OFF`; remove `caudioengine.dll` manually
- Mobile: OpenAL-soft adapted for iOS/Android

### Enable/disable

```cmake
-DNPLRUNTIME_AUDIO=ON   # client
-DNPLRUNTIME_AUDIO=OFF  # server, CI headless
```

---

## NPLMono2 (.NET Interop)

**Path:** `NPLRuntime/Plugins/NPLMono/NPLMono2/`

**Option:** `NPLRUNTIME_NPLMono2=ON`

Mono/.NET runtime bridge allowing NPL scripts to call C# assemblies and vice versa.

| Use case | Description |
|----------|-------------|
| Legacy integrations | Call existing .NET libraries from NPL |
| Tooling | Host managed plugins |

### Platform availability

| Platform | Typical setting |
|----------|-----------------|
| Windows | Available when Mono installed |
| Linux | Available with Mono runtime |
| Android | Disabled by default |
| Static builds | Disabled |
| Server | Disabled by default |

Requires Mono development packages on Linux (see `.travis.yml`).

---

## NplOce (OpenCASCADE CAD)

**Path:** `NPLRuntime/Plugins/NplOce/`

**Option:** `NPLRUNTIME_OCE=ON`

OpenCASCADE geometry kernel for CAD model loading and display. Works with DirectX-only `CadModel` in ParaEngine.

Off by default — large dependency, specialized use case.

---

## NPLRUNTIME_DARKNET

**Option:** `NPLRUNTIME_DARKNET=ON`

Optional darknet (neural network) integration. Vendored headers in `externals/darknet/`.

Off by default. Specialized ML inference use case.

---

## Plugin Loading Model

Plugins are **statically linked** at build time — not dynamic `.dll` plugins at runtime (except optional DLL mode for the main client on Windows).

Link chain:

```
Platform executable
  → ParaEngine (static)
  → RenderSystem (static)
  → Plugins (static, conditional)
  → externals (static/shared mix)
```

When `NPLRUNTIME_STATIC_LIB=ON`, the entire stack links into one binary.

When building as DLL (`PARAENGINE_CLIENT_DLL`), plugin symbols export through the DLL boundary.

---

## C++ Plugin Interface

Engine code accesses plugins through Framework interfaces:

```cpp
// Physics
IParaPhysics* physics = CGlobals::GetPhysicsWorld();

// Audio — accessed via ParaScriptBindings and internal audio manager
```

Adding a new plugin:

1. Create subdirectory under `Plugins/`
2. Add `add_subdirectory()` guard in `Plugins/CMakeLists.txt`
3. Add CMake option in root `CMakeLists.txt`
4. Link from `ParaEngine/CMakeLists.txt` when option is ON
5. Add ParaScriptBindings for NPL API surface
6. Document option in [build-system.md](build-system.md)

---

## External Plugin Pattern (NPL DLL plugins)

NPL also supports runtime-loaded DLL plugins registered via `DLLPlugInEntity` (see `NPLRuntimeState.h` reference). These are **NPL-level** plugins (script/C++ extensions loaded at runtime), distinct from CMake `Plugins/` build targets.

---

## Build Matrix Summary

| Build type | SQLite | Physics | Audio | Mono | OCE |
|------------|--------|---------|-------|------|-----|
| Windows client | ✓ | ✓ | ✓ | optional | optional |
| Windows server | ✓ | ✗ | ✗ | ✗ | ✗ |
| Linux server | ✓ | ✗ | ✗ | ✗ | ✗ |
| Linux SDL client | ✓ | ✓ | ✓ | ✗ | ✗ |
| Emscripten | ✓ | ✓ | ✓ | ✗ | ✗ |
| Android/iOS | ✓ | ✓ | ✓ | ✗ | ✗ |
| Static server lib | ✓ | ✗ | ✗ | ✗ | ✗ |

---

## Related

- [build-system.md](build-system.md) — CMake options for each plugin
- [paraengine-subsystems.md](paraengine-subsystems.md) — engine integration points
- [INSTALL.md](../INSTALL.md) — optional deps (mysql, mono, postgresql mentioned for Linux)
