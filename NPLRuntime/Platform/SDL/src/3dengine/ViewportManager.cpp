//-----------------------------------------------------------------------------
// Class:	ViewPortManagerEmscripten
// Authors:	big
// CreateDate:	2024.6.3
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "SceneState.h"
#include "ViewportManager.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "MoviePlatform.h"
#include "3dengine/RenderTarget.h"
#include "2dengine/GUIRoot.h"
#include <iostream>

#include "../emscripten/webxr.h"

void ParaEngine::CViewportManager::SetIsXR(bool isXR)
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

bool ParaEngine::CViewportManager::GetIsXR()
{
    return m_isXR;
}

void ParaEngine::CViewportManager::SaveWebXRView(int time, ParaWebXRRigidTransform *headPose, ParaWebXRView views[2], int viewCount)
{
    m_webXRTime = time;
    m_webXRHeadPose = headPose;
    memcpy(m_webXRViews, views, sizeof(views));
    m_webXRViewCount = viewCount;
}
