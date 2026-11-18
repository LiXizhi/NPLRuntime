# Haqi browser tests

Build the current engine with `scripts/dev_web.ps1 build` on Windows or
`scripts/dev_web.sh build` on Linux/macOS. Install Python 3 and have a sibling
`webparacraft` checkout containing `haqi1.html` and the Haqi asset packages.

On Windows, double-click either launcher:

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

Run from any directory using the path to the launcher:

```powershell
python tests/web/launch_haqi.py --renderer webgl
python tests/web/launch_haqi.py --renderer webgpu
python tests/web/launch_haqi.py --renderer webgl --web-root C:/lxzsrc/webparacraft --port 8092
```

Use `--engine-dir` to select another directory containing the matching
`ParaCraftSingleThread.js` and `ParaCraftSingleThread.wasm` pair.
Use `--no-browser` to serve without opening a tab. Windows batch launchers
forward these options as well. If another service occupies the port, select a
free port with `--port`.

## Manual checks

1. Log in to Haqi and enter the 3D world.
2. Rotate and zoom the camera around characters and pets with overhead names.
3. Check that the world renders and changing label sizes causes no crash.
4. Check the browser console for WASM memory errors and WebGPU validation errors.
5. Repeat with the other launcher.

The focused font-atlas regression is also available in
[`test_font_atlas_scale.lua`](../../NPLRuntime/tests/test_font_atlas_scale.lua).
It draws seven label scales in one batch and logs `FONT_ATLAS_SCALE_PASS`
after 120 rendered frames.
