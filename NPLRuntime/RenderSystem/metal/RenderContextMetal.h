#pragma once
#include "Framework/Interface/Render/IRenderContext.h"

namespace ParaEngine
{
	IRenderContext* CreateRenderContextMetal();

	class RenderContextMetal : public IRenderContext
	{
	public:
		RenderContextMetal() = default;
		virtual ~RenderContextMetal() override = default;

		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::Metal; }
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	};
}
