#include "RenderDeviceWGL.h"
#include "glad/glad.h"
#include "glad/glad_wgl.h"
ParaEngine::RenderDeviceOpenWGL::RenderDeviceOpenWGL(HDC context):m_WGLContext(context)
{

}

ParaEngine::RenderDeviceOpenWGL::~RenderDeviceOpenWGL()
{

}

bool ParaEngine::RenderDeviceOpenWGL::Present()
{
	if (!RenderDeviceOpenGL::Present()) return false;
	bool ret = SwapBuffers(m_WGLContext);
	return ret;
}

