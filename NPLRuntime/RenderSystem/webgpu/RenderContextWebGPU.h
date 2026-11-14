#pragma once
#include "Framework/Interface/Render/IRenderContext.h"

namespace ParaEngine
{
	class RenderContextWebGPU : public IRenderContext
	{
	public:
		RenderContextWebGPU() = default;
		virtual ~RenderContextWebGPU() override = default;

		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::WebGPU; }
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	};

	IRenderContext* CreateRenderContextWebGPU();
}
