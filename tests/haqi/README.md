# Haqi launchers and tests

## Native macOS / Windows

Build the native client first (`./build_linux.sh client --no-robotics` on macOS,
or `build_win32.bat` on Windows). Python 3, a sibling `webparacraft` checkout
with its generated Haqi packages, and `../ParaEngine/paraworld` are required.

```bash
tests/haqi/run_haqi_mac.sh
tests/haqi/run_haqi_mac.sh --renderer metal
```

On Windows, double-click `tests/haqi/run_haqi_native.bat`, or run:

```bat
tests\haqi\run_haqi_native.bat --renderer opengl
```

Both wrappers call `launch_haqi_native.py`, which selects the newest built
native executable. Supply `--exe` (or `PARAENGINE_EXE`) to select one explicitly.
`--web-root` / `WEBPARACRAFT_ROOT` and `--paraworld-root` / `PARAWORLD_ROOT`
override the source directories. Paths containing spaces must be quoted.

By default, the native test launcher signs in with the KeepWork login remembered
in that runtime. On the first run, sign in with **Remember password** selected.
Subsequent launches select the last character if it still belongs to the account,
or the first available character otherwise. The launcher rejoins that character's
last server when available, then falls back to Haqi's recommendation or another
available server. Pass `--manual-login` to choose an account, character, and server
yourself. Add `--profile-unlimited-fps` when you want an uncapped profiling session.

Each run prepares `build/haqi-mac-runtime` on macOS or
`build/haqi-native-runtime` on Windows (`--runtime-dir` overrides this):

- Refreshes Haqi packages and database seeds from
  `haqi1_paracraft_asset.{js,data}` and `paracraft_asset_script.{js,data}`.
- Refreshes `assets_manifest.txt` from the local webparacraft root if present,
  otherwise from `paracraft_asset_script.data`. `--asset-manifest` explicitly
  selects another file. The launcher prints its source, entry count and SHA-256.
  It does not pull webparacraft or download a newer distribution automatically.
- Links `script/` to Paraworld source (a directory junction on Windows) and
  copies `config/TaoMee.GameClient.config.xml` from that source on every run.
  The runtime is the `dev` directory, so Paraworld's manifest cannot override
  the refreshed webparacraft manifest.
- Creates missing `Database/` files from the seeds, preserving existing user
  databases. Optional `paracraft_asset_temp` files seed missing cache entries.

The client receives `noupdate=true isDevEnv=true debug=main mc=false`,
`bootstrapper=script/apps/Aries/main_loop.lua`, and
`config=config/TaoMee.GameClient.config.xml`. Runtime files stay outside the
source checkout's game data. Logs are `log.txt` and `stdout.log` in the runtime.
The launcher starts a new instance; close an existing instance before relaunching.
Use `--prepare-only` to refresh files without starting a client (preferably while
that runtime is closed). The Windows wrapper requires `python` on PATH;
the macOS wrapper uses `python3`.

### macOS Release builds and performance checks

Old macOS CMake caches may retain the previous forced `-O0` Release flags.
Clear those two cache entries once, then rebuild (substitute your build directory):

```bash
cmake -S NPLRuntime -B build/mac-current -U CMAKE_C_FLAGS_RELEASE -U CMAKE_CXX_FLAGS_RELEASE
BUILD_DIR="$PWD/build/mac-current" ./build_linux.sh client --no-robotics
cmake --build build/mac-current --target MacLuaJITSmoke
ctest --test-dir build/mac-current -R mac_luajit_gc_smoke --output-on-failure
```

