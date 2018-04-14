#pragma once
#include "ParaEngineRenderBase.h"
namespace ParaEngine
{
	class RenderWindowiOS : public CParaEngineRenderBase
	{

	public:
		RenderWindowiOS();
		~RenderWindowiOS();
		virtual unsigned int GetWidth() const override;
		virtual unsigned int GetHeight() const override;
		virtual intptr_t GetNativeHandle() const override;

	private:
		
	};
}
