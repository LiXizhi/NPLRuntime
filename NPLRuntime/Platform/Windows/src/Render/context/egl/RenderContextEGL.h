#pragma once
#include "Framework/Interface/Render/IRenderContext.h"
#include <windows.h>
#include "EGL/egl.h"
namespace ParaEngine
{
	
	class RenderContextEGL : public IRenderContext
	{
	public:
		RenderContextEGL() = default;
		virtual ~RenderContextEGL() override = default;
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	private:
		EGLDisplay m_display;
		EGLContext m_context;
		EGLSurface m_surface;

	};
}