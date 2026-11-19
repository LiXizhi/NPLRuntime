# Haqi browser tests

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

### Avatar and display checks

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
