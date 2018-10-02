#pragma once
#include "RenderTypes.h"
#include <stdint.h>
namespace ParaEngine
{
	struct RenderConfiguration
	{
		bool isWindowed;
		EPixelFormat colorFormat;
		EPixelFormat depthStencilFormat;
		uint32_t msaaSamples; ///< MSAA samples
		uint32_t screenWidth;
		uint32_t screenHeight;

		IRenderWindow* renderWindow;

		RenderConfiguration()
			:isWindowed(true)
			, colorFormat(EPixelFormat::A8R8G8B8)
			, depthStencilFormat(EPixelFormat::D24S8)
			, msaaSamples(0)
			, screenWidth(1024)
			, screenHeight(768)
			, renderWindow(nullptr)

		{

		}
	};

}

