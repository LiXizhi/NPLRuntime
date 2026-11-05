# MuJoCo 3.10.0 Integration

MuJoCo is built from `Server/trunk/mujoco-3.10.0`. The integration does not use an external MuJoCo checkout.

## Build requirements

- 64-bit target. MuJoCo 3.10.0 is disabled by default for 32-bit builds.
- MSVC with C++20 support for `NPLMuJoCoSimulation`.
- `NPLRUNTIME_MUJOCO=ON` (default for 64-bit builds).

The legacy ParaEngine targets remain on their existing C++ standard. Only the wrapper library that includes MuJoCo headers is compiled as C++20. The MuJoCo DLL is copied to the ParaWorld output directory after the client target is built.

The Client enables Assimp's STL importer for robot visual asset preparation. Embedded MuJoCo builds disable `tinyxml2_INSTALL_PKGCONFIG` because tinyxml2's pkg-config generation writes one output from every Visual Studio configuration and otherwise breaks multi-config CMake generation.

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
- `GetJointQPosAdr` / `GetJointDofAdr` for hinge joint `qpos` / `qvel` addresses
- `GetBodyPosition` and `GetBodyQuaternion` for raw MuJoCo world poses
- `GetBodyParaPosition` and `GetBodyParaQuaternion` for converted ParaEngine world poses
- `GetBodyLinearVelocity` / `GetBodyAngularVelocity` and `GetBodyParaLinearVelocity` / `GetBodyParaAngularVelocity` for COM twist (`cvel`)
- Contact details: `GetContactGeom*`, `GetContactBody*`, `GetContactDist`, `GetContactPosition` / `GetContactNormal`, `GetContactForce` (world), and matching `GetContactPara*` helpers
- Heightfield / geom: `FindHField`, `GetHFieldNRow/NCol/Size`, `Get/Set/FillHFieldElevation`, `FindGeom`, `Get/SetGeomPosition`, `SetGeomParaPosition`
- `RobotCoordinateConverter::ParaEnginePositionToMuJoCo` for sampling Paracraft terrain into MuJoCo

Array indices use MuJoCo's zero-based indexing. `GetLastError(0)` returns the most recent model loading error when `LoadModel` returns zero. Lua helpers: `MuJoCoObservation.lua`, `MuJoCoLocalTerrain.lua`. Local terrain model: `h1_2_paracraft.xml` (`local_terrain` 41×41 hfield).

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
cmake --build bin/client-x64-mujoco-boost187 --config Release --target ParaEngineClient ParaEngineClientApp --parallel 4
```

For testing in `D:/Paracraft_dev`, deploy `ParaEngineClient_d.exe`, `ParaEngineClient_d.dll`, `mujoco_d.dll`, `lua_d.dll`, `sqlite_d.dll`, and `libcurl_d.dll`. The verified client reached the normal Paracraft main loop without MuJoCo or core DLL loading errors.

Release deploys `ParaEngineClient.exe`, `ParaEngineClient.dll`, `mujoco.dll`, `lua.dll`, `sqlite.dll`, and `libcurl.dll`. Both verified executables are x64 (`PE Machine 0x8664`) and have run the H1_2 RL preset with MuJoCo loaded.

The `para-robot/paracraft-dev-cli/build-paracraft-client.js` helper wraps configuration, Debug/Release compilation, client shutdown, and deployment. From that repository use `npm run build:debug`, `npm run build:release`, or `npm run build:all`.

## Paracraft runtime tests

`MuJoCoQuickTest.lua` advances the selected model by 5000 steps immediately and logs model dimensions, contacts, and raw/converted pelvis poses. `MuJoCoLiveTest.lua` remains the 28-body handless static-FBX pose synchronization test. `MuJoCoParaXPrototype.lua` is now the primary complete-visual test: it loads one rigid-skinned ParaX character containing the body, both articulated hands, fingers, head shell, and front/rear logo geometry.

### Visual asset preparation

MuJoCo's H1_2 visual sources are STL files. Run the preparation script once after deploying a Client built with STL importer support:

```powershell
D:/Paracraft_dev/start_mujoco_world.bat `
	"worlds/DesignHouse/_user/zhanglei/test_mujoco" `
	"script/apps/Aries/Creator/Game/Robot/MuJoCoPrepareVisualAssets.lua" `
	"D:/Paracraft_dev/mujoco-models/h1_2_description/h1_2.xml"
```

The shared `MuJoCoH1VisualConfig.lua` map contains 28 body entries and 27 unique meshes because both wrist-yaw bodies use `wrist_yaw_link.STL`. The preparation script writes those 27 static FBX files under `h1_2_description/visual/`. `ParaAsset.ConvertGLB` remains backward compatible with its original seven-value TRS input and additionally accepts a 12-value 3x4 row-major matrix. Robot mesh preparation uses this matrix:

