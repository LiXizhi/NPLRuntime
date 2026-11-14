#pragma once
#include "Framework/Interface/Render/IRenderContext.h"

namespace ParaEngine
{
	class RenderContextVulkan : public IRenderContext
	{
	public:
		RenderContextVulkan() = default;
		virtual ~RenderContextVulkan() override = default;

		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::Vulkan; }
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	};

	IRenderContext* CreateRenderContextVulkan();
}
