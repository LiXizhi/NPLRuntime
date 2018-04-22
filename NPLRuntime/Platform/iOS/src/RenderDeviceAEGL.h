#pragma once
#include "RenderDeviceOpenGL.h"
namespace ParaEngine
{
	class RenderDeviceEGL : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceEGL();
		~RenderDeviceEGL();

		void Reset();
		virtual bool Present() override;
	};
}
