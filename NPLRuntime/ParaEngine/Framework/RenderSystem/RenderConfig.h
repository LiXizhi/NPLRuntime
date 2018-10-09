#pragma once
#include "RenderTypes.h"
#include <stdint.h>
namespace ParaEngine
{
	struct RenderConfiguration
	{
		uint32_t screenWidth;
		uint32_t screenHeight;
		IRenderWindow* renderWindow;
		bool isWindowed;
		RenderConfiguration()
			: screenWidth(960)
			, screenHeight(540)
			, renderWindow(nullptr)
			, isWindowed(false)
		{

		}
	};

}

