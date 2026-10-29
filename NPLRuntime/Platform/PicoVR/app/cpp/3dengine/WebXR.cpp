//-----------------------------------------------------------------------------
// Class: WebXR.cpp
// Authors: big
// CreateDate: 2026.10.28
// Desc: IParaWebXR for PICO native OpenXR.
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ViewportManager.h"
#include "NPLRuntime.h"
#include "PicoXR/PicoOpenXR.h"
#include "WebXR.h"
#include <cstring>

namespace ParaEngine
{
    // Same entry as "/stereo -s webxr" of the WebXR build. WebXRCamera does head tracking, controller rays,
    // mouse emulation and 3D GUI, and calls SetIsXR(true). It needs GameLogic, which is loaded after startup.
    static const char* s_activateWebXRCamera =
        "if(GameLogic and GameLogic.options and GameLogic.options.SetStereoConvergenceOffset) then "
        "NPL.load(\"(gl)script/ide/System/Scene/Cameras/WebXRCamera.lua\"); "
        "local WebXRCamera = commonlib.gettable(\"System.Scene.Cameras.WebXRCamera\"); "
        "if(WebXRCamera.activate) then WebXRCamera:activate(); end "
        "end";

    CParaWebXR::CParaWebXR()
    {
        m_isXR = false;
        m_webXRTime = 0;
        m_webXRViewCount = 0;
        memset(m_webXRLeftHandKey, 0, sizeof(m_webXRLeftHandKey));
        memset(m_webXRRightHandKey, 0, sizeof(m_webXRRightHandKey));
        m_webXRLeftView = m_webXRRightView = Vector4(0, 0, 0, 0);
        m_webXRLeftOrientation = m_webXRRightOrientation = Vector4(0, 0, 0, 1);
        m_webXRLeftHandOrientation = m_webXRRightHandOrientation = Vector4(0, 0, 0, 1);
        m_webXRLeftAxes = m_webXRRightAxes = Vector4(0, 0, 0, 0);
        m_webXRLeftPosition = m_webXRRightPosition = Vector3(0, 0, 0);
        m_webXRLeftHandPosition = m_webXRRightHandPosition = Vector3(0, 0, 0);
        m_webXRLeftFov = m_webXRRightFov = Vector4(0, 0, 0, 0);
    }

    void CParaWebXR::SetIsXR(bool isXR)
    {
        // isXR tells the script side whether WebXRCamera is active. The headset always shows stereo,
        // so the layout stays stereo even when it is turned off.
        CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_STEREO_WEBXR);
        m_isXR = isXR;
    }

    bool CParaWebXR::GetIsXR()
    {
        return m_isXR;
    }

    bool CParaWebXR::IsPressLeftHandKey(int keyCode)
    {
        return keyCode >= 0 && keyCode < 7 && m_webXRLeftHandKey[keyCode] == 1;
    }

    bool CParaWebXR::IsPressRightHandKey(int keyCode)
    {
        return keyCode >= 0 && keyCode < 7 && m_webXRRightHandKey[keyCode] == 1;
    }

    void CParaWebXR::UpdateWebXRView(
        int time,
        Vector4 leftView, Vector3 leftPosition, Vector4 leftOrientation,
        Vector4 rightView, Vector3 rightPosition, Vector4 rightOrientation,
        int viewCount,
        Vector3 leftHandPosition, Vector3 rightHandPosition,
        Vector4 leftHandOrientation, Vector4 rightHandOrientation,
        int leftHandKey[7], int rightHandKey[7], Vector4 leftAxes, Vector4 rightAxes)
    {
        m_webXRTime = time;
        m_webXRViewCount = viewCount;

        m_webXRLeftView = leftView;
        m_webXRLeftPosition = leftPosition;
        m_webXRLeftOrientation = leftOrientation;

        m_webXRRightView = rightView;
        m_webXRRightPosition = rightPosition;
        m_webXRRightOrientation = rightOrientation;

        m_webXRLeftHandPosition = leftHandPosition;
        m_webXRRightHandPosition = rightHandPosition;
        m_webXRLeftHandOrientation = leftHandOrientation;
        m_webXRRightHandOrientation = rightHandOrientation;

        memcpy(m_webXRLeftHandKey, leftHandKey, sizeof(int) * 7);
        memcpy(m_webXRRightHandKey, rightHandKey, sizeof(int) * 7);
        m_webXRLeftAxes = leftAxes;
        m_webXRRightAxes = rightAxes;
    }

    void CParaWebXR::UpdatePicoXRFrame(const PicoXRFrameData& data)
    {
        auto toVector3 = [](const float* v) { return Vector3(v[0], v[1], v[2]); };
        auto toVector4 = [](const float* v) { return Vector4(v[0], v[1], v[2], v[3]); };

        // keep the last known controller pose when tracking is lost for a frame
        Vector3 leftHandPosition = data.handActive[0] ? toVector3(data.handPosition[0]) : m_webXRLeftHandPosition;
        Vector4 leftHandOrientation = data.handActive[0] ? toVector4(data.handOrientation[0]) : m_webXRLeftHandOrientation;
        Vector3 rightHandPosition = data.handActive[1] ? toVector3(data.handPosition[1]) : m_webXRRightHandPosition;
        Vector4 rightHandOrientation = data.handActive[1] ? toVector4(data.handOrientation[1]) : m_webXRRightHandOrientation;

        int leftHandKey[7], rightHandKey[7];
        memcpy(leftHandKey, data.handKey[0], sizeof(leftHandKey));
        memcpy(rightHandKey, data.handKey[1], sizeof(rightHandKey));

        UpdateWebXRView(
            data.time,
            toVector4(data.viewport[0]), toVector3(data.eyePosition[0]), toVector4(data.eyeOrientation[0]),
            toVector4(data.viewport[1]), toVector3(data.eyePosition[1]), toVector4(data.eyeOrientation[1]),
            data.viewCount,
            leftHandPosition, rightHandPosition, leftHandOrientation, rightHandOrientation,
            leftHandKey, rightHandKey, toVector4(data.handAxes[0]), toVector4(data.handAxes[1]));

        m_webXRLeftFov = toVector4(data.eyeFov[0]);
        m_webXRRightFov = toVector4(data.eyeFov[1]);

        CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_STEREO_WEBXR);

        // try about once a second until the script side has turned on WebXRCamera
        static int s_frameCount = 0;
        if (!m_isXR && (s_frameCount++ % 90) == 0)
        {
            auto pState = CGlobals::GetNPLRuntime()->GetMainRuntimeState();
            if (pState)
                pState->DoString(s_activateWebXRCamera, (int)strlen(s_activateWebXRCamera));
        }
    }

    int CParaWebXR::InstallFields(CAttributeClass* pClass, bool bOverride)
    {
        IParaWebXR::InstallFields(pClass, bOverride);
        pClass->AddField("webXRLeftFov", FieldType_Vector4, NULL, (void*)GetWebXRLeftFov_s, NULL, NULL, bOverride);
        pClass->AddField("webXRRightFov", FieldType_Vector4, NULL, (void*)GetWebXRRightFov_s, NULL, NULL, bOverride);
        return S_OK;
    }
}
