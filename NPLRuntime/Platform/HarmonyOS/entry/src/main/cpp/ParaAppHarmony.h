//-----------------------------------------------------------------------------
// ParaAppHarmony.h - HarmonyOS Version
// Authors: YuanQuanwei, big
// CreateDate: 2018.3.29
// ModifyDate: 2025.8.4 (Migrated to HarmonyOS)
//-----------------------------------------------------------------------------

#pragma once

#include "ParaEngine.h"
#include "ParaEngineSettings.h"
#include "util/os_calls.h"
#include "util/StringHelper.h"
#include "ObjectAutoReleasePool.h"
#include "AttributesManager.h"
#include "PluginManager.h"
#include "MeshPhysicsObject.h"
#include "MeshObject.h"
#include "MissileObject.h"
#include "BipedObject.h"
#include "BMaxModel/BMaxObject.h"
#include "SkyMesh.h"
#include "BlockPieceParticle.h"
#include "ContainerObject.h"
#include "RenderTarget.h"
#include "WeatherEffect.h"
#include "OverlayObject.h"
#include "LightObject.h"
#include "NPLRuntime.h"
#include "EventsCenter.h"
#include "BootStrapper.h"
#include "NPL/NPLHelper.h"
#include "AISimulator.h"
#include "AsyncLoader.h"
#include "FileManager.h"
#include "Archive.h"
#include "NPLPackageConfig.h"
#include "IO/ResourceEmbedded.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIMouseVirtual.h"
#include "FrameRateController.h"
#include "FileLogger.h"
#include "Render/IRenderContext.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICConfigManager.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "terrain/Terrain.h"
#include "ViewportManager.h"
#include "MoviePlatform.h"
#include "ParaEngineAppBase.h"
#include <rawfile/raw_file_manager.h>

namespace ParaEngine {

    /**
     * ParaEngine应用程序类 - HarmonyOS版本
     * 负责管理ParaEngine的核心功能和生命周期
     */
    class CParaEngineAppHarmony : public CParaEngineAppBase {
    public:
        CParaEngineAppHarmony();
        virtual ~CParaEngineAppHarmony();

        // 应用程序生命周期
        bool StartApp();
        void StopApp();
        void OnPause();
        void OnResume();

        // 渲染相关
        void OnSurfaceCreated();
        void OnSurfaceChanged(int width, int height);
        void OnSurfaceDestroyed();
        void OnDrawFrame();

        // 输入处理
        void OnKeyEvent(EVirtualKey key, bool bPressed);
        void OnTouchEvent(int action, float x, float y, int pointerId = 0);
        void OnMotionEvent(float x, float y);

        // 初始化和清理
        bool Init();
        void Cleanup();

        // 获取应用状态
        bool IsAppStarted() const { return m_bAppStarted; }
        bool IsPaused() const { return m_bPaused; }

        // 设置资源管理器
        void SetRawFileManager(NativeResourceManager* manager) { m_rawFileManager = manager; }
        NativeResourceManager* GetRawFileManager() const { return m_rawFileManager; }

        // 获取应用目录
        const std::string& GetWritableDir() const { return m_writableDir; }
        void SetWritableDir(const std::string& dir) { m_writableDir = dir; }

        const std::string& GetResourceDir() const { return m_resourceDir; }
        void SetResourceDir(const std::string& dir) { m_resourceDir = dir; }

    private:
        // 应用状态
        bool m_bAppStarted;
        bool m_bPaused;
        bool m_bSurfaceCreated;

        // 目录路径
        std::string m_writableDir;
        std::string m_resourceDir;

        // HarmonyOS资源管理器
        NativeResourceManager* m_rawFileManager;

        // 渲染相关
        int m_screenWidth;
        int m_screenHeight;

        // 初始化各个子系统
        bool InitParaEngine();
        bool InitFileSystem();
        bool InitRenderer();
        bool InitScript();
        bool InitGUI();
        bool InitAssets();

        // 清理各个子系统
        void CleanupParaEngine();
        void CleanupFileSystem();
        void CleanupRenderer();
        void CleanupScript();
        void CleanupGUI();
        void CleanupAssets();

        // 帧更新
        void UpdateFrame();
        void RenderFrame();

        // 输入处理辅助函数
        void HandleKeyInput(EVirtualKey key, bool bPressed);
        void HandleTouchInput(int action, float x, float y, int pointerId);
        void HandleMouseInput(float x, float y);

        // 设置默认参数
        void SetDefaultParameters();

        // 日志输出
        void OutputLog(const char* format, ...);
    };

} // namespace ParaEngine
