#include "RenderDeviceEGL.h"


ParaEngine::RenderDeviceEGL::RenderDeviceEGL(EGLDisplay display, EGLSurface surface)
	:m_Display(display)
	,m_Surface(surface)
{

}

ParaEngine::RenderDeviceEGL::~RenderDeviceEGL()
{

}

void ParaEngine::RenderDeviceEGL::Reset(EGLDisplay display, EGLSurface surface)
{
	m_Display = display;
	m_Surface = surface;
}

bool ParaEngine::RenderDeviceEGL::Present()
{
	return eglSwapBuffers(m_Display, m_Surface) == EGL_TRUE;

}

