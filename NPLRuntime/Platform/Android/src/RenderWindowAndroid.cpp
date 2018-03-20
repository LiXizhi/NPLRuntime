#include "ParaEngine.h"
#include "RenderWindowAndroid.h"

using namespace ParaEngine;

IRenderWindow* CreateParaRenderWindow(const int width, const int height)
{
	return new RenderWindowAndroid(nullptr);
}

ParaEngine::RenderWindowAndroid::RenderWindowAndroid(ANativeWindow* nativeWindow):m_NativeWindow(nativeWindow)
{

}

ParaEngine::RenderWindowAndroid::~RenderWindowAndroid()
{
	m_NativeWindow = nullptr;
}

unsigned int ParaEngine::RenderWindowAndroid::GetWidth() const
{
	return ANativeWindow_getWidth(m_NativeWindow);
}

unsigned int ParaEngine::RenderWindowAndroid::GetHeight() const
{
	return ANativeWindow_getHeight(m_NativeWindow);
}

intptr_t ParaEngine::RenderWindowAndroid::GetNativeHandle() const
{
	return (intptr_t)m_NativeWindow;
}

