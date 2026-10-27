# WebXR on the Emscripten (Web) Build

How to set up, build, run and debug ParaCraft's WebXR (immersive VR in the browser)
support in the Emscripten build of NPLRuntime.

> **Short answer:** you do **not** need any extra C++ library or npm package for WebXR.
> WebXR is a browser API. The repo already contains a hand-written Emscripten JS library
> (`library_webxr.js`) that bridges it to C++, and it is linked into every Emscripten
> build automatically. What you need is the normal Emscripten toolchain plus some
> *testing* tools (a headset or an emulator, and a secure origin).

---

## 1. What you need

### Required (same as the regular web build)

| Item | Version / notes |
|------|-----------------|
| Linux / WSL Ubuntu | The build scripts assume a Linux shell. |
| **emsdk** | Pin **3.1.59** (the version the team last verified, see `NPLRuntime/cmake/README-WebRobot.md`; 4.0.10 was also verified for the robot build). `build_emscripten.sh` installs `latest`, which may drift — prefer `./emsdk install 3.1.59 && ./emsdk activate 3.1.59`. |
| **CMake** | ≥ 3.28 (checked by `build_emscripten.sh`). |
| **Boost** | 1.85, built with `toolset=emscripten` and `-fexceptions -sNO_DISABLE_EXCEPTION_CATCHING` (the script does this for you if `~/boost_1_85_0/stage/lib` is missing). |
| `paracraft_script`, `webparacraft` repos | Cloned automatically by `build_emscripten.sh` into `~/paracraft_script` and `~/webparacraft`. |
| Python 3 | For `scripts/serve.py` (local server with COOP/COEP headers). |

Emscripten ports pulled in automatically by the existing flags (nothing to install):
`SDL2`, `zlib`, `libjpeg`, `libpng`, `openal`, `embind`, `fetch`, `IDBFS`, `websocket`.
WebGL 2 is already enabled (`-sMAX_WEBGL_VERSION=2 -sFULL_ES2=1`), which WebXR needs.

### For testing WebXR (pick at least one)

| Tool | Use |
|------|-----|
| **Immersive Web Emulator** (Meta, Chrome/Edge extension) | Emulates a Quest headset + controllers in desktop Chrome DevTools. Fastest edit–test loop; no hardware needed. |
| **Meta Quest** (Quest 2/3/Pro) + Quest Browser | Real-device testing. Supports `immersive-vr`. |
| **Android `adb`** (Android platform-tools) | `adb reverse` so the headset can reach your dev server as `localhost` (secure context without certificates), and `chrome://inspect` remote debugging. |
| PC VR headset + desktop Chrome/Edge (Windows, via OpenXR runtime) | Alternative real-device path. |
| `mkcert` (optional) | Local HTTPS certificates if you prefer serving over LAN instead of `adb reverse`. |

**Secure context is mandatory.** `navigator.xr` only exists on `https://` pages or on
`http://localhost`. A headset opening `http://192.168.x.x:8080` will report *WebXR not
supported* (error `-2`).

---

## 2. How the existing WebXR layer is wired

```
Browser  navigator.xr / XRSession / XRWebGLLayer
   │
   ▼
NPLRuntime/Platform/SDL/emscripten/library_webxr.js   ← JS side, linked with --js-library
   │  (webxr_init, webxr_request_session, webxr_get_input_sources, webxr_get_input_pose …)
   ▼
NPLRuntime/Platform/SDL/emscripten/webxr.h            ← C declarations of the JS functions
   │
   ▼
NPLRuntime/Platform/SDL/emscripten/main.cpp            ← webxr_init(): per-XR-frame callback
   │  collects eye views, head pose, controller poses, buttons, axes
   ▼
ParaEngine/Framework/Interface/IParaWebXR.h            ← attribute object "WebXR" (fields for Lua)
Platform/SDL/src/3dengine/WebXR.{h,cpp}                ← CParaWebXR: SetIsXR() starts/ends session
   │
   ▼
ParaEngine/3dengine/ViewportManager.cpp                ← VIEW_LAYOUT_STEREO_WEBXR (left/right eye viewports)
Platform/SDL/src/RenderWindowSDL2.cpp                  ← resizes canvas to 2 × eye width while in XR
ParaEngine/Core/ParaEngineAppBase.cpp                  ← skips normal frame path while in XR
```

The link flag is set in `NPLRuntime/Platform/SDL/CMakeLists.txt`:

```cmake
set_property(TARGET ${TARGET_NAME} APPEND_STRING PROPERTY LINK_FLAGS
    "--js-library ${CMAKE_CURRENT_SOURCE_DIR}/emscripten/library_webxr.js")
```

### Frame loop while in XR

