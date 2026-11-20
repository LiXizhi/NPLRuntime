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
		void SetProfilingPresentation(bool enabled) override;
		int m_savedSwapInterval = 0;
		HDC m_WGLContext;
	};
}
