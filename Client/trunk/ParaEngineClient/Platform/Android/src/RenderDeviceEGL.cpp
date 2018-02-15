#include "RenderDeviceEGL.h"


ParaEngine::RenderDeviceEGL::RenderDeviceEGL(EGLDisplay display, EGLSurface surface)
	:m_Display(display)
	,m_Surface(surface)
{

}

ParaEngine::RenderDeviceEGL::~RenderDeviceEGL()
{

}

bool ParaEngine::RenderDeviceEGL::Present()
{
	eglSwapBuffers(m_Display, m_Surface);
}

