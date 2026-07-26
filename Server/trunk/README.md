# Server third-party sources

This directory contains source snapshots required to build NPLRuntime and the Paracraft client.

## MuJoCo and ONNX Runtime

| Directory | Upstream | Version |
|---|---|---|
| `mujoco-3.10.0` | <https://github.com/google-deepmind/mujoco> | 3.10.0 |
| `onnxruntime-1.27.1` | <https://github.com/microsoft/onnxruntime> | 1.27.1 |

The snapshots include upstream source, tests, model fixtures, build files, and license files. Local Python bytecode caches remain ignored. Keep the directory name, upstream version, and client CMake configuration in sync when upgrading either dependency.

## Windows robot runtime build

The Windows x64 client build includes MuJoCo and ONNX Runtime by default:

* MuJoCo is a CMake-compatible subproject and is built directly with `add_subdirectory`.
* ONNX Runtime is isolated with CMake `ExternalProject` and built through its official
	`tools/ci_build/build.py` driver. This prevents its protobuf, abseil, re2, and other
	dependencies from changing the parent project's CMake state.
* ONNX Runtime follows the active Paracraft configuration. Debug uses `/MTd`; Release
	uses `/MT`. The current integration builds the Windows x64 CPU execution provider
	and shared `onnxruntime.dll` only.

The first x64 build requires Python 3, Git, network access for third-party dependencies,
Visual Studio 2022 with the v143 C++ toolset, CMake, Boost 1.85 or newer, and the DirectX
SDK (June 2010). The parent project may use Visual Studio 2026 with v143; the isolated
ONNX Runtime build uses the Visual Studio 2022 generator because its build driver cannot
select v143 from the Visual Studio 2026 generator. Later builds reuse ONNX Runtime's
external build directory.

From `paracraft-64bit-cli`, run:

```
npm run build:windows:x64:debug
npm run build:windows:x64:release
```

The build copies `onnxruntime_d.dll` for Debug or `onnxruntime.dll` for Release,
the MuJoCo runtime, and Paracraft binaries to `ParaWorld/bin64`. ONNX Runtime is
disabled for the current Windows x86 build.