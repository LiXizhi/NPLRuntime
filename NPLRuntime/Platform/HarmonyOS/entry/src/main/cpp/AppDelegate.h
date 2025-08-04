//----------------------------------------------------------------------
// AppDelegate.h - HarmonyOS Version
// Authors: YuanQuanwei, LanZhihong, LiXizhi, big
// Company: ParaEngine
// CreateDate: 2018.3.29
// ModifyDate: 2025.8.4 (Migrated to HarmonyOS)
//-----------------------------------------------------------------------

#pragma once

// #include "ParaEngine.h"

namespace ParaEngine {
    // Forward declarations
    class CParaAppHarmony;
    
    /**
     * 应用委托类 - HarmonyOS版本
     * 负责处理应用生命周期事件和输入事件
     */
    class AppDelegate {
    public:
        AppDelegate();
        virtual ~AppDelegate();
        
        static AppDelegate* getInstance();
        
        // 应用生命周期方法
        virtual bool applicationDidFinishLaunching();
        virtual void applicationDidEnterBackground();
        virtual void applicationWillEnterForeground();
        virtual void applicationWillTerminate();
        
        // 渲染表面事件
        void onSurfaceCreated();
        void onSurfaceChanged(int width, int height);
        void onSurfaceDestroyed();
        void onDrawFrame();
        
        // 输入事件
        void onKeyEvent(int keyCode, int action);
        void onTouchEvent(int action, float x, float y, int pointerId = 0);
        void onMotionEvent(float x, float y);
        
        // 获取ParaApp实例
        CParaAppHarmony* getParaApp() const { return m_pParaApp; }
        
    private:
        static AppDelegate* s_pSharedApplication;
        CParaAppHarmony* m_pParaApp;
        
        // 尺寸变化处理
        struct {
            int width;
            int height;
        } m_newSize;
        bool m_bHasNewSize;
        bool m_bInMainLoop;
    };
    
} // namespace ParaEngine
