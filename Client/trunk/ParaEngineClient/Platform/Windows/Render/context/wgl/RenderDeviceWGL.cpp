#include "RenderDeviceWGL.h"
#include "glad/glad.h"
#include "glad/glad_wgl.h"
ParaEngine::RenderDeviceOpenWGL::RenderDeviceOpenWGL(HDC context)
{

}

ParaEngine::RenderDeviceOpenWGL::~RenderDeviceOpenWGL()
{

}

bool ParaEngine::RenderDeviceOpenWGL::Present()
{
	SwapBuffers(m_WGLContext);
	return true;
}

