#pragma once
#include "Framework/Interface/Render/IRenderContext.h"
#include <windows.h>
namespace ParaEngine
{
	
	class RenderContextWGL : public IRenderContext
	{
	public:
		RenderContextWGL() = default;
		virtual ~RenderContextWGL() override = default;
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	private:
		HGLRC mWGLRenderingContext;
	};
}