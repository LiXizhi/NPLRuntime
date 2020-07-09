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