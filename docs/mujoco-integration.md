# MuJoCo 3.10.0 Integration

MuJoCo is built from `Server/trunk/mujoco-3.10.0`. The integration does not use an external MuJoCo checkout.

## Build requirements

- 64-bit target. MuJoCo 3.10.0 is disabled by default for 32-bit builds.
- MSVC with C++20 support for `NPLMuJoCoSimulation`.
- `NPLRUNTIME_MUJOCO=ON` (default for 64-bit builds).

The legacy ParaEngine targets remain on their existing C++ standard. Only the wrapper library that includes MuJoCo headers is compiled as C++20. The MuJoCo DLL is copied to the ParaWorld output directory after the client target is built.

## Lua API

The first integration milestone exposes a handle-based `ParaMuJoCo` namespace:

- `LoadModel(filename)` and `DeleteModel(handle)`
- `IsValid(handle)` and `GetLastError(handle)`
- `Reset(handle)`, `Forward(handle)`, and `Step(handle, count)`
- `GetQPosCount`, `GetQVelCount`, and `GetControlCount`
- `GetQPos`/`SetQPos`, `GetQVel`/`SetQVel`, and `GetControl`/`SetControl`
- `GetTime(handle)` and `NameToId(handle, objectType, name)`
- `GetBodyCount`, `GetJointCount`, `GetActuatorCount`, and `GetContactCount`
- `FindBody`, `FindJoint`, and `FindActuator`
- `GetBodyPosition` and `GetBodyQuaternion` for raw MuJoCo world poses
- `GetBodyParaPosition` and `GetBodyParaQuaternion` for converted ParaEngine world poses

Array indices use MuJoCo's zero-based indexing. `GetLastError(0)` returns the most recent model loading error when `LoadModel` returns zero.

## Coordinate conversion

`RobotCoordinateConverter` is the single conversion boundary between MuJoCo's right-handed, Z-up robot coordinates and ParaEngine's left-handed, Y-up world coordinates. MuJoCo positions map as follows:

```text
(x, y, z) -> (-y, z, x)
```

This maps MuJoCo forward, left, and up to ParaEngine forward, left, and up. Rotations use the same basis transformation, equivalent to `R_para = C * R_mujoco * inverse(C)`. MuJoCo quaternion input is `(w,x,y,z)`; `GetBodyParaQuaternion` returns ParaEngine order `(x,y,z,w)`. The smoke test covers all unit axes, identity rotation, and a 90-degree rotation.

## Smoke test

The independent smoke test does not require the full ParaEngine dependency set:

```powershell
cmake -S cmake/mujoco-smoke -B bin/mujoco-smoke -G "Visual Studio 17 2022" -A x64 -T v143
cmake --build bin/mujoco-smoke --config Debug --target MuJoCoSimulationSmokeTest --parallel
ctest --test-dir bin/mujoco-smoke -C Debug --output-on-failure
```

It loads a minimal articulated MJCF model, writes joint state and control data, advances 5000 fixed steps, checks finite state, and verifies reset behavior.

## Windows x64 client build

The DirectX client build was verified with Boost 1.87 at `D:/gits/paracraft/boost/boost_1_87_0` and the DirectX SDK June 2010. Build the required Boost libraries with `link=static runtime-link=static address-model=64`, then configure the client with:

```powershell
$env:BOOST_ROOT = "D:/gits/paracraft/boost/boost_1_87_0"
$env:DXSDK_DIR = "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/"
cmake -S Client -B bin/client-x64-mujoco-boost187 -G "Visual Studio 17 2022" -A x64 -T v143 `
	-DBOOST_ROOT="$env:BOOST_ROOT" `
	-DBOOST_LIBRARYDIR="$env:BOOST_ROOT/stage/lib" `
	-DNPLRUNTIME_RENDERER=DIRECTX `
	-DNPLRUNTIME_MUJOCO=ON
cmake --build bin/client-x64-mujoco-boost187 --config Debug --target ParaEngineClient --parallel 4
cmake --build bin/client-x64-mujoco-boost187 --config Debug --target ParaEngineClientApp --parallel 4
```

For testing in `D:/Paracraft_dev`, deploy `ParaEngineClient_d.exe`, `ParaEngineClient_d.dll`, `mujoco_Debug.dll`, `lua_d.dll`, `sqlite_d.dll`, and `libcurl_d.dll`. The verified client reached the normal Paracraft main loop without MuJoCo or core DLL loading errors.

## Paracraft runtime tests

`MuJoCoQuickTest.lua` advances the selected model by 5000 steps immediately and logs model dimensions, contacts, and raw/converted pelvis poses. `MuJoCoLiveTest.lua` uses a real-time accumulator with a 2 ms fixed simulation step and a bounded catch-up count. It creates a temporary pelvis proxy next to the player and continuously updates its ParaEngine position. The timer, proxy, and model handle are released when the world unloads.

Select either script through the existing launcher without changing the Aries main loop:

```powershell
D:/Paracraft_dev/start_mujoco_world.bat `
	"worlds/DesignHouse/_user/zhanglei/test_mujoco" `
	"script/apps/Aries/Creator/Game/Robot/MuJoCoLiveTest.lua" `
	"D:/Paracraft_dev/mujoco-models/h1_2_description/h1_2.xml"
```

The live proxy currently validates fixed stepping and position synchronization only. Full body mesh/bone mapping and scene-object quaternion application remain separate visualization work.

## Phase-one status

This milestone establishes the build, runtime ownership, fixed-step, state, control, coordinate conversion, and Lua foundations. The remaining phase-one work includes the canonical robot contract, H1/G1 asset mapping, ParaX pose driving, local static terrain mirroring, Bullet backend parity, observations and contact details, ONNX policy deployment, diagnostics, and complete acceptance tests.