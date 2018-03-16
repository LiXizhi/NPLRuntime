#pragma once
#include "Framework/Interface/Render/IRenderContext.h"

typedef void *EGLContext;
typedef void *EGLDisplay;
typedef void *EGLSurface;

namespace ParaEngine
{
	class RenderContextEGL : public IRenderContext
	{
	public:
		RenderContextEGL();
		virtual ~RenderContextEGL();
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	private:
		EGLDisplay m_display;
		EGLContext m_context;
		EGLSurface m_surface;
		void ReleaseContext();
	};
}