//-----------------------------------------------------------------------------
// Class:	WebXR
// Authors:	big
// Emails:	onedou@126.com
// CreateDate:	2024.6.19
//-----------------------------------------------------------------------------

#pragma once
#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "ViewportManager.h"
#include "Globals.h"
#include "WebXR.h"
#include <iostream>

void ParaEngine::CParaWebXR::SetIsXR(bool isXR)
{
    if (isXR)
    {
        CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_STEREO_LEFT_RIGHT);
    }
    else
    {
        CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_DEFAULT);
    }
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
    // disabled.
}

