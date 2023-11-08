#pragma once
#import <Cocoa/Cocoa.h>
#include <string>
#include <cstdint>
#include "ParaEngine.h"
#include "ParaEngineRenderBase.h"
#include "Framework/InputSystem/VirtualKey.h"

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

        void setTitle(const char* title);
        const char* getTitle();
        
        void GetScaleFactor(double& x, double& y) const;
        float currentBackingScaleFactor = 1;
    public:
        bool OnShouldClose();
        
               
        void OnMouseDown(EMouseButton button, NSEvent* event);
        void OnMouseUp(EMouseButton button, NSEvent* event);
        void OnMouseMove(NSEvent* event);
        void OnKey(EKeyState state, NSEvent* event);
        void OnFlagsChanged(NSEvent* event);
        void OnScrollWheel(NSEvent* event);
        void OnInsertText(NSString* string);
        
        NSRect GetCharacterRect();
    private:
        void OnMouseEvent(EMouseButton button, EKeyState state, NSEvent* event);
        void OnMouseButton(EMouseButton button, EKeyState state,uint32_t x,uint32_t y);
        void OnKey(EVirtualKey key, EKeyState state);
        
        void OnMouseMove(uint32_t x, uint32_t y);
        void OnMouseWhell(float deltaX, float deltaY);
               
        void OnChar(unsigned int character);
    protected:
        bool m_shouldClose;
        NSWindow* m_window;
        float m_scrollMouseX;
        float m_scrollMouseY;
        EVirtualKey m_curPressKey = ParaEngine::EVirtualKey::KEY_UNKNOWN;
    };
}
