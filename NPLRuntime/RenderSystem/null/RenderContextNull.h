#pragma once
#include "Framework/Interface/Render/IRenderContext.h"

namespace ParaEngine
{
	class RenderContextNull : public IRenderContext
	{
	public:
		RenderContextNull() = default;
		virtual ~RenderContextNull() = default;
		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::Null; }
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	};

	IRenderContext* CreateRenderContextNull();
}
