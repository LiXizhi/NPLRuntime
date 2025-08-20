#import <UIKit/UIKit.h>

#include "ParaEngine.h"
#include "RenderWindowiOS.h"

using namespace ParaEngine;

IRenderWindow* CreateParaRenderWindow(const int width, const int height)
{
	return new RenderWindowiOS(nullptr);
}

ParaEngine::RenderWindowiOS::RenderWindowiOS(UIView* view)
:m_view(view)
{
    if (view == nullptr) {
        NSLog(@"Warning: RenderWindowiOS created with null view");
    }
}

ParaEngine::RenderWindowiOS::~RenderWindowiOS()
{
}

float ParaEngine::RenderWindowiOS::GetScaleX() const
{
    return [UIScreen mainScreen].scale;
}

float ParaEngine::RenderWindowiOS::GetScaleY() const
{
    return [UIScreen mainScreen].scale;
}

unsigned int ParaEngine::RenderWindowiOS::GetWidth() const
{
    if (m_view == nullptr) {
        return 0;
    }
    return (int)m_view.frame.size.width * [UIScreen mainScreen].scale;
}

unsigned int ParaEngine::RenderWindowiOS::GetHeight() const
{
    if (m_view == nullptr) {
        return 0;
    }
    return (int)m_view.frame.size.height * [UIScreen mainScreen].scale;
}

intptr_t ParaEngine::RenderWindowiOS::GetNativeHandle() const
{
    if (m_view == nullptr) {
        return 0;
    }
    return (intptr_t)(__bridge void*)m_view; //(intptr_t)m_view;
}

UIView* ParaEngine::RenderWindowiOS::GetView() const
{
    return m_view;
}
