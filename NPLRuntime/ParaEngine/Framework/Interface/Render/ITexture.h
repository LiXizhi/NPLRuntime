#pragma once
#include "IRenderTarget.h"
#include "Framework/Common/Math/Rect.h"
namespace IParaEngine
{
	class ITexture : public IRenderTarget
	{
	public:
		ITexture() = default;
		virtual ~ITexture() override = default;
		virtual void* Lock(unsigned int level, unsigned int& pitch,const ParaEngine::Rect* rect) = 0;
		virtual void Unlock(unsigned int level) = 0;
		virtual bool CopyTo(ITexture* target) = 0;
		virtual bool StretchRect(ITexture* target, const ParaEngine::Rect* srcRect, const ParaEngine::Rect* targetRect,const ParaEngine::ETextureFilterType filter) = 0;
	};
}