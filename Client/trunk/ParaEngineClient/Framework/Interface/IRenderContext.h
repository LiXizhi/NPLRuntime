#pragma once;
#include <vector>
#include <memory>
#include "RenderCommon.h"
namespace ParaEngine
{

	class IRenderDevice;
	class IRenderWindow;

	class IRenderContext
	{
	public: 
		IRenderContext() = default;
		virtual ~IRenderContext() = default;
		virtual IRenderDevice* CreateDevice(const RenderDeviceConfiguration& cfg) = 0;
		virtual bool ResetDevice(IRenderDevice* device, const RenderDeviceConfiguration& cfg) = 0;
	};
	using IRenderContextPtr = std::shared_ptr<IRenderContext>;
}
