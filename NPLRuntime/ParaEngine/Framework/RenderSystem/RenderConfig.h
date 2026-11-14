#pragma once
#include "RenderTypes.h"
#include "Framework/Interface/Render/RHITypes.h"
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
		ERHIBackend preferredBackend;
		bool debugValidation;
		bool allowSoftwareFallback;

		RenderConfiguration()
			: isWindowed(true)
			, colorFormat(PixelFormat::A8R8G8B8)
			, depthStencilFormat(PixelFormat::D24S8)
			, msaaSamples(0)
			, screenWidth(1024)
			, screenHeight(768)
			, renderWindow(nullptr)
			, preferredBackend(ERHIBackend::Auto)
			, debugValidation(false)
			, allowSoftwareFallback(true)
		{

		}
	};

}

