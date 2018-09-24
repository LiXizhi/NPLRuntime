#pragma once
#include "RenderDeviceOpenGL.h"
#include "EGL/egl.h"
namespace ParaEngine
{
	class RenderDeviceEGL : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceEGL(EGLDisplay display, EGLSurface surface);
		~RenderDeviceEGL();

		virtual bool Present() override;

	private:
		EGLDisplay m_Display;
		EGLSurface m_Surface;
	};
}
