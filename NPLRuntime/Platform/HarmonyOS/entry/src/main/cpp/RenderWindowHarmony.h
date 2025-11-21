#pragma once
#include "Framework/Interface/Render/IRenderWindow.h"

namespace ParaEngine
{
    class RenderWindowHarmony : public IRenderWindow
    {
    public:
        RenderWindowHarmony(int width, int height);
        virtual ~RenderWindowHarmony();
        
        virtual void* GetNativeHandle() override { return nullptr; }
    private:
        int m_width;
        int m_height;
    };
}
