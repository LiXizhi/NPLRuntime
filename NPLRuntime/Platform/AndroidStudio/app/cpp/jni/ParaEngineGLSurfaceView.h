//-----------------------------------------------------------------------------
// ParaEngineGLSurfaceView.h
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#pragma once

#include "JniHelper.h"

namespace ParaEngine {
    struct ParaEngineGLSurfaceView
    {
        static void setIMEKeyboardState(bool bOpen, bool bMoveView, int ctrlBottom);

    private:
        static const std::string classname;
    };

}