#pragma once
#include <string>
#include <cstdint>
#include "ParaEngineRenderBase.h"
#include "Framework/InputSystem/VirtualKey.h"

@class NSWindow;
namespace ParaEngine
{
    class RenderWindowOSX : public CParaEngineRenderBase
    {
    public:
        RenderWindowOSX(const int width,const int height);
        virtual ~RenderWindowOSX() override;
    public:
        virtual unsigned int GetWidth() const override;
        virtual unsigned int GetHeight() const override;
        virtual intptr_t GetNativeHandle() const override;
        void PollEvents();
        bool ShouldClose() const;
        bool isPressCommand = false;

        void setTitle(const char* title);
        const char* getTitle();
    public:
        bool OnShouldClose();
        void OnMouseButton(EMouseButton button, EKeyState state,uint32_t x,uint32_t y);
        void OnMouseMove(uint32_t x, uint32_t y);
        void OnMouseWhell(float deltaX, float deltaY);
        void OnKey(EVirtualKey key, EKeyState state);
        void OnChar(unsigned int character);
        
        
    protected:
        bool m_shouldClose;
        NSWindow* m_window;
        float m_scrollMouseX;
        float m_scrollMouseY;
    };
    
}
