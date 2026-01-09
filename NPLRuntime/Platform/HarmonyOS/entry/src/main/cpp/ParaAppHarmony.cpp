#include "ParaEngine.h"
#include "ParaAppHarmony.h"

using namespace ParaEngine;

// Create ParaEngine application instance for HarmonyOS
IParaEngineApp* CreateParaEngineApp()
{
    return new CParaEngineAppHarmony();
}

namespace ParaEngine {

    CParaEngineAppHarmony::CParaEngineAppHarmony() 
        : m_bAppStarted(false)
        , m_bPaused(false)
        , m_bSurfaceCreated(false)
        , m_rawFileManager(nullptr)
        , m_screenWidth(0)
        , m_screenHeight(0)
    {
        SetTouchInputting(true);
    }

    CParaEngineAppHarmony::~CParaEngineAppHarmony() {
        Cleanup();
    }

    bool CParaEngineAppHarmony::StartApp() {
        // TODO: Implement app startup
        m_bAppStarted = true;
        return true;
    }

    void CParaEngineAppHarmony::StopApp() {
        // TODO: Implement app stop
        m_bAppStarted = false;
    }

    void CParaEngineAppHarmony::OnPause() {
        m_bPaused = true;
    }

    void CParaEngineAppHarmony::OnResume() {
        m_bPaused = false;
    }

    void CParaEngineAppHarmony::OnSurfaceCreated() {
        m_bSurfaceCreated = true;
    }

    void CParaEngineAppHarmony::OnSurfaceChanged(int width, int height) {
        m_screenWidth = width;
        m_screenHeight = height;
    }

    void CParaEngineAppHarmony::OnSurfaceDestroyed() {
        m_bSurfaceCreated = false;
    }

    void CParaEngineAppHarmony::OnDrawFrame() {
        // TODO: Implement frame drawing
    }

    void CParaEngineAppHarmony::OnKeyEvent(EVirtualKey key, bool bPressed) {
        // TODO: Implement key event handling
    }

    void CParaEngineAppHarmony::OnTouchEvent(int action, float x, float y, int pointerId) {
        // TODO: Implement touch event handling
    }

    void CParaEngineAppHarmony::OnMotionEvent(float x, float y) {
        // TODO: Implement motion event handling
    }

    bool CParaEngineAppHarmony::Init() {
        // TODO: Implement initialization
        return true;
    }

    void CParaEngineAppHarmony::Cleanup() {
        // TODO: Implement cleanup
    }

} // namespace ParaEngine
