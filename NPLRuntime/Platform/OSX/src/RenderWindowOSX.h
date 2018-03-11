#pragma once
#include <string>
#include <cstdint>
#include <Cocoa/Cocoa.h>
#include "Framework/Interface/Render/IRenderWindow.h"

namespace ParaEngine
{
    class RenderWindowOSX : public IRenderWindow
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
