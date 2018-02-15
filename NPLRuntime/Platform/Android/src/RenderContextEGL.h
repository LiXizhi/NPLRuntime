#pragma once
#include "Framework/Interface/Render/IRenderContext.h"
namespace ParaEngine
{
	class RenderContextEGL : public IRenderContext
	{
	public:
		RenderContextEGL() = default;
		virtual ~RenderContextEGL() override = default;
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	};
}