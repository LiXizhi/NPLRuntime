# Platform Layer

The Platform layer provides OS-specific entry points, window/render context creation, and input routing. Each platform subdirectory produces a distinct executable or shared library that links `ParaEngine` + `RenderSystem` + optional `Plugins`.

**Router:** `NPLRuntime/Platform/CMakeLists.txt`

## Platform Selection Matrix

| Condition | Directory | Primary binary |
|-----------|-----------|----------------|
| `NPLRUNTIME_SERVER` | `Linux/` | `ParaEngineServer` |
| `SDL` or `EMSCRIPTEN` | `SDL/` | `ParaCraftSDL2` / `ParaCraft` |
| `ANDROID` + `ANDROID_STUDIO` | `AndroidStudio/` | APK via Gradle |
| `ANDROID` | `Android/` | `.so` + legacy APK |
| `IOS` | `iOS/` | iOS app / framework |
| `WIN32` (client) | `Windows/` | `ParaEngineClient.exe` |
| `MAC_SERVER` | `Linux/` | `ParaEngineServer` |
| `APPLE` (client) | `OSX/` | macOS client app |
| `LINUX` | `SDL/` | `ParaCraftSDL2` |
| else | `Linux/` | `ParaEngineServer` (fallback) |

---

## Windows Client

**Path:** `NPLRuntime/Platform/Windows/`

### Entry flow

```
WinMain()                          // src/main.cpp
  → CWindowsApplication
  → RenderWindowDelegate           // window creation, message pump
  → CParaEngineAppBase::InitApp()
  → CParaEngineAppBase::StartApp()
  → message loop → FrameMove() per tick
```

| File | Role |
|------|------|
| `src/main.cpp` | `WinMain` entry |
| `src/WindowsApplication.h/.cpp` | Windows app subclass |
| `src/ParaEngineApp.cpp` | Platform-specific app hooks |
| `src/RenderWindowDelegate.cpp` | HWND, WGL/D3D surface |

### CMake target

`WindowsApplication` → output `ParaEngineClient.exe`

Options:

- `PARAENGINE_CLIENT_DLL=ON` — build as DLL instead of EXE
- `NPLRUNTIME_HAQI_LAUNCHER` — alternate launcher variant
- `NPLRUNTIME_WEBVIEW` — WebView2 integration

### Debugging

Set VS debugging working directory to an NPL app root (e.g. ParacraftSDK `redist/`). Run `paraengineclient_d.exe` from `ParaWorld/bin/`.

---

## Linux / macOS Server

**Path:** `NPLRuntime/Platform/Linux/`

Despite the directory name, this is the **headless server** target used for Linux and macOS server builds (`MAC_SERVER`).

### Entry flow

```
main()                             // src/ParaEngineServer.cpp
  → CParaEngineServerApp
  → CParaEngineService::Run()
  → boost::asio steady_timer → FrameMove()
  → CNPLRuntime (worker threads for main states)
```

| File | Role |
|------|------|
| `src/ParaEngineServer.cpp` | `main()` entry |
| Server app init | Sets `SetHostMainStatesInFrameMove(false)` |

### CMake target

`ParaEngineServer`

CLI equivalent: `npls script.npl`

---

## SDL Desktop Client

**Path:** `NPLRuntime/Platform/SDL/`

Cross-platform desktop client using SDL2 for window, input, and OpenGL context.

| Platform | Binary |
|----------|--------|
| Linux | `ParaCraftSDL2` |
| Windows (LOCAL_DEBUG) | `ParaCraftSDL2` |
| Emscripten | `ParaCraft` / `ParaCraftSingleThread` |

### Build

```bash
cmake -S NPLRuntime -B out -DLINUX=TRUE -DSDL_ROOT=${SDL_ROOT}
cmake --build out
```

Run with working directory set to a Paracraft root (containing `config/`, `*.pkg`, `paraengine.sig`).

See `Platform/SDL/readme.md` for Emscripten-specific notes.

### Emscripten subdirectory

**Path:** `Platform/SDL/emscripten/`

WebAssembly output: `ParaCraft.html`, `.js`, `.wasm`, optional `worker.js`.

---

## macOS Client

**Path:** `NPLRuntime/Platform/OSX/`

Xcode project generated via `build_mac_client.sh`.

| Component | Role |
|-----------|------|
| `ParaAppOSX` | macOS application class |
| Asset folder | `Platform/OSX/assets/` — bundle resources |

