#pragma once

#include "JniHelper.h"

namespace ParaEngine {
    struct ParaEngineGLSurfaceView
    {
        static void setIMEKeyboardState(bool bOpen);

    private:
        static const std::string classname;
    };

}