The macOS client links a universal LuaJIT 2.1 archive built from
[upstream revision 63c8f1a9](https://github.com/LuaJIT/LuaJIT/commit/63c8f1a9b479ba5518cf6a170ff7729178bc3cfd).
Run `bash NPLRuntime/externals/LuaJIT2_1/build_mac.sh` to regenerate both slices
and their public headers. Pass a local LuaJIT Git checkout as the script's first
argument for an offline rebuild. The smoke test checks header/runtime agreement,
trace compilation, numeric string conversion and garbage collection.

For graphics performance, the macOS OpenGL UP buffer path maps unused ranges
without synchronization and discards storage before reusing a range. Using
`glBufferSubData` for every character draw caused repeated GPU waits on Apple
Silicon. The Mac loop also subtracts rendering time from the requested frame
interval instead of adding a fixed 10 ms wait after every frame.

`debug=main` starts the NPL debug listener; it does not attach a debugger.
When measuring, check `debug.gethook()` and `jit.status()` and keep the scene,
camera, resolution, and foreground state comparable. Do not measure during a build.

### Uncapped profiling

The [Haqi plaza profile](plaza-profile-2026-11-20.md) compares native and NPL hot spots under matching uncapped conditions. The [HUD batching follow-up](plaza-hud-batching-2026-11-20.md) measures draw counts, screenshot equivalence, bone evaluation, static mesh submission and NPL logging.

`ProfileUnlimitedFPS` defaults to false. Changes apply to the current process.
Use either command from the NPL console while looking at the same scene:

```lua
ParaEngine.GetAttributeObject():SetField("ProfileUnlimitedFPS", true)
ParaEngine.GetAttributeObject():SetField("ProfileUnlimitedFPS", false)
```

For a repeatable launch, pass `profile_unlimited_fps=true` in the engine command
line. Both native Haqi launcher wrappers also accept `--profile-unlimited-fps`.
Read `ParaEngine.GetAttributeObject():GetField("FPS", 0)` a few seconds after
switching. The counter measures rendered frames. The physical display and
compositor determine how many become visible.

| Graphical host | Behavior during profiling |
|---|---|
| macOS OpenGL and Metal | Removes the host timer delay and requests unsynchronized presentation. |
| Windows and native SDL | Removes the foreground timer delay. WGL and SDL OpenGL request swap interval zero. D3D11/12 already submit with interval zero. Vulkan selects an available mailbox or immediate mode, with FIFO fallback. The Windows background throttle still applies when enabled. |
| Browser | Requests immediate Emscripten callbacks while the page is visible and active. WebXR retains its own frame callbacks. WebGPU backends with surface capability queries use an advertised immediate or mailbox mode when available. The legacy WebGPU backend keeps its configured presentation mode. |
| Android | Bypasses the renderer's optional Java sleep and requests EGL swap interval zero on the GLSurfaceView render thread. EGL implementations can impose a minimum interval. |
| iOS app and embedded framework | Requests the screen maximum on the existing display link. Actual rates depend on the device, power policy and the host app's ProMotion setting in its Info.plist (`CADisableMinimumFrameDurationOnPhone`). |
| PICO OpenXR | Bypasses the engine frequency gate. The headset runtime continues to schedule XR frames. |

Switching the field off resumes the current `RefreshTimer` policy and restores
the host's ordinary presentation settings. Headless servers expose no profiling
field. The HarmonyOS scaffold has no active graphical frame loop to exercise.

### Platform scope

- Buffer storage is discarded before overlapping uploads in the shared OpenGL
  helper, including WebGL callers. Only macOS desktop uses unsynchronized mapped
  uploads; the other platforms retain `glBufferSubData`.
- The S3TC format constants are defined wherever platform headers omit them.
  Device capability checks still decide whether compressed textures can be used.
- Release flags and the LuaJIT linker workaround are scoped to macOS. Other
  graphics backends have their own buffer upload implementations.
- macOS and native SDL frame scheduling subtract elapsed frame work from the
  requested interval and update the base frame controller when it changes.
  The browser uses animation-frame scheduling by default.
- macOS and iOS relative file lookups honor the configured writable-directory
  override, so downloaded assets can be found and renamed in that directory.
- The native launcher has macOS and Windows wrappers. macOS gameplay was verified;
  Windows launch and non-macOS graphics changes still need platform validation.

With `NPLRUNTIME_BUILD_TESTS=ON`, run the portable frame-budget test and the
actual Foundation-based iOS file utility on macOS:

```bash
cmake --build build/mac-current --target FramePacingTest IOSWritablePathTest
ctest --test-dir build/mac-current -R 'frame_pacing_unit|ios_writable_path_host' --output-on-failure
```

The file test uses a temporary directory and covers the default path, overrides,
absolute paths, directory creation, asset rename, read, copy, cross-directory
move, deletion, and changing the override. It does not replace an iOS device
build or sandbox validation. Native SDL gameplay also needs platform testing.

## Browser tests

Build the current engine with `scripts/dev_web.ps1 build` on Windows or
`scripts/dev_web.sh build` on Linux/macOS. Install Python 3 and have a sibling
`webparacraft` checkout containing `haqi1.html` and the Haqi asset packages.

On Windows, double-click either launcher.

- [run_haqi_webgl.bat](../windows/run_haqi_webgl.bat)
- [run_haqi_webgpu.bat](../windows/run_haqi_webgpu.bat)

The launcher opens Haqi in the default browser and serves on loopback port 8091.
Keep the server terminal open while testing. Ctrl+C stops it. Starting the other
launcher reuses the matching server and opens Haqi with the other renderer.

The shell and game packages come from `../webparacraft`. Both engine files
come from this repository's `web/auto_generate_files`, overriding the shell's
engine. The served hash manifest uses the local engine hashes. Responses disable
caching, so after a completed rebuild, refresh the page to load the new engine.

## Command line

Run from the repository root.

```powershell
python tests/haqi/launch_haqi.py --renderer webgl
python tests/haqi/launch_haqi.py --renderer webgpu
python tests/haqi/launch_haqi.py --renderer webgl --web-root C:/lxzsrc/webparacraft --port 8093
```

Use `--engine-dir` to select another directory containing the matching
`ParaCraftSingleThread.js` and `ParaCraftSingleThread.wasm` pair.
Use `--no-browser` to serve without opening a tab. Windows batch launchers
forward these options as well. If another service occupies the port, select a
free port with `--port`.

## 家园世界 missing movement region

```powershell
python tests/haqi/launch_haqi.py --test-missing-region
```

This opens a disposable WebGL world on port 8092 and runs the check at startup.
The Haqi shell reads the fixture's Lua and terrain files through its
`searchroot` and `searchpath` overlay. The launcher's own loopback server
handles those file requests. The interactive game on port 8091 can stay open.

Open the browser console and look for `TERRAIN_REGION_CHECK_PASS`. The test
registers the missing `move_37_37.png`, queries it through
`ParaTerrain.GetRegionValue`, and checks an unreadable image, an unregistered
layer, and known colors on two terrain tiles. A Lua assertion prints
`TERRAIN_REGION_CHECK_FAIL`. The affected engine traps on the missing movement
region before it can print a result. To compare builds, pass `--engine-dir`.

## Manual checks

### GUI draw batching

Run `python tests/haqi/launch_haqi.py --test-gui-batching --draw-stats` for an offline WebGL scene.
The panel draws 48 overlapping translucent icons, alternating between power-of-two
and non-power-of-two textures. Below it are magnified, cropped, and repeated
images. Chinese HUD labels use three scales. Five overhead names are above the
ground and one is below it. Zoom the camera to check label sampling and depth.

Use `--draw-stats` for the regular Haqi launcher or add `&drawstats=1` to a
local Paracraft WebGL URL. The overlay shows GUI draws, other draws, the total
and the rendered frame interval. Click the gear for Settings or press **E** for
the inventory to compare the production panels.

Once the scene settles, paste [the draw counter](measure_gui_draws.js) into Chrome
DevTools. It reports built-in GUI draws, other draws, and elapsed milliseconds
between rendered frames over 120 samples. The same counter works in a logged-in
world. To compare engine versions, use the same browser, viewport and camera
with `--engine-dir` pointing to each saved JS/WASM pair.

In single-thread WebGL, the engine can pack small static GUI images as they
upload. It keeps the original textures for other uses and out-of-range UVs.
The atlas has two 1024 × 1024 pages at most, allocated on demand. Each page
uses the source pixel format. Compressed or mipmapped images use their original
textures. To disable atlas allocation, append `&cmdline=guiatlas%3D%22false%22`
to the launcher's test URL and reload. This passes `guiatlas="false"` to the
engine. Text uses the regular glyph atlases in both modes.

On supported OpenGL devices the GUI can bind four textures in one draw.
To disable multi-texture batching while keeping the atlas enabled, append
`&cmdline=guimultitexture%3D%22false%22` and reload.

Before multi-texture batching, in a desktop Chrome WebGL 2 capture (SwiftShader, 120 rendered frames), the
offline scene submitted 62 GUI draws per frame on the previous engine, 57 with
only the label batching change, and 10 with both changes. The six depth-tested
names went from six draws to one. The 3D scene submitted nine draws in each
run. Enabling the image atlas allocated one 4 MiB RGBA8 page. All three
captures matched pixel for pixel at 944 × 625. These counts are for the fixed
camera and assets in this fixture.

In the installed Paracraft app (`C:/Paracraft`, world `worlds/DesignHouse/test`),
the gear menu went from 34 GUI submissions to 6 and the block inventory went from
10 to 1 with multi-texture batching. The scene submitted 94 other draws per
frame in both cases at 944 x 625. The four-slot shader keeps sprite order and
samples glyph alpha directly. The test with 48 overlapping labeled Haqi buttons
gave identical before/after pixels and 103 GUI draws in both runs.

In the signed-in Haqi WebGL 2 client (1.2.288), the following measurements
used server 002 (泡泡广场), the town center on Haqi Island, the same character
and a 1494 × 714 desktop Chrome canvas. Both runs kept the GUI image atlas
enabled and used the normal glyph rendering path. Each value is the median
over 120 rendered frames after world entry.

| Visible UI | GUI draws, single texture | GUI draws, four textures | Four-texture shader draws (included in GUI) | Other draws, single / four |
| --- | ---: | ---: | ---: | ---: |
| Normal HUD, including quests, chat and hotbar | 72 | 37 | 9 | 729 / 747 |
| Daily rewards panel over the HUD | 128 | 75 | 13 | 697 / 746 |

To repeat, launch the regular game with `--draw-stats` and append `&bench=1`
to display medians for successive groups of 120 rendered frames. Use the same
server, character and camera for both launches. After changing the visible UI,
allow two full sample groups to pass. Other players and announcements change
during live runs. These desktop measurements establish draw-call savings only.

### Profile the town-center 3D draws

Append `&sceneprofile=1` to the regular `--draw-stats` game URL. After the
world has loaded, select **Capture scene draws**, then **Save … draws**.
The JSON contains one rendered frame in submission order, including the
WebGL method, instance count, shader uniforms, buffer and texture IDs, and
WASM stack indices. IDs are local to each capture. For C++ function names,
link the test engine with `--emit-symbol-map` and use the map from that exact
binary. The capture reads back GL state, so use separate frames for timing.

In a 1494 × 714 capture from server 002, the normal HUD used 45 draws. The
other 493 comprised 172 ordinary static-mesh draws, 132 ParaX model and
attachment draws, 109 terrain draws, 53 vegetation draws, 12 ocean draws,
6 drop shadows, 5 sky draws and 4 sorted transparent face-group draws.
Population varied between captures.

**Toggle scene instancing** switches the live scene's `UseInstancing` attribute.
**Compare scenery** saves four real frames from one camera. It captures both
settings off, extended terrain only, both on, then both off again. For these
four frames the diagnostic filters the main framebuffer to the indexed scenery
and terrain shader draws and saves a lossless PNG with each trace. The repeat
gives a baseline for effects that animate between frames. Normal rendering and
both original settings are restored automatically. Use **Capture scene draws**
for a full frame with players and HUD.

In the Haqi island town center on server 002, desktop Chrome reported 16
fragment texture units. The release WASM client gave these counts per frame.
Terrain includes the distant fog pass.

| Viewport | Static meshes off / on | Terrain 3 / 7 layers per pass | Combined draws saved |
| --- | ---: | ---: | ---: |
| 1680 × 1225 | 80 / 45 | 45 / 23 | 57 |
| 1494 × 714 | 111 / 67 | 63 / 35 | 72 |

In the wider view, batches of 12, 16 and 19 instances replaced 47 individual
draws with 3. Full unfiltered frames confirmed 111 and 67 static draws and 44
GUI draws in both. The 19 visible textured terrain cells took 59 and 31 draws
respectively, plus four fog draws in both modes. Seven cells fit in a single
extended pass. The others retained their extra layers in additive passes.

With both optimizations enabled, 99.80% of foreground pixels in the wider
view were within one 8-bit RGB level of the baseline. Repeating the baseline
gave 99.84% by the same measure. With only instancing switched, the sampled
fences, planters, stone wall, paving, palms and buildings differed by at most
one RGB level. These captures measure submissions and image fidelity on
desktop WebGL 2. Mobile frame time and battery use need device measurements.

Haqi enables static scenery instancing at client startup when the active
renderer supports it. The setting carries across world loads. To check the
default, enter the plaza after a fresh launch and use **Capture scene draws**
before touching either switch. Eligible repeated meshes use
`drawElementsInstanced`. For an in-place comparison, use **Toggle scene
instancing** or set the scene's `UseInstancing` field from Lua.

On a fresh 1680 × 1225 plaza launch, the first unmodified capture contained
281 draws, including three instanced scenery draws for 12, 13 and 13 objects.
At the same camera, switching instancing off raised scenery draws from 43 to
78. Switching terrain from seven to three layers per pass raised terrain draws
from 23 to 45. With both defaults on, 99.95% of foreground pixels were within
one RGB level of the baseline, compared with 99.97% for a repeated baseline.

Extended terrain is enabled by default for the built-in terrain effect on
desktop OpenGL and OpenGL ES 3 / WebGL 2 with at least 16 fragment samplers. To compare with
the narrower path in a live world, set
`ParaTerrain.GetAttributeObject():SetField("UseExtendedTerrainLayers", false)`.

On macOS, the desktop GLSL target must compile `TERRAIN_SEVEN_LAYERS` too.
Otherwise the renderer can submit seven layers to a shader that only samples
three, leaving black patches wherever the omitted layers contain the ground.
The terrain effect checks compiled parameter bindings before advertising the
seven-layer path: FX declarations inside inactive `#ifdef` branches alone do
not establish support. Other shader targets retain the narrower path unless
their compiled terrain shader supports the extra layers.

For a native regression check, launch Haqi into the island plaza and inspect
the grass banks, paving, and distant ground with `UseExtendedTerrainLayers`
both false and true from the same camera. Texture coverage should match; leave
the option true after testing. Check a fresh launch as well so a previous
session's disabled setting cannot hide the bug.

### Avatar and display checks

Character triangles use an adaptive budget by default on all engine backends.
`MaxCharTriangles` remains the configured base limit (150,000 in Haqi), while
the read-only `EffectiveMaxCharTriangles` reports the current rendering limit.
Every two seconds, if characters were omitted and the time-weighted average
FPS is at least 50, the effective limit grows by 25%. An average below 35
reduces it by 20%, down to the configured base. Between those thresholds it
holds steady. Brief FPS dips and frames up to 500 ms contribute to the average
instead of restarting a fast-frame streak and preventing further growth.
Background windows, invalid FPS samples, and loading stalls do not grow it.
Changing `MaxCharTriangles` or resetting the scene clears the learned budget.

For a strict limit, set the scene's `AdaptiveCharTriangleBudget` field false;
this immediately restores the configured budget. Setting it true resumes
adaptation. A zero or negative base retains the previous rendering behavior.
Other culling and per-importance character-count limits still apply.

Build `AdaptiveCharacterBudgetTest` and run
`ctest --test-dir build/mac-current -R adaptive_character_budget_unit --output-on-failure`
for the FPS/budget regression cases. In a crowded Haqi plaza, compare strict
and adaptive modes without moving the camera, wait for the budget to grow,
and check that previously omitted characters appear while FPS stays healthy.
The read-only `CharacterTriangleBudgetCulled` scene field counts character
draws skipped by the triangle budget in the previous frame (excluding shadow
passes). Check that this reaches zero; a single budget increase is insufficient
evidence that the crowd now fits.

For the missing avatar body and face, enter a Paracraft world from Haqi with
a customized character. Paste [test_avatar_texture_reload.lua](test_avatar_texture_reload.lua)
into the client's Lua console and run it. It unloads the player's body and face
textures as world entry does, then checks that both reload with their original
asset keys and dimensions.
Expect `HAQI_AVATAR_RELOAD_PASS` in the browser console.

1. Log in to Haqi and enter the 3D world.
2. Rotate and zoom the camera around characters and pets with overhead names.
3. Check that the world renders and changing label sizes causes no crash.
4. Check the browser console for WASM memory errors and WebGPU validation errors.
5. Repeat with the other launcher.

For the High-effects GUI regression, load
[`test_web_high_effect_gui.lua`](../../NPLRuntime/tests/test_web_high_effect_gui.lua)
in the browser client. Keep the browser size unchanged during its 135-frame run.
It switches between High and normal effects and submits three desktop resolution
presets, checking that the browser drawing size and GUI hit tests stay correct.
Expect `WEB_HIGH_EFFECT_GUI_PASS`; both corner buttons must show readable English
and Chinese text and change their labels when clicked. Close the overlay to
restore the original effect level. Also resize the browser and repeat the click
checks to verify that real canvas resizes still work.

The focused font-atlas regression is also available in
[`test_font_atlas_scale.lua`](../../NPLRuntime/tests/test_font_atlas_scale.lua).
It draws seven label scales in one batch and logs `FONT_ATLAS_SCALE_PASS`
after 120 rendered frames.

[`test_font_atlas_cache.lua`](../../NPLRuntime/tests/test_font_atlas_cache.lua)
adds Chinese labels, repeated scales, and a twelve-size stress phase.
It logs `FONT_ATLAS_CACHE_PASS` after 240 frames. A texture-allocation
profiler can use its `FONT_ATLAS_CACHE_BEGIN/END` markers: `warm` and `rewarm`
draws should allocate no textures and upload no glyphs. The `evict` and
`evict_repeat` phases must render safely; whether they trigger eviction depends on the renderer's font-size bucketing and UI scale.
