//-----------------------------------------------------------------------------
// Class: WebXR.h
// Authors: big
// Emails: onedou@126.com
// CreateDate: 2024.6.7
// ModifyDate: 2024.6.12
//-----------------------------------------------------------------------------

#pragma once
#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "ViewportManager.h"
#include "Globals.h"
#include "../../emscripten/webxr.h"
#include "WebXR.h"
#include <iostream>

void ParaEngine::CParaWebXR::SetIsXR(bool isXR)
{
    if (isXR)
    {
        webxr_request_session(
            WEBXR_SESSION_MODE_IMMERSIVE_VR,
            WEBXR_SESSION_FEATURE_LOCAL,
            WEBXR_SESSION_FEATURE_LOCAL
        );
        CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_STEREO_WEBXR);
    }
    else
    {
        CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_DEFAULT);
    }

    m_isXR = isXR;
}

bool ParaEngine::CParaWebXR::GetIsXR()
{
    return m_isXR;
}

bool ParaEngine::CParaWebXR::IsPressLeftHandKey(int keyCode)
{
    if (m_webXRLeftHandKey[keyCode] == 1)
        return true;
    else
        return false;
}

bool ParaEngine::CParaWebXR::IsPressRightHandKey(int keyCode)
{
    if (m_webXRRightHandKey[keyCode] == 1)
        return true;
    else
        return false;
}

void ParaEngine::CParaWebXR::UpdateWebXRView(
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
