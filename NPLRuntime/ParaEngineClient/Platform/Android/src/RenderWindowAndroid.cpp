#include "RenderWindowAndroid.h"


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