App Sandbox notes in [INSTALL.md](../INSTALL.md#mac-app-sandbox-api).

---

## iOS

**Path:** `NPLRuntime/Platform/iOS/`

| Component | Role |
|-----------|------|
| `AppDelegate.mm` | iOS app delegate |
| `ParaAppiOS` | Engine integration |
| `assets/` | App bundle assets (no symlinks) |
| OpenGL ES | Via `gl_ios_spec.c` |

Build: `./build_ios_os.sh` → Xcode project in `build/ios/ios_os`.

Boost prebuild: `./NPLRuntime/externals/boost/build_ios_and_osx.sh`

Option: `BUILD_FRAMEWORK=ON` for framework output.

---

## Android

**Path:** `NPLRuntime/Platform/Android/` or `AndroidStudio/`

| Mode | Path | Build |
|------|------|-------|
| Legacy NDK | `Android/` | VS + CMake + Nsight |
| Android Studio | `AndroidStudio/` | Gradle + CMake |

Assets: `Platform/Android/assets/` or AndroidStudio asset dirs.

Boost: `NPLRuntime/externals/boost/build_android.bat`

Entry: JNI bridge from Java/Kotlin activity to native `CParaEngineAppBase`.

OpenGL ES via `gl_android_spec.c`.

---

## HarmonyOS

**Path:** `NPLRuntime/Platform/HarmonyOS/`

Modern port using NAPI (replacing JNI) and ArkTS UI.

| Component | Role |
|-----------|------|
| `napi_main.cpp` | Native module entry |
| `paracraft_core/` | Native core library |
| `entry/` | ArkTS application UI |

Additional migration docs in `Platform/HarmonyOS/docs/`:

- `JNI_to_NAPI_Migration_Summary.md`
- `Project_Upgrade_Guide.md`
- `DevEco_5.0.5_Setup.md`

OpenGL via `gl_harmonyos_spec.c`.

---

## Common Bootstrap Sequence

All platforms converge on `CParaEngineAppBase`:

```
1. Platform creates IRenderWindow (or nullptr for server)
2. InitApp(window, commandLine)
   a. FindParaEngineDirectory — locate asset root adjacent to binary
   b. BootStrapAndLoadConfig — parse config/*.xml, mount .pkg archives
   c. InitSystemModules
      - CNPLRuntime::Init()
      - CParaWorldAsset initialization
      - Scene, GUI, audio, physics setup
   d. InitRenderEnvironment (skip if NULL renderer)
3. StartApp — fire initial NPL activation (typically bootstrap script)
4. Platform event loop calls FrameMove(deltaTime)
5. StopApp on exit
```

### Asset root discovery

`FindParaEngineDirectory()` searches relative to executable for:

- `config/` directory
- `*.pkg` package files
- `paraengine.sig` signature file

Mobile platforms bundle assets in platform-specific asset folders.

---

## Input Handling

| Platform | Input path |
|----------|------------|
| Windows | Win32 messages → `HandleUserInput()` |
| SDL | SDL events → touch/mouse/keyboard |
| iOS/Android | Touch primary; `SetTouchInputting(true)` |
| Emscripten | SDL emulated events |

Coordinate transforms: `GameToClient()` / `ClientToGame()` in app base.

---

## Output Locations

All platform builds copy artifacts to:

```
ParaWorld/bin32/   or   ParaWorld/bin64/
```

Debug suffix: `_d` on binaries and DLLs.

---

## Platform Macros

Common preprocessor guards used across Platform and ParaEngine:

| Macro | Meaning |
|-------|---------|
| `WIN32` / `_WIN32` | Windows |
| `LINUX` | Linux |
| `__APPLE__` / `APPLE` | macOS / iOS |
| `__ANDROID__` | Android |
| `EMSCRIPTEN` | WebAssembly |
| `PARAENGINE_MOBILE` | iOS + Android common code |
| `HARMONY_OS` | HarmonyOS |
| `USE_OPENGL_RENDERER` | OpenGL backend active |
| `USE_DIRECTX_RENDERER` | DirectX backend active |
| `EMSCRIPTEN_SINGLE_THREAD` | No pthread on web |

---

## Legacy Server Tree

`Server/trunk/` at repository root is a **separate legacy server** with its own platform code and vendored deps. Use `NPLRuntime` + `NPLRUNTIME_SERVER=ON` for current server development.

---

## Related

- [build-system.md](build-system.md) — how platform targets are selected in CMake
- [architecture.md](architecture.md) — app lifecycle and frame loop
- [render-system.md](render-system.md) — GL context creation handoff
- [INSTALL.md](../INSTALL.md) — per-platform install instructions
