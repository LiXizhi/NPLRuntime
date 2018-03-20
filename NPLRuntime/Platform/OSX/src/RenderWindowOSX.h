#pragma once
#include <string>
#include <cstdint>
//#include "Framework/Interface/Render/IRenderWindow.h"
#include "ParaEngineRenderBase.h"

@class NSWindow;
namespace ParaEngine
{
    class RenderWindowOSX : public CParaEngineRenderBase
    {
    public:
        RenderWindowOSX(const int width,const int height);
        ~RenderWindowOSX();
    public:
        virtual unsigned int GetWidth() const override ;
        virtual unsigned int GetHeight() const override;
        virtual intptr_t GetNativeHandle() const override;
        void PollEvents();
        bool ShouldClose() const;
    public:
         bool OnShouldClose();
    private:
        bool m_shouldClose;
        NSWindow* m_window;
    };
    
}
