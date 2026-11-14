#pragma once
#include "Framework/Interface/Render/IRenderContext.h"

namespace ParaEngine
{
	class RenderContextD3D11 : public IRenderContext
	{
	public:
		RenderContextD3D11() = default;
		virtual ~RenderContextD3D11() override = default;

		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::D3D11; }
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	};

	IRenderContext* CreateRenderContextD3D11();
}
