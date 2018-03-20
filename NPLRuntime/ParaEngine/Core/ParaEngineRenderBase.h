#pragma once
#include "Framework/Interface/Render/IRenderWindow.h"
namespace ParaEngine
{
	class CParaEngineRenderBase : public IRenderWindow,public IObject{
	public:
		CParaEngineRenderBase() {};
		virtual ~CParaEngineRenderBase() {};
	};

} // end namespace