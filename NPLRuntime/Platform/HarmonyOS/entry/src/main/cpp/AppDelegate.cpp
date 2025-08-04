//----------------------------------------------------------------------
// AppDelegate.cpp - HarmonyOS Version
// Authors: YuanQuanwei, LanZhihong, LiXizhi, big
// Company: ParaEngine
// CreateDate: 2018.3.29
// ModifyDate: 2025.8.4 (Migrated to HarmonyOS)
//-----------------------------------------------------------------------

// #include "ParaEngine.h"
#include "AppDelegate.h"
// #include "ParaAppHarmony.h"
// #include "RenderWindowHarmony.h"
// #include "RenderDeviceOpenHarmony.h"
// #include "RenderContextOpenHarmony.h"
// #include "ParaTime.h"
// #include "NPLRuntime.h"
#include "napi/ParaEngineHelper.h"

// #include <boost/bind.hpp>
#include <hilog/log.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <cassert>
#include <ctime>
#include <unordered_map>
#include <rawfile/raw_file_manager.h>

namespace ParaEngine {
    
    // HarmonyOS按键映射（替代Android的AKEYCODE_*）
    // TODO: Implement key mapping when ParaEngine is available
    // inline EVirtualKey toVirtualKey(int32_t keycode) {
    //     static std::unordered_map<int32_t, EVirtualKey> s_keymap;
    //     if (s_keymap.size() == 0) {
    //         // HarmonyOS按键码映射
    //         s_keymap[0] = EVirtualKey::KEY_UNKNOWN;
    //         s_keymap[1] = EVirtualKey::KEY_HOME;
    //         s_keymap[2] = EVirtualKey::KEY_ESCAPE; // 返回键映射为ESC
    //         s_keymap[7] = EVirtualKey::KEY_0;
    //         s_keymap[8] = EVirtualKey::KEY_1;
    //         s_keymap[9] = EVirtualKey::KEY_2;
    //         s_keymap[10] = EVirtualKey::KEY_3;
    //         s_keymap[11] = EVirtualKey::KEY_4;
    //         s_keymap[12] = EVirtualKey::KEY_5;
    //         s_keymap[13] = EVirtualKey::KEY_6;
    //         s_keymap[14] = EVirtualKey::KEY_7;
    //         s_keymap[15] = EVirtualKey::KEY_8;
    //         s_keymap[16] = EVirtualKey::KEY_9;
    //         
    //         // 字母键映射
    //         s_keymap[29] = EVirtualKey::KEY_A;
    //         s_keymap[30] = EVirtualKey::KEY_B;
    //         s_keymap[31] = EVirtualKey::KEY_C;
    //         s_keymap[32] = EVirtualKey::KEY_D;
    //         s_keymap[33] = EVirtualKey::KEY_E;
    //         s_keymap[34] = EVirtualKey::KEY_F;
    //         s_keymap[35] = EVirtualKey::KEY_G;
    //         s_keymap[36] = EVirtualKey::KEY_H;
    //         s_keymap[37] = EVirtualKey::KEY_I;
    //         s_keymap[38] = EVirtualKey::KEY_J;
    //         s_keymap[39] = EVirtualKey::KEY_K;
    //         s_keymap[40] = EVirtualKey::KEY_L;
    //         s_keymap[41] = EVirtualKey::KEY_M;
    //         s_keymap[42] = EVirtualKey::KEY_N;
    //         s_keymap[43] = EVirtualKey::KEY_O;
    //         s_keymap[44] = EVirtualKey::KEY_P;
    //         s_keymap[45] = EVirtualKey::KEY_Q;
    //         s_keymap[46] = EVirtualKey::KEY_R;
    //         s_keymap[47] = EVirtualKey::KEY_S;
    //         s_keymap[48] = EVirtualKey::KEY_T;
    //         s_keymap[49] = EVirtualKey::KEY_U;
    //         s_keymap[50] = EVirtualKey::KEY_V;
    //         s_keymap[51] = EVirtualKey::KEY_W;
    //         s_keymap[52] = EVirtualKey::KEY_X;
    //         s_keymap[53] = EVirtualKey::KEY_Y;
    //         s_keymap[54] = EVirtualKey::KEY_Z;
    //         
    //         // 方向键
    //         s_keymap[19] = EVirtualKey::KEY_UP;
    //         s_keymap[20] = EVirtualKey::KEY_DOWN;
    //         s_keymap[21] = EVirtualKey::KEY_LEFT;
    //         s_keymap[22] = EVirtualKey::KEY_RIGHT;
    //         
    //         // 功能键
    //         s_keymap[62] = EVirtualKey::KEY_SPACE;
    //         s_keymap[66] = EVirtualKey::KEY_RETURN;
    //         s_keymap[67] = EVirtualKey::KEY_DELETE;
    //         s_keymap[59] = EVirtualKey::KEY_LSHIFT;
    //         s_keymap[60] = EVirtualKey::KEY_RSHIFT;
    //         s_keymap[113] = EVirtualKey::KEY_LCONTROL;
    //         s_keymap[114] = EVirtualKey::KEY_RCONTROL;
    //     }
    //     
    //     auto it = s_keymap.find(keycode);
    //     return (it != s_keymap.end()) ? it->second : EVirtualKey::KEY_UNKNOWN;
    // }

