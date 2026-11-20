#include "RenderDeviceWGL.h"
#include "glad/glad.h"
#include "glad/glad_wgl.h"
ParaEngine::RenderDeviceOpenWGL::RenderDeviceOpenWGL(HDC context):m_WGLContext(context)
{

}

ParaEngine::RenderDeviceOpenWGL::~RenderDeviceOpenWGL()
{

}

void ParaEngine::RenderDeviceOpenWGL::SetProfilingPresentation(bool enabled)
{
	if (wglGetSwapIntervalEXT && wglSwapIntervalEXT)
	{
		if (enabled)
			m_savedSwapInterval = wglGetSwapIntervalEXT();
		wglSwapIntervalEXT(enabled ? 0 : m_savedSwapInterval);
	}
}

bool ParaEngine::RenderDeviceOpenWGL::Present()
{
	bool ret = SwapBuffers(m_WGLContext);
	return ret;
}

