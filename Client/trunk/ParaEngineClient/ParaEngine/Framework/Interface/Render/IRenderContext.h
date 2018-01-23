#pragma once;
#include "Framework/RenderSystem/RenderConfig.h"
#include <vector>
#include <memory>
namespace ParaEngine
{

	class IRenderDevice;
	class IRenderWindow;

	class IRenderContext
	{
	public: 
		static IRenderContext* Create();
		virtual ~IRenderContext() = default;
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) = 0;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) = 0;
	protected:
		IRenderContext() = default;

	};
	using IRenderContextPtr = std::shared_ptr<IRenderContext>;
}
