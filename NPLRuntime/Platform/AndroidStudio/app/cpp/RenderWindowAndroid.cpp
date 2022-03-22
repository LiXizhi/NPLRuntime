//-----------------------------------------------------------------------------
// RenderWindowAndroid.cpp
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "RenderWindowAndroid.h"
#include "jni/JniHelper.h"

ParaEngine::IRenderWindow* CreateParaRenderWindow(const int width, const int height)
{
    return new ParaEngine::RenderWindowAndroid(width, height);
}

namespace ParaEngine {
    RenderWindowAndroid::RenderWindowAndroid(int w, int h)
                        :m_width(w),
                         m_height(h)
    {
    }

    RenderWindowAndroid::~RenderWindowAndroid()
    {
    }

    unsigned int RenderWindowAndroid::GetWidth() const
    {
        return m_width;
    }

    unsigned int RenderWindowAndroid::GetHeight() const
    {
        return m_height;
    }

    intptr_t RenderWindowAndroid::GetNativeHandle() const
    {
        return (intptr_t)JniHelper::getNativeWindow();
    }
}
