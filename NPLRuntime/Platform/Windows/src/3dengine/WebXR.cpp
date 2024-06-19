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

void ParaEngine::CParaWebXR::UpdateWebXRView(
    int time,
    Vector4 leftView, Vector3 leftPosition, Vector4 leftOrientation,
    Vector4 rightView, Vector3 rightPosition, Vector4 rightOrientation,
    int viewCount)
{
    // TOOD...
}

