# Windows Paracraft Renderer Launchers (World PID 530)

Batch launchers in this folder start the built `ParaEngineClient.exe` (`build-win64\bin\ParaEngineClient.exe`) against the Paracraft application repository in `..\ParaEngine\Paraworld\` (relative to the `NPLRuntimeUnified` root) and automatically load **World PID `530`** (`paracraft://cmd/loadworld/530`).

## Interactive One-Click Launchers

| Batch File | Graphics Backend | Notes |
| :--- | :--- | :--- |
| `run_paracraft_530_auto.bat` | `Auto` (`D3D12` -> `Vulkan` -> `D3D11` -> `OpenGL`) | Default Windows priority order |
| `run_paracraft_530_d3d12.bat` | `DirectX 12` (Hardware) | Modern flip-model DXGI swapchain |
| `run_paracraft_530_d3d12_warp.bat` | `DirectX 12 (WARP)` | Software rasterizer for GPU-less / VM verification |
| `run_paracraft_530_vulkan.bat` | `Vulkan 1.1+` | Cross-platform explicit RHI backend |
| `run_paracraft_530_d3d11.bat` | `DirectX 11` (`FL 9_3` - `11_1`) | Broad Windows 7/8/10/11 hardware compatibility |
| `run_paracraft_530_d3d11_warp.bat` | `DirectX 11 (WARP)` | Software rasterizer fallback |
| `run_paracraft_530_opengl.bat` | `OpenGL 3.3 / 2.1` | Legacy OpenGL backend |
| `run_paracraft_530_webgpu.bat` | `WebGPU` | Uses Dawn/wgpu if present, otherwise falls back cleanly |

## Parameterized Launcher

```bat
launch_paracraft_530.bat [renderer] [world_pid] [--new-instance] [extra_args...]
```

- **Default `renderer`**: `auto`
- **Default `world_pid`**: `530`
- **`--new-instance`**: Keeps existing `ParaEngineClient.exe` processes running (lets NPL HTTP bind a free port instead of stopping port `8099` listeners).
- **Environment Overrides**:
  - `PARAWORLD_ROOT`: Custom path to `ParaEngine\Paraworld` (defaults to `<repo>\..\ParaEngine\Paraworld`).
  - `PARAENGINE_EXE`: Custom path to `ParaEngineClient.exe` (defaults to `<repo>\build-win64\bin\ParaEngineClient.exe`).

## Automated Multi-Renderer Smoke Test

```bat
test_all_renderers_530.bat
```

Sequentially launches Paracraft into World PID `530` across `d3d12`, `vulkan`, `d3d11`, and `opengl`, waits for `/ajax/paracraft_cli?action=health` (`worldEntered == true`), verifies `ParaEngine.GetStats(0)` matches each requested backend, and closes each client cleanly.