1. Normal mode: `emscripten_set_main_loop_arg(mainloop)` → `RunLoopOnce()` every browser frame.
2. When `isXR` becomes true, `mainloop()` stops calling `RunLoopOnce()`; instead the XR
   session's `requestAnimationFrame` drives the engine: `onFrame` (JS) binds the
   `XRWebGLLayer` framebuffer, then calls the C++ frame callback, which updates the
   `WebXR` attribute object and calls `RunLoopOnce()`.
3. On session end, the end-callback rebinds framebuffer 0, sets layout back to
   `VIEW_LAYOUT_DEFAULT`, and resizes the canvas to the page size.

### Lua / NPL access

The `ViewportManager` exposes a child attribute object named **`WebXR`**:

| Field | Type | Meaning |
|-------|------|---------|
| `isXR` | bool (get/set) | Set `true` to request an `immersive-vr` session; `false` to exit. |
| `webXRTime` | int | XR frame timestamp. |
| `webXRViewsCount` | int | Number of eye views (normally 2). |
| `webXRLeftView` / `webXRRightView` | Vector4 | Eye viewport `x, y, w, h` in the XR framebuffer. |
| `webXRLeftPosition` / `webXRRightPosition` | Vector3 | Eye position. |
| `webXRLeftOrientation` / `webXRRightOrientation` | Vector4 | Eye orientation quaternion. |
| `webXRLeftHandPosition` / `webXRRightHandPosition` | Vector3 | Controller target-ray position. |
| `webXRLeftHandOrientation` / `webXRRightHandOrientation` | Vector4 | Controller orientation quaternion. |
| `leftHandKey0..6` / `rightHandKey0..6` | bool | Controller buttons (xr-standard mapping: 0 trigger, 1 squeeze, 2 touchpad, 3 thumbstick, 4 A/X, 5 B/Y, 6 reserved). |
| `webXRLeftAxes` / `webXRRightAxes` | Vector4 | Touchpad / thumbstick axes. |

`isXR = true` calls `navigator.xr.requestSession()`, which **must happen shortly after a
user gesture** (a click / key press). Trigger it from a button handler, not on startup.

---

## 3. Build

```bash
# one-time: toolchain
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk/emsdk
cd ~/emsdk/emsdk
./emsdk install 3.1.59
./emsdk activate 3.1.59
source ./emsdk_env.sh

# Boost 1.85 in ~/boost_1_85_0 (build_emscripten.sh builds it if stage/lib is missing)

# from the NPLRuntime repo root
./build_emscripten.sh        # answer Y to run cmake, then choose a -j count
```

The script defaults to the **single-thread** build (`build/emscripten_single_thread`,
target `ParaCraftSingleThread`). Both single- and multi-thread builds include WebXR.
Output is copied to `HTML_ROOT` (`~/webparacraft` by default).

Equivalent manual configure:

```bash
emcmake cmake -S ./NPLRuntime -B build/emscripten_single_thread \
  -DBUILD_TESTING=OFF -DEMSCRIPTEN=ON -DEMSCRIPTEN_SINGLE_THREAD=ON \
  -DCMAKE_BUILD_TYPE=RELEASE \
  -DBOOST_ROOT="$HOME/boost_1_85_0" \
  -DAPP_ROOT="$HOME/paracraft" -DHTML_ROOT="$HOME/webparacraft"
cd build/emscripten_single_thread && emmake make -j8
```

Changing only `library_webxr.js` does **not** always trigger a relink — touch
`Platform/SDL/emscripten/main.cpp` or delete the output `.js` to force it.

---

## 4. Run and test

### Desktop with the emulator (fastest)

1. `scripts/dev_web.sh all && scripts/dev_web.sh serve` → builds, copies the engine into
   `~/webparacraft/auto_generate_files`, and serves it on port 8088 under `/npl/webparacraft/`
   (the layout `index.html` and `ServiceWorker.js` require on localhost). See
   [web-dev-workflow.md](web-dev-workflow.md). `scripts/serve.py` serves the repo at `/` and no
   longer works with current webparacraft.
2. Install **Immersive Web Emulator** in Chrome, open DevTools → *WebXR* tab, pick a Quest device.
3. Open `http://127.0.0.1:8088/npl/webparacraft/index.html`, trigger `isXR = true`
   from a UI button. Move the emulated headset/controllers in the DevTools panel.

### Quest headset over USB (no certificates needed)

```bash
adb devices                       # enable developer mode on the Quest first
adb reverse tcp:8088 tcp:8088     # headset's localhost:8088 → your PC's 8088
```

Open `http://localhost:8088/npl/webparacraft/index.html` in Quest Browser. For logs and breakpoints, open
`chrome://inspect/#devices` on the PC and inspect the Quest Browser tab.

### Over LAN / staging

