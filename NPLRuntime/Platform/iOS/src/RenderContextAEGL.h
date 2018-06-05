#pragma once
#include "Framework/Interface/Render/IRenderContext.h"

namespace ParaEngine
{
	class RenderContextAEGL : public IRenderContext
	{
	public:
		RenderContextAEGL();
		virtual ~RenderContextAEGL();
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	private:
		void ReleaseContext();
	};
}
