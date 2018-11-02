#pragma once
#include "Framework/Interface/Render/IRenderContext.h"

namespace ParaEngine {
    class RenderContxtAGL : public IRenderContext
    {
    public:
        virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
        virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
    };
}
