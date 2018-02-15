#pragma once
#include "RenderDeviceOpenGL.h"
namespace ParaEngine
{
	class RenderDeviceOpenWGL : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceOpenWGL(HDC context);
		~RenderDeviceOpenWGL();

		virtual bool Present() override;

	private:
		HDC m_WGLContext;
	};
}
