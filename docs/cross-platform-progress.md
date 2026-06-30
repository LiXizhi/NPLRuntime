# NPLRuntime Cross-Platform Porting Progress

## Background

This document tracks the merge of the **dev** branch into the **cp-old** branch of
[github.com/LiXizhi/NPLRuntime](https://github.com/LiXizhi/NPLRuntime).

### The Two Branches

| | dev branch | cp-old branch |
|-|------------|---------------|
| **Local path** | `C:\lxzsrc\NPLRuntime` | `C:\lxzsrc\NPLRuntimeCPOld` |
| **Engine source** | `Client/trunk/ParaEngineClient/` | `NPLRuntime/ParaEngine/` |
| **Renderer** | Windows DirectX 9 only | OpenGL + DirectX + Null (headless) |
| **Platforms** | Windows only | Windows, Linux, macOS, iOS, Android, Emscripten, HarmonyOS |
| **Build system** | Legacy Visual Studio solution | CMake 3.15+ (cross-platform) |
| **Status** | Active Windows development | Cross-platform port (cp = cross-platform) |

### Directory Correspondence

The `dev` branch kept the original Windows-centric layout under
`Client/trunk/ParaEngineClient/` while cp-old re-rooted that same tree to
`NPLRuntime/ParaEngine/` and reorganised platform-specific code:

| dev path | cp-old path | Notes |
|----------|-------------|-------|
| `Client/trunk/ParaEngineClient/` | `NPLRuntime/ParaEngine/` | Core engine (this doc tracks these) |
| `Client/trunk/externals/` | `NPLRuntime/externals/` | Vendored 3rd-party libs |
| `Client/trunk/PhysicsBT/` | `NPLRuntime/Plugins/PhysicsBT/` | Bullet physics plugin |
| `Client/trunk/ParaEngineClientApp/` | `NPLRuntime/ParaEngineClientApp/` | Thin EXE host |
| `common/d3dapp.*`, `common/d3dutil.*` | `NPLRuntime/Platform/Windows/` | Windows entry point |
| `renderer/OpenGL/GL*.{h,cpp}` | `NPLRuntime/ParaEngine/OpenGLWrapper/` | GL wrapper layer |
| `platform/win32/ParaEngineApp.*` | `NPLRuntime/Platform/Windows/src/ParaEngineApp.*` | Win32 bootstrap |
| `ic/` | `NPLRuntime/ParaEngine/InfoCenter/` | Info-centre DB |
| `shaders/` | `NPLRuntime/ParaEngine/shaders/` | Shader files |

### Merge Goal

The goal is to make **cp-old the single authoritative source** that builds on all
platforms.  Before merging we need to confirm that every meaningful feature/fix
committed to **dev** after the cp-old branch was forked has been ported.

Files in dev that are **not** present in cp-old, or that differ in content, are
tracked in the sections below so reviewers know what still needs attention.

### How to Read This Document

- **Section 1** – Files only in dev (missing from cp-old). Must decide: port, discard, or mark obsolete.
- **Section 2** – Files in both branches but with different content. The cp-old version may already incorporate cross-platform changes; some may need updates from dev.
- **Section 3** – Files byte-for-byte identical (fully ported / unchanged).
- **Section 4** – Files only in cp-old (new cross-platform additions not in dev).

> **Goal**: Merge dev branch (Windows/DirectX) into cp-old branch (cross-platform).
> Verify all code in dev has been ported to cp-old.
>
> - **dev**:   `Client/trunk/ParaEngineClient/`  (Windows DirectX, 1559 files)
> - **cp-old**: `NPLRuntime/ParaEngine/`          (Cross-platform, 1392 files)

## Summary

| Category | Count | Notes |
|----------|------:|-------|
| Files identical (fully ported) | 56 | Same content in both branches |
| Files different (partially ported / diverged) | 1191 | Exist in both but content differs |
| Files only in dev (NOT ported) | 183 | Need to be reviewed/ported to cp-old (18 Core/ interface files relocated to `Framework/Interface/`; 105 `Engine/Obsoleted/` and 6 `shaders/obsoleted/` skipped as obsolete) |
| Files only in cp-old (new platform code) | 145 | New cross-platform additions |
| **Total dev files** | **1559** | |
| **Total cp-old files** | **1392** | |

### Section 2 status breakdown (files present in both branches)

| Status | Count | Notes |
|--------|------:|-------|
| `identical-content` | 723 | Effectively identical (CRLF-only or whitespace diffs) |
| `identical` | 15 | Byte-for-byte identical (also listed in Section 3) |
| `minor diff` | 229 | <5% of lines changed |
| `moderate diff` | 112 | 5–20% of lines changed |
| `significant diff` | 99 | >20% of lines changed |
| `binary/non-code` | 32 | Non-text files (icons, .fxo, docs, etc.) |
| **Total** | **1210** | |

## Section 1: Files Only in Dev (Not Ported to cp-old)

These files exist in the **dev branch** but are **missing from cp-old**.
They need to be reviewed: some may be Windows-only features, some may need porting.

### `(root)/` (4 files, 3 code)

| File | Lines | Ext |
|------|------:|-----|
| `MSVCPCHSupport.cmake` | 27 | .cmake |
| `PEClientInstall.cmake` | 11 | .cmake |
| `ParaEngineClientConfig.win32.h` | 85 | .h |
| `ParaEngineRes.aps` | 141 | .aps |

### `2dengine/` (2 files, 2 code)

| File | Lines | Ext |
|------|------:|-----|
| `2dengine/GUIDirectInput.cpp` | 636 | .cpp |
| `2dengine/GUIDirectInput.h` | 122 | .h |

### `AutoRigger/` (2 files, 2 code)

| File | Lines | Ext |
|------|------:|-----|
| `AutoRigger/Matcher.cpp` | 649 | .cpp |
| `AutoRigger/Matcher.h` | 89 | .h |

### `Core/` (1 file truly missing; 18 relocated to `Framework/Interface/`)

> **Note:** 18 of the 19 files originally listed here as "dev-only" were actually
> **relocated** in cp-old from `Core/` to `NPLRuntime/ParaEngine/Framework/Interface/`.
> They are **not missing** — see Section 4 for their cp-old equivalents.
> Only one file has no equivalent anywhere in cp-old:

| File | Lines | Ext | Status |
|------|------:|-----|--------|
| `Core/ParaEngineAppImp.h` | 10 | .h | **truly missing** — no cp-old equivalent |

**Relocated files** (dev `Core/` → cp-old `Framework/Interface/`):

| dev file | cp-old file | Content |
|----------|-------------|---------|
| `Core/IBatchedElementDraw.h` | `Framework/Interface/IBatchedElementDraw.h` | identical |
| `Core/IEnvironmentSim.h` | `Framework/Interface/IEnvironmentSim.h` | identical |
| `Core/IEvent.h` | `Framework/Interface/IEvent.h` | differs (25→25 ln) |
| `Core/IInfoCenter.h` | `Framework/Interface/IInfoCenter.h` | identical |
| `Core/IMovieCodec.h` | `Framework/Interface/IMovieCodec.h` | differs (99→95 ln) |
| `Core/INPL.h` | `Framework/Interface/INPL.h` | differs (83→83 ln) |
| `Core/INPLAcitvationFile.h` | `Framework/Interface/INPLAcitvationFile.h` | identical |
| `Core/INPLRuntime.h` | `Framework/Interface/INPLRuntime.h` | differs (398→397 ln) |
| `Core/INPLRuntimeState.h` | `Framework/Interface/INPLRuntimeState.h` | identical |
| `Core/INPLScriptingState.h` | `Framework/Interface/INPLScriptingState.h` | identical |
| `Core/IParaAudioEngine.h` | `Framework/Interface/IParaAudioEngine.h` | differs (570→570 ln) |
| `Core/IParaDebugDraw.h` | `Framework/Interface/IParaDebugDraw.h` | identical |
| `Core/IParaEngineApp.h` | `Framework/Interface/IParaEngineApp.h` | differs (486→533 ln) |
| `Core/IParaEngineCore.h` | `Framework/Interface/IParaEngineCore.h` | differs (39→43 ln) |
| `Core/IParaPhysics.h` | `Framework/Interface/IParaPhysics.h` | identical |
| `Core/IParaWebPlayer.h` | `Framework/Interface/IParaWebPlayer.h` | identical |
| `Core/IParaWebService.h` | `Framework/Interface/IParaWebService.h` | identical |
| `Core/baseinterface.h` | `Framework/Interface/BaseInterface.h` | differs (60→61 ln) |

### `Engine/` (9 files, 9 code)

> **Note:** 105 files under `Engine/Obsoleted/` were removed — they are obsolete Windows-only code not needed in cp-old.

| File | Lines | Ext |
|------|------:|-----|
| `Engine/D3DWindowDefault.cpp` | 354 | .cpp |
| `Engine/D3DWindowDefault.h` | 70 | .h |
| `Engine/EditorHelper.cpp` | 364 | .cpp |
| `Engine/EditorHelper.h` | 82 | .h |
| `Engine/OSWindows.cpp` | 332 | .cpp |
| `Engine/OSWindows.h` | 61 | .h |
| `Engine/ParaEngineApp.cpp` | 3426 | .cpp |
| `Engine/ParaEngineApp.h` | 596 | .h |
| `Engine/ParaWorld.cpp` | 263 | .cpp |

### `IO/` (4 files, 4 code)

| File | Lines | Ext |
|------|------:|-----|
| `IO/BlueTooth.cpp` | 124 | .cpp |
| `IO/BlueTooth.h` | 31 | .h |
| `IO/ReadFile.cpp` | 142 | .cpp |
| `IO/ReadFile.h` | 47 | .h |

### `OpenGLTest/` (1 files, 1 code)

| File | Lines | Ext |
|------|------:|-----|
| `OpenGLTest/ParaWorld.cpp` | 122 | .cpp |

### `ParaScriptBindings/` (2 files, 2 code)

| File | Lines | Ext |
|------|------:|-----|
| `ParaScriptBindings/local_bridge_manual.cpp` | 54 | .cpp |
| `ParaScriptBindings/local_bridge_manual.h` | 18 | .h |

### `common/` (16 files, 16 code)

| File | Lines | Ext |
|------|------:|-----|
| `common/D3DWindowUtil.cpp` | 258 | .cpp |
| `common/D3DWindowUtil.h` | 45 | .h |
| `common/d3dapp.cpp` | 1856 | .cpp |
| `common/d3dapp.h` | 182 | .h |
| `common/d3denumeration.cpp` | 742 | .cpp |
| `common/d3denumeration.h` | 137 | .h |
| `common/d3dfile.cpp` | 590 | .cpp |
| `common/d3dfile.h` | 73 | .h |
| `common/d3dfont.cpp` | 883 | .cpp |
| `common/d3dfont.h` | 82 | .h |
| `common/d3dres.h` | 43 | .h |
| `common/d3dsettings.cpp` | 970 | .cpp |
| `common/d3dsettings.h` | 126 | .h |
| `common/dxstdafx.h` | 73 | .h |
| `common/dxutil.cpp` | 1359 | .cpp |
| `common/dxutil.h` | 181 | .h |

### `ic/` (9 files, 9 code)

| File | Lines | Ext |
|------|------:|-----|
| `ic/ICConfigManager.cpp` | 745 | .cpp |
| `ic/ICConfigManager.h` | 233 | .h |
| `ic/ICDBManager.cpp` | 955 | .cpp |
| `ic/ICDBManager.h` | 334 | .h |
| `ic/ICRecordSet.cpp` | 509 | .cpp |
| `ic/ICRecordSet.h` | 126 | .h |
| `ic/ParaDatabase.h` | 18 | .h |
| `ic/SQLStatement.cpp` | 188 | .cpp |
| `ic/SQLStatement.h` | 89 | .h |

### `platform/` (26 files, 24 code)

| File | Lines | Ext |
|------|------:|-----|
| `platform/PlatformMacro.h` | 22 | .h |
| `platform/ios/LocalBridgeHandler.mm` | 77 | .mm |
| `platform/ios/LocalBridgeHandler_ios.h` | 12 | .h |
| `platform/ios/LocalBridgePB.pb.cc` | 17241 | .cc |
| `platform/ios/LocalBridgePB.pb.h` | 11156 | .h |
| `platform/ios/LocalBridge_ios.cpp` | 24 | .cpp |
| `platform/ios/LocalBridge_ios.h` | 8 | .h |
| `platform/mac/AppDelegate.cpp` | 243 | .cpp |
| `platform/mac/AppDelegate.h` | 76 | .h |
| `platform/mac/ParaEngineApp.cpp` | 769 | .cpp |
| `platform/mac/ParaEngineApp.h` | 388 | .h |
| `platform/mac/ParaGLMac.h` | 24 | .h |
| `platform/mac/ParaSimpleAudioEngine.cpp` | 308 | .cpp |
| `platform/mac/ParaSimpleAudioEngine.h` | 87 | .h |
| `platform/mac/ParaSimpleAudioSource.cpp` | 401 | .cpp |
| `platform/mac/ParaSimpleAudioSource.h` | 157 | .h |
| `platform/mac/SimpleAudioEngine.cpp` | 80 | .cpp |
| `platform/mac/SimpleAudioEngine.h` | 32 | .h |
| `platform/mac/fsmacosxwrapper.m` | 1274 | .m |
| `platform/mac/fsmacosxwrappercpp.cpp` | 95 | .cpp |
| `platform/mac/fssimplewindow.h` | 181 | .h |
| `platform/mac/mac_main.cpp` | 55 | .cpp |
| `platform/win32/ParaEngineApp.cpp` | 1683 | .cpp |
| `platform/win32/ParaEngineApp.h` | 465 | .h |
| `platform/win32/ParaEngineGLView.cpp` | 1032 | .cpp |
| `platform/win32/ParaEngineGLView.h` | 199 | .h |

### `renderer/` (28 files, 28 code)

| File | Lines | Ext |
|------|------:|-----|
| `renderer/OpenGL/GLFont.h` | 19 | .h |
| `renderer/OpenGL/GLFontAtlas.cpp` | 449 | .cpp |
| `renderer/OpenGL/GLFontAtlas.h` | 102 | .h |
| `renderer/OpenGL/GLFontAtlasCache.cpp` | 125 | .cpp |
| `renderer/OpenGL/GLFontAtlasCache.h` | 26 | .h |
| `renderer/OpenGL/GLFontFreeType.cpp` | 655 | .cpp |
| `renderer/OpenGL/GLFontFreeType.h` | 81 | .h |
| `renderer/OpenGL/GLImage.cpp` | 837 | .cpp |
| `renderer/OpenGL/GLImage.h` | 115 | .h |
| `renderer/OpenGL/GLLabel.cpp` | 722 | .cpp |
| `renderer/OpenGL/GLLabel.h` | 213 | .h |
| `renderer/OpenGL/GLProgram.cpp` | 757 | .cpp |
| `renderer/OpenGL/GLProgram.h` | 311 | .h |
| `renderer/OpenGL/GLProgramCache.cpp` | 374 | .cpp |
| `renderer/OpenGL/GLProgramCache.h` | 71 | .h |
| `renderer/OpenGL/GLTexture2D.cpp` | 1037 | .cpp |
| `renderer/OpenGL/GLTexture2D.h` | 214 | .h |
| `renderer/OpenGL/GLType.cpp` | 550 | .cpp |
| `renderer/OpenGL/GLType.h` | 339 | .h |
| `renderer/OpenGL/OpenGLWrapper.h` | 79 | .h |
| `renderer/OpenGL/edtaa3func.cpp` | 521 | .cpp |
| `renderer/OpenGL/edtaa3func.h` | 94 | .h |
| `renderer/OpenGL/s3tc.cpp` | 182 | .cpp |
| `renderer/OpenGL/s3tc.h` | 49 | .h |
| `renderer/OpenGL/uthash.h` | 940 | .h |
| `renderer/RenderDeviceOpenGL.cpp` | 719 | .cpp |
| `renderer/RenderDeviceOpenGL.h` | 83 | .h |
| `renderer/VirtualKeyDef.h` | 359 | .h |

### `res/` (1 files, 0 code)

| File | Lines | Ext |
|------|------:|-----|
| `res/haqi.ico` | 183 | .ico |

### `shaders/` (78 files, 46 code)

> **Note:** 6 files under `shaders/obsoleted/` were removed — obsolete shaders not needed in cp-old.

| File | Lines | Ext |
|------|------:|-----|
| `shaders/BMaxModel.fx` | 231 | .fx |
| `shaders/BMaxModel.fxo` | 37 | .fxo |
| `shaders/GUI_simple.fx` | 64 | .fx |
| `shaders/GUI_simple.fxo` | 2 | .fxo |
| `shaders/blockEffect.fx` | 325 | .fx |
| `shaders/blockEffect.fxo` | 37 | .fxo |
| `shaders/common_shader_data.h` | 37 | .h |
| `shaders/dropShadow.fx` | 51 | .fx |
| `shaders/dropShadow.fxo` | 13 | .fxo |
| `shaders/fullscreen_glow.fx` | 423 | .fx |
| `shaders/fullscreen_glow.fxo` | 45 | .fxo |
| `shaders/fx/BMaxModel.fx` | 108 | .fx |
| `shaders/fx/blockEffect.fx` | 211 | .fx |
| `shaders/fx/simple_particle.fx` | 85 | .fx |
| `shaders/fx/sky.fx` | 144 | .fx |
| `shaders/glsl/guiEffect.fx.glsl` | 67 | .glsl |
| `shaders/glsl/guiTextEffect.fx.glsl` | 62 | .glsl |
| `shaders/glsl/simple_mesh_normal.fx.glsl` | 101 | .glsl |
| `shaders/glsl/singleColorEffect.fx.glsl` | 51 | .glsl |
| `shaders/glsl/skydome.fx.glsl` | 213 | .glsl |
| `shaders/glsl/terrain_normal.fx.glsl` | 130 | .glsl |
| `shaders/light_scattering_constants.h` | 124 | .h |
| `shaders/occlusion_test.fx` | 69 | .fx |
| `shaders/occlusion_test.fxo` | 3 | .fxo |
| `shaders/ocean_water.fx` | 360 | .fx |
| `shaders/ocean_water.fxo` | 32 | .fxo |
| `shaders/ocean_water_cloud.fx` | 171 | .fx |
| `shaders/ocean_water_cloud.fxo` | 21 | .fxo |
| `shaders/ocean_water_high.fx` | 288 | .fx |
| `shaders/ocean_water_high.fxo` | 50 | .fxo |
| `shaders/ocean_water_quad.fx` | 98 | .fx |
| `shaders/ocean_water_quad.fxo` | 11 | .fxo |
| `shaders/patch_deform.h` | 97 | .h |
| `shaders/screenWave.fx` | 82 | .fx |
| `shaders/screenWave.fxo` | 6 | .fxo |
| `shaders/shadowmap_blur.fx` | 152 | .fx |
| `shaders/shadowmap_blur.fxo` | 8 | .fxo |
| `shaders/simple_VS.fx` | 24 | .fx |
| `shaders/simple_VS.fxo` | 1 | .fxo |
| `shaders/simple_cad_model.fx` | 105 | .fx |
| `shaders/simple_cad_model.fxo` | 7 | .fxo |
| `shaders/simple_mesh.fx` | 149 | .fx |
| `shaders/simple_mesh.fxo` | 8 | .fxo |
| `shaders/simple_mesh_normal.fx` | 273 | .fx |
| `shaders/simple_mesh_normal.fxo` | 30 | .fxo |
| `shaders/simple_mesh_normal_border.fx` | 87 | .fx |
| `shaders/simple_mesh_normal_border.fxo` | 4 | .fxo |
| `shaders/simple_mesh_normal_instanced.fx` | 211 | .fx |
| `shaders/simple_mesh_normal_instanced.fxo` | 20 | .fxo |
| `shaders/simple_mesh_normal_low.fx` | 222 | .fx |
| `shaders/simple_mesh_normal_low.fxo` | 19 | .fxo |
| `shaders/simple_mesh_normal_selected.fx` | 176 | .fx |
| `shaders/simple_mesh_normal_selected.fxo` | 14 | .fxo |
| `shaders/simple_mesh_normal_shadow.fx` | 230 | .fx |
| `shaders/simple_mesh_normal_shadow.fxo` | 21 | .fxo |
| `shaders/simple_mesh_normal_specialeffects.fx` | 170 | .fx |
| `shaders/simple_mesh_normal_specialeffects.fxo` | 13 | .fxo |
| `shaders/simple_mesh_normal_tex2.fx` | 247 | .fx |
| `shaders/simple_mesh_normal_tex2.fxo` | 16 | .fxo |
| `shaders/simple_mesh_normal_unlit.fx` | 167 | .fx |
| `shaders/simple_mesh_normal_unlit.fxo` | 9 | .fxo |
| `shaders/simple_mesh_normal_vegetation.fx` | 247 | .fx |
| `shaders/simple_mesh_normal_vegetation.fxo` | 33 | .fxo |
| `shaders/simple_mesh_normal_vegetation.h` | 18 | .h |
| `shaders/simple_particle.fx` | 85 | .fx |
| `shaders/simple_particle.fxo` | 4 | .fxo |
| `shaders/sky.fx` | 144 | .fx |
| `shaders/sky.fxo` | 13 | .fxo |
| `shaders/skydome.fx` | 201 | .fx |
| `shaders/skydome.fxo` | 24 | .fxo |
| `shaders/smesh_ctor.fx` | 165 | .fx |
| `shaders/smesh_ctor.fxo` | 12 | .fxo |
| `shaders/terrain_normal.fx` | 452 | .fx |
| `shaders/terrain_normal.fxo` | 70 | .fxo |
| `shaders/terrain_normal_depth_shadowmap.fx` | 578 | .fx |
| `shaders/terrain_normal_depth_shadowmap.fxo` | 63 | .fxo |
| `shaders/water_ripples.fx` | 86 | .fx |
| `shaders/water_ripples.fxo` | 8 | .fxo |

## Section 2: Files Different Between Branches

These files exist in **both** branches but have different content.
The cp-old version may have been modified for cross-platform support.

### `(root)/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `CMakeLists.txt` | 544 | 416 | - | - | binary/non-code |
| `ParaEngineRes.rc` | 251 | 251 | - | - | binary/non-code |
| `ResourceEmbedded.cpp` | 80 | 80 | +11 | -11 | significant diff |

### `2dengine/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `2dengine/CMakeLists.txt` | 7 | 7 | - | - | binary/non-code |
| `2dengine/EventBinding.cpp` | 1159 | 1016 | +120 | -263 | significant diff |
| `2dengine/EventBinding.h` | 153 | 145 | +4 | -12 | moderate diff |
| `2dengine/FontRendererOpenGL.cpp` | 315 | 410 | +122 | -27 | significant diff |
| `2dengine/FontRendererOpenGL.h` | 39 | 41 | +6 | -4 | significant diff |
| `2dengine/GUIAttributeGrid.cpp` | 151 | 151 | +1 | -1 | minor diff |
| `2dengine/GUIAttributeGrid.h` | 29 | 29 | +0 | -0 | identical-content |
| `2dengine/GUIBase.cpp` | 2936 | 2936 | +30 | -30 | minor diff |
| `2dengine/GUIBase.h` | 1222 | 1225 | +221 | -218 | significant diff |
| `2dengine/GUIButton.cpp` | 788 | 772 | +277 | -293 | significant diff |
| `2dengine/GUIButton.h` | 154 | 151 | +0 | -3 | minor diff |
| `2dengine/GUIContainer.cpp` | 1735 | 1734 | +6 | -7 | minor diff |
| `2dengine/GUIContainer.h` | 354 | 354 | +0 | -0 | identical-content |
| `2dengine/GUIEdit.cpp` | 1961 | 2001 | +194 | -154 | moderate diff |
| `2dengine/GUIEdit.h` | 246 | 296 | +65 | -15 | significant diff |
| `2dengine/GUIEvent.cpp` | 877 | 883 | +18 | -12 | minor diff |
| `2dengine/GUIEvent.h` | 231 | 231 | +6 | -6 | moderate diff |
| `2dengine/GUIHighlight.cpp` | 523 | 523 | +1 | -1 | minor diff |
| `2dengine/GUIHighlight.h` | 165 | 165 | +0 | -0 | identical-content |
| `2dengine/GUIIME.cpp` | 1837 | 1836 | +8 | -9 | minor diff |
| `2dengine/GUIIME.h` | 381 | 381 | +1 | -1 | minor diff |
| `2dengine/GUIIMEDelegate.cpp` | 136 | 257 | +251 | -130 | significant diff |
| `2dengine/GUIIMEDelegate.h` | 67 | 163 | +119 | -23 | significant diff |
| `2dengine/GUIIMEEditBox.cpp` | 1000 | 1037 | +41 | -4 | minor diff |
| `2dengine/GUIIMEEditBox.h` | 116 | 122 | +7 | -1 | moderate diff |
| `2dengine/GUIKeyboardVirtual.cpp` | 177 | 153 | +59 | -83 | significant diff |
| `2dengine/GUIKeyboardVirtual.h` | 73 | 63 | +21 | -31 | significant diff |
| `2dengine/GUIListBox.cpp` | 1217 | 1216 | +3 | -4 | minor diff |
| `2dengine/GUIListBox.h` | 177 | 177 | +0 | -0 | identical-content |
| `2dengine/GUIMouseVirtual.cpp` | 357 | 301 | +167 | -223 | significant diff |
| `2dengine/GUIMouseVirtual.h` | 119 | 96 | +21 | -44 | significant diff |
| `2dengine/GUIPosition.cpp` | 403 | 403 | +0 | -0 | identical-content |
| `2dengine/GUIPosition.h` | 190 | 190 | +0 | -0 | identical-content |
| `2dengine/GUIResource.cpp` | 900 | 900 | +0 | -0 | identical-content |
| `2dengine/GUIResource.h` | 318 | 318 | +0 | -0 | identical-content |
| `2dengine/GUIRoot.cpp` | 3313 | 2924 | +271 | -660 | significant diff |
| `2dengine/GUIRoot.h` | 684 | 718 | +62 | -28 | moderate diff |
| `2dengine/GUIScript.cpp` | 15 | 15 | +0 | -0 | identical-content |
| `2dengine/GUIScript.h` | 9 | 9 | +0 | -0 | identical-content |
| `2dengine/GUIScrollBar.cpp` | 942 | 942 | +3 | -3 | minor diff |
| `2dengine/GUIScrollBar.h` | 156 | 156 | +0 | -0 | identical-content |
| `2dengine/GUISlider.cpp` | 605 | 604 | +3 | -4 | minor diff |
| `2dengine/GUISlider.h` | 58 | 58 | +0 | -0 | identical-content |
| `2dengine/GUIState.cpp` | 56 | 56 | +4 | -4 | moderate diff |
| `2dengine/GUIState.h` | 64 | 64 | +1 | -1 | minor diff |
| `2dengine/GUIText.cpp` | 505 | 516 | +12 | -1 | minor diff |
| `2dengine/GUIText.h` | 126 | 126 | +0 | -0 | identical-content |
| `2dengine/GUIToolTip.cpp` | 584 | 583 | +1 | -2 | minor diff |
| `2dengine/GUIToolTip.h` | 122 | 122 | +0 | -0 | identical-content |
| `2dengine/GUIUniBuffer.cpp` | 946 | 165 | +8 | -789 | significant diff |
| `2dengine/GUIUniBuffer.h` | 221 | 63 | +19 | -177 | significant diff |
| `2dengine/GUIWebBrowser.cpp` | 707 | 707 | +1 | -1 | minor diff |
| `2dengine/GUIWebBrowser.h` | 179 | 179 | +0 | -0 | identical-content |
| `2dengine/GrowableArray.h` | 172 | 172 | +0 | -0 | identical-content |
| `2dengine/TextureParams.cpp` | 172 | 172 | +7 | -7 | moderate diff |
| `2dengine/TextureParams.h` | 60 | 60 | +0 | -0 | identical-content |
| `2dengine/TouchGestureBase.h` | 16 | 16 | +0 | -0 | identical-content |
| `2dengine/TouchGesturePinch.cpp` | 101 | 101 | +4 | -4 | moderate diff |
| `2dengine/TouchGesturePinch.h` | 47 | 47 | +0 | -0 | identical-content |
| `2dengine/TouchSessions.cpp` | 160 | 160 | +0 | -0 | identical-content |
| `2dengine/TouchSessions.h` | 53 | 53 | +0 | -0 | identical-content |

### `3dengine/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `3dengine/AIBase.cpp` | 40 | 40 | +0 | -0 | identical-content |
| `3dengine/AIBase.h` | 41 | 41 | +0 | -0 | identical-content |
| `3dengine/AIModuleNPC.cpp` | 152 | 152 | +0 | -0 | identical-content |
| `3dengine/AIModuleNPC.h` | 65 | 65 | +0 | -0 | identical-content |
| `3dengine/AITasks.cpp` | 134 | 134 | +0 | -0 | identical-content |
| `3dengine/AITasks.h` | 135 | 135 | +0 | -0 | identical-content |
| `3dengine/AnimInstanceBase.cpp` | 219 | 219 | +16 | -16 | moderate diff |
| `3dengine/AnimInstanceBase.h` | 275 | 275 | +89 | -89 | significant diff |
| `3dengine/AttributeProvider.cpp` | 396 | 396 | +2 | -2 | minor diff |
| `3dengine/AttributeProvider.h` | 189 | 189 | +1 | -1 | minor diff |
| `3dengine/AudioEngine2.cpp` | 1243 | 1243 | +9 | -9 | minor diff |
| `3dengine/AudioEngine2.h` | 509 | 509 | +88 | -88 | significant diff |
| `3dengine/AutoCamera.cpp` | 2648 | 2648 | +1 | -1 | minor diff |
| `3dengine/AutoCamera.h` | 655 | 656 | +4 | -3 | minor diff |
| `3dengine/BaseCamera.cpp` | 840 | 840 | +0 | -0 | identical-content |
| `3dengine/BaseCamera.h` | 402 | 402 | +0 | -0 | identical-content |
| `3dengine/BaseObject.cpp` | 1329 | 1329 | +0 | -0 | identical-content |
| `3dengine/BaseObject.h` | 1360 | 1360 | +0 | -0 | identical-content |
| `3dengine/BatchedElementDraw.cpp` | 516 | 526 | +54 | -44 | moderate diff |
| `3dengine/BatchedElementDraw.h` | 130 | 129 | +0 | -1 | minor diff |
| `3dengine/BillBoardViewInfo.cpp` | 45 | 45 | +0 | -0 | identical-content |
| `3dengine/BillBoardViewInfo.h` | 49 | 49 | +0 | -0 | identical-content |
| `3dengine/BipedController.cpp` | 205 | 205 | +0 | -0 | identical-content |
| `3dengine/BipedController.h` | 103 | 103 | +0 | -0 | identical-content |
| `3dengine/BipedObject.cpp` | 5794 | 5794 | +0 | -0 | identical-content |
| `3dengine/BipedObject.h` | 1033 | 1033 | +0 | -0 | identical-content |
| `3dengine/BipedStateManager.cpp` | 1024 | 1024 | +1 | -1 | minor diff |
| `3dengine/BipedStateManager.h` | 213 | 214 | +3 | -2 | minor diff |
| `3dengine/BipedWayPoint.cpp` | 47 | 47 | +0 | -0 | identical-content |
| `3dengine/BipedWayPoint.h` | 72 | 72 | +0 | -0 | identical-content |
| `3dengine/BlockDynamicObject.cpp` | 264 | 264 | +0 | -0 | identical-content |
| `3dengine/BlockDynamicObject.h` | 97 | 97 | +0 | -0 | identical-content |
| `3dengine/BlockPieceParticle.cpp` | 192 | 192 | +4 | -4 | minor diff |
| `3dengine/BlockPieceParticle.h` | 69 | 69 | +0 | -0 | identical-content |
| `3dengine/BufferPicking.cpp` | 421 | 448 | +50 | -23 | moderate diff |
| `3dengine/BufferPicking.h` | 154 | 154 | +1 | -1 | minor diff |
| `3dengine/CameraFrustum.cpp` | 122 | 122 | +0 | -0 | identical-content |
| `3dengine/CameraFrustum.h` | 45 | 45 | +0 | -0 | identical-content |
| `3dengine/CanvasCamera.cpp` | 150 | 150 | +0 | -0 | identical-content |
| `3dengine/CanvasCamera.h` | 84 | 84 | +0 | -0 | identical-content |
| `3dengine/CharacterDB.cpp` | 426 | 426 | +2 | -2 | minor diff |
| `3dengine/CharacterDB.h` | 108 | 108 | +1 | -1 | minor diff |
| `3dengine/ContainerObject.cpp` | 72 | 75 | +5 | -2 | moderate diff |
| `3dengine/ContainerObject.h` | 27 | 27 | +0 | -0 | identical-content |
| `3dengine/CustomCharCommon.cpp` | 200 | 203 | +7 | -4 | moderate diff |
| `3dengine/CustomCharCommon.h` | 304 | 304 | +2 | -2 | minor diff |
| `3dengine/CustomCharFace.cpp` | 260 | 260 | +0 | -0 | identical-content |
| `3dengine/CustomCharFace.h` | 114 | 114 | +1 | -1 | minor diff |
| `3dengine/CustomCharModelInstance.cpp` | 1937 | 1937 | +0 | -0 | identical-content |
| `3dengine/CustomCharModelInstance.h` | 411 | 411 | +4 | -4 | minor diff |
| `3dengine/CustomCharRenderTarget.cpp` | 245 | 245 | +1 | -1 | minor diff |
| `3dengine/CustomCharRenderTarget.h` | 37 | 37 | +0 | -0 | identical-content |
| `3dengine/CustomCharSettings.cpp` | 352 | 352 | +0 | -0 | identical-content |
| `3dengine/CustomCharSettings.h` | 57 | 57 | +0 | -0 | identical-content |
| `3dengine/DummyAnimInstance.cpp` | 67 | 67 | +0 | -0 | identical-content |
| `3dengine/DummyAnimInstance.h` | 27 | 27 | +0 | -0 | identical-content |
| `3dengine/EffectFileHandles.h` | 88 | 88 | +0 | -0 | identical-content |
| `3dengine/EffectHelper.cpp` | 40 | 40 | +6 | -6 | significant diff |
| `3dengine/EffectHelper.h` | 19 | 19 | +0 | -0 | identical-content |
| `3dengine/EnvironmentSim.cpp` | 579 | 579 | +150 | -150 | significant diff |
| `3dengine/EnvironmentSim.h` | 99 | 99 | +0 | -0 | identical-content |
| `3dengine/FaceTrackingCtrler.cpp` | 89 | 89 | +0 | -0 | identical-content |
| `3dengine/FaceTrackingCtrler.h` | 42 | 42 | +0 | -0 | identical-content |
| `3dengine/Fog.cpp` | 68 | 68 | +0 | -0 | identical-content |
| `3dengine/Fog.h` | 32 | 32 | +0 | -0 | identical-content |
| `3dengine/IGameObject.cpp` | 586 | 586 | +4 | -4 | minor diff |
| `3dengine/IGameObject.h` | 367 | 367 | +0 | -0 | identical-content |
| `3dengine/IHeadOn3D.cpp` | 679 | 680 | +14 | -13 | minor diff |
| `3dengine/IHeadOn3D.h` | 235 | 235 | +0 | -0 | identical-content |
| `3dengine/IRefObject.cpp` | 178 | 178 | +0 | -0 | identical-content |
| `3dengine/IRefObject.h` | 160 | 160 | +0 | -0 | identical-content |
| `3dengine/IScene.h` | 82 | 82 | +0 | -0 | identical-content |
| `3dengine/IViewClippingObject.cpp` | 786 | 788 | +11 | -9 | minor diff |
| `3dengine/IViewClippingObject.h` | 276 | 276 | +0 | -0 | identical-content |
| `3dengine/KeyFrame.cpp` | 114 | 114 | +0 | -0 | identical-content |
| `3dengine/KeyFrame.h` | 328 | 328 | +0 | -0 | identical-content |
| `3dengine/LightGeoUtil.cpp` | 115 | 115 | +0 | -0 | identical-content |
| `3dengine/LightGeoUtil.h` | 30 | 30 | +0 | -0 | identical-content |
| `3dengine/LightManager.cpp` | 166 | 166 | +2 | -2 | minor diff |
| `3dengine/LightManager.h` | 72 | 72 | +0 | -0 | identical-content |
| `3dengine/LightObject.cpp` | 655 | 658 | +11 | -8 | minor diff |
| `3dengine/LightObject.h` | 231 | 230 | +12 | -13 | moderate diff |
| `3dengine/LightParam.cpp` | 165 | 165 | +4 | -4 | minor diff |
| `3dengine/LightParam.h` | 66 | 66 | +0 | -0 | identical-content |
| `3dengine/MCIController.cpp` | 52 | 52 | +0 | -0 | identical-content |
| `3dengine/MCIController.h` | 49 | 49 | +0 | -0 | identical-content |
| `3dengine/ManagedDef.h` | 24 | 24 | +0 | -0 | identical-content |
| `3dengine/ManagedLoader.cpp` | 93 | 93 | +0 | -0 | identical-content |
| `3dengine/ManagedLoader.h` | 84 | 84 | +0 | -0 | identical-content |
| `3dengine/MeshEntity.cpp` | 290 | 309 | +19 | -0 | moderate diff |
| `3dengine/MeshEntity.h` | 171 | 175 | +7 | -3 | moderate diff |
| `3dengine/MeshObject.cpp` | 1354 | 1355 | +22 | -21 | minor diff |
| `3dengine/MeshObject.h` | 344 | 345 | +1 | -0 | minor diff |
| `3dengine/MeshPhysicsObject.cpp` | 914 | 914 | +182 | -182 | significant diff |
| `3dengine/MeshPhysicsObject.h` | 336 | 336 | +7 | -7 | minor diff |
| `3dengine/MiniSceneGraph.cpp` | 1520 | 1507 | +24 | -37 | minor diff |
| `3dengine/MiniSceneGraph.h` | 432 | 432 | +0 | -0 | identical-content |
| `3dengine/MissileObject.cpp` | 279 | 279 | +0 | -0 | identical-content |
| `3dengine/MissileObject.h` | 92 | 92 | +0 | -0 | identical-content |
| `3dengine/MovieCtrlers.cpp` | 554 | 554 | +0 | -0 | identical-content |
| `3dengine/MovieCtrlers.h` | 140 | 140 | +0 | -0 | identical-content |
| `3dengine/MoviePlatform.cpp` | 1403 | 1437 | +180 | -146 | significant diff |
| `3dengine/MoviePlatform.h` | 365 | 366 | +4 | -3 | minor diff |
| `3dengine/ObjectAutoReleasePool.cpp` | 86 | 86 | +0 | -0 | identical-content |
| `3dengine/ObjectAutoReleasePool.h` | 57 | 57 | +0 | -0 | identical-content |
| `3dengine/ObjectEvent.cpp` | 60 | 60 | +0 | -0 | identical-content |
| `3dengine/ObjectEvent.h` | 25 | 25 | +0 | -0 | identical-content |
| `3dengine/OceanManager.cpp` | 2880 | 2880 | +63 | -63 | minor diff |
| `3dengine/OceanManager.h` | 622 | 622 | +0 | -0 | identical-content |
| `3dengine/OverlayObject.cpp` | 113 | 113 | +0 | -0 | identical-content |
| `3dengine/OverlayObject.h` | 56 | 56 | +0 | -0 | identical-content |
| `3dengine/ParaEngineSettings.cpp` | 1795 | 1635 | +157 | -317 | significant diff |
| `3dengine/ParaEngineSettings.h` | 792 | 810 | +778 | -760 | significant diff |
| `3dengine/ParaMeshXMLFile.cpp` | 350 | 350 | +142 | -142 | significant diff |
| `3dengine/ParaMeshXMLFile.h` | 136 | 136 | +0 | -0 | identical-content |
| `3dengine/ParaPhysics.cpp` | 92 | 92 | +0 | -0 | identical-content |
| `3dengine/ParaPhysics.h` | 47 | 47 | +0 | -0 | identical-content |
| `3dengine/ParaXAnimInstance.cpp` | 1103 | 1103 | +20 | -20 | minor diff |
| `3dengine/ParaXAnimInstance.h` | 245 | 245 | +4 | -4 | minor diff |
| `3dengine/ParaXEntity.cpp` | 350 | 350 | +0 | -0 | identical-content |
| `3dengine/ParaXEntity.h` | 162 | 162 | +0 | -0 | identical-content |
| `3dengine/ParaXMaterial.h` | 135 | 135 | +0 | -0 | identical-content |
| `3dengine/ParaXModelCanvas.cpp` | 567 | 559 | +1 | -9 | minor diff |
| `3dengine/ParaXModelCanvas.h` | 241 | 241 | +0 | -0 | identical-content |
| `3dengine/ParaXRefObject.h` | 43 | 43 | +0 | -0 | identical-content |
| `3dengine/ParaXSerializer.cpp` | 1962 | 1962 | +0 | -0 | identical-content |
| `3dengine/ParaXSerializer.h` | 297 | 297 | +0 | -0 | identical-content |
| `3dengine/ParaXStaticBase.cpp` | 158 | 158 | +3 | -3 | minor diff |
| `3dengine/ParaXStaticBase.h` | 199 | 201 | +8 | -6 | moderate diff |
| `3dengine/PhysicsWorld.cpp` | 1920 | 1920 | +0 | -0 | identical-content |
| `3dengine/PhysicsWorld.h` | 401 | 401 | +0 | -0 | identical-content |
| `3dengine/PortalFrustum.cpp` | 536 | 536 | +0 | -0 | identical-content |
| `3dengine/PortalFrustum.h` | 137 | 137 | +0 | -0 | identical-content |
| `3dengine/PortalNode.cpp` | 590 | 593 | +14 | -11 | minor diff |
| `3dengine/PortalNode.h` | 164 | 164 | +0 | -0 | identical-content |
| `3dengine/PredefinedEvents.cpp` | 37 | 37 | +0 | -0 | identical-content |
| `3dengine/PredefinedEvents.h` | 24 | 24 | +0 | -0 | identical-content |
| `3dengine/RayCollider.cpp` | 306 | 306 | +0 | -0 | identical-content |
| `3dengine/RayCollider.h` | 164 | 164 | +0 | -0 | identical-content |
| `3dengine/RenderTarget.cpp` | 801 | 830 | +136 | -107 | significant diff |
| `3dengine/RenderTarget.h` | 229 | 229 | +3 | -3 | minor diff |
| `3dengine/SceneObject.cpp` | 5049 | 5044 | +115 | -120 | minor diff |
| `3dengine/SceneObject.h` | 1325 | 1325 | +430 | -430 | significant diff |
| `3dengine/SceneObjectPicking.cpp` | 835 | 831 | +0 | -4 | minor diff |
| `3dengine/SceneObjectPicking.h` | 56 | 56 | +0 | -0 | identical-content |
| `3dengine/SceneState.cpp` | 411 | 410 | +1 | -2 | minor diff |
| `3dengine/SceneState.h` | 629 | 625 | +3 | -7 | minor diff |
| `3dengine/SceneWriter.cpp` | 233 | 233 | +0 | -0 | identical-content |
| `3dengine/SceneWriter.h` | 73 | 73 | +0 | -0 | identical-content |
| `3dengine/ScriptParticle.cpp` | 563 | 570 | +8 | -1 | minor diff |
| `3dengine/ScriptParticle.h` | 136 | 136 | +0 | -0 | identical-content |
| `3dengine/SelectionManager.cpp` | 474 | 474 | +0 | -0 | identical-content |
| `3dengine/SelectionManager.h` | 220 | 220 | +0 | -0 | identical-content |
| `3dengine/SequenceCtler.cpp` | 769 | 769 | +0 | -0 | identical-content |
| `3dengine/SequenceCtler.h` | 177 | 177 | +0 | -0 | identical-content |
| `3dengine/SequenceEntity.cpp` | 175 | 175 | +0 | -0 | identical-content |
| `3dengine/SequenceEntity.h` | 112 | 112 | +0 | -0 | identical-content |
| `3dengine/SequenceManager.cpp` | 23 | 23 | +0 | -0 | identical-content |
| `3dengine/SequenceManager.h` | 15 | 15 | +0 | -0 | identical-content |
| `3dengine/ShadowMap.cpp` | 1626 | 1638 | +88 | -76 | moderate diff |
| `3dengine/ShadowMap.h` | 158 | 158 | +1 | -1 | minor diff |
| `3dengine/ShadowVolume.cpp` | 145 | 148 | +5 | -2 | minor diff |
| `3dengine/ShadowVolume.h` | 101 | 101 | +0 | -0 | identical-content |
| `3dengine/SkyMesh.cpp` | 860 | 863 | +45 | -42 | moderate diff |
| `3dengine/SkyMesh.h` | 293 | 293 | +3 | -3 | minor diff |
| `3dengine/SortedFaceGroups.cpp` | 687 | 682 | +49 | -54 | moderate diff |
| `3dengine/SortedFaceGroups.h` | 136 | 136 | +0 | -0 | identical-content |
| `3dengine/SphereObject.cpp` | 70 | 70 | +0 | -0 | identical-content |
| `3dengine/SphereObject.h` | 84 | 84 | +0 | -0 | identical-content |
| `3dengine/SunLight.cpp` | 326 | 326 | +1 | -1 | minor diff |
| `3dengine/SunLight.h` | 223 | 223 | +0 | -0 | identical-content |
| `3dengine/TerrainTile.cpp` | 470 | 470 | +0 | -0 | identical-content |
| `3dengine/TerrainTile.h` | 115 | 115 | +0 | -0 | identical-content |
| `3dengine/TerrainTileRoot.cpp` | 592 | 592 | +0 | -0 | identical-content |
| `3dengine/TerrainTileRoot.h` | 167 | 167 | +0 | -0 | identical-content |
| `3dengine/TextureComposer.cpp` | 129 | 129 | +1 | -1 | minor diff |
| `3dengine/TextureComposer.h` | 75 | 75 | +0 | -0 | identical-content |
| `3dengine/TileObject.cpp` | 220 | 220 | +0 | -0 | identical-content |
| `3dengine/TileObject.h` | 117 | 117 | +0 | -0 | identical-content |
| `3dengine/ViewCullingObject.cpp` | 295 | 295 | +0 | -0 | identical-content |
| `3dengine/ViewCullingObject.h` | 90 | 90 | +0 | -0 | identical-content |
| `3dengine/Viewport.cpp` | 740 | 740 | +67 | -67 | moderate diff |
| `3dengine/Viewport.h` | 305 | 305 | +0 | -0 | identical-content |
| `3dengine/ViewportManager.cpp` | 909 | 970 | +91 | -30 | moderate diff |
| `3dengine/ViewportManager.h` | 135 | 132 | +100 | -103 | significant diff |
| `3dengine/WeatherEffect.cpp` | 249 | 249 | +0 | -0 | identical-content |
| `3dengine/WeatherEffect.h` | 92 | 92 | +0 | -0 | identical-content |
| `3dengine/WeatherParticle.cpp` | 235 | 235 | +0 | -0 | identical-content |
| `3dengine/WeatherParticle.h` | 112 | 112 | +0 | -0 | identical-content |
| `3dengine/WeatherParticleSpawner.cpp` | 195 | 195 | +0 | -0 | identical-content |
| `3dengine/WeatherParticleSpawner.h` | 125 | 125 | +4 | -4 | moderate diff |
| `3dengine/Win32MCI.cpp` | 59 | 61 | +4 | -2 | moderate diff |
| `3dengine/Win32MCI.h` | 15 | 15 | +0 | -0 | identical-content |
| `3dengine/WorldInfo.cpp` | 50 | 50 | +0 | -0 | identical-content |
| `3dengine/WorldInfo.h` | 57 | 57 | +0 | -0 | identical-content |
| `3dengine/XRefObject.cpp` | 499 | 499 | +3 | -3 | minor diff |
| `3dengine/XRefObject.h` | 245 | 245 | +0 | -0 | identical-content |
| `3dengine/ZoneNode.cpp` | 357 | 360 | +15 | -12 | moderate diff |
| `3dengine/ZoneNode.h` | 114 | 114 | +0 | -0 | identical-content |
| `3dengine/light_scattering_data.cpp` | 173 | 173 | +0 | -0 | identical-content |
| `3dengine/light_scattering_data.h` | 53 | 53 | +1 | -1 | minor diff |
| `3dengine/sceneRenderList.h` | 334 | 334 | +13 | -13 | moderate diff |

### `AutoRigger/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `AutoRigger/AutoRigger.cpp` | 1400 | 1354 | +12 | -58 | moderate diff |
| `AutoRigger/AutoRigger.h` | 64 | 77 | +17 | -4 | significant diff |

### `BMaxModel/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `BMaxModel/BMaxAnimGenerator.cpp` | 428 | 428 | +0 | -0 | identical-content |
| `BMaxModel/BMaxAnimGenerator.h` | 122 | 122 | +0 | -0 | identical-content |
| `BMaxModel/BMaxBlockModelNode.cpp` | 75 | 75 | +0 | -0 | identical-content |
| `BMaxModel/BMaxBlockModelNode.h` | 33 | 33 | +0 | -0 | identical-content |
| `BMaxModel/BMaxFrameNode.cpp` | 382 | 382 | +0 | -0 | identical-content |
| `BMaxModel/BMaxFrameNode.h` | 69 | 69 | +0 | -0 | identical-content |
| `BMaxModel/BMaxGlassModelNode.cpp` | 49 | 49 | +0 | -0 | identical-content |
| `BMaxModel/BMaxGlassModelNode.h` | 19 | 18 | +0 | -1 | moderate diff |
| `BMaxModel/BMaxNode.cpp` | 439 | 439 | +0 | -0 | identical-content |
| `BMaxModel/BMaxNode.h` | 90 | 90 | +0 | -0 | identical-content |
| `BMaxModel/BMaxObject.cpp` | 634 | 635 | +11 | -10 | minor diff |
| `BMaxModel/BMaxObject.h` | 142 | 142 | +0 | -0 | identical-content |
| `BMaxModel/BMaxParser.cpp` | 1837 | 1838 | +37 | -36 | minor diff |
| `BMaxModel/BMaxParser.h` | 216 | 217 | +21 | -20 | moderate diff |
| `BMaxModel/BlocksParser.cpp` | 642 | 642 | +0 | -0 | identical-content |
| `BMaxModel/BlocksParser.h` | 29 | 29 | +0 | -0 | identical-content |
| `BMaxModel/Rectangle.cpp` | 127 | 127 | +0 | -0 | identical-content |
| `BMaxModel/Rectangle.h` | 41 | 41 | +0 | -0 | identical-content |

### `BlockEngine/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `BlockEngine/BlockChunk.cpp` | 999 | 999 | +126 | -126 | significant diff |
| `BlockEngine/BlockChunk.h` | 334 | 334 | +38 | -38 | significant diff |
| `BlockEngine/BlockCommon.cpp` | 540 | 540 | +0 | -0 | identical-content |
| `BlockEngine/BlockCommon.h` | 187 | 187 | +0 | -0 | identical-content |
| `BlockEngine/BlockConfig.cpp` | 40 | 40 | +0 | -0 | identical-content |
| `BlockEngine/BlockConfig.h` | 36 | 36 | +0 | -0 | identical-content |
| `BlockEngine/BlockCoordinate.cpp` | 175 | 175 | +0 | -0 | identical-content |
| `BlockEngine/BlockCoordinate.h` | 178 | 178 | +0 | -0 | identical-content |
| `BlockEngine/BlockDataCodec.cpp` | 11 | 11 | +0 | -0 | identical-content |
| `BlockEngine/BlockDataCodec.h` | 378 | 378 | +7 | -7 | minor diff |
| `BlockEngine/BlockDirection.cpp` | 97 | 97 | +0 | -0 | identical-content |
| `BlockEngine/BlockDirection.h` | 26 | 26 | +0 | -0 | identical-content |
| `BlockEngine/BlockFacing.cpp` | 32 | 32 | +0 | -0 | identical-content |
| `BlockEngine/BlockFacing.h` | 28 | 28 | +0 | -0 | identical-content |
| `BlockEngine/BlockIndex.h` | 98 | 98 | +0 | -0 | identical-content |
| `BlockEngine/BlockLightGridBase.cpp` | 185 | 185 | +0 | -0 | identical-content |
| `BlockEngine/BlockLightGridBase.h` | 168 | 168 | +0 | -0 | identical-content |
| `BlockEngine/BlockLightGridClient.cpp` | 1394 | 1393 | +0 | -1 | minor diff |
| `BlockEngine/BlockLightGridClient.h` | 194 | 194 | +18 | -18 | moderate diff |
| `BlockEngine/BlockLightGridServer.cpp` | 85 | 85 | +0 | -0 | identical-content |
| `BlockEngine/BlockLightGridServer.h` | 63 | 63 | +0 | -0 | identical-content |
| `BlockEngine/BlockMaterial.cpp` | 71 | 71 | +0 | -0 | identical-content |
| `BlockEngine/BlockMaterial.h` | 101 | 101 | +0 | -0 | identical-content |
| `BlockEngine/BlockMaterialManager.cpp` | 44 | 44 | +0 | -0 | identical-content |
| `BlockEngine/BlockMaterialManager.h` | 34 | 34 | +0 | -0 | identical-content |
| `BlockEngine/BlockModel.cpp` | 1784 | 1784 | +0 | -0 | identical-content |
| `BlockEngine/BlockModel.h` | 451 | 451 | +67 | -67 | significant diff |
| `BlockEngine/BlockModelManager.cpp` | 224 | 224 | +0 | -0 | identical-content |
| `BlockEngine/BlockModelManager.h` | 35 | 35 | +0 | -0 | identical-content |
| `BlockEngine/BlockModelProvider.cpp` | 102 | 102 | +1 | -1 | minor diff |
| `BlockEngine/BlockModelProvider.h` | 68 | 68 | +0 | -0 | identical-content |
| `BlockEngine/BlockReadWriteLock.cpp` | 205 | 205 | +0 | -0 | identical-content |
| `BlockEngine/BlockReadWriteLock.h` | 240 | 240 | +0 | -0 | identical-content |
| `BlockEngine/BlockRegion.cpp` | 2434 | 2434 | +29 | -29 | minor diff |
| `BlockEngine/BlockRegion.h` | 283 | 283 | +0 | -0 | identical-content |
| `BlockEngine/BlockRenderTask.cpp` | 118 | 118 | +6 | -6 | moderate diff |
| `BlockEngine/BlockRenderTask.h` | 126 | 126 | +10 | -10 | moderate diff |
| `BlockEngine/BlockTemplate.cpp` | 670 | 670 | +0 | -0 | identical-content |
| `BlockEngine/BlockTemplate.h` | 330 | 330 | +0 | -0 | identical-content |
| `BlockEngine/BlockTessellateFastCutConfig.cpp` | 11153 | 11153 | +0 | -0 | identical-content |
| `BlockEngine/BlockTessellateFastCutConfig.h` | 38 | 38 | +0 | -0 | identical-content |
| `BlockEngine/BlockTessellators.cpp` | 1107 | 1107 | +0 | -0 | identical-content |
| `BlockEngine/BlockTessellators.h` | 92 | 89 | +1 | -4 | moderate diff |
| `BlockEngine/BlockWorld.cpp` | 2822 | 2822 | +0 | -0 | identical-content |
| `BlockEngine/BlockWorld.h` | 731 | 731 | +0 | -0 | identical-content |
| `BlockEngine/BlockWorldClient.cpp` | 3524 | 3424 | +398 | -498 | significant diff |
| `BlockEngine/BlockWorldClient.h` | 422 | 426 | +5 | -1 | minor diff |
| `BlockEngine/BlockWorldManager.cpp` | 78 | 78 | +0 | -0 | identical-content |
| `BlockEngine/BlockWorldManager.h` | 37 | 37 | +0 | -0 | identical-content |
| `BlockEngine/CarpetModelProvider.cpp` | 158 | 158 | +5 | -5 | moderate diff |
| `BlockEngine/CarpetModelProvider.h` | 24 | 24 | +0 | -0 | identical-content |
| `BlockEngine/ChunkMaxHeight.cpp` | 67 | 67 | +0 | -0 | identical-content |
| `BlockEngine/ChunkMaxHeight.h` | 32 | 32 | +0 | -0 | identical-content |
| `BlockEngine/ChunkVertexBuilderManager.cpp` | 345 | 369 | +27 | -3 | moderate diff |
| `BlockEngine/ChunkVertexBuilderManager.h` | 82 | 85 | +3 | -0 | minor diff |
| `BlockEngine/MultiFrameBlockWorldRenderer.cpp` | 699 | 699 | +0 | -0 | identical-content |
| `BlockEngine/MultiFrameBlockWorldRenderer.h` | 208 | 208 | +0 | -0 | identical-content |
| `BlockEngine/RenderableChunk.cpp` | 1151 | 1137 | +11 | -25 | minor diff |
| `BlockEngine/RenderableChunk.h` | 274 | 274 | +1 | -1 | minor diff |
| `BlockEngine/SlopeModelProvider.cpp` | 614 | 614 | +0 | -0 | identical-content |
| `BlockEngine/SlopeModelProvider.h` | 35 | 35 | +0 | -0 | identical-content |
| `BlockEngine/StairModelProvider.cpp` | 724 | 724 | +0 | -0 | identical-content |
| `BlockEngine/StairModelProvider.h` | 25 | 25 | +0 | -0 | identical-content |
| `BlockEngine/WireModelProvider.cpp` | 283 | 283 | +1 | -1 | minor diff |
| `BlockEngine/WireModelProvider.h` | 22 | 22 | +0 | -0 | identical-content |

### `CadModel/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `CadModel/CadContentLoader.cpp` | 355 | 357 | +12 | -10 | moderate diff |
| `CadModel/CadContentLoader.h` | 61 | 61 | +2 | -2 | moderate diff |
| `CadModel/CadImporterFactory.h` | 51 | 51 | +0 | -0 | identical-content |
| `CadModel/CadModel.cpp` | 240 | 243 | +10 | -7 | moderate diff |
| `CadModel/CadModel.h` | 130 | 130 | +2 | -2 | minor diff |
| `CadModel/CadModelNode.cpp` | 277 | 277 | +2 | -2 | minor diff |
| `CadModel/CadModelNode.h` | 67 | 67 | +0 | -0 | identical-content |
| `CadModel/ICadModelImporter.h` | 57 | 57 | +0 | -0 | identical-content |

### `CommonFramework/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `CommonFramework/BaseTable.cpp` | 808 | 808 | +0 | -0 | identical-content |
| `CommonFramework/BaseTable.h` | 295 | 295 | +0 | -0 | identical-content |
| `CommonFramework/DataTable.cpp` | 1160 | 1160 | +0 | -0 | identical-content |
| `CommonFramework/DataTable.h` | 235 | 235 | +0 | -0 | identical-content |
| `CommonFramework/bimap.h` | 2177 | 2177 | +0 | -0 | identical-content |
| `CommonFramework/stdint.h` | 224 | 224 | +0 | -0 | identical-content |

### `Core/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `Core/Array.h` | 453 | 453 | +0 | -0 | identical-content |
| `Core/AssetEntity.cpp` | 277 | 277 | +2 | -2 | minor diff |
| `Core/AssetEntity.h` | 276 | 276 | +1 | -1 | minor diff |
| `Core/AssetManager.h` | 448 | 448 | +70 | -70 | significant diff |
| `Core/AttributeClass.cpp` | 203 | 203 | +0 | -0 | identical-content |
| `Core/AttributeClass.h` | 108 | 108 | +4 | -4 | moderate diff |
| `Core/AttributeClassIDTable.h` | 52 | 52 | +0 | -0 | identical-content |
| `Core/AttributeField.cpp` | 133 | 133 | +0 | -0 | identical-content |
| `Core/AttributeField.h` | 210 | 210 | +0 | -0 | identical-content |
| `Core/AttributeModelProxy.h` | 86 | 86 | +2 | -2 | minor diff |
| `Core/AttributesManager.cpp` | 339 | 339 | +0 | -0 | identical-content |
| `Core/AttributesManager.h` | 67 | 67 | +0 | -0 | identical-content |
| `Core/BootStrapper.cpp` | 243 | 243 | +1 | -1 | minor diff |
| `Core/BootStrapper.h` | 78 | 78 | +0 | -0 | identical-content |
| `Core/CommandLineParams.cpp` | 135 | 135 | +4 | -4 | moderate diff |
| `Core/CommandLineParams.h` | 46 | 47 | +4 | -3 | moderate diff |
| `Core/CommandLineParser.hpp` | 113 | 113 | +0 | -0 | identical-content |
| `Core/ContentLoaderCCSFace.cpp` | 295 | 295 | +0 | -0 | identical-content |
| `Core/ContentLoaderCCSFace.h` | 96 | 96 | +0 | -0 | identical-content |
| `Core/ContentLoaderCCSSkin.cpp` | 267 | 267 | +0 | -0 | identical-content |
| `Core/ContentLoaderCCSSkin.h` | 96 | 96 | +0 | -0 | identical-content |
| `Core/ContentLoaderMesh.cpp` | 411 | 410 | +13 | -14 | moderate diff |
| `Core/ContentLoaderMesh.h` | 103 | 103 | +1 | -1 | minor diff |
| `Core/ContentLoaderParaX.cpp` | 434 | 434 | +12 | -12 | moderate diff |
| `Core/ContentLoaderParaX.h` | 89 | 89 | +1 | -1 | minor diff |
| `Core/ContentLoaderTexture.cpp` | 244 | 243 | +10 | -11 | moderate diff |
| `Core/ContentLoaderTexture.h` | 117 | 117 | +4 | -4 | moderate diff |
| `Core/ContentLoaders.h` | 6 | 6 | +0 | -0 | identical-content |
| `Core/DataProviderManager.cpp` | 136 | 136 | +3 | -3 | minor diff |
| `Core/DataProviderManager.h` | 89 | 89 | +0 | -0 | identical-content |
| `Core/DatabaseEntity.cpp` | 45 | 45 | +0 | -0 | identical-content |
| `Core/DatabaseEntity.h` | 49 | 49 | +0 | -0 | identical-content |
| `Core/DynamicAttributeField.cpp` | 32 | 32 | +0 | -0 | identical-content |
| `Core/DynamicAttributeField.h` | 26 | 26 | +0 | -0 | identical-content |
| `Core/DynamicAttributesSet.cpp` | 108 | 108 | +0 | -0 | identical-content |
| `Core/DynamicAttributesSet.h` | 66 | 66 | +0 | -0 | identical-content |
| `Core/DynamicVertexBufferEntity.h` | 33 | 51 | +19 | -1 | significant diff |
| `Core/DynamicVertexBufferEntityDirectX.cpp` | 217 | 218 | +8 | -7 | moderate diff |
| `Core/DynamicVertexBufferEntityDirectX.h` | 49 | 49 | +0 | -0 | identical-content |
| `Core/DynamicVertexBufferEntityOpenGL.cpp` | 100 | 100 | +1 | -1 | minor diff |
| `Core/DynamicVertexBufferEntityOpenGL.h` | 50 | 50 | +0 | -0 | identical-content |
| `Core/DynamicVertexBufferManager.cpp` | 71 | 102 | +31 | -0 | significant diff |
| `Core/DynamicVertexBufferManager.h` | 19 | 19 | +0 | -0 | identical-content |
| `Core/EventClasses.cpp` | 217 | 217 | +0 | -0 | identical-content |
| `Core/EventClasses.h` | 192 | 205 | +14 | -1 | moderate diff |
| `Core/EventHandler.cpp` | 107 | 107 | +0 | -0 | identical-content |
| `Core/EventHandler.h` | 64 | 64 | +0 | -0 | identical-content |
| `Core/EventsCenter.cpp` | 298 | 298 | +1 | -1 | minor diff |
| `Core/EventsCenter.h` | 140 | 140 | +0 | -0 | identical-content |
| `Core/Events_def.h` | 355 | 355 | +0 | -0 | identical-content |
| `Core/FrameRateController.cpp` | 340 | 340 | +0 | -0 | identical-content |
| `Core/FrameRateController.h` | 95 | 95 | +2 | -2 | minor diff |
| `Core/Globals.cpp` | 322 | 329 | +41 | -34 | significant diff |
| `Core/Globals.h` | 128 | 139 | +14 | -3 | moderate diff |
| `Core/IAnimated.cpp` | 144 | 144 | +0 | -0 | identical-content |
| `Core/IAnimated.h` | 363 | 363 | +0 | -0 | identical-content |
| `Core/IAttributeFields.cpp` | 319 | 313 | +1 | -7 | minor diff |
| `Core/IAttributeFields.h` | 228 | 227 | +24 | -25 | significant diff |
| `Core/IPCManager.cpp` | 69 | 69 | +0 | -0 | identical-content |
| `Core/IPCManager.h` | 42 | 42 | +0 | -0 | identical-content |
| `Core/ImageEntity.cpp` | 136 | 137 | +7 | -6 | moderate diff |
| `Core/ImageEntity.h` | 77 | 73 | +4 | -8 | moderate diff |
| `Core/InterprocessMsg.h` | 60 | 61 | +1 | -0 | minor diff |
| `Core/InterprocessQueue.hpp` | 880 | 880 | +0 | -0 | identical-content |
| `Core/MiscEntity.cpp` | 91 | 90 | +3 | -4 | moderate diff |
| `Core/MiscEntity.h` | 50 | 50 | +0 | -0 | identical-content |
| `Core/NPLInterface.hpp` | 2067 | 2067 | +0 | -0 | identical-content |
| `Core/NPLPackageConfig.cpp` | 99 | 99 | +0 | -0 | identical-content |
| `Core/NPLPackageConfig.h` | 34 | 34 | +0 | -0 | identical-content |
| `Core/NPLTypes.h` | 115 | 115 | +0 | -0 | identical-content |
| `Core/ObjScriptPool.h` | 169 | 169 | +0 | -0 | identical-content |
| `Core/ObjectFactory.h` | 35 | 35 | +0 | -0 | identical-content |
| `Core/PERef.cpp` | 93 | 93 | +0 | -0 | identical-content |
| `Core/PERef.h` | 332 | 332 | +0 | -0 | identical-content |
| `Core/PERefPtr.h` | 143 | 143 | +0 | -0 | identical-content |
| `Core/PEtypes.h` | 547 | 573 | +95 | -69 | significant diff |
| `Core/ParaEngine.h` | 53 | 68 | +20 | -5 | significant diff |
| `Core/ParaEngineAppBase.cpp` | 843 | 1757 | +1094 | -180 | significant diff |
| `Core/ParaEngineAppBase.h` | 449 | 459 | +418 | -408 | significant diff |
| `Core/ParaEngineCore.cpp` | 151 | 181 | +38 | -8 | significant diff |
| `Core/ParaEngineCore.h` | 53 | 56 | +3 | -0 | moderate diff |
| `Core/ParaEngineInfo.cpp` | 77 | 75 | +5 | -7 | moderate diff |
| `Core/ParaEngineInfo.h` | 31 | 31 | +0 | -0 | identical-content |
| `Core/ParaEngineServerApp.cpp` | 112 | 106 | +11 | -17 | significant diff |
| `Core/ParaEngineServerApp.h` | 51 | 45 | +7 | -13 | significant diff |
| `Core/ParaEngineService.cpp` | 272 | 268 | +14 | -18 | moderate diff |
| `Core/ParaEngineService.h` | 73 | 75 | +13 | -11 | significant diff |
| `Core/ParaPlatformConfig.h` | 102 | 107 | +24 | -19 | significant diff |
| `Core/ParaPlatformMacros.h` | 31 | 31 | +0 | -0 | identical-content |
| `Core/ParaUtils.hpp` | 116 | 116 | +0 | -0 | identical-content |
| `Core/ParaWorldAsset.cpp` | 1250 | 1257 | +21 | -14 | minor diff |
| `Core/ParaWorldAsset.h` | 286 | 286 | +1 | -1 | minor diff |
| `Core/ParameterBlock.cpp` | 554 | 572 | +32 | -14 | moderate diff |
| `Core/ParameterBlock.h` | 207 | 205 | +4 | -6 | minor diff |
| `Core/ParticleElement.h` | 88 | 88 | +0 | -0 | identical-content |
| `Core/Platforms.cpp` | 243 | 243 | +0 | -0 | identical-content |
| `Core/Platforms.h` | 80 | 81 | +2 | -1 | minor diff |
| `Core/PluginAPI.cpp` | 27 | 27 | +0 | -0 | identical-content |
| `Core/PluginAPI.h` | 150 | 150 | +0 | -0 | identical-content |
| `Core/PluginLoader.hpp` | 386 | 380 | +1 | -7 | minor diff |
| `Core/PluginManager.cpp` | 452 | 448 | +15 | -19 | moderate diff |
| `Core/PluginManager.h` | 174 | 174 | +0 | -0 | identical-content |
| `Core/SpriteFontEntity.cpp` | 61 | 61 | +0 | -0 | identical-content |
| `Core/SpriteFontEntity.h` | 82 | 82 | +0 | -0 | identical-content |
| `Core/SpriteFontEntityDirectX.cpp` | 234 | 237 | +7 | -4 | minor diff |
| `Core/SpriteFontEntityDirectX.h` | 62 | 62 | +0 | -0 | identical-content |
| `Core/SpriteFontEntityOpenGL.cpp` | 82 | 100 | +20 | -2 | significant diff |
| `Core/SpriteFontEntityOpenGL.h` | 37 | 44 | +8 | -1 | significant diff |
| `Core/TextureEntity.cpp` | 608 | 574 | +122 | -156 | significant diff |
| `Core/TextureEntity.h` | 384 | 389 | +45 | -40 | significant diff |
| `Core/TextureEntityDirectX.cpp` | 1946 | 1757 | +343 | -532 | significant diff |
| `Core/TextureEntityDirectX.h` | 159 | 151 | +8 | -16 | moderate diff |
| `Core/TextureEntityOpenGL.cpp` | 622 | 1142 | +598 | -78 | significant diff |
| `Core/TextureEntityOpenGL.h` | 95 | 110 | +28 | -13 | significant diff |
| `Core/TouchEventSession.cpp` | 170 | 170 | +0 | -0 | identical-content |
| `Core/TouchEventSession.h` | 56 | 56 | +0 | -0 | identical-content |
| `Core/UrlLoaders.cpp` | 978 | 1220 | +311 | -69 | significant diff |
| `Core/UrlLoaders.h` | 352 | 510 | +169 | -11 | significant diff |
| `Core/Variable.cpp` | 516 | 516 | +0 | -0 | identical-content |
| `Core/Variable.h` | 197 | 197 | +0 | -0 | identical-content |
| `Core/coreexport.h` | 22 | 37 | +25 | -10 | significant diff |
| `Core/ipc_message_queue.hpp` | 652 | 654 | +4 | -2 | minor diff |
| `Core/luaSQLite.cpp` | 1984 | 1998 | +19 | -5 | minor diff |
| `Core/luaSQLite.h` | 9 | 9 | +0 | -0 | identical-content |
| `Core/memdebug.h` | 29 | 29 | +1 | -1 | moderate diff |
| `Core/unordered_ref_array.h` | 405 | 405 | +2 | -2 | minor diff |

### `Engine/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `Engine/BVHSerializer.cpp` | 1895 | 1895 | +599 | -599 | significant diff |
| `Engine/BVHSerializer.h` | 357 | 357 | +0 | -0 | identical-content |
| `Engine/CharacterDBProvider.cpp` | 219 | 219 | +2 | -2 | minor diff |
| `Engine/CharacterDBProvider.h` | 99 | 99 | +1 | -1 | minor diff |
| `Engine/ChestDBProvider.cpp` | 203 | 203 | +2 | -2 | minor diff |
| `Engine/ChestDBProvider.h` | 86 | 86 | +1 | -1 | minor diff |
| `Engine/DirectXEngine.cpp` | 684 | 692 | +36 | -28 | moderate diff |
| `Engine/DirectXEngine.h` | 126 | 126 | +2 | -2 | minor diff |
| `Engine/DocGen_NPL_mainpage.h` | 73 | 73 | +0 | -0 | identical-content |
| `Engine/DropShadowRenderer.cpp` | 333 | 337 | +32 | -28 | moderate diff |
| `Engine/DropShadowRenderer.h` | 59 | 59 | +1 | -1 | minor diff |
| `Engine/DynamicObject.cpp` | 24 | 24 | +0 | -0 | identical-content |
| `Engine/DynamicObject.h` | 19 | 19 | +0 | -0 | identical-content |
| `Engine/DynamicRenderable.cpp` | 164 | 167 | +7 | -4 | moderate diff |
| `Engine/DynamicRenderable.h` | 57 | 57 | +0 | -0 | identical-content |
| `Engine/EdgeBuilder.cpp` | 64 | 64 | +0 | -0 | identical-content |
| `Engine/EdgeBuilder.h` | 55 | 55 | +0 | -0 | identical-content |
| `Engine/EngineUtil.cpp` | 11 | 11 | +0 | -0 | identical-content |
| `Engine/EngineUtil.h` | 48 | 48 | +0 | -0 | identical-content |
| `Engine/FlashTextureManager.cpp` | 1708 | 1708 | +6 | -6 | minor diff |
| `Engine/FlashTextureManager.h` | 392 | 392 | +0 | -0 | identical-content |
| `Engine/FreeImageMemIO.cpp` | 122 | 122 | +0 | -0 | identical-content |
| `Engine/FreeImageMemIO.h` | 33 | 33 | +0 | -0 | identical-content |
| `Engine/FruitDBProvider.cpp` | 198 | 198 | +2 | -2 | minor diff |
| `Engine/FruitDBProvider.h` | 81 | 81 | +1 | -1 | minor diff |
| `Engine/GDIEngine.cpp` | 446 | 446 | +3 | -3 | minor diff |
| `Engine/GDIEngine.h` | 149 | 149 | +0 | -0 | identical-content |
| `Engine/GameNetCommon.cpp` | 135 | 135 | +0 | -0 | identical-content |
| `Engine/GameNetCommon.h` | 247 | 247 | +0 | -0 | identical-content |
| `Engine/GlowEffect.cpp` | 355 | 349 | +31 | -37 | moderate diff |
| `Engine/GlowEffect.h` | 59 | 59 | +0 | -0 | identical-content |
| `Engine/GuiConsole.cpp` | 94 | 94 | +0 | -0 | identical-content |
| `Engine/GuiConsole.h` | 31 | 31 | +0 | -0 | identical-content |
| `Engine/HTMLBrowserManager.cpp` | 899 | 903 | +9 | -5 | minor diff |
| `Engine/HTMLBrowserManager.h` | 221 | 221 | +0 | -0 | identical-content |
| `Engine/IHTMLRenderer.h` | 187 | 187 | +0 | -0 | identical-content |
| `Engine/ITouchInputTranslator.h` | 27 | 27 | +0 | -0 | identical-content |
| `Engine/ItemDBProvider.cpp` | 462 | 462 | +2 | -2 | minor diff |
| `Engine/ItemDBProvider.h` | 101 | 101 | +1 | -1 | minor diff |
| `Engine/KidsDBProvider.cpp` | 1769 | 1769 | +2 | -2 | minor diff |
| `Engine/KidsDBProvider.h` | 298 | 298 | +1 | -1 | minor diff |
| `Engine/LibInterface.cpp` | 106 | 106 | +1 | -1 | minor diff |
| `Engine/MathLib.h` | 109 | 109 | +0 | -0 | identical-content |
| `Engine/MirrorSurface.cpp` | 297 | 301 | +15 | -11 | moderate diff |
| `Engine/MirrorSurface.h` | 107 | 107 | +0 | -0 | identical-content |
| `Engine/Modifier.cpp` | 26 | 26 | +0 | -0 | identical-content |
| `Engine/Modifier.h` | 32 | 32 | +1 | -1 | moderate diff |
| `Engine/MovieClip.cpp` | 40 | 40 | +0 | -0 | identical-content |
| `Engine/MovieClip.h` | 70 | 70 | +0 | -0 | identical-content |
| `Engine/NpcDatabase.cpp` | 1042 | 1042 | +2 | -2 | minor diff |
| `Engine/NpcDatabase.h` | 207 | 207 | +1 | -1 | minor diff |
| `Engine/OcclusionQueryBank.cpp` | 239 | 242 | +5 | -2 | minor diff |
| `Engine/OcclusionQueryBank.h` | 89 | 89 | +1 | -1 | minor diff |
| `Engine/ParaEngineClientConfig.h` | 81 | 81 | +0 | -0 | identical-content |
| `Engine/ParaEngineClientConfig.win32.h` | 81 | 81 | +0 | -0 | identical-content |
| `Engine/ParaEngineServer.cpp` | 77 | 77 | +0 | -0 | identical-content |
| `Engine/PetAIDBProvider.cpp` | 153 | 153 | +2 | -2 | minor diff |
| `Engine/PetAIDBProvider.h` | 62 | 62 | +1 | -1 | minor diff |
| `Engine/PetDBProvider.cpp` | 237 | 237 | +2 | -2 | minor diff |
| `Engine/PetDBProvider.h` | 91 | 91 | +1 | -1 | minor diff |
| `Engine/PuzzleDBProvider.cpp` | 244 | 244 | +2 | -2 | minor diff |
| `Engine/PuzzleDBProvider.h` | 83 | 83 | +1 | -1 | minor diff |
| `Engine/QuestDBProvider.cpp` | 213 | 213 | +2 | -2 | minor diff |
| `Engine/QuestDBProvider.h` | 80 | 80 | +1 | -1 | minor diff |
| `Engine/RpgCharacter.cpp` | 406 | 406 | +0 | -0 | identical-content |
| `Engine/RpgCharacter.h` | 269 | 269 | +0 | -0 | identical-content |
| `Engine/ScreenShotSystem.cpp` | 2423 | 2397 | +14 | -40 | minor diff |
| `Engine/ScreenShotSystem.h` | 354 | 354 | +0 | -0 | identical-content |
| `Engine/SpriteObject.cpp` | 178 | 180 | +20 | -18 | significant diff |
| `Engine/SpriteObject.h` | 146 | 146 | +2 | -2 | minor diff |
| `Engine/StaticMesh.cpp` | 1833 | 1848 | +67 | -52 | moderate diff |
| `Engine/StaticMesh.h` | 133 | 136 | +8 | -5 | moderate diff |
| `Engine/StringTable.cpp` | 247 | 247 | +2 | -2 | minor diff |
| `Engine/StringTable.h` | 76 | 76 | +1 | -1 | minor diff |
| `Engine/TitleDBProvider.cpp` | 176 | 176 | +2 | -2 | minor diff |
| `Engine/TitleDBProvider.h` | 66 | 66 | +1 | -1 | minor diff |
| `Engine/ValueTracker.cpp` | 30 | 30 | +0 | -0 | identical-content |
| `Engine/ValueTracker.h` | 276 | 276 | +0 | -0 | identical-content |
| `Engine/VoxelMesh.cpp` | 156 | 156 | +2 | -2 | minor diff |
| `Engine/VoxelMesh.h` | 96 | 96 | +0 | -0 | identical-content |
| `Engine/WaveEffect.cpp` | 121 | 124 | +17 | -14 | significant diff |
| `Engine/WaveEffect.h` | 37 | 37 | +0 | -0 | identical-content |
| `Engine/config.h` | 74 | 74 | +0 | -0 | identical-content |
| `Engine/guicon.cpp` | 134 | 134 | +0 | -0 | identical-content |
| `Engine/guicon.h` | 15 | 15 | +0 | -0 | identical-content |
| `Engine/resource.h` | 88 | 88 | +0 | -0 | identical-content |
| `Engine/vslua_debugger.h` | 119 | 119 | +0 | -0 | identical-content |

### `Framework/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `Framework/Common/PlatformBridge/PlatformBridge.cpp` | 73 | 74 | +8 | -7 | significant diff |
| `Framework/Common/PlatformBridge/PlatformBridge.h` | 51 | 51 | +0 | -0 | identical-content |
| `Framework/InputSystem/VirtualKey.cpp` | 32 | 32 | +0 | -0 | identical-content |
| `Framework/InputSystem/VirtualKey.h` | 289 | 287 | +81 | -83 | significant diff |

### `IO/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `IO/Archive.cpp` | 51 | 51 | +0 | -0 | identical-content |
| `IO/Archive.h` | 133 | 133 | +0 | -0 | identical-content |
| `IO/AssetManifest.cpp` | 1106 | 1106 | +6 | -6 | minor diff |
| `IO/AssetManifest.h` | 345 | 345 | +2 | -2 | minor diff |
| `IO/AsyncLoader.cpp` | 1043 | 1263 | +370 | -150 | significant diff |
| `IO/AsyncLoader.h` | 397 | 422 | +30 | -5 | moderate diff |
| `IO/FileData.cpp` | 103 | 103 | +0 | -0 | identical-content |
| `IO/FileData.h` | 50 | 50 | +1 | -1 | minor diff |
| `IO/FileHandle.cpp` | 36 | 36 | +0 | -0 | identical-content |
| `IO/FileHandle.h` | 28 | 27 | +1 | -2 | moderate diff |
| `IO/FileLogger.cpp` | 237 | 237 | +2 | -2 | minor diff |
| `IO/FileLogger.h` | 98 | 98 | +0 | -0 | identical-content |
| `IO/FileManager.cpp` | 327 | 334 | +26 | -19 | moderate diff |
| `IO/FileManager.h` | 139 | 131 | +1 | -9 | moderate diff |
| `IO/FilePath.cpp` | 253 | 253 | +0 | -0 | identical-content |
| `IO/FilePath.h` | 136 | 136 | +0 | -0 | identical-content |
| `IO/FileSearchResult.cpp` | 135 | 129 | +0 | -6 | minor diff |
| `IO/FileSearchResult.h` | 73 | 73 | +0 | -0 | identical-content |
| `IO/FileSystemWatcher.cpp` | 327 | 327 | +0 | -0 | identical-content |
| `IO/FileSystemWatcher.h` | 140 | 140 | +6 | -6 | moderate diff |
| `IO/FileUtils.cpp` | 1806 | 877 | +152 | -1081 | significant diff |
| `IO/FileUtils.h` | 257 | 257 | +3 | -3 | minor diff |
| `IO/IDataLoader.h` | 165 | 169 | +4 | -0 | minor diff |
| `IO/IFile.h` | 78 | 78 | +7 | -7 | moderate diff |
| `IO/MemReadFile.cpp` | 122 | 122 | +20 | -20 | significant diff |
| `IO/MemReadFile.h` | 58 | 57 | +16 | -17 | significant diff |
| `IO/ParaFile.cpp` | 1974 | 1942 | +27 | -59 | minor diff |
| `IO/ParaFile.h` | 684 | 684 | +6 | -6 | minor diff |
| `IO/ResourceEmbedded.h` | 41 | 41 | +0 | -0 | identical-content |
| `IO/SerialPort.cpp` | 729 | 729 | +4 | -4 | minor diff |
| `IO/ZipArchive.cpp` | 1867 | 1859 | +206 | -214 | significant diff |
| `IO/ZipArchive.h` | 446 | 444 | +50 | -52 | significant diff |
| `IO/ZipWriter.cpp` | 531 | 531 | +0 | -0 | identical-content |
| `IO/ZipWriter.h` | 100 | 100 | +0 | -0 | identical-content |

### `NPL/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `NPL/AISimulator.cpp` | 241 | 241 | +0 | -0 | identical-content |
| `NPL/AISimulator.h` | 92 | 92 | +0 | -0 | identical-content |
| `NPL/EmscriptenWebSocket.h` | 185 | 185 | +1 | -1 | minor diff |
| `NPL/NPLActivationFile.cpp` | 78 | 132 | +55 | -1 | significant diff |
| `NPL/NPLActivationFile.h` | 42 | 43 | +1 | -0 | minor diff |
| `NPL/NPLCodec.cpp` | 190 | 190 | +11 | -11 | moderate diff |
| `NPL/NPLCodec.h` | 77 | 77 | +10 | -10 | significant diff |
| `NPL/NPLCommon.cpp` | 197 | 197 | +0 | -0 | identical-content |
| `NPL/NPLCommon.h` | 259 | 259 | +0 | -0 | identical-content |
| `NPL/NPLCompiler.cpp` | 196 | 196 | +0 | -0 | identical-content |
| `NPL/NPLCompiler.h` | 79 | 79 | +0 | -0 | identical-content |
| `NPL/NPLConfig.cpp` | 21 | 21 | +0 | -0 | identical-content |
| `NPL/NPLConfig.h` | 18 | 18 | +0 | -0 | identical-content |
| `NPL/NPLConnection.cpp` | 1022 | 1021 | +0 | -1 | minor diff |
| `NPL/NPLConnection.h` | 450 | 450 | +77 | -77 | significant diff |
| `NPL/NPLConnectionManager.cpp` | 132 | 132 | +0 | -0 | identical-content |
| `NPL/NPLConnectionManager.h` | 64 | 64 | +0 | -0 | identical-content |
| `NPL/NPLDispatcher.cpp` | 703 | 704 | +93 | -92 | significant diff |
| `NPL/NPLDispatcher.h` | 251 | 253 | +2 | -0 | minor diff |
| `NPL/NPLHelper.cpp` | 1657 | 1656 | +0 | -1 | minor diff |
| `NPL/NPLHelper.h` | 192 | 192 | +0 | -0 | identical-content |
| `NPL/NPLMemPool.cpp` | 36 | 36 | +0 | -0 | identical-content |
| `NPL/NPLMemPool.h` | 171 | 171 | +0 | -0 | identical-content |
| `NPL/NPLMessage.cpp` | 24 | 24 | +0 | -0 | identical-content |
| `NPL/NPLMessage.h` | 132 | 132 | +0 | -0 | identical-content |
| `NPL/NPLMessageQueue.cpp` | 27 | 27 | +0 | -0 | identical-content |
| `NPL/NPLMessageQueue.h` | 312 | 325 | +32 | -19 | moderate diff |
| `NPL/NPLMsgHeader.h` | 11 | 11 | +0 | -0 | identical-content |
| `NPL/NPLMsgIn.h` | 57 | 57 | +1 | -1 | minor diff |
| `NPL/NPLMsgIn_parser.cpp` | 392 | 392 | +1 | -1 | minor diff |
| `NPL/NPLMsgIn_parser.h` | 233 | 233 | +1 | -1 | minor diff |
| `NPL/NPLMsgOut.cpp` | 199 | 199 | +1 | -1 | minor diff |
| `NPL/NPLMsgOut.h` | 105 | 106 | +2 | -1 | minor diff |
| `NPL/NPLNetClient.cpp` | 890 | 890 | +0 | -0 | identical-content |
| `NPL/NPLNetClient.h` | 431 | 431 | +0 | -0 | identical-content |
| `NPL/NPLNetServer.cpp` | 555 | 555 | +0 | -0 | identical-content |
| `NPL/NPLNetServer.h` | 251 | 251 | +0 | -0 | identical-content |
| `NPL/NPLNetUDPServer.cpp` | 343 | 343 | +0 | -0 | identical-content |
| `NPL/NPLNetUDPServer.h` | 193 | 193 | +1 | -1 | minor diff |
| `NPL/NPLParser.cpp` | 723 | 723 | +0 | -0 | identical-content |
| `NPL/NPLParser.h` | 205 | 205 | +0 | -0 | identical-content |
| `NPL/NPLRuntime.cpp` | 1353 | 1353 | +159 | -159 | significant diff |
| `NPL/NPLRuntime.h` | 810 | 816 | +216 | -210 | significant diff |
| `NPL/NPLStateMemAllocator.cpp` | 190 | 190 | +0 | -0 | identical-content |
| `NPL/NPLStateMemAllocator.h` | 64 | 64 | +0 | -0 | identical-content |
| `NPL/NPLStruct_obsoleted.h` | 47 | 47 | +0 | -0 | identical-content |
| `NPL/NPLTable.cpp` | 289 | 289 | +0 | -0 | identical-content |
| `NPL/NPLTable.h` | 268 | 268 | +0 | -0 | identical-content |
| `NPL/NPLUDPDispatcher.cpp` | 388 | 388 | +0 | -0 | identical-content |
| `NPL/NPLUDPDispatcher.h` | 132 | 134 | +2 | -0 | minor diff |
| `NPL/NPLUDPRoute.cpp` | 523 | 523 | +0 | -0 | identical-content |
| `NPL/NPLUDPRoute.h` | 325 | 344 | +29 | -10 | moderate diff |
| `NPL/NPLUDPRouteManager.cpp` | 158 | 168 | +12 | -2 | moderate diff |
| `NPL/NPLUDPRouteManager.h` | 67 | 71 | +4 | -0 | moderate diff |
| `NPL/NPLWriter.cpp` | 215 | 215 | +0 | -0 | identical-content |
| `NPL/NPLWriter.h` | 104 | 104 | +0 | -0 | identical-content |
| `NPL/NPL_imp.cpp` | 165 | 165 | +1 | -1 | minor diff |
| `NPL/NPL_imp.h` | 53 | 53 | +0 | -0 | identical-content |
| `NPL/NPL_readme.txt` | 234 | 234 | - | - | binary/non-code |
| `NPL/NeuronFileState.cpp` | 127 | 127 | +0 | -0 | identical-content |
| `NPL/NeuronFileState.h` | 97 | 97 | +0 | -0 | identical-content |

### `PaintEngine/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `PaintEngine/PaintDevice.cpp` | 45 | 45 | +0 | -0 | identical-content |
| `PaintEngine/PaintDevice.h` | 55 | 55 | +0 | -0 | identical-content |
| `PaintEngine/PaintEngine.cpp` | 188 | 188 | +0 | -0 | identical-content |
| `PaintEngine/PaintEngine.h` | 210 | 210 | +0 | -0 | identical-content |
| `PaintEngine/PaintEngineGPU.cpp` | 861 | 858 | +76 | -79 | moderate diff |
| `PaintEngine/PaintEngineGPU.h` | 150 | 150 | +0 | -0 | identical-content |
| `PaintEngine/PaintEngineGPUState.cpp` | 62 | 62 | +0 | -0 | identical-content |
| `PaintEngine/PaintEngineGPUState.h` | 37 | 37 | +0 | -0 | identical-content |
| `PaintEngine/PaintEngineRaster.cpp` | 53 | 52 | +4 | -5 | moderate diff |
| `PaintEngine/PaintEngineRaster.h` | 41 | 41 | +0 | -0 | identical-content |
| `PaintEngine/ParaRegion.cpp` | 90 | 90 | +0 | -0 | identical-content |
| `PaintEngine/ParaRegion.h` | 133 | 133 | +0 | -0 | identical-content |
| `PaintEngine/qbrush.cpp` | 97 | 97 | +0 | -0 | identical-content |
| `PaintEngine/qbrush.h` | 54 | 54 | +0 | -0 | identical-content |
| `PaintEngine/qfont.cpp` | 131 | 131 | +0 | -0 | identical-content |
| `PaintEngine/qfont.h` | 107 | 107 | +0 | -0 | identical-content |
| `PaintEngine/qmatrix.cpp` | 338 | 338 | +0 | -0 | identical-content |
| `PaintEngine/qmatrix.h` | 116 | 116 | +0 | -0 | identical-content |
| `PaintEngine/qpainterpath.cpp` | 75 | 75 | +0 | -0 | identical-content |
| `PaintEngine/qpainterpath.h` | 274 | 274 | +0 | -0 | identical-content |
| `PaintEngine/qpen.cpp` | 144 | 144 | +0 | -0 | identical-content |
| `PaintEngine/qpen.h` | 71 | 71 | +0 | -0 | identical-content |
| `PaintEngine/qpolygon.cpp` | 360 | 360 | +0 | -0 | identical-content |
| `PaintEngine/qpolygon.h` | 138 | 138 | +0 | -0 | identical-content |
| `PaintEngine/qtextoption.h` | 46 | 46 | +0 | -0 | identical-content |
| `PaintEngine/qtransform.cpp` | 1039 | 1039 | +0 | -0 | identical-content |
| `PaintEngine/qtransform.h` | 336 | 336 | +0 | -0 | identical-content |
| `PaintEngine/qvectorpath.cpp` | 88 | 88 | +0 | -0 | identical-content |
| `PaintEngine/qvectorpath.h` | 141 | 141 | +0 | -0 | identical-content |

### `ParaScriptBindings/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `ParaScriptBindings/IObjectScriptingInterface.cpp` | 93 | 93 | +0 | -0 | identical-content |
| `ParaScriptBindings/IObjectScriptingInterface.h` | 84 | 84 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScripting.cpp` | 518 | 517 | +1 | -2 | minor diff |
| `ParaScriptBindings/ParaScripting.h` | 9 | 9 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScripting2.cpp` | 476 | 477 | +1 | -0 | minor diff |
| `ParaScriptBindings/ParaScripting3.cpp` | 456 | 453 | +0 | -3 | minor diff |
| `ParaScriptBindings/ParaScripting4.cpp` | 285 | 285 | +1 | -1 | minor diff |
| `ParaScriptBindings/ParaScripting5.cpp` | 653 | 669 | +25 | -9 | moderate diff |
| `ParaScriptBindings/ParaScriptingAudio.cpp` | 632 | 632 | +125 | -125 | significant diff |
| `ParaScriptBindings/ParaScriptingAudio.h` | 392 | 392 | +44 | -44 | significant diff |
| `ParaScriptBindings/ParaScriptingBlockWorld.cpp` | 616 | 616 | +1 | -1 | minor diff |
| `ParaScriptBindings/ParaScriptingBlockWorld.h` | 192 | 193 | +2 | -1 | minor diff |
| `ParaScriptBindings/ParaScriptingBrowserManager.cpp` | 168 | 168 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingBrowserManager.h` | 95 | 95 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingCharacter.cpp` | 1410 | 1411 | +56 | -55 | moderate diff |
| `ParaScriptBindings/ParaScriptingCharacter.h` | 716 | 716 | +4 | -4 | minor diff |
| `ParaScriptBindings/ParaScriptingCommon.cpp` | 1015 | 948 | +20 | -87 | moderate diff |
| `ParaScriptBindings/ParaScriptingCommon.h` | 572 | 566 | +0 | -6 | minor diff |
| `ParaScriptBindings/ParaScriptingFlashPlayer.cpp` | 176 | 176 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingFlashPlayer.h` | 111 | 111 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingGUI.cpp` | 3045 | 3035 | +0 | -10 | minor diff |
| `ParaScriptBindings/ParaScriptingGUI.h` | 1116 | 1116 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingGlobal.cpp` | 2354 | 2186 | +110 | -278 | moderate diff |
| `ParaScriptBindings/ParaScriptingGlobal.h` | 756 | 752 | +4 | -8 | minor diff |
| `ParaScriptBindings/ParaScriptingGraphics.cpp` | 237 | 237 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingGraphics.h` | 100 | 100 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingHTMLBrowser.cpp` | 226 | 226 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingHTMLBrowser.h` | 57 | 57 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingIC.cpp` | 123 | 123 | +1 | -1 | minor diff |
| `ParaScriptBindings/ParaScriptingIC.h` | 34 | 34 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingIO.cpp` | 2147 | 2141 | +6 | -12 | minor diff |
| `ParaScriptBindings/ParaScriptingIO.h` | 794 | 788 | +4 | -10 | minor diff |
| `ParaScriptBindings/ParaScriptingIPC.cpp` | 249 | 249 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingIPC.h` | 117 | 117 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingMisc.cpp` | 304 | 304 | +5 | -5 | minor diff |
| `ParaScriptBindings/ParaScriptingMisc.h` | 187 | 187 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingMovie.cpp` | 333 | 332 | +18 | -19 | moderate diff |
| `ParaScriptBindings/ParaScriptingMovie.h` | 147 | 148 | +1 | -0 | minor diff |
| `ParaScriptBindings/ParaScriptingNPL.cpp` | 2146 | 2144 | +2 | -4 | minor diff |
| `ParaScriptBindings/ParaScriptingNPL.h` | 764 | 764 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingNetwork.cpp` | 748 | 748 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingNetwork.h` | 296 | 296 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingPainter.cpp` | 551 | 861 | +310 | -0 | significant diff |
| `ParaScriptBindings/ParaScriptingPainter.h` | 136 | 137 | +1 | -0 | minor diff |
| `ParaScriptBindings/ParaScriptingScene.cpp` | 3304 | 3316 | +31 | -19 | minor diff |
| `ParaScriptBindings/ParaScriptingScene.h` | 2078 | 2081 | +9 | -6 | minor diff |
| `ParaScriptBindings/ParaScriptingTerrain.cpp` | 880 | 880 | +3 | -3 | minor diff |
| `ParaScriptBindings/ParaScriptingTerrain.h` | 542 | 541 | +4 | -5 | minor diff |
| `ParaScriptBindings/ParaScriptingTest.cpp` | 88 | 88 | +0 | -0 | identical-content |
| `ParaScriptBindings/ParaScriptingWorld.cpp` | 741 | 741 | +1 | -1 | minor diff |
| `ParaScriptBindings/ParaScriptingWorld.h` | 574 | 574 | +0 | -0 | identical-content |

### `ParaXModel/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `ParaXModel/AnimTable.cpp` | 424 | 424 | +0 | -0 | identical-content |
| `ParaXModel/AnimTable.h` | 290 | 290 | +0 | -0 | identical-content |
| `ParaXModel/BoneAnimProvider.cpp` | 360 | 360 | +0 | -0 | identical-content |
| `ParaXModel/BoneAnimProvider.h` | 182 | 182 | +0 | -0 | identical-content |
| `ParaXModel/BoneChain.cpp` | 68 | 78 | +11 | -1 | moderate diff |
| `ParaXModel/BoneChain.h` | 35 | 35 | +1 | -1 | moderate diff |
| `ParaXModel/ColladaModelLoader.cpp` | 72 | 72 | +0 | -0 | identical-content |
| `ParaXModel/ColladaModelLoader.h` | 33 | 33 | +0 | -0 | identical-content |
| `ParaXModel/FBXMaterial.h` | 114 | 114 | +0 | -0 | identical-content |
| `ParaXModel/FBXModelInfo.cpp` | 122 | 122 | +0 | -0 | identical-content |
| `ParaXModel/FBXModelInfo.h` | 58 | 58 | +0 | -0 | identical-content |
| `ParaXModel/FBXParser.cpp` | 3537 | 3247 | +182 | -472 | moderate diff |
| `ParaXModel/FBXParser.h` | 138 | 135 | +25 | -28 | significant diff |
| `ParaXModel/GltfModel.cpp` | 1525 | 1525 | +4 | -4 | minor diff |
| `ParaXModel/GltfModel.h` | 355 | 355 | +0 | -0 | identical-content |
| `ParaXModel/MeshHeader.h` | 18 | 18 | +0 | -0 | identical-content |
| `ParaXModel/ModelRenderPass.cpp` | 681 | 657 | +75 | -99 | significant diff |
| `ParaXModel/ModelRenderPass.h` | 102 | 102 | +12 | -12 | significant diff |
| `ParaXModel/PLYParser.cpp` | 207 | 207 | +0 | -0 | identical-content |
| `ParaXModel/PLYParser.h` | 30 | 30 | +0 | -0 | identical-content |
| `ParaXModel/ParaVoxelModel.cpp` | 2072 | 2072 | +0 | -0 | identical-content |
| `ParaXModel/ParaVoxelModel.h` | 440 | 440 | +1 | -1 | minor diff |
| `ParaXModel/ParaXBone.cpp` | 1541 | 1541 | +0 | -0 | identical-content |
| `ParaXModel/ParaXBone.h` | 264 | 264 | +0 | -0 | identical-content |
| `ParaXModel/ParaXModel.cpp` | 2499 | 2499 | +0 | -0 | identical-content |
| `ParaXModel/ParaXModel.h` | 408 | 408 | +0 | -0 | identical-content |
| `ParaXModel/ParaXModelCommon.cpp` | 38 | 38 | +0 | -0 | identical-content |
| `ParaXModel/ParaXModelCommon.h` | 117 | 117 | +0 | -0 | identical-content |
| `ParaXModel/ParaXModelExporter.cpp` | 111 | 111 | +0 | -0 | identical-content |
| `ParaXModel/ParaXModelExporter.h` | 14 | 14 | +0 | -0 | identical-content |
| `ParaXModel/ParaXModelInstance.h` | 48 | 48 | +0 | -0 | identical-content |
| `ParaXModel/ParaXStaticModel.cpp` | 1026 | 1036 | +20 | -10 | minor diff |
| `ParaXModel/ParaXStaticModel.h` | 140 | 142 | +2 | -0 | minor diff |
| `ParaXModel/ParaXStaticModelRenderPass.cpp` | 27 | 27 | +0 | -0 | identical-content |
| `ParaXModel/ParaXStaticModelRenderPass.h` | 47 | 47 | +0 | -0 | identical-content |
| `ParaXModel/ParticleSystemRef.cpp` | 69 | 69 | +0 | -0 | identical-content |
| `ParaXModel/ParticleSystemRef.h` | 63 | 63 | +0 | -0 | identical-content |
| `ParaXModel/TextureAnim.cpp` | 58 | 45 | +0 | -13 | significant diff |
| `ParaXModel/TextureAnim.h` | 18 | 16 | +0 | -2 | moderate diff |
| `ParaXModel/XFileCharModelExporter.cpp` | 1202 | 1202 | +1 | -1 | minor diff |
| `ParaXModel/XFileCharModelExporter.h` | 186 | 186 | +0 | -0 | identical-content |
| `ParaXModel/XFileCharModelParser.cpp` | 1104 | 1104 | +0 | -0 | identical-content |
| `ParaXModel/XFileCharModelParser.h` | 88 | 88 | +0 | -0 | identical-content |
| `ParaXModel/XFileDataObject.cpp` | 1204 | 1204 | +0 | -0 | identical-content |
| `ParaXModel/XFileDataObject.h` | 95 | 95 | +0 | -0 | identical-content |
| `ParaXModel/XFileExporter.cpp` | 288 | 288 | +0 | -0 | identical-content |
| `ParaXModel/XFileExporter.h` | 56 | 56 | +0 | -0 | identical-content |
| `ParaXModel/XFileHelper.cpp` | 15 | 15 | +0 | -0 | identical-content |
| `ParaXModel/XFileHelper.h` | 144 | 144 | +0 | -0 | identical-content |
| `ParaXModel/XFileParser.cpp` | 1297 | 1300 | +16 | -13 | minor diff |
| `ParaXModel/XFileParser.h` | 122 | 122 | +0 | -0 | identical-content |
| `ParaXModel/XFileStaticModelParser.cpp` | 309 | 310 | +2 | -1 | minor diff |
| `ParaXModel/XFileStaticModelParser.h` | 26 | 26 | +0 | -0 | identical-content |
| `ParaXModel/animated.h` | 469 | 469 | +0 | -0 | identical-content |
| `ParaXModel/glTFModelExporter.cpp` | 2061 | 2529 | +703 | -235 | significant diff |
| `ParaXModel/glTFModelExporter.h` | 407 | 402 | +18 | -23 | moderate diff |
| `ParaXModel/modelheaders.h` | 400 | 400 | +0 | -0 | identical-content |
| `ParaXModel/particle.cpp` | 1188 | 1188 | +0 | -0 | identical-content |
| `ParaXModel/particle.h` | 292 | 292 | +0 | -0 | identical-content |

### `VoxelMesh/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `VoxelMesh/DataGrid.cpp` | 183 | 183 | +0 | -0 | identical-content |
| `VoxelMesh/DataGrid.h` | 162 | 162 | +0 | -0 | identical-content |
| `VoxelMesh/IsoSurfaceBuilder.cpp` | 374 | 374 | +0 | -0 | identical-content |
| `VoxelMesh/IsoSurfaceBuilder.h` | 193 | 193 | +0 | -0 | identical-content |
| `VoxelMesh/IsoSurfaceBuilderTables.h` | 297 | 297 | +0 | -0 | identical-content |
| `VoxelMesh/IsoSurfaceRenderable.cpp` | 200 | 203 | +10 | -7 | moderate diff |
| `VoxelMesh/IsoSurfaceRenderable.h` | 46 | 46 | +1 | -1 | minor diff |
| `VoxelMesh/MetaBall.cpp` | 95 | 95 | +0 | -0 | identical-content |
| `VoxelMesh/MetaBall.h` | 34 | 34 | +0 | -0 | identical-content |
| `VoxelMesh/MetaHeightmap.cpp` | 98 | 98 | +0 | -0 | identical-content |
| `VoxelMesh/MetaHeightmap.h` | 48 | 48 | +0 | -0 | identical-content |
| `VoxelMesh/MetaObject.h` | 43 | 43 | +0 | -0 | identical-content |
| `VoxelMesh/MetaWorldFragment.cpp` | 80 | 80 | +0 | -0 | identical-content |
| `VoxelMesh/MetaWorldFragment.h` | 83 | 83 | +0 | -0 | identical-content |
| `VoxelMesh/VoxelTerrainEntity.cpp` | 20 | 20 | +0 | -0 | identical-content |
| `VoxelMesh/VoxelTerrainEntity.h` | 47 | 47 | +0 | -0 | identical-content |
| `VoxelMesh/VoxelTerrainManager.cpp` | 45 | 45 | +0 | -0 | identical-content |
| `VoxelMesh/VoxelTerrainManager.h` | 42 | 42 | +0 | -0 | identical-content |

### `WebBrowser/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `WebBrowser/IBrowserMsgListener.h` | 14 | 15 | +1 | -0 | moderate diff |
| `WebBrowser/WBClientSite.cpp` | 412 | 412 | +0 | -0 | identical-content |
| `WebBrowser/WBClientSite.h` | 203 | 203 | +0 | -0 | identical-content |
| `WebBrowser/WBStorage.cpp` | 150 | 150 | +0 | -0 | identical-content |
| `WebBrowser/WBStorage.h` | 99 | 99 | +0 | -0 | identical-content |
| `WebBrowser/WebBrowser.cpp` | 571 | 571 | +0 | -0 | identical-content |
| `WebBrowser/WebBrowser.h` | 78 | 78 | +0 | -0 | identical-content |

### `WebSocket/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `WebSocket/ByteBuffer.cpp` | 386 | 386 | +0 | -0 | identical-content |
| `WebSocket/ByteBuffer.h` | 195 | 195 | +0 | -0 | identical-content |
| `WebSocket/README.md` | 71 | 71 | - | - | binary/non-code |
| `WebSocket/WebSocketCommon.h` | 39 | 39 | +0 | -0 | identical-content |
| `WebSocket/WebSocketFrame.cpp` | 68 | 68 | +0 | -0 | identical-content |
| `WebSocket/WebSocketFrame.h` | 99 | 99 | +0 | -0 | identical-content |
| `WebSocket/WebSocketReader.cpp` | 287 | 287 | +0 | -0 | identical-content |
| `WebSocket/WebSocketReader.h` | 56 | 56 | +0 | -0 | identical-content |
| `WebSocket/WebSocketWriter.cpp` | 185 | 184 | +1 | -2 | minor diff |
| `WebSocket/WebSocketWriter.h` | 51 | 51 | +0 | -0 | identical-content |

### `curllua/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `curllua/Lua-cURL-callback.c` | 113 | 113 | +0 | -0 | identical-content |
| `curllua/Lua-cURL-getinfo.c` | 145 | 145 | +0 | -0 | identical-content |
| `curllua/Lua-cURL-multi.c` | 266 | 266 | +0 | -0 | identical-content |
| `curllua/Lua-cURL-post.c` | 131 | 131 | +0 | -0 | identical-content |
| `curllua/Lua-cURL-setopt.c` | 305 | 305 | +0 | -0 | identical-content |
| `curllua/Lua-cURL-share.c` | 70 | 70 | +1 | -1 | minor diff |
| `curllua/Lua-cURL-share.h` | 9 | 9 | +0 | -0 | identical-content |
| `curllua/Lua-cURL.c` | 344 | 344 | +0 | -0 | identical-content |
| `curllua/Lua-cURL.h` | 125 | 125 | +0 | -0 | identical-content |
| `curllua/Lua-utility.c` | 71 | 71 | +0 | -0 | identical-content |
| `curllua/Lua-utility.h` | 38 | 38 | +0 | -0 | identical-content |
| `curllua/curl_error.cpp` | 31 | 31 | +0 | -0 | identical-content |
| `curllua/curl_error.h` | 14 | 14 | +0 | -0 | identical-content |

### `debugtools/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `debugtools/Profiler.cpp` | 237 | 237 | +0 | -0 | identical-content |
| `debugtools/Profiler.h` | 56 | 56 | +0 | -0 | identical-content |
| `debugtools/paradebug.cpp` | 305 | 304 | +1 | -2 | minor diff |
| `debugtools/paradebug.h` | 63 | 63 | +0 | -0 | identical-content |

### `dirmonitor/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `dirmonitor/basic_dir_monitor.hpp` | 103 | 103 | +0 | -0 | identical-content |
| `dirmonitor/dir_monitor.hpp` | 31 | 199 | +169 | -1 | significant diff |
| `dirmonitor/fsevents/basic_dir_monitor_service.hpp` | 153 | 153 | +15 | -15 | moderate diff |
| `dirmonitor/fsevents/dir_monitor_impl.hpp` | 241 | 241 | +1 | -1 | minor diff |
| `dirmonitor/inotify/basic_dir_monitor_service.hpp` | 158 | 158 | +0 | -0 | identical-content |
| `dirmonitor/inotify/dir_monitor_impl.hpp` | 211 | 211 | +1 | -1 | minor diff |
| `dirmonitor/kqueue/basic_dir_monitor_service.hpp` | 167 | 167 | +15 | -15 | moderate diff |
| `dirmonitor/kqueue/dir_monitor_impl.hpp` | 279 | 279 | +0 | -0 | identical-content |
| `dirmonitor/windows/basic_dir_monitor_service.hpp` | 355 | 355 | +0 | -0 | identical-content |
| `dirmonitor/windows/dir_monitor_impl.hpp` | 100 | 100 | +0 | -0 | identical-content |

### `doc/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `doc/Document.cmake` | 13 | 13 | +0 | -0 | identical-content |
| `doc/EventsReference.txt` | 60 | 60 | - | - | binary/non-code |
| `doc/InstallFiles.txt` | 123 | 123 | - | - | binary/non-code |
| `doc/NamespaceBind.txt` | 32 | 32 | - | - | binary/non-code |
| `doc/ReadMe.txt` | 57 | 57 | - | - | binary/non-code |
| `doc/bones.txt` | 53 | 53 | - | - | binary/non-code |
| `doc/bugs.txt` | 34 | 34 | - | - | binary/non-code |
| `doc/changes.txt` | 2828 | 2828 | - | - | binary/non-code |
| `doc/changes_andy.txt` | 76 | 76 | - | - | binary/non-code |
| `doc/changes_clayman.txt` | 13 | 13 | - | - | binary/non-code |
| `doc/changes_lixizhi.txt` | 4813 | 4813 | - | - | binary/non-code |
| `doc/changes_lwl.txt` | 313 | 313 | - | - | binary/non-code |
| `doc/changes_sunlingfeng.txt` | 173 | 173 | - | - | binary/non-code |
| `doc/character_script_template.txt` | 30 | 30 | - | - | binary/non-code |
| `doc/config.txt` | 12 | 12 | - | - | binary/non-code |
| `doc/copyright.txt` | 49 | 49 | - | - | binary/non-code |
| `doc/copyright_SDK.txt` | 56 | 56 | - | - | binary/non-code |
| `doc/copyright_product.txt` | 49 | 49 | - | - | binary/non-code |

### `flashplayer/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `flashplayer/!UNLOCK_KEY.txt` | 44 | 44 | - | - | binary/non-code |
| `flashplayer/contact.txt` | 2 | 2 | - | - | binary/non-code |
| `flashplayer/f_in_box/include/f_in_box.h` | 653 | 653 | +0 | -0 | identical-content |
| `flashplayer/history.txt` | 91 | 91 | - | - | binary/non-code |
| `flashplayer/license.txt` | 77 | 77 | - | - | binary/non-code |
| `flashplayer/purchase.txt` | 2 | 2 | - | - | binary/non-code |
| `flashplayer/readme.txt` | 113 | 113 | - | - | binary/non-code |

### `jabber/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `jabber/JC_Options.cpp` | 41 | 41 | +0 | -0 | identical-content |
| `jabber/JC_Options.h` | 104 | 104 | +0 | -0 | identical-content |
| `jabber/JabberEventBinding.cpp` | 187 | 187 | +0 | -0 | identical-content |
| `jabber/JabberEventBinding.h` | 171 | 171 | +0 | -0 | identical-content |
| `jabber/NPLJabberClient.cpp` | 1485 | 1485 | +0 | -0 | identical-content |
| `jabber/NPLJabberClient.h` | 594 | 594 | +0 | -0 | identical-content |
| `jabber/NPL_TCPConnection.cpp` | 354 | 354 | +0 | -0 | identical-content |
| `jabber/NPL_TCPConnection.h` | 138 | 138 | +2 | -2 | minor diff |
| `jabber/ParaRingBuffer.h` | 306 | 306 | +0 | -0 | identical-content |

### `math/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `math/AABBCollider.cpp` | 21 | 21 | +0 | -0 | identical-content |
| `math/AABBCollider.h` | 14 | 14 | +0 | -0 | identical-content |
| `math/Collider.cpp` | 23 | 23 | +0 | -0 | identical-content |
| `math/Collider.h` | 111 | 111 | +0 | -0 | identical-content |
| `math/FPU.h` | 139 | 141 | +3 | -1 | minor diff |
| `math/ParaAngle.h` | 131 | 131 | +0 | -0 | identical-content |
| `math/ParaAxisAlignedBox.cpp` | 499 | 499 | +0 | -0 | identical-content |
| `math/ParaAxisAlignedBox.h` | 352 | 352 | +0 | -0 | identical-content |
| `math/ParaColor.cpp` | 478 | 478 | +17 | -17 | moderate diff |
| `math/ParaColor.h` | 414 | 414 | +13 | -13 | moderate diff |
| `math/ParaDVector3.h` | 640 | 640 | +0 | -0 | identical-content |
| `math/ParaGlobals.h` | 10 | 10 | +0 | -0 | identical-content |
| `math/ParaLine.cpp` | 103 | 103 | +0 | -0 | identical-content |
| `math/ParaLine.h` | 349 | 349 | +0 | -0 | identical-content |
| `math/ParaMargins.cpp` | 14 | 14 | +0 | -0 | identical-content |
| `math/ParaMargins.h` | 449 | 449 | +0 | -0 | identical-content |
| `math/ParaMath.cpp` | 1313 | 1313 | +1 | -1 | minor diff |
| `math/ParaMath.h` | 725 | 725 | +0 | -0 | identical-content |
| `math/ParaMathMatrix.cpp` | 454 | 454 | +0 | -0 | identical-content |
| `math/ParaMathMatrix.h` | 58 | 58 | +0 | -0 | identical-content |
| `math/ParaMathMisc.cpp` | 56 | 56 | +0 | -0 | identical-content |
| `math/ParaMathMisc.h` | 6 | 6 | +0 | -0 | identical-content |
| `math/ParaMathUtility.h` | 49 | 49 | +0 | -0 | identical-content |
| `math/ParaMathVector.cpp` | 83 | 83 | +0 | -0 | identical-content |
| `math/ParaMathVector.h` | 22 | 22 | +0 | -0 | identical-content |
| `math/ParaMatrix3.cpp` | 1516 | 1516 | +0 | -0 | identical-content |
| `math/ParaMatrix3.h` | 265 | 265 | +0 | -0 | identical-content |
| `math/ParaMatrix4.cpp` | 390 | 390 | +0 | -0 | identical-content |
| `math/ParaMatrix4.h` | 514 | 514 | +0 | -0 | identical-content |
| `math/ParaNamespaces.h` | 95 | 95 | +0 | -0 | identical-content |
| `math/ParaPlane.cpp` | 220 | 220 | +0 | -0 | identical-content |
| `math/ParaPlane.h` | 170 | 170 | +0 | -0 | identical-content |
| `math/ParaPlaneBoundedVolume.h` | 128 | 128 | +0 | -0 | identical-content |
| `math/ParaPoint.cpp` | 15 | 15 | +0 | -0 | identical-content |
| `math/ParaPoint.h` | 376 | 376 | +0 | -0 | identical-content |
| `math/ParaQuaternion.cpp` | 752 | 752 | +0 | -0 | identical-content |
| `math/ParaQuaternion.h` | 230 | 230 | +2 | -2 | minor diff |
| `math/ParaRay.h` | 115 | 115 | +0 | -0 | identical-content |
| `math/ParaRect.cpp` | 578 | 578 | +0 | -0 | identical-content |
| `math/ParaRect.h` | 912 | 912 | +0 | -0 | identical-content |
| `math/ParaSize.cpp` | 65 | 65 | +0 | -0 | identical-content |
| `math/ParaSize.h` | 390 | 390 | +0 | -0 | identical-content |
| `math/ParaSphere.h` | 101 | 101 | +0 | -0 | identical-content |
| `math/ParaVector2.cpp` | 51 | 51 | +0 | -0 | identical-content |
| `math/ParaVector2.h` | 497 | 497 | +0 | -0 | identical-content |
| `math/ParaVector3.cpp` | 176 | 176 | +0 | -0 | identical-content |
| `math/ParaVector3.h` | 698 | 698 | +0 | -0 | identical-content |
| `math/ParaVector4.cpp` | 16 | 16 | +0 | -0 | identical-content |
| `math/ParaVector4.h` | 381 | 381 | +0 | -0 | identical-content |
| `math/ParaViewport.h` | 38 | 39 | +14 | -13 | significant diff |
| `math/ShapeAABB.cpp` | 437 | 437 | +0 | -0 | identical-content |
| `math/ShapeAABB.h` | 274 | 274 | +0 | -0 | identical-content |
| `math/ShapeCone.cpp` | 133 | 133 | +0 | -0 | identical-content |
| `math/ShapeCone.h` | 25 | 25 | +0 | -0 | identical-content |
| `math/ShapeFrustum.cpp` | 203 | 203 | +0 | -0 | identical-content |
| `math/ShapeFrustum.h` | 55 | 55 | +0 | -0 | identical-content |
| `math/ShapeOBB.cpp` | 265 | 265 | +0 | -0 | identical-content |
| `math/ShapeOBB.h` | 111 | 111 | +0 | -0 | identical-content |
| `math/ShapeRay.cpp` | 300 | 300 | +0 | -0 | identical-content |
| `math/ShapeRay.h` | 53 | 53 | +0 | -0 | identical-content |
| `math/ShapeSphere.cpp` | 60 | 60 | +0 | -0 | identical-content |
| `math/ShapeSphere.h` | 100 | 100 | +0 | -0 | identical-content |
| `math/asm_math.h` | 92 | 92 | +0 | -0 | identical-content |

### `mdxfile/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `mdxfile/Bone.cpp` | 1943 | 1943 | +0 | -0 | identical-content |
| `mdxfile/Bone.h` | 350 | 350 | +0 | -0 | identical-content |
| `mdxfile/GeoChunk.cpp` | 611 | 611 | +0 | -0 | identical-content |
| `mdxfile/GeoChunk.h` | 73 | 73 | +0 | -0 | identical-content |
| `mdxfile/Geoset.cpp` | 83 | 83 | +0 | -0 | identical-content |
| `mdxfile/Geoset.h` | 45 | 45 | +0 | -0 | identical-content |
| `mdxfile/GeosetAnim.cpp` | 120 | 120 | +0 | -0 | identical-content |
| `mdxfile/GeosetAnim.h` | 58 | 58 | +0 | -0 | identical-content |
| `mdxfile/MDXModel.cpp` | 1388 | 1388 | +0 | -0 | identical-content |
| `mdxfile/MDXModel.h` | 129 | 129 | +0 | -0 | identical-content |
| `mdxfile/Material.cpp` | 199 | 199 | +0 | -0 | identical-content |
| `mdxfile/Material.h` | 92 | 92 | +0 | -0 | identical-content |
| `mdxfile/ModelUtils.h` | 28 | 28 | +0 | -0 | identical-content |
| `mdxfile/ParaEngineXFileDef.h` | 345 | 345 | +0 | -0 | identical-content |
| `mdxfile/ParaXFileGUID.h` | 182 | 182 | +133 | -133 | significant diff |
| `mdxfile/SFmpq_static.h` | 31 | 31 | +0 | -0 | identical-content |
| `mdxfile/SFmpqapi.h` | 423 | 423 | +0 | -0 | identical-content |
| `mdxfile/Sequence.cpp` | 37 | 37 | +0 | -0 | identical-content |
| `mdxfile/Sequence.h` | 40 | 40 | +0 | -0 | identical-content |
| `mdxfile/Skeleton.cpp` | 219 | 219 | +0 | -0 | identical-content |
| `mdxfile/Skeleton.h` | 8 | 8 | +0 | -0 | identical-content |
| `mdxfile/Texture.cpp` | 161 | 161 | +0 | -0 | identical-content |
| `mdxfile/Texture.h` | 52 | 52 | +0 | -0 | identical-content |
| `mdxfile/Types.h` | 9 | 9 | +0 | -0 | identical-content |
| `mdxfile/Utility.cpp` | 1138 | 1138 | +0 | -0 | identical-content |
| `mdxfile/Utility.h` | 455 | 455 | +0 | -0 | identical-content |
| `mdxfile/blp.cpp` | 275 | 275 | +0 | -0 | identical-content |
| `mdxfile/blp.h` | 4 | 4 | +0 | -0 | identical-content |
| `mdxfile/imageUtils.h` | 9 | 9 | +0 | -0 | identical-content |
| `mdxfile/math.h` | 75 | 75 | +0 | -0 | identical-content |
| `mdxfile/math3d.cpp` | 60 | 60 | +0 | -0 | identical-content |
| `mdxfile/math3d.h` | 15 | 15 | +0 | -0 | identical-content |
| `mdxfile/mdx.cpp` | 106 | 106 | +0 | -0 | identical-content |
| `mdxfile/mdx.h` | 94 | 94 | +0 | -0 | identical-content |
| `mdxfile/mdxCustomVertex.h` | 14 | 14 | +0 | -0 | identical-content |
| `mdxfile/paraengineXtmp.h` | 59 | 59 | +0 | -0 | identical-content |
| `mdxfile/rmxfguid.h` | 223 | 223 | +0 | -0 | identical-content |
| `mdxfile/rmxftmpl.h` | 339 | 339 | +0 | -0 | identical-content |
| `mdxfile/utils.h` | 25 | 25 | +0 | -0 | identical-content |

### `physics/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `physics/ErrorStream.h` | 68 | 68 | +0 | -0 | identical-content |
| `physics/Stream.cpp` | 223 | 223 | +0 | -0 | identical-content |
| `physics/Stream.h` | 78 | 78 | +0 | -0 | identical-content |
| `physics/cooking.cpp` | 114 | 114 | +0 | -0 | identical-content |
| `physics/cooking.h` | 21 | 21 | +0 | -0 | identical-content |

### `protocol/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `protocol/pb.cpp` | 451 | 473 | +41 | -19 | moderate diff |
| `protocol/pb.h` | 36 | 36 | +0 | -0 | identical-content |

### `renderer/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `renderer/CMakeLists.txt` | 7 | 7 | - | - | binary/non-code |
| `renderer/EffectManager.cpp` | 3296 | 3263 | +490 | -523 | significant diff |
| `renderer/EffectManager.h` | 453 | 453 | +5 | -5 | minor diff |
| `renderer/ParaVertexBuffer.cpp` | 233 | 248 | +26 | -11 | moderate diff |
| `renderer/ParaVertexBuffer.h` | 94 | 86 | +4 | -12 | moderate diff |
| `renderer/ParaVertexBufferPool.cpp` | 259 | 258 | +2 | -3 | minor diff |
| `renderer/ParaVertexBufferPool.h` | 98 | 98 | +2 | -2 | minor diff |
| `renderer/RenderCore.h` | 79 | 82 | +14 | -11 | significant diff |
| `renderer/RenderCoreOpenGL.h` | 594 | 595 | +40 | -39 | moderate diff |
| `renderer/RenderDevice.cpp` | 47 | 47 | +0 | -0 | identical-content |
| `renderer/RenderDevice.h` | 198 | 36 | +0 | -162 | significant diff |
| `renderer/RenderDeviceDirectX.cpp` | 148 | 148 | +15 | -15 | significant diff |
| `renderer/RenderDeviceDirectX.h` | 45 | 45 | +4 | -4 | moderate diff |
| `renderer/SpriteRenderer.cpp` | 405 | 410 | +10 | -5 | minor diff |
| `renderer/SpriteRenderer.h` | 140 | 140 | +3 | -3 | minor diff |
| `renderer/SpriteRendererDirectX.cpp` | 109 | 113 | +5 | -1 | moderate diff |
| `renderer/SpriteRendererDirectX.h` | 44 | 44 | +0 | -0 | identical-content |
| `renderer/SpriteRendererOpenGL.cpp` | 336 | 345 | +25 | -16 | moderate diff |
| `renderer/SpriteRendererOpenGL.h` | 68 | 68 | +0 | -0 | identical-content |
| `renderer/VertexDeclaration.h` | 11 | 7 | +0 | -4 | significant diff |
| `renderer/VertexDeclarationOpenGL.cpp` | 156 | 155 | +19 | -20 | significant diff |
| `renderer/VertexDeclarationOpenGL.h` | 44 | 46 | +13 | -11 | significant diff |
| `renderer/VertexFVF.cpp` | 28 | 28 | +0 | -0 | identical-content |
| `renderer/VertexFVF.h` | 160 | 160 | +0 | -0 | identical-content |
| `renderer/effect_file.cpp` | 39 | 39 | +0 | -0 | identical-content |
| `renderer/effect_file.h` | 340 | 341 | +2 | -1 | minor diff |
| `renderer/effect_file_DirectX.cpp` | 1454 | 1426 | +10 | -38 | minor diff |
| `renderer/effect_file_DirectX.h` | 263 | 262 | +0 | -1 | minor diff |
| `renderer/effect_file_OpenGL.cpp` | 1267 | 1545 | +833 | -555 | significant diff |
| `renderer/effect_file_OpenGL.h` | 234 | 257 | +60 | -37 | significant diff |

### `res/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `res/character_script_template.txt` | 30 | 30 | - | - | binary/non-code |
| `res/paraworld.ico` | 120 | 183 | - | - | binary/non-code |

### `terrain/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `terrain/Brush.cpp` | 247 | 247 | +0 | -0 | identical-content |
| `terrain/Brush.h` | 32 | 32 | +0 | -0 | identical-content |
| `terrain/DetailTexture.cpp` | 368 | 381 | +30 | -17 | moderate diff |
| `terrain/DetailTexture.h` | 77 | 77 | +2 | -2 | moderate diff |
| `terrain/DetailTextureFactory.cpp` | 236 | 236 | +0 | -0 | identical-content |
| `terrain/DetailTextureFactory.h` | 128 | 128 | +0 | -0 | identical-content |
| `terrain/DynamicTerrainLoader.cpp` | 316 | 316 | +0 | -0 | identical-content |
| `terrain/DynamicTerrainLoader.h` | 60 | 60 | +0 | -0 | identical-content |
| `terrain/GlobalTerrain.cpp` | 1370 | 1394 | +25 | -1 | minor diff |
| `terrain/GlobalTerrain.h` | 692 | 692 | +1 | -1 | minor diff |
| `terrain/ImageLoader.h` | 11 | 11 | +0 | -0 | identical-content |
| `terrain/Loader.cpp` | 343 | 348 | +8 | -3 | minor diff |
| `terrain/Loader.h` | 61 | 61 | +0 | -0 | identical-content |
| `terrain/Settings.cpp` | 389 | 389 | +0 | -0 | identical-content |
| `terrain/Settings.h` | 181 | 181 | +0 | -0 | identical-content |
| `terrain/TTerrain.cpp` | 226 | 226 | +0 | -0 | identical-content |
| `terrain/TTerrain.h` | 138 | 138 | +0 | -0 | identical-content |
| `terrain/Terrain.cpp` | 4004 | 4021 | +905 | -888 | significant diff |
| `terrain/Terrain.h` | 1460 | 1462 | +302 | -300 | significant diff |
| `terrain/TerrainBlock.cpp` | 1154 | 1153 | +0 | -1 | minor diff |
| `terrain/TerrainBlock.h` | 173 | 174 | +1 | -0 | minor diff |
| `terrain/TerrainBuffer.cpp` | 322 | 328 | +7 | -1 | minor diff |
| `terrain/TerrainBuffer.h` | 71 | 73 | +2 | -0 | minor diff |
| `terrain/TerrainCommon.cpp` | 16 | 16 | +0 | -0 | identical-content |
| `terrain/TerrainCommon.h` | 2 | 2 | +0 | -0 | identical-content |
| `terrain/TerrainConfig.h` | 19 | 19 | +0 | -0 | identical-content |
| `terrain/TerrainException.h` | 30 | 30 | +0 | -0 | identical-content |
| `terrain/TerrainFilters.cpp` | 719 | 719 | +0 | -0 | identical-content |
| `terrain/TerrainFilters.h` | 141 | 141 | +0 | -0 | identical-content |
| `terrain/TerrainGeoMipmapIndices.cpp` | 917 | 917 | +0 | -0 | identical-content |
| `terrain/TerrainGeoMipmapIndices.h` | 61 | 61 | +0 | -0 | identical-content |
| `terrain/TerrainLattice.cpp` | 1574 | 1583 | +9 | -0 | minor diff |
| `terrain/TerrainLattice.h` | 402 | 403 | +1 | -0 | minor diff |
| `terrain/TerrainLoadListener.h` | 13 | 13 | +0 | -0 | identical-content |
| `terrain/TerrainRegions.cpp` | 293 | 296 | +7 | -4 | minor diff |
| `terrain/TerrainRegions.h` | 126 | 126 | +0 | -0 | identical-content |
| `terrain/TerrainVertex.cpp` | 16 | 16 | +0 | -0 | identical-content |
| `terrain/TerrainVertex.h` | 14 | 14 | +0 | -0 | identical-content |
| `terrain/Texture.cpp` | 324 | 326 | +2 | -0 | minor diff |
| `terrain/Texture.h` | 177 | 177 | +0 | -0 | identical-content |
| `terrain/TextureCell.cpp` | 505 | 508 | +5 | -2 | minor diff |
| `terrain/TextureCell.h` | 140 | 140 | +0 | -0 | identical-content |
| `terrain/TextureFactory.cpp` | 19 | 19 | +0 | -0 | identical-content |
| `terrain/TextureFactory.h` | 61 | 61 | +0 | -0 | identical-content |
| `terrain/TextureGenerator.cpp` | 21 | 21 | +0 | -0 | identical-content |
| `terrain/TextureGenerator.h` | 17 | 17 | +0 | -0 | identical-content |
| `terrain/TextureSet.cpp` | 294 | 294 | +0 | -0 | identical-content |
| `terrain/TextureSet.h` | 78 | 78 | +0 | -0 | identical-content |
| `terrain/Triangle.cpp` | 61 | 61 | +0 | -0 | identical-content |
| `terrain/Triangle.h` | 23 | 23 | +0 | -0 | identical-content |
| `terrain/TriangleFan.cpp` | 110 | 110 | +1 | -1 | minor diff |
| `terrain/TriangleFan.h` | 34 | 34 | +0 | -0 | identical-content |
| `terrain/TriangleStrip.cpp` | 207 | 207 | +1 | -1 | minor diff |
| `terrain/TriangleStrip.h` | 36 | 36 | +0 | -0 | identical-content |
| `terrain/WorldNameFactory.cpp` | 213 | 213 | +0 | -0 | identical-content |
| `terrain/WorldNameFactory.h` | 80 | 79 | +1 | -2 | minor diff |

### `util/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `util/CSingleton.h` | 41 | 41 | +1 | -1 | minor diff |
| `util/CommonFileDialog.cpp` | 198 | 188 | +3 | -13 | moderate diff |
| `util/CommonFileDialog.h` | 31 | 31 | +0 | -0 | identical-content |
| `util/ConvertUTF.c` | 590 | 590 | +1 | -1 | minor diff |
| `util/ConvertUTF.h` | 253 | 253 | +0 | -0 | identical-content |
| `util/ConvertUTFWrapper.cpp` | 146 | 146 | +1 | -1 | minor diff |
| `util/CopyOnWriteHolder.h` | 112 | 112 | +0 | -0 | identical-content |
| `util/CoroutineThread.h` | 189 | 189 | +1 | -1 | minor diff |
| `util/CyoDecode.cpp` | 374 | 374 | +0 | -0 | identical-content |
| `util/CyoDecode.h` | 45 | 45 | +0 | -0 | identical-content |
| `util/CyoEncode.cpp` | 280 | 280 | +0 | -0 | identical-content |
| `util/CyoEncode.h` | 45 | 45 | +0 | -0 | identical-content |
| `util/EnumProcess.hpp` | 296 | 296 | +0 | -0 | identical-content |
| `util/FastRandom.cpp` | 90 | 90 | +0 | -0 | identical-content |
| `util/FastRandom.h` | 34 | 34 | +0 | -0 | identical-content |
| `util/HttpUtility.cpp` | 261 | 261 | +0 | -0 | identical-content |
| `util/HttpUtility.h` | 88 | 88 | +0 | -0 | identical-content |
| `util/ICollection.h` | 384 | 384 | +0 | -0 | identical-content |
| `util/IObjectDrag.cpp` | 123 | 122 | +0 | -1 | minor diff |
| `util/IObjectDrag.h` | 112 | 112 | +0 | -0 | identical-content |
| `util/Log.cpp` | 553 | 587 | +48 | -14 | moderate diff |
| `util/Log.h` | 315 | 315 | +1 | -1 | minor diff |
| `util/LogService.cpp` | 109 | 109 | +0 | -0 | identical-content |
| `util/LogService.h` | 165 | 165 | +2 | -2 | minor diff |
| `util/MD5.cpp` | 416 | 416 | +0 | -0 | identical-content |
| `util/MD5.h` | 80 | 80 | +0 | -0 | identical-content |
| `util/MidiMsg.cpp` | 209 | 233 | +46 | -22 | significant diff |
| `util/MidiMsg.h` | 46 | 52 | +7 | -1 | moderate diff |
| `util/Mutex.hpp` | 220 | 220 | +0 | -0 | identical-content |
| `util/MyByteSwap.h` | 250 | 250 | +1 | -1 | minor diff |
| `util/NetworkAdapter.cpp` | 129 | 31 | +8 | -106 | significant diff |
| `util/NetworkAdapter.h` | 34 | 34 | +0 | -0 | identical-content |
| `util/ObjectManager.cpp` | 79 | 79 | +0 | -0 | identical-content |
| `util/ObjectManager.h` | 38 | 38 | +0 | -0 | identical-content |
| `util/ParaMemPool.cpp` | 75 | 75 | +0 | -0 | identical-content |
| `util/ParaMemPool.h` | 285 | 284 | +5 | -6 | minor diff |
| `util/ParaTime.cpp` | 338 | 338 | +0 | -0 | identical-content |
| `util/ParaTime.h` | 74 | 74 | +0 | -0 | identical-content |
| `util/PoolBase.h` | 105 | 105 | +0 | -0 | identical-content |
| `util/Report.cpp` | 99 | 99 | +0 | -0 | identical-content |
| `util/Report.h` | 79 | 79 | +0 | -0 | identical-content |
| `util/ScopedLock.h` | 24 | 24 | +0 | -0 | identical-content |
| `util/Semaphore.hpp` | 129 | 129 | +0 | -0 | identical-content |
| `util/SimpleTranslator.h` | 96 | 96 | +0 | -0 | identical-content |
| `util/StringBuilder.cpp` | 42 | 42 | +0 | -0 | identical-content |
| `util/StringBuilder.h` | 252 | 252 | +0 | -0 | identical-content |
| `util/StringBuilder.hpp` | 520 | 520 | +2 | -2 | minor diff |
| `util/StringHelper.cpp` | 1628 | 1548 | +15 | -95 | moderate diff |
| `util/StringHelper.h` | 312 | 312 | +0 | -0 | identical-content |
| `util/Thread.hpp` | 154 | 154 | +0 | -0 | identical-content |
| `util/Tick.cpp` | 50 | 50 | +0 | -0 | identical-content |
| `util/Tick.h` | 26 | 26 | +0 | -0 | identical-content |
| `util/Type.cpp` | 388 | 388 | +0 | -0 | identical-content |
| `util/Type.h` | 564 | 564 | +0 | -0 | identical-content |
| `util/bitlib_lua.c` | 197 | 197 | +0 | -0 | identical-content |
| `util/bitlib_lua.h` | 23 | 23 | +0 | -0 | identical-content |
| `util/cow_ptr.hpp` | 274 | 274 | +0 | -0 | identical-content |
| `util/dl_malloc.c` | 5705 | 5713 | +11 | -3 | minor diff |
| `util/dl_malloc.h` | 560 | 561 | +2 | -1 | minor diff |
| `util/dl_malloc_include.h` | 45 | 45 | +0 | -0 | identical-content |
| `util/fast_atof.h` | 335 | 335 | +0 | -0 | identical-content |
| `util/fixed_string.hpp` | 249 | 249 | +0 | -0 | identical-content |
| `util/intrusive_ptr.cpp` | 53 | 53 | +0 | -0 | identical-content |
| `util/intrusive_ptr.h` | 89 | 89 | +0 | -0 | identical-content |
| `util/keyboard.cpp` | 46 | 47 | +5 | -4 | moderate diff |
| `util/keyboard.h` | 28 | 28 | +0 | -0 | identical-content |
| `util/lua_pack.c` | 408 | 408 | +0 | -0 | identical-content |
| `util/lua_pack.h` | 31 | 31 | +0 | -0 | identical-content |
| `util/luafilesystem.c` | 834 | 834 | +0 | -0 | identical-content |
| `util/luafilesystem.h` | 17 | 17 | +0 | -0 | identical-content |
| `util/mutex.h` | 229 | 248 | +21 | -2 | moderate diff |
| `util/os_calls.cpp` | 522 | 530 | +36 | -28 | moderate diff |
| `util/os_calls.h` | 148 | 148 | +0 | -0 | identical-content |
| `util/regularexpression.h` | 97 | 97 | +0 | -0 | identical-content |
| `util/sha1.cpp` | 423 | 423 | +0 | -0 | identical-content |
| `util/sha1.h` | 93 | 93 | +0 | -0 | identical-content |
| `util/stack_containers.hpp` | 256 | 256 | +0 | -0 | identical-content |
| `util/stdint_win32.h` | 223 | 223 | +0 | -0 | identical-content |
| `util/tinyjson.hpp` | 579 | 579 | +0 | -0 | identical-content |
| `util/unordered_array.hpp` | 36 | 36 | +0 | -0 | identical-content |

### `zlib/`

| File | dev lines | cp-old lines | +added | -removed | Status |
|------|----------:|-------------:|-------:|---------:|--------|
| `zlib/zip.cpp` | 2861 | 2861 | +0 | -0 | identical-content |
| `zlib/zip.h` | 203 | 203 | +0 | -0 | identical-content |

## Section 3: Identical Files (Fully Ported)

These files are **byte-for-byte identical** in both branches.

<details><summary>Show all identical files</summary>

| File |
|------|
| `Core/NPLMiniRuntime.hpp` |
| `Core/ParaEngine.cxx` |
| `IO/SerialPort.h` |
| `NPL/NPLRuntimeState.cpp` |
| `NPL/NPLRuntimeState.h` |
| `NPL/NPLScriptingState.cpp` |
| `NPL/NPLScriptingState.h` |
| `NPL/sample.config.xml` |
| `PaintEngine/Painter.cpp` |
| `PaintEngine/Painter.h` |
| `PaintEngine/PainterState.cpp` |
| `PaintEngine/PainterState.h` |
| `ParaEngineClient.def` |
| `ParaEngineClientConfig.h.in` |
| `ParaXModel/ParaXmodel.templates` |
| `ParaXModel/XFileParsing.inl` |
| `doc/NPLDebugging` |
| `doc/ParaEngineBootStrapping` |
| `doc/ProgrammingIntro` |
| `doc/_ParaMain` |
| `doc/_ParaNPL` |
| `doc/_ParaTerrain` |
| `flashplayer/f_in_box/include/functions_decl.inl` |
| `flashplayer/f_in_box/include/messages.inl` |
| `flashplayer/f_in_box/include/notification_messages.inl` |
| `flashplayer/f_in_box/lib/f_in_box.lib` |
| `flashplayer/f_in_box/lib/vssver2.scc` |
| `math/ParaAngle.inl` |
| `math/ParaDVector3.inl` |
| `math/ParaVector3.inl` |
| `math/ParaVector4.inl` |
| `res/ParaEngine.manifest` |
| `res/ParaXmodel.templates` |
| `res/codec.dat` |
| `res/codec_wmv.dat` |
| `res/codec_wmvd.dat` |
| `res/codec_xvid.dat` |
| `res/codec_xvidd.dat` |
| `res/codecdb.dat` |
| `res/cursor.tga` |
| `res/db_character.sql` |
| `res/db_chest.sql` |
| `res/db_create_npc_table.sql` |
| `res/db_fruit.sql` |
| `res/db_item.sql` |
| `res/db_npc_procedures.sql` |
| `res/db_pet.sql` |
| `res/db_pet_AI.sql` |
| `res/db_puzzle.sql` |
| `res/db_quest_table.sql` |
| `res/db_scene.sql` |
| `res/db_string_table.sql` |
| `res/db_title.sql` |
| `res/directx.ico` |
| `res/kidsmovie.ico` |
| `res/npl.syntax` |

</details>

## Section 4: Files Only in cp-old (New Cross-Platform Code)

These files exist **only in cp-old** — they are new cross-platform additions
that do not exist in the dev branch.

### `2dengine/` (4 files)

| File | Lines |
|------|------:|
| `2dengine/GUIUniBufferDirectX.cpp` | 781 |
| `2dengine/GUIUniBufferDirectX.h` | 251 |
| `2dengine/GUIUniBufferOpenGL.cpp` | 699 |
| `2dengine/GUIUniBufferOpenGL.h` | 155 |

### `Core/` (8 files)

| File | Lines |
|------|------:|
| `Core/MainLoopBase.cpp` | 27 |
| `Core/MainLoopBase.h` | 37 |
| `Core/ParaEngineClientConfig.win32.h` | 84 |
| `Core/ParaEngineRenderBase.h` | 15 |
| `Core/ParaImage.cpp` | 1782 |
| `Core/ParaImage.h` | 151 |
| `Core/s3tc.cpp` | 194 |
| `Core/s3tc.h` | 49 |

### `Framework/` (44 files)

| File | Lines |
|------|------:|
| `Framework/Common/Helper/EditorHelper.cpp` | 78 |
| `Framework/Common/Helper/EditorHelper.h` | 82 |
| `Framework/Common/Math/Color4f.h` | 27 |
| `Framework/Common/Math/Math.h` | 2 |
| `Framework/Common/Math/Rect.h` | 16 |
| `Framework/Common/Math/Vector2.h` | 15 |
| `Framework/Common/Time/ParaTimer.cpp` | 92 |
| `Framework/Common/Time/ParaTimer.h` | 24 |
| `Framework/FileSystem/ParaFileUtils.cpp` | 283 |
| `Framework/FileSystem/ParaFileUtils.h` | 36 |
| `Framework/FileSystem/ReadFile.cpp` | 128 |
| `Framework/FileSystem/ReadFile.h` | 44 |
| `Framework/Interface/BaseInterface.h` | 61 |
| `Framework/Interface/IBatchedElementDraw.h` | 88 |
| `Framework/Interface/IEnvironmentSim.h` | 151 |
| `Framework/Interface/IEvent.h` | 26 |
| `Framework/Interface/IInfoCenter.h` | 11 |
| `Framework/Interface/IMediaPlayer.h` | 22 |
| `Framework/Interface/IMovieCodec.h` | 96 |
| `Framework/Interface/INPL.h` | 83 |
| `Framework/Interface/INPLAcitvationFile.h` | 66 |
| `Framework/Interface/INPLRuntime.h` | 397 |
| `Framework/Interface/INPLRuntimeState.h` | 177 |
| `Framework/Interface/INPLScriptingState.h` | 95 |
| `Framework/Interface/IParaAudioEngine.h` | 571 |
| `Framework/Interface/IParaDebugDraw.h` | 74 |
| `Framework/Interface/IParaEngineApp.h` | 533 |
| `Framework/Interface/IParaEngineCore.h` | 43 |
| `Framework/Interface/IParaPhysics.h` | 418 |
| `Framework/Interface/IParaWebPlayer.h` | 104 |
| `Framework/Interface/IParaWebService.h` | 570 |
| `Framework/Interface/IParaWebView.cpp` | 35 |
| `Framework/Interface/IParaWebView.h` | 57 |
| `Framework/Interface/IParaWebXR.h` | 114 |
| `Framework/Interface/Render/IEffect.h` | 0 |
| `Framework/Interface/Render/IIndexBuffer.h` | 5 |
| `Framework/Interface/Render/IRenderContext.h` | 23 |
| `Framework/Interface/Render/IRenderDevice.h` | 113 |
| `Framework/Interface/Render/IRenderWindow.h` | 19 |
| `Framework/Interface/Render/ISurface.h` | 20 |
| `Framework/Interface/Render/ITexture.h` | 21 |
| `Framework/Interface/Render/IVertexBuffer.h` | 5 |
| `Framework/RenderSystem/RenderConfig.h` | 32 |
| `Framework/RenderSystem/RenderTypes.h` | 375 |

### `IO/` (3 files)

| File | Lines |
|------|------:|
| `IO/FileManager.h.orig` | 131 |
| `IO/ReadFileBoost.cpp` | 130 |
| `IO/ReadFileBoost.h` | 44 |

### `InfoCenter/` (9 files)

| File | Lines |
|------|------:|
| `InfoCenter/ICConfigManager.cpp` | 745 |
| `InfoCenter/ICConfigManager.h` | 232 |
| `InfoCenter/ICDBManager.cpp` | 961 |
| `InfoCenter/ICDBManager.h` | 334 |
| `InfoCenter/ICRecordSet.cpp` | 509 |
| `InfoCenter/ICRecordSet.h` | 126 |
| `InfoCenter/ParaDatabase.h` | 18 |
| `InfoCenter/SQLStatement.cpp` | 188 |
| `InfoCenter/SQLStatement.h` | 89 |

### `OpenGLWrapper/` (23 files)

| File | Lines |
|------|------:|
| `OpenGLWrapper/GLFont.h` | 19 |
| `OpenGLWrapper/GLFontAtlas.cpp` | 463 |
| `OpenGLWrapper/GLFontAtlas.h` | 105 |
| `OpenGLWrapper/GLFontAtlasCache.cpp` | 127 |
| `OpenGLWrapper/GLFontAtlasCache.h` | 26 |
| `OpenGLWrapper/GLFontFreeType.cpp` | 654 |
| `OpenGLWrapper/GLFontFreeType.h` | 81 |
| `OpenGLWrapper/GLImage.cpp` | 6 |
| `OpenGLWrapper/GLImage.h` | 23 |
| `OpenGLWrapper/GLLabel.cpp` | 715 |
| `OpenGLWrapper/GLLabel.h` | 213 |
| `OpenGLWrapper/GLProgram.cpp` | 929 |
| `OpenGLWrapper/GLProgram.cpp.orig` | 800 |
| `OpenGLWrapper/GLProgram.h` | 297 |
| `OpenGLWrapper/GLProgramCache.cpp` | 374 |
| `OpenGLWrapper/GLProgramCache.h` | 71 |
| `OpenGLWrapper/GLTexture2D.cpp` | 540 |
| `OpenGLWrapper/GLTexture2D.h` | 138 |
| `OpenGLWrapper/GLType.cpp` | 568 |
| `OpenGLWrapper/GLType.h` | 340 |
| `OpenGLWrapper/edtaa3func.cpp` | 521 |
| `OpenGLWrapper/edtaa3func.h` | 94 |
| `OpenGLWrapper/uthash.h` | 940 |

### `ParaScriptBindings/` (2 files)

| File | Lines |
|------|------:|
| `ParaScriptBindings/ParaScriptingWebView.cpp` | 100 |
| `ParaScriptBindings/ParaScriptingWebView.h` | 34 |

### `d3dcommon/` (3 files)

| File | Lines |
|------|------:|
| `d3dcommon/dxstdafx.h` | 72 |
| `d3dcommon/dxutil.cpp` | 1359 |
| `d3dcommon/dxutil.h` | 181 |

### `shaders/` (48 files)

| File | Lines |
|------|------:|
| `shaders/d3d9_spec/BMaxModel.fx` | 108 |
| `shaders/d3d9_spec/GUI_simple.fx` | 64 |
| `shaders/d3d9_spec/blockEffect.fx` | 211 |
| `shaders/d3d9_spec/common_shader_data.h` | 37 |
| `shaders/d3d9_spec/dropShadow.fx` | 51 |
| `shaders/d3d9_spec/fullscreen_glow.fx` | 423 |
| `shaders/d3d9_spec/light_scattering_constants.h` | 124 |
| `shaders/d3d9_spec/occlusion_test.fx` | 69 |
| `shaders/d3d9_spec/ocean_water.fx` | 360 |
| `shaders/d3d9_spec/ocean_water_cloud.fx` | 171 |
| `shaders/d3d9_spec/ocean_water_high.fx` | 288 |
| `shaders/d3d9_spec/ocean_water_quad.fx` | 98 |
| `shaders/d3d9_spec/patch_deform.h` | 97 |
| `shaders/d3d9_spec/screenWave.fx` | 82 |
| `shaders/d3d9_spec/shadowmap_blur.fx` | 152 |
| `shaders/d3d9_spec/simple_VS.fx` | 24 |
| `shaders/d3d9_spec/simple_cad_model.fx` | 105 |
| `shaders/d3d9_spec/simple_mesh.fx` | 149 |
| `shaders/d3d9_spec/simple_mesh_normal.fx` | 273 |
| `shaders/d3d9_spec/simple_mesh_normal_border.fx` | 87 |
| `shaders/d3d9_spec/simple_mesh_normal_instanced.fx` | 211 |
| `shaders/d3d9_spec/simple_mesh_normal_low.fx` | 222 |
| `shaders/d3d9_spec/simple_mesh_normal_selected.fx` | 176 |
| `shaders/d3d9_spec/simple_mesh_normal_shadow.fx` | 230 |
| `shaders/d3d9_spec/simple_mesh_normal_specialeffects.fx` | 170 |
| `shaders/d3d9_spec/simple_mesh_normal_tex2.fx` | 247 |
| `shaders/d3d9_spec/simple_mesh_normal_unlit.fx` | 167 |
| `shaders/d3d9_spec/simple_mesh_normal_vegetation.fx` | 247 |
| `shaders/d3d9_spec/simple_mesh_normal_vegetation.h` | 18 |
| `shaders/d3d9_spec/simple_particle.fx` | 85 |
| `shaders/d3d9_spec/sky.fx` | 144 |
| `shaders/d3d9_spec/skydome.fx` | 198 |
| `shaders/d3d9_spec/smesh_ctor.fx` | 165 |
| `shaders/d3d9_spec/terrain_normal.fx` | 452 |
| `shaders/d3d9_spec/terrain_normal_depth_shadowmap.fx` | 578 |
| `shaders/d3d9_spec/water_ripples.fx` | 86 |
| `shaders/opengl_spec/BMaxModel.fx` | 232 |
| `shaders/opengl_spec/GuiEffect.fx` | 64 |
| `shaders/opengl_spec/GuiTextEffect.fx` | 65 |
| `shaders/opengl_spec/RedBlueStereo.fx` | 72 |
| `shaders/opengl_spec/blockEffect.fx` | 380 |
| `shaders/opengl_spec/simple_mesh_normal.fx` | 94 |
| `shaders/opengl_spec/simple_particle.fx` | 85 |
| `shaders/opengl_spec/singleColorEffect.fx` | 40 |
| `shaders/opengl_spec/sky.fx` | 144 |
| `shaders/opengl_spec/skydome.fx` | 161 |
| `shaders/opengl_spec/terrain_normal.fx` | 130 |
| `shaders/opengl_spec/unlit_texture.fx` | 47 |

### `util/` (1 files)

| File | Lines |
|------|------:|
| `util/AutoPtr.h` | 70 |

## Legend

| Status | Count | Meaning |
|--------|------:|---------|
| `identical-content` | 723 | Effectively identical (CRLF-only or whitespace diffs) |
| `identical` | 15 | Byte-for-byte same in both branches |
| `minor diff` | 229 | <5% of lines changed |
| `moderate diff` | 112 | 5-20% of lines changed |
| `significant diff` | 99 | >20% of lines changed |
| `binary/non-code` | 32 | Non-text file or unsupported extension |