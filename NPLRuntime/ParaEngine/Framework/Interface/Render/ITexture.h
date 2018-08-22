#pragma once
#include "Framework/Interface/Render/IDeviceResource.h"
#include "Framework/Common/Math/Rect.h"
#include "Framework/RenderSystem/RenderTypes.h"
#include "Framework/Common/Image.hpp"
#include "Framework/Common/Math/Color4f.h"
namespace IParaEngine
{
	class ITexture : public IDeviceResource
	{
	public:
		ITexture() = default;
		virtual ~ITexture() override = default;
		virtual bool CopyTo(ITexture* target) = 0;
		virtual bool StretchRect(ITexture* target, const ParaEngine::Rect* srcRect, const ParaEngine::Rect* targetRect,const ParaEngine::ETextureFilter filter) = 0;
		
		virtual bool UpdateImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels) = 0;
		virtual ParaEngine::ImagePtr GetImage(uint32_t level) = 0;

		virtual ParaEngine::ETextureFilter GetMinFilter() const = 0;
		virtual ParaEngine::ETextureFilter GetMagFilter() const = 0;
		virtual ParaEngine::ETextureFilter GetMipFilter() const = 0;

		virtual bool SetMinFilter(ParaEngine::ETextureFilter type) = 0;
		virtual bool SetMipFilter(ParaEngine::ETextureFilter type) = 0;
		virtual bool SetMagFilter(ParaEngine::ETextureFilter type) = 0;

		virtual ParaEngine::ETextureWrapMode GetAddressU() const = 0;
		virtual ParaEngine::ETextureWrapMode GetAddressV() const = 0;

		virtual bool SetAddressU(ParaEngine::ETextureWrapMode mode) = 0;
		virtual bool SetAddressV(ParaEngine::ETextureWrapMode mode) = 0;

		virtual bool SetBorderColor(const ParaEngine::Color4f& color) = 0;
		virtual ParaEngine::Color4f GetBorderColor() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

	};
}