```text
0 -1 0 0
0  0 1 0
1  0 0 0
```

It applies the same `(x,y,z) -> (-y,z,x)` basis to mesh vertices that `RobotCoordinateConverter` applies to body positions. FBX is used instead of GLB at runtime because `LoadStaticMesh(.fbx)` creates a mesh object whose full quaternion can be updated; `CreateCharacter` does not preserve arbitrary rigid-body pitch and roll.

Select either script through the existing launcher without changing the Aries main loop:

```powershell
D:/Paracraft_dev/start_mujoco_world.bat `
	"worlds/DesignHouse/_user/zhanglei/test_mujoco" `
	"script/apps/Aries/Creator/Game/Robot/MuJoCoLiveTest.lua" `
	"D:/Paracraft_dev/mujoco-models/h1_2_description/h1_2.xml"
```

Pass `mujoco_live_simulate="true"` as an extra command-line argument to enable the 2 ms fixed-step simulation. Without a controller the H1_2 model will fall under gravity, so frozen mode is the default visualization check. `mujoco_visual_distance` changes the default three-meter player-relative placement.

The 28 real mesh objects validate handless body-object position and quaternion synchronization. A successful visibility log includes `visuals=28`, `fallbacks=0`, and a nonzero pelvis AABB. This path is retained as a lower-level diagnostic; it is no longer the target visual representation.

### Complete H1 ParaX asset

The reproducible Blender 4.2 LTS builder is maintained in the `para-robot` repository:

```text
unitree/tools/blender/build_h1_parax_prototype.py
```

Full mode reads the H1_2 URDF, creates one rigid bone per visual link, binds each link with weight `1.0`, embeds dark-body and unlit-white-logo materials, and exports one GLB. The current verified asset contains:

- 55 URDF visual links, including both hands, fingers, and `logo_link`;
- 74 mesh chunks and 130 imported ParaX nodes/bones;
- 788,274 triangles with no missing vertex weights;
- 75 render passes and 74 geosets;
- AABB size approximately `0.592 x 1.788 x 0.511` metres.

ParaX render passes use 16-bit local indices. Assimp expands the original GLB to more than two million vertices, and the old importer skipped faces whose local index span exceeded 65,535. The builder therefore pre-splits every link into deterministic chunks of at most 20,000 triangles, remapping each chunk to contiguous local vertices while retaining its original rigid bone. This restored all 788,274 triangles; the unchunked asset retained only 684,811.

The body and logo materials include embedded 2x2 diffuse textures because `ModelRenderPass::init_FX()` rejects passes without a loaded diffuse texture. Imported STL material slots are cleared, every polygon uses material slot zero, and the geometric front/rear logo is offset 2 mm away from the torso to prevent z-fighting at large world coordinates. The logo material name ends in `_u`, selecting ParaX unlit rendering.

GLB/glTF is right-handed while ParaEngine is left-handed. The ParaX Assimp path now applies `aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder` only for `.glb/.gltf`; the GLB asset itself remains standards-compliant instead of containing viewer-specific mirrored geometry. The latest build is deployed, but front/rear logo readability should receive one final visual confirmation in both Windows 3D Viewer and Paracraft.

Run the complete model test without changing `main_loop.lua`:

```powershell
Set-Location D:/Paracraft_dev
./ParaEngineClient_d.exe `
	'bootstrapper="script/apps/Aries/Creator/Game/Robot/MuJoCoBootstrapper.lua"' `
	'dev="D:/gits/paracraft/paracraft_script"' `
	'world="worlds/DesignHouse/_user/zhanglei/test_mujoco"' `
	'mujoco_test_script="script/apps/Aries/Creator/Game/Robot/MuJoCoParaXPrototype.lua"' `
	'mujoco_model="D:/Paracraft_dev/mujoco-models/h1_2_description/h1_2.xml"' `
	'mujoco_parax_model="D:/Paracraft_dev/mujoco-models/h1_2_description/parax/h1_2_full.glb"' `
	'mujoco_parax_test_pose="false"' `
	'mc="true"' 'isDevEnv="true"' 'isDevMode="true"' 'noclientupdate="true"' `
	'logfile="mujoco-world.log"'
```

The ParaX character must be created as a global visitor. Local objects are attached to the finite terrain quadtree, which does not cover worlds using coordinates near `(20000, 20000)`; such an object loads correctly but never reaches `PrepareRenderObject()`.

## Phase-one status

Lua-side phase one is largely complete: fixed-step driver, ParaX bone drive, control demo, observation/contact, local heightfield, `MuJoCoRobotSimulation`, and frozen **RobotContractManifest v1.0** (obsDim=178, actionDim=51, hash logged). C++ exposes matching `RobotTypes.h` / `IRobotSimulation.h`; a concrete `MujocoRobotSimulation` implementation, optional merged-box stairs, diagnostics, and ONNX policy loading remain next.