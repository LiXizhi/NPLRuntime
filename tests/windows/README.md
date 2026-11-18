# Windows Paracraft Launchers (World PID 530)

For browser Haqi tests, use [run_haqi_webgl.bat](run_haqi_webgl.bat) or
[run_haqi_webgpu.bat](run_haqi_webgpu.bat). See the [Haqi browser test guide](../web/README.md)
for prerequisites, server options, and camera/label regression checks.

Pure Windows batch launchers that start `build-win64\bin\ParaEngineClient.exe` normally in `..\ParaEngine\Paraworld\` and load **World PID `530`** (`paracraft://cmd/loadworld/530`) with no injected test harness or background CLI server.

## One-Click Renderer Launchers

| Batch File | Graphics Backend | Notes |
| :--- | :--- | :--- |
| `run_paracraft_530_auto.bat` | `Auto` (`D3D12` -> `Vulkan` -> `D3D11` -> `OpenGL`) | Default Windows priority order |
| `run_paracraft_530_d3d12.bat` | `DirectX 12` (Hardware GPU) | Modern flip-model DXGI swapchain |
| `run_paracraft_530_d3d12_warp.bat` | `DirectX 12 (WARP)` | Microsoft CPU software rasterizer |
| `run_paracraft_530_vulkan.bat` | `Vulkan 1.1+` | Cross-platform explicit RHI backend |
| `run_paracraft_530_d3d11.bat` | `DirectX 11` (`FL 9_3` - `11_1`) | Broad Windows 7/8/10/11 hardware compatibility |
| `run_paracraft_530_d3d11_warp.bat` | `DirectX 11 (WARP)` | Microsoft CPU software rasterizer |
| `run_paracraft_530_opengl.bat` | `OpenGL 3.3 / 2.1` | Legacy OpenGL backend |
| `run_paracraft_530_webgpu.bat` | `WebGPU` | Uses Dawn/wgpu if present, otherwise falls back cleanly |

## Parameterized Launcher

```bat
launch_paracraft_530.bat [renderer] [world_pid] [extra_args...]
```

- **Default `renderer`**: `auto`
- **Default `world_pid`**: `530` (pass another number, e.g. `launch_paracraft_530.bat d3d12 1234`, to open a different world)
- **Environment Overrides**:
  - `PARAWORLD_ROOT`: Custom path to `ParaEngine\Paraworld` (defaults to `<repo>\..\ParaEngine\Paraworld`).
  - `PARAENGINE_EXE`: Custom path to `ParaEngineClient.exe` (defaults to `<repo>\build-win64\bin\ParaEngineClient.exe`).
