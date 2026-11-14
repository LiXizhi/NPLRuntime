#pragma once
#include "Framework/Interface/Render/IRenderContext.h"
#include <windows.h>

namespace ParaEngine
{
	class RenderContextWGL : public IRenderContext
	{
	public:
		RenderContextWGL() = default;
		virtual ~RenderContextWGL() override;

		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::OpenGL; }
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;

	private:
		HWND m_hWnd = nullptr;
		HDC m_hDC = nullptr;
		HGLRC mWGLRenderingContext = nullptr;
	};

	IRenderContext* CreateRenderContextWGL();
}
