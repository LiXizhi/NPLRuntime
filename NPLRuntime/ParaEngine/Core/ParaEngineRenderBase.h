#pragma once
#include "Framework/Interface/Render/IRenderWindow.h"
namespace ParaEngine
{
	class CParaEngineRenderBase : public IRenderWindow,public IObject{
	public:
		CParaEngineRenderBase() {};
		virtual ~CParaEngineRenderBase() {};
        
        
        virtual float GetScaleX() const override { return 1.0f;};
        virtual float GetScaleY() const override { return 1.0f;};
	};

} // end namespace
