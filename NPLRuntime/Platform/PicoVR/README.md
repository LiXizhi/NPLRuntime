# PicoVR: native VR app for PICO headsets

This builds Paracraft as a native OpenXR APK for PICO 4, PICO 4 Ultra and Neo3. It does not use the Web build. Most of the code is shared with `../AndroidStudio/app`:

| Shared from AndroidStudio/app | Added by PicoVR |
|---|---|
| Java (`ParaEngineActivity` and the rest), res, assets | `PicoVRActivity`: requests an OpenGL ES 3 context |
| Native code in `cpp/` (JNI, file system, AppDelegate) | `cpp/PicoXR/PicoOpenXR.*`: OpenXR instance, session, swapchain, controllers, frame loop |
| | `cpp/3dengine/WebXR.*`: implements `IParaWebXR` for PICO |

## Build

```bash
cd NPLRuntime/Platform/PicoVR
./gradlew :PicoVR:assembleDebug      # or assembleRelease, or open this folder in Android Studio
adb install -r app/build/outputs/apk/debug/paracraft-pico-debug-*.apk
```

- **Boost:** the build needs a Boost built for Android (`<root>/android-build/stage/arm64-v8a/lib`), the same one the phone build uses. Set its path in `local.properties`:
  ```
  android.boost.root=/path/to/boost_1_74_0
  ```
  You can also pass `-PANDROID_BOOST_ROOT=...` on the command line. Either one overrides the `BOOST_ROOT` environment variable, which may point to a desktop Boost. If neither is set, `BOOST_ROOT` is used.

- Assets come from `../AndroidStudio/app/assets`. Prepare them the same way as for the phone build.
- `-DPICO_VR=ON` makes the root CMake build `Platform/PicoVR/app` in place of `AndroidStudio/app`.
- The OpenXR loader is the Khronos `org.khronos.openxr:openxr_loader_for_android` prefab package. Set the version with `OPENXR_LOADER_VERSION` in `gradle.properties`.
  - On old PICO OS versions without the standard loader, use the loader `.so` from the PICO OpenXR SDK. Link that in place of `OpenXR::openxr_loader` in `app/CMakeLists.txt`.
- Read the logs with `adb logcat -s ParaEngineXR ParaEngine`.

## How it works

```
GLSurfaceView GL thread (ES3 context)
  └─ ParaEngineRenderer.onDrawFrame → AppDelegate::handle_mainloop   [PICO_VR]
        └─ CPicoOpenXR::Frame
              xrPollEvent → xrWaitFrame → xrBeginFrame
              xrLocateViews + xrSyncActions → CParaWebXR::UpdatePicoXRFrame
              bind swapchain FBO (2*eyeW x eyeH) → CParaEngineApp::DoWork()
              xrEndFrame(one projection layer: left half and right half)
```

- **Render target:** the render window is `2 * eyeWidth x eyeHeight`. The engine uses its existing `VIEW_LAYOUT_STEREO_WEBXR` layout to draw the left eye in the left half and the right eye in the right half.
- **Engine interface:** the engine sees the same `IParaWebXR` attribute fields as the WebXR build (`isXR`, `webXRLeftView`, `webXRLeftPosition`, ..., `leftHandKey0..6`, `webXRLeftAxes`). The Lua VR code therefore stays unchanged.
  - The PICO build also adds `webXRLeftFov` and `webXRRightFov`: `(angleLeft, angleRight, angleUp, angleDown)` in radians.
- **Reference space:** `LOCAL` (same as WebXR `local`): right-handed, y up.
- **Controller buttons:** follow the xr-standard order, `0 trigger, 1 squeeze, 2 touchpad (none), 3 thumbstick click, 4 A/X, 5 B/Y, 6 menu`.
  - Axes are `(0, 0, thumbstickX, thumbstickY)`. y is positive downward, as on WebXR gamepads.
- **XR mode:** `isXR` turns on with the first headset frame. It can not be turned off.

## Known limitations of this first version

- **Projection:** the projection matrix still comes from the scene camera fov. The headset's asymmetric per-eye fov is exposed (`webXR*Fov`) but not applied yet, so scale and stereo may feel slightly off.
- **Head tracking:** the Lua side must apply head tracking to the camera, as in the WebXR build. Check that the VR camera code runs when `isXR` is already true at startup.
- **UI before a world is loaded:** WebXRCamera only starts in a world. Until then the 2D GUI (login, world list) is shown on a quad layer fixed 1.5 m in front of the start pose. The controller ray drives the mouse with a square cursor, and the trigger is the left button. Inside a world WebXRCamera draws a 3D GUI and handles the controllers.
- **Lifecycle:** when the GL surface is destroyed, the OpenXR session is destroyed and created again on the next surface.
- **Performance:** there is no multiview or foveation yet. Turn off shadows and post processing for a stable 72/90 Hz.
