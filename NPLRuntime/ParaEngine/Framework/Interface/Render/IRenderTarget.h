#pragma once
#include "Framework/Common/RefCountedObject.hpp"
namespace IParaEngine
{
	class IRenderTarget : public ParaEngine::RefCountedObject
	{
	public:
		IRenderTarget() = default;
		virtual ~IRenderTarget() override = default;
	};
}