    AppDelegate* AppDelegate::s_pSharedApplication = nullptr;

    AppDelegate::AppDelegate() 
        : m_pParaApp(nullptr), m_bHasNewSize(false), m_bInMainLoop(false) {
        // ParaEngine::CParaEngine::StaticInit();
        s_pSharedApplication = this;
    }

    AppDelegate::~AppDelegate() {
        if (m_pParaApp) {
            // delete m_pParaApp;
            m_pParaApp = nullptr;
        }
        
        // ParaEngine::CParaEngine::StaticExit();
        s_pSharedApplication = nullptr;
    }

    AppDelegate* AppDelegate::getInstance() {
        if (!s_pSharedApplication) {
            s_pSharedApplication = new AppDelegate();
        }
        return s_pSharedApplication;
    }

    bool AppDelegate::applicationDidFinishLaunching() {
        OH_LOG_INFO(LOG_APP, "ParaEngine: Application did finish launching");
        
        // TODO: Initialize ParaEngine when core classes are available
        // if (!m_pParaApp) {
        //     m_pParaApp = new CParaAppHarmony();
        // }
        // return m_pParaApp->StartApp();
        
        return true;
    }

    void AppDelegate::applicationDidEnterBackground() {
        OH_LOG_INFO(LOG_APP, "ParaEngine: Application did enter background");
        
        // TODO: Handle pause when ParaEngine is available
        // if (m_pParaApp) {
        //     m_pParaApp->OnPause();
        // }
    }

    void AppDelegate::applicationWillEnterForeground() {
        OH_LOG_INFO(LOG_APP, "ParaEngine: Application will enter foreground");
        
        // TODO: Handle resume when ParaEngine is available
        // if (m_pParaApp) {
        //     m_pParaApp->OnResume();
        // }
    }

    void AppDelegate::applicationWillTerminate() {
        OH_LOG_INFO(LOG_APP, "ParaEngine: Application will terminate");
        
        // TODO: Handle termination when ParaEngine is available
        // if (m_pParaApp) {
        //     m_pParaApp->StopApp();
        // }
    }

    void AppDelegate::onSurfaceCreated() {
        OH_LOG_INFO(LOG_APP, "ParaEngine: Surface created");
        
        // TODO: Handle surface creation when ParaEngine is available
        // if (m_pParaApp) {
        //     m_pParaApp->OnSurfaceCreated();
        // }
    }

    void AppDelegate::onSurfaceChanged(int width, int height) {
        OH_LOG_INFO(LOG_APP, "ParaEngine: Surface changed %d x %d", width, height);
        
        m_newSize.width = width;
        m_newSize.height = height;
        m_bHasNewSize = true;
        
        // TODO: Handle surface change when ParaEngine is available
        // if (m_pParaApp && !m_bInMainLoop) {
        //     m_pParaApp->OnSizeChanged(width, height);
        // }
    }

    void AppDelegate::onSurfaceDestroyed() {
        OH_LOG_INFO(LOG_APP, "ParaEngine: Surface destroyed");
        
        // TODO: Handle surface destruction when ParaEngine is available
        // if (m_pParaApp) {
        //     m_pParaApp->OnSurfaceDestroyed();
        // }
    }

    void AppDelegate::onDrawFrame() {
        // TODO: Handle frame drawing when ParaEngine is available
        // if (m_bHasNewSize && m_pParaApp) {
        //     m_pParaApp->OnSizeChanged(m_newSize.width, m_newSize.height);
        //     m_bHasNewSize = false;
        // }
        // 
        // if (m_pParaApp) {
        //     m_bInMainLoop = true;
        //     m_pParaApp->OnDrawFrame();
        //     m_bInMainLoop = false;
        // }
    }

    void AppDelegate::onKeyEvent(int keyCode, int action) {
        // TODO: Handle key events when ParaEngine is available
        // if (m_pParaApp) {
        //     EVirtualKey vKey = toVirtualKey(keyCode);
        //     bool bPressed = (action == 0); // 0 = KEY_DOWN, 1 = KEY_UP
        //     m_pParaApp->OnKeyEvent(vKey, bPressed);
        // }
    }

    void AppDelegate::onTouchEvent(int action, float x, float y, int pointerId) {
        // TODO: Handle touch events when ParaEngine is available
        // if (m_pParaApp) {
        //     m_pParaApp->OnTouchEvent(action, x, y, pointerId);
        // }
    }

    void AppDelegate::onMotionEvent(float x, float y) {
        // TODO: Handle motion events when ParaEngine is available
        // if (m_pParaApp) {
        //     m_pParaApp->OnMotionEvent(x, y);
        // }
    }

} // namespace ParaEngine
