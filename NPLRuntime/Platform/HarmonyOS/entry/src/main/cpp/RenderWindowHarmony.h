#pragma once
#include "Framework/Interface/Render/IRenderWindow.h"

namespace ParaEngine
{
    class RenderWindowHarmony : public IRenderWindow
    {
    public:
        RenderWindowHarmony(int width, int height);
        virtual ~RenderWindowHarmony();
        
        virtual unsigned int GetWidth() const override { return m_width; }
        virtual unsigned int GetHeight() const override { return m_height; }
        virtual float GetScaleX() const override { return 1.0f; }
        virtual float GetScaleY() const override { return 1.0f; }
        virtual intptr_t GetNativeHandle() const override { return 0; }
        
    private:
        int m_width;
        int m_height;
    };
}
