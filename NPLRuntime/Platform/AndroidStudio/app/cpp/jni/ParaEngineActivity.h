#pragma once

#include "JniHelper.h"

namespace ParaEngine {

    struct GLContextAttrs
    {
        int redBits;
        int greenBits;
        int blueBits;
        int alphaBits;
        int depthBits;
        int stencilBits;
        int multisamplingCount;
    };

    struct ParaEngineActivity
    {
    public:
        static std::string getLauncherIntentData();

    private:
        static const std::string classname;
    };
}
