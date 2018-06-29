#pragma once
#include "IRenderTarget.h"
namespace IParaEngine
{
	class ITexture : public IRenderTarget
	{
	public:
		ITexture() = default;
		virtual ~ITexture() override = default;
		virtual void* Lock(unsigned int level, unsigned int& pitch) = 0;
		virtual void Unlock() = 0;
	};
}