Serve over HTTPS (nginx config in `NPLRuntime/Platform/SDL/readme.md`, or `mkcert` locally).
If embedded in an `<iframe>`, add `allow="xr-spatial-tracking; cross-origin-isolated"`.

---

## 5. Known issues in the current WebXR code

Found while reviewing the code; fix these before building on top of it.

| # | Where | Problem | Fix |
|---|-------|---------|-----|
| 1 | `emscripten/main.cpp` frame callback | `static ParaWebXRRigidTransform *_controllerTransformations;` is a **null pointer** passed to `webxr_get_input_pose`, so controller poses are written to wasm address 0 (silent memory corruption, not a crash). | Use a stack struct: `ParaWebXRRigidTransform pose; if (webxr_get_input_pose(&sources[i], &pose, …)) { … }`. |
| 2 | same | `ParaWebXRInputSource sources[2];` but `webxr_get_input_sources(sources, 5, …)` → stack overflow when more than 2 input sources exist (hands + controllers, trackers). | Pass `2` (or size the array to the max you pass). |
| 3 | same | `leftHandKey` / `rightHandKey` uninitialized when that hand is absent; return value of `webxr_get_input_pose` ignored. | `int leftHandKey[7] = {0};` etc.; skip pose update on `false`. |
| 4 | `library_webxr.js` `_nativize_input_source` | Dereferences `gamepad` without a null check → `TypeError` every frame for hand-tracking inputs (no gamepad). | `if (gamepad && gamepad.buttons[i])`, same for axes; write zeros otherwise. |
| 5 | `library_webxr.js` `_set_input_callback` | `_malloc(8)` but writes a 60-byte struct (heap overflow); uses undefined variable `i`. Only hit if `webxr_set_select*_callback` is used (currently unused). | `_malloc(60)` (or a `sizeof` passed from C), use the real source index. |
| 6 | `library_webxr.js` `toFeatureList` | Returns `features` instead of `f`. Currently masked because `webxr_request_session` drops the feature args. | Return `f`, and forward `requiredFeatures/optionalFeatures` in `webxr_request_session`. Request `local-floor` as optional if you want floor-relative tracking — it is not granted unless requested. |
| 7 | `library_webxr.js` reference spaces | `WebXR.refSpace` is set by whichever `requestReferenceSpace` promise resolves *last*, not by preference order. | Await them in order and pick the first that succeeds. |
| 8 | GL context | `makeXRCompatible()` may lose/restore the context on some systems. | Create the WebGL context with `xrCompatible: true` (via the SDL/Emscripten context attributes or `Module.preinitializedWebGLContext`) so no switch is needed. |
| 9 | ASYNCIFY | The XR frame callback calls `RunLoopOnce()` from the XR RAF, outside Emscripten's main loop. If anything in that path unwinds with ASYNCIFY (e.g. `emscripten_sleep`), the frame breaks. | Keep blocking/async calls out of the XR frame path. |
| 10 | Emscripten API drift | `mergeInto(LibraryManager.library, …)` is deprecated in newer Emscripten (use `addToLibrary`); library code also relies on `setValue`/`getValue`/`_malloc` being available. | If you upgrade past 3.1.59, add `__deps: ['$setValue','$getValue','malloc','free']` where needed and switch to `addToLibrary`. |

---

## 6. Troubleshooting

| Symptom | Cause |
|---------|-------|
| Error callback `-2` (API unsupported) | Page is not a secure context (use `localhost` via `adb reverse`, or HTTPS), or the browser has no WebXR. |
| Error callback `-3` (GL incapable) | `makeXRCompatible()` rejected — no XR device/runtime, or GPU blocklisted. |
| `requestSession` rejected with *NotAllowedError* | Not called from a user gesture. Trigger `isXR` from a click. |
| `requestSession` rejected with *NotSupportedError* | A *required* feature isn't supported (see issue 6) or `immersive-vr` not available. |
| Black screen in headset, desktop still renders | Engine rendered to framebuffer 0 instead of the XR layer FBO — something rebinds FBO 0 during the XR frame. |
| Only left eye renders / squashed image | Canvas wasn't resized to `2 × eye width` (see `SetSDLWindowSize` in the first XR frame) or `VIEW_LAYOUT_STEREO_WEBXR` not applied. |
| `SharedArrayBuffer is not defined` | Multi-thread build without COOP/COEP headers — use `scripts/serve.py` or the nginx headers. |

## References

- WebXR Device API: https://www.w3.org/TR/webxr/
- MDN WebXR: https://developer.mozilla.org/en-US/docs/Web/API/WebXR_Device_API
- Emscripten JS libraries: https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html
- The original wrapper this code is derived from: https://github.com/WonderlandEngine/emscripten-webxr
- Immersive Web Emulator: https://github.com/meta-quest/immersive-web-emulator
