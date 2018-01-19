#pragma once
#include "RenderTypes.h"
#include <stdint.h>
namespace ParaEngine
{
	struct RenderConfiguration
	{
		bool isWindowed;
		PixelFormat colorFormat;
		PixelFormat depthStencilFormat;
		uint32_t msaaSamples; ///< MSAA samples
		uint32_t screenWidth;
		uint32_t screenHeight;

		IRenderWindow* renderWindow;

		RenderConfiguration()
			:isWindowed(true)
			, colorFormat(PixelFormat::A8R8G8B8)
			, depthStencilFormat(PixelFormat::D24S8)
			, msaaSamples(0)
			, screenWidth(1024)
			, screenHeight(768)
			, renderWindow(nullptr)

		{

		}
	};

}

