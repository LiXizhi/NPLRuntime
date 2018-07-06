#pragma once
#include "Framework/Common/RefCountedObject.hpp"
namespace IParaEngine
{
	class IRenderTarget : public ParaEngine::RefCountedObject
	{
	public:
		IRenderTarget() = default;
		virtual ~IRenderTarget() override = default;
		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;
	};
}