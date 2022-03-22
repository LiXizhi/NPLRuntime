//-----------------------------------------------------------------------------
// ParaEngineActivity.h
// Authors: LanZhihong, big
// CreateDate: 2019.12.30
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

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
        static void setScreenOrientation(int type);

    private:
        static const std::string classname;
    };
}
