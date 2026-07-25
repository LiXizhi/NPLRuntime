# Server third-party sources

This directory contains source snapshots required to build NPLRuntime and the Paracraft client.

## MuJoCo and ONNX Runtime

| Directory | Upstream | Version |
|---|---|---|
| `mujoco-3.10.0` | <https://github.com/google-deepmind/mujoco> | 3.10.0 |
| `onnxruntime-1.27.1` | <https://github.com/microsoft/onnxruntime> | 1.27.1 |

The snapshots include upstream source, tests, model fixtures, build files, and license files. Local Python bytecode caches remain ignored. Keep the directory name, upstream version, and client CMake configuration in sync when upgrading either dependency.