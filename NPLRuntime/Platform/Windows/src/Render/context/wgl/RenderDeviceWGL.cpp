#include "RenderDeviceWGL.h"
#include "glad/glad.h"
#include "glad/glad_wgl.h"


#if GLAD_CORE_DEBUG
void _post_call_callback_default(const char *name, void *funcptr, int len_args, ...) {
	GLenum error_code;
	error_code = glad_glGetError();

	if (error_code != GL_NO_ERROR) {
		fprintf(stderr, "GL ERROR %d in %s\n", error_code, name);
	}
}
#endif

ParaEngine::RenderDeviceOpenWGL::RenderDeviceOpenWGL(HDC context):m_WGLContext(context)
{
#if GLAD_CORE_DEBUG
	glad_set_post_callback(_post_call_callback_default);
#endif
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

