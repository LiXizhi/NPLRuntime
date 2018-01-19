#pragma once
#include "Framework/Interface/Render/IRenderContext.h"
#include <windows.h>
namespace ParaEngine
{
	
	class RenderContextOpenGL : public IRenderContext
	{
	public:
		static RenderContextOpenGL* Create();
		virtual ~RenderContextOpenGL() override;
		

		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;


		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;

	private:
		RenderContextOpenGL();
	};
}